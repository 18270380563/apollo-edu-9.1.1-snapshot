# 2026-10-29 两轮自动驾驶横向摆动测试（cruise=10 m/s 与 cruise=5 m/s）

Apollo EDU 9.1.1 实车（192.168.10.6，容器 `apollo_neo_dev_edu_9.1.1`）当天的两次自动驾驶测试：

| 轮次 | 规划巡航速度 | 实际最高车速 | 自动驾驶时长 | 现象 |
|---|---|---|---|---|
| 第一轮 | `default_cruise_speed = 10` | 6.76 m/s | 9.2 s | **从接管第一帧起就剧烈摆动**，末段 ±50% 转向饱和 |
| 第二轮 | `default_cruise_speed = 5` | 5.22 m/s | 38.8 s | 前 20 s 稳定（包络 0.4~1.2%），**t≈32 s 起指数发散**，制动退速后自愈 |

完整结论见 `横向控制剧烈摆动根因分析.md`（同目录）。

---

## 目录结构

```
2026-10-29_cruise10_cruise5/
├── README.md
├── 横向控制剧烈摆动根因分析.md      横向控制视角的根因分析（主文档）
├── records/                          cyber_recorder 原始记录（Git LFS）
│   ├── round1_cruise10_a_20261029_143236/   14:32:43 ~ 14:33:43，28 个文件
│   ├── round1_cruise10_b_20261029_143456/   14:35:07 ~ 14:36:07，41 个文件
│   └── round2_cruise5_20261029_144509/      14:45:10 ~ 14:47:07，26 个文件
├── 分析数据/                         edu_debug 导出的逐周期打点（JSONL/TSV）
│   ├── edu_debug_drive_1029/         第一轮
│   └── edu_debug_drive_1029_r2/      第二轮
└── 中间脚本/
    └── analyze_lateral_oscillation.py   复现分析文档里的全部数字
```

> 第一轮记录分成 `_143236` 和 `_143456` 两段，是因为中途 recorder 重启（14:33:43 ~ 14:35:07 有约 80 s 空档），
> 两段合起来才是这一轮的完整记录。

## 记录文件的通道

每个 `keydrv_<tag>_<TS>.record.000xx.<时间>` 是**单通道**记录（本 build 的 cyber_recorder 不支持多通道合并），
`<tag>` 与话题对应：

| tag | 话题 |
|---|---|
| eduplan | `/apollo/edu_debug/planning` |
| eductrl | `/apollo/edu_debug/control` |
| plan / plancmd | `/apollo/planning`, `/apollo/planning/command` |
| ctrl / ctrldbg | `/apollo/control`, `/apollo/control/debug` |
| chassis | `/apollo/canbus/chassis` |
| pose | `/apollo/localization/pose` |
| odom | `/apollo/sensor/gnss/odometry` |
| obst / predict | `/apollo/perception/obstacles`, `/apollo/prediction` |
| routing / task / hmi / tf | `/apollo/routing_response`, `/apollo/task_manager`, `/apollo/hmi/status`, `/tf` |

## 怎么用

播放某一路（容器内）：

```bash
source /opt/apollo/neo/setup.sh
cyber_recorder play -f keydrv_ctrl_20261029_144509.record.00000.20261029144510 -c /apollo/control
```

查看记录信息（起止时间、消息数）：

```bash
cyber_recorder info keydrv_ctrl_20261029_144509.record.00000.20261029144510
```

复现分析数字：

```bash
python 中间脚本/analyze_lateral_oscillation.py
```

> 脚本默认读取 `<本目录>/分析数据/edu_debug_drive_1029*/control_state.jsonl`（以脚本所在目录的上一级为根），
> 在本目录直接运行即可。

## 重要单位约定

`cmd_steer`、`lat.*_contribution`、`steer_limit` 全部是**百分比**，不是角度：

```
max_steer_angle = 8.37333 rad = 479.75°（方向盘）;  steer_ratio = 16.18
100% = 479.75° 方向盘 = 29.65° 前轮;   1% = 4.797° 方向盘 = 0.2965° 前轮
```

## 数据来源说明

- `records/` 是 cyber_recorder 的原始记录，可直接用 `cyber_recorder play/info`。
- `分析数据/` 是 edu_debug 打点经 `export_debug_record` 导出后转成的 JSONL（控制 10 Hz、规划 10 Hz），
  字段含义见主文档 §2。
