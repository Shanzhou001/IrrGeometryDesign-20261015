# IrrGeometryDesign：地震采样设计与重构验证

> 2026-10-23 后续修订：当前复现脚本默认输出 `results/enhanced_20261023/`，协议 `physical_psf_v3`。新增真正 SA-only 重构、实际坐标选点和物理距离软约束、10 种子二维基准、固定参数分 CDP 诊断及严格缓存校验。下面 `classic_psf_v2` 的数字与 `revised_20261023/` 对应此前归档快照；最新结果及边界见 `docs/REPAIR_NOTES_20261023.md` 和新目录 `RESULTS.md`。两版本真实选点目标不同，不能混为同一次实验。

<!-- add by wushanzhou in 2026/10/23 -->

最新完整复现仍使用下文的构建与运行命令，`python tools/run_revised_studies.py` 默认写入增强版目录；`python tools/verify_revised_results.py` 默认复核增强版。`--sampling-2d` 新增 `--seeds`；合成和真实掩码研究均显式区分 `SAOnly` 与 `Hybrid`。缓存同时校验命令、源码、脚本、运行环境、输入、程序和已归档输出，缺失或变更的输出会重新运行。

新增和上一轮改动已按实际代码位置补注 `add by wushanzhou in 2026/10/23`；24 日补充修复标为 `2026/10/24`，历史署名改名时保留原日期。无日期的旧署名使用补注日期。运行 `python tools/audit_change_annotations.py` 可核对补注覆盖，`--apply` 会实际补注，新增注释使用执行当天日期；默认比较基线为 `63d1408`。C++ 使用 `//`、Python 使用 `#`、批处理使用独立 `rem`，数据文件与历史结果不插入源码注释。

<!-- add by wushanzhou in 2026/10/23 -->

<!-- add by wushanzhou in 2026/10/24 -->

本项目研究给定采样点数下的地震采样设计，以及采样几何、互干度和波场重构之间的关系。工程为 C++ / Qt 4.7.4 / Visual Studio 2008 SP1 x64；Python 用于资料转换、统计与独立复核。

**当前增强版协议为 `physical_psf_v3`；下文基础归档为 `classic_psf_v2`。** 增强版结果位于 [`results/enhanced_20261023/`](results/enhanced_20261023/)，基础结果位于 [`results/revised_20261023/`](results/revised_20261023/)。两版本分别归档，不应拼接不同目标下的数字。

增强版已完成 12 个正式步骤、630 次合成重构、210 次真实掩码留出和 150 行二维对照；24 项 C++、12 项 Python 回归及 1,680 行独立复核通过。完整证据见 [验证汇总](results/enhanced_20261023/validation/validation_summary.json)。

<!-- add by wushanzhou in 2026/10/24 -->

用户确认只有一份 59 道真实波形资料。本项目没有新增独立真实工区；不同随机留出划分也不计为独立真实资料。

## 当前实现和结论边界

- 正式采样基准、消融、空间约束和掩码—重构研究，统一使用经典 Fourier/PSF 互干度；二维实验实际优化二维 PSF。
- SA、局部搜索、混合搜索、随机重启采用相同的 **200 次目标评估上限**。局部搜索允许提前停止，实际评估次数和耗时分别报告；不把上限相同写成实际耗时完全相同。
- SA-only 与混合搜索共享初始掩码、搜索种子及前 64 次 SA 评估。SA 前缀单独保存，局部精修的贡献可以直接复算。
- 带约束方法明确优化 `J = μ + λ_gap P_gap + λ_nn P_nn`。它与纯 μ 方法的比较属于权衡研究，不进入同目标算法优劣的 Holm 检验族。
- 每一类掩码都保存为文本，记录种子、预算、目标值、状态和规范化二值序列 SHA1；重构指标记录同一掩码哈希。
- 一键流程默认将本次采样基准的 50% 最优掩码直接传给重构。真实 SPS 是另一份独立几何资料，不声称与这份合成波场来自同一工区。
- 真实留出验证采用实际 offset 坐标。幅度标定仅从已知道计算，留出振幅不参与求解输入或标定；评价恢复到原输入振幅单位。

