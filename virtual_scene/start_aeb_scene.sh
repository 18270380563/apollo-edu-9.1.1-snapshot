#!/usr/bin/env bash
# st5 虚拟场景一键启动（AEB 虚实结合）
#
# 默认 fuse 模式：真实感知照常跑，虚拟障碍物叠加到融合通道
#   真实 /apollo/perception/obstacles ─┐
#                                       ├─► merge_obstacles ─► /apollo/perception/obstacles_fused
#   虚拟 /apollo/virtual/obstacles ────┘
#
#   ./start_aeb_scene.sh                     # fuse（推荐，不停感知）
#   ./start_aeb_scene.sh --with-aeb          # fuse + 起 aeb_shadow 指向融合通道
#   ./start_aeb_scene.sh --replace           # 旧的替换模式：停感知，只留虚拟
#   ./start_aeb_scene.sh --dry-run           # 只写测试通道，什么都不影响
set -o pipefail

MODE=fuse; AHEAD=40; MODE_PED=cross; SPEED=1.2; DELAY=5; WITH_AEB=0; SPAN=5; PED_SRC=sim_obstacle; TRIG=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --ahead)    AHEAD="$2"; shift 2;;
    --ped)      MODE_PED="$2"; shift 2;;
    --span)     SPAN="$2"; shift 2;;
    --speed)    SPEED="$2"; shift 2;;
    --delay)    DELAY="$2"; shift 2;;
    --trigger-dist) TRIG="$2"; shift 2;;   # 车进到行人 start 点这么近才开始走（0=按时间）
    --replace)  MODE=replace; shift;;
    --dry-run)  MODE=dry; shift;;
    --with-aeb) WITH_AEB=1; shift;;
    *) echo "unknown arg: $1"; exit 2;;
  esac
done

DIR=/apollo_workspace/virtual_scene
RUN=/tmp/aeb_scene
mkdir -p "$RUN"
source /opt/apollo/neo/setup.sh
cd "$DIR" || exit 1

pkill -9 -f "sim_obstacle .*scene_live.json" 2>/dev/null
pkill -9 -f merge_obstacles 2>/dev/null
sleep 0.5

# 1. 位姿 + 场景
timeout 6 stdbuf -oL cyber_channel echo /apollo/localization/pose 2>/dev/null | head -40 > "$RUN/pose.txt"
X=$(grep -m1 -A3 "position {" "$RUN/pose.txt" | grep "x:" | head -1 | awk '{print $2}')
Y=$(grep -m1 -A3 "position {" "$RUN/pose.txt" | grep "y:" | head -1 | awk '{print $2}')
[[ -z "${X:-}" || -z "${Y:-}" ]] && { echo "[FAIL] 读不到定位"; exit 3; }
echo "[1/4] 车辆位姿 x=$X y=$Y"

if [[ "$MODE_PED" == "pingpong" ]]; then
  echo "[2/4] pingpong 模式：用注入器往复横穿，不需要场景 json"
else
  python3 make_ped_scenario.py --ego-x "$X" --ego-y "$Y" --ahead "$AHEAD" \
          --mode "$MODE_PED" --speed "$SPEED" --delay "$DELAY" --out "$DIR/scene_live.json" \
    || { echo "[FAIL] 场景生成失败"; exit 4; }
  echo "[2/4] 场景已生成（车前 ${AHEAD}m / ${MODE_PED} / ${SPEED}m/s / 第${DELAY}s 出现）"
fi

