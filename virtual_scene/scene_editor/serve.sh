#!/bin/sh
# 在车上（宿主机，不是容器里）把场景编辑器挂到局域网。
# 宿主机 python3 = 3.6.9，够用；页面是纯静态的单文件 HTML，不需要任何后端。
#
#   ssh apollo@192.168.10.6
#   cd /home/apollo/apollo-edu_9.1.1/virtual_scene/scene_editor
#   ./serve.sh            # 前台跑，Ctrl-C 停
#   ./serve.sh 8091       # 换端口
#
# 然后在笔记本浏览器打开 http://192.168.10.6:8090/
set -e
cd "$(dirname "$0")"
PORT="${1:-8090}"

# 页面和地图快照是生成物，不进 git；没有就先建一份
if [ ! -f scene_editor.html ]; then
  echo "没有 scene_editor.html，先跑 refresh.sh"
  ./refresh.sh
fi

echo "serving $(pwd) on http://0.0.0.0:$PORT/"
echo "笔记本访问: http://192.168.10.6:$PORT/"
exec python3 -m http.server "$PORT" --bind 0.0.0.0
