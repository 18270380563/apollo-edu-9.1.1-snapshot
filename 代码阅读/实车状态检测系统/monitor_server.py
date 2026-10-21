#!/usr/bin/env python3
"""Apollo 实车状态只读监控服务（Python 3 标准库）。

只提供：
  - GET /                 静态仪表板
  - GET /app.js /style.css
  - GET /api/summary /api/modules /api/channels /api/processes
        /api/libraries /api/config /api/nodes
  - GET /api/realtime  实时订阅快照（只读）
  - GET /api/stream    Server-Sent Events，推送实时快照

没有 POST/PUT/DELETE 控制接口，不会启动、停止或操作任何 Apollo 模块。
实时层不可用时，上面的健康接口仍然照常工作。
"""

from __future__ import annotations

import argparse
import json
import os
import signal
import sys
import threading
import time
import traceback
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import parse_qs, urlsplit

# 避免在目标目录生成 __pycache__；本地模块导入前设置。
sys.dont_write_bytecode = True

from collector import Collector, validate_config  # noqa: E402
from realtime_bridge import (  # noqa: E402
    DEFAULT_REALTIME_CONFIG_PATH,
    RealtimeManager,
    load_realtime_config,
    unavailable_snapshot,
    validate_realtime_config,
)

BASE_DIR = Path(__file__).resolve().parent
WEB_DIR = BASE_DIR / "web"

API_KEYS = {
    "/api/summary": "summary",
    "/api/modules": "modules",
    "/api/channels": "channels",
    "/api/processes": "processes",
    "/api/libraries": "libraries",
    "/api/config": "config",
    "/api/nodes": "nodes",
}

STATIC_FILES = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/index.html": ("index.html", "text/html; charset=utf-8"),
    "/app.js": ("app.js", "application/javascript; charset=utf-8"),
    "/style.css": ("style.css", "text/css; charset=utf-8"),
}


def _unavailable(source: str, reason: str, extra: dict[str, Any] | None = None) -> dict[str, Any]:
    payload: dict[str, Any] = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "source": source,
        "status": "unavailable",
        "reason": reason,
        "available": False,
    }
    if extra:
        payload.update(extra)
    return payload


def failure_snapshot(reason: str) -> dict[str, Any]:
    """整份快照意外失败时，仍然给前端一个结构完整的 JSON。"""
    summary = _unavailable(
        "aggregate",
        reason,
        {
            "read_only": True,
            "counts": {
                "modules": None,
                "processes": None,
                "mainboards": None,
                "channels": None,
                "nodes": None,
                "library_processes": None,
                "suggested_channels_seen": None,
                "suggested_channels_total": None,
            },
            "alerts": [{"level": "critical", "source": "server", "message": reason}],
            "environment": {"available": False, "apollo_root": None, "in_container": None},
            "data_sources": {},
        },
    )
    modules = _unavailable(
        "config+procfs+maps+cyber_cli", reason, {"modules": [], "counts": None}
    )
    return {
        "collected_at": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "duration_ms": 0,
        "read_only": True,
        "summary": summary,
        "modules": modules,
        "processes": _unavailable(
            "procfs+ps", reason, {"processes": [], "count": None, "scanned": None, "errors": []}
        ),
        "libraries": _unavailable(
            "procfs:/proc/<pid>/maps",
            reason,
            {"processes": [], "count": None, "readable_count": None},
        ),
        "channels": _unavailable(
            "cyber_cli",
            reason,
            {"channels": None, "count": None, "suggested": [], "suggested_count": None, "suggested_total": None},
        ),
        "nodes": _unavailable("cyber_cli", reason, {"nodes": None, "count": None}),
        "config": _unavailable(
            "config", reason, {"modules": [], "control_endpoints": [], "read_only": True}
        ),
    }


class SnapshotCache:
    """短缓存，避免前端多个接口轮询时重复执行 cyber 命令。"""

    def __init__(self, collector: Collector, ttl: float = 2.0) -> None:
        self.collector = collector
        self.ttl = max(0.0, float(ttl))
        self._lock = threading.Lock()
        self._created = 0.0
        self._data: dict[str, Any] | None = None

    def get(self, force: bool = False) -> dict[str, Any]:
        with self._lock:
            age = time.monotonic() - self._created
            if self._data is not None and not force and age < self.ttl:
                return self._data
            try:
                data = self.collector.collect()
            except Exception as exc:  # 采集器自身的兜底
                traceback.print_exc(file=sys.stderr)
                data = failure_snapshot(
                    f"采集器未处理异常：{exc.__class__.__name__}: {exc}"
                )
            self._data = data
            self._created = time.monotonic()
            return data


