"""Apollo Cyber 只读实时数据桥。

这个模块只做三件事：
1. 按 config/realtime_channels.json 创建 Cyber Python reader；
2. 把收到的消息保留“每个通道最新一条”，并做有上限的 protobuf/图片转换；
3. 给 monitor_server.py 提供线程安全的快照，供 /api/realtime 和 SSE 使用。

边界：
- 不创建 writer、service、client，不发送任何消息；
- 不调用 cyber_launch，不启动/停止 Apollo 模块；
- 不写历史、不落盘、不调用 shell；
- Cyber、protobuf、OpenCV 任一不可用时只降级并给出原因，不让网页退出。

本模块只依赖 Python 标准库。google.protobuf、Cyber Python、OpenCV 都是
“已存在则使用”的可选依赖，导入失败不会影响 monitor_server 启动。
"""

from __future__ import annotations

import base64
import copy
import importlib
import json
import math
import os
import re
import threading
import time
from collections import deque
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Any, Callable, Iterable, Mapping

BASE_DIR = Path(__file__).resolve().parent
DEFAULT_REALTIME_CONFIG_PATH = BASE_DIR / "config" / "realtime_channels.json"

ALLOWED_KINDS = {"protobuf", "image", "compressed_image"}
ALLOWED_CAN_FIELD_KINDS = {"float", "int", "bool", "string", "enum"}
ID_RE = re.compile(r"^[A-Za-z0-9_.-]{1,80}$")

DEFAULT_LIMITS: dict[str, int] = {
    "max_depth": 8,
    "max_items": 200,
    "max_json_bytes": 262144,
    "max_image_bytes": 2097152,
    "max_string_chars": 2000,
}

# protobuf FieldDescriptor.type 的数值常量。刻意不 import google.protobuf，
# 这样没有安装 protobuf 的机器也能导入本模块并显示“不可用”。
PROTO_TYPE_DOUBLE = 1
PROTO_TYPE_FLOAT = 2
PROTO_TYPE_INT64 = 3
PROTO_TYPE_UINT64 = 4
PROTO_TYPE_INT32 = 5
PROTO_TYPE_FIXED64 = 6
PROTO_TYPE_FIXED32 = 7
PROTO_TYPE_BOOL = 8
PROTO_TYPE_STRING = 9
PROTO_TYPE_GROUP = 10
PROTO_TYPE_MESSAGE = 11
PROTO_TYPE_BYTES = 12
PROTO_TYPE_UINT32 = 13
PROTO_TYPE_ENUM = 14
PROTO_TYPE_SFIXED32 = 15
PROTO_TYPE_SFIXED64 = 16
PROTO_TYPE_SINT32 = 17
PROTO_TYPE_SINT64 = 18
PROTO_TYPE_REPEATED = 3
JS_MAX_SAFE_INTEGER = 9007199254740991

# 防止无限等待或巨量数据拖垮网页。这些是内部安全阀，不是车辆参数。
MAX_RAW_IMAGE_BYTES = 64 * 1024 * 1024
MAX_IMAGE_PIXELS = 40_000_000
MAX_CHANNEL_EVENTS = 240


def now_iso() -> str:
    """本地时区 ISO 时间，便于现场直接看。"""
    return datetime.now().astimezone().isoformat(timespec="milliseconds")


def _truncate(text: Any, limit: int = 1000) -> str:
    value = str(text or "").strip()
    return value if len(value) <= limit else value[:limit] + "…"


def _finite_number(value: Any) -> int | float | None:
    """NaN/Infinity 不能进入 JSON；统一变成 None。"""
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, float):
        if not math.isfinite(value):
            return None
        return value
    if isinstance(value, int):
        if abs(value) > JS_MAX_SAFE_INTEGER:
            return int(value)
        return value
    return None


def _json_bytes(payload: Any) -> int:
    try:
        return len(
            json.dumps(
                payload,
                ensure_ascii=False,
                separators=(",", ":"),
                allow_nan=False,
            ).encode("utf-8")
        )
    except (TypeError, ValueError):
        return 0


# ---------------------------------------------------------------------------
# 配置
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class ChannelSpec:
    id: str
    label: str
    channel: str
    kind: str
    python_module: str
    message_class: str
    enabled: bool = True
    max_fps: float = 10.0
    group: str = "other"
    source: str = ""
    site_confirm: str = ""


@dataclass(frozen=True)
class CanFieldSpec:
    path: str
    label: str
    kind: str = "string"
    unit: str = ""
    source: str = "chassis"
    enum_labels: Mapping[str, str] = field(default_factory=dict)
    fallback_source: str = ""
    fallback_path: str = ""


@dataclass
class RealtimeConfig:
    path: Path
    raw: dict[str, Any] = field(default_factory=dict)
    channels: list[ChannelSpec] = field(default_factory=list)
    limits: dict[str, int] = field(default_factory=lambda: dict(DEFAULT_LIMITS))
    can: dict[str, Any] = field(default_factory=dict)
    error: str = ""


def _normalise_limits(raw: Any, errors: list[str]) -> dict[str, int]:
    limits = dict(DEFAULT_LIMITS)
    if raw is None:
        return limits
    if not isinstance(raw, dict):
        errors.append("limits 必须是对象")
        return limits
    for key, default in DEFAULT_LIMITS.items():
        if key not in raw:
            continue
        try:
            value = int(raw[key])
        except (TypeError, ValueError):
            errors.append(f"limits.{key} 必须是整数")
            continue
        if value <= 0:
            errors.append(f"limits.{key} 必须大于 0")
            continue
        limits[key] = value
    limits["max_depth"] = min(max(1, limits["max_depth"]), 32)
    limits["max_items"] = min(max(1, limits["max_items"]), 5000)
    limits["max_json_bytes"] = min(
        max(4096, limits["max_json_bytes"]), 20 * 1024 * 1024
    )
    limits["max_image_bytes"] = min(
        max(1024, limits["max_image_bytes"]), 32 * 1024 * 1024
    )
    limits["max_string_chars"] = min(max(32, limits["max_string_chars"]), 100_000)
    return limits


def _normalise_can(raw: Any, errors: list[str]) -> dict[str, Any]:
    can: dict[str, Any] = {
        "chassis_channel_id": "chassis",
        "detail_channel_id": "chassis_detail",
        "fields": [],
    }
    if raw is None:
        return can
    if not isinstance(raw, dict):
        errors.append("can 必须是对象")
        return can
    can["chassis_channel_id"] = str(raw.get("chassis_channel_id") or "chassis")
    can["detail_channel_id"] = str(raw.get("detail_channel_id") or "chassis_detail")
    fields_raw = raw.get("fields") or []
    if not isinstance(fields_raw, list):
        errors.append("can.fields 必须是数组")
        return can
    for index, item in enumerate(fields_raw):
        if not isinstance(item, dict):
            errors.append(f"can.fields 第 {index + 1} 项不是对象")
            continue
        path = str(item.get("path") or "").strip()
        label = str(item.get("label") or "").strip()
        if not path or not label:
            errors.append(f"can.fields 第 {index + 1} 项缺少 path 或 label")
            continue
        kind = str(item.get("kind") or "string").strip().lower()
        if kind not in ALLOWED_CAN_FIELD_KINDS:
            errors.append(f"can.fields.{path} 的 kind 不支持：{kind}")
            continue
        enum_labels_raw = item.get("enum_labels") or {}
        if not isinstance(enum_labels_raw, dict):
            errors.append(f"can.fields.{path} 的 enum_labels 必须是对象")
            enum_labels_raw = {}
        can["fields"].append(
            CanFieldSpec(
                path=path,
                label=label,
                kind=kind,
                unit=str(item.get("unit") or ""),
                source=str(item.get("source") or "chassis"),
                enum_labels={
                    str(key): str(value) for key, value in enum_labels_raw.items()
                },
                fallback_source=str(item.get("fallback_source") or ""),
                fallback_path=str(item.get("fallback_path") or ""),
            )
        )
    return can


