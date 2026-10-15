# IrrGeometryDesign-20261015：非规则地震观测系统设计与科研验证系统

本项目为面向复杂地表与高效勘探场景开发的**非规则地震观测系统设计、曲波变换相干度优化与波场压缩感知（Compressed Sensing）闭环验证系统**。系统基于 C++、Qt 4.7.4 和 Visual Studio 2008 SP1 (x64) 架构开发，构建了从野外观测系统空间离散采样、模拟退火算法优化、空间属性综合质量评价到地震波场保真重构的完整学术闭环。

---

## 一、系统核心模块与科学背景

在地震勘探采集设计中，野外接收道数的减少能够显著降低装备投入与野外施工成本。然而：
- **规则减抽（Regular Decimation）**：在空间波数域产生严重的离散周期性相干尖峰（空间假频/Aliasing），破坏有效波场；
- **纯随机抽样（Pure Random Subsampling）**：虽然将假频相干尖峰打散为不相关噪声，但在野外物理空间容易出现检波点过度聚集与大尺度无采样孔洞（Sampling Gaps）；
- **本项目采用的 Jitter 抖动网格与模拟退火优化（Jitter-SA）**：基于 Hennenfent & Herrmann (2008) 提出的抖动网格理论，结合经典 Metropolis 接受准则的模拟退火算法，在保证空间采样点均匀受限分布的同时，全局最小化曲波变换 Gram 矩阵的最大相干度 $\mu$，实现假频能量的最优随机化抑制。

### 核心源码模块与职责划分

| 模块文件 | 核心职责 |
| :--- | :--- |
| `IrrGeometryDesign/sample.h`<br>`IrrGeometryDesign/Sampling.cpp` | **空间采样算法引擎**：提供严格点数守恒（$m = \text{round}(N \cdot c)$）的规则抽样（`Sampling1R_Count`）、纯随机抽样（`Sampling1S_Count`）、抖动网格抽样（`Sampling1J_Count`）与分段随机抽样（`Sampling1F_Count`），并包含严密的二进制掩码合法性校验（`ValidateSamplingMask`）。 |
| `IrrGeometryDesign/OptimizeMiu.h`<br>`IrrGeometryDesign/OptimizeMiu.cpp` | **模拟退火相干度全局优化**：实现经典 Metropolis 准则（$p = \exp(-\Delta / T)$）与进度退火策略；严格保持解交换扰动时的采样点数精确守恒；分别记录马尔可夫链探索轨迹（`mu_trace`）与历史最优单调非增轨迹（`best_mu_trace`）；集成共轭梯度（CG）精修。 |
| `IrrGeometryDesign/CalculateMiu.h`<br>`IrrGeometryDesign/CurveletMiu.cpp` | **曲波变换与相干度评估**：基于一维/二维快速离散曲波变换（FDCT Wrapping）与快速傅里叶变换（FFTW 2.1.5），构建标准化 Gram 矩阵，精确计算空间采样算子与曲波基之间的最大非对角相干度 $\mu$。 |
| `IrrGeometryDesign/spsmaskbuilder.h`<br>`IrrGeometryDesign/spsmaskbuilder.cpp` | **SPS 数据处理与掩码提取**：支持 SPS（R点、S点、X道对）野外标准测线坐标解析，构建真实 CMP 面元与野外激发接收网格掩码。 |
| `IrrGeometryDesign/geometryexperimentengine.h`<br>`IrrGeometryDesign/geometryexperimentengine.cpp` | **科研验证全套驱动引擎**：驱动多采样率对比基准、空间属性分析与二维波场凸集投影（POCS）压缩感知重构，输出标准 CSV 数据表与文本报告。 |
| `IrrGeometryDesign/ExpRng.h`<br>`IrrGeometryDesign/ExpRng.cpp` | **科学计算可复现伪随机数引擎**：基于 Xorshift128+ 算法实现高周期、确定性播种（`seed`）、状态检查（`current_seed`）与 Box-Muller 高斯白噪声生成（`gaussian`）。 |
| `IrrGeometryDesign/main.cpp` | **应用程序主入口**：支持图形交互界面启动与自动化命令行运行参数（`--run-experiments`, `--test`）。 |

---

## 二、开发环境与构建指南

### 1. 工具链与依赖要求

