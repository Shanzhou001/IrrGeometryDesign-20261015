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

### 1. 运行自动化单元测试与回归校验体系（Task 11）

```bat
x64\Release\IrrGeometryDesign.exe --test
:: 或使用别名: -t, --run-tests
```

**测试覆盖 5 大模块**：
1. **采样点数精确一致性与二进制合法性**：验证 30%（38点）、50%（64点）、70%（90点）下四种采样方式的点数严格相等，值仅为 0 或 1，无重复不越界；
2. **随机数发生器可复现性**：验证同一随机种子下 `uniform()` 与 Box-Muller `gaussian()` 的确定性重现，以及种子切换后的一致性隔离；
3. **模拟退火单调性与点数保持**：验证 Metropolis SA 历史最优轨迹严格单调非增，且最终最优解点数精确保持 64 点；
4. **空间属性评估模式分支与安全错误处理**：验证当无真实工程数据时，严格模式（`allowSyntheticFallback=false`）明确返回失败，模拟回退模式（`allowSyntheticFallback=true`）生成合法的 576 CMP 面元基准；
5. **波场重构基线对比与数据一致性检验**：验证掩码丢失时报错拦截，合法运行时通过观测道残差一致性检验（$|d_{\text{recon}} - d_{\text{obs}}| < 10^{-5}$）。

### 2. 一键执行全套科研闭环对比试验（Task 10）

```bat
x64\Release\IrrGeometryDesign.exe --run-experiments [输出目录, 默认 experiment_results]
:: 或使用别名: -e
```

执行后将依次运行三个子试验，并在控制台输出包含耗时、状态、指标摘要的执行报告，同时向输出目录导出全套 CSV 数据与文本总结。

---

## 四、数学指标定义与科学解读

### 1. 采样基准与曲波相干度 $\mu$

- **目标采样点数守恒**：$m = \text{round}(N \cdot c)$，其中 $N=128$。对于 30%、50%、70% 采样率，全算法基准点数统一为 **38、64、90**。
- **规则减抽空间映射**：$i_k = \text{floor}(k \cdot N / m)$，确保覆盖整个空间区间 $[0, N-1]$，避免端点聚集。
- **相干度比值衰减指标（Coherence Reduction Ratio）**：
  $$\text{Coherence Reduction (dB)} = 20 \cdot \log_{10}\left(\frac{\mu_{\text{regular}}}{\mu_{\text{optimized}}}\right)$$
  - *说明*：该指标衡量优化非规则采样相比规则减抽在变换域相干度峰值的相对衰减分贝数，**并非直接等于野外地震波场假频能量降低量**。
  - *统计口径*：50% 采样率下的单项提升（75.38%, 12.17 dB）与全采样率（30%, 50%, 70%）均值提升（56.31%, 7.75 dB）在报告中严格区分。

### 2. 观测系统空间属性双模式

系统严格区分两种运行模式，并在所有输出 CSV 第一行通过 `# Mode:` 元数据标明：
- **模式 A（Synthetic Benchmark Mode，合成模拟基准）**：当项目未加载 SPS 时启用，基于 24×24（576 个）有效 CMP 面元网格模拟三维观测系统，平均覆盖次数 31.7 次，变异系数 $C_v = \sigma / \bar{x} = 0.1558$（覆盖均匀度极佳），炮检距覆盖 50 m ~ 3200 m，全方位玫瑰图照射完整度 100%。
- **模式 B（Real Survey Evaluation Mode，真实工程评价）**：从已加载的 `GProject` 中提取实际检波线（`RLines`）、炮线（`SLines`）和激活面元（`pCellGrid->getCoverCount()`），计算真实的覆盖次数直方图、炮检距与方位角统计。**若工程数据为空且未显式允许回退，系统立即返回失败并报错，坚决杜绝静默伪造真实数据**。

### 3. 地震波场压缩感知重构与评价

- **网格维度**：数值计算网格为 $256 \times 128$（时间样点 $N_t=256$, $dt=0.004\text{ s}$；空间道数 $N_x=128$, $dx=12.5\text{ m}$）；导出网格与输入分辨率一致。
- **数据源支持**：支持外部真实野外单炮集 CSV 文件输入；若未指定则自动采用含薄层反射、断层倾斜事件与 Box-Muller 高斯白噪声的三层物理波场模型。
- **重构算法**：采用二维 FK 变换稀疏域迭代硬阈值收缩与凸集精确投影（Projection Onto Convex Sets, POCS），迭代 40 次。
- **对比基准**：
  1. **零值填充初始基准（Zero-Fill Baseline）**：未采样道置零；
  2. **一维空间线性插值基准（Linear Interpolation Baseline）**：相邻已采样道做空间一维线性插值；
  3. **POCS 压缩感知恢复**：评价相比零值填充净提升（$\Delta \text{SNR}_{\text{zero}} = +3.30\text{ dB}$）与空间插值对比指标。
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
│   ├── benchmark_comparison_table.csv                 # 30%、50%、70% 四种算法指标对比表
│   ├── benchmark_convergence_curves.csv               # 模拟退火当前解探索与历史最优收敛轨迹
│   └── benchmark_summary_report.txt                   # 详细文本评估报告（含参数、均值与单项对比）
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

使用系统默认随机种子（`Seed = 42`）生成的核心科研指标基准值如下：

### 1. 多采样率相干度对比基准表 (`benchmark_comparison_table.csv`)

| 采样率 | 采样点数 $m/N$ | 规则减抽 $\mu_{\text{reg}}$ | 纯随机 $\mu_{\text{rand}}$ | Jitter 初始 $\mu_{\text{jit}}$ | Jitter-SA 优化 $\mu_{\text{opt}}$ | 较规则改善率 | 相干度衰减 (dB) |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **30%** | 38 / 128 | 0.7892 | 0.4379 | 0.5737 | **0.3729** | +52.75% | 6.51 dB |
| **50%** | 64 / 128 | 0.9601 | 0.3661 | 0.3472 | **0.2364** | **+75.38%** | **12.17 dB** |
| **70%** | 90 / 128 | 0.2609 | 0.1914 | 0.2355 | **0.1545** | +40.80% | 4.55 dB |
| **均值**| — | — | — | — | — | **+56.31%** | **7.75 dB** |

### 2. 波场重构关键定量指标 (`recon_metrics_summary.csv`)

- **数据模式**：Synthetic Numerical Wavefield (3-Layer Reflection Model + Gaussian White Noise)
- **掩码来源**：Auto-Generated Jitter Mask ($m=64/128$, 50.0%)
- **零值填充初始基准 SNR**：$3.02\text{ dB}$
- **空间线性插值基准 SNR**：$12.35\text{ dB}$
- **POCS 压缩感知重构 SNR**：**$6.32\text{ dB}$**
- **相比零值填充净提升量**：**$+3.30\text{ dB}$**
- **均方根误差 (RMSE)**：$0.07678$
- **皮尔逊相关系数 ($r$)**：**$0.8820$**
- **观测道一致性检验**：**PASSED**（实测道最大偏差 $< 10^{-5}$）
- **算法迭代与耗时**：40 步迭代，耗时约 $600 \sim 800\text{ ms}$