def load_realtime_config(
    config_path: str | os.PathLike[str] | None = None,
) -> RealtimeConfig:
    """读取并校验实时通道配置；任何错误都放进 error，不抛异常。"""
    path = Path(config_path or DEFAULT_REALTIME_CONFIG_PATH).expanduser().resolve()
    config = RealtimeConfig(path=path)
    errors: list[str] = []
    try:
        with path.open("r", encoding="utf-8") as handle:
            raw = json.load(handle)
    except FileNotFoundError:
        config.error = f"实时配置文件不存在：{path}"
        return config
    except (OSError, json.JSONDecodeError) as exc:
        config.error = f"读取实时配置失败：{exc.__class__.__name__}: {exc}"
        return config

    if not isinstance(raw, dict):
        config.error = "实时配置根节点必须是对象"
        return config

    schema_version = raw.get("schema_version")
    if schema_version is not None and schema_version != 1:
        errors.append(f"不支持的 schema_version：{schema_version}")

    config.raw = raw
    config.limits = _normalise_limits(raw.get("limits"), errors)
    config.can = _normalise_can(raw.get("can"), errors)

    channels_raw = raw.get("channels")
    if not isinstance(channels_raw, list) or not channels_raw:
        errors.append("channels 必须是非空数组")
        channels_raw = []

    seen_ids: set[str] = set()
    seen_channels: set[str] = set()
    channels: list[ChannelSpec] = []
    for index, item in enumerate(channels_raw):
        if not isinstance(item, dict):
            errors.append(f"channels 第 {index + 1} 项不是对象")
            continue
        channel_id = str(item.get("id") or "").strip()
        channel_name = str(item.get("channel") or "").strip()
        kind = str(item.get("kind") or "").strip().lower()
        python_module = str(item.get("python_module") or "").strip()
        message_class = str(item.get("message_class") or "").strip()
        label = str(item.get("label") or channel_id).strip()
        if not channel_id:
            errors.append(f"channels 第 {index + 1} 项缺少 id")
            continue
        if not ID_RE.match(channel_id):
            errors.append(f"channel id 不合法：{channel_id}")
            continue
        if channel_id in seen_ids:
            errors.append(f"channel id 重复：{channel_id}")
            continue
        seen_ids.add(channel_id)
        if not channel_name.startswith("/"):
            errors.append(f"{channel_id}: channel 必须以 / 开头")
            continue
        if channel_name in seen_channels:
            errors.append(f"channel 重复订阅：{channel_name}")
            continue
        seen_channels.add(channel_name)
        if kind not in ALLOWED_KINDS:
            errors.append(f"{channel_id}: kind 不支持：{kind}")
            continue
        if not python_module or not message_class:
            errors.append(f"{channel_id}: 必须配置 python_module 和 message_class")
            continue
        try:
            max_fps = float(item.get("max_fps", 10))
        except (TypeError, ValueError):
            errors.append(f"{channel_id}: max_fps 必须是数字")
            continue
        if max_fps <= 0:
            errors.append(f"{channel_id}: max_fps 必须大于 0")
            continue
        channels.append(
            ChannelSpec(
                id=channel_id,
                label=label or channel_id,
                channel=channel_name,
                kind=kind,
                python_module=python_module,
                message_class=message_class,
                enabled=bool(item.get("enabled", True)),
                max_fps=min(max(0.1, max_fps), 120.0),
                group=str(item.get("group") or "other").strip() or "other",
                source=str(item.get("source") or ""),
                site_confirm=str(item.get("site_confirm") or ""),
            )
        )

    if not any(spec.enabled for spec in channels) and not errors:
        errors.append("channels 中没有 enabled=true 的订阅")
    config.channels = channels
    if errors:
        config.error = "；".join(errors)
    return config


def validate_realtime_config(
    config_path: str | os.PathLike[str] | None = None,
) -> list[str]:
    """给 run_monitor.sh --check 使用的校验入口。"""
    config = load_realtime_config(config_path)
    return [config.error] if config.error else []


# ---------------------------------------------------------------------------
# protobuf -> JSON
# ---------------------------------------------------------------------------


def _enum_name(field_descriptor: Any, value: Any) -> str:
    enum_type = getattr(field_descriptor, "enum_type", None)
    try:
        number = int(value)
    except (TypeError, ValueError):
        return str(value)
    values_by_number = getattr(enum_type, "values_by_number", None)
    if values_by_number is not None:
        try:
            enum_value = values_by_number[number]
        except (KeyError, TypeError):
            enum_value = None
        if enum_value is not None:
            return str(getattr(enum_value, "name", enum_value))
    values = getattr(enum_type, "values", None)
    if values:
        for item in values:
            if int(getattr(item, "number", -1)) == number:
                return str(getattr(item, "name", number))
    # 没有描述符时也不能只返回裸数字，否则网页会误以为枚举已经解码。
    return f"UNKNOWN_ENUM_{number}"


def _is_map_field(field_descriptor: Any) -> bool:
    message_type = getattr(field_descriptor, "message_type", None)
    if message_type is None:
        return False
    options = getattr(message_type, "GetOptions", None)
    if options is None:
        return False
    try:
        return bool(options().map_entry)
    except Exception:
        return False


