#!/bin/sh
# 车上一条命令刷新编辑器：读当前 live 地图 + 实时定位，重建页面。
#
#   ssh apollo@192.168.10.6
#   cd /home/apollo/apollo-edu_9.1.1/virtual_scene/scene_editor
#   ./refresh.sh            # 车停稳、定位正常时跑
#
# 出来的 scene_editor.html 就是 serve.sh 挂出去的那个页面。
set -e
cd "$(dirname "$0")"

MAP=/home/apollo/apollo-edu_9.1.1/data/map_data/st5/base_map.txt
[ -f "$MAP" ] || { echo "找不到 $MAP"; exit 1; }

echo "== 1/2 解析 live 地图 =="
python3 map_to_json.py "$MAP" st5 live 0 0 "" -o map_live.json

echo "== 2/2 读实时定位并重建页面 =="
# -u apollo：容器默认 root，root 跑会在仓库里留下 root 属主的 dumps/
# timeout 30：cyber 节点偶发不退出，兜一层
S=$(docker exec -u apollo apollo_neo_dev_edu_9.1.1 bash -lc \
    'source /opt/apollo/neo/setup.sh >/dev/null 2>&1; cd /apollo_workspace/virtual_scene/scene_editor && LANG=C.UTF-8 timeout 30 python3 ego_pose.py --bare' \
    2>/dev/null | tail -1)
case "$S" in
  ''|*[!0-9.]*) echo "⚠ 没读到定位（模块没起？），起点 s 保持默认"; python3 build_scene_editor.py map_live.json scene_editor.html ;;
  *)            python3 build_scene_editor.py map_live.json scene_editor.html --ego-s "$S" ;;
esac

# 让 http://<车>:8090/ 直接出编辑器，而不是目录列表
ln -sf scene_editor.html index.html
echo "页面: http://127.0.0.1:8090/  (serve.sh 起着的话)"
