# Apollo 实车运行状态只读监控（第二版：健康状态 + 实时数据）

**一句话：这个网页有两个层次：上半部分看“进程和库在不在”，新增的“实时数据”区域看“Cyber 到底收到了什么”——摄像头显示最新一帧，CAN 显示解码后的 Chassis / ChassisDetail，其他模块显示最新一条 protobuf 字段。**

> 只读监控，不代替 DreamView，不具备车辆控制能力。
> 没有启动/停止模块、录制、参数写入、发送控制消息的接口。
> 默认只监听 `127.0.0.1`，没有登录、没有 HTTPS，不适合直接暴露到公网。

---

## 1. 先分清两个概念：健康状态 vs 实时消息内容

这是第二版最重要的变化，也是现场最容易误解的地方。

| 看什么 | “健康状态”区域 | “实时数据”区域 |
|---|---|---|
| 回答的问题 | 进程在不在？DAG 对应的 `.so` 有没有真的映射？`cyber_channel` 能不能列出通道？ | Cyber reader 有没有收到消息？消息里的字段是什么？摄像头有没有画面？ |
| 数据来源 | `/proc`、DAG 文件、`/proc/<pid>/maps`、`cyber_channel list`、`cyber_node list` | Apollo Cyber Python API 的只读 reader |
| 典型误判 | 进程在跑，但通道根本没数据 | 通道有数据，但车辆不一定在自动驾驶 |
| 不可用时 | 显示“不可用”和原因，不显示假 0 | 显示 `available=false`、`status=unavailable` 和具体原因 |
| 是否保存历史 | 不保存 | 不保存，每个通道只留最新一条/最新一帧 |

**进程在跑 ≠ 数据在流；数据在流 ≠ 车辆控制正常。** 这两块要分开看。

---

## 2. 安全边界（先读）

| 项目 | 本系统怎么做 |
|---|---|
| 启动/停止 Apollo | 不做，也没有对应接口 |
| 车辆控制 | 不做，不创建 writer，不创建 service/client，不发送任何 Cyber/车辆消息 |
| HTTP 方法 | 只接受 `GET`/`HEAD`；`POST`/`PUT`/`DELETE`/`PATCH` 返回 405 |
| Cyber 操作 | 只创建 Node 和 reader；不创建 writer；不调用 `cyber_launch` |
| 外部命令 | 健康层固定参数、`shell=False`、带超时；实时层不调用 shell |
| 数据写入 | 不写 Apollo 配置、不写日志文件、不落历史数据库 |
| 监听地址 | 默认 `127.0.0.1`；只有显式 `HOST=0.0.0.0` 才对外 |
| 认证/加密 | 没有。只适合受信车辆局域网 |
| 可选依赖 | Cyber Python、protobuf、OpenCV 都是“已存在则使用”，不是启动必需品 |

页面会显示通道名、进程命令行、JSON 字段等敏感信息。开放到局域网前，请确认网络里没有无关人员。

---

## 3. 数据链路：一条消息是怎么到网页的

```text
Apollo 模块 / 车型驱动
        │ 发布（本系统不发布）
        ▼
Cyber 通道（例如 /apollo/canbus/chassis）
        │
        ▼
realtime_bridge.py
  ├─ 动态导入 generated *_pb2 模块
  ├─ Node.create_reader(channel, MessageClass, callback)   ← 只读
  ├─ callback 收到消息后只保留最新一条/最新一帧
  ├─ protobuf 反射转 JSON：字段名、枚举名、过滤 bytes
  └─ 图片转 base64 data URI
        │
        ▼
RealtimeStore（线程安全，有上限）
        │
        ├─ GET /api/realtime  → 一次性 JSON 快照
        └─ GET /api/stream    → Server-Sent Events 持续推送
                                  │
                                  ▼
                            浏览器 web/app.js
```

浏览器的策略：

1. 优先用 `EventSource('/api/stream')` 接收 SSE；
2. SSE 断开或不支持时，自动退回每 2 秒轮询 `/api/realtime`；
3. 15 秒后自动尝试重连 SSE；
4. 所有动态内容都用 `textContent` 写入，不把消息字段当 HTML 执行。

