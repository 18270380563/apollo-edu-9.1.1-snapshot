# -*- coding: utf-8 -*-
"""横向控制振荡专项分析（第一轮 cruise=10 / 第二轮 cruise=5）。

复现 2026-10-29_横向控制剧烈摆动根因分析.md 里的所有数字。

用法：
    python 中间脚本/analyze_lateral_oscillation.py

数据来源：
    分析数据/edu_debug_drive_1029/      第一轮（default_cruise_speed = 10）
    分析数据/edu_debug_drive_1029_r2/   第二轮（default_cruise_speed = 5）

单位说明（很重要）：
    cmd_steer / lat.*_contribution / steer_limit 全部是「百分比」，
    不是角度。100% = 方向盘 479.75° = 前轮 29.65°。
    max_steer_angle = 8.37333 rad，steer_ratio = 16.18
    1% = 4.797° 方向盘 = 0.2965° 前轮
"""
import sys
import json
import math
import os

sys.stdout.reconfigure(encoding='utf-8', errors='replace')

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEG_PER_PCT = 8.37333 / math.pi * 180 / 100.0      # 4.797 °方向盘 / 1%
WHEELBASE = 2.75                                    # m，由 steer_limit 反解得到
MAX_LAT_ACC = 4.0                                   # m/s^2, conf: max_lateral_acceleration


def load(base, name):
    path = os.path.join(ROOT, base, name)
    rows = []
    if not os.path.exists(path):
        return rows
    with open(path, encoding='utf-8') as fh:
        for line in fh:
            line = line.strip()
            if line:
                try:
                    rows.append(json.loads(line))
                except ValueError:
                    pass
    rows.sort(key=lambda r: float(r.get('seq', 0)))
    return rows


def g(row, key):
    try:
        val = row.get(key)
        if val in (None, '', 'nan', 'unset'):
            return 0.0
        return float(val)
    except (TypeError, ValueError):
        return 0.0


class Round(object):
    def __init__(self, tag, base):
        self.tag = tag
        self.ctrl = [r for r in load(base, 'control_state.jsonl')
                     if r.get('chassis_mode') == '1']
        self.t0 = g(self.ctrl[0], 'seq') if self.ctrl else 0.0
        self.t = [g(r, 'seq') - self.t0 for r in self.ctrl]

    def col(self, key):
        return [g(r, key) for r in self.ctrl]

    def window(self, lo, hi):
        return [r for r, tt in zip(self.ctrl, self.t) if lo <= tt / 10.0 <= hi]


# ---------------------------------------------------------------- 1. 幅值包络
def envelope(rd, width=24, step=6):
    """滑窗 RMS*sqrt2 当作正弦幅值（单位 %）。"""
    out = []
    n = len(rd.ctrl)
    cmd = rd.col('cmd_steer')
    if n <= width:
        return out
    starts = list(range(0, n - width, step))
    if starts[-1] != n - width:      # 补一个贴着末帧的窗口，保证覆盖到结尾
        starts.append(n - width)
    for i in starts:
        seg = cmd[i:i + width]
        mean = sum(seg) / len(seg)
        rms = math.sqrt(sum((x - mean) ** 2 for x in seg) / len(seg))
        j = i + width // 2
        out.append((rd.t[j] / 10.0, g(rd.ctrl[j], 'chassis_v'), rms * math.sqrt(2)))
    return out