class MonitorHTTPServer(ThreadingHTTPServer):
    daemon_threads = True
    allow_reuse_address = True

    def __init__(
        self,
        server_address: tuple[str, int],
        collector: Collector,
        web_dir: Path = WEB_DIR,
        cache_ttl: float = 2.0,
        realtime: RealtimeManager | None = None,
        sse_heartbeat: float = 5.0,
        sse_poll: float = 0.5,
        sse_min_interval: float = 0.6,
        max_sse_clients: int = 8,
    ) -> None:
        self.collector = collector
        self.web_dir = web_dir.resolve()
        self.cache = SnapshotCache(collector, ttl=cache_ttl)
        self.realtime = realtime
        self.sse_heartbeat = max(0.2, float(sse_heartbeat))
        self.sse_poll = max(0.1, float(sse_poll))
        self.sse_min_interval = max(0.0, float(sse_min_interval))
        self.stop_event = threading.Event()
        self._sse_lock = threading.Lock()
        self._active_sse = 0
        self.max_sse_clients = max(1, int(max_sse_clients))
        super().__init__(server_address, MonitorRequestHandler)

    def acquire_sse_slot(self) -> bool:
        with self._sse_lock:
            if self._active_sse >= self.max_sse_clients:
                return False
            self._active_sse += 1
            return True

    def release_sse_slot(self) -> None:
        with self._sse_lock:
            if self._active_sse > 0:
                self._active_sse -= 1

    @property
    def active_sse(self) -> int:
        with self._sse_lock:
            return self._active_sse

    def shutdown(self) -> None:
        # 先通知正在写 SSE 的 handler 线程退出，再走标准库的关闭流程。
        self.stop_event.set()
        super().shutdown()


