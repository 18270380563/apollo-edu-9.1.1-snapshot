"""Apollo 只读运行状态采集器。

设计原则：
1. 只读：不启动、停止、重启任何 Apollo 模块，不发送控制指令。
2. 分层：环境、进程、动态库、Cyber、模块各层单独容错；某一层失败不会让整份快照失败。
3. 可替换：所有外部命令都经过 CommandRunner，测试里可注入假 runner 和假 /proc。
4. 不猜测：命令不可用、文件不可读、输出格式不认识时，返回 available=false 或
   status="unavailable"/"unknown"，不会用 0 冒充真实数据。

本模块只依赖 Python 3 标准库。
"""

from __future__ import annotations

import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
import traceback
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Any, Callable, Iterable, Mapping, Sequence

BASE_DIR = Path(__file__).resolve().parent
DEFAULT_CONFIG_PATH = BASE_DIR / "config" / "modules.json"

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
DAG_LIBRARY_RE = re.compile(r'module_library\s*:\s*"([^"]+)"')
CHANNEL_COUNT_RE = re.compile(r"The number of channels is:\s*(\d+)")
NODE_COUNT_RE = re.compile(r"Number of active nodes:\s*(\d+)")

# 只暴露和排障有关的环境变量，避免把账号、密钥等无关内容带到网页上。
APOLLO_ENV_KEYS = (
    "APOLLO_ROOT_DIR",
    "APOLLO_IN_DOCKER",
    "APOLLO_LIB_PATH",
    "APOLLO_DAG_PATH",
    "APOLLO_CONF_PATH",
    "APOLLO_LAUNCH_PATH",
    "APOLLO_BIN_PREFIX",
    "APOLLO_ENV_WORKROOT",
    "APOLLO_CONFIG_HOME",
    "CYBER_PATH",
    "CYBER_DOMAIN_ID",
    "CYBER_IP",
    "GLOG_log_dir",
)


def now_iso() -> str:
    """带本地时区偏移的 ISO 时间，便于实车现场直接看。"""
    return datetime.now().astimezone().isoformat(timespec="seconds")


def _truncate(text: str, limit: int = 600) -> str:
    text = (text or "").strip()
    return text if len(text) <= limit else text[:limit] + "…"


def _unique(items: Iterable[str]) -> list[str]:
    seen: set[str] = set()
    result: list[str] = []
    for item in items:
        if item and item not in seen:
            seen.add(item)
            result.append(item)
    return result


def _same_path(left: str, right: str) -> bool:
    """比较配置里的相对路径和 /proc 里的绝对路径，不做文件系统访问。"""
    a = (left or "").rstrip("/")
    b = (right or "").rstrip("/")
    if not a or not b:
        return False
    return a == b or a.endswith("/" + b) or b.endswith("/" + a)


@dataclass(frozen=True)
class CommandResult:
    """一次外部命令的结果；用数据类而不是抛异常，调用方必须显式处理失败。"""

    argv: tuple[str, ...] = ()
    returncode: int | None = None
    stdout: str = ""
    stderr: str = ""
    error: str = ""
    timed_out: bool = False

    @property
    def ok(self) -> bool:
        return (
            self.returncode == 0
            and not self.timed_out
            and not self.error
        )


class CommandRunner:
    """外部命令的唯一入口。默认有超时、无 shell、捕获所有常见异常。"""

    def which(self, name: str) -> str | None:
        return shutil.which(name)

    def run(
        self,
        argv: Sequence[str],
        timeout: float = 3.0,
        env: Mapping[str, str] | None = None,
    ) -> CommandResult:
        argv_tuple = tuple(str(item) for item in argv)
        if not argv_tuple:
            return CommandResult(argv=argv_tuple, error="空命令")

        child_env = os.environ.copy()
        if env:
            child_env.update({str(key): str(value) for key, value in env.items()})

        try:
            completed = subprocess.run(
                list(argv_tuple),
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                errors="replace",
                timeout=max(0.1, float(timeout)),
                env=child_env,
                check=False,
            )
        except FileNotFoundError as exc:
            return CommandResult(argv=argv_tuple, error=f"命令不存在：{exc}")
        except subprocess.TimeoutExpired as exc:
            stdout = exc.stdout or ""
            stderr = exc.stderr or ""
            if isinstance(stdout, bytes):
                stdout = stdout.decode("utf-8", "replace")
            if isinstance(stderr, bytes):
                stderr = stderr.decode("utf-8", "replace")
            return CommandResult(
                argv=argv_tuple,
                stdout=stdout,
                stderr=stderr,
                error=f"命令超过 {timeout:g} 秒未返回",
                timed_out=True,
            )
        except OSError as exc:
            return CommandResult(argv=argv_tuple, error=f"命令执行失败：{exc}")

        return CommandResult(
            argv=argv_tuple,
            returncode=completed.returncode,
            stdout=completed.stdout or "",
            stderr=completed.stderr or "",
        )


@dataclass
class ProcInfo:
    pid: int
    ppid: int | None
    state: str
    name: str
    command: str
    argv: list[str]
    source: str
    uid: int | None = None
    rss_kb: int | None = None

    @property
    def is_mainboard(self) -> bool:
        if not self.argv:
            return False
        first = Path(self.argv[0]).name
        return first == "mainboard" or self.argv[0].endswith("/cyber/mainboard/mainboard")

    @property
    def is_cyber_tool(self) -> bool:
        if not self.argv:
            return False
        first = Path(self.argv[0]).name
        return first in {
            "cyber_channel",
            "cyber_node",
            "cyber_monitor",
            "cyber_launch",
            "cyber_recorder",
        }


