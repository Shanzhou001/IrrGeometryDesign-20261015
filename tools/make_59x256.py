#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
make_59x256.py
==============
生成 `data/real_seismic/real_gather_59x256.csv`（留出验证输入）。

背景
----
留出验证（`--holdout-validate` / `--holdout-batch`）使用 **原始 59 道** 道集，
只做 8 ms -> 4 ms 时间重采样，**不做道数插值**。此前仓库中只有产物 CSV，
没有生成脚本，无法复现。本脚本补齐这一环。

与 convert_rraw_to_engine_csv.py 的区别：
  - 那个脚本输出 256x128（插值到 128 道，供主重构流程）；
  - 本脚本输出 256x59（保留原始道数，供留出验证）。

处理步骤（固定参数，与已提交文件一致）
--------------------------------------
1. IEEE-LE float32 解码（该 SEG-Y 头声明 format=1/IBM，但 payload 实为 IEEE-LE）；
2. 8 ms -> 4 ms 线性重采样：250 -> 256 点；
3. 峰值归一化。

用法
----
    python tools/make_59x256.py --input data/real_seismic/Data/RRAW.SGY \
        --output data/real_seismic/real_gather_59x256.csv --verify
"""
import argparse
import hashlib
import os
import struct
import sys

import numpy as np

TARGET_NT = 256
TARGET_DT_MS = 4.0
SRC_DT_MS = 8.0
SEGY_TEXT_HDR = 3200
SEGY_BIN_HDR = 400
TRACE_HDR = 240


def read_segy(path):
    raw = open(path, "rb").read()
    bin_hdr = raw[SEGY_TEXT_HDR:SEGY_TEXT_HDR + SEGY_BIN_HDR]
    ns = struct.unpack_from("<H", bin_hdr, 20)[0]
    fmt = struct.unpack_from("<H", bin_hdr, 24)[0]
    dt_raw = struct.unpack_from("<H", bin_hdr, 16)[0]
    trace_bytes = TRACE_HDR + ns * 4
    ntraces = (len(raw) - (SEGY_TEXT_HDR + SEGY_BIN_HDR)) // trace_bytes

    data = np.zeros((ns, ntraces), dtype=np.float64)
    offsets = np.zeros(ntraces, dtype=np.int64)
    for t in range(ntraces):
        off = SEGY_TEXT_HDR + SEGY_BIN_HDR + t * trace_bytes
        offsets[t] = struct.unpack_from("<i", raw, off + 36)[0]
        data[:, t] = struct.unpack_from("<%df" % ns, raw, off + TRACE_HDR)
    meta = {
        "ns": ns,
        "ntraces": ntraces,
        "dt_us": 8000,          # 由文本头 "Resampled 8.0 ms" 与 ns=250 印证
        "dt_raw_field": dt_raw,
        "declared_format": fmt,
        "sha256": hashlib.sha256(raw).hexdigest(),
    }
    return data, offsets, meta


def convert(data, offsets):
    """按偏移距升序排序 -> 时间重采样 -> 峰值归一化（与已提交文件一致）。"""
    order = np.argsort(offsets, kind="stable")
    data = data[:, order]
    offsets = offsets[order]
    ns, nx = data.shape
    t_src = np.arange(ns) * SRC_DT_MS
    t_dst = np.arange(TARGET_NT) * TARGET_DT_MS
    out = np.zeros((TARGET_NT, nx))
    for j in range(nx):
        out[:, j] = np.interp(t_dst, t_src, data[:, j])
    peak = np.abs(out).max()
    if peak > 0:
        out /= peak
    return out, offsets


def write_csv(path, mat, meta):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", newline="\n") as fh:
        fh.write("# Real field gather, ORIGINAL %d traces (NO trace interpolation)\n" % mat.shape[1])
        fh.write("# time 8->4 ms resample only (%d->%d samples); offset-sorted; peak normalized\n"
                 % (meta["ns"], TARGET_NT))
        fh.write("# Geometry: Nt=%d, Nx=%d\n" % (mat.shape[0], mat.shape[1]))
        for i in range(mat.shape[0]):
            fh.write(",".join("%.6g" % v for v in mat[i, :]) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True)
    ap.add_argument("--output", required=True)
    ap.add_argument("--verify", action="store_true",
                    help="与已存在的输出逐值比较（容差 1e-6 相对）")
    args = ap.parse_args()

    if not os.path.exists(args.input):
        sys.exit("ERROR: input not found: %s" % args.input)

    data, offsets, meta = read_segy(args.input)
    mat, offsets = convert(data, offsets)
    print("source: %d traces x %d samples, declared format=%d, sha256=%s..."
          % (meta["ntraces"], meta["ns"], meta["declared_format"], meta["sha256"][:16]))
    print("output shape: %d x %d (offset-sorted, %d..%d m)"
          % (mat.shape[0], mat.shape[1], int(offsets[0]), int(offsets[-1])))
    if mat.shape != (TARGET_NT, meta["ntraces"]):
        sys.exit("ERROR: unexpected output shape %s" % (mat.shape,))

    if args.verify and os.path.exists(args.output):
        old = []
        for line in open(args.output, encoding="utf-8", errors="replace"):
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            old.append([float(x) for x in s.split(",")])
        old = np.array(old)
        same_shape = old.shape == mat.shape
        maxdiff = float(np.abs(old - mat).max()) if same_shape else float("nan")
        print("verify: existing shape=%s, max|diff|=%.3e" % (old.shape, maxdiff))
        # 已提交文件按 %.6g 写出，逐值比较容差取 1e-6
        ok = same_shape and maxdiff < 1e-6
        print("verify: %s" % ("IDENTICAL (数据行逐值一致)" if ok else "DIFFERS"))
        if not ok:
            sys.exit(2)
        return

    write_csv(args.output, mat, meta)
    print("written: %s" % args.output)


if __name__ == "__main__":
    main()