**核心研究问题仍是：降低 μ 能否改善最终重构，以及空间覆盖约束能否改善权衡。** μ 的降低不能直接写成物理假频能量降低，也不能替代缺失道重构精度。

当前结果与机器可读汇总见 [RESULTS.md](results/revised_20261023/RESULTS.md)、[summary.json](results/revised_20261023/summary.json)；论文方法与讨论草稿见 [docs/THESIS_METHODS.md](docs/THESIS_METHODS.md)。这些文件以正式实验实际输出为准。

本轮结果显示：Hybrid 在 μ 目标下优于等上限 Restart，但低于 SA-only；空间约束在三个模型的九个采样率组中有八组改善了无约束 Hybrid 的平均缺失道 SNR，仍有例外，且未普遍超过普通 Jitter 或规则采样。真实研究的各组平均 POCS SNR 仍低于零填充。论文主线据此收敛为“空间约束与重构效应评估”，不主张算法普遍最优。

## 构建和完整复现

依赖：Windows x64、VS2008 SP1 x64 工具链、Qt 4.7.4、FFTW 2.1.5 和仓库内 Eigen。Qt 路径由 `QTDIR` 或 `qmake.exe` 定位；VS 路径可用 `VCVAUTO` 覆盖。编译产物为 `x64\Release\IrrGeometryDesign.exe`。

```bat
build_release.bat
x64\Release\IrrGeometryDesign.exe --test
python -m pip install -r requirements.txt
python tools\test_science_tools.py
python tools\run_revised_studies.py
python tools\verify_revised_results.py results\enhanced_20261023
```

<!-- add by wushanzhou in 2026/10/24 -->

已验证的 Python 环境为 Python 3.12.10 / NumPy 2.5.3。C++ 构建不依赖 Python。含中文的 C++ 文件保持 UTF-8 BOM / CRLF，以兼容 MSVC 9.0。

`run_revised_studies.py` 执行采样基准、20 种子消融及统计、10 种子空间约束、二维基准、三组合成模型的掩码重构、真实参考场对照、真实留出批次及 10 种子真实掩码研究。遇到子进程失败立即退出，不用不完整结果生成成功结论。

```bat
:: 小规模流程检查，单独写入 revised_20261023_smoke
python tools\run_revised_studies.py --quick

:: 可复用匹配二进制和输入的已成功步骤
python tools\run_revised_studies.py --resume
```

运行清单 [`run_manifest.json`](results/revised_20261023/run_manifest.json) 记录命令、退出码、耗时、程序和源码哈希、输入哈希及运行库版本。图件写入 `figures/revised_20261023/`；历史图件保留。逐次重构矩阵和运行子目录可重建，不纳入 Git；汇总、掩码和轨迹保留。

## 数学口径

固定总格点数 `N`，保留点数为 `m = round(N·c)`。`N=128` 时，30% / 50% / 70% 对应 38 / 64 / 90 点；所有方法使用相同点数。

正式一维目标为：

\[
\mu(\Omega)=\max_{\Delta=1,\ldots,N-1}\frac1m\left|\sum_{k\in\Omega}e^{-2\pi i\Delta k/N}\right|.
\]

二维目标为二值掩码二维 FFT 除 DC 外的最大模，除以采样点数 `m`。选择与报告使用同一二维掩码，不再用逐行平均值替代跨行相干性。16×16 基准在整个网格上统一守恒点数；规则、抖动初值均说明为展平序列构造，不声称是二维最优分层设计。

空间间隔包括左右边界：

\[
g_{max}=\max\{k_0,k_{i+1}-k_i,N-1-k_{m-1}\}.
\]

`NN-CV` 为每个采样点最近邻距离的标准差除以均值。软惩罚使用 `P_gap=max(0,g_max/(N/m)-1)`、`P_nn=NN-CV`。单元占用率按 `[ceil(iN/m), ceil((i+1)N/m))` 的互不重叠整数单元统计。新版 Count 抖动在全部采样率下都保证每个保留点单元恰一点，高采样率不再采用“对丢弃点抖动”的不同定义。

