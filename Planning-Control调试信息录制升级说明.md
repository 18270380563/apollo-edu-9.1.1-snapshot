# Planning / Control 调试信息录制升级说明

> 目标：让 `cyber_recorder` 录制的包里，除了原有的话题消息，还带上 planning、control
> 两个模块的**配置文件快照**和**运行关键状态**，从而能离线回答两类问题：
> 1. 为什么车的速度没有按我们配置的速度走？
> 2. 为什么改了配置文件没有生效（到底加载了哪个文件、内容是什么、被谁覆盖）？

---

## 1. 总体设计

```
planning / control 进程
        │  EduDebugLogger（单例，位于 workspace 的 modules/edu_debug）
        ├── 启动时：把所有 gflag、组件配置、planner 配置、配置文件树（含内容+sha1）打包成
        │          DebugDump 消息，发布到专用 channel
        └── 运行时：按固定频率（0.1~5 Hz）发布状态 kv / 文本曲线 / 事件
                        │
                        ▼
        /apollo/edu_debug/planning      (topic, DebugDump)
        /apollo/edu_debug/control       (topic, DebugDump)
                        │
                        ▼
             cyber_recorder record -a      （-a 录制所有 channel，自动包含上面两个）
                        │
                        ▼
        录制文件 .record  ──►  export_debug_record 工具离线展开成文本/JSON
```

为什么用**独立 channel** 而不是往现有话题里塞字段：
`modules/common_msgs` 在 `/opt/apollo/neo/src` 里是只读镜像，无法扩展 Apollo 的 proto；
所以在 workspace 里新建了 `modules/edu_debug/proto/debug_dump.proto`，用自定义消息承载
任意 key/value、文本和文件内容。

为什么**启动快照会被缓存并慢速重放**：
录制通常是在模块启动之后才开始的。EduDebugLogger 会把启动期打出的 config/flags/manifest
缓存下来，之后每 0.25 s 重放一条、20 s 一个循环，这样"晚启动的录制"也一定能录到完整快照。

---

## 2. 新增文件（workspace 内，随源码编译，不需要改 neo）

| 路径 | 作用 |
|---|---|
| `modules/edu_debug/proto/debug_dump.proto` | DebugDump 消息定义（header/module/source/kind/name/text/entries/分片字段…） |
| `modules/edu_debug/debug_logger.h/.cc` | 单例日志器：`Init/DumpAllFlags/DumpFile/DumpText/DumpKv/DumpTree/DumpState/DumpEvent/ShouldPublish/WatchFile/Tick` |
| `modules/edu_debug/sha1.h` | 计算配置文件的 sha1，用来判断"改的这个文件到底有没有被加载" |
| `modules/edu_debug/tools/export_debug_record.cc` | 离线工具：把录制文件里的 edu_debug 消息展开成目录树 |
| `modules/edu_debug/BUILD`、`proto/BUILD`、`tools/BUILD` | 用**原生 bazel 规则**（不是 apollo_cc_library） |

> 注意：`modules/edu_debug` 是 workspace 本地新增包，没有安装进 `/opt/apollo/neo`，
> 因此不在 buildtool 自动生成的 `tools/package/dynamic_deps.bzl` 的 `SOURCE` 表里。
> `apollo_*` 宏会把这种包当成"预编译二进制包"并丢弃它的所有 deps，导致
> `no such target '//modules/edu_debug:edu_debug_logger'`。所以这三个 BUILD 文件
> 全部改用原生 `cc_library` / `cc_binary`，并显式写出真实的第三方 label
> （`//cyber` → `@cyber//:cyber` + `@cyber//:cyber_Ccyber`，另加 `@fastrtps//:fastrtps`）。

---

## 3. planning 侧打了什么

### 3.1 启动期（`planning_component.cc: Init`）

紧跟 `ComponentBase::GetProtoConfig(&config_)` 之后：

- `DumpAllFlags("planning_component.Init")` —— 全部 gflag 当前值（含默认值、来自哪个 flagfile）
- `config_value: component_config_path` —— DAG 解析出的组件配置绝对路径
- `config_value: planning_config` —— `PlanningConfig` 的 `DebugString()`
- `DumpFile(FLAGS_planner_config_path)` —— public_road_planner_config.pb.txt 的
  `path/realpath/exists/size/mtime/sha1` + 全文