- **操作系统**：Windows 7 / 10 / 11 (x64)
- **编译器**：Microsoft Visual Studio 2008 SP1 (MSVC 9.0) x64 Cross/Native Compiler
- **GUI 框架**：Qt 4.7.4 for VS2008 (x64)，需确保 `QTDIR` 配置正确且 `bin` 目录加入系统 `PATH`
- **数学库**：FFTW 2.1.5 (x64 动态库 `fftw.dll` 已随项目提供)
- **文件编码规范**：所有包含中文注释与提示的 C++ 头文件与源文件**必须保存为 UTF-8 带签名（UTF-8 with BOM，`utf-8-sig`）**，以确保 MSVC 9.0 代码页（CP936）正常解析且不发生跨行截断。

### 2. 构建命令

项目根目录下提供了经过严密验证的一键编译批处理脚本：

```bat
:: 编译 Release x64 版本（优化性能，推荐用于科学计算实验）
build_release.bat

:: 编译 Debug x64 版本（包含完整调试符号）
build_debug.bat
```

构建生成的二进制文件位于：
- `x64\Release\IrrGeometryDesign.exe`
- `x64\Debug\IrrGeometryDesign.exe`

---

## 三、命令行参数与自动化运行

系统无需依赖图形交互界面即可在终端完整运行科研验证流程：

#### 1. 运行自动化单元测试与回归校验体系（Task 11 / Section X）

```bat
x64\Release\IrrGeometryDesign.exe --test
:: 或使用别名: -t, --run-tests
```

**测试覆盖 9 大核心科研与工程校验模块**：
1. **采样点数精确一致性与二进制合法性 (P0 / Task 1)**：验证 30%（38点）、50%（64点）、70%（90点）下四种采样方式的点数严格守恒，掩码值仅为 0 或 1，无重复不越界；
2. **规则减抽空间分布公式与全区间覆盖验证 (P1 / Section IV)**：验证严格公式 $i_k = \lfloor k \cdot N / m \rfloor$，确保首端覆盖 0、末端覆盖区间尾部、无连续聚集；
3. **随机数发生器可复现性与主/实种子独立性 (P0 / Section III)**：验证 Xorshift128+ 与 Box-Muller 算法在同一随机种子下的严格复现性与跨种子独立性；
4. **多随机种子重复试验计算与统计量检验 (P0 / Section III)**：验证 `repeatRuns > 1` 时的独立循环执行、独立种子派生与严格复现性；
5. **模拟退火历史最优非增性与 CG 精修防恶化保护 (P1 / Section V)**：验证 Metropolis SA 历史最优轨迹严格单调非增，且在 CG 精修导致相干度升高时强制回退保护最优解；
6. **采样掩码文件严格校验与非法场景拦截 (P1 / Section VII)**：对不存在文件、空文件、截断文件 (127点)、多余文件 (129点)、非法非二值字符等场景实现 100% 拦截并报错；
7. **地震波场压缩感知重构基线对比与观测一致性检验 (P1 / Section VI)**：验证零值填充、空间线性插值基准与 POCS 重构，并强制检验观测实测道无损一致性（误差 $< 10^{-5}$）；
8. **真实 SPS 观测系统模式严格防护与文件完整性审计 (P1 / Section VIII)**：验证真实工程模式下未初始化工程对象时坚决拒绝并明确报错（绝不静默伪造真实数据），同时审计 `sps/5151-5171` 的物理文件完整性；
9. **子实验失败状态正确向上传播与退出码控制 (P0 / Section II)**：验证子实验失败时主流程立即捕获并返回非零退出码（退出码 1），确保外部自动化流水线能够准确感知错误。

### 2. 一键执行全套科研闭环对比试验（Task 10）

```bat
x64\Release\IrrGeometryDesign.exe --run-experiments [输出目录, 默认 experiment_results] [--repeat <N>] [--seed <S>]
:: 或使用别名: -e, -r, -s
```

参数说明：
- `输出目录`（可选）：默认 `experiment_results`；
- `--repeat <N>`（可选）：每个采样率独立重复试验轮数，默认 3 轮（可设为 1 或更多）；
- `--seed <S>`（可选）：主随机数种子，默认 42。

执行后将依次运行三个子试验，并在控制台输出包含耗时、状态、指标摘要的执行报告，所有子实验成功时返回退出码 `0`，任何子实验失败时返回非零退出码 `1`。

---

## 四、数学指标定义与科学解读

### 1. 采样基准与曲波相干度 $\mu$