# ---------------------------------------------------------------- 2. 等效增益
def lstsq4(rows):
    """fb = a*latE + b*latEr + c*headE + d*headEr，返回 (a,b,c,d,rms)。"""
    keys = ('lat.lateral_error', 'lat.lateral_error_rate',
            'lat.heading_error', 'lat.heading_error_rate')
    xs = [[g(r, k) for k in keys] for r in rows]
    ys = [g(r, 'lat.steer_angle_feedback') for r in rows]
    n, p = len(xs), 4
    if n <= p:
        return None
    mat = [[sum(xs[k][i] * xs[k][j] for k in range(n)) for j in range(p)]
           for i in range(p)]
    vec = [sum(xs[k][i] * ys[k] for k in range(n)) for i in range(p)]
    for i in range(p):
        piv = max(range(i, p), key=lambda r: abs(mat[r][i]))
        mat[i], mat[piv] = mat[piv], mat[i]
        vec[i], vec[piv] = vec[piv], vec[i]
        if abs(mat[i][i]) < 1e-12:
            return None
        for r in range(p):
            if r == i:
                continue
            m = mat[r][i] / mat[i][i]
            for c in range(p):
                mat[r][c] -= m * mat[i][c]
            vec[r] -= m * vec[i]
    k = [vec[i] / mat[i][i] for i in range(p)]
    res = [sum(xs[j][i] * k[i] for i in range(p)) - ys[j] for j in range(n)]
    rms = math.sqrt(sum(e * e for e in res) / n)
    return k + [rms]


# ---------------------------------------------------------------- 3. 执行器滞后
def corr(x, y, lag):
    if lag >= 0:
        a, b = x[:len(x) - lag], y[lag:]
    else:
        a, b = x[-lag:], y[:len(y) + lag]
    n = min(len(a), len(b))
    if n < 4:
        return 0.0
    a, b = a[:n], b[:n]
    ma, mb = sum(a) / n, sum(b) / n
    sa = math.sqrt(sum((u - ma) ** 2 for u in a))
    sb = math.sqrt(sum((v - mb) ** 2 for v in b))
    if sa * sb == 0:
        return 0.0
    return sum((a[i] - ma) * (b[i] - mb) for i in range(n)) / (sa * sb)


def actuator_lag(rd, lo, hi):
    seg = rd.window(lo, hi)
    if len(seg) < 8:
        return None
    cmd = [g(r, 'cmd_steer') for r in seg]
    pct = [g(r, 'chassis_steer_pct') for r in seg]
    cs = {lag: corr(cmd, pct, lag) for lag in range(0, 6)}
    best = max(cs, key=lambda k: cs[k])
    return best, cs[best], max(cmd) - min(cmd), max(pct) - min(pct), cs