实时层初始化失败、Cyber 没运行、protobuf 模块不存在、通道没有数据，都只影响实时区域。`/api/summary`、`/api/modules` 等健康接口照常工作。

---

## 4. 源码依据：通道和类型不是猜的

实现前阅读了 `apollo-edu-9.1.1-container-src/src`，下面这些事实直接决定了配置和代码。

### 4.1 Cyber Python API

| 源码位置 | 关键事实 | 对实现的影响 |
|---|---|---|
| `cyber/python/cyber_py3/cyber.py` | `init()`、`shutdown()`、`Node.create_reader()`、`Node.spin()`；reader 回调由 Cyber 线程触发 | 后台线程导入 Cyber、init、创建 reader，然后 `spin()` 等待关闭 |
| `cyber/python/cyber_py3/cyber.py` | 模块导入时就 `import _cyber_wrapper` | Cyber 不可导入时必须在导入阶段安全降级，不能拖垮网页 |
| `cyber/python/cyber_py3/examples/py_listener.py` | `cyber.init()` → `Node(...)` → `create_reader(channel, MessageClass, callback)` → `spin()` → `cyber.shutdown()` | 只使用这条只读路径 |
| `cyber/docs/cyber_python_api_cn.md` | 读通道用 reader + callback + spin | 没有 Writer、Service、Client 路径 |
| `cyber/python/internal/py_cyber.cc` | `PyReader` 内部按通道创建 reader，回调里解析 protobuf | 回调必须自己捕获异常，不能让异常穿回 C++ |

### 4.2 本配置实际订阅的通道

| 页面区域 | 通道 | Python 模块 | 消息类 |
|---|---|---|---|
| CAN | `/apollo/canbus/chassis` | `modules.common_msgs.chassis_msgs.chassis_pb2` | `Chassis` |
| CAN | `/apollo/canbus/chassis_detail` | `modules.common_msgs.chassis_msgs.chassis_detail_pb2` | `ChassisDetail` |
| 定位 | `/apollo/localization/pose` | `modules.common_msgs.localization_msgs.localization_pb2` | `LocalizationEstimate` |
| 控制 | `/apollo/control` | `modules.common_msgs.control_msgs.control_cmd_pb2` | `ControlCommand` |
| 规划 | `/apollo/planning` | `modules.common_msgs.planning_msgs.planning_pb2` | `ADCTrajectory` |
| 感知 | `/apollo/perception/obstacles` | `modules.common_msgs.perception_msgs.perception_obstacle_pb2` | `PerceptionObstacles` |
| 感知 | `/apollo/perception/traffic_light` | `modules.common_msgs.perception_msgs.traffic_light_detection_pb2` | `TrafficLightDetection` |
| 预测 | `/apollo/prediction` | `modules.common_msgs.prediction_msgs.prediction_obstacle_pb2` | `PredictionObstacles` |
| 路由 | `/apollo/routing_response` | `modules.common_msgs.routing_msgs.routing_pb2` | `RoutingResponse` |
| 路由 | `/apollo/raw_routing_response` | 同上 | `RoutingResponse` |

通道名来自 `modules/common/adapters/adapter_gflags.cc`；路由的 `/apollo/raw_routing_response` 来自 `modules/routing/conf/routing_config.pb.txt`。两个路由通道都列入配置，现场通常只有其中一个有数据，另一个会明确显示不可用。

### 4.3 摄像头压缩通道

源码里的相机配置 `compress_conf.output_channel` 明确给出了压缩通道：