硬约束先检查必要可行性条件 `(m+1)·g_max_bound >= N-1`，再构造可行初值。正式硬约束采用 `ceil(N/m)`；例如 `N=128,m=38,g<=3` 被明确拒绝，不再把不可行结果标为成功。

旧 `CurveletMiu1DEigenParam` 是曲波系数块 Gram 相对参考 Gram 的偏差，保留作历史 GUI 接口及数学诊断。它不是当前经典互干度目标，不能把两者混写成一个 μ。旧多尺度诊断也不用于宣称当前优化目标具有曲波字典下的理论保证。

## 搜索协议

主混合方法先运行 64 次评估的 Metropolis SA（含初始评估），再对采样点做保持点数的 ±1 贪心局部移动，总上限 200。SA-only 延续同一温度日程和随机流至相同上限；Local-only 从同一初值直接开始；随机重启从同一初值和新抖动候选中保留最优。

温度日程固定为 `0.1·0.01^((eval-2)/63)`，有效温度最低截断到 `1e-8`。这个版本用于可复算的受控对照，不称为全局最优保证。原接口中沿用的 `CG` 名称指历史局部搜索，不是标准共轭梯度法；正式结果用 SA / Local / Hybrid / Restart 表述。

软约束默认 `λ_gap=λ_nn=0.3`，另扫描 0、0.1、0.3、1、3、10；敏感性覆盖三档采样率和 10 个种子。默认权重不是在真实留出 SNR 上选出的最优值。

## 重构和真实坐标

重构实现是 **POCS 式曲波收缩与观测投影**：4 个尺度、16 个粗尺度方向、最高频尺度用小波，保留最粗尺度，其他系数采用 Wiener 型收缩 `|c|²/(|c|²+λ_s²)`。阈值余弦调度从 0.0265 降到 0.0005，最高频阈值加倍，默认 40 次迭代。不能把该非线性收缩直接写成已经证明收敛的经典凸投影算法。

合成实验同时报告零填充、线性插值、热启动重构；主表优先使用仅缺失道 SNR。全幅 SNR 还包含精确保留的已知道，不能用于掩盖缺失道误差。冷／热启动对照由主套件另行输出。

真实原始文件为 `data/real_seismic/Data/RRAW.SGY`，59 道、250 样点、文本头指示 8 ms，包含 5 个 CDP 值。头部的浮点格式声明与实际 IEEE little-endian payload 不一致，转换程序按已核对的实际编码处理。

```bat
python tools\make_59x256.py --input data\real_seismic\Data\RRAW.SGY --output data\real_seismic\real_gather_59x256.csv
python tools\convert_rraw_to_engine_csv.py --input data\real_seismic\Data\RRAW.SGY --output data\real_seismic\real_gather_256x128.csv
```

- **59 道留出输入：**只做时间重采样、0–1.020 s 窗口截取和峰值归一化，保留原始道数和实际 offset 元数据。
- **128 道参考场：**按实际 offset 重采样为均匀网格，另做 8–70 Hz 带通。网格范围 −1560～1430 m，道距为 `2990/127≈23.5433 m`，不是 12.5 m。
- **留出求解：**从已知道构造初值，在均匀网格上进行曲波收缩，用实际坐标的线性观测算子投影回已知测量；评价位置仍是原来的实测道坐标。求解器幅度标定只用已知道峰值，恢复后再换回评价单位。
- **参考场局限：**128 道参考场由插值构造，其 SNR 偏向插值先验，不能作为真实完整地下波场精度。真实主证据是 59 道实测值的留出评价。

真实掩码研究的**选择目标仍按排序后的道序号定义**，空间惩罚也是序号口径；重构和米制最大间隔使用实际坐标。这一研究检验当前选择方法的真实后果，不宣称已经实现连续非均匀坐标上的采样优化。

## 实验与统计

正式归档包括：

