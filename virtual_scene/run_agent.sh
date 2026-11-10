#!/bin/sh
# 宿主上跑：起一个虚拟障碍注入器（= 剧本里的一个演员）
#
#   ./run_agent.sh --id 9901 --ahead 45 --lateral 0 --mode cross --dir left \
#                  --speed 1.2 --trigger-dist 14 --width 0.7 --length 0.5 --height 1.75
#
# 和直接 docker exec 的区别，就三件事：
#   1. -u apollo —— 否则写出来的 shm 段 DreamView/prediction 读不到
#   2. 先 source setup.sh —— 否则 LIB_PATH/APOLLO_CONF_PATH 缺失会静默回落
#   3. 强制 --channel /apollo/virtual/obstacles —— 绝不会和真实感知抢同一个话题
#
# 想覆盖话题：自己再传一个 --channel，argparse 取最后一个。
set -e
CT=apollo_neo_dev_edu_9.1.1
exec docker exec -u apollo "$CT" bash -lc '
  source /opt/apollo/neo/setup.sh >/dev/null 2>&1
  cd /apollo_workspace/virtual_scene
  exec python3 -u pedestrian_injector.py --channel /apollo/virtual/obstacles "$@"
' _ "$@"