| 页面名称 | 压缩通道 | 来源 |
|---|---|---|
| 前视 6mm | `/apollo/sensor/camera/front_6mm/image/compressed` | `camera_front_6mm.pb.txt`；`adapter_gflags.cc:189` |
| 前视 12mm | `/apollo/sensor/camera/front_12mm/image/compressed` | `camera_front_12mm.pb.txt`；`adapter_gflags.cc:191` |
| 后视 6mm | `/apollo/sensor/camera/rear_6mm/image/compressed` | `camera_rear_6mm.pb.txt`；`adapter_gflags.cc:200` |
| 左鱼眼 | `/apollo/sensor/camera/left_fisheye/image/compressed` | `camera_left_fisheye.pb.txt`；`adapter_gflags.cc:194` |
| 右鱼眼 | `/apollo/sensor/camera/right_fisheye/image/compressed` | `camera_right_fisheye.pb.txt`；`adapter_gflags.cc:197` |
| 长焦红绿灯 | `/apollo/sensor/camera/traffic/image_long/compressed` | `camera_long_conf.pb.txt` |

原始 `Image` 通道也存在（例如 `/apollo/sensor/camera/front_6mm/image`），但本配置优先订阅 `CompressedImage`：压缩图可以直接转 base64 给浏览器，不需要解压、重新编码，CPU 开销小。

`Image` 和 `CompressedImage` 的区别：

- `Image`：`data` 是**原始像素**，需要知道宽、高、`step`、`encoding`，还必须用 OpenCV 重新编码成 JPEG，浏览器才能显示；
- `CompressedImage`：`data` 已经是 JPEG/PNG 字节，服务端只做格式检查、大小检查和 base64 编码；
- 本系统只有在 OpenCV **已经存在** 时才处理原始 `Image`；没有 OpenCV 就返回明确原因，不会强制安装，也不会让网页退出。

### 4.4 CAN 的边界

`/apollo/canbus/chassis` 和 `/apollo/canbus/chassis_detail` 是 **Apollo 已经解码后的 protobuf 消息**，不是原始 CAN ID、也不是 8 字节原始帧。

`canbus_component.cc` 里的发布逻辑：

- `PublishChassis()` 每轮都会写 `chassis_writer_`，对应 `/apollo/canbus/chassis`；
- `/apollo/canbus/chassis_detail` 只有在 `FLAGS_enable_chassis_detail_pub` 为真时才发布；
- 当前源码默认 `enable_chassis_detail_pub=true`，`canbus.conf` 也显式打开；
- 但“默认打开”不等于“实车一定发布”，车型驱动可能不同，所以页面仍会标“需现场确认”。

如果确实要**原始 CAN ID / 8 字节数据**，需要车型 canbus 驱动或 CAN 卡提供具体话题/接口。本系统不猜这种车型私有通道，也不会把解码后的 Chassis 说成原始 CAN 帧。

---

## 5. 目录结构

```text
实车状态检测系统/
├── collector.py                  # 第一版健康采集：进程、DAG、maps、cyber_channel
├── realtime_bridge.py            # 第二版实时层：Cyber reader、RealtimeStore、protobuf/图片转换
├── monitor_server.py             # 标准库 HTTP + /api/realtime + SSE
├── run_monitor.sh                # 一键启动 / --check
├── test_collector.py             # 健康层测试
├── test_realtime.py              # 实时层测试（不依赖真实 Cyber/protobuf/OpenCV）
├── config/
│   ├── modules.json              # 健康层模块清单
│   └── realtime_channels.json    # 实时通道、消息类型、CAN 字段、图片上限
├── web/
│   ├── index.html                # 健康状态 + 实时数据
│   ├── app.js                    # SSE/轮询、摄像头、CAN、JSON 渲染
│   └── style.css                 # 响应式样式
└── README.md
```

正常用 `./run_monitor.sh` 启动不会生成 `__pycache__`、日志、数据库或缓存文件。手动跑 `py_compile` 会产生 `__pycache__`，验证完要清掉。

---

## 6. 运行环境：为什么必须在 Apollo 的同一套环境里

实时数据不是从网页“远程查询”出来的，而是监控进程自己创建 Cyber reader 收出来的。因此监控进程必须和 Apollo：

1. **在同一个容器/运行环境里**；
2. **在同一个 PID 命名空间里**（健康层要看到 Apollo 的 `/proc/<pid>`）；
3. **在同一个 IPC 命名空间里**（Cyber 的共享内存通信依赖它）；
4. **使用同一个 `CYBER_DOMAIN_ID`**（默认 80，但现场可能被覆盖）；
5. **`PYTHONPATH` 能找到 Cyber Python 和 generated `*_pb2` 模块**。