class ProtoJsonConverter:
    """用描述符反射把 protobuf 消息转成有大小上限的 JSON 字典。

    优先保留 proto 字段名；枚举输出枚举名；bytes 一律不展开，只留长度。
    这样即使 planning/perception 的 debug 字段很大，也不会把网页撑爆。
    """

    def __init__(self, limits: Mapping[str, Any] | None = None) -> None:
        merged = dict(DEFAULT_LIMITS)
        if limits:
            merged.update({key: int(value) for key, value in limits.items()})
        self.max_depth = max(1, int(merged["max_depth"]))
        self.max_items = max(1, int(merged["max_items"]))
        self.max_json_bytes = max(1024, int(merged["max_json_bytes"]))
        self.max_string_chars = max(32, int(merged["max_string_chars"]))
        self._field_budget = 0

    def convert(self, message: Any) -> tuple[dict[str, Any], str]:
        self._field_budget = max(100, self.max_items * 4)
        try:
            data = self._message(message, depth=0)
            if not isinstance(data, dict):
                data = {"value": data}
        except Exception as exc:
            return (
                {"__error__": f"{exc.__class__.__name__}: {_truncate(exc, 300)}"},
                f"protobuf 转 JSON 失败：{exc.__class__.__name__}: {_truncate(exc, 300)}",
            )
        return self._fit_size(data), ""

    def _fit_size(self, data: dict[str, Any]) -> dict[str, Any]:
        if _json_bytes(data) <= self.max_json_bytes:
            return data

        header = data.get("header")
        if not isinstance(header, dict):
            header = None
        compact: dict[str, Any] = {
            "__truncated__": True,
            "__reason__": f"消息超过 {self.max_json_bytes} 字节，已省略大部分字段。",
            "__top_level_fields__": list(data.keys())[:30],
        }
        if header:
            compact["header"] = {
                key: header[key]
                for key in ("timestamp_sec", "module_name", "sequence_num", "frame_id")
                if key in header
            }
        if _json_bytes(compact) <= self.max_json_bytes:
            return compact

        # 极端情况下 header 也很大，继续压缩到只留说明。
        compact.pop("header", None)
        compact["__top_level_fields__"] = compact["__top_level_fields__"][:10]
        return compact

    def _message(self, message: Any, depth: int) -> dict[str, Any]:
        if depth >= self.max_depth:
            return {"__truncated__": f"嵌套超过 {self.max_depth} 层，已停止展开。"}
        try:
            listed = message.ListFields()
        except Exception as exc:
            raise TypeError(f"消息没有 ListFields()：{exc}") from exc

        result: dict[str, Any] = {}
        for field_descriptor, value in listed:
            if self._field_budget <= 0:
                result["__truncated__"] = "字段数量超过上限，后续字段已省略。"
                break
            self._field_budget -= 1
            name = str(getattr(field_descriptor, "name", "") or "unknown")
            try:
                result[name] = self._value(field_descriptor, value, depth + 1)
            except Exception as exc:
                result[name] = f"<字段转换失败：{exc.__class__.__name__}: {_truncate(exc, 160)}>"
        return result

    def _value(self, field_descriptor: Any, value: Any, depth: int) -> Any:
        label = getattr(field_descriptor, "label", 1)
        field_type = int(getattr(field_descriptor, "type", 0))
        if label == PROTO_TYPE_REPEATED:
            if _is_map_field(field_descriptor):
                return self._map_value(field_descriptor, value, depth)
            result: list[Any] = []
            total: int | None = None
            try:
                total = len(value)  # 多数 protobuf 容器支持；不支持也不影响遍历
            except (TypeError, AttributeError):
                total = None
            omitted = 0
            try:
                iterator = iter(value)
            except TypeError:
                iterator = iter([value])
            for index, item in enumerate(iterator):
                if index >= self.max_items:
                    omitted = (total - self.max_items) if total is not None else 1
                    break
                result.append(self._scalar(field_descriptor, item, depth))
            if omitted:
                result.append(f"…（已省略 {omitted} 项）")
            return result
        return self._scalar(field_descriptor, value, depth)

    def _map_value(self, field_descriptor: Any, value: Any, depth: int) -> dict[str, Any]:
        result: dict[str, Any] = {}
        try:
            iterator = iter(value.items())
        except AttributeError:
            return {}
        # map 字段的 message_type 是 map entry；真正要转换的是 entry 的 value
        # 字段，不能拿 map 字段本身递归，否则会无限递归。
        value_field = None
        message_type = getattr(field_descriptor, "message_type", None)
        fields_by_name = getattr(message_type, "fields_by_name", None)
        if fields_by_name:
            value_field = fields_by_name.get("value")
        value_field = value_field or field_descriptor
        total: int | None = None
        try:
            total = len(value)
        except (TypeError, AttributeError):
            total = None
        omitted = 0
        for index, (map_key, map_value) in enumerate(iterator):
            if index >= self.max_items:
                omitted = (total - self.max_items) if total is not None else 1
                break
            result[str(map_key)] = self._scalar(
                value_field, map_value, depth, map_value=True
            )
        if omitted:
            result["__truncated__"] = f"已省略 {omitted} 项"
        return result

    def _scalar(
        self,
        field_descriptor: Any,
        value: Any,
        depth: int,
        map_value: bool = False,
    ) -> Any:
        field_type = int(getattr(field_descriptor, "type", 0))
        if field_type == PROTO_TYPE_MESSAGE or field_type == PROTO_TYPE_GROUP:
            nested = getattr(field_descriptor, "message_type", None)
            if _is_map_field(field_descriptor):
                return self._map_value(field_descriptor, value, depth)
            # map 的 value 可能已经是消息对象，直接递归；普通消息字段也走这里。
            if nested is not None and hasattr(value, "ListFields"):
                return self._message(value, depth)
            if isinstance(value, Mapping):
                return {
                    str(key): _truncate(val, self.max_string_chars)
                    for key, val in list(value.items())[: self.max_items]
                }
            return str(value)
        if field_type == PROTO_TYPE_ENUM:
            return _enum_name(field_descriptor, value)
        if field_type == PROTO_TYPE_BYTES:
            try:
                size = len(value)
            except TypeError:
                size = 0
            return f"<bytes {size} 字节，已省略>"
        if field_type in {PROTO_TYPE_DOUBLE, PROTO_TYPE_FLOAT}:
            number = _finite_number(value)
            return number
        if field_type in (PROTO_TYPE_BOOL,):
            return bool(value)
        if field_type == PROTO_TYPE_STRING:
            return _truncate(value, self.max_string_chars)
        if field_type in {
            PROTO_TYPE_INT32,
            PROTO_TYPE_INT64,
            PROTO_TYPE_UINT32,
            PROTO_TYPE_UINT64,
            PROTO_TYPE_SFIXED32,
            PROTO_TYPE_SFIXED64,
            PROTO_TYPE_SINT32,
            PROTO_TYPE_SINT64,
            PROTO_TYPE_FIXED32,
            PROTO_TYPE_FIXED64,
        }:
            number = _finite_number(value)
            return number
        # 未知字段类型：转成短字符串，避免 bytes 或对象直接进入 JSON。
        return _truncate(value, self.max_string_chars)


def proto_message_to_json(
    message: Any, limits: Mapping[str, Any] | None = None
) -> tuple[dict[str, Any], str]:
    """公开的便捷函数，测试和调试可以直接用。"""
    return ProtoJsonConverter(limits).convert(message)


# ---------------------------------------------------------------------------
# 图片
# ---------------------------------------------------------------------------


def _image_dimensions(data: bytes) -> tuple[int | None, int | None]:
    """不依赖图片库，从 JPEG/PNG 头里读宽高。"""
    if data.startswith(b"\x89PNG\r\n\x1a\n") and len(data) >= 24:
        try:
            width = int.from_bytes(data[16:20], "big")
            height = int.from_bytes(data[20:24], "big")
            return width, height
        except ValueError:
            return None, None
    if data[:2] == b"\xff\xd8":
        index = 2
        length = len(data)
        while index + 9 < length:
            if data[index] != 0xFF:
                index += 1
                continue
            marker = data[index + 1]
            index += 2
            if marker in (0xD8, 0xD9) or 0xD0 <= marker <= 0xD7:
                continue
            if index + 2 > length:
                break
            segment_length = int.from_bytes(data[index : index + 2], "big")
            if segment_length < 2:
                break
            if marker in {
                0xC0,
                0xC1,
                0xC2,
                0xC3,
                0xC5,
                0xC6,
                0xC7,
                0xC9,
                0xCA,
                0xCB,
                0xCD,
                0xCE,
                0xCF,
            } and index + 7 <= length:
                height = int.from_bytes(data[index + 3 : index + 5], "big")
                width = int.from_bytes(data[index + 5 : index + 7], "big")
                return width, height
            index += segment_length
    return None, None


def _sniff_image_mime(data: bytes, declared: str = "") -> str:
    # 先看真实文件头，不能只信 format 字段；声明的格式和实际字节不一致时，
    # 浏览器会显示破图，所以这里直接判为不可用并给出原因。
    if data.startswith(b"\xff\xd8"):
        return "image/jpeg"
    if data.startswith(b"\x89PNG\r\n\x1a\n"):
        return "image/png"
    return ""