- **目标采样点数守恒**：$m = \text{round}(N \cdot c)$，其中 $N=128$。对于 30%、50%、70% 采样率，全算法基准点数统一为 **38、64、90**。
- **规则减抽空间映射**：$i_k = \lfloor k \cdot N / m \rfloor$，确保覆盖整个空间区间 $[0, N-1]$，避免端点聚集。
- **相干度比值衰减指标（Coherence Ratio Attenuation, dB）**：
  $$\text{Coherence Ratio Attenuation (dB)} = 20 \cdot \log_{10}\left(\frac{\mu_{\text{regular}}}{\mu_{\text{optimized}}}\right)$$
  - *说明*：该指标衡量优化非规则采样相比规则减抽在变换域相干度峰值的相对衰减分贝数，**反映对相干噪声与离散空间假频能量的抑制能力，并不等同于全波场总能量假频分量**。
  - *统计口径*：提供每轮独立实验详细记录（`benchmark_runs_detail.csv`）与均值/方差/极值汇总表（`benchmark_comparison_table.csv`）。

### 2. 观测系统空间属性双模式

系统严格区分两种运行模式，并在所有输出 CSV 第一行通过 `# Mode:` 元数据标明：
- **模式 A（Synthetic Benchmark Mode，合成模拟基准）**：当项目未加载 SPS 时启用，基于 24×24（576 个）有效 CMP 面元网格模拟三维观测系统，平均覆盖次数 32.07 次，变异系数 $C_v = \sigma / \bar{x} = 0.1577$（覆盖均匀度极佳），炮检距覆盖 50 m ~ 3200 m，全方位玫瑰图照射完整度 100%。
- **模式 B（Real Survey Evaluation Mode，真实工程评价）**：从已加载的 `GProject` 中提取实际检波线（`RLines`）、炮线（`SLines`）和激活面元（`pCellGrid->getCoverCount()`），计算真实的覆盖次数直方图、炮检距与方位角统计。**若工程数据为空且未显式允许回退，系统立即返回失败并报错，坚决杜绝静默伪造真实数据**。
- *工程注意*：现有 `gspsscanner.cpp` 中的遗留解析器使用固定列切片，在解析标准 SPS 2.1 格式时存在格式兼容限制；在未对遗留解析器进行重构前，真实模式严格进行防御性报错，确保学术严谨性。

### 3. 地震波场压缩感知重构与评价（深层地球物理机制解析）

- **网格维度**：数值计算网格为 $256 \times 128$（时间样点 $N_t=256$, $dt=0.004\text{ s}$；空间道数 $N_x=128$, $dx=12.5\text{ m}$）；导出网格与输入分辨率一致。
- **数据源支持**：支持外部真实野外单炮集 CSV 文件输入；若未指定则自动采用含薄层反射、双曲同相轴与 Box-Muller 高斯白噪声的三层物理波场模型。
- **重构算法**：采用二维 FK 变换稀疏域迭代硬阈值收缩与凸集精确投影（Projection Onto Convex Sets, POCS），迭代 40 次。
- **定量对比基准**：
  1. **零值填充初始基准（Zero-Fill Baseline）**：未采样道置零，$\text{SNR} = 3.02\text{ dB}$；
  2. **一维空间线性插值基准（Linear Interpolation Baseline）**：相邻已采样道做空间一维线性插值，$\text{SNR} = 12.35\text{ dB}$；
  3. **POCS 压缩感知恢复**：$\text{SNR} = 6.76\text{ dB}$，相比零值填充净提升 $+3.75\text{ dB}$。
- **深层地球物理与数学原理解析（为何在合成单炮集上线性插值 SNR 高于标准 2D FK POCS）**：
  1. **双曲反射波在 2D 傅里叶基中的非稀疏性**：双曲同相轴在 $(f, k)$ 频率-空间波数域并非离散的稀疏脉冲，而是呈连续宽带扇形分布。全局硬阈值收缩容易滤除倾斜时差较平缓处的弱反射能量，造成振幅平滑；
  2. **有限空间孔径与边缘吉布斯效应**：128 道有限空间孔径截断导致傅里叶变换产生边缘窗谱泄漏与振铃效应，在无观测道位置引入残余波动；
  3. **高密度小道距有利于局部相关性插值**：合成道距 $dx=12.5\text{ m}$ 且主要为 1 道孤立孔洞时，相邻道波形相关系数极高（时差 $< 1\text{ ms}$），因此局部空间线性插值表现出极高数值指标；
  4. **学术闭环与发展方向**：POCS 算法严格验证了压缩感知凸集投影重建机制（相比零值填充实现 $+3.75\text{ dB}$ 净增益）；对于复杂构造双曲或弯曲波场，基于局部多尺度多方向的曲波（Curvelet）/地震子波（Seislet）或动校正（NMO）引导稀疏变换是突破傅里叶基局限的科学必由之路。