只在宿主机上打开源码目录，健康层也许能读到 DAG 文件，但实时层收不到车上的 Cyber 消息。跨 PID/IPC/Cyber 域时，页面会显示 `unavailable` 和原因，这是环境限制，不是网页能绕过去的。

### 6.1 可选依赖

| 依赖 | 用途 | 没有时 |
|---|---|---|
| `cyber.python.cyber_py3.cyber` / `cyber_py3.cyber` | 只读订阅 | 实时层 `unavailable`，健康接口照常 |
| `google.protobuf` | 消息对象解析 | 对应通道导入/转换失败，显示原因 |
| generated `*_pb2` | 每个通道的消息类 | 该通道 `unavailable`，其他通道不受影响 |
| OpenCV `cv2` | 仅原始 `Image` 转 JPEG | 原始 Image 通道显示原因；压缩图不受影响 |

这些都是“已存在则使用”，不是启动必需项。`pip install`、`apt install` 不是本系统的启动步骤。

---

## 7. 怎么启动和检查

### 7.1 静态检查

```bash
cd "/home/zwk/桌面/deepseek-tesk/apollo628/代码阅读/实车状态检测系统"
./run_monitor.sh --check
```

检查内容：

- `config/modules.json` 是否可解析；
- `config/realtime_channels.json` 是否可解析，通道 id/通道名/类型/模块是否合法；
- 首页、`app.js`、`style.css` 是否存在且非空；
- `collector.py`、`realtime_bridge.py`、`monitor_server.py` 和测试文件语法；
- 端口范围。

检查不会启动服务，也不会执行 Apollo 命令。

### 7.2 启动

```bash
./run_monitor.sh
```

默认地址：`http://127.0.0.1:8848`。

指定端口：

```bash
HOST=127.0.0.1 PORT=9000 ./run_monitor.sh
```

如果 `cyber_channel` 不在 PATH，可以显式指定工具目录（只影响健康层的查询命令）：

```bash
MONITOR_CYBER_BIN_DIR=/apollo/bazel-bin/cyber/tools ./run_monitor.sh
```

实时通道配置默认是 `config/realtime_channels.json`，也可以显式指定：

```bash
./run_monitor.sh --realtime-config /path/to/realtime_channels.json
```

如果现场 shell 还没有 Cyber 环境，先在同一容器 shell 里加载 Apollo 的环境脚本（只设置变量和 PATH，不启动模块）：

```bash
source "${APOLLO_ROOT_DIR:-/apollo}/cyber/setup.bash"
```

### 7.3 只启动健康层

如果只想看进程和库，不想导入 Cyber，可以：

```bash
./run_monitor.sh --no-realtime
```

此时 `/api/realtime` 会明确返回“实时采集层未启用”，健康接口不受影响。

### 7.4 局域网访问（有风险）

```bash
HOST=0.0.0.0 PORT=8848 ./run_monitor.sh
```

**警告：** 没有登录、没有 HTTPS，页面包含通道名、进程命令行、JSON 字段等敏感信息。必须配合主机防火墙/受信网段，绝对不要映射到公网。

---

## 8. HTTP 接口

所有接口都是 `GET`，返回 `application/json; charset=utf-8`（`/api/stream` 例外，返回 `text/event-stream`）。

| 接口 | 内容 |
|---|---|
| `/api/summary` | 总体健康状态、模块计数、数据源可用性、告警 |
| `/api/modules` | 每个模块的状态、原因、进程、DAG、目标库 |
| `/api/channels` | `cyber_channel list` 的通道、建议通道是否出现 |
| `/api/processes` | 匹配到的 Apollo 进程 |
| `/api/libraries` | 进程真实映射的 `.so` |
| `/api/config` | 配置摘要、白名单环境变量、路径 |
| `/api/nodes` | 活跃节点列表 |
| `/api/realtime` | 所有实时通道状态、最新 JSON、CAN 字段、图片 data URI、统计 |
| `/api/stream` | SSE，持续推送实时快照 |

示例：