def encode_compressed_image(
    message: Any, max_image_bytes: int
) -> dict[str, Any]:
    """把 CompressedImage 直接变成浏览器可显示的 data URI。

    这里不解压再压缩，避免额外 CPU 和画质损失；只检查格式、大小并取宽高。
    """
    try:
        data = bytes(getattr(message, "data", b"") or b"")
    except Exception as exc:
        return _image_error(f"读取 CompressedImage.data 失败：{exc}")
    if not data:
        return _image_error("CompressedImage.data 为空")
    if len(data) > max_image_bytes:
        return _image_error(
            f"压缩图片 {len(data)} 字节，超过上限 {max_image_bytes} 字节"
        )
    declared = str(getattr(message, "format", "") or "")
    mime = _sniff_image_mime(data, declared)
    if not mime:
        return _image_error(
            f"无法识别压缩图片格式（format={declared or '空'}，"
            "只接受 JPEG/PNG）"
        )
    width, height = _image_dimensions(data)
    try:
        width = int(getattr(message, "width", 0) or 0) or width
        height = int(getattr(message, "height", 0) or 0) or height
    except (TypeError, ValueError):
        pass
    encoded = base64.b64encode(data).decode("ascii")
    return {
        "available": True,
        "reason": "",
        "format": mime.split("/", 1)[1],
        "mime": mime,
        "width": width,
        "height": height,
        "bytes": len(data),
        "data_uri": f"data:{mime};base64,{encoded}",
    }


def _image_error(reason: str, **extra: Any) -> dict[str, Any]:
    payload = {
        "available": False,
        "reason": reason,
        "format": "",
        "mime": "",
        "width": None,
        "height": None,
        "bytes": None,
        "data_uri": "",
    }
    payload.update(extra)
    return payload


def encode_raw_image(message: Any, max_image_bytes: int) -> dict[str, Any]:
    """把原始 Image 像素编码成 JPEG；OpenCV 不存在时明确返回原因。"""
    try:
        import cv2  # type: ignore
        import numpy as np  # type: ignore
    except Exception as exc:
        return _image_error(
            "原始 Image 像素需要 OpenCV(cv2) 才能编码成 JPEG；"
            f"当前环境不可用：{exc.__class__.__name__}: {_truncate(exc, 160)}。"
            "优先订阅 CompressedImage 通道。"
        )

    try:
        width = int(getattr(message, "width", 0) or 0)
        height = int(getattr(message, "height", 0) or 0)
        step = int(getattr(message, "step", 0) or 0)
        encoding = str(getattr(message, "encoding", "") or "").upper()
        data = bytes(getattr(message, "data", b"") or b"")
    except Exception as exc:
        return _image_error(f"读取 Image 字段失败：{exc}")
    if width <= 0 or height <= 0 or step <= 0 or not data:
        return _image_error("Image 的 width/height/step/data 不完整")
    if width * height > MAX_IMAGE_PIXELS:
        return _image_error(f"图片像素数 {width}x{height} 超过安全上限")
    if len(data) > MAX_RAW_IMAGE_BYTES:
        return _image_error(f"原始像素 {len(data)} 字节，超过安全上限")
    if len(data) < step * height:
        return _image_error(
            f"原始像素长度 {len(data)} 小于 step*height={step * height}"
        )

    channels_by_encoding = {
        "RGB8": 3,
        "BGR8": 3,
        "RGBA8": 4,
        "BGRA8": 4,
        "MONO8": 1,
        "YUV422": 2,
        "YUYV": 2,
        "UYVY": 2,
    }
    channels = channels_by_encoding.get(encoding)
    if channels is None:
        return _image_error(
            f"原始 Image 编码 {encoding or '空'} 暂不支持；优先订阅 CompressedImage"
        )
    try:
        frame = np.frombuffer(data[: step * height], dtype=np.uint8).reshape(
            height, step
        )
        frame = frame[:, : width * channels]
        if channels == 1:
            image = frame.reshape(height, width)
        else:
            image = frame.reshape(height, width, channels)
        if encoding == "RGB8":
            image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        elif encoding == "RGBA8":
            image = cv2.cvtColor(image, cv2.COLOR_RGBA2BGR)
        elif encoding == "BGRA8":
            image = cv2.cvtColor(image, cv2.COLOR_BGRA2BGR)
        elif encoding in {"YUV422", "YUYV"}:
            image = cv2.cvtColor(image, cv2.COLOR_YUV2BGR_YUYV)
        elif encoding == "UYVY":
            image = cv2.cvtColor(image, cv2.COLOR_YUV2BGR_UYVY)
    except Exception as exc:
        return _image_error(
            f"原始像素转 OpenCV 图像失败：{exc.__class__.__name__}: {_truncate(exc, 160)}"
        )

    for scale in (1.0, 0.75, 0.5):
        if scale != 1.0:
            try:
                image = cv2.resize(
                    image,
                    (max(1, int(width * scale)), max(1, int(height * scale))),
                    interpolation=cv2.INTER_AREA,
                )
            except Exception as exc:
                return _image_error(f"图片缩放失败：{exc}")
        for quality in (80, 65, 50, 35):
            try:
                ok, encoded = cv2.imencode(
                    ".jpg", image, [int(cv2.IMWRITE_JPEG_QUALITY), quality]
                )
            except Exception as exc:
                return _image_error(f"JPEG 编码失败：{exc}")
            if not ok:
                continue
            raw = encoded.tobytes()
            if len(raw) <= max_image_bytes:
                return {
                    "available": True,
                    "reason": "",
                    "format": "jpeg",
                    "mime": "image/jpeg",
                    "width": int(image.shape[1]),
                    "height": int(image.shape[0]),
                    "bytes": len(raw),
                    "data_uri": "data:image/jpeg;base64,"
                    + base64.b64encode(raw).decode("ascii"),
                }
    return _image_error(
        f"JPEG 编码后仍超过上限 {max_image_bytes} 字节，已放弃这一帧"
    )


# ---------------------------------------------------------------------------
# 存储
# ---------------------------------------------------------------------------


@dataclass
class ChannelRuntime:
    spec: ChannelSpec
    available: bool = False
    status: str = "unavailable"
    reason: str = "尚未启动实时订阅。"
    channel_present: bool | None = None
    received_count: int = 0
    processed_count: int = 0
    error_count: int = 0
    dropped_by_fps: int = 0
    message: dict[str, Any] | None = None
    image: dict[str, Any] | None = None
    image_frame_seq: int = 0
    last_received_at: str = ""
    last_message_time: float | None = None
    last_message_time_iso: str = ""
    last_processed_monotonic: float = 0.0
    last_error: str = ""
    message_bytes: int | None = None
    recent_processed: deque[float] = field(
        default_factory=lambda: deque(maxlen=MAX_CHANNEL_EVENTS)
    )
    recent_received: deque[float] = field(
        default_factory=lambda: deque(maxlen=MAX_CHANNEL_EVENTS)
    )


def _compute_hz(samples: Iterable[float]) -> float | None:
    values = list(samples)
    if len(values) < 2:
        return None
    span = values[-1] - values[0]
    if span <= 0:
        return None
    return round((len(values) - 1) / span, 2)


def _message_timestamp(message: Any) -> tuple[float | None, str]:
    candidates: list[float] = []
    for attr in ("measurement_time", "timestamp", "start_timestamp"):
        try:
            value = float(getattr(message, attr, 0) or 0)
        except (TypeError, ValueError):
            continue
        if math.isfinite(value) and value > 0:
            candidates.append(value)
    header = getattr(message, "header", None)
    if header is not None:
        try:
            value = float(getattr(header, "timestamp_sec", 0) or 0)
        except (TypeError, ValueError):
            value = 0.0
        if math.isfinite(value) and value > 0:
            candidates.append(value)
    if not candidates:
        return None, ""
    value = max(candidates)
    try:
        iso = datetime.fromtimestamp(value).astimezone().isoformat(
            timespec="milliseconds"
        )
    except (OverflowError, OSError, ValueError):
        iso = ""
    return value, iso


