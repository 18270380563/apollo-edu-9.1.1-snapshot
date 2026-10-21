"""collector.py 与 monitor_server.py 的最小回归测试。

只依赖标准库。测试用临时目录伪造 /proc、DAG 和 maps，不需要真实 Apollo。
运行：python3 test_collector.py
"""

from __future__ import annotations

import http.client
import json
import sys
import tempfile
import threading
import unittest
from pathlib import Path
from typing import Any, Callable, Mapping, Sequence

sys.dont_write_bytecode = True

from collector import (  # noqa: E402
    Collector,
    CommandResult,
    CommandRunner,
    validate_config,
)
import monitor_server  # noqa: E402

BASE_DIR = Path(__file__).resolve().parent
REAL_CONFIG = BASE_DIR / "config" / "modules.json"


class FakeRunner(CommandRunner):
    """可注入的工具查找与命令结果；调用记录用于确认没有执行写操作。"""

    def __init__(
        self,
        tools: Mapping[str, str] | None = None,
        handler: Callable[[Sequence[str]], CommandResult] | None = None,
    ) -> None:
        self.tools = dict(tools or {})
        self.handler = handler
        self.calls: list[list[str]] = []

    def which(self, name: str) -> str | None:
        return self.tools.get(name)

    def run(
        self,
        argv: Sequence[str],
        timeout: float = 3.0,
        env: Mapping[str, str] | None = None,
    ) -> CommandResult:
        call = [str(item) for item in argv]
        self.calls.append(call)
        if self.handler is None:
            return CommandResult(argv=tuple(call), error="测试未提供命令结果")
        return self.handler(call)