class Collector:
    """把 Apollo 只读状态整理成可直接 JSON 序列化的快照。"""

    def __init__(
        self,
        config_path: str | os.PathLike[str] | None = None,
        apollo_root: str | os.PathLike[str] | None = None,
        proc_root: str | os.PathLike[str] = "/proc",
        runner: CommandRunner | None = None,
        cyber_tool_dir: str | os.PathLike[str] | None = None,
        cyber_timeout: float = 3.0,
    ) -> None:
        self.config_path = Path(config_path or DEFAULT_CONFIG_PATH).expanduser().resolve()
        self.explicit_apollo_root = (
            Path(apollo_root).expanduser().resolve() if apollo_root else None
        )
        self.proc_root = Path(proc_root)
        self.runner = runner or CommandRunner()
        self.cyber_tool_dir = (
            Path(cyber_tool_dir).expanduser().resolve() if cyber_tool_dir else None
        )
        self.cyber_timeout = float(cyber_timeout)

        self.config: dict[str, Any] = {}
        self.modules: list[dict[str, Any]] = []
        self.config_error = ""
        self._apollo_root_cache: Path | None | bool = False
        self._dag_cache: dict[str, dict[str, Any]] = {}

        self._load_config()

    # ------------------------------------------------------------------
    # 配置与路径
    # ------------------------------------------------------------------
    def _load_config(self) -> None:
        try:
            with self.config_path.open("r", encoding="utf-8") as handle:
                config = json.load(handle)
            if not isinstance(config, dict):
                raise ValueError("配置根节点必须是对象")
            modules = config.get("modules")
            if not isinstance(modules, list) or not modules:
                raise ValueError("modules 必须是非空数组")

            seen: set[str] = set()
            normalized: list[dict[str, Any]] = []
            for index, raw in enumerate(modules):
                if not isinstance(raw, dict):
                    raise ValueError(f"第 {index + 1} 个模块不是对象")
                module_id = str(raw.get("id", "")).strip()
                if not module_id:
                    raise ValueError(f"第 {index + 1} 个模块缺少 id")
                if module_id in seen:
                    raise ValueError(f"模块 id 重复：{module_id}")
                seen.add(module_id)

                keywords = raw.get("process_keywords") or []
                keywords_any = raw.get("process_keywords_any") or []
                if not isinstance(keywords, list) or not isinstance(keywords_any, list):
                    raise ValueError(f"{module_id}: process_keywords 必须是数组")
                if not keywords and not keywords_any:
                    raise ValueError(f"{module_id}: 至少需要一个进程匹配关键词")

                dags = raw.get("dags") or []
                channels = raw.get("suggested_channels") or []
                if not isinstance(dags, list) or not isinstance(channels, list):
                    raise ValueError(f"{module_id}: dags/suggested_channels 必须是数组")
                if raw.get("binary") and dags:
                    raise ValueError(f"{module_id}: 二进制模块不应配置 dags")

                item = dict(raw)
                item["id"] = module_id
                item["name"] = str(raw.get("name") or module_id)
                item["description"] = str(raw.get("description") or "")
                item["process_keywords"] = [str(v) for v in keywords if str(v)]
                item["process_keywords_any"] = [
                    str(v) for v in keywords_any if str(v)
                ]
                item["dags"] = [str(v) for v in dags if str(v)]
                item["suggested_channels"] = [str(v) for v in channels if str(v)]
                item["binary"] = bool(raw.get("binary", False))
                item["library_check"] = bool(raw.get("library_check", True))
                normalized.append(item)

            self.config = config
            self.modules = normalized
        except Exception as exc:  # 配置坏了也要让服务能启动并说明原因
            self.config = {}
            self.modules = []
            self.config_error = f"读取模块配置失败：{exc}"

    def _find_apollo_root(self) -> Path | None:
        if self._apollo_root_cache is not False:
            return self._apollo_root_cache if isinstance(self._apollo_root_cache, Path) else None

        apollo_config = self.config.get("apollo", {}) if isinstance(self.config, dict) else {}
        markers = apollo_config.get("root_markers") or ["cyber", "modules"]
        markers = [str(item) for item in markers if str(item)]

        candidates: list[Path] = []
        if self.explicit_apollo_root is not None:
            candidates.append(self.explicit_apollo_root)
        env_root = os.environ.get("APOLLO_ROOT_DIR", "").strip()
        if env_root:
            candidates.append(Path(env_root))
        for candidate in apollo_config.get("root_candidates") or []:
            candidates.append(Path(str(candidate)))
        candidates.append(Path("/apollo"))

        for candidate in candidates:
            try:
                candidate = candidate.expanduser().resolve()
            except OSError:
                continue
            if all((candidate / marker).exists() for marker in markers):
                self._apollo_root_cache = candidate
                return candidate

        self._apollo_root_cache = None
        return None

    @property
    def apollo_root(self) -> Path | None:
        return self._find_apollo_root()

    def _resolve_config_path(self, value: str) -> Path:
        """配置路径相对 Apollo 根目录；/apollo 前缀在挂载根不同的时候做一次重映射。"""
        raw = str(value or "").strip()
        if not raw:
            return Path("")
        path = Path(raw)
        root = self.apollo_root
        if path.is_absolute():
            if (
                root is not None
                and str(root) != "/apollo"
                and raw.startswith("/apollo/")
                and not path.exists()
            ):
                remapped = root / raw[len("/apollo/") :]
                if remapped.exists():
                    return remapped
            return path
        if root is not None:
            return root / raw
        return path

    def _remap_apollo_path(self, value: str) -> Path:
        raw = str(value or "").strip()
        path = Path(raw)
        root = self.apollo_root
        if (
            path.is_absolute()
            and root is not None
            and str(root) != "/apollo"
            and raw.startswith("/apollo/")
            and not path.exists()
        ):
            return root / raw[len("/apollo/") :]
        return path

    def _library_candidates(self, library: str) -> list[Path]:
        raw = str(library or "").strip()
        if not raw:
            return []
        path = Path(raw)
        if path.is_absolute():
            candidates = [path]
            remapped = self._remap_apollo_path(raw)
            if remapped != path:
                candidates.append(remapped)
            return candidates

        candidates: list[Path] = []
        env_lib = os.environ.get("APOLLO_LIB_PATH", "").strip()
        for directory in env_lib.split(":"):
            if directory:
                candidates.append(Path(directory) / raw)
        root = self.apollo_root
        if root is not None:
            for subdir in (self.config.get("apollo", {}) or {}).get(
                "library_search_dirs", ["bazel-bin", ""]
            ):
                subdir = str(subdir or "")
                candidates.append(root / subdir / raw if subdir else root / raw)
        return _unique_paths(candidates)

    def _resolve_library(self, library: str) -> Path:
        candidates = self._library_candidates(library)
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return candidates[0] if candidates else Path(library)

    # ------------------------------------------------------------------
    # DAG
    # ------------------------------------------------------------------
    def _read_dag(self, relative_path: str) -> dict[str, Any]:
        path = self._resolve_config_path(relative_path)
        cache_key = str(path)
        try:
            mtime = path.stat().st_mtime
        except OSError:
            mtime = 0

        cached = self._dag_cache.get(cache_key)
        if cached and cached.get("_mtime") == mtime:
            return cached

        info: dict[str, Any] = {
            "declared_path": relative_path,
            "resolved_path": str(path),
            "exists": False,
            "libraries": [],
            "error": "",
            "_mtime": mtime,
        }
        if not path.is_file():
            info["error"] = "DAG 文件不存在"
            self._dag_cache[cache_key] = info
            return info

        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            info["error"] = f"DAG 不可读：{exc}"
            self._dag_cache[cache_key] = info
            return info

        # 去掉行注释，避免把注释里的 module_library 当成真实配置。
        cleaned = "\n".join(line.split("#", 1)[0] for line in text.splitlines())
        libraries = _unique(DAG_LIBRARY_RE.findall(cleaned))
        info["exists"] = True
        info["libraries"] = libraries
        self._dag_cache[cache_key] = info
        return info

    def _active_dags(self, spec: Mapping[str, Any], process_dags: Sequence[str]) -> list[str]:
        active: list[str] = []
        for dag in spec.get("dags", []):
            if any(_same_path(str(dag), str(proc_dag)) for proc_dag in process_dags):
                active.append(str(dag))
        return active

    def _expected_targets_for_view(self, view: Mapping[str, Any]) -> list[dict[str, Any]]:
        targets: list[dict[str, Any]] = []
        seen: set[str] = set()
        module_ids = set(view.get("modules", []))
        for spec in self.modules:
            if spec["id"] not in module_ids or not spec.get("library_check", True):
                continue
            for dag in self._active_dags(spec, view.get("dags", [])):
                info = self._read_dag(dag)
                for library in info.get("libraries", []):
                    resolved = self._resolve_library(library)
                    name = Path(library).name
                    if name in seen:
                        continue
                    seen.add(name)
                    targets.append(
                        {
                            "name": name,
                            "declared": library,
                            "resolved_path": str(resolved),
                            "exists_on_disk": resolved.is_file(),
                            "dag": dag,
                        }
                    )
        return targets

    # ------------------------------------------------------------------
    # 环境
    # ------------------------------------------------------------------
    def collect_environment(self) -> dict[str, Any]:
        root = self.apollo_root
        env_values = {
            key: os.environ[key]
            for key in APOLLO_ENV_KEYS
            if key in os.environ and os.environ[key] != ""
        }
        in_container = Path("/.dockerenv").exists() or str(
            os.environ.get("APOLLO_IN_DOCKER", "")
        ).lower() in {"1", "true", "yes"}

        def path_info(relative: str) -> dict[str, Any]:
            path = self._resolve_config_path(relative)
            return {"path": str(path), "exists": path.exists()}

        paths = {
            "mainboard": path_info("bazel-bin/cyber/mainboard/mainboard"),
            "cyber_setup": path_info("cyber/setup.bash"),
            "cyber_tools": path_info("bazel-bin/cyber/tools"),
            "bazel_bin": path_info("bazel-bin"),
            "log_dir": path_info("data/log"),
        }

        if root is None:
            reason = (
                "未找到同时包含 cyber/ 和 modules/ 的 Apollo 根目录。"
                "可能不在 Cyber 容器内，或需要用 --apollo-root 指定源码/安装根目录。"
            )
            return {
                "timestamp": now_iso(),
                "source": "env+fs",
                "status": "unavailable",
                "reason": reason,
                "available": False,
                "apollo_root": None,
                "in_container": in_container,
                "docker_marker": Path("/.dockerenv").exists(),
                "config_home": os.environ.get("APOLLO_CONFIG_HOME", ""),
                "env": env_values,
                "paths": paths,
            }

        return {
            "timestamp": now_iso(),
            "source": "env+fs",
            "status": "ok",
            "reason": "",
            "available": True,
            "apollo_root": str(root),
            "in_container": in_container,
            "docker_marker": Path("/.dockerenv").exists(),
            "config_home": os.environ.get("APOLLO_CONFIG_HOME", ""),
            "env": env_values,
            "paths": paths,
        }

    # ------------------------------------------------------------------
    # 进程
    # ------------------------------------------------------------------
    def _read_status(self, pid_dir: Path) -> dict[str, Any]:
        result: dict[str, Any] = {"name": "", "state": "", "ppid": None, "uid": None, "rss_kb": None}
        try:
            text = (pid_dir / "status").read_text(encoding="utf-8", errors="replace")
        except OSError:
            return result
        for line in text.splitlines():
            if line.startswith("Name:"):
                result["name"] = line.split(":", 1)[1].strip()
            elif line.startswith("State:"):
                result["state"] = line.split(":", 1)[1].strip()
            elif line.startswith("PPid:"):
                try:
                    result["ppid"] = int(line.split(":", 1)[1].strip())
                except ValueError:
                    pass
            elif line.startswith("Uid:"):
                parts = line.split(":", 1)[1].split()
                if parts:
                    try:
                        result["uid"] = int(parts[0])
                    except ValueError:
                        pass
            elif line.startswith("VmRSS:"):
                parts = line.split(":", 1)[1].split()
                if parts:
                    try:
                        result["rss_kb"] = int(parts[0])
                    except ValueError:
                        pass
        return result

    def _read_proc_processes(
        self,
    ) -> tuple[list[ProcInfo], bool, str, list[str]]:
        errors: list[str] = []
        if not self.proc_root.is_dir():
            return [], False, f"/proc 不可用：{self.proc_root} 不是目录", errors

        pid_dirs = sorted(
            (item for item in self.proc_root.iterdir() if item.name.isdigit()),
            key=lambda item: int(item.name),
        )
        processes: list[ProcInfo] = []
        unreadable = 0

        for pid_dir in pid_dirs:
            try:
                raw = (pid_dir / "cmdline").read_bytes()
            except (OSError, PermissionError) as exc:
                unreadable += 1
                if len(errors) < 8:
                    errors.append(f"PID {pid_dir.name}: 无法读取 cmdline（{exc.__class__.__name__}）")
                continue

            argv = [part.decode("utf-8", "replace") for part in raw.split(b"\0") if part]
            if not argv:
                # 内核线程没有 cmdline，不是错误。
                continue
            command = " ".join(argv)
            status = self._read_status(pid_dir)
            processes.append(
                ProcInfo(
                    pid=int(pid_dir.name),
                    ppid=status["ppid"],
                    state=status["state"] or "?",
                    name=status["name"] or Path(argv[0]).name,
                    command=command,
                    argv=argv,
                    source="procfs",
                    uid=status["uid"],
                    rss_kb=status["rss_kb"],
                )
            )

        if pid_dirs and not processes and unreadable:
            return [], False, "所有 /proc/<pid>/cmdline 都无法读取，可能权限不足", errors
        reason = ""
        if unreadable:
            reason = f"有 {unreadable} 个进程因权限或竞态无法读取，已跳过；其余数据仍可用。"
        return processes, True, reason, errors

    def _read_ps_processes(self) -> tuple[list[ProcInfo], bool, str, list[str]]:
        ps_path = self.runner.which("ps")
        if not ps_path:
            return [], False, "未找到 ps 命令，且 /proc 不可用", []
        result = self.runner.run(
            [ps_path, "-eo", "pid=,ppid=,stat=,args="],
            timeout=5.0,
        )
        if not result.ok:
            reason = result.error or _truncate(result.stderr) or f"ps 退出码 {result.returncode}"
            return [], False, f"ps 采集失败：{reason}", []
        processes: list[ProcInfo] = []
        for line in result.stdout.splitlines():
            match = re.match(r"^\s*(\d+)\s+(\d+)\s+(\S+)\s+(.*)$", line)
            if not match:
                continue
            pid, ppid, state, command = match.groups()
            command = command.strip()
            if not command:
                continue
            try:
                argv = shlex.split(command)
            except ValueError:
                argv = command.split()
            processes.append(
                ProcInfo(
                    pid=int(pid),
                    ppid=int(ppid),
                    state=state,
                    name=Path(argv[0]).name if argv else Path(command.split()[0]).name,
                    command=command,
                    argv=argv,
                    source="ps",
                )
            )
        return processes, True, "已从 ps 读取进程；可用性低于 /proc。", []

    @staticmethod
    def _parse_dags(argv: Sequence[str], command: str) -> list[str]:
        dags: list[str] = []
        if argv:
            index = 0
            while index < len(argv):
                token = argv[index]
                if token in {"-d", "--dag_conf"}:
                    index += 1
                    while index < len(argv) and not argv[index].startswith("-"):
                        dags.append(argv[index])
                        index += 1
                    continue
                if token.startswith("--dag_conf="):
                    dags.append(token.split("=", 1)[1])
                index += 1
        else:
            for pattern in (r"(?:^|\s)-d\s+(\S+)", r"--dag_conf(?:=|\s+)(\S+)"):
                dags.extend(re.findall(pattern, command))
        return _unique(str(dag) for dag in dags if str(dag).endswith(".dag"))

    def _match_spec(self, spec: Mapping[str, Any], command: str) -> bool:
        for keyword in spec.get("process_keywords", []):
            if str(keyword) not in command:
                return False
        any_keywords = spec.get("process_keywords_any", [])
        if any_keywords and not any(str(keyword) in command for keyword in any_keywords):
            return False
        return True

    def collect_processes(self) -> dict[str, Any]:
        processes, available, reason, errors = self._read_proc_processes()
        source = "procfs"
        if not available:
            processes, available, reason, errors = self._read_ps_processes()
            source = "ps"

        if not processes and available and source == "procfs" and not reason:
            # /proc 可读但没有进程：这是真实结果，不是不可用。
            reason = "已读取 /proc，但没有发现带命令行的进程。"
        if not available:
            return {
                "timestamp": now_iso(),
                "source": "procfs+ps",
                "status": "unavailable",
                "reason": reason or "进程数据不可用",
                "available": False,
                "processes": [],
                "count": None,
                "scanned": None,
                "errors": errors,
            }

        views: list[dict[str, Any]] = []
        for process in processes:
            matched_modules = [
                spec["id"]
                for spec in self.modules
                if self._match_spec(spec, process.command)
                and (process.is_mainboard or spec.get("binary", False))
            ]
            if not matched_modules and not process.is_mainboard and not process.is_cyber_tool:
                continue
            dags = self._parse_dags(process.argv, process.command)
            views.append(
                {
                    "pid": process.pid,
                    "ppid": process.ppid,
                    "name": process.name,
                    "state": process.state,
                    "uid": process.uid,
                    "rss_kb": process.rss_kb,
                    "command": process.command,
                    "source": process.source,
                    "is_mainboard": process.is_mainboard,
                    "is_cyber_tool": process.is_cyber_tool,
                    "modules": matched_modules,
                    "dags": dags,
                }
            )

        status = "ok"
        if errors:
            status = "degraded"
        if not views and not errors:
            status = "ok"
        return {
            "timestamp": now_iso(),
            "source": source,
            "status": status,
            "reason": reason or "已读取进程数据。",
            "available": True,
            "processes": views,
            "count": len(views),
            "scanned": len(processes),
            "errors": errors,
        }

    # ------------------------------------------------------------------
    # 动态库
    # ------------------------------------------------------------------
    @staticmethod
    def _read_maps(maps_path: Path) -> tuple[list[dict[str, Any]], str]:
        try:
            text = maps_path.read_text(encoding="utf-8", errors="replace")
        except PermissionError as exc:
            return [], f"/proc/<pid>/maps 权限不足：{exc}"
        except FileNotFoundError:
            return [], "/proc/<pid>/maps 不存在（进程可能已退出）"
        except OSError as exc:
            return [], f"/proc/<pid>/maps 读取失败：{exc}"

        libraries: list[dict[str, Any]] = []
        seen: set[str] = set()
        for line in text.splitlines():
            parts = line.split(None, 5)
            if len(parts) < 6:
                continue
            path = parts[5].strip()
            if not path or path.startswith("["):
                continue
            deleted = path.endswith(" (deleted)")
            if deleted:
                path = path[: -len(" (deleted)")]
            name = Path(path).name
            if ".so" not in name:
                continue
            if path in seen:
                continue
            seen.add(path)
            libraries.append({"name": name, "path": path, "deleted": deleted})
        return libraries, ""

    def collect_libraries(self, processes_payload: Mapping[str, Any]) -> dict[str, Any]:
        # 刻意不用 ldd：ldd 只解析磁盘上的依赖关系，不能证明某个 .so 已进入
        # 运行中进程的地址空间；/proc/<pid>/maps 才是实际映射的证据。
        if not processes_payload.get("available"):
            return {
                "timestamp": now_iso(),
                "source": "procfs:/proc/<pid>/maps",
                "status": "unavailable",
                "reason": f"进程数据不可用，无法检查动态库：{processes_payload.get('reason', '')}",
                "available": False,
                "processes": [],
                "count": None,
                "readable_count": None,
                "reason_summary": "",
            }

        candidates = [
            view
            for view in processes_payload.get("processes", [])
            if view.get("is_mainboard") or view.get("modules")
        ]
        if not candidates:
            return {
                "timestamp": now_iso(),
                "source": "procfs:/proc/<pid>/maps",
                "status": "ok",
                "reason": "没有需要检查动态库的 Apollo 进程。",
                "available": True,
                "processes": [],
                "count": 0,
                "readable_count": 0,
                "reason_summary": "",
            }

        process_reports: list[dict[str, Any]] = []
        reasons: list[str] = []
        readable_count = 0
        root = self.apollo_root

        for view in candidates:
            targets = self._expected_targets_for_view(view)
            expected_by_name = {target["name"]: target for target in targets}
            expected_paths = {str(Path(target["resolved_path"]).resolve()) for target in targets}
            maps_path = self.proc_root / str(view["pid"]) / "maps"
            libraries, maps_error = self._read_maps(maps_path)

            targets_found: list[dict[str, Any]] = []
            dependencies: list[dict[str, Any]] = []
            for library in libraries:
                path = library["path"]
                resolved = str(Path(path).resolve())
                is_target = library["name"] in expected_by_name or resolved in expected_paths
                if is_target:
                    roles = [
                        target["dag"]
                        for target in targets
                        if target["name"] == library["name"]
                        or str(Path(target["resolved_path"]).resolve()) == resolved
                    ]
                    targets_found.append(
                        {
                            **library,
                            "role": "target",
                            "target_for_dags": _unique(roles),
                        }
                    )
                else:
                    if root is not None and path.startswith(str(root) + os.sep):
                        scope = "apollo"
                    elif path.startswith(("/opt/apollo", "/usr/", "/lib/", "/usr/lib")):
                        scope = "system"
                    else:
                        scope = "other"
                    dependencies.append({**library, "role": "dependency", "scope": scope})

            found_names = {item["name"] for item in targets_found}
            missing_targets = [
                target for target in targets if target["name"] not in found_names
            ]
            available = not maps_error
            if available:
                readable_count += 1
            else:
                reasons.append(f"PID {view['pid']}：{maps_error}")

            process_reports.append(
                {
                    "pid": view["pid"],
                    "name": view["name"],
                    "command": view["command"],
                    "modules": view.get("modules", []),
                    "is_mainboard": bool(view.get("is_mainboard")),
                    "source": "procfs:/proc/<pid>/maps",
                    "status": "ok" if available else "unavailable",
                    "reason": maps_error or "已从 maps 解析真实映射的动态库。",
                    "available": available,
                    "targets": targets_found,
                    "dependencies": dependencies,
                    "counts": {
                        "target": len(targets_found),
                        "dependency": len(dependencies),
                        "total": len(targets_found) + len(dependencies),
                    },
                    "missing_targets": missing_targets,
                }
            )

        if readable_count == 0:
            status = "unavailable"
        elif reasons:
            status = "degraded"
        else:
            status = "ok"
        return {
            "timestamp": now_iso(),
            "source": "procfs:/proc/<pid>/maps",
            "status": status,
            "reason": "；".join(reasons)
            if reasons
            else "已从 /proc/<pid>/maps 解析真实映射的 .so。",
            "available": readable_count > 0,
            "processes": process_reports,
            "count": len(process_reports),
            "readable_count": readable_count,
            "reason_summary": "；".join(reasons),
        }

    # ------------------------------------------------------------------
    # Cyber 节点 / 通道
    # ------------------------------------------------------------------
    def _resolve_cyber_tool(self, name: str) -> str | None:
        which = self.runner.which(name)
        if which:
            return which

        base_candidates: list[Path] = []
        if self.cyber_tool_dir is not None:
            base_candidates.append(self.cyber_tool_dir)
        env_dir = os.environ.get("MONITOR_CYBER_BIN_DIR", "").strip()
        if env_dir:
            base_candidates.append(Path(env_dir))
        root = self.apollo_root
        if root is not None:
            subdir = (self.config.get("apollo", {}) or {}).get(
                "cyber_tool_subdir", "bazel-bin/cyber/tools"
            )
            base_candidates.append(root / str(subdir))
        base_candidates.append(Path("/apollo/bazel-bin/cyber/tools"))

        for base in base_candidates:
            for candidate in (base / name / name, base / name):
                if candidate.is_file() and os.access(candidate, os.X_OK):
                    return str(candidate)
        return None

    def _cyber_env(self) -> dict[str, str]:
        env: dict[str, str] = {}
        root = self.apollo_root
        if root is not None:
            env.setdefault("CYBER_PATH", str(root / "cyber"))
            env.setdefault("APOLLO_ENV_WORKROOT", str(root))
        # 尽量让 cyber 工具把日志写到 stderr（由本服务捕获），不去写 Apollo 日志目录。
        env.setdefault("GLOG_logtostderr", "1")
        return env

    def _run_cyber_list(self, tool_path: str | None, tool_name: str) -> tuple[CommandResult | None, str]:
        if not tool_path:
            return None, (
                f"未找到 {tool_name}。需要与 Apollo 使用同一 Cyber 环境（同一容器、"
                "同一 CYBER_DOMAIN_ID），或设置 MONITOR_CYBER_BIN_DIR。"
            )
        result = self.runner.run(
            [tool_path, "list"],
            timeout=self.cyber_timeout,
            env=self._cyber_env(),
        )
        if result.timed_out:
            return result, f"{tool_name} list 超时（{self.cyber_timeout:g} 秒）"
        if not result.ok:
            detail = result.error or _truncate(result.stderr) or f"退出码 {result.returncode}"
            return result, f"{tool_name} list 失败：{detail}"
        return result, ""

    @staticmethod
    def _clean_lines(text: str) -> list[str]:
        return [ANSI_RE.sub("", line).strip() for line in (text or "").splitlines()]

    def _parse_channel_list(self, result: CommandResult) -> tuple[list[str] | None, str]:
        lines = self._clean_lines(result.stdout)
        header_index = -1
        expected: int | None = None
        for index, line in enumerate(lines):
            match = CHANNEL_COUNT_RE.search(line)
            if match:
                header_index = index
                expected = int(match.group(1))
                break

        rest = lines[header_index + 1 :] if header_index >= 0 else lines
        channels = _unique(
            line for line in rest if line.startswith("/") and " " not in line
        )
        if header_index < 0 and not channels:
            return None, "cyber_channel list 输出无法识别（没有通道计数行，也没有 / 开头的通道名）"
        if expected is not None and expected != len(channels):
            return channels, f"cyber_channel 报告 {expected} 个通道，但解析到 {len(channels)} 个"
        return channels, ""

    def _parse_node_list(self, result: CommandResult) -> tuple[list[str] | None, str]:
        lines = self._clean_lines(result.stdout)
        header_index = -1
        expected: int | None = None
        for index, line in enumerate(lines):
            match = NODE_COUNT_RE.search(line)
            if match:
                header_index = index
                expected = int(match.group(1))
                break

        rest = lines[header_index + 1 :] if header_index >= 0 else []
        nodes = _unique(
            line
            for line in rest
            if line
            and not line.startswith("[")
            and " " not in line
            and not line.startswith(("INFO", "WARNING", "ERROR"))
        )
        if header_index < 0:
            return None, "cyber_node list 输出无法识别（没有节点计数行）"
        if expected is not None and expected != len(nodes):
            return nodes, f"cyber_node 报告 {expected} 个节点，但解析到 {len(nodes)} 个"
        return nodes, ""

    def collect_cyber(self) -> dict[str, Any]:
        channel_tool = self._resolve_cyber_tool("cyber_channel")
        node_tool = self._resolve_cyber_tool("cyber_node")

        channel_result, channel_error = self._run_cyber_list(channel_tool, "cyber_channel")
        channel_names: list[str] | None = None
        channel_reason = channel_error
        if channel_result is not None and not channel_error:
            parsed, parse_reason = self._parse_channel_list(channel_result)
            channel_names = parsed
            channel_reason = parse_reason
        channel_available = channel_names is not None
        if not channel_available and not channel_reason:
            channel_reason = "Cyber 通道信息不可用"

        node_result, node_error = self._run_cyber_list(node_tool, "cyber_node")
        node_names: list[str] | None = None
        node_reason = node_error
        if node_result is not None and not node_error:
            parsed, parse_reason = self._parse_node_list(node_result)
            node_names = parsed
            node_reason = parse_reason
        node_available = node_names is not None
        if not node_available and not node_reason:
            node_reason = "Cyber 节点信息不可用"

        channels_status = "ok" if channel_available and not channel_reason else (
            "degraded" if channel_available else "unavailable"
        )
        nodes_status = "ok" if node_available and not node_reason else (
            "degraded" if node_available else "unavailable"
        )
        channels = {
            "timestamp": now_iso(),
            "source": "cyber_cli",
            "status": channels_status,
            "reason": channel_reason,
            "available": channel_available,
            "command": channel_tool or "",
            "channels": [
                {"name": name, "suggested_for": []} for name in (channel_names or [])
            ]
            if channel_available
            else None,
            "count": len(channel_names) if channel_names is not None else None,
            "raw_error": _truncate(channel_result.stderr if channel_result else ""),
        }
        nodes = {
            "timestamp": now_iso(),
            "source": "cyber_cli",
            "status": nodes_status,
            "reason": node_reason,
            "available": node_available,
            "command": node_tool or "",
            "nodes": node_names,
            "count": len(node_names) if node_names is not None else None,
            "raw_error": _truncate(node_result.stderr if node_result else ""),
        }
        return {"channels": channels, "nodes": nodes}

    # ------------------------------------------------------------------
    # 模块与汇总
    # ------------------------------------------------------------------
    def _module_channel_rows(
        self, spec: Mapping[str, Any], channel_names: set[str] | None
    ) -> list[dict[str, Any]]:
        rows: list[dict[str, Any]] = []
        for name in spec.get("suggested_channels", []):
            if channel_names is None:
                present: bool | None = None
            else:
                present = name in channel_names
            rows.append({"name": name, "present": present})
        return rows

    def collect_modules(
        self,
        processes_payload: Mapping[str, Any],
        libraries_payload: Mapping[str, Any],
        cyber_payload: Mapping[str, Any],
    ) -> dict[str, Any]:
        proc_available = bool(processes_payload.get("available"))
        process_views = processes_payload.get("processes", []) if proc_available else []
        if not self.modules:
            return {
                "timestamp": now_iso(),
                "source": "config+procfs+maps+cyber_cli",
                "status": "unavailable",
                "reason": self.config_error or "没有可用的模块配置。",
                "available": False,
                "modules": [],
                "counts": {
                    "total": 0,
                    "running": 0,
                    "stopped": 0,
                    "degraded": 0,
                    "unknown": 0,
                },
            }
        environment_available = self.apollo_root is not None
        library_by_pid = {
            int(item["pid"]): item
            for item in libraries_payload.get("processes", [])
            if item.get("pid") is not None
        }
        channels_payload = cyber_payload.get("channels", {})
        channel_names: set[str] | None = None
        if channels_payload.get("available") and channels_payload.get("channels") is not None:
            channel_names = {item["name"] for item in channels_payload["channels"]}

        module_reports: list[dict[str, Any]] = []
        for spec in self.modules:
            views = [
                view for view in process_views if spec["id"] in view.get("modules", [])
            ]
            dag_rows: list[dict[str, Any]] = []
            for dag in spec.get("dags", []):
                info = self._read_dag(dag)
                active = any(
                    any(_same_path(dag, proc_dag) for proc_dag in view.get("dags", []))
                    for view in views
                )
                dag_rows.append(
                    {
                        "declared_path": dag,
                        "resolved_path": info["resolved_path"],
                        "exists": info["exists"],
                        "libraries": info["libraries"],
                        "error": info["error"],
                        "active": active,
                    }
                )

            channel_rows = self._module_channel_rows(spec, channel_names)
            base: dict[str, Any] = {
                "id": spec["id"],
                "name": spec["name"],
                "description": spec["description"],
                "suggested_channels": channel_rows,
                "dags": dag_rows,
                "launch": spec.get("launch", ""),
                "conf": spec.get("conf", []),
                "process_keywords": spec.get("process_keywords", []),
                "process_keywords_any": spec.get("process_keywords_any", []),
                "processes": [
                    {
                        "pid": view["pid"],
                        "name": view["name"],
                        "state": view["state"],
                        "command": view["command"],
                        "is_mainboard": view["is_mainboard"],
                        "dags": view["dags"],
                    }
                    for view in views
                ],
            }

            if not environment_available:
                base.update(
                    {
                        "status": "unknown",
                        "reason": (
                            "未检测到 Apollo 根目录（包含 cyber/ 和 modules/），"
                            "无法核对 DAG 和目标 .so；请确认监控是否在 Cyber 容器内。"
                        ),
                        "library_check": "unavailable",
                        "loaded_target_libraries": [],
                        "missing_target_libraries": [],
                    }
                )
                module_reports.append(base)
                continue

            if not proc_available:
                base.update(
                    {
                        "status": "unknown",
                        "reason": f"进程数据不可用，无法判断模块状态：{processes_payload.get('reason', '')}",
                        "library_check": "unavailable",
                        "loaded_target_libraries": [],
                        "missing_target_libraries": [],
                    }
                )
                module_reports.append(base)
                continue

            if not views:
                base.update(
                    {
                        "status": "stopped",
                        "reason": "未发现同时匹配配置关键词的进程。",
                        "library_check": "skipped",
                        "loaded_target_libraries": [],
                        "missing_target_libraries": [],
                    }
                )
                module_reports.append(base)
                continue

            if spec.get("binary") or not spec.get("library_check", True):
                base.update(
                    {
                        "status": "running",
                        "reason": "进程存在；二进制模块不做动态库校验。",
                        "library_check": "skipped",
                        "loaded_target_libraries": [],
                        "missing_target_libraries": [],
                    }
                )
                module_reports.append(base)
                continue

            expected_targets: dict[str, dict[str, Any]] = {}
            loaded_targets: dict[str, dict[str, Any]] = {}
            missing_targets: dict[str, dict[str, Any]] = {}
            libraries_unavailable = False
            libraries_checked = False

            for view in views:
                report = library_by_pid.get(int(view["pid"]))
                if not report:
                    continue
                for target in report.get("targets", []):
                    loaded_targets[target["name"]] = target
                    libraries_checked = True
                for target in report.get("missing_targets", []):
                    missing_targets[target["name"]] = target
                    expected_targets.setdefault(target["name"], target)
                    libraries_checked = True
                if not report.get("available"):
                    libraries_unavailable = True

            active_dags = [
                row
                for row in dag_rows
                if row["active"] or (not views and row["active"])
            ]
            dag_exists = [row for row in active_dags if row["exists"]]
            no_expected = not expected_targets and not loaded_targets

            if no_expected and not libraries_checked:
                if not dag_exists:
                    status = "degraded"
                    reason = (
                        "进程存在，但匹配的 DAG 文件不存在或为空，无法确定目标 .so；"
                        "这是配置/环境不完整，不代表模块本身故障。"
                    )
                    check = "unavailable"
                else:
                    status = "degraded"
                    reason = "进程存在，但 DAG 中没有可解析的 module_library，无法确认目标 .so。"
                    check = "unavailable"
            elif libraries_unavailable and not loaded_targets:
                status = "degraded"
                reason = "进程存在，但 /proc/<pid>/maps 不可读，无法确认目标 .so 已加载。"
                check = "unavailable"
            elif missing_targets and not loaded_targets:
                status = "degraded"
                reason = "进程存在，但未发现任何预期目标库：" + "、".join(sorted(missing_targets))
                check = "missing"
            elif missing_targets and loaded_targets:
                status = "degraded"
                reason = "只确认到部分目标库；未发现：" + "、".join(sorted(missing_targets))
                check = "partial"
            elif loaded_targets:
                status = "running"
                reason = "已在进程 maps 中确认目标库：" + "、".join(sorted(loaded_targets))
                check = "confirmed"
            else:
                status = "degraded"
                reason = "进程存在，但无法从 DAG 与 maps 得到可核验的目标库。"
                check = "unavailable"

            base.update(
                {
                    "status": status,
                    "reason": reason,
                    "library_check": check,
                    "loaded_target_libraries": [
                        {
                            "name": item["name"],
                            "path": item["path"],
                            "pid": next(
                                (
                                    int(view["pid"])
                                    for view in views
                                    if int(view["pid"]) in library_by_pid
                                    and any(
                                        target["name"] == item["name"]
                                        for target in library_by_pid[int(view["pid"])].get(
                                            "targets", []
                                        )
                                    )
                                ),
                                None,
                            ),
                        }
                        for item in sorted(loaded_targets.values(), key=lambda row: row["name"])
                    ],
                    "missing_target_libraries": sorted(
                        missing_targets.values(), key=lambda row: row["name"]
                    ),
                }
            )
            module_reports.append(base)

        counts = {
            "total": len(module_reports),
            "running": sum(1 for item in module_reports if item["status"] == "running"),
            "stopped": sum(1 for item in module_reports if item["status"] == "stopped"),
            "degraded": sum(1 for item in module_reports if item["status"] == "degraded"),
            "unknown": sum(1 for item in module_reports if item["status"] == "unknown"),
        }
        if not environment_available:
            status = "unavailable"
            reason = "Apollo 环境不可用，模块状态只能标为 unknown。"
        elif not proc_available:
            status = "unavailable"
            reason = "进程数据不可用，模块状态只能标为 unknown。"
        elif counts["degraded"] or counts["unknown"]:
            status = "degraded"
            reason = "存在降级或未知状态的模块。"
        elif counts["running"] == 0 and counts["stopped"] > 0:
            status = "stopped"
            reason = "配置的模块都没有运行。"
        else:
            status = "ok"
            reason = "模块状态采集完成。"
        return {
            "timestamp": now_iso(),
            "source": "config+procfs+maps+cyber_cli",
            "status": status,
            "reason": reason,
            "available": proc_available and environment_available,
            "modules": module_reports,
            "counts": counts,
        }

    def _decorate_cyber(
        self, cyber_payload: Mapping[str, Any]
    ) -> tuple[dict[str, Any], dict[str, Any]]:
        fallback_reason = str(cyber_payload.get("reason") or "Cyber 数据采集失败")
        channels_payload = dict(
            cyber_payload.get("channels")
            or {
                "timestamp": now_iso(),
                "source": "cyber_cli",
                "status": "unavailable",
                "reason": fallback_reason,
                "available": False,
                "channels": None,
                "count": None,
            }
        )
        nodes_payload = dict(
            cyber_payload.get("nodes")
            or {
                "timestamp": now_iso(),
                "source": "cyber_cli",
                "status": "unavailable",
                "reason": fallback_reason,
                "available": False,
                "nodes": None,
                "count": None,
            }
        )
        suggested_rows: list[dict[str, Any]] = []
        channel_to_modules: dict[str, list[str]] = {}
        for spec in self.modules:
            for channel in spec.get("suggested_channels", []):
                channel_to_modules.setdefault(channel, []).append(spec["id"])

        available = bool(channels_payload.get("available"))
        discovered = channels_payload.get("channels")
        discovered_names = {item["name"] for item in discovered} if discovered else set()
        if discovered:
            for item in discovered:
                item["suggested_for"] = channel_to_modules.get(item["name"], [])
        for channel, module_ids in channel_to_modules.items():
            suggested_rows.append(
                {
                    "name": channel,
                    "modules": module_ids,
                    "present": (channel in discovered_names) if available else None,
                }
            )
        channels_payload["suggested"] = suggested_rows
        channels_payload["suggested_count"] = sum(
            1 for row in suggested_rows if row["present"] is True
        )
        channels_payload["suggested_total"] = len(suggested_rows)
        return channels_payload, nodes_payload

    # ------------------------------------------------------------------
    # 汇总与总入口
    # ------------------------------------------------------------------
    def collect_summary(
        self,
        environment_payload: Mapping[str, Any],
        processes_payload: Mapping[str, Any],
        libraries_payload: Mapping[str, Any],
        modules_payload: Mapping[str, Any],
        channels_payload: Mapping[str, Any],
        nodes_payload: Mapping[str, Any],
    ) -> dict[str, Any]:
        counts = dict(modules_payload.get("counts", {}))
        process_available = bool(processes_payload.get("available"))
        channel_available = bool(channels_payload.get("available"))
        node_available = bool(nodes_payload.get("available"))
        library_available = bool(libraries_payload.get("available"))
        env_available = bool(environment_payload.get("available"))

        mainboard_count = (
            sum(
                1
                for item in processes_payload.get("processes", [])
                if item.get("is_mainboard")
            )
            if process_available
            else None
        )
        summary_counts = {
            "modules": counts,
            "processes": processes_payload.get("count") if process_available else None,
            "mainboards": mainboard_count,
            "channels": channels_payload.get("count") if channel_available else None,
            "nodes": nodes_payload.get("count") if node_available else None,
            "library_processes": libraries_payload.get("count") if library_available else None,
            "suggested_channels_seen": channels_payload.get("suggested_count")
            if channel_available
            else None,
            "suggested_channels_total": channels_payload.get("suggested_total"),
        }

        alerts: list[dict[str, str]] = []

        def add_alert(level: str, source: str, message: str) -> None:
            if message:
                alerts.append({"level": level, "source": source, "message": message})

        if not env_available:
            add_alert("warning", "environment", environment_payload.get("reason", ""))
        if not process_available:
            add_alert("critical", "processes", processes_payload.get("reason", ""))
        elif processes_payload.get("status") == "degraded":
            add_alert("warning", "processes", processes_payload.get("reason", ""))
        if not library_available:
            add_alert("warning", "libraries", libraries_payload.get("reason", ""))
        elif libraries_payload.get("status") == "degraded":
            add_alert("warning", "libraries", libraries_payload.get("reason", ""))
        if not channel_available:
            add_alert("warning", "channels", channels_payload.get("reason", ""))
        if not node_available:
            add_alert("warning", "nodes", nodes_payload.get("reason", ""))
        if channel_available and channels_payload.get("status") == "degraded":
            add_alert("warning", "channels", channels_payload.get("reason", ""))

        for module in modules_payload.get("modules", []):
            if module["status"] in {"degraded", "unknown"}:
                add_alert("warning", f"module:{module['id']}", module.get("reason", ""))
        if counts.get("stopped"):
            add_alert(
                "info",
                "modules",
                f"有 {counts['stopped']} 个配置模块未运行；如果车辆未启动这是正常现象。",
            )

        if not counts.get("total"):
            status = "unavailable"
            reason = "没有可用的模块配置，无法汇总。"
        elif not env_available:
            status = "unavailable"
            reason = (
                "未检测到同时包含 cyber/ 和 modules/ 的 Apollo 根目录，"
                "无法确认实车模块状态；进程列表仍可能可读，但请先确认是否在 Cyber 容器内。"
            )
        elif not process_available:
            status = "unavailable"
            reason = "进程数据不可用，无法判断实车运行状态。"
        elif counts.get("running", 0) == 0 and counts.get("stopped", 0) > 0:
            status = "stopped"
            reason = "没有配置模块在运行；若车辆未启动，这是正常状态。"
        elif counts.get("degraded") or counts.get("unknown"):
            status = "degraded"
            reason = "有模块降级或状态未知，请查看模块卡片和不可用原因。"
        elif counts.get("stopped"):
            status = "degraded"
            reason = f"部分模块未运行（{counts['stopped']} 个）。"
        elif not channel_available or not node_available:
            status = "degraded"
            reason = "进程和模块已运行，但 Cyber 通道/节点信息不完整。"
        else:
            status = "ok"
            reason = "配置的模块都在运行，目标 .so 已确认映射。"

        return {
            "timestamp": now_iso(),
            "source": "aggregate",
            "status": status,
            "reason": reason,
            "available": status != "unavailable",
            "read_only": True,
            "environment": {
                "available": env_available,
                "apollo_root": environment_payload.get("apollo_root"),
                "in_container": environment_payload.get("in_container"),
            },
            "counts": summary_counts,
            "alerts": alerts,
            "data_sources": {
                "environment": {
                    "available": env_available,
                    "reason": environment_payload.get("reason", ""),
                },
                "processes": {
                    "available": process_available,
                    "source": processes_payload.get("source", ""),
                    "reason": processes_payload.get("reason", ""),
                },
                "libraries": {
                    "available": library_available,
                    "reason": libraries_payload.get("reason", ""),
                },
                "channels": {
                    "available": channel_available,
                    "reason": channels_payload.get("reason", ""),
                },
                "nodes": {
                    "available": node_available,
                    "reason": nodes_payload.get("reason", ""),
                },
            },
        }

    def collect_config(self, environment_payload: Mapping[str, Any]) -> dict[str, Any]:
        return {
            "timestamp": now_iso(),
            "source": "config",
            "status": "ok" if not self.config_error else "unavailable",
            "reason": self.config_error,
            "available": not self.config_error,
            "config_path": str(self.config_path),
            "schema_version": self.config.get("schema_version"),
            "apollo": environment_payload,
            "modules": self.modules,
            "read_only": True,
            "control_endpoints": [],
            "notes": [
                "本服务只有 GET 查询接口，没有启动、停止、重启或车辆控制接口。",
                "模块、DAG、通道建议均来自 config/modules.json 与只读采集。",
            ],
        }

    def _error_payload(self, source: str, exc: Exception) -> dict[str, Any]:
        return {
            "timestamp": now_iso(),
            "source": source,
            "status": "unavailable",
            "reason": f"采集器内部错误：{exc.__class__.__name__}: {exc}",
            "available": False,
        }

    def _safe(self, source: str, function: Callable[[], dict[str, Any]]) -> dict[str, Any]:
        try:
            return function()
        except Exception as exc:  # 单个数据源出错不能拖垮整个服务
            traceback.print_exc(file=sys.stderr)
            return self._error_payload(source, exc)

    def collect(self) -> dict[str, Any]:
        started = time.monotonic()
        environment_payload = self._safe("environment", self.collect_environment)
        processes_payload = self._safe("processes", self.collect_processes)
        libraries_payload = self._safe(
            "libraries", lambda: self.collect_libraries(processes_payload)
        )
        cyber_payload = self._safe("cyber", self.collect_cyber)
        modules_payload = self._safe(
            "modules",
            lambda: self.collect_modules(
                processes_payload, libraries_payload, cyber_payload
            ),
        )
        channels_payload, nodes_payload = self._decorate_cyber(cyber_payload)
        summary_payload = self._safe(
            "summary",
            lambda: self.collect_summary(
                environment_payload,
                processes_payload,
                libraries_payload,
                modules_payload,
                channels_payload,
                nodes_payload,
            ),
        )
        config_payload = self._safe(
            "config", lambda: self.collect_config(environment_payload)
        )
        elapsed_ms = int((time.monotonic() - started) * 1000)
        return {
            "collected_at": now_iso(),
            "duration_ms": elapsed_ms,
            "read_only": True,
            "summary": summary_payload,
            "modules": modules_payload,
            "processes": processes_payload,
            "libraries": libraries_payload,
            "channels": channels_payload,
            "nodes": nodes_payload,
            "config": config_payload,
        }


def _unique_paths(paths: Iterable[Path]) -> list[Path]:
    seen: set[str] = set()
    result: list[Path] = []
    for path in paths:
        key = str(path)
        if key not in seen:
            seen.add(key)
            result.append(path)
    return result


def validate_config(config_path: str | os.PathLike[str] | None = None) -> list[str]:
    """给 --check 用的轻量校验；返回错误列表，空列表表示通过。"""
    collector = Collector(config_path=config_path)
    errors: list[str] = []
    if collector.config_error:
        errors.append(collector.config_error)
    if not collector.modules:
        errors.append("没有加载到任何模块")
    if collector.apollo_root is None:
        # 只提示，不阻止在开发机上做静态检查。
        pass
    for spec in collector.modules:
        if spec["binary"]:
            continue
        if not spec["dags"]:
            errors.append(f"{spec['id']}: 非二进制模块没有配置 DAG")
        for dag in spec["dags"]:
            if not dag.endswith(".dag"):
                errors.append(f"{spec['id']}: DAG 路径不以 .dag 结尾：{dag}")
    return errors
