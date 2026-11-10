#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""planning 就绪门（启动时序闸门）

用法：
  # 1) 一次性检查：READY 才允许挂 auto（返回 0=就绪，1=未就绪）
  python3 planning_gate.py --check --timeout 0.5 --window 3

  # 2) 常驻监控：挂在 auto 时 planning 掉线就报警（可选发 STOP）
  python3 planning_gate.py --watch --stop-on-fail
"""

import argparse
import os
import sys
import time

from cyber.python.cyber_py3 import cyber
from modules.common_msgs.planning_msgs import planning_pb2
from modules.common_msgs.planning_msgs import pad_msg_pb2
from modules.common_msgs.chassis_msgs import chassis_pb2


class Gate(object):
    def __init__(self):
        self.last_planning_ts = 0.0
        self.last_planning_wall = 0.0
        self.traj_points = 0
        self.driving_mode = None
        self.planning_count = 0

    def on_planning(self, msg):
        self.last_planning_ts = msg.header.timestamp_sec
        self.last_planning_wall = time.time()
        self.traj_points = len(msg.trajectory_point)
        self.planning_count += 1

    def on_chassis(self, msg):
        self.driving_mode = msg.driving_mode


def mode_name(m):
    return {
        chassis_pb2.Chassis.COMPLETE_MANUAL: "MANUAL",
        chassis_pb2.Chassis.COMPLETE_AUTO_DRIVE: "AUTO",
        chassis_pb2.Chassis.AUTO_STEER_ONLY: "AUTO_STEER_ONLY",
        chassis_pb2.Chassis.AUTO_SPEED_ONLY: "AUTO_SPEED_ONLY",
        chassis_pb2.Chassis.EMERGENCY_MODE: "EMERGENCY",
    }.get(m, str(m))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="一次性检查后退出")
    ap.add_argument("--watch", action="store_true", help="常驻监控")
    ap.add_argument("--timeout", type=float, default=0.5, help="planning 多久没更新算掉线(s)")
    ap.add_argument("--window", type=float, default=3.0, help="检查窗口(s)")
    ap.add_argument("--stop-on-fail", action="store_true",
                    help="掉线且处于 AUTO 时发 STOP（默认只报警）")
    args = ap.parse_args()

    cyber.init()
    g = Gate()
    node = cyber.Node("planning_gate_%d" % os.getpid())
    node.create_reader("/apollo/planning", planning_pb2.ADCTrajectory, g.on_planning)
    node.create_reader("/apollo/canbus/chassis", chassis_pb2.Chassis, g.on_chassis)
    pad_writer = node.create_writer("/apollo/planning/pad", pad_msg_pb2.PadMessage)

    if args.check:
        t0 = time.time()
        time.sleep(args.window)
        fresh = (time.time() - g.last_planning_wall) <= args.timeout
        hz = g.planning_count / max(1e-6, (time.time() - t0))
        ok = fresh and g.planning_count > 0 and g.traj_points > 0
        print("planning: fresh=%s msgs=%d rate=%.1fHz traj_points=%d mode=%s"
              % (fresh, g.planning_count, hz, g.traj_points, mode_name(g.driving_mode)))
        if ok:
            print("READY  - safe to engage auto")
            sys.stdout.flush(); os._exit(0)
        print("NOT READY - do NOT engage auto")
        sys.stdout.flush(); os._exit(1)

    # watch 模式
    print("gate: watching planning (timeout=%.2fs, stop_on_fail=%s)"
          % (args.timeout, args.stop_on_fail))
    sys.stdout.flush()
    was_stale = False
    while True:
        time.sleep(0.2)
        now = time.time()
        stale = (now - g.last_planning_wall) > args.timeout
        in_auto = g.driving_mode == chassis_pb2.Chassis.COMPLETE_AUTO_DRIVE
        if stale and not was_stale:
            print("[%s] planning STALE (last %.2fs ago) mode=%s traj=%d"
                  % (time.strftime("%H:%M:%S"), now - g.last_planning_wall,
                     mode_name(g.driving_mode), g.traj_points))
            sys.stdout.flush()
            if in_auto:
                print("  !! in AUTO but planning stale")
                if args.stop_on_fail:
                    pad = pad_msg_pb2.PadMessage()
                    pad.header.timestamp_sec = now
                    pad.header.module_name = "planning_gate"
                    pad.action = pad_msg_pb2.PadMessage.STOP
                    pad_writer.write(pad)
                    print("  !! sent PadMessage(STOP)")
                sys.stdout.flush()
            was_stale = True
        elif not stale and was_stale:
            print("[%s] planning recovered, traj=%d mode=%s"
                  % (time.strftime("%H:%M:%S"), g.traj_points, mode_name(g.driving_mode)))
            sys.stdout.flush()
            was_stale = False
    return 0


if __name__ == "__main__":
    sys.exit(main())
