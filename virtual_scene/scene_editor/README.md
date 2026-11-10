# 虚拟场景剧本编辑器（车上版）

把「横穿行人 / 静态障碍 / 前车」这类剧本画在 st5 车道上，导出成
`pedestrian_injector.py` 的命令行。整个东西是**一个静态 HTML**，没有后端，
地图和几何全部内嵌在页面里。

- 页面入口：`scene_editor.html`（由 `scene_editor_template.html` + `map_live.json` 生成）
- 地图来源：`/home/apollo/apollo-edu_9.1.1/data/map_data/st5/base_map.txt`
  （= 容器里的 `/apollo/modules/map/data/st5/`，就是 planning 真正在用的那份）
- 定位来源：容器内 `/apollo/localization/pose`

## 三条命令

```sh
ssh apollo@192.168.10.6
cd /home/apollo/apollo-edu_9.1.1/virtual_scene/scene_editor

./refresh.sh      # 读 live 地图 + 实时定位，重建 scene_editor.html
./serve.sh        # 起静态服务器，默认 8090；Ctrl-C 停
./serve.sh 8091   # 换端口
```

然后笔记本浏览器打开 `http://192.168.10.6:8090/`。

`refresh.sh` 干两件事：

1. 用 `map_to_json.py` 把 live `base_map.txt` 解析成 `map_live.json`
   （车道中线、限速、左右宽度采样、道路边界多边形）；
2. 在容器里跑 `ego_pose.py --bare` 读实时定位，投影到 Lane_1 上得到
   「自车当前在车道上的 s」，用 `--ego-s` 写进页面默认值。

**为什么要第 2 步**：编辑器导出的注入器命令用 `--ahead`，而 `--ahead` 是相对
**启动时自车位姿**算的。自车停在哪条车道位置只有定位知道，猜一个会让整条命令
偏出几十米。

⚠ **起点 s 是快照，不是活数据**：`refresh.sh` 读的是它跑那一刻的定位。
车一开，页面里的起点就过时了。所以流程是：**把车停到准备起跑的位置 → 跑
`refresh.sh` → 再打开页面画剧本**，不要隔一轮跑车还拿旧页面导命令。

页面会把上次编辑的内容存在浏览器 localStorage 里，但**起点 s 每次都以页面里
烘焙的值（= refresh.sh 读到的定位）为准**，不会被旧存档盖回去。

## 页面怎么用

1. 打开就是 st5 的 Lane_1（180 m，半宽 3.32 m），自车停在 `s=当前定位值`。
2. 左侧按钮加演员：横穿行人 / 静态行人 / 静态障碍 / 前车。画布上拖就能挪（s、l 都跟着变）。
3. 右侧改参数：起步 s、横向 l0、速度、横向跨度 span、触发方式（时间 / 距离）。
4. 右下角「校验」即时给结论：会不会撞、会不会绕行、激活时刻是否晚于场景时长。
5. 「导出」三个页签：
   - **注入器命令**：直接粘到容器里跑；
   - **Scenario JSON**：`sim_obstacle` 的场景格式（本车 sim_obstacle 卡在等 routing response，暂跑不了）；
   - **场景 JSON**：编辑器自己的格式，可以粘回去继续改。

## 硬约束（都在校验里报了）

| 约束 | 数值 | 依据 |
|---|---|---|
| `--channel` 必须显式写 | `/apollo/perception/obstacles` | 漏写 = 注入器与真实感知两个写者轮流覆盖，planning 看到交替的两份世界（2026-11-09 横穿行人的根因） |
| 可绕行障碍宽度 | ≤ 2.60 m | 车道半宽 3.32、路径边界 ±2.31、自车半宽 1.01 |
| 距离触发 | 欧氏距离到**演员起点** | `pedestrian_injector.py` 按 `hypot(live - origin) <= --trigger-dist` 判 |

## 容器里跑注入器的姿势

```sh
docker exec -it apollo_neo_dev_edu_9.1.1 bash
# 必须是 apollo 用户 + 完整 env，否则 shm 段别的进程读不到
source /opt/apollo/neo/setup.sh
cd /apollo_workspace/virtual_scene
python3 ./pedestrian_injector.py --ahead 45 --lateral -3.4 --mode cross \
  --speed 1.2 --trigger-dist 14 --width 0.7 --length 0.5 --height 1.75 \
  --duration 30 --rate 10 --channel /apollo/perception/obstacles
```

## 已知限制

- 只支持单一车道（st5 就一条 Lane_1）。多车道地图要在 `map_to_json.py` 里挑车道。
- 自车模型是一维运动学（匀加速到巡航速度后匀速），不考虑横向、不考虑实际控制跟踪误差。
- 页面里画的是**该跑什么**，不是**跑了什么**。跑了什么要回放 `.record` 比对。