| 目录 | 内容 |
|---|---|
| `synthetic/` | 10 轮 × 3 采样率基准，及相同优化掩码的合成重构 |
| `ablation_multiseed/` | 20 种子 × 3 档；保存所有方法和共同 SA 前缀掩码 |
| `constrained/` | 5 方案 × 3 档 × 10 种子；另有 6 λ × 3 档 × 10 种子 |
| `benchmark_2d/` | 同点数的 5 种二维掩码及真正二维目标 |
| `mask_recon_builtin/` | 三层反射合成模型，6 类掩码 × 3 档 × 10 实例 |
| `mask_recon_dipping/` | 倾斜反射模型，同样 180 次重构 |
| `mask_recon_faulted/` | 断层错移反射模型，同样 180 次重构 |
| `real_sps/` | 独立真实工区的覆盖、偏移距、方位及抽稀评价 |
| `real_field/` | 128 道插值参考场口径对照 |
| `real_field_holdout/` | 原 6 组 keep × seed 配置，使用修正坐标与标定 |
| `real_mask_study/` | keep=20/30/40，6 类掩码，10 个种子，共 180 次留出 |

新增两组合成模型的事件、噪声、种子和矩阵 SHA256 在 `data/synthetic_models/*.json` 中；它们是受控合成场景，不能替代独立真实资料。

为兼容旧表结构，`mask_recon_study.csv` 中 `mask_type=SA` 指本版本的 **Hybrid（SA＋局部精修）**，不指 SA-only。真实掩码表直接使用 `Hybrid`。

统计对同一采样率、同一种子的配对结果使用 Wilcoxon 符号秩检验；小样本采用含并列中秩的精确符号置换分布，动态规划计算。报告真实 Student-t 尾概率作为参考，并给出配对秩二列相关效应量及另行注明的 Cliff's δ。

Holm 校正族只包含 3 档 × 4 个同目标对照，共 12 项。不把 Constrained 的不同目标混入同目标优越性检验。当前 Wilcoxon 的对称差值分布假设、单数据集限制，以及确定性规则掩码重复种子的非独立性，应在论文中说明。

## 回归与独立复核

`--test` 共 **23 项**，除原有工程回归外，还检查目标一致、预算守恒、SA 前缀相同、抖动单元、硬约束可行性、矩形二维 FFT 与直接 DFT 的一致性、非法留出 CSV、留出数据隔离、幅度缩放等变性、坐标投影一致性、优化掩码传递及真实道距元数据。

Python 有 **7 项**检查，包含 Student-t 的解析值／临界值、含并列值的符号置换穷举、退化差值、Holm、实际 offset 插值和相关分析的并列秩。

`verify_revised_results.py` 用 NumPy 独立计算 Fourier PSF，复核归档掩码点数、SHA1、二维值、SA 前缀、重构输入哈希和真实观测一致性。测试通过说明这些实现合同成立，不等于方法在任意资料上有优势。

本轮构建、23/7 项回归、1,425 行独立核验及预处理误差记录见 [validation_summary.json](results/revised_20261023/validation/validation_summary.json)。科研图件的收敛曲线为代表性单次运行，柱状图为多种子汇总；主套件重构使用 μ 最优掩码，三者用途不同，应在论文图注中分别说明。

## 论文可主张的范围

论文应分别回答：在相同评估上限下谁更有效地降低 μ；加入空间约束付出多少 μ 代价；缺失道重构是否改善；真实资料是否优于零填充和线性插值。不能只选择其中有利的一个指标。

真实 SPS 抽稀后平均覆盖次数约减半，即使 CV 或方位扇区变化较小，也不能写“全部属性无损”。点数减少也不能直接等同真实总施工成本同比下降。

SPS 主报告使用项目面元网格；抽稀对照在其内部统一网格上计算完整与抽稀配置。比较时应使用同一份 `attribute_thinning_comparison.csv` 的基准行和抽稀行，不能把不同报告的面元数拼成一张改善表。

本版本尚未实现三维采样与重构基准、速度分析／NMO／叠加／成像验证，也没有新增真实独立数据集。关于真实泛化、成像质量和实际施工收益的主张仍需额外资料支持。