# 2. 起虚拟障碍
case "$MODE" in
  fuse)
    VIRT=/apollo/virtual/obstacles; OUT=/apollo/perception/obstacles_fused
    if [[ "$MODE_PED" == "pingpong" ]]; then
      # 往复横穿用注入器（官方 tracked agent 只走一次，且车停着不推进）
      setsid nohup python3 -u "$DIR/pedestrian_injector.py" --ahead "$AHEAD" \
          --mode pingpong --span "$SPAN" --speed "$SPEED" --start-delay "$DELAY" \
          --trigger-dist "$TRIG" --channel "$VIRT" > "$RUN/injector.log" 2>&1 < /dev/null &
      PED_SRC=injector
    else
      setsid nohup /opt/apollo/neo/bin/sim_obstacle "$DIR/scene_live.json" \
          --publish_routing=true --perception_obstacle_topic="$VIRT" \
          > "$RUN/sim_obstacle.log" 2>&1 < /dev/null &
      PED_SRC=sim_obstacle
    fi
    setsid nohup python3 "$DIR/merge_obstacles.py" --virtual "$VIRT" --out "$OUT" \
        > "$RUN/fusion.log" 2>&1 < /dev/null &
    echo "[3/4] 融合模式：真实感知保持运行，虚拟障碍($PED_SRC) → $VIRT，融合结果 → $OUT"
    ;;
  replace)
    VIRT=/apollo/perception/obstacles; OUT=$VIRT
    if pgrep -f "mainboard -p perception" > /dev/null; then
      for p in $(pgrep -f "mainboard -p perception"); do
        tr '\0' ' ' < "/proc/$p/cmdline" > "$RUN/perception_$p.cmd"
      done
      pkill -f "mainboard -p perception"; sleep 2
      echo "[3/4] 替换模式：已停真实感知（命令存于 $RUN/perception_*.cmd）"
    fi
    setsid nohup /opt/apollo/neo/bin/sim_obstacle "$DIR/scene_live.json" \
        --publish_routing=true --perception_obstacle_topic="$VIRT" \
        > "$RUN/sim_obstacle.log" 2>&1 < /dev/null &
    ;;
  dry)
    VIRT=/apollo/test/sim_obstacles; OUT=$VIRT
    setsid nohup /opt/apollo/neo/bin/sim_obstacle "$DIR/scene_live.json" \
        --publish_routing=true --perception_obstacle_topic="$VIRT" \
        > "$RUN/sim_obstacle.log" 2>&1 < /dev/null &
    echo "[3/4] DRY：真实感知不动，虚拟障碍 → $VIRT"
    ;;
esac

# 3. 等 routing 握手（注入器模式不需要，只要 3s）
if [[ "${PED_SRC:-}" == "injector" ]]; then
  echo "[4/4] 注入器模式：无需 routing 握手，等待 5s 确认输出..."
  sleep 5
  if ! pgrep -f "pedestrian_injector.py" > /dev/null; then
    echo "      ✘ 注入器退出，日志："; tail -6 "$RUN/injector.log"; exit 5
  fi
else
  echo "[4/4] 等待 routing 握手与 agent 启动（约 20s）..."
  sleep 20
  if ! pgrep -f "sim_obstacle .*scene_live.json" > /dev/null; then
    echo "      ✘ sim_obstacle 退出，日志："
    grep -viE "bvar|forward jumps" "$RUN/sim_obstacle.log" | tail -6
    exit 5
  fi
fi

# 4. 可选：aeb_shadow 指向融合通道
if [[ "$WITH_AEB" == "1" ]]; then
  if [[ "$MODE" == "dry" ]]; then
    echo "      ⚠ --with-aeb 在 dry 模式下忽略"
  else
    CONF=$RUN/aeb_shadow_fused.json
    sed "s#\"obstacles_channel\": *\"[^\"]*\"#\"obstacles_channel\": \"$OUT\"#" \
        /apollo_workspace/modules/safety_manager/aeb_shadow/aeb_shadow.conf.json > "$CONF"
    pkill -9 -f "aeb_shadow.py" 2>/dev/null
    setsid nohup python3 /apollo_workspace/modules/safety_manager/aeb_shadow/aeb_shadow.py \
        --config "$CONF" > "$RUN/aeb_shadow.log" 2>&1 < /dev/null &
    echo "      ✔ aeb_shadow 已启动，读取 $OUT"
  fi
fi

sleep 2
echo
echo "通道：$OUT"
echo "  看障碍物: cyber_channel echo $OUT | grep -E 'id:|type:' | head -30"
[[ "$WITH_AEB" == "1" ]] && echo "  看 AEB  : cyber_channel echo /apollo/safety/aeb_shadow_status"
echo "  停止    : $DIR/stop_aeb_scene.sh"
