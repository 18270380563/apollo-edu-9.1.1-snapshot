#!/usr/bin/env bash
# 停止虚拟场景（融合节点/AEB shadow），必要时恢复真实感知
set -o pipefail
RUN=/tmp/aeb_scene
source /opt/apollo/neo/setup.sh

pkill -9 -f "sim_obstacle .*scene_live.json" 2>/dev/null
pkill -9 -f "merge_obstacles.py" 2>/dev/null
pkill -9 -f "aeb_shadow.py" 2>/dev/null
pkill -9 -f "pedestrian_injector.py" 2>/dev/null
echo "[1/3] 已停 sim_obstacle / injector / merge_obstacles / aeb_shadow"

restored=0
for f in "$RUN"/perception_*.cmd; do
  [[ -f "$f" ]] || continue
  restored=1
  if pgrep -f "mainboard -p perception" > /dev/null; then
    echo "[2/3] 真实感知本来就在跑，无需恢复"
    break
  fi
  CMD=$(cat "$f")
  cd /apollo || exit 1
  setsid nohup $CMD > /apollo/data/log/perception_restore.out 2>&1 < /dev/null &
  echo "[2/3] 已按原命令恢复感知: ${CMD:0:70}..."
  rm -f "$f"
  break
done
[[ "$restored" == "0" ]] && echo "[2/3] 融合模式没停过感知，无需恢复"

sleep 3
if pgrep -f "mainboard -p perception" > /dev/null; then
  echo "[3/3] ✔ 感知进程在跑"
else
  echo "[3/3] ✘ 感知没起来，检查 /apollo/data/log/perception_restore.out"
fi
echo "残留 sim_obstacle: $(pgrep -c -f sim_obstacle || true)"
