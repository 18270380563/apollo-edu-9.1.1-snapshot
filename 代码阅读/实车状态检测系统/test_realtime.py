"""realtime_bridge.py 与 /api/realtime、/api/stream 的标准库测试。

不依赖真实 Cyber、真实 protobuf、OpenCV 或摄像头：
- 用假的 protobuf 描述符/对象测试字段、枚举和 bytes 过滤；
- 用假的 CompressedImage 测试 data URI；
- 用临时端口测试 JSON API 和 SSE；
- 用假的 Cyber loader 测试导入失败降级。

运行：python3 -B test_realtime.py
"""

from __future__ import annotations

import http.client
import json
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path
from typing import Any, Callable, Mapping
from unittest import mock

sys.dont_write_bytecode = True

from collector import Collector  # noqa: E402
from realtime_bridge import (  # noqa: E402
    DEFAULT_LIMITS,
    RealtimeManager,
    RealtimeStore,
    encode_compressed_image,
    load_realtime_config,
    proto_message_to_json,
    validate_realtime_config,
)
import monitor_server  # noqa: E402

BASE_DIR = Path(__file__).resolve().parent
REAL_REALTIME_CONFIG = BASE_DIR / "config" / "realtime_channels.json"


# ---------------------------------------------------------------------------
# 假 protobuf：只实现转换器需要的 ListFields / DESCRIPTOR
# ---------------------------------------------------------------------------


class FakeEnumValue:
    def __init__(self, name: str, number: int) -> None:
        self.name = name
        self.number = number


class FakeEnum:
    def __init__(self, name: str, values: Mapping[str, int]) -> None:
        self.name = name
        self.values_by_number = {
            number: FakeEnumValue(value_name, number)
            for value_name, number in values.items()
        }
        self.values = list(self.values_by_number.values())


class FakeOptions:
    map_entry = False


class FakeDescriptor:
    def __init__(self, name: str, fields: list["FakeField"] | None = None) -> None:
        self.name = name
        self.full_name = name
        self.fields = fields or []
        self.fields_by_name = {field.name: field for field in self.fields}

    def GetOptions(self) -> FakeOptions:
        return FakeOptions()


class FakeField:
    def __init__(
        self,
        name: str,
        field_type: int,
        label: int = 1,
        enum_type: FakeEnum | None = None,
        message_type: FakeDescriptor | None = None,
    ) -> None:
        self.name = name
        self.type = field_type
        self.label = label
        self.enum_type = enum_type
        self.message_type = message_type


class FakeMessage:
    def __init__(self, descriptor: FakeDescriptor, values: Mapping[str, Any]) -> None:
        self.DESCRIPTOR = descriptor
        self._values = dict(values)

    def __getattr__(self, name: str) -> Any:
        if name.startswith("_"):
            raise AttributeError(name)
        if name in self._values:
            return self._values[name]
        raise AttributeError(name)

    def ListFields(self) -> list[tuple[FakeField, Any]]:
        result = []
        for field in self.DESCRIPTOR.fields:
            if field.name in self._values and self._values[field.name] is not None:
                result.append((field, self._values[field.name]))
        return result

    def SerializeToString(self) -> bytes:
        return json.dumps(
            {key: str(value) for key, value in self._values.items()},
            ensure_ascii=False,
        ).encode("utf-8")

    def ByteSize(self) -> int:
        return len(self.SerializeToString())


DRIVING_MODE_ENUM = FakeEnum(
    "DrivingMode",
    {
        "COMPLETE_MANUAL": 0,
        "COMPLETE_AUTO_DRIVE": 1,
        "AUTO_STEER_ONLY": 2,
        "AUTO_SPEED_ONLY": 3,
        "EMERGENCY_MODE": 4,
    },
)
GEAR_ENUM = FakeEnum(
    "GearPosition",
    {
        "GEAR_NEUTRAL": 0,
        "GEAR_DRIVE": 1,
        "GEAR_REVERSE": 2,
        "GEAR_PARKING": 3,
        "GEAR_LOW": 4,
        "GEAR_INVALID": 5,
        "GEAR_NONE": 6,
    },
)
ERROR_ENUM = FakeEnum(
    "ErrorCode",
    {
        "NO_ERROR": 0,
        "CMD_NOT_IN_PERIOD": 1,
        "CHASSIS_ERROR": 2,
        "MANUAL_INTERVENTION": 3,
        "CHASSIS_CAN_NOT_IN_PERIOD": 4,
        "UNKNOWN_ERROR": 5,
    },
)