```bash
curl -s http://127.0.0.1:8848/api/summary | python3 -m json.tool
curl -s http://127.0.0.1:8848/api/realtime | python3 -m json.tool
curl -N http://127.0.0.1:8848/api/stream
```

调试时可以加 `?refresh=1` 跳过健康层的 2 秒缓存：

```bash
curl -s 'http://127.0.0.1:8848/api/summary?refresh=1' | python3 -m json.tool
```

没有写接口。对 `/api/*` 发 `POST` 会得到 405 和明确原因。

### 8.1 `/api/realtime` 的结构

关键字段：

```json
{
  "status": "ok / degraded / unavailable",
  "available": true,
  "reason": "为什么可用/不可用",
  "read_only": true,
  "counts": {
    "channels_total": 16,
    "channels_enabled": 16,
    "channels_ready": 8,
    "channels_with_message": 2,
    "channels_with_image": 1,
    "channels_error": 1
  },
  "channels": [
    {
      "id": "chassis",
      "label": "底盘 Chassis",
      "channel": "/apollo/canbus/chassis",
      "kind": "protobuf",
      "available": true,
      "status": "ok",
      "reason": "",
      "statistics": {
        "received_count": 100,
        "processed_count": 100,
        "error_count": 0,
        "sequence": 100,
        "hz": 10.0,
        "message_bytes": 123,
        "last_received_at": "2026-01-01T12:00:00.000+08:00"
      },
      "message": { "speed_mps": 1.5, "driving_mode": "COMPLETE_AUTO_DRIVE" },
      "image": null
    }
  ],
  "can": {
    "available": true,
    "fields": [
      {
        "key": "speed_mps",
        "label": "车速",
        "value": 1.5,
        "display": "1.5 m/s",
        "unit": "m/s",
        "source": "chassis",
        "fault": false
      }
    ],
    "faults": [],
    "chassis_detail": { "statistics": {} }
  }
}
```

重点：

- `available=false` 时不要把它当成 0；
- `reason` 必须能看出是 Cyber 没导入、模块不存在、通道没数据、图片过大还是 JSON 被截断；
- `message` 只保留最新一条；
- `image.data_uri` 只保留最新一帧。

### 8.2 SSE 的事件

SSE 使用标准 `text/event-stream`，事件有：

- `hello`：连接后的第一份完整快照；
- `realtime`：后续快照；
- `unavailable`：实时层不可用时的明确原因；
- 注释行 `: keep-alive ...`：心跳，防止代理断开。

为了不重复传大图片，SSE 对“没有变化的帧”会省略 `data_uri`，并设置 `data_uri_omitted: true`；浏览器会保留上一帧。首次连接仍会传完整图片。

---

## 9. 网页怎么看

### 9.1 摄像头

每个摄像头卡片显示：

- 实时画面（`<img>`，src 是 `data:image/jpeg;base64,...` 或 PNG）；
- 通道名；
- 帧率、宽×高、图片字节数；
- 最后更新时间；
- 没有画面时的具体原因。

没有帧时显示“尚未收到画面”“通道未发现”“压缩图超过上限”“原始 Image 需要 OpenCV”等原因，不会显示假图或假 0。

### 9.2 CAN

CAN 表格显示中文名、值、单位、原始字段、来源。重点字段包括：

- `speed_mps` 车速；
- `engine_rpm` 发动机转速；
- `throttle_percentage` 油门开度；
- `brake_percentage` 制动开度；
- `steering_percentage` 转向比例；
- `gear_location` 挡位；
- `driving_mode` 驾驶模式；
- `error_code` 底盘故障码；
- `battery_soc_percentage` 电池电量；
- `chassis_error_mask` 故障掩码；
- 各类 `is_*_online` 在线状态；
- `ChassisDetail` 里的电池、EPS、车速有效等字段。

枚举显示“中文（英文枚举名）”，例如 `完全自动驾驶（COMPLETE_AUTO_DRIVE）`，不只显示数字。

下方“故障 / 在线状态”会把 `error_code` 非 `NO_ERROR`、故障掩码非 0、`*_online=false`、`*_fail=true`、`*_valid=false` 等标出来。没有故障时显示“当前没有发现已配置的故障或离线标志”，但这只代表配置里列出的字段，不是全车诊断。