def write_module_config(path: Path, modules: list[dict[str, Any]]) -> None:
    path.write_text(
        json.dumps(
            {
                "schema_version": 1,
                "apollo": {"root_markers": ["cyber", "modules"]},
                "modules": modules,
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )


def canbus_module() -> dict[str, Any]:
    return {
        "id": "canbus",
        "name": "底盘与车辆接口",
        "description": "测试模块",
        "process_keywords": ["mainboard", "modules/canbus/dag/canbus.dag"],
        "dags": ["modules/canbus/dag/canbus.dag"],
        "launch": "modules/canbus/launch/canbus.launch",
        "conf": [],
        "suggested_channels": ["/apollo/canbus/chassis", "/apollo/control"],
    }


def make_apollo_root(tmp: Path) -> Path:
    root = tmp / "apollo"
    (root / "cyber").mkdir(parents=True, exist_ok=True)
    dag_dir = root / "modules" / "canbus" / "dag"
    dag_dir.mkdir(parents=True, exist_ok=True)
    (dag_dir / "canbus.dag").write_text(
        """
module_config {
  module_library : "modules/canbus/libcanbus_component.so"
  timer_components { class_name : "CanbusComponent" }
}
""",
        encoding="utf-8",
    )
    lib_dir = root / "bazel-bin" / "modules" / "canbus"
    lib_dir.mkdir(parents=True, exist_ok=True)
    (lib_dir / "libcanbus_component.so").write_bytes(b"fake")
    return root


def write_fake_proc(
    proc_root: Path,
    root: Path,
    pid: int = 4242,
    with_maps: bool = True,
    pid_dir: Path | None = None,
) -> Path:
    pid_dir = pid_dir or (proc_root / str(pid))
    pid_dir.mkdir(parents=True, exist_ok=True)
    cmdline = [
        "mainboard",
        "-d",
        "/apollo/modules/canbus/dag/canbus.dag",
        "-p",
        "canbus",
    ]
    (pid_dir / "cmdline").write_bytes(b"\0".join(part.encode() for part in cmdline) + b"\0")
    (pid_dir / "status").write_text(
        "Name:\tmainboard\nState:\tS (sleeping)\nPPid:\t1\nUid:\t1000\t1000\t1000\t1000\nVmRSS:\t 20480 kB\n",
        encoding="utf-8",
    )
    if with_maps:
        target = root / "bazel-bin" / "modules" / "canbus" / "libcanbus_component.so"
        maps = (
            "7f0000000000-7f0000001000 r-xp 00000000 08:01 123 "
            + str(target)
            + "\n"
            "7f0000001000-7f0000002000 r-xp 00000000 08:01 124 "
            + "/usr/lib/x86_64-linux-gnu/libc.so.6\n"
            "7f0000002000-7f0000003000 rw-p 00000000 00:00 0 [heap]\n"
        )
        (pid_dir / "maps").write_text(maps, encoding="utf-8")
    return pid_dir


def make_collector(
    tmp: Path,
    runner: FakeRunner,
    *,
    proc_root: Path | None = None,
    root: Path | None = None,
    config: Path | None = None,
) -> Collector:
    config = config or (tmp / "modules.json")
    if not config.exists():
        write_module_config(config, [canbus_module()])
    return Collector(
        config_path=config,
        apollo_root=root or (tmp / "missing_apollo"),
        proc_root=proc_root or (tmp / "missing_proc"),
        runner=runner,
        cyber_tool_dir=None,
    )


def cyber_handler(call: Sequence[str]) -> CommandResult:
    tool = Path(call[0]).name
    if tool == "cyber_channel":
        return CommandResult(
            argv=tuple(call),
            returncode=0,
            stdout=(
                "The number of channels is: 2\n"
                "/apollo/canbus/chassis\n"
                "/apollo/control\n"
            ),
        )
    if tool == "cyber_node":
        return CommandResult(
            argv=tuple(call),
            returncode=0,
            stdout="Number of active nodes: 1\ncanbus\n",
        )
    return CommandResult(argv=tuple(call), error="意外命令")


class CommandRunnerTestCase(unittest.TestCase):
    def test_missing_command_and_timeout_are_captured(self) -> None:
        runner = CommandRunner()
        missing = runner.run(["apollo-monitor-command-that-does-not-exist"], timeout=0.2)
        self.assertFalse(missing.ok)
        self.assertTrue(missing.error)

        timed_out = runner.run(["sleep", "2"], timeout=0.1)
        self.assertTrue(timed_out.timed_out)
        self.assertFalse(timed_out.ok)
        self.assertIn("超过", timed_out.error)


class CollectorTestCase(unittest.TestCase):
    def test_no_commands_and_no_proc_degrades_cleanly(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            collector = make_collector(tmp, FakeRunner())
            snapshot = collector.collect()

            self.assertFalse(snapshot["processes"]["available"])
            self.assertTrue(snapshot["processes"]["reason"])
            self.assertFalse(snapshot["channels"]["available"])
            self.assertIsNone(snapshot["channels"]["channels"])
            self.assertIsNone(snapshot["channels"]["count"])
            self.assertTrue(snapshot["modules"]["modules"])
            self.assertTrue(
                all(
                    module["status"] == "unknown"
                    for module in snapshot["modules"]["modules"]
                )
            )
            self.assertEqual(snapshot["summary"]["status"], "unavailable")
            json.dumps(snapshot, ensure_ascii=False)  # 必须可序列化

    def test_empty_proc_is_zero_not_unavailable(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "proc"
            proc.mkdir()
            collector = make_collector(
                tmp, FakeRunner(), proc_root=proc, root=root
            )
            snapshot = collector.collect()

            self.assertTrue(snapshot["processes"]["available"])
            self.assertEqual(snapshot["processes"]["count"], 0)
            self.assertEqual(snapshot["modules"]["counts"]["stopped"], 1)
            self.assertEqual(snapshot["modules"]["modules"][0]["status"], "stopped")
            self.assertEqual(snapshot["libraries"]["count"], 0)
            self.assertEqual(snapshot["summary"]["status"], "stopped")
            self.assertIsNone(snapshot["channels"]["count"])

    def test_fake_proc_maps_and_cyber_success(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "proc"
            write_fake_proc(proc, root)
            runner = FakeRunner(
                tools={
                    "cyber_channel": "/fake/cyber_channel",
                    "cyber_node": "/fake/cyber_node",
                },
                handler=cyber_handler,
            )
            collector = make_collector(
                tmp, runner, proc_root=proc, root=root
            )
            snapshot = collector.collect()

            self.assertTrue(snapshot["processes"]["available"])
            self.assertEqual(snapshot["processes"]["count"], 1)
            process = snapshot["processes"]["processes"][0]
            self.assertTrue(process["is_mainboard"])
            self.assertEqual(process["dags"], ["/apollo/modules/canbus/dag/canbus.dag"])

            module = snapshot["modules"]["modules"][0]
            self.assertEqual(module["status"], "running")
            self.assertEqual(module["library_check"], "confirmed")
            self.assertIn("libcanbus_component.so", [item["name"] for item in module["loaded_target_libraries"]])

            lib_process = snapshot["libraries"]["processes"][0]
            self.assertTrue(lib_process["available"])
            self.assertEqual(lib_process["targets"][0]["name"], "libcanbus_component.so")
            self.assertTrue(lib_process["targets"][0]["role"] == "target")
            self.assertTrue(
                any(item["name"] == "libc.so.6" for item in lib_process["dependencies"])
            )
            self.assertFalse(lib_process["missing_targets"])

            self.assertTrue(snapshot["channels"]["available"])
            self.assertEqual(snapshot["channels"]["count"], 2)
            self.assertTrue(snapshot["nodes"]["available"])
            self.assertEqual(snapshot["nodes"]["nodes"], ["canbus"])
            self.assertEqual(snapshot["summary"]["status"], "ok")
            self.assertEqual(
                runner.calls,
                [
                    ["/fake/cyber_channel", "list"],
                    ["/fake/cyber_node", "list"],
                ],
            )

    def test_missing_maps_marks_module_degraded_not_running(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "proc"
            write_fake_proc(proc, root, with_maps=False)
            collector = make_collector(
                tmp, FakeRunner(), proc_root=proc, root=root
            )
            snapshot = collector.collect()

            module = snapshot["modules"]["modules"][0]
            self.assertEqual(module["status"], "degraded")
            self.assertEqual(module["library_check"], "unavailable")
            self.assertIn("maps", module["reason"])
            self.assertFalse(snapshot["libraries"]["processes"][0]["available"])
            self.assertEqual(snapshot["summary"]["status"], "degraded")

    def test_ps_fallback_when_proc_missing(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "no_such_proc"

            def handler(call: Sequence[str]) -> CommandResult:
                if Path(call[0]).name == "ps":
                    return CommandResult(
                        argv=tuple(call),
                        returncode=0,
                        stdout=(
                            " 123 1 S mainboard -d "
                            "/apollo/modules/canbus/dag/canbus.dag -p canbus\n"
                        ),
                    )
                return CommandResult(argv=tuple(call), error="未预期")

            runner = FakeRunner(tools={"ps": "/fake/ps"}, handler=handler)
            collector = make_collector(
                tmp, runner, proc_root=proc, root=root
            )
            snapshot = collector.collect()

            self.assertTrue(snapshot["processes"]["available"])
            self.assertEqual(snapshot["processes"]["source"], "ps")
            self.assertEqual(snapshot["processes"]["count"], 1)
            self.assertEqual(snapshot["modules"]["modules"][0]["status"], "degraded")

    def test_broken_config_is_unavailable_without_crashing(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            config = tmp / "modules.json"
            config.write_text("{not valid json", encoding="utf-8")
            collector = make_collector(tmp, FakeRunner(), config=config)
            snapshot = collector.collect()

            self.assertFalse(snapshot["modules"]["available"])
            self.assertEqual(snapshot["modules"]["status"], "unavailable")
            self.assertFalse(snapshot["config"]["available"])
            self.assertIn("配置", snapshot["config"]["reason"])
            self.assertEqual(snapshot["summary"]["status"], "unavailable")
            json.dumps(snapshot, ensure_ascii=False)

    def test_bad_cyber_output_is_unavailable_not_zero(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "proc"
            proc.mkdir()

            def handler(call: Sequence[str]) -> CommandResult:
                if Path(call[0]).name == "cyber_channel":
                    return CommandResult(argv=tuple(call), returncode=0, stdout="garbage\n")
                return CommandResult(
                    argv=tuple(call),
                    returncode=0,
                    stdout="Number of active nodes: 0\n",
                )

            runner = FakeRunner(
                tools={
                    "cyber_channel": "/fake/cyber_channel",
                    "cyber_node": "/fake/cyber_node",
                },
                handler=handler,
            )
            collector = make_collector(tmp, runner, proc_root=proc, root=root)
            snapshot = collector.collect()

            self.assertFalse(snapshot["channels"]["available"])
            self.assertIsNone(snapshot["channels"]["channels"])
            self.assertIsNone(snapshot["channels"]["count"])
            self.assertIn("无法识别", snapshot["channels"]["reason"])
            self.assertTrue(snapshot["nodes"]["available"])
            self.assertEqual(snapshot["nodes"]["count"], 0)

    def test_real_config_is_valid(self) -> None:
        errors = validate_config(REAL_CONFIG)
        self.assertEqual(errors, [])
        collector = Collector(config_path=REAL_CONFIG)
        self.assertGreaterEqual(len(collector.modules), 10)

    def test_http_endpoints_return_json_and_static_page(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            root = make_apollo_root(tmp)
            proc = tmp / "proc"
            write_fake_proc(proc, root)
            runner = FakeRunner(
                tools={
                    "cyber_channel": "/fake/cyber_channel",
                    "cyber_node": "/fake/cyber_node",
                },
                handler=cyber_handler,
            )
            collector = make_collector(tmp, runner, proc_root=proc, root=root)
            server = monitor_server.MonitorHTTPServer(
                ("127.0.0.1", 0),
                collector=collector,
                web_dir=monitor_server.WEB_DIR,
                cache_ttl=0,
            )
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            port = server.server_address[1]
            try:
                connection = http.client.HTTPConnection("127.0.0.1", port, timeout=5)
                connection.request("GET", "/")
                response = connection.getresponse()
                body = response.read().decode("utf-8")
                self.assertEqual(response.status, 200)
                self.assertIn("Apollo 实车运行状态", body)

                for path in (
                    "/api/summary",
                    "/api/modules",
                    "/api/channels",
                    "/api/processes",
                    "/api/libraries",
                    "/api/config",
                    "/api/nodes",
                ):
                    connection.request("GET", path)
                    response = connection.getresponse()
                    raw = response.read().decode("utf-8")
                    self.assertEqual(response.status, 200, path)
                    payload = json.loads(raw)
                    self.assertIsInstance(payload, dict, path)
                    self.assertIn("status", payload, path)
                    self.assertIn("available", payload, path)

                connection.request("POST", "/api/summary")
                response = connection.getresponse()
                response.read()
                self.assertEqual(response.status, 405)
                connection.close()
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=3)


if __name__ == "__main__":
    unittest.main(verbosity=2)