HEADER_DESC = FakeDescriptor(
    "apollo.common.Header",
    [
        FakeField("timestamp_sec", 1),
        FakeField("module_name", 9),
        FakeField("sequence_num", 13),
    ],
)
CHECK_RESPONSE_DESC = FakeDescriptor(
    "apollo.canbus.CheckResponse",
    [
        FakeField("is_eps_online", 8),
        FakeField("is_epb_online", 8),
        FakeField("is_esp_online", 8),
    ],
)
CHASSIS_DESC = FakeDescriptor(
    "apollo.canbus.Chassis",
    [
        FakeField("engine_started", 8),
        FakeField("engine_rpm", 2),
        FakeField("speed_mps", 2),
        FakeField("odometer_m", 2),
        FakeField("throttle_percentage", 2),
        FakeField("brake_percentage", 2),
        FakeField("steering_percentage", 2),
        FakeField("steering_torque_nm", 2),
        FakeField("parking_brake", 8),
        FakeField("driving_mode", 14, enum_type=DRIVING_MODE_ENUM),
        FakeField("error_code", 14, enum_type=ERROR_ENUM),
        FakeField("gear_location", 14, enum_type=GEAR_ENUM),
        FakeField("header", 11, message_type=HEADER_DESC),
        FakeField("frame_id", 9),
        FakeField("chassis_error_mask", 5),
        FakeField("battery_soc_percentage", 5),
        FakeField("check_response", 11, message_type=CHECK_RESPONSE_DESC),
        FakeField("data", 12),
    ],
)
BATTERY_DESC = FakeDescriptor(
    "apollo.canbus.Battery",
    [FakeField("battery_percent", 1), FakeField("fuel_level", 1)],
)
DETAIL_CHECK_DESC = FakeDescriptor(
    "apollo.canbus.CheckResponseSignal",
    [
        FakeField("is_eps_online", 8),
        FakeField("is_epb_online", 8),
        FakeField("is_esp_online", 8),
        FakeField("is_vcu_online", 8),
    ],
)
CHASSIS_DETAIL_DESC = FakeDescriptor(
    "apollo.canbus.ChassisDetail",
    [
        FakeField("battery", 11, message_type=BATTERY_DESC),
        FakeField("check_response", 11, message_type=DETAIL_CHECK_DESC),
    ],
)
COMPRESSED_DESC = FakeDescriptor(
    "apollo.drivers.CompressedImage",
    [
        FakeField("format", 9),
        FakeField("data", 12),
        FakeField("width", 13),
        FakeField("height", 13),
    ],
)
IMAGE_DESC = FakeDescriptor(
    "apollo.drivers.Image",
    [
        FakeField("frame_id", 9),
        FakeField("width", 13),
        FakeField("height", 13),
        FakeField("encoding", 9),
        FakeField("step", 13),
        FakeField("data", 12),
    ],
)


def make_header(sequence: int = 1) -> FakeMessage:
    return FakeMessage(
        HEADER_DESC,
        {
            "timestamp_sec": 1700000000.0 + sequence,
            "module_name": "test",
            "sequence_num": sequence,
        },
    )


def make_chassis(
    speed: float = 1.5,
    *,
    error: int = 0,
    driving_mode: int = 1,
    gear: int = 1,
    sequence: int = 1,
    engine_rpm: float = 900.0,
    include_detail_check: bool = True,
) -> FakeMessage:
    check = FakeMessage(
        CHECK_RESPONSE_DESC,
        {"is_eps_online": True, "is_epb_online": True, "is_esp_online": True},
    )
    return FakeMessage(
        CHASSIS_DESC,
        {
            "header": make_header(sequence),
            "engine_started": True,
            "engine_rpm": engine_rpm,
            "speed_mps": speed,
            "odometer_m": 1234.5,
            "throttle_percentage": 12.5,
            "brake_percentage": 0.0,
            "steering_percentage": -3.25,
            "steering_torque_nm": 1.2,
            "parking_brake": False,
            "driving_mode": driving_mode,
            "error_code": error,
            "gear_location": gear,
            "chassis_error_mask": 0,
            "battery_soc_percentage": 88,
            "check_response": check,
            "data": b"raw-can-bytes-not-for-json",
        },
    )


def make_chassis_detail(battery: float = 77.7, eps_online: bool = True) -> FakeMessage:
    return FakeMessage(
        CHASSIS_DETAIL_DESC,
        {
            "battery": FakeMessage(BATTERY_DESC, {"battery_percent": battery}),
            "check_response": FakeMessage(
                DETAIL_CHECK_DESC,
                {
                    "is_eps_online": eps_online,
                    "is_epb_online": True,
                    "is_esp_online": True,
                    "is_vcu_online": True,
                },
            ),
        },
    )


