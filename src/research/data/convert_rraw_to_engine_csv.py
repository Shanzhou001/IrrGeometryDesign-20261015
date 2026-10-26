#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
convert_rraw_to_engine_csv.py
=============================
将当前 RRAW SEG-Y 资料（含五个 CDP）转换为引擎使用的 256x128 CSV。

背景
----
主重构接口要求 Nt=256 (dt=4 ms) x Nx=128 的数值矩阵。空间间距由实际 offset
生成，本资料为约 23.5433 m；12.5 m 是合成场的默认值。原资料为 59 道 / 250 样点 / 8 ms，
因此本脚本的每一步都写死参数并记录到 CSV 头部，保证任何人可逐位复现。

数据格式说明（重要）
--------------------
RRAW.SGY 的二进制头 bytes 3225-3226 声明 format code = 1（IBM 浮点），
既有处理明确采用 **IEEE-754 little-endian float32 配置**。IBM 大端解释会得到
约 1e25 量级，但这不能排除所有其他字节解释。--check-encoding 仅检查当前配置的
幅值合理性，不自动鉴定编码；格式冲突及不同配置的描述性证据见 v4 诊断记录。
不得根据隐藏测试 SNR 选择解码配置，源格式最终仍宜由资料提供方确认。

处理流程（参数固定）
--------------------
1. 读取 59 道，按 offset（字节 37-40, int32）升序排序；
2. 时间轴重采样 8 ms -> 4 ms：250 点线性插值到 256 点，覆盖 0 ~ 1.020 s；
   （固定历史诊断窗口；不能据此推断完整记录的恢复质量）
3. 道数重采样 59 -> 128：沿道方向线性插值（上采样 2.17x）；
4. 8-70 Hz 固定频带滤波（FFT 分箱上的线性斜坡 8-12 Hz / 60-70 Hz）；
5. 全局峰值归一化到 max|amp| = 1.0。

此空间插值/全场归一化产物仅用于历史诊断。严格真实道留出实验使用
audit_joint_studies 的原始振幅流程，不使用此产物作为独立测量真值。

用法
----
    python -m research.data.convert_rraw_to_engine_csv \
        --input  data/real_seismic/Data/RRAW.SGY \
        --output data/real_seismic/real_gather_256x128.csv --allow-unconfirmed-real

    # 仅检查编码判定
    python -m research.data.convert_rraw_to_engine_csv --input ... --check-encoding
