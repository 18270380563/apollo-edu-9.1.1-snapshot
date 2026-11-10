#!/bin/sh
# 宿主上跑：停掉虚拟场景相关的所有进程（看护 / 融合 / 注入器 / aeb_shadow）
#
# 注意 pkill 用 root 执行：老的注入器可能是 root 起的。
# 模式里用 [.] 防止把自己这条 ssh 命令行也匹配掉（11-10 踩过，直接断了会话）。
#
# 顺序：先杀看护（supervise_fusion.sh），再杀 merge——反过来的话看护会在 1 s 后
# 把刚杀掉的 merge 又拉起来。
set -e
CT=apollo_neo_dev_edu_9.1.1
docker exec "$CT" bash -lc '
  pkill -f "supervise_fusion[.]sh" 2>/dev/null || true
  sleep 0.3
  pkill -f "merge_obstacles[.]py" 2>/dev/null || true
  pkill -f "pedestrian_injector[.]py" 2>/dev/null || true
  pkill -f "aeb_shadow[.]py" 2>/dev/null || true
  sleep 0.5
  echo "剩余："
  ps aux | grep -E "supervise_fusion[.]sh|merge_obstacles[.]py|pedestrian_injector[.]py|aeb_shadow[.]py" | grep -v grep || echo "  （无）"
'
