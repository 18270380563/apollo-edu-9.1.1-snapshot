#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""分析方向盘左右摆：控制指令 vs 实际转角 vs 横向误差。

用法: python3 analyze_steer.py <record_file> [--csv out.csv]
"""

import math
import sys

from cyber.python.cyber_py3 import record
from modules.common_msgs.control_msgs import control_cmd_pb2

try:
    from modules.common_msgs.control_msgs import control_debug_pb2 as _dbgmod
except Exception:
    _dbgmod = None
from modules.common_msgs.chassis_msgs import chassis_pb2

CH_CONTROL = "/apollo/control"
CH_CHASSIS = "/apollo/canbus/chassis"
CH_DEBUG = "/apollo/control/debug"


def read_all(path):
    ctrl, chas, dbg = [], [], []
    reader = record.RecordReader(path)
    for item in reader.read_messages():
        ch, raw = item[0], item[1]
        if ch == CH_CONTROL:
            m = control_cmd_pb2.ControlCommand()
            m.ParseFromString(raw)
            ctrl.append((m.header.timestamp_sec, m))
        elif ch == CH_CHASSIS:
            m = chassis_pb2.Chassis()
            m.ParseFromString(raw)
            chas.append((m.header.timestamp_sec, m))
        elif ch == CH_DEBUG and _dbgmod is not None:
            m = _dbgmod.ControlDebugInfo()
            m.ParseFromString(raw)
            dbg.append((m.header.timestamp_sec, m))
    return ctrl, chas, dbg


def sign_changes(vals, eps):
    return sum(1 for i in range(1, len(vals))
               if vals[i - 1] * vals[i] < 0 and abs(vals[i]) > eps)


def pct(vals, q):
    s = sorted(vals)
    return s[min(len(s) - 1, int(len(s) * q))]


def main():
    path = sys.argv[1]
    ctrl, chas, dbg = read_all(path)
    if not ctrl or not chas:
        print("missing control/chassis: control=%d chassis=%d" % (len(ctrl), len(chas)))
        return 1

    t0 = min(ctrl[0][0], chas[0][0])
    ct = [(t - t0, m) for t, m in ctrl]
    cs = [(t - t0, m) for t, m in chas]
    db = [(t - t0, m) for t, m in dbg]
    dur = max(ct[-1][0], cs[-1][0])

    print("duration %.1fs, control %d msgs (%.0f Hz), chassis %d msgs (%.0f Hz), debug %d"
          % (dur, len(ct), len(ct) / dur, len(cs), len(cs) / dur, len(db)))

    target = [m.steering_target for _, m in ct]
    actual = [m.steering_percentage for _, m in cs]
    speed = [m.speed_mps for _, m in cs]
    mode = [m.driving_mode for _, m in cs]

    print("\n--- steering target (%) ---")
    print("  min %.2f  max %.2f  mean %.2f  std %.2f  p5 %.2f  p95 %.2f"
          % (min(target), max(target), sum(target) / len(target),
             math.sqrt(sum((v - sum(target) / len(target)) ** 2 for v in target) / len(target)),
             pct(target, 0.05), pct(target, 0.95)))
    print("--- steering actual (%) ---")
    print("  min %.2f  max %.2f  mean %.2f  std %.2f  p5 %.2f  p95 %.2f"
          % (min(actual), max(actual), sum(actual) / len(actual),
             math.sqrt(sum((v - sum(actual) / len(actual)) ** 2 for v in actual) / len(actual)),
             pct(actual, 0.05), pct(actual, 0.95)))

    nt = sign_changes(target, 0.5)
    na = sign_changes(actual, 0.5)
    print("\n--- 摆动次数（符号翻转，阈值 0.5%%）---")
    print("  steering_target 翻转 %d 次 -> 平均 %.2f s/次" % (nt, dur / max(nt, 1)))
    print("  steering_actual 翻转 %d 次 -> 平均 %.2f s/次" % (na, dur / max(na, 1)))

    # 实际转角滞后于指令的时间（互相关）
    def norm_corr(a, b):
        n = min(len(a), len(b))
        a = a[:n]; b = b[:n]
        ma = sum(a) / n; mb = sum(b) / n
        va = math.sqrt(sum((x - ma) ** 2 for x in a) / n)
        vb = math.sqrt(sum((x - mb) ** 2 for x in b) / n)
        if va < 1e-9 or vb < 1e-9:
            return 0.0
        return sum((a[i] - ma) * (b[i] - mb) for i in range(n)) / (n * va * vb)

    def lag_corr(a, b, max_lag=60):
        best, bl = -2, 0
        n = min(len(a), len(b))
        for lag in range(-max_lag, max_lag + 1):
            if lag >= 0:
                aa, bb = a[:n - lag], b[lag:]
            else:
                aa, bb = a[-lag:], b[:n + lag]
            if len(aa) < 100:
                continue
            c = norm_corr(aa, bb)
            if c > best:
                best, bl = c, lag
        return bl, best

    # 重采样到等间隔便于互相关
    def resample(series, step=0.01):
        out = []
        j = 0
        t = 0.0
        end = series[-1][0]
        while t <= end:
            while j + 1 < len(series) and series[j + 1][0] < t:
                j += 1
            out.append(series[j][1])
            t += step
        return out

    def resample_time(series, step=0.01):
        """按时间线性插值到等间隔网格"""
        ts_ = [t for t, _ in series]
        vs_ = [v for _, v in series]
        out = []
        t = ts_[0]
        j = 0
        while t <= ts_[-1]:
            while j + 1 < len(ts_) and ts_[j + 1] < t:
                j += 1
            if j + 1 < len(ts_) and ts_[j + 1] > ts_[j]:
                w = (t - ts_[j]) / (ts_[j + 1] - ts_[j])
                out.append(vs_[j] * (1 - w) + vs_[j + 1] * w)
            else:
                out.append(vs_[j])
            t += step
        return out

    rt = resample_time([(t, m.steering_target) for t, m in ct])
    ra = resample_time([(t, m.steering_percentage) for t, m in cs])
    n = min(len(rt), len(ra))
    lag, c = lag_corr(rt[:n], ra[:n])
    print("\n--- 指令/实际 互相关 ---")
    print("  best lag %+d samples (%.2f s), corr=%.3f" % (lag, lag * 0.01, c))

    # 分段统计：起步瞬态 vs 稳态
    def window_stats(name, lo, hi):
        tv = [(t, m.steering_target) for t, m in ct if lo <= t < hi]
        if not tv:
            return
        vals = [v for _, v in tv]
        mean = sum(vals) / len(vals)
        std = math.sqrt(sum((v - mean) ** 2 for v in vals) / len(vals))
        nsc = sign_changes(vals, 0.5)
        print("  %-14s t=[%.0f,%.0f)s  n=%4d  min %6.2f  max %6.2f  std %5.2f  flips %d"
              % (name, lo, hi, len(vals), min(vals), max(vals), std, nsc))

    # 振荡成分：去掉 2 s 慢趋势后的残差
    def hi_rms(vals, tau=2.0, dt=0.01):
        y = vals[0]
        res = []
        a = dt / (tau + dt)
        for x in vals:
            y = y + a * (x - y)
            res.append(x - y)
        m = sum(res) / len(res)
        return math.sqrt(sum((r - m) ** 2 for r in res) / len(res))

    # 局部极值计数（每 10 秒多少次方向反转）
    def extrema(vals, dt=0.01, eps=0.3):
        n = 0
        d_prev = 0
        for i in range(1, len(vals)):
            d = vals[i] - vals[i - 1]
            if abs(d) < eps:
                continue
            if d_prev * d < 0:
                n += 1
            d_prev = d
        return n

    print("\n--- 振荡成分（去 2s 慢趋势）---")
    print("  target 残差 RMS = %.2f %%   极值 %d 次/%.0fs"
          % (hi_rms([m.steering_target for t, m in ct if t > 6]),
             extrema([m.steering_target for t, m in ct if t > 6]),
             max(0, ct[-1][0] - 6)))

    print("\n--- 分段：控制指令 ---")
    window_stats("起步瞬态", 0, 3)
    window_stats("过渡", 3, 6)
    window_stats("稳态", 6, 999)

    print("\n--- 速度/模式 ---")
    print("  speed min %.2f max %.2f mean %.2f m/s" % (min(sh), max(sh), sum(sh) / len(sh)))
    from collections import Counter
    print("  driving_mode: %s" % dict(Counter(mode)))

    # 时间序列表
    print("\n   t(s)  target%  actual%   lat_err(m)  head_err(deg)  v(m/s)")
    last = -9
    di = 0
    for t, m in ct:
        if t - last < 0.2:
            continue
        last = t
        # 最近 chassis
        while di + 1 < len(cs) and cs[di + 1][0] <= t:
            di += 1
        a = cs[di][1].steering_percentage
        v = cs[di][1].speed_mps
        le = he = float("nan")
        if db:
            j = min(range(len(db)), key=lambda i: abs(db[i][0] - t))
            if abs(db[j][0] - t) < 0.05:
                d = db[j][1]
                if d.has_simple_lateral_debug():
                    le = d.simple_lateral_debug.lateral_error
                    he = math.degrees(d.simple_lateral_debug.heading_error)
        print("%7.2f  %7.2f  %7.2f  %10.3f  %12.2f  %6.2f"
              % (t, m.steering_target, a, le, he, v))
    return 0


if __name__ == "__main__":
    sys.exit(main())
