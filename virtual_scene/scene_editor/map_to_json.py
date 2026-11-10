#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""把 Apollo base_map.txt（text proto）解析成编辑器用的 JSON。

输出: 远程/日志/分析输出/<日期>_<地图名>_lanes.json
结构: {header, origin, lanes:[{id, points:[[x,y]...], length, speed_limit,
        left_width:[...], right_width:[...], road_polygon:[[x,y]...]}], bbox}

用法: python -I map_to_json.py <base_map.txt> <地图名>
"""
import io
import json
import math
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTD = os.path.join(ROOT, u"远程", u"日志", u"分析输出")
# 本脚本也会被放到车上（/apollo_workspace/virtual_scene/scene_editor/）跑，
# 那里没有本地工作区的目录结构，输出就落在脚本旁边。
if not os.path.isdir(OUTD):
    OUTD = os.path.dirname(os.path.abspath(__file__))

src = sys.argv[1]
name = sys.argv[2] if len(sys.argv) > 2 else "map"
date = sys.argv[3] if len(sys.argv) > 3 else "map"
# 可选: dx dy tag —— 用于把 2026-11-09 19:58 备份（Map Editor 移位版）校正回实车在用的地图
dx = float(sys.argv[4]) if len(sys.argv) > 5 else 0.0
dy = float(sys.argv[5]) if len(sys.argv) > 5 else 0.0
tag = sys.argv[6] if len(sys.argv) > 6 else ""
if dx or dy:
    tag = tag or "corrected"
# 可选: -o <输出文件> —— 车上 refresh.sh 用它固定写到 map_live.json
OUT_OVERRIDE = None
if "-o" in sys.argv:
    OUT_OVERRIDE = sys.argv[sys.argv.index("-o") + 1]

with io.open(src, "r", encoding="utf-8", errors="replace") as f:
    text = f.read()


def blocks(t, keyword):
    """返回 keyword { ... } 的所有最外层块（允许缩进，按大括号配平）。"""
    out = []
    for m in re.finditer(r"^[ \t]*%s \{" % keyword, t, re.M):
        i = m.end() - 1
        depth = 0
        for j in range(i, len(t)):
            if t[j] == "{":
                depth += 1
            elif t[j] == "}":
                depth -= 1
                if depth == 0:
                    out.append((m.start(), t[i + 1:j]))
                    break
    return out


def points_of(block):
    """只取 line_segment 里的点。注意别把后面的 start_position {x,y} 也吞进来
    （那会把首点重复成末点，折线长度凭空多一倍）。"""
    pts = []
    for _p, ls in blocks(block, "line_segment"):
        pts.extend((float(a), float(b)) for a, b in
                   re.findall(r"x:\s*(-?[\d.eE+]+)\s*\n\s*y:\s*(-?[\d.eE+]+)", ls))
    return pts


def poly_len(pts):
    return sum(math.hypot(pts[i + 1][0] - pts[i][0], pts[i + 1][1] - pts[i][1])
               for i in range(len(pts) - 1))


def widths(blk, keyword):
    """一个车道里 keyword(left_sample/right_sample) 会有很多块，把 width 全收上来。"""
    return [float(w) for _p, b in blocks(blk, keyword)
            for w in re.findall(r"width:\s*(-?[\d.eE+]+)", b)]


hdr = re.search(r'version:\s*"([^"]*)"', text)
lanes = []
for _pos, blk in blocks(text, "lane"):
    lid = re.search(r'id:\s*"(Lane_[^"]*)"', blk)
    if not lid:
        continue
    cc = blocks(blk, "central_curve")
    pts = points_of(cc[0][1]) if cc else []
    lw = widths(blk, "left_sample")
    rw = widths(blk, "right_sample")
    lanes.append({
        "id": lid.group(1),
        "points": pts,
        "length": float(re.search(r"\n  length:\s*(-?[\d.eE+]+)", blk).group(1))
        if re.search(r"\n  length:\s*(-?[\d.eE+]+)", blk) else poly_len(pts),
        "speed_limit": float(re.search(r"speed_limit:\s*(-?[\d.eE+]+)", blk).group(1))
        if re.search(r"speed_limit:\s*(-?[\d.eE+]+)", blk) else None,
        "type": (re.search(r"\n  type:\s*(\w+)", blk).group(1)
                 if re.search(r"\n  type:\s*(\w+)", blk) else ""),
        "n_width_samples": len(lw),
        "width_min": min(lw) if lw else None,
        "width_max": max(lw) if lw else None,
        "left_width": [lw[0], lw[len(lw) // 2], lw[-1]] if lw else [],
        "right_width": [rw[0], rw[len(rw) // 2], rw[-1]] if rw else [],
    })

# 车道外边界多边形（road/section/boundary/outer_polygon/edge）
polys = []
for _pos, blk in blocks(text, "road"):
    for _p2, sec in blocks(blk, "section"):
        for _p3, bd in blocks(sec, "outer_polygon"):
            for _p4, edge in blocks(bd, "edge"):
                etype = re.search(r"type:\s*(\w+)", edge)
                polys.append({"type": etype.group(1) if etype else "",
                              "points": points_of(edge)})

xs = [p[0] for l in lanes for p in l["points"]]
ys = [p[1] for l in lanes for p in l["points"]]

if dx or dy:
    for l in lanes:
        l["points"] = [[x + dx, y + dy] for x, y in l["points"]]
    for b in polys:
        b["points"] = [[x + dx, y + dy] for x, y in b["points"]]
    xs = [x + dx for x in xs]
    ys = [y + dy for y in ys]

origin = [min(xs), min(ys)] if xs else [0, 0]

data = {
    "map": name,
    "version": hdr.group(1) if hdr else "",
    "applied_shift": [dx, dy] if (dx or dy) else None,
    "origin": origin,
    "lanes": lanes,
    "road_boundaries": polys,
    "bbox": [min(xs), min(ys), max(xs), max(ys)] if xs else None,
}

out = OUT_OVERRIDE or os.path.join(OUTD, u"%s_%s_lanes%s.json"
                                   % (date, name, ("_" + tag) if tag else ""))
with io.open(out, "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False)

print("lanes=%d  boundaries=%d" % (len(lanes), len(polys)))
for l in lanes:
    p = l["points"]
    print("  %-10s pts=%-4d len=%.2f m  speed_limit=%.2f  type=%s"
          % (l["id"], len(p), l["length"], l["speed_limit"] or 0, l["type"]))
    if p:
        print("      start (%.2f, %.2f)  end (%.2f, %.2f)"
              % (p[0][0], p[0][1], p[-1][0], p[-1][1]))
    if l["left_width"]:
        print("      left  width %.4f / %.4f / %.4f"
              % (l["left_width"][0], l["left_width"][1], l["left_width"][-1]))
        print("      right width %.4f / %.4f / %.4f"
              % (l["right_width"][0], l["right_width"][1], l["right_width"][-1]))
print("bbox x[%.2f, %.2f] y[%.2f, %.2f]  origin=(%.2f, %.2f)"
      % (data["bbox"][0], data["bbox"][2], data["bbox"][1], data["bbox"][3],
         origin[0], origin[1]))
print("written %s" % out)
