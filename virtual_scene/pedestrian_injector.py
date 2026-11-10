#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""虚拟场景注入器（AEB / planning 虚实结合测试用，不依赖 routing / sim_obstacle）

两种用法：

1) 单个演员（命令行参数，老用法不变）：
     # 车头前方 30 m 站一个行人，5 秒后开始以 1.5 m/s 朝车走来
     python3 pedestrian_injector.py --ahead 30 --mode toward --speed 1.5 --start-delay 5
     # 距离触发：车开到离行人 30 m 以内，行人才开始走
     python3 pedestrian_injector.py --ahead 45 --trigger-dist 30 --mode cross --speed 1.2

2) 整个场景（编辑器导出的 scene-editor/1 JSON，一次起全部演员）：
     python3 pedestrian_injector.py --scene-file scene.json
     python3 pedestrian_injector.py --scene-b64 "$(base64 -w0 scene.json)"
   关键是所有演员在**同一个进程、同一时刻 t0** 起，多演员时序不会因为人一条条
   粘命令而漂移；一个进程可以发多个 id，也就没有节点名冲突。

发布的话题默认 /apollo/perception/obstacles；融合链路里由 run_agent.sh / run_scene.sh
固定成 /apollo/virtual/obstacles（别再往真感知通道写）。
"""

import argparse
import base64
import io
import json
import math
import os
import sys
import time

from cyber.python.cyber_py3 import cyber
from modules.common_msgs.localization_msgs import localization_pb2
from modules.common_msgs.perception_msgs import perception_obstacle_pb2

# 容器里 LANG 常是 C，stdout 按 ascii 编码，--help / 参数报错里的中文会直接抛
# UnicodeEncodeError，盖掉真正想看的提示。统一成 utf-8。
try:
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8",
                                  errors="replace", line_buffering=True)
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8",
                                  errors="replace", line_buffering=True)
except Exception:
    pass

OBSTACLE_ID = 9901

TYPE_ENUM = {
    "pedestrian": perception_obstacle_pb2.PerceptionObstacle.PEDESTRIAN,
    "vehicle": perception_obstacle_pb2.PerceptionObstacle.VEHICLE,
    "cyclist": perception_obstacle_pb2.PerceptionObstacle.BICYCLE,
    "unknown_unmovable": perception_obstacle_pb2.PerceptionObstacle.UNKNOWN_UNMOVABLE,
    "unknown_movable": perception_obstacle_pb2.PerceptionObstacle.UNKNOWN_MOVABLE,
}

# 编辑器 JSON 里的 type 是枚举名
SCENE_TYPE = {
    "PEDESTRIAN": "pedestrian",
    "VEHICLE": "vehicle",
    "BICYCLE": "cyclist",
    "UNKNOWN_UNMOVABLE": "unknown_unmovable",
    "UNKNOWN_MOVABLE": "unknown_movable",
}


def read_first(channel, msg_type, timeout=5.0):
    box = {}
    node = cyber.Node("ped_inject_probe_%d" % int(time.time() * 1000))
    node.create_reader(channel, msg_type, lambda m: box.__setitem__("m", m))
    deadline = time.time() + timeout
    while "m" not in box and time.time() < deadline:
        time.sleep(0.02)
    return box.get("m")


def make_actor(**kw):
    a = {
        "id": OBSTACLE_ID, "name": "", "type": "pedestrian",
        "mode": "toward", "dir": "left", "ahead": 30.0, "lateral": 0.0,
        "speed": 1.5, "span": 4.0, "cross_cap": None,
        "start_delay": 0.0, "trigger_dist": 0.0,
        "duration": None, "w": 0.7, "l": 0.5, "h": 1.75,
    }
    a.update(kw)
    return a


def actors_from_scene(scene, ego_s0, args):
    """把编辑器导出的 scene-editor/1 JSON 展开成演员列表。"""
    agents = scene.get("agents") or []
    if not agents:
        raise ValueError("场景里没有演员")
    scene_dur = scene.get("duration") or 0.0
    out = []
    for ag in agents:
        t = SCENE_TYPE.get(ag.get("type"), "pedestrian")
        motion = ag.get("motion", "static")
        direction = ag.get("dir", 1)
        mode = motion
        mode_dir = "left" if direction > 0 else "right"
        back = (motion == "along" and direction < 0)
        span = abs(float(ag.get("span", 4.0) or 4.0))
        trigger = ag.get("trigger", "time")
        if trigger == "distance":
            trig_d = float(ag.get("trigDist", 0.0) or 0.0)
            delay = 0.0
        else:
            trig_d = 0.0
            delay = float(ag.get("tStart", 0.0) or 0.0)
        dur = scene_dur if scene_dur > 0 else None
        if args.duration is not None:
            dur = args.duration
        out.append(make_actor(
            id=int(ag["id"]),
            name=ag.get("name", ""),
            type=t,
            mode=mode,
            dir=mode_dir,
            ahead=float(ag["s0"]) - ego_s0,
            lateral=float(ag["l0"]),
            speed=float(ag.get("speed", 1.5) or 0.0),
            span=span,
            cross_cap=(span if motion == "cross" else None),
            back=back,
            start_delay=delay,
            trigger_dist=trig_d,
            duration=dur,
            w=float(ag.get("w", 0.7)),
            l=float(ag.get("l", 0.5)),
            h=float(ag.get("h", 1.75)),
        ))
    # id 撞车 = 后面的演员会被融合节点按 id 去重吃掉
    ids = [a["id"] for a in out]
    if len(ids) != len(set(ids)):
        raise ValueError("场景里 actor id 重复：%s" % ids)
    return out


def actor_origin(actor, ego):
    h = ego["heading"]
    return {
        "x": ego["x"] + actor["ahead"] * math.cos(h) - actor["lateral"] * math.sin(h),
        "y": ego["y"] + actor["ahead"] * math.sin(h) + actor["lateral"] * math.cos(h),
    }


def actor_pose(actor, origin, move_t, ego_heading):
    """返回 (px, py, vx, vy, theta)。move_t 是"开始动之后"的秒数。

    运动学与编辑器 scene_editor.html 里的 agentPose() 一致（2026-11-10 对齐）：
      cross     横穿到 l0 ± span 就停住，不会一直走下去
      pingpong  在 l0 与 l0 ± span 之间三角波往复（页面里画出来什么样就什么样）
      along     dir<0（back）时沿车头反方向走，不是朝车走
    """
    h = ego_heading
    mode = actor["mode"]
    if mode == "static":
        return origin["x"], origin["y"], 0.0, 0.0, h

    v = max(0.05, actor["speed"])
    sign = 1.0 if actor["dir"] == "left" else -1.0
    left_x, left_y = -math.sin(h), math.cos(h)   # 车体左向

    if mode == "cross":
        max_l = abs(actor["cross_cap"]) if actor.get("cross_cap") else float("inf")
        moved = min(max_l, v * move_t)
        lat = sign * moved
        px = origin["x"] + left_x * lat
        py = origin["y"] + left_y * lat
        still = moved >= max_l
        vx = 0.0 if still else left_x * v * sign
        vy = 0.0 if still else left_y * v * sign
        return px, py, vx, vy, h + sign * math.pi / 2

    if mode == "pingpong":
        span = max(0.1, abs(actor["span"]))
        period = 2.0 * span / v
        ph = (move_t % period) / period
        tri = ph * 2.0 if ph < 0.5 else (1.0 - ph) * 2.0
        lat = sign * span * tri
        px = origin["x"] + left_x * lat
        py = origin["y"] + left_y * lat
        out = 1.0 if ph < 0.5 else -1.0
        vx = left_x * v * sign * out
        vy = left_y * v * sign * out
        return px, py, vx, vy, h + sign * out * math.pi / 2

    if mode == "along":
        sgn = -1.0 if actor.get("back") else 1.0
        px = origin["x"] + math.cos(h) * v * move_t * sgn
        py = origin["y"] + math.sin(h) * v * move_t * sgn
        vx = math.cos(h) * v * sgn
        vy = math.sin(h) * v * sgn
        return px, py, vx, vy, h if sgn > 0 else h + math.pi

    # toward：朝车初始位置走
    dx = -actor["ahead"] * math.cos(h) + actor["lateral"] * math.sin(h)
    dy = -actor["ahead"] * math.sin(h) - actor["lateral"] * math.cos(h)
    n = math.hypot(dx, dy) or 1.0
    dirx, diry = dx / n, dy / n
    px = origin["x"] + dirx * v * move_t
    py = origin["y"] + diry * v * move_t
    return px, py, dirx * v, diry * v, math.atan2(diry, dirx)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--id", type=int, default=OBSTACLE_ID,
                    help="单个演员的障碍物 id；场景模式下以 JSON 里的 id 为准")
    ap.add_argument("--type", default="pedestrian", choices=sorted(TYPE_ENUM),
                    help="障碍物类型，决定 planning 把它当行人/车辆/静止障碍物")
    ap.add_argument("--dir", default="left", choices=["left", "right"],
                    help="cross 模式的横穿方向：left=从右往左（默认），right=从左往右")
    ap.add_argument("--ahead", type=float, default=30.0, help="初始位置：车头前方多少米")
    ap.add_argument("--lateral", type=float, default=0.0, help="横向偏移，正=左")
    ap.add_argument("--mode", default="toward",
                    choices=["toward", "cross", "along", "static", "pingpong"])
    ap.add_argument("--back", action="store_true",
                    help="along 模式反向：沿车头反方向走（与编辑器 dir<0 一致）")
    ap.add_argument("--loop", action="store_true", help="往复横穿，永不停止")
    ap.add_argument("--span", type=float, default=None,
                    help="pingpong 往复幅度 / cross 横穿距离 m；pingpong 默认 4.0，cross 不给则一直走")
    ap.add_argument("--speed", type=float, default=1.5, help="速度 m/s")
    ap.add_argument("--start-delay", type=float, default=0.0, help="延迟多少秒后才开始动")
    ap.add_argument("--trigger-dist", type=float, default=0.0,
                    help="距离触发：自车离演员起点小于该距离(m)才开始动；0=不启用")
    ap.add_argument("--duration", type=float, default=None,
                    help="单个演员的运动时长 s；不给时 pingpong/--loop 无限跑，其余 30 s")
    ap.add_argument("--rate", type=float, default=10.0, help="发布频率 Hz")
    ap.add_argument("--channel", default="/apollo/perception/obstacles")
    ap.add_argument("--width", type=float, default=0.7)
    ap.add_argument("--length", type=float, default=0.5)
    ap.add_argument("--height", type=float, default=1.75)
    ap.add_argument("--ego-x", type=float, default=None, help="不给则从定位读取")
    ap.add_argument("--ego-y", type=float, default=None)
    ap.add_argument("--ego-heading", type=float, default=None)
    ap.add_argument("--scene-file", default=None,
                    help="编辑器导出的 scene-editor/1 JSON 文件：一次起全部演员")
    ap.add_argument("--scene-b64", default=None,
                    help="同上，但 JSON 用 base64 传（run_scene.sh 用这个，避免引号地狱）")
    ap.add_argument("--node-name", default=None,
                    help="cyber 节点名；不给则按第一个 id 生成，必须全局唯一")
    args = ap.parse_args()

    cyber.init()

    # 1. 取锚点：演员轨迹固定在地图上，不跟着车跑
    if args.ego_x is not None and args.ego_y is not None:
        ego = {"x": args.ego_x, "y": args.ego_y, "heading": args.ego_heading or 0.0}
    else:
        loc = read_first("/apollo/localization/pose", localization_pb2.LocalizationEstimate)
        if loc is None:
            print("no localization pose; start localization or pass --ego-x/--ego-y")
            return 2
        ego = {"x": loc.pose.position.x, "y": loc.pose.position.y,
               "heading": loc.pose.heading}

    scene = None
    if args.scene_b64 or args.scene_file:
        if args.scene_b64:
            raw = base64.b64decode(args.scene_b64.encode("ascii")).decode("utf-8")
        else:
            with io.open(args.scene_file, encoding="utf-8") as f:
                raw = f.read()
        scene = json.loads(raw)
        ego_s0 = float((scene.get("ego") or {}).get("s0", 0.0))
        try:
            actors = actors_from_scene(scene, ego_s0, args)
        except (ValueError, KeyError) as e:
            print("场景解析失败: %s" % e)
            return 2
    else:
        dur = args.duration
        if args.loop or (args.mode == "pingpong" and dur is None):
            dur = 0.0
        elif dur is None:
            dur = 30.0
        actors = [make_actor(
            id=args.id, type=args.type, mode=args.mode, dir=args.dir,
            ahead=args.ahead, lateral=args.lateral, speed=args.speed,
            span=(args.span if args.span is not None else 4.0),
            cross_cap=args.span, back=args.back, start_delay=args.start_delay,
            trigger_dist=args.trigger_dist, duration=dur,
            w=args.width, l=args.length, h=args.height,
        )]

    h = ego["heading"]
    origins = {a["id"]: actor_origin(a, ego) for a in actors}

    # 节点名必须唯一：cyber 里同名节点会让后来的直接 terminate。
    # 默认按第一个演员 id 起名，单演员行为与老版本一致。
    node_name = args.node_name or ("virtual_obstacle_%d" % actors[0]["id"])
    node = cyber.Node(node_name)
    writer = node.create_writer(args.channel, perception_obstacle_pb2.PerceptionObstacles)

    # 距离触发要持续读实时位姿（车会动），不能只用启动时的锚点
    live = {"x": ego["x"], "y": ego["y"]}

    def on_pose(m):
        live["x"] = m.pose.position.x
        live["y"] = m.pose.position.y

    node.create_reader("/apollo/localization/pose", localization_pb2.LocalizationEstimate, on_pose)

    print("anchor ego=(%.2f, %.2f) h=%.1fdeg channel=%s actors=%d node=%s"
          % (ego["x"], ego["y"], math.degrees(h), args.channel, len(actors), node_name))
    for a in actors:
        o = origins[a["id"]]
        print("  id=%d type=%s start=(%.2f, %.2f) mode=%s dir=%s v=%.2f m/s "
              "delay=%.1fs trigger=%s dur=%s %s"
              % (a["id"], a["type"], o["x"], o["y"], a["mode"], a["dir"],
                 a["speed"], a["start_delay"], a["trigger_dist"], a["duration"],
                 a["name"]))
    sys.stdout.flush()

    t0 = time.time()
    period = 1.0 / args.rate
    seq = 0
    last_log = 0.0
    last_wait_log = 0.0
    # 距离触发的演员先空着，等车开进 trigger_dist 再定
    trig = {a["id"]: (t0 if a["trigger_dist"] <= 0.0 else None) for a in actors}

    def finished(a, now):
        if a["duration"] is None or a["duration"] <= 0:
            return False
        tt = trig[a["id"]]
        if tt is None:
            return False
        return (now - tt) >= a["start_delay"] + a["duration"]

    while True:
        now = time.time()
        if actors and all(finished(a, now) for a in actors):
            break
        time.sleep(period)
        now = time.time()

        msg = perception_obstacle_pb2.PerceptionObstacles()
        msg.header.timestamp_sec = now
        msg.header.module_name = node_name
        msg.header.sequence_num = seq
        seq += 1

        active = 0
        for a in actors:
            if trig[a["id"]] is None:
                o = origins[a["id"]]
                d = math.hypot(live["x"] - o["x"], live["y"] - o["y"])
                if d > a["trigger_dist"]:
                    if now - last_wait_log >= 1.0:
                        last_wait_log = now
                        print("waiting: id=%d ego->start = %.1f m (need <= %.1f m)"
                              % (a["id"], d, a["trigger_dist"]))
                    continue  # 车还没进来，演员"还没出现"
                trig[a["id"]] = now
                print("TRIGGERED: id=%d ego->start = %.1f m <= %.1f m"
                      % (a["id"], d, a["trigger_dist"]))

            elapsed = now - trig[a["id"]]
            move_t = max(0.0, elapsed - a["start_delay"])
            if move_t <= 0.0:
                continue  # 到点之前不发布 —— 相当于"还没出现"
            if a["duration"] and a["duration"] > 0 and move_t > a["duration"]:
                continue

            px, py, vx, vy, theta = actor_pose(a, origins[a["id"]], move_t, h)
            ob = msg.perception_obstacle.add()
            ob.id = a["id"]
            ob.position.x = px
            ob.position.y = py
            ob.position.z = 0.0
            ob.theta = theta
            ob.velocity.x = vx
            ob.velocity.y = vy
            ob.velocity.z = 0.0
            ob.length = a["l"]
            ob.width = a["w"]
            ob.height = a["h"]
            ob.type = TYPE_ENUM[a["type"]]
            ob.confidence = 1.0
            ob.tracking_time = move_t
            ob.timestamp = now
            active += 1

            if now - last_log >= 0.5:
                print("t=%5.1fs id=%d pos=(%.1f, %.1f) move_t=%.1fs"
                      % (elapsed, a["id"], px, py, move_t))
        if now - last_log >= 0.5:
            last_log = now

        if active:
            writer.write(msg)

    print("done, %d messages on %s" % (seq, args.channel))
    sys.stdout.flush()
    os._exit(0)


if __name__ == "__main__":
    sys.exit(main())