def _message_byte_size(message: Any) -> int | None:
    for method_name in ("ByteSize",):
        method = getattr(message, method_name, None)
        if callable(method):
            try:
                return int(method())
            except Exception:
                pass
    try:
        return len(message.SerializeToString())
    except Exception:
        return None


def _get_path(mapping: Any, path: str) -> Any:
    current = mapping
    for part in path.split("."):
        if not isinstance(current, Mapping) or part not in current:
            return None
        current = current[part]
    return current


def _format_can_value(value: Any, spec: CanFieldSpec) -> str:
    if value is None:
        return "不可用"
    if spec.kind == "enum":
        english = str(value)
        chinese = spec.enum_labels.get(english, "")
        if chinese and chinese != english:
            return f"{chinese}（{english}）"
        return english
    if isinstance(value, bool):
        text = "是" if value else "否"
        return f"{text}（{'true' if value else 'false'}）"
    if isinstance(value, float):
        if not math.isfinite(value):
            return "不可用"
        text = f"{value:.3f}".rstrip("0").rstrip(".")
    else:
        text = str(value)
    if spec.unit:
        return f"{text} {spec.unit}"
    return text


def _can_value_is_missing(spec: CanFieldSpec, value: Any) -> bool:
    if value is None:
        return True
    key = spec.path.rsplit(".", 1)[-1]
    # Chassis.battery_soc_percentage 的 proto 默认值是 -1，表示车型没有提供；
    # 不能把它显示成“电池 -1%”。
    if key == "battery_soc_percentage":
        try:
            if float(value) < 0:
                return True
        except (TypeError, ValueError):
            pass
    if isinstance(value, float) and not math.isfinite(value):
        return True
    if isinstance(value, str) and not value.strip():
        return True
    return False


def _can_value_is_fault(spec: CanFieldSpec, value: Any) -> bool:
    key = spec.path.rsplit(".", 1)[-1]
    if value is None:
        return False
    if key == "error_code":
        return str(value) not in {"NO_ERROR", "0"}
    if key == "chassis_error_mask":
        return bool(value)
    if key.endswith("_online"):
        return value is False
    if key.endswith("_fail") or key.endswith("_fault"):
        return value is True
    if key in {
        "is_failure_status",
        "is_vehicle_standstill",
    }:
        return False
    if key.endswith("_valid"):
        return value is False
    return False