def make_compressed(
    data: bytes,
    *,
    fmt: str = "jpeg",
    width: int = 640,
    height: int = 480,
) -> FakeMessage:
    return FakeMessage(
        COMPRESSED_DESC,
        {"format": fmt, "data": data, "width": width, "height": height},
    )


def make_image() -> FakeMessage:
    return FakeMessage(
        IMAGE_DESC,
        {
            "frame_id": "camera_test",
            "width": 0,
            "height": 0,
            "encoding": "RGB8",
            "step": 0,
            "data": b"",
        },
    )


# ---------------------------------------------------------------------------
# 配置和服务器辅助
# ---------------------------------------------------------------------------


def default_channels() -> list[dict[str, Any]]:
    return [
        {
            "id": "chassis",
            "label": "底盘 Chassis",
            "channel": "/apollo/canbus/chassis",
            "kind": "protobuf",
            "python_module": "modules.common_msgs.chassis_msgs.chassis_pb2",
            "message_class": "Chassis",
            "enabled": True,
            "max_fps": 100000,
            "group": "can",
            "site_confirm": "需现场确认：测试用假消息。",
        },
        {
            "id": "chassis_detail",
            "label": "底盘明细 ChassisDetail",
            "channel": "/apollo/canbus/chassis_detail",
            "kind": "protobuf",
            "python_module": "modules.common_msgs.chassis_msgs.chassis_detail_pb2",
            "message_class": "ChassisDetail",
            "enabled": True,
            "max_fps": 100000,
            "group": "can",
            "site_confirm": "需现场确认：测试用假消息。",
        },
        {
            "id": "camera_test",
            "label": "测试摄像头",
            "channel": "/apollo/sensor/camera/test/image/compressed",
            "kind": "compressed_image",
            "python_module": "modules.common_msgs.sensor_msgs.sensor_image_pb2",
            "message_class": "CompressedImage",
            "enabled": True,
            "max_fps": 100000,
            "group": "camera",
            "site_confirm": "需现场确认：测试用假消息。",
        },
    ]


def default_can_fields() -> list[dict[str, Any]]:
    return [
        {"source": "chassis", "path": "speed_mps", "label": "车速", "unit": "m/s", "kind": "float"},
        {
            "source": "chassis",
            "path": "driving_mode",
            "label": "驾驶模式",
            "kind": "enum",
            "enum_labels": {
                "COMPLETE_MANUAL": "完全手动",
                "COMPLETE_AUTO_DRIVE": "完全自动驾驶",
            },
        },
        {
            "source": "chassis",
            "path": "gear_location",
            "label": "挡位",
            "kind": "enum",
            "enum_labels": {"GEAR_DRIVE": "前进挡"},
        },
        {
            "source": "chassis",
            "path": "error_code",
            "label": "底盘故障码",
            "kind": "enum",
            "enum_labels": {"NO_ERROR": "无故障", "CHASSIS_ERROR": "底盘故障"},
        },
        {
            "source": "detail",
            "path": "battery.battery_percent",
            "label": "电池百分比",
            "unit": "%",
            "kind": "float",
        },
        {
            "source": "detail",
            "path": "check_response.is_eps_online",
            "label": "EPS 在线",
            "kind": "bool",
            "fallback_source": "chassis",
            "fallback_path": "check_response.is_eps_online",
        },
    ]