`ChassisDetail` 的完整 JSON 在折叠区里，默认不展开，且服务端已经过滤了 bytes/超大字段。

### 9.3 其他通道

定位、控制、规划、感知、预测、路由等按通道显示最新 protobuf 字段 JSON，默认折叠。规划/感知/预测消息可能很大，服务端有最大递归、最大数组项、最大 JSON 字节上限；页面还会再截断显示。

---

## 10. 配置：怎么加自己的车型字段和通道

### 10.1 实时通道配置 `config/realtime_channels.json`

每个通道的字段：

```json
{
  "id": "chassis",
  "label": "底盘 Chassis",
  "channel": "/apollo/canbus/chassis",
  "kind": "protobuf",
  "python_module": "modules.common_msgs.chassis_msgs.chassis_pb2",
  "message_class": "Chassis",
  "enabled": true,
  "max_fps": 20,
  "group": "can",
  "source": "源码出处",
  "site_confirm": "需现场确认：..."
}
```

| 字段 | 必填 | 说明 |
|---|---|---|
| `id` | 是 | 唯一，页面和 SSE 用它做合并 |
| `label` | 是 | 页面显示名 |
| `channel` | 是 | 必须是 `/` 开头的 Cyber 通道 |
| `kind` | 是 | `protobuf`、`image`、`compressed_image` |
| `python_module` | 是 | generated `*_pb2` 的完整导入路径 |
| `message_class` | 是 | 模块里的消息类名 |
| `enabled` | 否 | 默认 true；false 时不创建 reader |
| `max_fps` | 否 | 每秒最多处理多少条；超出的只计数、不解码 |
| `group` | 否 | 页面分组：`can`、`camera`、`localization` 等 |
| `source` | 否 | 源码依据，便于现场核对 |
| `site_confirm` | 否 | “需现场确认”的说明 |

摄像头通道用 `kind: "compressed_image"`，消息类 `CompressedImage`；原始像素通道用 `kind: "image"`，消息类 `Image`。

### 10.2 CAN 字段配置

`config/realtime_channels.json` 的 `can.fields` 决定表格显示什么：

```json
{
  "source": "detail",
  "path": "eps.steering_angle",
  "label": "方向盘转角（ChassisDetail）",
  "unit": "deg",
  "kind": "float",
  "fallback_source": "chassis",
  "fallback_path": "steering_percentage"
}
```

| 字段 | 说明 |
|---|---|
| `source` | `chassis` 或 `detail` |
| `path` | protobuf 字段路径，用 `.` 表示嵌套，例如 `check_response.is_eps_online` |
| `label` | 中文名 |
| `unit` | 单位，没有就留空 |
| `kind` | `float`、`int`、`bool`、`string`、`enum` |
| `enum_labels` | 枚举英文名 → 中文名 |
| `fallback_source` / `fallback_path` | 主路径没有值时，尝试另一个消息的路径 |

新增车型字段的步骤：

1. 在 `chassis.proto` 或 `chassis_detail.proto` 找到字段名和嵌套路径；
2. 在 `can.fields` 加一项；
3. 如果是枚举，把枚举名抄进 `enum_labels`；
4. 运行 `./run_monitor.sh --check`；
5. 重启**监控服务**（不是 Apollo），看表里是否出现值；
6. 如果没有值，先看 `missing_fields` 和通道 reason，不要直接认为是网页坏了。

### 10.3 摄像头通道配置

新增相机：

1. 在 `modules/drivers/camera/conf/*.pb.txt` 找 `compress_conf.output_channel`；
2. 确认通道名以 `/compressed` 结尾；
3. 复制一个 camera 配置项，改 `id`、`label`、`channel`；
4. `kind` 保持 `compressed_image`，`message_class` 保持 `CompressedImage`；
5. 检查该相机驱动是否真的启动。

不要只按“常见 Apollo 通道名”猜；不同车型/驱动版本的通道可能不一样。

### 10.4 generated `*_pb2` 模块

