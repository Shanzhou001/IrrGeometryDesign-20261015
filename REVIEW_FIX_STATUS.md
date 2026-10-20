# THESIS_CODE_REVIEW 修复状态总表（第二轮后更新）

> **2026-10-20 下午更新**：上一版列出的 13 项未修复事项已**全部处理完毕**，本表为最终状态。

## 一、第二轮完成项（13/13）

| # | 事项 | 状态 | 产出 |
| :---: | :--- | :---: | :--- |
| 1 | 均匀性三指标 | ✅ | `benchmark_runs_detail.csv` 新增 9 列；报告新增 Uniformity 段 |
| 2 | SA/CG/等预算消融 | ✅ | `--ablation` → `results/ablation/hybrid_ablation.csv` |
| 3 | 抖动/SA 掩码留出验证 | ✅ | `--holdout-validate --mask` 支持外部掩码 |
| 4 | 抽稀后 SPS 属性重算 | ✅ | `attribute_thinning_comparison.csv` |
| 5 | μ 度量重构 | ✅ | 块级 Gram，多尺度 μ 真正随参数变化 |
| 6 | GUI 2D 流程统一 | ✅ | 12 处 `UnOptiMiuSA` → `UnOptiMiuSAEx(neighbor=true)` |
| 7 | 构建脚本路径化 | ✅ | `build_*.bat` 支持 QTDIR/VCVAUTO/VCARCH |
| 8 | 乱码注释修复 | ✅ | `gproject.h` 78 行 + `gproject.cpp` 47 行；C4819 归零 |
| 9 | 仓库清理 | ✅ | 删除重复 sps.txt、.ncb/.suo/moc_test/.user；.gitignore 扩展 |
| 10 | Test 5 CG 回退真触发 | ✅ | 新增回退断言与恶化场景 |
| 11 | Test 2 覆盖断言 | ✅ | 改为末点公式 + 跨度 + 间隔范围三重校验 |
| 12 | 2D 基准 | ✅ | `--sampling-2d` → `results/benchmark_2d/sampling_2d_benchmark.csv` |
| 13 | 重建/回归/基准复跑/文档 | ✅ | 15/15 PASS；基准 0.3460/0.2238/0.1438 逐位复现 |

## 二、本轮新发现的科学结论（必须写入论文）

1. **SA 破坏了空间均匀性**：最大间隔 4.20→6.87，NN-CV 0.353→0.488，抖动单元占用率仅 56.31%。
   "Jitter-SA 保持均匀分布" 在当前实现下不成立。
2. **混合优化优势有限**：相比等预算随机重启 +0.46~+1.14 dB；70% 档 SA+CG 反而差于 CG-only。
3. **参数鲁棒性证据消失**：修正 μ 度量后，7 组参数中仅 4 组保持 SA<Rand<Reg；且多尺度 μ 超出 [0,1]（CSV 已标 OUT_OF_RANGE）。
   所以论文只能写"单尺度（1,4）下 Jitter-SA 最优”，不能写"结论不依赖参数"。
4. **留出验证中抖动掩码实有帮助**：抖动掩码 +1.17 dB vs 随机掩码 +0.93 dB（同 seed 同 k）。
5. **2D 改善幅度高于一维基准**：SA 相对二维规则仅 34.41%/69.58%/16.64%，不应把一维结论外推。

## 三、当前验证基线

```text
构建      build_release.bat（已路径化）     0 error(s)，C4819=0
回归      --test                          PASS=15 FAIL=0 SKIP=0
基准      --repeat 10 --seed 42          0.3460 / 0.2238 / 0.1438，12.71 dB
波场      3.00 / 11.83 / 13.90           缺失道 8.82 / 10.90
消融      --ablation                      SA+CG vs 抖动重启 +0.50/+1.14/+0.46 dB
均匀性    benchmark report              max gap 2.67/7.90/4.20/6.87
2D        --sampling-2d --grid 32         34.41% / 69.58% / 16.64%
属性      attribute_thinning_comparison  9072→4536 检波点，fold 106.93→53.62，24/24 扇区不变
```

## 四、仍存在的客观限制（非代码缺陷）

| 项 | 说明 |
| :--- | :--- |
| 3D 未实现 | 本项目仅做到 2D；3D 需新建 fdct3d 基准与存储引擎 |
| 多尺度 μ 超出 [0,1] | 块级 Gram 偏差的理论上界不是 1；已在 CSV 标注 OUT_OF_RANGE，论文需声明口径 |
| 属性重算用等间隔抽稀 | 只作"抽稀影响”示意，未与 Jitter-SA 掩码对接（需 2D 完整流程） |
| 1D/2D μ 口径差异 | 2D μ 为逐行 1D μ 均值，不包含跨线相干性 |