class MonitorRequestHandler(BaseHTTPRequestHandler):
    server_version = "ApolloReadOnlyMonitor/1.0"
    sys_version = ""
    protocol_version = "HTTP/1.1"

    # ------------------------------------------------------------------
    # HTTP 方法：只读
    # ------------------------------------------------------------------
    def do_GET(self) -> None:  # noqa: N802
        self._handle(send_body=True)

    def do_HEAD(self) -> None:  # noqa: N802
        self._handle(send_body=False)

    def do_POST(self) -> None:  # noqa: N802
        self._method_not_allowed()

    def do_PUT(self) -> None:  # noqa: N802
        self._method_not_allowed()

    def do_DELETE(self) -> None:  # noqa: N802
        self._method_not_allowed()

    def do_PATCH(self) -> None:  # noqa: N802
        self._method_not_allowed()

    def _method_not_allowed(self) -> None:
        # 不读取请求体，直接关闭连接，避免把未消费的 body 当成下一个请求。
        self.close_connection = True
        self._send_json(
            405,
            {
                "source": "server",
                "status": "unavailable",
                "reason": "本服务是只读监控，只接受 GET/HEAD，不提供控制接口。",
                "available": False,
            },
            send_body=True,
            extra_headers={"Allow": "GET, HEAD"},
        )

    # ------------------------------------------------------------------
    # 路由
    # ------------------------------------------------------------------
    def _handle(self, send_body: bool) -> None:
        try:
            parsed = urlsplit(self.path)
            path = parsed.path
            if path in API_KEYS:
                force = parse_qs(parsed.query).get("refresh", ["0"])[0] in {"1", "true", "yes"}
                data = self.server.cache.get(force=force)  # type: ignore[attr-defined]
                payload = data.get(API_KEYS[path])
                if payload is None:
                    payload = _unavailable("server", f"接口 {path} 没有对应数据")
                self._send_json(200, payload, send_body=send_body)
                return

            if path == "/api/realtime":
                self._send_json(
                    200, self._realtime_snapshot(), send_body=send_body
                )
                return

            if path == "/api/stream":
                self._handle_stream(send_body=send_body)
                return

            if path == "/favicon.ico":
                self._send_empty(204, send_body)
                return

            if path in STATIC_FILES:
                filename, content_type = STATIC_FILES[path]
                self._send_static(filename, content_type, send_body)
                return

            self._send_json(
                404,
                {
                    "source": "server",
                    "status": "unavailable",
                    "reason": f"未知路径：{path}",
                    "available": False,
                },
                send_body=send_body,
            )
        except (BrokenPipeError, ConnectionResetError):
            return
        except Exception as exc:
            traceback.print_exc(file=sys.stderr)
            try:
                self._send_json(
                    500,
                    {
                        "source": "server",
                        "status": "unavailable",
                        "reason": f"服务端错误：{exc.__class__.__name__}: {exc}",
                        "available": False,
                    },
                    send_body=send_body,
                )
            except Exception:
                pass

    # ------------------------------------------------------------------
    # 实时数据 / SSE
    # ------------------------------------------------------------------
    def _realtime_snapshot(self) -> dict[str, Any]:
        manager = getattr(self.server, "realtime", None)  # type: ignore[attr-defined]
        if manager is None:
            return unavailable_snapshot(
                "实时采集层未启用（启动参数 --no-realtime，或服务没有创建 "
                "RealtimeManager）。健康接口仍可用。"
            )
        try:
            snapshot = manager.snapshot()
            if not isinstance(snapshot, dict):
                raise TypeError("实时快照不是 JSON 对象")
            return snapshot
        except Exception as exc:
            traceback.print_exc(file=sys.stderr)
            return unavailable_snapshot(
                f"实时快照生成失败：{exc.__class__.__name__}: {exc}"
            )

    @staticmethod
    def _sse_data(event: str, payload: dict[str, Any]) -> bytes:
        body = json.dumps(
            payload,
            ensure_ascii=False,
            separators=(",", ":"),
            allow_nan=False,
        )
        # JSON 字符串里的换行会被转义，因此 data 一定是单行，符合 SSE 规范。
        return f"event: {event}\ndata: {body}\n\n".encode("utf-8")

    def _write_chunk(self, data: bytes) -> None:
        if not data:
            return
        self.wfile.write(f"{len(data):X}\r\n".encode("ascii"))
        self.wfile.write(data)
        self.wfile.write(b"\r\n")
        self.wfile.flush()

    def _handle_stream(self, send_body: bool) -> None:
        """标准库实现 SSE；客户端断开或服务停止时 handler 线程必须退出。"""
        if not send_body:
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream; charset=utf-8")
            self.send_header("Content-Length", "0")
            self.send_header("Cache-Control", "no-cache")
            self._security_headers()
            self.end_headers()
            return

        server = self.server  # type: ignore[assignment]
        if not server.acquire_sse_slot():  # type: ignore[attr-defined]
            self._send_json(
                503,
                {
                    "source": "realtime_sse",
                    "status": "unavailable",
                    "available": False,
                    "reason": "同时打开的 SSE 客户端过多，请关闭旧页面后重试。",
                },
                send_body=True,
                extra_headers={"Retry-After": "5"},
            )
            return

        self.close_connection = True
        try:
            self.send_response(200)
            self.send_header(
                "Content-Type", "text/event-stream; charset=utf-8"
            )
            self.send_header("Cache-Control", "no-cache, no-transform")
            self.send_header("Connection", "close")
            self.send_header("X-Accel-Buffering", "no")
            self.send_header("Transfer-Encoding", "chunked")
            self._security_headers()
            self.end_headers()
            # 慢客户端/半开连接不能把 handler 线程永远卡在 write 上。
            try:
                self.connection.settimeout(30.0)
            except OSError:
                pass

            self._write_chunk(b"retry: 3000\n\n")
            payload = self._realtime_snapshot()
            self._write_chunk(self._sse_data("hello", payload))
            if not payload.get("available"):
                self._write_chunk(
                    self._sse_data(
                        "unavailable",
                        {
                            "timestamp": payload.get("timestamp"),
                            "status": payload.get("status", "unavailable"),
                            "available": False,
                            "reason": payload.get("reason", "实时层不可用"),
                        },
                    )
                )

            manager = getattr(server, "realtime", None)
            known_images: dict[str, int] = {}
            if isinstance(payload, dict):
                for channel in payload.get("channels") or []:
                    image = channel.get("image") if isinstance(channel, dict) else None
                    if (
                        isinstance(image, dict)
                        and image.get("data_uri")
                        and image.get("frame_seq") is not None
                    ):
                        known_images[str(channel.get("id"))] = int(
                            image["frame_seq"]
                        )
            last_version = int(payload.get("version") or 0)
            last_send = time.monotonic()
            last_heartbeat = time.monotonic()

            while not server.stop_event.is_set():  # type: ignore[attr-defined]
                server.stop_event.wait(server.sse_poll)  # type: ignore[attr-defined]
                if server.stop_event.is_set():  # type: ignore[attr-defined]
                    break
                now = time.monotonic()
                if manager is not None and now - last_send >= server.sse_min_interval:
                    try:
                        current = manager.snapshot(
                            include_images=True,
                            known_image_seqs=known_images,
                        )
                    except Exception as exc:
                        current = unavailable_snapshot(
                            f"实时快照生成失败：{exc.__class__.__name__}: {exc}"
                        )
                    version = int(current.get("version") or 0)
                    # 只在版本变化时推送；不可用状态已经由 hello/unavailable
                    # 事件说明，不能每个轮询周期重复刷同一条大 JSON。
                    if version != last_version:
                        self._write_chunk(self._sse_data("realtime", current))
                        last_version = version
                        last_send = now
                        last_heartbeat = now
                        for channel in current.get("channels") or []:
                            if not isinstance(channel, dict):
                                continue
                            image = channel.get("image")
                            if (
                                isinstance(image, dict)
                                and image.get("data_uri")
                                and image.get("frame_seq") is not None
                            ):
                                known_images[str(channel.get("id"))] = int(
                                    image["frame_seq"]
                                )
                if now - last_heartbeat >= server.sse_heartbeat:
                    self._write_chunk(
                        f": keep-alive {time.strftime('%Y-%m-%dT%H:%M:%S%z')}\n\n".encode(
                            "utf-8"
                        )
                    )
                    last_heartbeat = now
        except (BrokenPipeError, ConnectionResetError, TimeoutError, OSError):
            pass
        except Exception as exc:
            # 响应头已经发出去了，不能再改成 JSON；记录后直接结束这一条 SSE。
            traceback.print_exc(file=sys.stderr)
            sys.stderr.write(
                f"SSE handler 异常退出：{exc.__class__.__name__}: {exc}\n"
            )
        finally:
            try:
                self.wfile.write(b"0\r\n\r\n")
                self.wfile.flush()
            except Exception:
                pass
            try:
                server.release_sse_slot()  # type: ignore[attr-defined]
            except Exception:
                pass

    # ------------------------------------------------------------------
    # 响应
    # ------------------------------------------------------------------
    def _security_headers(self) -> None:
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Referrer-Policy", "no-referrer")
        self.send_header(
            "Content-Security-Policy",
            "default-src 'self'; script-src 'self'; style-src 'self'; "
            "connect-src 'self'; img-src 'self' data:; base-uri 'none'; "
            "form-action 'none'; frame-ancestors 'none'",
        )

    def _send_json(
        self,
        status: int,
        payload: dict[str, Any],
        send_body: bool = True,
        extra_headers: dict[str, str] | None = None,
    ) -> None:
        body = json.dumps(
            payload,
            ensure_ascii=False,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self._security_headers()
        for key, value in (extra_headers or {}).items():
            self.send_header(key, value)
        self.end_headers()
        if send_body:
            self.wfile.write(body)

    def _send_static(self, filename: str, content_type: str, send_body: bool) -> None:
        file_path = (self.server.web_dir / filename).resolve()  # type: ignore[attr-defined]
        # 白名单已经把路径限制在 web/ 下，这里再做一次边界检查。
        if self.server.web_dir not in file_path.parents:  # type: ignore[attr-defined]
            self._send_json(
                404,
                {"source": "server", "status": "unavailable", "reason": "文件不在 web 目录内"},
                send_body=send_body,
            )
            return
        try:
            body = file_path.read_bytes()
        except OSError as exc:
            self._send_json(
                500,
                {
                    "source": "server",
                    "status": "unavailable",
                    "reason": f"静态文件读取失败：{exc}",
                    "available": False,
                },
                send_body=send_body,
            )
            return
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-cache")
        self._security_headers()
        self.end_headers()
        if send_body:
            self.wfile.write(body)

    def _send_empty(self, status: int, send_body: bool) -> None:
        self.send_response(status)
        self.send_header("Content-Length", "0")
        self.send_header("Cache-Control", "no-store")
        self._security_headers()
        self.end_headers()

    def log_message(self, fmt: str, *args: Any) -> None:
        # 只写 stderr，不落盘，避免生成日志垃圾。
        sys.stderr.write(
            "%s [%s] %s\n"
            % (self.log_date_time_string(), self.address_string(), fmt % args)
        )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Apollo 实车运行状态只读监控（标准库 HTTP 服务）"
    )
    parser.add_argument(
        "--host",
        default=os.environ.get("HOST", "127.0.0.1"),
        help="绑定地址，默认 127.0.0.1；实车局域网访问显式改为 0.0.0.0",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=os.environ.get("PORT", "8848"),
        help="监听端口，默认 8848；传 0 可让系统分配临时端口",
    )
    parser.add_argument(
        "--config",
        default=str(BASE_DIR / "config" / "modules.json"),
        help="模块配置文件路径",
    )
    parser.add_argument(
        "--apollo-root",
        default=os.environ.get("MONITOR_APOLLO_ROOT", ""),
        help="Apollo 根目录（包含 cyber/ 和 modules/），默认自动探测",
    )
    parser.add_argument(
        "--cyber-tool-dir",
        default=os.environ.get("MONITOR_CYBER_BIN_DIR", ""),
        help="cyber_channel/cyber_node 所在目录，默认从 PATH 和 Apollo 根目录查找",
    )
    parser.add_argument(
        "--cache-ttl",
        type=float,
        default=2.0,
        help="快照缓存秒数，默认 2 秒",
    )
    parser.add_argument(
        "--realtime-config",
        default=str(DEFAULT_REALTIME_CONFIG_PATH),
        help="实时通道配置文件路径，默认 config/realtime_channels.json",
    )
    parser.add_argument(
        "--no-realtime",
        action="store_true",
        help="只启动健康监控，不导入 Cyber、不创建任何 reader",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="只做静态检查，不启动 HTTP 服务",
    )
    return parser