Python 模块路径来自源码里的实际 import，例如：

```text
modules.common_msgs.chassis_msgs.chassis_pb2
modules.common_msgs.sensor_msgs.sensor_image_pb2
modules.common_msgs.localization_msgs.localization_pb2
modules.common_msgs.control_msgs.control_cmd_pb2
modules.common_msgs.planning_msgs.planning_pb2
modules.common_msgs.perception_msgs.perception_obstacle_pb2
modules.common_msgs.prediction_msgs.prediction_obstacle_pb2
modules.common_msgs.routing_msgs.routing_pb2
```

如果现场环境里没有这些生成模块，对应通道会显示：

```text
动态导入失败：modules...chassis_pb2.Chassis；ModuleNotFoundError...
需现场确认该 generated *_pb2 模块是否在 PYTHONPATH 中。
```

这时要检查 Apollo 构建产物/Python 路径，而不是改网页硬编码。

---

## 11. 怎么确认“真实消息”到底有没有到

### 11.1 看页面

1. 打开“实时数据”区域；
2. 看右上角连接状态：`SSE 已连接` / `轮询中` / `实时层不可用`；
3. 看每个通道的 `reader` 状态和 reason；
4. 看“已有消息”“已有画面”计数；
5. 摄像头卡片没有画面时，读卡片里的原因。

### 11.2 看 JSON

```bash
curl -s http://127.0.0.1:8848/api/realtime | python3 -m json.tool
```

重点检查：

- 顶层 `available`、`status`、`reason`；
- `cyber.initialized`；
- 每个通道的 `available`、`status`、`reason`；
- `statistics.received_count` 是否在增长；
- `statistics.hz` 是否合理；
- `image.data_uri` 是否以 `data:image/` 开头。

### 11.3 手工只读核对（不经过本系统）

如果页面说通道没数据，可以在同一容器/Cyber 域里用 Apollo 自带工具只读确认：

```bash
cyber_channel list
cyber_channel info /apollo/canbus/chassis
```

如果这些工具也看不到通道，先查模块是否启动、`CYBER_DOMAIN_ID` 是否一致，而不是先怀疑网页。

### 11.4 看服务端 stderr

监控服务只把错误写到 stderr，不落盘。Cyber 导入失败、通道创建失败、protobuf 转换错误都会体现在接口 reason 里；如果进程直接退出，先看启动终端里的 Python traceback。

---

## 12. 降级规则：什么情况显示什么

| 情况 | 实时层表现 | 健康层表现 |
|---|---|---|
| Cyber Python 导入失败 | 顶层 `available=false`、`status=unavailable`，所有通道 `unavailable`，reason 含 ImportError | 照常工作 |
| Cyber 没运行 / init 返回 False | 顶层 `available=false`，reason 说明 IPC/域/权限问题 | 照常工作 |
| generated `*_pb2` 不存在 | 该通道 `unavailable`，其他通道继续 | 照常工作 |
| 通道当前没被发布 | 该通道 `unavailable`，reason 说明“Cyber 当前未发现这个通道” | 照常工作 |
| reader 创建成功但没消息 | 通道 `waiting`、`available=true`，reason 说明等待第一条消息 | 照常工作 |
| 图片超过上限 | `image.available=false`，`reason` 写明字节数和上限 | 照常工作 |
| 原始 Image 但无 OpenCV | `image.available=false`，reason 提示优先用 CompressedImage | 照常工作 |
| protobuf 太大 | 截断并带 `__truncated__` / `__reason__`，或返回紧凑摘要 | 照常工作 |
| SSE 断开 | 浏览器退回 `/api/realtime` 轮询，15 秒后重连 | 照常工作 |

**不会出现：** Cyber 不可用却显示 0 条消息、没有画面却显示假图、没有 Chassis 却显示 0 车速。

---

## 13. 限制和“需现场确认”

### 13.1 当前限制