def write_config(
    path: Path,
    *,
    channels: list[dict[str, Any]] | None = None,
    can_fields: list[dict[str, Any]] | None = None,
    limits: Mapping[str, Any] | None = None,
) -> None:
    path.write_text(
        json.dumps(
            {
                "schema_version": 1,
                "limits": dict(limits or DEFAULT_LIMITS),
                "can": {
                    "chassis_channel_id": "chassis",
                    "detail_channel_id": "chassis_detail",
                    "fields": can_fields if can_fields is not None else default_can_fields(),
                },
                "channels": channels if channels is not None else default_channels(),
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )


def make_store(
    tmp: Path,
    *,
    limits: Mapping[str, Any] | None = None,
    channels: list[dict[str, Any]] | None = None,
    raw_image_encoder: Callable[[Any, int], dict[str, Any]] | None = None,
) -> RealtimeStore:
    config_path = tmp / "realtime.json"
    write_config(config_path, channels=channels, limits=limits)
    return RealtimeStore(
        load_realtime_config(config_path),
        raw_image_encoder=raw_image_encoder,
    )


def wait_until(predicate: Callable[[], bool], timeout: float = 3.0) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.02)
    return False


def make_collector(tmp: Path) -> Collector:
    config = tmp / "modules.json"
    config.write_text(
        json.dumps(
            {
                "schema_version": 1,
                "apollo": {},
                "modules": [
                    {
                        "id": "test_binary",
                        "name": "测试二进制",
                        "description": "HTTP 测试用",
                        "process_keywords": ["apollo-test-binary-not-running"],
                        "suggested_channels": [],
                        "binary": True,
                    }
                ],
            },
            ensure_ascii=False,
        ),
        encoding="utf-8",
    )
    return Collector(
        config_path=config,
        apollo_root=tmp / "missing_apollo",
        proc_root=tmp / "missing_proc",
    )


class HTTPServerHarness:
    def __init__(
        self,
        realtime: RealtimeManager | None = None,
        *,
        heartbeat: float = 0.2,
    ) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        tmp = Path(self.tmp.name)
        self.collector = make_collector(tmp)
        self.server = monitor_server.MonitorHTTPServer(
            ("127.0.0.1", 0),
            collector=self.collector,
            web_dir=monitor_server.WEB_DIR,
            cache_ttl=0,
            realtime=realtime,
            sse_heartbeat=heartbeat,
            sse_poll=0.05,
            sse_min_interval=0,
        )
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.port = self.server.server_address[1]

    def request(self, path: str, method: str = "GET") -> tuple[int, str, bytes]:
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        try:
            connection.request(method, path)
            response = connection.getresponse()
            body = response.read()
            return response.status, response.getheader("Content-Type", ""), body
        finally:
            connection.close()

    def close(self) -> None:
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=3)
        self.tmp.cleanup()


# ---------------------------------------------------------------------------
# 测试
# ---------------------------------------------------------------------------