"""  # add by wushanzhou in 2026/10/24

import argparse
import hashlib
import os
import struct
import sys
import json
from pathlib import Path
from research.common.provenance import add_real_data_arguments, real_source_provenance

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
    with open(path, "rb") as stream:  # add by wushanzhou in 2026/10/23
        raw = stream.read()  # add by wushanzhou in 2026/10/23
    if len(raw) < SEGY_TEXT_HDR + SEGY_BIN_HDR + TRACE_HDR:  # add by wushanzhou in 2026/10/23
        raise ValueError("truncated SEG-Y headers")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23

    # 重要：RRAW.SGY 的二进制头与道头**均为 little-endian**（与 SEG-Y 标准的
    # big-endian 相反）。字节级核对证据：
    #   idx 3220-3221 (1-based 3221) LE = 250 -> samples per trace，合理
    #   idx 3224-3225 (1-based 3225) LE = 1   -> format code，合理
    # 同一字段按大端会解释为 ns=64000、format=256；不是原注释所称的两个零。 # add by wushanzhou in 2026/10/24
    bin_hdr = raw[SEGY_TEXT_HDR:SEGY_TEXT_HDR + SEGY_BIN_HDR]
    ns = struct.unpack_from("<H", bin_hdr, 20)[0]        # 1-based 3221-3222
    dt_raw = struct.unpack_from("<H", bin_hdr, 16)[0]    # 1-based 3217-3218
    fmt = struct.unpack_from("<H", bin_hdr, 24)[0]       # 1-based 3225-3226

    # Actual bytes: binary dt and all 59 trace dt fields are 8000 us. The old "64" comment was wrong.  # add by wushanzhou in 2026/10/24
    dt_us = None  # add by wushanzhou in 2026/10/24

    trace_bytes = TRACE_HDR + ns * 4
    avail = len(raw) - (SEGY_TEXT_HDR + SEGY_BIN_HDR)
    if ns < 1 or avail % trace_bytes or avail < trace_bytes:  # add by wushanzhou in 2026/10/23
        raise ValueError("invalid sample count or incomplete SEG-Y trace payload")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23
    ntraces = avail // trace_bytes

    data = np.zeros((ns, ntraces), dtype=np.float64)
    offsets = np.zeros(ntraces, dtype=np.int64)
    cdps = np.zeros(ntraces, dtype=np.int64)

    for t in range(ntraces):
        off = SEGY_TEXT_HDR + SEGY_BIN_HDR + t * trace_bytes
        trace_ns = struct.unpack_from("<H", raw, off + 114)[0]  # add by wushanzhou in 2026/10/24
        trace_dt = struct.unpack_from("<H", raw, off + 116)[0]  # add by wushanzhou in 2026/10/24
        if trace_ns != ns or trace_dt <= 0 or (dt_us is not None and trace_dt != dt_us):  # add by wushanzhou in 2026/10/24
            raise ValueError(f"inconsistent trace ns/dt at trace {t}")  # add by wushanzhou in 2026/10/24
        dt_us = trace_dt  # add by wushanzhou in 2026/10/24
        cdps[t] = struct.unpack_from("<i", raw, off + 20)[0]
        offsets[t] = struct.unpack_from("<i", raw, off + 36)[0]
        # payload 按 IEEE-754 little-endian 解码（见模块说明）
        data[:, t] = struct.unpack_from("<%df" % ns, raw, off + TRACE_HDR)

    if not np.isfinite(data).all():  # add by wushanzhou in 2026/10/23
        raise ValueError("non-finite SEG-Y sample")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23

    if dt_us != 8000:  # add by wushanzhou in 2026/10/24
        raise ValueError("This RRAW conversion profile requires trace-header dt=8000 us; resampling constants would otherwise be incorrect")  # add by wushanzhou in 2026/10/24
    if dt_raw not in (0, dt_us):  # add by wushanzhou in 2026/10/24
        raise ValueError("binary and trace-header sampling intervals disagree")  # add by wushanzhou in 2026/10/24
    meta = {
        "ns": ns,
        "dt_us": dt_us,
        "dt_raw_field": dt_raw,
        "dt_trace_field": dt_us,  # add by wushanzhou in 2026/10/24
        "trace_headers_checked": ntraces,  # add by wushanzhou in 2026/10/24
        "payload_profile": "RRAW_IEEE_LE_override",
        "source_status": "unconfirmed",
        "field_claims_eligible": False,  # add by wushanzhou in 2026/10/24
        "declared_format": fmt,
        "ntraces": ntraces,
        "file_bytes": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
    }
    return data, offsets, cdps, meta


def check_encoding(path):
    """检查既定 IEEE-LE 配置的幅值合理性；不鉴定有争议的源格式。""" # add by wushanzhou in 2026/10/24
    data, offsets, cdps, meta = read_segy(path)
    finite = data[np.isfinite(data)]
    peak = float(np.abs(finite).max()) if finite.size else 0.0
    print("declared format code : %d (1 = IBM float)" % meta["declared_format"])
    print("decoded as IEEE-LE   : peak |amp| = %.1f" % peak)
    print("IEEE-LE plausibility : %s" % ("PASS" if peak < 1e6 else "FAIL")) # add by wushanzhou in 2026/10/24
    print("This check does not identify the encoding or resolve the header/profile conflict.") # add by wushanzhou in 2026/10/24
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
    x_src = offsets.astype(float)  # add by wushanzhou in 2026/10/23
    if not np.all(np.diff(x_src) > 0):  # add by wushanzhou in 2026/10/23
        raise ValueError("offsets must be distinct and increasing")  # add by wushanzhou in 2026/10/23
    x_dst = np.linspace(x_src[0], x_src[-1], TARGET_NX)  # add by wushanzhou in 2026/10/23
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
        fh.write("# RRAW legacy diagnostic reference field; source declaration is recorded in the provenance sidecar\n")
        fh.write("# payload_profile=RRAW_IEEE_LE_override; source_status=" + meta.get("source_status", "unconfirmed") + "\n")
        fh.write("# diagnostic_only=true; spatial interpolation and full-field normalization prevent strict raw-trace holdout claims\n")
        fh.write("# Source SHA256: %s\n" % meta["sha256"])
        fh.write("# Source: %d traces x %d samples, dt=%d us, declared format=%d "
                 "(payload interpreted using the retained IEEE-LE float32 override)\n"
                 % (meta["ntraces"], meta["ns"], meta["dt_us"], meta["declared_format"]))
        fh.write("# offsets_m=" + ",".join("%.9g" % x for x in np.linspace(offsets[0], offsets[-1], TARGET_NX)) + "\n")  # add by wushanzhou in 2026/10/23
        fh.write("# Offsets (m): %s\n" % ", ".join(str(int(o)) for o in offsets))
        fh.write("# Processing: offset-sorted; time 8->4 ms (250->256 via interp); "
                 "traces 59->128 via actual-offset interp; bandpass %.0f-%.0f Hz; peak-normalized to 1.0\n"  # add by wushanzhou in 2026/10/23
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
                    help="check fixed IEEE-LE profile plausibility, not encoding identification") # add by wushanzhou in 2026/10/24
    add_real_data_arguments(ap)
    args = ap.parse_args()

    if not os.path.exists(args.input):
        sys.exit("ERROR: input not found: %s" % args.input)

    if args.check_encoding:
        ok = check_encoding(args.input)
        sys.exit(0 if ok else 2)

    if not args.output:
        sys.exit("ERROR: --output is required unless --check-encoding is used")

    provenance = real_source_provenance(args.input, args.source_confirmation, args.allow_unconfirmed_real)
    data, offsets, cdps, meta = read_segy(args.input)
    meta["source_status"] = provenance["status"]
    mat, offsets = convert(data, offsets)
    write_csv(args.output, mat, meta, offsets)
    provenance.update(preprocessing="legacy spatial/time interpolation, filtering, full-field peak normalization",
                      strict_preprocessing_isolation=False, field_claims_eligible=False, output_sha256=hashlib.sha256(Path(args.output).read_bytes()).hexdigest())
    Path(args.output + ".provenance.json").write_text(json.dumps(provenance,indent=2),encoding="utf-8")

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
