#!/bin/sh
# 宿主上跑：起「真实感知 + 虚拟障碍」融合节点（可选带 aeb_shadow）
#
#   cd ~/apollo-edu_9.1.1/virtual_scene
#   ./start_virtual_scene.sh          # 只起融合
#   ./start_virtual_scene.sh --aeb    # 融合 + aeb_shadow 影子
#   ./start_virtual_scene.sh --force  # 车在动也强行重起
#
# 为什么要 -u apollo：
#   容器里默认是 root。root 起的 cyber 节点写出来的 shm 段 apollo 用户读不到，
#   DreamView / prediction 都会报 "get shm failed: Permission denied"，
#   页面上一个障碍物都看不见。2026-11-10 现场复现过。
#
# 为什么带看护（supervise_fusion.sh）：
#   prediction 已订阅 /apollo/perception/obstacles_fused，merge 一死 planning
#   就看到空世界。看护在它退出后 1 s 内自动拉起。
set -e
CT=apollo_neo_dev_edu_9.1.1
DIR=/apollo_workspace/virtual_scene
LOG=/apollo/data/log/virtual_fusion.log

AEB=0
FORCE=0
for a in "$@"; do
  case "$a" in
    --aeb)   AEB=1 ;;
    --force) FORCE=1 ;;
    *) echo "unknown arg: $a"; exit 2 ;;
  esac
done

# 安全闸：重起融合有 1~2 s 没有 fused，车在动时等于让 planning 短暂失明。
SPEED=$(docker exec -u apollo "$CT" bash -lc '
  source /opt/apollo/neo/setup.sh >/dev/null 2>&1
  timeout 4 cyber_channel echo /apollo/canbus/chassis 2>/dev/null \
    | sed -n "s/^speed_mps:[[:space:]]*//p" | head -1
' 2>/dev/null | tr -d '\r' | tail -1)
SPEED=${SPEED:-0}
if [ "$FORCE" -eq 0 ] && ! echo "$SPEED" | awk '{exit !($1 > 0.3)}'; then
  echo "⛔ 车速 ${SPEED} m/s：车在动，重起融合会让 prediction 短暂拿到空障碍。"
  echo "   停稳后再执行，或加 --force 强行重起。"
  exit 3
fi

# 旧的融合/影子/看护先停（老的可能是 root 起的，apollo 杀不掉，这里用 root 兜底）
docker exec "$CT" bash -lc 'pkill -f "supervise_fusion[.]sh" 2>/dev/null; pkill -f "merge_obstacles[.]py" 2>/dev/null; pkill -f "aeb_shadow[.]py" 2>/dev/null; true'
sleep 0.5

docker exec -u apollo "$CT" bash -lc "
  source /opt/apollo/neo/setup.sh >/dev/null 2>&1
  cd $DIR
  nohup sh supervise_fusion.sh >> $LOG 2>&1 &
  echo 'fusion supervisor pid='\$!
  sleep 1.5
"

if [ "$AEB" -eq 1 ]; then
  docker exec -u apollo "$CT" bash -lc "
    source /opt/apollo/neo/setup.sh >/dev/null 2>&1
    nohup python3 /apollo_workspace/modules/safety_manager/aeb_shadow/aeb_shadow.py \
        --config /apollo_workspace/modules/safety_manager/aeb_shadow/aeb_shadow_fused.json \
        >> $LOG 2>&1 &
    echo 'aeb_shadow pid='\$!
  "
fi

sleep 1
docker exec "$CT" bash -lc 'ps aux | grep -E "supervise_fusion[.]sh|merge_obstacles[.]py|aeb_shadow[.]py" | grep -v grep'
echo "日志: $LOG（容器内），宿主是 ~/apollo-edu_9.1.1/data/log/virtual_fusion.log"