- **不替代 DreamView**：没有地图、轨迹回放、HMI 操作、录制；
- **不保存历史**：每个通道只留最新一条/一帧，刷新/重启后没有过去数据；
- **不做控制**：不创建 writer/service/client，不发送车辆指令；
- **原始 CAN 帧不在范围内**：Chassis/ChassisDetail 是解码后消息；
- **SSE 是单向推送**：浏览器不能通过它发命令；
- **没有认证/TLS**：只适合受信局域网；
- **大消息会截断**：规划、感知、预测的 debug/数组可能只显示前一部分；
- **通道列表延迟**：Cyber 拓扑发现需要一点时间，启动瞬间可能显示“未发现通道”，收到消息后会转为正常；
- **不做自动重试全部通道**：如果某个通道在监控启动后才出现，当前 reader 可能已经创建但通道列表标记为不存在；以实际收到的 `received_count` 为准，必要时重启监控服务。

### 13.2 必须现场确认

1. **generated `*_pb2` 是否存在**：源码里只有 `.proto` 和构建规则，没有预生成的 `_pb2.py`；实际 Python 路径取决于现场构建/安装。
2. **通道是否真的发布**：源码里有定义不等于当前车型/当前 DAG 会发布。
3. **`/apollo/canbus/chassis_detail` 是否发布**：源码默认打开，但车型配置可能覆盖 `--enable_chassis_detail_pub`。
4. **路由通道二选一**：`/apollo/routing_response` 和 `/apollo/raw_routing_response` 可能只有一个有数据。
5. **车型 CAN 字段是否填充**：不同车型的 `ChassisDetail` 字段完整度不同；没有值的字段会列在 `missing_fields`。
6. **相机压缩通道**：`compress_conf.output_channel` 是源码配置，但相机驱动可能没启用，或实际发布的是 raw Image。
7. **Cyber 域**：`CYBER_DOMAIN_ID` 默认 80，现场可能被覆盖；监控和 Apollo 必须一致。
8. **OpenCV 是否存在**：只影响原始 Image，不影响 CompressedImage。
9. **权限**：不同用户、容器安全策略、共享内存权限都可能影响 Cyber reader；接口会写原因。

---

## 14. 测试和验收

在目标目录执行：

```bash
python3 -B -m py_compile collector.py realtime_bridge.py monitor_server.py test_collector.py test_realtime.py
python3 -B test_collector.py
python3 -B test_realtime.py
./run_monitor.sh --check
```

测试覆盖：

- Cyber 导入失败：实时层 `unavailable`，健康接口仍可用；
- 实时配置缺失/坏 JSON/重复 id/通道名错误；
- 假 protobuf：字段名、枚举名、bytes 过滤、NaN、递归/数组/JSON 字节上限；
- 假 CompressedImage：data URI、宽高、超过上限原因；
- 假 Image：无 OpenCV/尺寸不完整时返回原因；
- 多线程同时写：只保留最新值，计数一致；
- `/api/realtime` 200 合法 JSON；
- SSE 返回合法 `text/event-stream`、`hello`/`unavailable` 事件，客户端断开后 handler 计数归零；
- `POST` 仍返回 405；
- 真实 `config/realtime_channels.json` 可加载，包含 CAN、6 路相机和其他数据通道。

手工验收（临时端口，不占用常用端口）：

```bash
PORT=18888 ./run_monitor.sh
curl -s -o /dev/null -w '%{http_code}\n' http://127.0.0.1:18888/
curl -s http://127.0.0.1:18888/api/summary | python3 -m json.tool
curl -s http://127.0.0.1:18888/api/realtime | python3 -m json.tool
curl -N http://127.0.0.1:18888/api/stream
```

验收后按 `Ctrl+C` 停止，确认端口释放。如果手动执行了 `py_compile`，清理缓存：

```bash
rm -rf __pycache__
```

正常用 `./run_monitor.sh` 启动不会生成 `__pycache__`。

---

## 15. 最后再强调一次

**这是只读监控，不是 DreamView 的替代品，不具备车辆控制能力。**
它适合在实车/容器内回答两个问题：

1. 进程和库在不在？
2. Cyber 到底有没有收到消息，消息内容是什么？

真正的车辆控制、规划行为、HMI 操作，仍然以 Apollo 原生 DreamView 和现场流程为准。
