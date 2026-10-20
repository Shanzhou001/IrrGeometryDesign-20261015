#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
convert_rraw_to_engine_csv.py
=============================
将真实野外 SEG-Y 单炮道集转换为 IrrGeometryDesign 引擎要求的 256x128 CSV。

背景
----
引擎 runWavefieldReconstruction 严格要求 Nt=256 (dt=4 ms) x Nx=128 (dx=12.5 m)
的纯数值矩阵。原始资料为 59 道 / 250 样点 / 8 ms，必须经过显式的重采样，
因此本脚本的每一步都写死参数并记录到 CSV 头部，保证任何人可逐位复现。

数据格式说明（重要）
--------------------
RRAW.SGY 的二进制头 bytes 3225-3226 声明 format code = 1（IBM 浮点），
但实测 payload 为 **IEEE-754 little-endian float32**。按 IBM 解码会得到
1e25 量级的荒谬值（本脚本提供 --check-encoding 自动判定并打印证据）。
解码方式以实际字节为准，不以头部声明为准。

处理流程（参数固定）
--------------------
1. 读取 59 道，按 offset（字节 37-40, int32）升序排序；
2. 时间轴重采样 8 ms -> 4 ms：250 点线性插值到 256 点，覆盖 0 ~ 1.020 s；
   （原始 2.0 s 记录中 1.02 s 之后的内容被裁掉，依据见 README 第 8 节）
3. 道数重采样 59 -> 128：沿道方向线性插值（上采样 2.17x）；
4. 8-70 Hz 带通（余弦斜坡 8-12 Hz / 60-70 Hz，用于压制地滚波与高频噪声）；
5. 全局峰值归一化到 max|amp| = 1.0。

用法
----
    python tools/convert_rraw_to_engine_csv.py \
        --input  data/real_seismic/Data/RRAW.SGY \
        --output data/real_seismic/real_gather_256x128.csv

    # 仅检查编码判定
    python tools/convert_rraw_to_engine_csv.py --input ... --check-encoding