class ConfigTests(unittest.TestCase):
    def test_missing_and_broken_config_are_unavailable(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            missing = load_realtime_config(tmp / "missing.json")
            self.assertTrue(missing.error)
            self.assertIn("不存在", missing.error)
            self.assertEqual(validate_realtime_config(tmp / "missing.json"), [missing.error])

            broken = tmp / "broken.json"
            broken.write_text("{not json", encoding="utf-8")
            parsed = load_realtime_config(broken)
            self.assertTrue(parsed.error)
            self.assertIn("读取实时配置失败", parsed.error)

    def test_bad_channel_config_reports_reason(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            path = tmp / "bad.json"
            write_config(
                path,
                channels=[
                    {
                        "id": "x",
                        "label": "x",
                        "channel": "no-leading-slash",
                        "kind": "protobuf",
                        "python_module": "",
                        "message_class": "",
                    }
                ],
            )
            errors = validate_realtime_config(path)
            self.assertTrue(errors)
            self.assertIn("channel 必须以 / 开头", errors[0])

    def test_duplicate_channel_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            path = tmp / "duplicate.json"
            channels = default_channels()
            channels.append(dict(channels[0]))
            write_config(path, channels=channels)
            errors = validate_realtime_config(path)
            self.assertTrue(any("id 重复" in item for item in errors))

    def test_real_config_has_required_channels_and_fields(self) -> None:
        errors = validate_realtime_config(REAL_REALTIME_CONFIG)
        self.assertEqual(errors, [])
        config = load_realtime_config(REAL_REALTIME_CONFIG)
        names = {item.channel for item in config.channels}
        required = {
            "/apollo/canbus/chassis",
            "/apollo/canbus/chassis_detail",
            "/apollo/sensor/camera/front_6mm/image/compressed",
            "/apollo/sensor/camera/front_12mm/image/compressed",
            "/apollo/sensor/camera/rear_6mm/image/compressed",
            "/apollo/sensor/camera/left_fisheye/image/compressed",
            "/apollo/sensor/camera/right_fisheye/image/compressed",
            "/apollo/sensor/camera/traffic/image_long/compressed",
        }
        self.assertTrue(required.issubset(names), required - names)
        self.assertGreaterEqual(len(config.can.get("fields", [])), 20)
        self.assertTrue(all(item.site_confirm for item in config.channels))
        self.assertTrue(all(item.python_module and item.message_class for item in config.channels))


class ConverterTests(unittest.TestCase):
    def test_enum_names_bytes_filter_and_nan(self) -> None:
        message = make_chassis(speed=1.5, error=2, driving_mode=1, gear=1)
        # 再加一个 NaN，确认不会生成非法 JSON。
        message._values["steering_percentage"] = float("nan")
        payload, error = proto_message_to_json(message)
        self.assertEqual(error, "")
        self.assertEqual(payload["speed_mps"], 1.5)
        self.assertEqual(payload["driving_mode"], "COMPLETE_AUTO_DRIVE")
        self.assertEqual(payload["gear_location"], "GEAR_DRIVE")
        self.assertEqual(payload["error_code"], "CHASSIS_ERROR")
        self.assertTrue(str(payload["data"]).startswith("<bytes"))
        self.assertIsNone(payload["steering_percentage"])
        json.dumps(payload, ensure_ascii=False, allow_nan=False)

    def test_depth_items_and_json_size_limits(self) -> None:
        node_desc = FakeDescriptor("Fake.Node", [])
        node_desc.fields = [
            FakeField("level", 5),
            FakeField("child", 11, message_type=node_desc),
            FakeField("values", 5, label=3),
        ]
        node_desc.fields_by_name = {item.name: item for item in node_desc.fields}
        node = FakeMessage(node_desc, {"level": 0, "values": list(range(50))})
        current = node
        for level in range(1, 8):
            child = FakeMessage(node_desc, {"level": level, "values": []})
            current._values["child"] = child
            current = child
        payload, error = proto_message_to_json(
            node,
            {"max_depth": 3, "max_items": 5, "max_json_bytes": 4096},
        )
        self.assertEqual(error, "")
        self.assertLessEqual(len(payload["values"]), 6)
        self.assertIn("已省略", payload["values"][-1])
        self.assertIn("__truncated__", payload["child"]["child"]["child"])
        json.dumps(payload, ensure_ascii=False, allow_nan=False)

    def test_json_size_cap_returns_compact_payload(self) -> None:
        message = make_chassis(speed=1.0)
        message._values["frame_id"] = "x" * 5000
        payload, error = proto_message_to_json(
            message,
            {
                "max_depth": 8,
                "max_items": 20,
                "max_json_bytes": 4096,
                "max_string_chars": 5000,
            },
        )
        self.assertEqual(error, "")
        self.assertLessEqual(
            len(json.dumps(payload, ensure_ascii=False, allow_nan=False).encode("utf-8")),
            4096,
        )
        self.assertTrue(payload.get("__truncated__"))


class StoreTests(unittest.TestCase):
    def test_latest_value_and_can_decoding(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(tmp)
            self.assertTrue(store.ingest("chassis", make_chassis(1.0, sequence=1)))
            time.sleep(0.02)
            self.assertTrue(store.ingest("chassis", make_chassis(2.5, error=2, sequence=2)))
            self.assertTrue(store.ingest("chassis_detail", make_chassis_detail(66.6)))
            snapshot = store.snapshot()
            channel = next(item for item in snapshot["channels"] if item["id"] == "chassis")
            self.assertEqual(channel["statistics"]["received_count"], 2)
            self.assertEqual(channel["statistics"]["sequence"], 2)
            self.assertEqual(channel["message"]["speed_mps"], 2.5)
            self.assertEqual(len(snapshot["channels"]), 3)  # 只有最新值，不存历史
            self.assertTrue(snapshot["can"]["available"])
            fields = {item["key"]: item for item in snapshot["can"]["fields"]}
            self.assertEqual(fields["speed_mps"]["value"], 2.5)
            self.assertIn("完全自动驾驶", fields["driving_mode"]["display"])
            self.assertIn("底盘故障", fields["error_code"]["display"])
            self.assertEqual(fields["battery.battery_percent"]["value"], 66.6)
            self.assertTrue(any(item["key"] == "error_code" for item in snapshot["can"]["faults"]))

    def test_compressed_image_data_uri_and_size_limit(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(tmp)
            image_bytes = b"\xff\xd8\xff\xe0" + b"fake-jpeg-body" + b"\xff\xd9"
            self.assertTrue(store.ingest("camera_test", make_compressed(image_bytes)))
            snapshot = store.snapshot()
            camera = next(item for item in snapshot["channels"] if item["id"] == "camera_test")
            self.assertTrue(camera["image"]["available"])
            self.assertTrue(camera["image"]["data_uri"].startswith("data:image/jpeg;base64,"))
            self.assertEqual(camera["image"]["width"], 640)
            self.assertEqual(camera["image"]["height"], 480)
            self.assertIn("<bytes", camera["message"]["data"])

    def test_compressed_image_over_limit_has_reason(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(
                tmp,
                limits={**DEFAULT_LIMITS, "max_image_bytes": 1024},
            )
            huge = b"\xff\xd8" + b"x" * 2048 + b"\xff\xd9"
            self.assertTrue(store.ingest("camera_test", make_compressed(huge)))
            camera = next(
                item for item in store.snapshot()["channels"] if item["id"] == "camera_test"
            )
            self.assertFalse(camera["image"]["available"])
            self.assertEqual(camera["status"], "degraded")
            self.assertIn("超过上限", camera["image"]["reason"])
            self.assertFalse(camera["image"]["data_uri"])

    def test_raw_image_without_valid_size_returns_reason(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            channels = default_channels()
            channels[2] = {
                **channels[2],
                "id": "image_test",
                "label": "原始 Image 测试",
                "channel": "/apollo/sensor/camera/test/image",
                "kind": "image",
            }
            store = make_store(tmp, channels=channels)
            self.assertTrue(store.ingest("image_test", make_image()))
            channel = next(
                item for item in store.snapshot()["channels"] if item["id"] == "image_test"
            )
            self.assertFalse(channel["image"]["available"])
            self.assertEqual(channel["status"], "degraded")
            self.assertTrue(channel["image"]["reason"])
            self.assertFalse(channel["image"]["data_uri"])

    def test_sse_image_omission_keeps_previous_on_client(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(tmp)
            data = b"\xff\xd8\xff\xe0" + b"x" * 20 + b"\xff\xd9"
            store.ingest("camera_test", make_compressed(data))
            first = store.snapshot()
            camera = next(item for item in first["channels"] if item["id"] == "camera_test")
            seq = camera["image"]["frame_seq"]
            self.assertTrue(camera["image"]["data_uri"])
            second = store.snapshot(known_image_seqs={"camera_test": seq})
            camera2 = next(item for item in second["channels"] if item["id"] == "camera_test")
            self.assertEqual(camera2["image"]["data_uri"], "")
            self.assertTrue(camera2["image"]["data_uri_omitted"])

    def test_thread_safe_latest_only(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(tmp)
            errors: list[BaseException] = []

            def worker(worker_id: int) -> None:
                try:
                    for index in range(50):
                        store.ingest(
                            "chassis",
                            make_chassis(
                                speed=float(worker_id * 100 + index),
                                sequence=worker_id * 100 + index,
                            ),
                        )
                except BaseException as exc:  # 测试线程里的异常要带回主线程
                    errors.append(exc)

            threads = [threading.Thread(target=worker, args=(index,)) for index in range(8)]
            for thread in threads:
                thread.start()
            for thread in threads:
                thread.join(timeout=5)
            self.assertFalse(errors)
            snapshot = store.snapshot()
            channel = next(item for item in snapshot["channels"] if item["id"] == "chassis")
            self.assertEqual(channel["statistics"]["received_count"], 400)
            stats = channel["statistics"]
            self.assertEqual(stats["processed_count"] + stats["dropped_by_fps"], 400)
            self.assertGreater(stats["processed_count"], 0)
            self.assertIsNotNone(channel["message"])
            self.assertIsInstance(channel["message"]["speed_mps"], (int, float))
            self.assertEqual(len(snapshot["channels"]), 3)


class ManagerTests(unittest.TestCase):
    def test_cyber_import_failure_degrades(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            config_path = tmp / "realtime.json"
            write_config(config_path)

            def broken_loader() -> tuple[Any, str]:
                raise ImportError("测试：没有 Cyber")

            manager = RealtimeManager(config_path, cyber_loader=broken_loader)
            manager.start()
            try:
                self.assertTrue(
                    wait_until(
                        lambda: manager.snapshot()["status"] == "unavailable",
                        3.0,
                    )
                )
                snapshot = manager.snapshot()
                self.assertEqual(snapshot["status"], "unavailable")
                self.assertFalse(snapshot["available"])
                self.assertIn("没有 Cyber", snapshot["reason"])
                self.assertTrue(snapshot["channels"])
                self.assertTrue(all(not item["available"] for item in snapshot["channels"]))
                json.dumps(snapshot, ensure_ascii=False, allow_nan=False)
            finally:
                manager.stop()

    def test_fake_cyber_reader_and_callback(self) -> None:
        class FakeNode:
            def __init__(self) -> None:
                self.readers: dict[str, Callable[[Any], None]] = {}

            def create_reader(
                self,
                channel: str,
                message_class: Any,
                callback: Callable[[Any], None],
            ) -> object:
                self.readers[channel] = callback
                self.message_class = message_class
                return object()

            def spin(self) -> None:
                time.sleep(0.02)

        class FakeCyber:
            def __init__(self) -> None:
                self.node = FakeNode()
                self.shutdown_called = False

            class ChannelUtils:
                @staticmethod
                def get_channels(sleep_s: int) -> list[str]:
                    return ["/apollo/canbus/chassis"]

            def init(self, module_name: str) -> bool:
                self.module_name = module_name
                return True

            def Node(self, name: str) -> FakeNode:
                self.node_name = name
                return self.node

            def shutdown(self) -> None:
                self.shutdown_called = True

        fake_cyber = FakeCyber()
        fake_pb2 = type("FakePb2", (), {"Chassis": FakeMessage})

        def fake_import(name: str) -> Any:
            if name == "modules.common_msgs.chassis_msgs.chassis_pb2":
                return fake_pb2
            raise ImportError(name)

        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            path = tmp / "realtime.json"
            write_config(path, channels=[default_channels()[0]])
            manager = RealtimeManager(
                path,
                cyber_loader=lambda: (fake_cyber, "fake.cyber"),
            )
            with mock.patch("realtime_bridge.importlib.import_module", side_effect=fake_import):
                manager.start()
                try:
                    self.assertTrue(
                        wait_until(
                            lambda: manager.snapshot()["counts"]["channels_ready"] == 1,
                            3.0,
                        )
                    )
                    snapshot = manager.snapshot()
                    self.assertTrue(snapshot["available"])
                    self.assertTrue(snapshot["cyber"]["initialized"])
                    channel = snapshot["channels"][0]
                    self.assertTrue(channel["available"])
                    self.assertEqual(channel["status"], "waiting")
                    callback = fake_cyber.node.readers["/apollo/canbus/chassis"]
                    callback(make_chassis(4.5))
                    self.assertTrue(
                        wait_until(
                            lambda: manager.snapshot()["counts"]["channels_with_message"] == 1,
                            2.0,
                        )
                    )
                    snapshot = manager.snapshot()
                    self.assertEqual(snapshot["channels"][0]["message"]["speed_mps"], 4.5)
                    self.assertIn("车速", [item["label"] for item in snapshot["can"]["fields"]])
                finally:
                    manager.stop()
            self.assertTrue(fake_cyber.shutdown_called)
            self.assertFalse(manager.running)

    def test_bad_config_snapshot_is_unavailable(self) -> None:
        manager = RealtimeManager("/no/such/realtime.json")
        manager.start()
        try:
            snapshot = manager.snapshot()
            self.assertFalse(snapshot["available"])
            self.assertEqual(snapshot["status"], "unavailable")
            self.assertIn("不存在", snapshot["reason"])
        finally:
            manager.stop()


class APITests(unittest.TestCase):
    def _unavailable_manager(self, tmp: Path) -> RealtimeManager:
        config_path = tmp / "realtime.json"
        write_config(config_path)

        def broken_loader() -> tuple[Any, str]:
            raise ImportError("测试：Cyber 不可用")

        manager = RealtimeManager(config_path, cyber_loader=broken_loader)
        manager.start()
        self.assertTrue(wait_until(lambda: manager.snapshot()["status"] == "unavailable"))
        return manager

    def test_all_endpoints_json_and_realtime_unavailable(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            manager = self._unavailable_manager(Path(raw_tmp))
            harness = HTTPServerHarness(realtime=manager)
            try:
                status, content_type, _ = harness.request("/")
                self.assertEqual(status, 200)
                self.assertIn("text/html", content_type)

                for path in (
                    "/api/summary",
                    "/api/modules",
                    "/api/channels",
                    "/api/processes",
                    "/api/libraries",
                    "/api/config",
                    "/api/nodes",
                    "/api/realtime",
                ):
                    status, content_type, body = harness.request(path)
                    self.assertEqual(status, 200, path)
                    self.assertIn("application/json", content_type, path)
                    payload = json.loads(body.decode("utf-8"))
                    self.assertIsInstance(payload, dict, path)
                    self.assertIn("status", payload, path)
                    self.assertIn("available", payload, path)
                realtime = json.loads(harness.request("/api/realtime")[2].decode("utf-8"))
                self.assertFalse(realtime["available"])
                self.assertIn("Cyber", realtime["reason"])
                self.assertEqual(realtime["channels"][0]["status"], "unavailable")

                status, _, _ = harness.request("/api/realtime", method="POST")
                self.assertEqual(status, 405)
            finally:
                harness.close()
                manager.stop()

    def test_realtime_api_contains_latest_can_and_image(self) -> None:
        class StaticRealtime:
            def __init__(self, store: RealtimeStore) -> None:
                self.store = store

            def snapshot(
                self,
                include_images: bool = True,
                known_image_seqs: Mapping[str, int] | None = None,
            ) -> dict[str, Any]:
                payload = self.store.snapshot(
                    include_images=include_images,
                    known_image_seqs=known_image_seqs,
                )
                payload.update(
                    {
                        "status": "ok",
                        "available": True,
                        "reason": "测试快照",
                        "cyber": {
                            "available": True,
                            "initialized": True,
                            "module": "fake",
                            "node": "fake",
                            "channel_list_available": True,
                            "channel_count": 3,
                            "reason": "",
                        },
                    }
                )
                return payload

        with tempfile.TemporaryDirectory() as raw_tmp:
            tmp = Path(raw_tmp)
            store = make_store(tmp)
            store.ingest("chassis", make_chassis(3.25, error=2))
            time.sleep(0.02)
            store.ingest("chassis_detail", make_chassis_detail(55.5))
            image_bytes = b"\xff\xd8\xff\xe0" + b"x" * 30 + b"\xff\xd9"
            store.ingest("camera_test", make_compressed(image_bytes))
            harness = HTTPServerHarness(realtime=StaticRealtime(store))  # type: ignore[arg-type]
            try:
                status, content_type, body = harness.request("/api/realtime")
                self.assertEqual(status, 200)
                self.assertIn("application/json", content_type)
                payload = json.loads(body.decode("utf-8"))
                self.assertTrue(payload["available"])
                self.assertTrue(payload["can"]["available"])
                fields = {item["key"]: item for item in payload["can"]["fields"]}
                self.assertEqual(fields["speed_mps"]["value"], 3.25)
                camera = next(
                    item for item in payload["channels"] if item["id"] == "camera_test"
                )
                self.assertTrue(
                    camera["image"]["data_uri"].startswith("data:image/jpeg;base64,")
                )
                self.assertEqual(camera["image"]["width"], 640)
            finally:
                harness.close()

    def test_sse_returns_valid_event_and_releases_thread(self) -> None:
        with tempfile.TemporaryDirectory() as raw_tmp:
            manager = self._unavailable_manager(Path(raw_tmp))
            harness = HTTPServerHarness(realtime=manager, heartbeat=0.2)
            connection = http.client.HTTPConnection("127.0.0.1", harness.port, timeout=5)
            try:
                connection.request("GET", "/api/stream", headers={"Accept": "text/event-stream"})
                response = connection.getresponse()
                self.assertEqual(response.status, 200)
                self.assertIn("text/event-stream", response.getheader("Content-Type", ""))
                raw = b""
                deadline = time.monotonic() + 3
                while (
                    (b"event: hello" not in raw or b"event: unavailable" not in raw)
                    and time.monotonic() < deadline
                ):
                    chunk = response.read1(8192)
                    if not chunk:
                        break
                    raw += chunk
                text = raw.decode("utf-8", "replace")
                self.assertIn("event: hello", text)
                self.assertIn("event: unavailable", text)
                data_lines = [
                    line[6:]
                    for line in text.splitlines()
                    if line.startswith("data: ")
                ]
                self.assertTrue(data_lines)
                payload = json.loads(data_lines[0])
                self.assertFalse(payload["available"])
                self.assertIn("reason", payload)
                self.assertTrue(harness.server.active_sse >= 1)
            finally:
                try:
                    response.close()
                except Exception:
                    pass
                connection.close()
                # 只靠客户端断开，不主动停服务；心跳写入应发现断连并释放线程。
                self.assertTrue(wait_until(lambda: harness.server.active_sse == 0, 3.0))
                harness.close()
                manager.stop()

    def test_sse_without_manager_returns_unavailable_event(self) -> None:
        harness = HTTPServerHarness(realtime=None, heartbeat=0.2)
        connection = http.client.HTTPConnection("127.0.0.1", harness.port, timeout=5)
        try:
            connection.request("GET", "/api/stream")
            response = connection.getresponse()
            self.assertEqual(response.status, 200)
            raw = b""
            deadline = time.monotonic() + 3
            while b"event: unavailable" not in raw and time.monotonic() < deadline:
                chunk = response.read1(8192)
                if not chunk:
                    break
                raw += chunk
            self.assertIn(b"event: unavailable", raw)
            self.assertIn(b"RealtimeManager", raw)
        finally:
            connection.close()
            harness.server.stop_event.set()
            harness.close()


class SourceBoundaryTests(unittest.TestCase):
    def test_config_does_not_create_writers(self) -> None:
        # 配置里只允许 reader 相关字段；出现 writer/service 关键字就说明有人误加。
        raw = json.loads(REAL_REALTIME_CONFIG.read_text(encoding="utf-8"))
        text = json.dumps(raw, ensure_ascii=False)
        for forbidden in ("create_writer", "create_service", "control_command"):
            self.assertNotIn(forbidden, text)
        self.assertTrue(raw.get("read_only"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