- **观测道一致性强制检验**：
  $$\max_{j \in \Omega_{\text{obs}}} \|d_{\text{recon}}(:, j) - d_{\text{obs}}(:, j)\|_\infty < 10^{-5}$$
  确保压缩感知算法在填补缺失道时，绝对不篡改已观测实测道的真实记录。

---

## 五、实验成果文件目录索引

每次运行生成的全部结果均保存在 `experiment_results/` 目录下：

```text
experiment_results/
├── cmp_mask.txt                                       # 全局统一重构采样掩码 (1=保留道, 0=抽取道, 严格64点)
│
├── 01_sampling_benchmark/                             # 试验 1：不同采样率对比试验
│   ├── benchmark_runs_detail.csv                      # 所有独立重复试验每轮测量的完整明细表
│   ├── benchmark_comparison_table.csv                 # 30%、50%、70% 四种算法均值、方差与极值基准表
│   ├── benchmark_convergence_curves.csv               # 模拟退火当前解探索与历史最优收敛轨迹
│   └── benchmark_summary_report.txt                   # 详细文本评估报告（含随机种子、参数与结论）
│
├── 02_attribute_evaluation/                           # 试验 2：观测系统空间属性质量评估
│   ├── attribute_fold_distribution.csv                # CMP 面元覆盖次数直方图与百分比
│   ├── attribute_offset_distribution.csv              # 16 级炮检距区间分布表
│   ├── attribute_azimuth_rose.csv                     # 24 扇区方位角玫瑰图统计分布
│   └── attribute_quality_report.txt                   # 覆盖变异系数、成本节约率与玫瑰图评分报告
│
└── 03_wavefield_reconstruction/                       # 试验 3：地震波场压缩感知重构试验
    ├── cmp_mask.txt                                   # 试验实际使用的二值掩码持久化文件
    ├── recon_original_gather.csv                      # 全分辨率原始参考波场单炮集 (256x128)
    ├── recon_subsampled_gather.csv                    # 50% 欠采样观测波场单炮集 (256x128)
    ├── recon_recovered_gather.csv                     # POCS CS 恢复波场单炮集 (256x128)
    ├── recon_residual_slice.csv                       # 重构残差空间切片 (256x128)
    ├── recon_metrics_summary.csv                      # 信噪比、均方根误差、相关系数与耗时汇总表
    └── recon_experiment_report.txt                    # 重构科研报告与基线对比分析
```

---

## 六、科研复现核验基准数据

使用系统默认主随机种子（`Seed = 42`，重复 3 轮独立试验）生成的核心科研指标基准值如下：

### 1. 多采样率相干度对比基准表 (`benchmark_comparison_table.csv`)

| 采样率 | 采样点数 $m/N$ | 规则减抽 $\mu_{\text{reg}}$ | 纯随机均值 $\mu_{\text{rand}}$ | Jitter-SA 优化均值 $\mu_{\text{opt}}$ | SA 标准差 $\sigma$ | [极小值, 极大值] | 较规则平均改善率 | 相干度比值衰减 (dB) |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **30%** | 38 / 128 | 0.7892 | 0.4595 | **0.3614** | 0.0247 | [0.3338, 0.3938] | +54.21% | 6.80 dB |
| **50%** | 64 / 128 | 0.9601 | 0.3242 | **0.2240** | 0.0051 | [0.2196, 0.2311] | **+76.67%** | **12.64 dB** |
| **70%** | 90 / 128 | 0.2609 | 0.2356 | **0.1333** | 0.0079 | [0.1245, 0.1438] | +48.90% | 5.85 dB |
| **全率均值** | — | — | — | — | — | — | **+59.93%** | **8.43 dB** |

### 2. 波场重构关键定量指标 (`recon_metrics_summary.csv`)

- **数据模式**：Synthetic Numerical Wavefield (3-Layer Reflection Model + Gaussian White Noise)
- **掩码来源**：External File (`cmp_mask.txt`，来自 50% Jitter-SA 优化掩码，严格 64 点)
- **零值填充初始基准 SNR**：$3.02\text{ dB}$
- **空间线性插值基准 SNR**：$12.35\text{ dB}$
- **POCS 压缩感知重构 SNR**：**$6.76\text{ dB}$**
- **相比零值填充净提升量**：**$+3.75\text{ dB}$**
- **均方根误差 (RMSE)**：$0.072965$
- **皮尔逊相关系数 ($r$)**：**$0.8923$**
- **观测道一致性检验**：**PASSED**（实测道最大偏差 $< 10^{-5}$）
- **算法迭代与耗时**：40 步迭代，耗时约 $790\text{ ms}$