def main():
    rounds = [Round('第一轮 cruise=10', '分析数据/edu_debug_drive_1029'),
              Round('第二轮 cruise=5', '分析数据/edu_debug_drive_1029_r2')]

    for rd in rounds:
        if not rd.ctrl:
            print('%s: 没有 AUTO 帧，跳过' % rd.tag)
            continue
        print('=' * 78)
        print('%s   AUTO %d 帧 = %.1f s' % (rd.tag, len(rd.ctrl), len(rd.ctrl) / 10.0))
        print('=' * 78)

        cmd = rd.col('cmd_steer')
        pct = rd.col('chassis_steer_pct')
        fb = rd.col('lat.steer_angle_feedback')
        lim = rd.col('lat.steer_angle_limited')
        le = rd.col('lat.lateral_error')
        he = rd.col('lat.heading_error')
        vs = rd.col('chassis_v')
        print('  车速        : %.2f → %.2f (max %.2f m/s)' % (vs[0], vs[-1], max(vs)))
        print('  cmd_steer   : %.2f .. %.2f  幅 %.1f%%  = %.0f° 方向盘   (前轮 %.1f°)'
              % (min(cmd), max(cmd), max(cmd) - min(cmd),
                 (max(cmd) - min(cmd)) * DEG_PER_PCT,
                 (max(cmd) - min(cmd)) * DEG_PER_PCT / 16.18))
        print('  steer_pct实车: %.2f .. %.2f  幅 %.1f%%  (只实现指令幅度的 %.0f%%)'
              % (min(pct), max(pct), max(pct) - min(pct),
                 100.0 * (max(pct) - min(pct)) / max(1e-9, max(cmd) - min(cmd))))
        print('  feedback    : %.2f .. %.2f  (最终输出被限到 %.2f .. %.2f，反馈饱和)'
              % (min(fb), max(fb), min(lim), max(lim)))
        print('  max|lat_err|: %.3f m    max|head_err|: %.3f rad' % (
            max(abs(x) for x in le), max(abs(x) for x in he)))
        dcmd = [abs(cmd[i] - cmd[i - 1]) for i in range(1, len(cmd))]
        dpct = [abs(pct[i] - pct[i - 1]) for i in range(1, len(pct))]
        print('  指令最大跳变: %.1f%%/100ms = %.0f°/s 方向盘' % (
            max(dcmd), max(dcmd) * DEG_PER_PCT * 10))
        print('  实车最大跳变: %.1f%%/100ms = %.0f°/s 方向盘（执行器物理上限）' % (
            max(dpct), max(dpct) * DEG_PER_PCT * 10))

        print('\n  --- 转向指令包络（2.4 s 滑窗）vs 车速 ---')
        print('    t(s)     v     幅值(%%)')
        for tt, vv, amp in envelope(rd):
            print('   %6.1f  %5.2f  %8.2f' % (tt, vv, amp))

    print('\n' + '=' * 78)
    print('等效反馈增益  fb = a*latE + b*latEr + c*headE + d*headEr')
    print('=' * 78)
    print('  窗口             n    a(%%/m)  b(%%/(m/s))  c(%%/rad)  d(%%/(rad/s))  rms')
    windows = [(rounds[0], 3.5, 5.5, 'R1 4-5.5 m/s'),
               (rounds[0], 6.2, 8.4, 'R1 6.2-6.8 m/s'),
               (rounds[1], 15.0, 22.0, 'R2 稳定 15-22s'),
               (rounds[1], 22.0, 28.0, 'R2 稳定 22-28s'),
               (rounds[1], 15.0, 31.0, 'R2 稳定 15-31s'),
               (rounds[1], 33.0, 37.0, 'R2 发散 33-37s'),
               (rounds[1], 38.0, 42.0, 'R2 发散 38-42s')]
    for rd, lo, hi, name in windows:
        seg = rd.window(lo, hi)
        k = lstsq4(seg)
        if not k:
            continue
        print('  %-14s %4d %9.1f %10.1f %10.1f %12.1f %7.2f'
              % (name, len(seg), k[0], k[1], k[2], k[3], k[4]))

    print('\n' + '=' * 78)
    print('执行器/指令通道滞后（cmd_steer → chassis_steer_pct）')
    print('=' * 78)
    for rd, lo, hi, name in ((rounds[0], 6.0, 8.4, 'R1 6.0-8.3s'),
                             (rounds[1], 15.0, 31.0, 'R2 15-31s 稳定'),
                             (rounds[1], 33.0, 41.0, 'R2 33-41s 发散')):
        res = actuator_lag(rd, lo, hi)
        if not res:
            continue
        lag, r, campl, pampl, cs = res
        print('  %-14s 滞后 %d 帧 (%d ms, r=%.2f)   指令幅 %.1f%% / 实现幅 %.1f%%'
              % (name, lag, lag * 100, r, campl, pampl))
        print('    lag→r: ' + ', '.join('%d:%.2f' % (k, v) for k, v in cs.items()))

    print('\n' + '=' * 78)
    print('横向加速度限幅 steer_limit = atan(4.0 * 2.75 / v^2) * 16.18 * 180/pi / 479.75 * 100')
    print('=' * 78)
    for v in (2.0, 3.0, 4.0, 5.0, 5.2, 6.0, 6.4, 6.7, 8.0, 10.0):
        lim = math.atan(MAX_LAT_ACC * WHEELBASE / (v * v)) * 16.18 * 180 / math.pi / 479.75 * 100
        print('  v=%5.1f m/s -> 限幅 %6.1f%% (%.0f° 方向盘)' % (v, lim, lim * DEG_PER_PCT))


if __name__ == '__main__':
    main()