- `DumpTree("/apollo/modules/planning", {.pb.txt,.conf,.xml,.dag,.launch,.json})`
  —— 把运行期**实际生效**的整棵配置树（/apollo/modules/* 是 profiles/HQEV 的符号链接）
  全部落进录制：manifest（路径/真实路径/大小/mtime/sha1）+ 每个文件的内容
- 之后每 5 s 重新计算被 dump 过的文件 sha1，一旦变化就发 `config_file_changed` 事件
  （用于确认"我改的文件到底什么时候生效"）

### 3.2 每帧规划输出（`planning_component.cc: Proc`，0.5 Hz）

`planning_output` 状态：

- 轨迹层面：`frame_seq`、`trajectory_type`、`estop/estop_reason`、`status_code/msg`、
  `gear`、点数、`v_first/v_max/v_end`、`a_abs_max`、`t_end`、`latency_total_ms/init_frame_ms`
- 速度决策输入：`default_cruise_speed`、`upper_speed_limit`
- 上游指令：planning command 是否存在、是否 motion、是否带 `target_speed`、具体值
- 车辆状态：`chassis_v`、throttle/brake/steer/gear/mode
- 场景：`scenario / stage / scenario_msg`（来自 `ADCTrajectory.debug.planning_data`）
- `main_decision`（截断 800 字符）

### 3.3 速度链细节（`public_road_planner.cc: Plan`）

**5 Hz `planning_speed_state`**（这是回答"速度为什么没按配置走"的核心）：

- 全局：`frame_seq / scenario / stage / scenario_status / scenario_msg / vehicle_v /
  target_speed_flag(FLAGS_default_cruise_speed) / upper_speed_limit / rl_count`
- 每条参考线（最多 3 条）`rl{i}.*`：
  - 身份：`id`（Lanes）、`drivable`、`change_lane`、`traj_type`
  - **三层巡航速度**：`cruise`(GetCruiseSpeed，实际用)、`base_cruise`(GetBaseCruiseSpeed)、
    `st_cruise`(StGraphData.cruise_speed) —— 对比这三者即可看出是谁把速度改掉了
  - ST 图：`st_init_v`、`st_bounds`(st 边界数量)、`stops`(停止决策数量)、`blocker`(阻塞障碍物 id)
  - 速度规划输出：`speed_n`、`v_plan_s0`、`v_plan_max`、`v_traj0`(最终轨迹首点速度)
  - **限速曲线**：`limit_s0/s10/s20/s30/s50/s100`（地图限速+曲率限速合成的 SpeedLimit）
  - 路径：`path_len`、`path_n`

**1 Hz `planning_speed_curve` 文本轨迹**（trace 类型，前 120 m，每 2 m 一行）：

```
# frame_seq=… scenario=… stage=… cruise=… base_cruise=… default_cruise=…
s   v_limit   v_plan   v_traj
0   8.3000    0.0000   0.0000
2   8.3000    1.2000   1.1800
...
```

一列一列对比就能定位"是限速压的、是优化器算的、还是轨迹生成时掉的"。

---

## 4. control 侧打了什么

### 4.1 启动期（`control_component.cc: Init`，pipeline 加载之后）

- `DumpAllFlags` —— 全部 gflag（可以看到 `--use_control_submodules=false` 这类关键开关）
- `config_value: pipeline_file` —— `FLAGS_pipeline_file`
- `config_value: control_pipeline` —— pipeline proto 的 `DebugString()`
- `DumpFile(FLAGS_calibration_table_file)` —— 标定表：路径/sha1/全文
- `DumpTree("/apollo/modules/control", …)` —— 整棵 control 配置树快照

### 4.2 每控制周期（`control_component.cc: Proc`，10 Hz）

`control_state` 状态：

- `cycle_ms`、`estop`、`estop_reason`
- chassis：`v / throttle / brake / steer_pct / steer_torque / throttle_cmd / brake_cmd /
  steer_cmd / engine_rpm / parking_brake / gear / mode`
- 追踪的轨迹：`seq / age / points / v0 / a0 / v_end / t_end / estop / replan`
- 输出命令：`throttle / brake / steer / steer_rate / speed / gear / latency_ms /
  latency_exceeded / status_msg`
- 横向控制内部量（`lat.*`，来自 `ControlCommand.debug().simple_lateral_debug()`，约 17 项）
- 纵向控制内部量（`lon.*`，约 32 项：目标速度/加速度、pid 各项、坡度、限幅等）

> 说明：本车 `control.conf` 里 `--use_control_submodules=false`，实际跑的是
> ControlComponent 内置的 LatPlusController / LonPlusController（预编译，无源码），
> 所以 control 侧的细节全部来自 `ControlCommand.debug()` 暴露的调试量，
> 组件层面再补 chassis / trajectory / 命令 / 时延。

---

## 5. 怎么看这些数据

### 5.1 录制

```bash
# 容器内，与平时一样录全量
cyber_recorder record -a
# 会自动包含 /apollo/edu_debug/planning 与 /apollo/edu_debug/control
cyber_recorder info <record> | grep edu_debug
```

### 5.2 离线展开

```bash
# 编译（一次即可）
cd /apollo_workspace && buildtool build -p modules/edu_debug --opt

# 展开
./bazel-bin/modules/edu_debug/tools/export_debug_record -f <record> -o ./edu_debug_export
```

输出结构：

```
edu_debug_export/
  planning/
    flags.txt              全部 gflag
    config_values.txt      组件配置 / planner 配置的 DebugString
    manifest_tree.tsv      配置树清单：path realpath size mtime sha1
    manifest.tsv           被单独 dump 的文件清单
    config/<name>          每个配置文件的内容（分片自动拼接）
    state/planning_output.jsonl
    state/planning_speed_state.jsonl
    events.jsonl           scenario_changed / config_file_changed
    trace/planning_speed_curve.txt
  control/
    flags.txt
    config_values.txt
    manifest_tree.tsv
    config/<name>
    state/control_state.jsonl
    events.jsonl
```

### 5.3 典型排查流程

**问题 A：速度没到设定值**

1. `flags.txt` 里看 `default_cruise_speed`、`planning_upper_speed_limit`；
   命令里是否有 `target_speed` 看 `state/planning_output.jsonl` 的 `cmd_target_speed`。
2. `state/planning_speed_state.jsonl` 逐个时间点看：
   - `cruise` vs `base_cruise` vs `st_cruise` 不一致 → 被 scenario/task 的
     `LimitCruiseSpeed` 或 ST 图压低；
   - `limit_s*` 很小 → 地图限速或曲率限速；
   - `stops`/`st_bounds` 非空、`blocker` 有值 → 障碍物/停止点压速；
   - `v_plan_max` 远小于 `cruise` 而 `limit` 不低 → 优化器/边界问题；
   - `v_traj0` 与 `v_plan_s0` 差很多 → 轨迹生成阶段掉速。
3. `trace/planning_speed_curve.txt` 看沿 s 的 `v_limit / v_plan / v_traj` 三条曲线形状，
   判断降速发生在哪个 s 区间（路口？弯道？障碍物？）。

**问题 B：改了配置没生效**

1. `manifest_tree.tsv` 里找到该文件，确认 `realpath` 是不是你以为的那个
   （`/apollo/modules/...` 指向 `profiles/HQEV/...`）；
2. `config/<name>` 里是该文件**被加载时的真实内容**，直接对比你的修改；
3. `sha1` 与磁盘上当前文件的 sha1 不一致 → 改的不是同一份文件，或模块没重启；
4. `events.jsonl` 里的 `config_file_changed` 给出文件在运行过程中被改动的时刻；
5. `flags.txt` 里看同名 gflag 是否被 flagfile 覆盖成了别的值。

**问题 C：控制跟不住 / 抖动**

1. `state/control_state.jsonl` 联查 `cycle_ms`（周期是否超时）、`trajectory.age`（轨迹是否陈旧）、
   `lat.*` / `lon.*` 内部量与最终 `throttle/brake/steer`；
2. `config_values.txt` 确认标定表、pipeline 与预期一致。

---

## 6. 构建与部署

### 6.1 编译（容器内）

```bash
cd /apollo_workspace
bash 中间脚本/build_edu_debug.sh
```

脚本依次执行：

```bash
buildtool build -p modules/control/control_component --opt -j 16
buildtool build -p modules/planning/planning_component --opt -j 16
```

### 6.2 重启模块（确认车辆静止/手动模式后）

```bash
cyber_launch stop /apollo/modules/planning/launch/planning.launch
cyber_launch stop /apollo/modules/control/launch/control.launch
cyber_launch start /apollo/modules/planning/launch/planning.launch
cyber_launch start /apollo/modules/control/launch/control.launch
```

### 6.3 备份

改动前已把远程原始文件备份为 `*.bak.20261027_212714`：

- `modules/planning/planning_component/planning_component.cc`
- `modules/planning/planners/public_road/public_road_planner.cc`
- `modules/control/control_component/control_component.cc`
- 三个 `BUILD` 文件（control_component / public_road / planning_component）

---

## 7. 已知限制

1. `modules/common_msgs` 只读，无法给 Apollo 原生 proto 加字段，因此采用独立 channel。
2. `ConfigUtil::LoadMergedConfig`（task 级 default+specific 合并）尚未打点，
   目前依靠 `DumpTree` 的完整配置树 + sha1 覆盖"加载了哪些文件、内容是什么"；
   若需要"每个 task 实际合并后的 proto"，后续可在
   `modules/planning/planning_base/common/util/config_util.h` 增加一处 DumpText。
3. control 的 LatPlus/LonPlus 控制器是预编译 .so，只能通过 `ControlCommand.debug()` 观察。
4. `DumpTree` 单进程上限 4000 个文件 / 单文件 4 MiB，超出会截断并在 manifest 里可见。