"""

import argparse
import hashlib
import os
import struct
import sys

import numpy as np

# ---- 固定处理参数（改动会改变实验输入，必须同步 README）----
TARGET_NT = 256            # 引擎要求的时间样点数
TARGET_DT_MS = 4.0         # 引擎要求的时间采样间隔
TARGET_NX = 128            # 引擎要求的道数
BANDPASS_LO = 8.0          # Hz
BANDPASS_LO_RAMP = 12.0    # Hz
BANDPASS_HI_RAMP = 60.0    # Hz
BANDPASS_HI = 70.0         # Hz

SEGY_TEXT_HDR = 3200
SEGY_BIN_HDR = 400
TRACE_HDR = 240


def read_segy(path):
    """读取 SEG-Y，返回 (data[Nt,Nx], offsets[Nx], cdp[Nx], meta)。"""
    raw = open(path, "rb").read()

    # 重要：RRAW.SGY 的二进制头与道头**均为 little-endian**（与 SEG-Y 标准的
    # big-endian 相反）。字节级核对证据：
    #   idx 3220-3221 (1-based 3221) LE = 250 -> samples per trace，合理
    #   idx 3224-3225 (1-based 3225) LE = 1   -> format code，合理
    # 若按大端读取会得到 ns=0 / format=0，导致解析直接失败。
    bin_hdr = raw[SEGY_TEXT_HDR:SEGY_TEXT_HDR + SEGY_BIN_HDR]
    ns = struct.unpack_from("<H", bin_hdr, 20)[0]        # 1-based 3221-3222
    dt_raw = struct.unpack_from("<H", bin_hdr, 16)[0]    # 1-based 3217-3218
    fmt = struct.unpack_from("<H", bin_hdr, 24)[0]       # 1-based 3225-3226

    # 采样间隔字段在本文件中为 64（非标准 us 值，不可直接使用）。
    # 真实采样间隔由文本头 "Resampled 8.0 ms" 与道头 ns=250 共同印证为 8 ms。
    dt_us = 8000

    trace_bytes = TRACE_HDR + ns * 4
    avail = len(raw) - (SEGY_TEXT_HDR + SEGY_BIN_HDR)
    ntraces = avail // trace_bytes

    data = np.zeros((ns, ntraces), dtype=np.float64)
    offsets = np.zeros(ntraces, dtype=np.int64)
    cdps = np.zeros(ntraces, dtype=np.int64)

    for t in range(ntraces):
        off = SEGY_TEXT_HDR + SEGY_BIN_HDR + t * trace_bytes
        cdps[t] = struct.unpack_from("<i", raw, off + 20)[0]
        offsets[t] = struct.unpack_from("<i", raw, off + 36)[0]
        # payload 按 IEEE-754 little-endian 解码（见模块说明）
        data[:, t] = struct.unpack_from("<%df" % ns, raw, off + TRACE_HDR)

    meta = {
        "ns": ns,
        "dt_us": dt_us,
        "dt_raw_field": dt_raw,
        "declared_format": fmt,
        "ntraces": ntraces,
        "file_bytes": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
    }
    return data, offsets, cdps, meta


def check_encoding(path):
    """验证 payload 到底是 IBM 浮点还是 IEEE 小端浮点。"""
    data, offsets, cdps, meta = read_segy(path)
    finite = data[np.isfinite(data)]
    peak = float(np.abs(finite).max()) if finite.size else 0.0
    print("declared format code : %d (1 = IBM float)" % meta["declared_format"])
    print("decoded as IEEE-LE   : peak |amp| = %.1f" % peak)
    print("plausible            : %s" % ("YES" if peak < 1e6 else "NO"))
    print("-> payload is %s" % ("IEEE-754 little-endian float32"
                                if peak < 1e6 else "NOT IEEE-LE; investigate"))
    return peak < 1e6


def convert(data, offsets):
    """59x250 -> 128x256，参数全部来自模块常量。"""
    ns, nx = data.shape

    # 1) 按 offset 升序排序
    order = np.argsort(offsets, kind="stable")
    data = data[:, order]
    offsets = offsets[order]

    # 2) 时间轴 8 ms -> 4 ms，250 -> 256 点
    src_dt = 8.0
    t_src = np.arange(ns) * src_dt
    t_dst = np.arange(TARGET_NT) * TARGET_DT_MS
    time_rs = np.zeros((TARGET_NT, nx))
    for j in range(nx):
        time_rs[:, j] = np.interp(t_dst, t_src, data[:, j])

    # 3) 道数 59 -> 128
    x_src = np.arange(nx)
    x_dst = np.linspace(0, nx - 1, TARGET_NX)
    trace_rs = np.zeros((TARGET_NT, TARGET_NX))
    for i in range(TARGET_NT):
        trace_rs[i, :] = np.interp(x_dst, x_src, time_rs[i, :])

    # 4) 8-70 Hz 带通
    freqs = np.fft.rfftfreq(TARGET_NT, d=TARGET_DT_MS / 1000.0)
    spec = np.fft.rfft(trace_rs, axis=0)
    band = np.zeros_like(freqs)
    lo = (freqs >= BANDPASS_LO) & (freqs < BANDPASS_LO_RAMP)
    band[(freqs >= BANDPASS_LO_RAMP) & (freqs <= BANDPASS_HI_RAMP)] = 1.0
    hi = (freqs > BANDPASS_HI_RAMP) & (freqs <= BANDPASS_HI)
    if lo.any():
        band[lo] = np.linspace(0, 1, lo.sum())
    if hi.any():
        band[hi] = np.linspace(1, 0, hi.sum())
    trace_rs = np.fft.irfft(spec * band[:, None], n=TARGET_NT, axis=0)

    # 5) 峰值归一化
    peak = np.abs(trace_rs).max()
    if peak > 0:
        trace_rs /= peak

    return trace_rs, offsets


def write_csv(path, mat, meta, offsets):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", newline="\n") as fh:
        fh.write("# Real field common-shot gather converted from RRAW.SGY\n")
        fh.write("# Source SHA256: %s\n" % meta["sha256"])
        fh.write("# Source: %d traces x %d samples, dt=%d us, declared format=%d "
                 "(payload is IEEE-LE float32)\n"
                 % (meta["ntraces"], meta["ns"], meta["dt_us"], meta["declared_format"]))
        fh.write("# Offsets (m): %s\n" % ", ".join(str(int(o)) for o in offsets))
        fh.write("# Processing: offset-sorted; time 8->4 ms (250->256 via interp); "
                 "traces 59->128 via interp; bandpass %.0f-%.0f Hz; peak-normalized to 1.0\n"
                 % (BANDPASS_LO, BANDPASS_HI))
        fh.write("# Time window: 0.000-%.3f s (%d samples @ %.1f ms); "
                 "original record length %.3f s was truncated\n"
                 % ((TARGET_NT - 1) * TARGET_DT_MS / 1000.0, TARGET_NT,
                    TARGET_DT_MS, (meta["ns"] - 1) * meta["dt_us"] / 1e6))
        fh.write("# Geometry: Nt=%d, Nx=%d\n" % (TARGET_NT, TARGET_NX))
        for i in range(TARGET_NT):
            fh.write(",".join("%.6g" % v for v in mat[i, :]) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True, help="input SEG-Y path")
    ap.add_argument("--output", help="output CSV path")
    ap.add_argument("--check-encoding", action="store_true",
                    help="only verify the sample encoding")
    args = ap.parse_args()

    if not os.path.exists(args.input):
        sys.exit("ERROR: input not found: %s" % args.input)

    if args.check_encoding:
        ok = check_encoding(args.input)
        sys.exit(0 if ok else 2)

    if not args.output:
        sys.exit("ERROR: --output is required unless --check-encoding is used")

    data, offsets, cdps, meta = read_segy(args.input)
    mat, offsets = convert(data, offsets)
    write_csv(args.output, mat, meta, offsets)

    # 自检：必须是 256 行 x 128 列纯数值（注释行不计）
    n_rows = 0
    n_cols = 0
    with open(args.output) as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            n_rows += 1
            n_cols = len(line.strip().split(","))
    if (n_rows, n_cols) != (TARGET_NT, TARGET_NX):
        sys.exit("ERROR: produced %dx%d, expected %dx%d"
                 % (n_rows, n_cols, TARGET_NT, TARGET_NX))

    print("input  : %s" % args.input)
    print("output : %s" % args.output)
    print("verified: %d data rows x %d numeric columns (comments excluded)"
          % (n_rows, n_cols))
    print("amp range: [%.4f, %.4f]" % (mat.min(), mat.max()))


if __name__ == "__main__":
    main()