class RealtimeStore:
    """每个通道只保留最新一条消息/一帧图片，所有方法线程安全。"""

    def __init__(
        self,
        config: RealtimeConfig,
        raw_image_encoder: Callable[[Any, int], dict[str, Any]] | None = None,
    ) -> None:
        self.config = config
        self.limits = dict(config.limits)
        self.converter = ProtoJsonConverter(config.limits)
        self.raw_image_encoder = raw_image_encoder or encode_raw_image
        self._lock = threading.RLock()
        self._states: dict[str, ChannelRuntime] = {
            spec.id: ChannelRuntime(spec=spec) for spec in config.channels
        }
        self._version = 0

    @property
    def version(self) -> int:
        with self._lock:
            return self._version

    def bump_version(self) -> int:
        with self._lock:
            self._version += 1
            return self._version

    def channel_state(self, channel_id: str) -> ChannelRuntime | None:
        with self._lock:
            return self._states.get(channel_id)

    def set_status(
        self,
        channel_id: str,
        status: str,
        available: bool,
        reason: str = "",
        channel_present: bool | None = None,
    ) -> None:
        with self._lock:
            state = self._states.get(channel_id)
            if state is None:
                return
            changed = (
                state.status != status
                or state.available != available
                or state.reason != (reason or "")
                or (
                    channel_present is not None
                    and state.channel_present != channel_present
                )
            )
            state.status = status
            state.available = available
            state.reason = reason or ""
            if channel_present is not None:
                state.channel_present = channel_present
            if changed:
                self._version += 1

    def mark_reader_ready(
        self, channel_id: str, channel_present: bool | None = None
    ) -> None:
        if channel_present is False:
            self.set_status(
                channel_id,
                "unavailable",
                False,
                "Cyber 当前未发现这个通道。reader 已创建但收不到数据；"
                "请确认发布该通道的模块已启动、通道名正确、CYBER_DOMAIN_ID 一致。",
                channel_present=False,
            )
        elif channel_present is True:
            self.set_status(
                channel_id,
                "waiting",
                True,
                "已发现通道并创建只读 reader，尚未收到消息。",
                channel_present=True,
            )
        else:
            self.set_status(
                channel_id,
                "waiting",
                True,
                "只读 reader 已创建；未能查询通道列表，等待第一条消息。",
                channel_present=None,
            )

    def mark_reader_error(self, channel_id: str, reason: str) -> None:
        with self._lock:
            state = self._states.get(channel_id)
            if state is None:
                return
            state.error_count += 1
            state.last_error = reason
            state.status = "unavailable"
            state.available = False
            state.reason = reason
            self._version += 1

    def ingest(
        self,
        channel_id: str,
        message: Any,
        received_at: str | None = None,
        raw_size: int | None = None,
    ) -> bool:
        """处理一条消息。返回 True 表示已作为最新值保存。

        max_fps 节流时仍会累计 received_count，但不重新解码；这样既保留真实
        接收频率，又不会让大消息转换占满 CPU。
        """
        with self._lock:
            state = self._states.get(channel_id)
            if state is None:
                return False
            spec = state.spec
            mono = time.monotonic()
            wall = received_at or now_iso()
            state.received_count += 1
            state.last_received_at = wall
            state.recent_received.append(mono)
            min_interval = 1.0 / max(0.1, spec.max_fps)
            if (
                state.last_processed_monotonic
                and mono - state.last_processed_monotonic < min_interval
            ):
                state.dropped_by_fps += 1
                self._version += 1
                return False
            state.last_processed_monotonic = mono
            state.message_bytes = raw_size if raw_size is not None else _message_byte_size(message)

            try:
                if spec.kind == "protobuf":
                    data, error = self.converter.convert(message)
                    if error:
                        self._record_error(state, error)
                        self._version += 1
                        return False
                    state.message = data
                    state.image = None
                elif spec.kind == "compressed_image":
                    image = encode_compressed_image(
                        message, int(self.limits["max_image_bytes"])
                    )
                    state.message = {
                        "format": getattr(message, "format", ""),
                        "width": getattr(message, "width", None),
                        "height": getattr(message, "height", None),
                        "data": f"<bytes {len(getattr(message, 'data', b'') or b'')} 字节，已省略>",
                    }
                    self._apply_image(state, image)
                elif spec.kind == "image":
                    image = self.raw_image_encoder(
                        message, int(self.limits["max_image_bytes"])
                    )
                    state.message = {
                        "frame_id": getattr(message, "frame_id", ""),
                        "width": getattr(message, "width", None),
                        "height": getattr(message, "height", None),
                        "encoding": getattr(message, "encoding", ""),
                        "step": getattr(message, "step", None),
                        "data": f"<bytes {len(getattr(message, 'data', b'') or b'')} 字节，已省略>",
                    }
                    self._apply_image(state, image)
                else:
                    self._record_error(state, f"未知的通道类型：{spec.kind}")
                    self._version += 1
                    return False
            except Exception as exc:
                self._record_error(
                    state,
                    f"处理消息失败：{exc.__class__.__name__}: {_truncate(exc, 300)}",
                )
                self._version += 1
                return False

            state.processed_count += 1
            state.channel_present = True
            state.recent_processed.append(mono)
            state.last_message_time, state.last_message_time_iso = _message_timestamp(
                message
            )
            if state.image is not None and not state.image.get("available"):
                # 通道本身还在收消息，但这一帧不能给网页看；状态必须降级，
                # 否则页面会出现“正常”徽标配“没有画面”的矛盾。
                state.status = "degraded"
                state.available = True
                state.error_count += 1
                state.last_error = str(state.image.get("reason") or "图片处理失败")
                state.reason = f"最近一次图片处理失败：{state.last_error}"
            else:
                state.status = "ok"
                state.available = True
                state.reason = ""
                state.last_error = ""
            self._version += 1
            return True

    def _apply_image(self, state: ChannelRuntime, image: dict[str, Any]) -> None:
        if image.get("available"):
            state.image_frame_seq += 1
            state.image = {**image, "frame_seq": state.image_frame_seq}
        else:
            state.image = {
                **image,
                "frame_seq": state.image_frame_seq,
                "data_uri": "",
            }

    def _record_error(self, state: ChannelRuntime, reason: str) -> None:
        state.error_count += 1
        state.last_error = reason
        state.status = "degraded"
        state.available = True
        state.reason = reason
        if state.spec.kind in {"image", "compressed_image"}:
            state.image = {
                "available": False,
                "reason": reason,
                "format": "",
                "mime": "",
                "width": None,
                "height": None,
                "bytes": None,
                "data_uri": "",
                "frame_seq": state.image_frame_seq,
            }

    def _build_can(self) -> dict[str, Any]:
        can_config = self.config.can or {}
        chassis_id = str(can_config.get("chassis_channel_id") or "chassis")
        detail_id = str(can_config.get("detail_channel_id") or "chassis_detail")
        chassis_state = self._states.get(chassis_id)
        detail_state = self._states.get(detail_id)
        chassis_message = chassis_state.message if chassis_state else None
        detail_message = detail_state.message if detail_state else None
        available = bool(chassis_message)
        reason = ""
        if not available:
            if chassis_state is None:
                reason = f"配置里没有底盘通道 {chassis_id}。"
            else:
                reason = chassis_state.reason or "尚未收到 Chassis 消息。"
        fields: list[dict[str, Any]] = []
        faults: list[dict[str, Any]] = []
        missing: list[str] = []
        for spec in can_config.get("fields", []):
            source_message = chassis_message if spec.source == "chassis" else detail_message
            value = _get_path(source_message, spec.path)
            source_used = spec.source
            if (
                value is None
                and spec.fallback_source
                and spec.fallback_path
            ):
                fallback_message = (
                    chassis_message
                    if spec.fallback_source == "chassis"
                    else detail_message
                )
                value = _get_path(fallback_message, spec.fallback_path)
                if value is not None:
                    source_used = spec.fallback_source
            if _can_value_is_missing(spec, value):
                missing.append(spec.label)
                continue
            row = {
                "key": spec.path,
                "label": spec.label,
                "value": value,
                "display": _format_can_value(value, spec),
                "unit": spec.unit,
                "source": source_used,
                "enum_labels": dict(spec.enum_labels),
                "fault": _can_value_is_fault(spec, value),
            }
            fields.append(row)
            if row["fault"]:
                faults.append(
                    {
                        "key": spec.path,
                        "label": spec.label,
                        "value": value,
                        "display": row["display"],
                        "source": source_used,
                    }
                )
        return {
            "available": available,
            "reason": reason,
            "chassis_channel_id": chassis_id,
            "detail_channel_id": detail_id,
            "chassis_channel": (
                chassis_state.spec.channel if chassis_state is not None else chassis_id
            ),
            "detail_channel": (
                detail_state.spec.channel if detail_state is not None else detail_id
            ),
            "fields": fields,
            "faults": faults,
            "missing_fields": missing,
            "chassis": self._channel_summary(chassis_state),
            "chassis_detail": self._channel_summary(detail_state),
        }

    def _channel_summary(self, state: ChannelRuntime | None) -> dict[str, Any]:
        if state is None:
            return {
                "available": False,
                "status": "unavailable",
                "reason": "配置里没有这个通道。",
                "statistics": None,
            }
        return {
            "available": state.available,
            "status": state.status,
            "reason": state.reason,
            "channel": state.spec.channel,
            "statistics": self._statistics(state),
        }

    @staticmethod
    def _statistics(state: ChannelRuntime) -> dict[str, Any]:
        now = time.monotonic()
        while state.recent_processed and now - state.recent_processed[0] > 5.0:
            state.recent_processed.popleft()
        while state.recent_received and now - state.recent_received[0] > 5.0:
            state.recent_received.popleft()
        return {
            "received_count": state.received_count,
            "processed_count": state.processed_count,
            "error_count": state.error_count,
            "dropped_by_fps": state.dropped_by_fps,
            "sequence": state.processed_count,
            "hz": _compute_hz(state.recent_processed),
            "received_hz": _compute_hz(state.recent_received),
            "message_bytes": state.message_bytes,
            "last_received_at": state.last_received_at or None,
            "last_message_time": state.last_message_time,
            "last_message_time_iso": state.last_message_time_iso or None,
            "last_error": state.last_error,
        }

    def snapshot(
        self,
        include_images: bool = True,
        known_image_seqs: Mapping[str, int] | None = None,
    ) -> dict[str, Any]:
        with self._lock:
            known = dict(known_image_seqs or {})
            channels: list[dict[str, Any]] = []
            ready = 0
            with_message = 0
            with_image = 0
            errors = 0
            for state in self._states.values():
                if state.available:
                    ready += 1
                if state.message is not None:
                    with_message += 1
                if state.image and state.image.get("available"):
                    with_image += 1
                if state.error_count or state.status in {"degraded", "unavailable"}:
                    errors += 1
                image_out: dict[str, Any] | None = None
                if state.image is not None:
                    image_out = copy.deepcopy(state.image)
                    if (
                        include_images
                        and image_out.get("data_uri")
                        and known.get(state.spec.id) == state.image_frame_seq
                    ):
                        image_out["data_uri"] = ""
                        image_out["data_uri_omitted"] = True
                        image_out["reason"] = (
                            image_out.get("reason")
                            or "这一帧没有变化，SSE 本次未重复传输图片。"
                        )
                    elif not include_images:
                        image_out["data_uri"] = ""
                        image_out["data_uri_omitted"] = True
                channels.append(
                    {
                        "id": state.spec.id,
                        "label": state.spec.label,
                        "channel": state.spec.channel,
                        "kind": state.spec.kind,
                        "group": state.spec.group,
                        "enabled": state.spec.enabled,
                        "python_module": state.spec.python_module,
                        "message_class": state.spec.message_class,
                        "max_fps": state.spec.max_fps,
                        "site_confirm": state.spec.site_confirm,
                        "source": state.spec.source,
                        "available": state.available,
                        "status": state.status,
                        "reason": state.reason,
                        "channel_present": state.channel_present,
                        "statistics": self._statistics(state),
                        "message": copy.deepcopy(state.message),
                        "image": image_out,
                    }
                )
            can = self._build_can()
            enabled = sum(1 for spec in self.config.channels if spec.enabled)
            return {
                "timestamp": now_iso(),
                "source": "cyber_python_reader",
                "read_only": True,
                "version": self._version,
                "counts": {
                    "channels_total": len(self.config.channels),
                    "channels_enabled": enabled,
                    "channels_ready": ready,
                    "channels_with_message": with_message,
                    "channels_with_image": with_image,
                    "channels_error": errors,
                },
                "limits": dict(self.limits),
                "channels": channels,
                "can": can,
            }


