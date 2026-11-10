#!/bin/sh
# 融合节点看护（容器内、apollo 用户下跑；由 start_virtual_scene.sh 拉起）
#
# 为什么要它：2026-11-10 起 prediction 的输入已经切到
# /apollo/perception/obstacles_fused。merge_obstacles.py 一死，planning 看到的
# 就是"空世界"，而且没有任何报错——这是新的单点。这里在它退出后 1 s 内拉起来。
#
# 停止：stop_virtual_scene.sh 先杀本脚本、再杀 merge_obstacles.py（顺序不能反，
# 否则看护会把刚杀掉的 merge 又拉起来）。
set -e
cd /apollo_workspace/virtual_scene
LOG=/apollo/data/log/virtual_fusion.log
HZ=${FUSION_RATE:-10}

while true; do
  python3 -u merge_obstacles.py \
      --real /apollo/perception/obstacles \
      --virtual /apollo/virtual/obstacles \
      --out /apollo/perception/obstacles_fused \
      --rate "$HZ" --ttl 0.6 --real-ttl 1.0 >> "$LOG" 2>&1 || true
  echo "[$(date '+%F %T')] merge exited, restart in 1s" >> "$LOG"
  sleep 1
done
