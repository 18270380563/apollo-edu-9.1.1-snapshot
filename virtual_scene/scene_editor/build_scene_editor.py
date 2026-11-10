#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""把地图 JSON 注入编辑器模板，生成一个可双击打开的独立 HTML。

用法:
    python -I build_scene_editor.py [地图 JSON] [输出 HTML] [--ego-s 数值]

    --ego-s：把编辑器默认的「起点 s」设成给定值。车上用 ego_pose.py 读实时定位
             拿这个值，页面一打开自车就站在真实位置，导出的 --ahead 才对。

默认:
    地图 JSON = 远程/日志/分析输出/20261110_st5_lanes_corrected.json
    输出      = 中间脚本/scene_editor.html

为什么内嵌而不是 fetch：浏览器在 file:// 下禁止 fetch 本地文件，
内嵌之后双击就能用，不需要起 http server。
"""
import io
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
TPL = os.path.join(HERE, u"scene_editor_template.html")
if not os.path.isfile(TPL):
    TPL = os.path.join(ROOT, u"中间脚本", u"scene_editor_template.html")
DEF_MAP = os.path.join(ROOT, u"远程", u"日志", u"分析输出",
                       u"20261110_st5_lanes_corrected.json")
if not os.path.isfile(DEF_MAP):
    DEF_MAP = os.path.join(HERE, u"map_live.json")
if not os.path.isfile(DEF_MAP):
    DEF_MAP = os.path.join(HERE, u"20261110_st5_lanes_live.json")
DEF_OUT = os.path.join(HERE, u"scene_editor.html")

argv = [a for a in sys.argv[1:] if not a.startswith("--")]
ego_s = None
for i, a in enumerate(sys.argv):
    if a == "--ego-s":
        if i + 1 >= len(sys.argv):
            raise SystemExit("--ego-s 后面要跟一个数值")
        ego_s = float(sys.argv[i + 1])
    elif a.startswith("--ego-s="):
        ego_s = float(a.split("=", 1)[1])

map_path = argv[0] if len(argv) > 0 else DEF_MAP
out_path = argv[1] if len(argv) > 1 else DEF_OUT

with io.open(map_path, "r", encoding="utf-8") as f:
    data = json.load(f)

# 压缩成一行，省体积
payload = json.dumps(data, ensure_ascii=False, separators=(",", ":"))

with io.open(TPL, "r", encoding="utf-8") as f:
    tpl = f.read()

if "__MAP_JSON__" not in tpl:
    raise SystemExit("模板里找不到 __MAP_JSON__ 占位符")

html = tpl.replace("__MAP_JSON__", payload)

if ego_s is not None:
    html, n = re.subn(r"(ego:\s*\{\s*s0:\s*)-?[\d.]+", r"\g<1>%s" % ego_s, html, count=1)
    if n != 1:
        raise SystemExit("模板里没找到 ego.s0 默认值，改不了起点")

with io.open(out_path, "w", encoding="utf-8") as f:
    f.write(html)

print("map   : %s" % map_path)
print("shift : %s" % (data.get("applied_shift"),))
print("lanes : %d" % len(data.get("lanes") or []))
if ego_s is not None:
    print("ego s : %.1f" % ego_s)
print("out   : %s  (%.1f KB)" % (out_path, len(html.encode("utf-8")) / 1024.0))