def run_check(args: argparse.Namespace) -> int:
    errors = validate_config(args.config)
    if errors:
        for error in errors:
            print(f"[错误] {error}", file=sys.stderr)
        return 1

    realtime_errors = validate_realtime_config(args.realtime_config)
    if realtime_errors:
        for error in realtime_errors:
            print(f"[错误] 实时配置：{error}", file=sys.stderr)
        return 1
    realtime_config = load_realtime_config(args.realtime_config)
    enabled_realtime = sum(
        1 for channel in realtime_config.channels if channel.enabled
    )

    missing: list[str] = []
    for filename in ("index.html", "app.js", "style.css"):
        path = WEB_DIR / filename
        if not path.is_file() or path.stat().st_size == 0:
            missing.append(str(path))
    if missing:
        for path in missing:
            print(f"[错误] 缺少或为空的网页文件：{path}", file=sys.stderr)
        return 1

    for filename in (
        "collector.py",
        "realtime_bridge.py",
        "monitor_server.py",
        "test_collector.py",
        "test_realtime.py",
    ):
        path = BASE_DIR / filename
        if not path.exists():
            if filename in {"test_collector.py", "test_realtime.py"}:
                continue
            print(f"[错误] 缺少文件：{path}", file=sys.stderr)
            return 1
        try:
            compile(path.read_text(encoding="utf-8"), str(path), "exec")
        except SyntaxError as exc:
            print(f"[错误] {filename} 语法错误：{exc}", file=sys.stderr)
            return 1

    if not (0 <= int(args.port) <= 65535):
        print(f"[错误] 端口超出范围：{args.port}", file=sys.stderr)
        return 1

    collector = Collector(
        config_path=args.config,
        apollo_root=args.apollo_root or None,
        cyber_tool_dir=args.cyber_tool_dir or None,
    )
    print(f"[OK] 配置：{collector.config_path}（{len(collector.modules)} 个模块）")
    print(
        f"[OK] 实时配置：{realtime_config.path}（"
        f"{len(realtime_config.channels)} 个通道，{enabled_realtime} 个启用，"
        f"CAN 字段 {len(realtime_config.can.get('fields', []))} 个）"
    )
    print("[OK] 网页：web/index.html、web/app.js、web/style.css")
    print(
        "[OK] Python 语法：collector.py、realtime_bridge.py、"
        "monitor_server.py、test_collector.py、test_realtime.py"
    )
    print(f"[OK] 监听参数：HOST={args.host} PORT={args.port}")
    if collector.apollo_root is None:
        print(
            "[提示] 当前未探测到 Apollo 根目录；在开发机上这是正常的，"
            "实车/容器内请用 --apollo-root 或 APOLLO_ROOT_DIR 指定。"
        )
    print("检查通过；未启动服务，未执行任何 Apollo 命令。")
    return 0


