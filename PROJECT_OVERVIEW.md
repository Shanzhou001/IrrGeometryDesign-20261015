# Viking 海上地震接收道选择与缺失道重建

本项目研究一条实际海上地震测线上的两个连续决策。给定有限的接收道数，先选择要保留的位置，再用保留道估计缺失波形。系统把布局搜索、恢复算法和评分接在同一流程上，使每项结果都能追溯到原始炮集、采样掩码和逐例预测。

当前研究数据为 Mobil AVO Viking Graben Line 12。正式实验入口为 Viking v2，候选恢复器在每个案例的 Viking 观测道上做内层选择。论文题目可表述为**《预算约束下的海上地震接收道选择与缺失道重建研究（以 Viking Graben Line 12 为例）》**。

## 问题与应用场景

一炮地震记录可以看成时间 × 接收位置的矩阵。一列是某个接收位置记录的时间波形。当接收数量、传输或处理能力有限时，需要决定保留哪些列，并利用已知列估计缺失列。等间隔抽稀可能产生相干空间混叠。无约束的随机抽稀可能留下较大的接收空档。即使从 120 道中只选 60 道，候选组合数也约为 \(9.66\times 10^{34}\)。

项目区分两类预算。**采样预算**限定保留道数，**搜索预算**限定候选布局目标的评价次数。研究问题是给定这两项预算，怎样生成满足覆盖要求的布局，以及几何目标改善后，实际缺失道恢复是否随之改善。Jitter 欠采样和最大空档控制的背景见 [Hennenfent 与 Herrmann 的波场重建研究](https://slim.gatech.edu/Publications/Public/Journals/Geophysics/2008/hennenfent08GEOsdw/hennenfent08GEOsdw.pdf)。

当前应用形态是离线实验工具。用户可以用已有炮集进行受控抽稀，比较不同布局和恢复器，再通过原始记录评分。这为布设方案设计和处理流程评估提供依据。现场应用还需要独立验证采集约束、固定布设策略、下游成像和费用模型。

## 数据与实验口径

| 项目 | 当前方案 |
|---|---|
| 实测资料 | [Viking Graben Line 12](https://wiki.seg.org/wiki/Mobil_AVO_viking_graben_line_12)，公开海上叠前炮集 |
| 输入位置 | `data/public/viking_line12_small/`，含原始 SEG-Y 子集、道头几何、派生窗、来源和使用说明 |
| 数据身份 | `src/research/data/viking_line12_subset.json` 登记炮号、采样和输入文件的 SHA-256 |
| 正式评分炮 | FFID 53、453、953，来自同一条二维测线 |
| 时间与空间 | 每炮 120 道，采样间隔 4 ms，接收间距按资料记录和相对几何解释为 25 m。每个评分窗 256 个样点，起点为样点 256、512、768 |
| 抽稀设计 | 三组共同隐藏道，每组 15 道。保留 36、60、84 道，分别约为 30%、50%、70% |
| 先导与预算 | 每种布局保留 12 条先导道，计入保留数。优化器目标评价上限为 300 次。Jitter 是初始布局参考 |
| 覆盖约束 | 加硬间距约束的布局在 36、60、84 道条件下分别使用 175、100、75 m 的最大空档上限 |
| 重复与验证 | 三个配对掩码种子，两个互不重叠的内层验证分组。每次分组都保留先导道 |
| 正式规模 | 3 炮 × 3 窗 × 3 隐藏集 × 3 保留数 × 3 种子 × 5 布局，共 1,215 个配置。统计解释限定为同一工区的描述性对照 |

数据来源的完整档案见 [Zenodo](https://zenodo.org/records/23481108)。本地交付的是带身份登记的子集。读取时核对 SEG-Y 字节格式、道头、坐标和窗内样点。原始道头的坐标单位码存在历史不一致，报告按数据说明和核实的相对道间距解释空间距离。

## 方法与实现

1. **读入炮集和几何。** 从登记的 SEG-Y 记录读取波形和接收位置，构造时间 × 道矩阵。共同隐藏道预先确定，在布局和恢复评估过程中隔离。
2. **生成初始布局。** 用 Jitter 在分区内抽取点位并合入先导道，形成满足初始覆盖要求的方案。同一条件下的算法共享掩码种子和初始布局。
3. **优化选道。** 用归一化 Fourier Gram 互干度或其等价的归一化掩码频谱旁瓣评价布局。模拟退火在给定温度下探索候选交换，离散局部搜索继续精修。自适应搜索可切换阶段并利用剩余评价次数。
4. **加入覆盖和信号信息。** `Adaptive_Gap` 对最大空档设硬约束。`Adaptive_Signal` 根据已观测先导道的频谱估计权重，在相同布点约束下使用加权旁瓣目标。约束、互干度和恢复质量分别评价。
5. **恢复缺失道。** 线性插值提供基准，曲波候选采用阈值和观测投影，f-k 路线用二维 FFT 和 FISTA 求解带傅里叶稀疏约束的重建。候选配置和数字增益从观测道内部的验证分组确定，最后用全部观测道重建并保留其测量值。
6. **独立评分。** 从登记输入重新读取目标，逐一核对掩码、预测数组、观测道一致性、互干度和 SNR／NMSE。NMSE 是共同隐藏道预测平方误差与目标能量的比值。逐例 SNR 为 `-10 log10(NMSE)`，汇总按已登记的炮点和实验条件进行。

核心算法是 **Jitter、模拟退火、离散交换、Fourier 互干度、线性插值、曲波阈值和 f-k FISTA**。旧接口中 `CG` 表示离散局部精修的兼容名称。论文和简历可以具体说明自己的工作是如何定义可行选道问题、如何控制搜索预算，以及如何检验每项改变的实际贡献。

## Viking 实测结果

以下为共同隐藏道上的平均 SNR，单位为 dB。表中挑出代表性对照以说明保留率和恢复器的作用，完整统计包含全部布局、时间窗、隐藏集与种子。

| 保留道 | 布局 | 固定冷启动曲波 | 固定 f-k | 内层选模 |
|---:|---|---:|---:|---:|
| 36 | Jitter | 1.838 | 0.990 | 2.367 |
| 36 | Adaptive_Mu | 2.056 | 1.183 | 2.454 |
| 60 | Jitter | 3.042 | 3.215 | 3.874 |
| 60 | SA_Mu | 3.275 | 3.476 | 4.080 |
| 60 | Adaptive_Gap | 3.085 | 3.354 | 4.097 |
| 84 | Jitter | 4.159 | 5.497 | 5.487 |
| 84 | Adaptive_Signal | 4.268 | 5.680 | 5.599 |

保留 60 道时，Adaptive_Gap 加内层选模的平均 SNR 比 Jitter 加内层选模高约 0.224 dB。固定 f-k 时，同档 SA_Mu 的 SNR 比 Adaptive_Gap 高约 0.122 dB。保留 84 道时，Adaptive_Signal 加固定 f-k 的结果为 5.680 dB。布局和恢复方法会共同影响排序，因此论文应给出对应的恢复规则、对照和适用条件。

完整证据位于 [Viking 实测报告](results/viking_only/report.html)、[配对结果](results/viking_only/viking/analysis/paired_effects.csv)、[独立核验](results/viking_only/viking_audit/verification.json) 和 [图件目录](figures/index.html)。当前统计支持在这条测线和登记协议下的效果比较。需要验证跨工区规律时，要在新的测线或场景开展独立试验。

本版从新目录重新运行全部配置。独立核验覆盖 351 次布局搜索和 4,860 份预测数组，观测道投影误差为 0，评分最大复算差约为 `1.78e-15`。冻结方案、逐例结果表、汇总表和配对统计与上一版 Viking 表逐字节一致。见[本轮复算证据](output/viking_acceptance/result_replay.json)。

## 运行与复核

在 `IrrGeometryDesign` 目录执行。项目使用 Python 3.12，依赖见 `requirements.txt`。本地 C++ 构建使用仓库已有的 Qt4／VS2008 工具链。Release 和 Debug 构建命令分别为 `python -B pipeline.py build --mode Release` 和 `python -B pipeline.py build --mode Debug`。

```powershell
python -m pip install -r requirements.txt
python -B pipeline.py closure-status
python -B pipeline.py run
```

Qt 运行包位于 `output/runtime_viking/`。从“空间优化与科研试验 → 毕业实验与结果图库”进入 **Viking 实测实验与结果图库**，填写 Python 解释器并运行。Qt 和命令行使用同一 Viking 工作流。已有验收批次时会检查批次身份并刷新图库。为重新计算指定全新目录：

```powershell
python -B pipeline.py thesis --scope viking --workers 8 --out results/viking_replay
python -B pipeline.py closure-status
```

用完整独立审计重新评分时，指定实际批次和一个尚未存在的输出目录。审计会读取逐例预测与原始输入，耗时长于状态检查：

```powershell
python -B pipeline.py module research.validation.verify_viking_batch --root results/viking_only/viking --out results/viking_reaudit
```

`results/active.txt` 指向正式活动批次。`workflow.json` 登记完成的步骤和证据哈希。`viking/frozen_plan.json` 保存完整实验协议，`viking/recovery.csv` 保存每个配置的指标，`viking/predictions/` 保存预测和掩码，`viking_audit/verification.json` 保存独立核验。`figures/current.catalog.json` 是 Qt 与网页共用的图件清单。

| 目录或模块 | 作用 |
|---|---|
| `src/native/` | C++ 布局搜索、曲波恢复、Qt 窗口和原生数值验证 |
| `src/research/data/viking.py` | 登记数据身份、SEG-Y 道头、窗口读取及样点核验 |
| `src/research/experiments/viking_v2.py` | 固定协议、先导权重、布局搜索和逐例恢复 |
| `src/research/recovery.py` | f-k FISTA、恢复候选和观测道内选模 |
| `src/research/experiments/viking_workflow.py` | CLI 与 Qt 共用的运行、验收和发布入口 |
| `src/research/validation/verify_viking_batch.py` | 从原始记录和保留预测独立重算指标 |

仅含 Viking 数据的干净目录完成了 205 项 Python 回归，199 项通过，6 项为可选历史输入测试跳过，零失败。Release 和 Debug 的核心、数值、投影、流程和 Viking 实测集成检查均通过。清理项目目录后，带依赖的运行包再次通过 Qt 入口、三张图件加载、翻页、缩放和刷新检查。见[干净目录检查](output/viking_acceptance/clean_directory_checks.json)和[清理后 Qt 验收](output/viking_acceptance/gui_after_cleanup.json)。

## 论文与项目介绍口径

开题和中期的主线是地震采样点筛选与实测重建。当前实现将范围收敛到二维海上测线的接收道选择，技术上使用实际执行的离散交换精修，并验证布局指标与恢复结果之间的关系。一个适合论文答辩或项目演示的陈述是：

> 我以 Viking 海上地震实测炮集为对象，研究在固定接收道数和搜索预算下的布点与缺失道恢复。系统用 Jitter 构造共同起点，用模拟退火和离散交换优化布局，并加入最大空档和先导频谱约束。恢复阶段比较线性、曲波和 f-k FISTA，采用共同隐藏道评分。在 1,215 个配置中，我分析了采样比例、布点和恢复器对波形误差的影响，给出可复算的结果及其适用范围。

从求职角度，项目对应**信号处理算法、地震数据处理、科学计算软件和启发式优化**。技术面试可以展示一份炮集怎样经过道头核验、抽稀、重建和误差复算，并用固定恢复器的配对比较解释为什么同一套实验中会出现不同布局排名。
