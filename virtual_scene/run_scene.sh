#!/bin/sh
# 宿主上跑：一条命令把编辑器导出的整个场景起来（所有演员同一个进程、同一个 t0）
#
#   cd ~/apollo-edu_9.1.1/virtual_scene
#   ./run_scene.sh scene.json              # 文件
#   ./run_scene.sh <<'SCENE'               # 编辑器导出的命令块直接粘进来
#   { ... scene-editor/1 JSON ... }
#   SCENE
#
# 为什么不用一条演员一条命令：多进程的 t0 是各自进程启动时刻，人手粘命令之间
# 差几秒，几个演员的时序就散了。这里把 JSON 整个塞给一个注入器进程，
# 同时起、同时算。
set -e
CT=apollo_neo_dev_edu_9.1.1
if [ -n "$1" ] && [ -f "$1" ]; then
  B64=$(base64 -w0 "$1")
else
  B64=$(base64 -w0)
fi
if [ -z "$B64" ]; then
  echo "用法: ./run_scene.sh <scene.json>   或   ./run_scene.sh <<'SCENE' ... SCENE" >&2
  exit 2
fi
exec docker exec -u apollo "$CT" bash -lc '
  source /opt/apollo/neo/setup.sh >/dev/null 2>&1
  cd /apollo_workspace/virtual_scene
  exec python3 -u pedestrian_injector.py --scene-b64 "$0" --channel /apollo/virtual/obstacles
' "$B64"