# ---------------------------------------------------------------------------
# Cyber 管理器
# ---------------------------------------------------------------------------


def _unavailable_snapshot(reason: str) -> dict[str, Any]:
    return {
        "timestamp": now_iso(),
        "source": "cyber_python_reader",
        "status": "unavailable",
        "available": False,
        "read_only": True,
        "reason": reason,
        "version": 0,
        "cyber": {
            "available": False,
            "module": "",
            "node": "",
            "initialized": False,
            "channel_list_available": False,
            "channel_count": None,
            "reason": reason,
        },
        "counts": {
            "channels_total": 0,
            "channels_enabled": 0,
            "channels_ready": 0,
            "channels_with_message": 0,
            "channels_with_image": 0,
            "channels_error": 0,
        },
        "limits": dict(DEFAULT_LIMITS),
        "channels": [],
        "can": {
            "available": False,
            "reason": reason,
            "fields": [],
            "faults": [],
            "missing_fields": [],
            "chassis": {"available": False, "status": "unavailable", "reason": reason},
            "chassis_detail": {
                "available": False,
                "status": "unavailable",
                "reason": reason,
            },
        },
    }


class RealtimeManager:
    """只读 Cyber 订阅管理器。

    start() 在后台线程里导入 Cyber、初始化、创建 reader，然后 spin；
    stop() 只关闭本进程的 Cyber 环境，不碰 Apollo 的进程和模块。
    """

    def __init__(
        self,
        config_path: str | os.PathLike[str] | None = None,
        cyber_loader: Callable[[], tuple[Any, str]] | None = None,
        raw_image_encoder: Callable[[Any, int], dict[str, Any]] | None = None,
    ) -> None:
        self.config = load_realtime_config(config_path)
        self.store = RealtimeStore(
            self.config, raw_image_encoder=raw_image_encoder
        )
        self.cyber_loader = cyber_loader or self._default_cyber_loader
        self._lock = threading.RLock()
        self._thread: threading.Thread | None = None
        self._stop_event = threading.Event()
        self._cyber_module: Any = None
        self._node: Any = None
        self._shutdown_lock = threading.Lock()
        self._cyber_shutdown_done = False
        self._started_once = False
        self.status = "unavailable"
        self.available = False
        self.reason = self.config.error or "实时采集尚未启动。"
        self.cyber_error = self.config.error
        self.cyber_module_name = ""
        self.cyber_initialized = False
        self.channel_list_available = False
        self.channel_count: int | None = None
        self.started_at = ""
        self.stopped_at = ""

    # ------------------------------------------------------------------
    # 生命周期
    # ------------------------------------------------------------------
    @staticmethod
    def _default_cyber_loader() -> tuple[Any, str]:
        errors: list[str] = []
        for module_name in (
            "cyber.python.cyber_py3.cyber",
            "cyber_py3.cyber",
        ):
            try:
                module = importlib.import_module(module_name)
                return module, module_name
            except Exception as exc:
                errors.append(f"{module_name}: {exc.__class__.__name__}: {exc}")
        raise ImportError(
            "无法导入 Apollo Cyber Python API。需要与 Apollo 同一容器/环境，"
            "并让 PYTHONPATH 能找到 cyber_python 与 _cyber_wrapper。尝试结果："
            + "；".join(errors)
        )

    def _shutdown_cyber(self, cyber: Any) -> None:
        """只关闭本进程的 Cyber 环境，且保证并发 stop/异常路径不重复调用。"""
        with self._shutdown_lock:
            if self._cyber_shutdown_done:
                return
            self._cyber_shutdown_done = True
        try:
            cyber.shutdown()
        except Exception as exc:
            self.cyber_error = (
                f"关闭 Cyber 环境时出错：{exc.__class__.__name__}: {_truncate(exc, 200)}"
            )

    def start(self) -> None:
        with self._lock:
            if self._thread is not None and self._thread.is_alive():
                return
            if self._started_once:
                self.status = "unavailable"
                self.available = False
                self.reason = (
                    "实时采集已经停止过。Cyber 的 Python 环境不能在同一进程里安全重入，"
                    "请重启监控服务。"
                )
                return
            self._started_once = True
            self._stop_event.clear()
            self._cyber_shutdown_done = False
            self.status = "starting"
            self.available = False
            self.reason = "正在导入 Cyber 并创建只读 reader……"
            self.cyber_error = ""
            self._thread = threading.Thread(
                target=self._run,
                name="apollo-realtime-cyber",
                daemon=True,
            )
            self._thread.start()

    def stop(self, timeout: float = 3.0) -> None:
        self._stop_event.set()
        cyber = self._cyber_module
        if cyber is not None and self.cyber_initialized:
            self._shutdown_cyber(cyber)
        thread = self._thread
        if thread is not None and thread.is_alive():
            thread.join(timeout=max(0.1, float(timeout)))
        with self._lock:
            self._node = None
            self._cyber_module = None
            self.status = "unavailable"
            self.available = False
            if not self.reason.startswith("实时采集已经停止"):
                self.reason = "实时采集已停止；健康接口不受影响。"
            self.stopped_at = now_iso()
        self.store.bump_version()

    @property
    def running(self) -> bool:
        thread = self._thread
        return bool(thread is not None and thread.is_alive())

    def _set_cyber(
        self,
        *,
        module_name: str | None = None,
        initialized: bool | None = None,
        channel_list_available: bool | None = None,
        channel_count: int | None = None,
        error: str | None = None,
    ) -> None:
        with self._lock:
            if module_name is not None:
                self.cyber_module_name = module_name
            if initialized is not None:
                self.cyber_initialized = initialized
            if channel_list_available is not None:
                self.channel_list_available = channel_list_available
            if channel_count is not None:
                self.channel_count = channel_count
            if error is not None:
                self.cyber_error = error
        self.store.bump_version()

    def _run(self) -> None:
        if self.config.error:
            with self._lock:
                self.status = "unavailable"
                self.available = False
                self.reason = self.config.error
            self._stop_event.wait()
            return

        cyber_module: Any = None
        node: Any = None
        try:
            # Cyber 的 glog 在 init 时初始化；默认可能写 Apollo 日志目录。
            # 本服务不落盘，所以让 glog 输出到 stderr（由启动终端接收）。
            os.environ["GLOG_logtostderr"] = "1"
            cyber_module, module_name = self.cyber_loader()
            self._cyber_module = cyber_module
            self._set_cyber(module_name=module_name)
            try:
                init_result = cyber_module.init("apollo_realtime_monitor")
            except Exception as exc:
                raise RuntimeError(
                    f"Cyber init 失败：{exc.__class__.__name__}: {_truncate(exc, 300)}"
                ) from exc
            if init_result is False:
                raise RuntimeError(
                    "Cyber init 返回 False；可能没有可用的 Cyber 运行环境、"
                    "共享内存/IPC 权限不足，或 CYBER_DOMAIN_ID 不一致。"
                )
            self._set_cyber(initialized=True)
            if self._stop_event.is_set():
                self._shutdown_cyber(cyber_module)
                return

            try:
                node = cyber_module.Node("apollo_realtime_monitor")
            except Exception as exc:
                raise RuntimeError(
                    f"创建只读 Cyber Node 失败：{exc.__class__.__name__}: {_truncate(exc, 300)}"
                ) from exc
            self._node = node

            channel_names: set[str] | None = None
            channel_utils = getattr(cyber_module, "ChannelUtils", None)
            get_channels = getattr(channel_utils, "get_channels", None)
            if callable(get_channels):
                try:
                    # C++ 参数是 uint8_t，传整数秒，避免 Python 包装层类型错误。
                    raw_names = get_channels(1)
                    channel_names = {str(name) for name in (raw_names or [])}
                    self._set_cyber(
                        channel_list_available=True,
                        channel_count=len(channel_names),
                    )
                except Exception as exc:
                    self._set_cyber(
                        channel_list_available=False,
                        error=(
                            "查询 Cyber 通道列表失败，仍会创建 reader："
                            f"{exc.__class__.__name__}: {_truncate(exc, 240)}"
                        ),
                    )

            for spec in self.config.channels:
                if not spec.enabled:
                    self.store.set_status(
                        spec.id,
                        "unavailable",
                        False,
                        "配置里 enabled=false，未创建 reader。",
                    )
                    continue
                if self._stop_event.is_set():
                    return
                try:
                    module = importlib.import_module(spec.python_module)
                    message_class = getattr(module, spec.message_class)
                except Exception as exc:
                    self.store.mark_reader_error(
                        spec.id,
                        f"动态导入失败：{spec.python_module}.{spec.message_class}；"
                        f"{exc.__class__.__name__}: {_truncate(exc, 300)}。"
                        "需现场确认该 generated *_pb2 模块是否在 PYTHONPATH 中。",
                    )
                    continue
                callback = self._make_callback(spec)
                try:
                    reader = node.create_reader(
                        spec.channel, message_class, callback
                    )
                except Exception as exc:
                    self.store.mark_reader_error(
                        spec.id,
                        f"创建 reader 失败：{exc.__class__.__name__}: {_truncate(exc, 300)}",
                    )
                    continue
                if reader is None:
                    self.store.mark_reader_error(
                        spec.id,
                        "Cyber create_reader 返回 None；通道或消息类型可能不匹配。",
                    )
                    continue
                present: bool | None = None
                if channel_names is not None:
                    present = spec.channel in channel_names
                self.store.mark_reader_ready(spec.id, present)

            if self._stop_event.is_set():
                return

            with self._lock:
                self.status = "degraded"
                self.available = True
                self.reason = (
                    "实时订阅已建立，正在等待第一条消息。"
                    if self.store.snapshot(include_images=False)["counts"][
                        "channels_with_message"
                    ]
                    == 0
                    else "实时订阅已建立。"
                )
                self.started_at = now_iso()
            self.store.bump_version()

            # Node.spin() 内部只是循环等待 cyber shutdown。消息回调由 Cyber
            # 的 reader 线程触发，所以这里必须保持 spin 线程存活。
            while not self._stop_event.is_set():
                try:
                    node.spin()
                except Exception as exc:
                    if not self._stop_event.is_set():
                        raise RuntimeError(
                            f"Node.spin 异常：{exc.__class__.__name__}: {_truncate(exc, 300)}"
                        ) from exc
                if not self._stop_event.is_set():
                    # spin 意外返回时不要空转。
                    self._stop_event.wait(0.5)
        except Exception as exc:
            if cyber_module is not None and self.cyber_initialized:
                self._shutdown_cyber(cyber_module)
            reason = f"实时层不可用：{exc.__class__.__name__}: {_truncate(exc, 600)}"
            with self._lock:
                self.status = "unavailable"
                self.available = False
                self.reason = reason
                self.cyber_error = reason
            self._set_cyber(error=reason)
            for spec in self.config.channels:
                state = self.store.channel_state(spec.id)
                if state is None or state.received_count == 0:
                    self.store.set_status(
                        spec.id,
                        "unavailable",
                        False,
                        f"Cyber 未就绪：{reason}",
                    )
            self._stop_event.wait(1.0)
        finally:
            # 如果这是自己异常退出，保留错误原因；正常 stop 由 stop() 收尾。
            with self._lock:
                if self._stop_event.is_set():
                    self.available = False
                    self.status = "unavailable"

    def _make_callback(self, spec: ChannelSpec) -> Callable[[Any], None]:
        def callback(message: Any) -> None:
            try:
                raw_size = _message_byte_size(message)
                self.store.ingest(
                    spec.id,
                    message,
                    received_at=now_iso(),
                    raw_size=raw_size,
                )
            except Exception as exc:
                # 绝不能让异常穿过 ctypes 回调回到 Cyber 线程。
                try:
                    self.store.mark_reader_error(
                        spec.id,
                        f"reader 回调异常：{exc.__class__.__name__}: {_truncate(exc, 300)}",
                    )
                except Exception:
                    pass

        return callback

    # ------------------------------------------------------------------
    # 快照
    # ------------------------------------------------------------------
    def snapshot(
        self,
        include_images: bool = True,
        known_image_seqs: Mapping[str, int] | None = None,
    ) -> dict[str, Any]:
        base = self.store.snapshot(
            include_images=include_images, known_image_seqs=known_image_seqs
        )
        with self._lock:
            status = self.status
            available = self.available
            reason = self.reason
            cyber_info = {
                "available": bool(self.cyber_initialized and not self.cyber_error),
                "module": self.cyber_module_name,
                "node": "apollo_realtime_monitor" if self._node is not None else "",
                "initialized": bool(self.cyber_initialized),
                "channel_list_available": bool(self.channel_list_available),
                "channel_count": self.channel_count,
                "reason": self.cyber_error,
            }
            started_at = self.started_at
            stopped_at = self.stopped_at

        counts = base["counts"]
        if self.config.error:
            status = "unavailable"
            available = False
            reason = self.config.error
        elif available and not self.cyber_initialized:
            status = "starting"
        elif available:
            if counts["channels_ready"] == 0:
                status = "unavailable"
                available = False
                reason = (
                    "Cyber 已初始化，但没有任何可用的只读 reader 或通道；"
                    "请查看每个通道的 python_module、通道名和 Cyber 域。"
                )
            elif counts["channels_error"]:
                status = "degraded"
                if not reason or reason == "实时订阅已建立，正在等待第一条消息。":
                    reason = "部分通道不可用或尚未收到消息，请查看每个通道的原因。"
            elif counts["channels_with_message"] == 0:
                status = "degraded"
                reason = "实时订阅已建立，但还没有收到任何消息；请检查通道和 Cyber 域。"
            else:
                status = "ok"
                reason = "实时消息正在接收。"
        base.update(
            {
                "status": status,
                "available": available,
                "reason": reason,
                "cyber": cyber_info,
                "started_at": started_at or None,
                "stopped_at": stopped_at or None,
            }
        )
        return base


# 名字别名：需求里用 CyberSubscriberManager 指代同一个只读订阅管理器。
CyberSubscriberManager = RealtimeManager

# 给 monitor_server.py 的便捷别名（公开名字，避免导入私有函数）。
unavailable_snapshot = _unavailable_snapshot


__all__ = [
    "DEFAULT_REALTIME_CONFIG_PATH",
    "unavailable_snapshot",
    "ChannelSpec",
    "CyberSubscriberManager",
    "RealtimeConfig",
    "RealtimeManager",
    "RealtimeStore",
    "ProtoJsonConverter",
    "load_realtime_config",
    "validate_realtime_config",
    "proto_message_to_json",
    "encode_compressed_image",
    "encode_raw_image",
    "now_iso",
]