def main() -> int:
    if sys.version_info < (3, 10):
        print("错误：需要 Python 3.10 或更高版本。", file=sys.stderr)
        return 2
    parser = build_parser()
    args = parser.parse_args()

    if args.check:
        return run_check(args)

    if not (0 <= int(args.port) <= 65535):
        print(f"端口超出范围：{args.port}", file=sys.stderr)
        return 2

    collector = Collector(
        config_path=args.config,
        apollo_root=args.apollo_root or None,
        cyber_tool_dir=args.cyber_tool_dir or None,
    )
    if collector.config_error:
        print(f"[警告] {collector.config_error}", file=sys.stderr)
        print("[警告] 服务仍会启动，接口会返回不可用原因。", file=sys.stderr)

    realtime_manager: RealtimeManager | None = None
    if not args.no_realtime:
        realtime_manager = RealtimeManager(args.realtime_config)
        if realtime_manager.config.error:
            print(
                f"[警告] 实时配置不可用：{realtime_manager.config.error}",
                file=sys.stderr,
            )
            print(
                "[警告] 服务仍会启动；/api/realtime 会返回不可用原因。",
                file=sys.stderr,
            )
        realtime_manager.start()
        # Cyber 的 C++ init 会安装自己的 SIGINT 处理函数。等它完成初始化后，
        # 主线程再把 Python 的 Ctrl+C 处理恢复回来，避免网页服务停不下来。
        deadline = time.monotonic() + 3.0
        while (
            realtime_manager.status == "starting"
            and time.monotonic() < deadline
        ):
            time.sleep(0.05)

    try:
        server = MonitorHTTPServer(
            (args.host, int(args.port)),
            collector=collector,
            web_dir=WEB_DIR,
            cache_ttl=args.cache_ttl,
            realtime=realtime_manager,
        )
    except OSError as exc:
        print(f"无法绑定 {args.host}:{args.port}：{exc}", file=sys.stderr)
        if realtime_manager is not None:
            realtime_manager.stop()
        return 1

    host, port = server.server_address[:2]
    print(f"Apollo 只读监控已启动：http://{host}:{port}")
    print("只读：只有 GET 查询接口，不会启动/停止 Apollo，也不会控制车辆。")
    if realtime_manager is not None:
        print(
            f"实时通道：{len(realtime_manager.config.channels)} 个配置，"
            "订阅状态与不可用原因见 /api/realtime 或网页“实时数据”区域。"
        )
    if args.host == "0.0.0.0":
        print(
            "警告：当前绑定到 0.0.0.0，同一局域网内任何人都能访问该只读页面；"
            "请只在受信车辆局域网使用，并配置主机防火墙。",
            file=sys.stderr,
        )
    def handle_stop(signum: int, frame: Any) -> None:
        raise KeyboardInterrupt

    # 前台 Ctrl+C 和后台 SIGTERM/SIGHUP 都走同一条关闭路径。
    for signal_name in ("SIGINT", "SIGTERM", "SIGHUP"):
        signal_value = getattr(signal, signal_name, None)
        if signal_value is not None:
            signal.signal(signal_value, handle_stop)

    print("按 Ctrl+C 停止。", flush=True)
    try:
        server.serve_forever(poll_interval=0.5)
    except KeyboardInterrupt:
        print("\n收到停止信号，正在关闭监控服务……", file=sys.stderr)
    finally:
        server.shutdown()
        server.server_close()
        if realtime_manager is not None:
            realtime_manager.stop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
