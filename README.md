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
| `IrrGeometryDesign/ExpRng.h`<br>`IrrGeometryDesign/ExpRng.cpp` | **科学计算可复现伪随机数引擎**：基于 xorshift64* 算法实现高周期、确定性播种（`seed`）、状态检查（`current_seed`）与 Box-Muller 高斯白噪声生成（`gaussian`）。 |
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

**测试覆盖 15 大核心科研与工程校验模块**：
1. **采样点数精确一致性与二进制合法性 (P0 / Task 1)**：验证 30%（38点）、50%（64点）、70%（90点）下四种采样方式的点数严格守恒，掩码值仅为 0 或 1，无重复不越界；
2. **规则减抽空间分布公式与全区间覆盖验证 (P1 / Section IV)**：验证严格公式 $i_k = \lfloor k \cdot N / m \rfloor$，确保首端覆盖 0、末端覆盖区间尾部、无连续聚集；
3. **随机数发生器可复现性与主/实种子独立性 (P0 / Section III)**：验证 xorshift64* 与 Box-Muller 算法在同一随机种子下的严格复现性与跨种子独立性；
4. **多随机种子重复试验计算与统计量检验 (P0 / Section III)**：验证 `repeatRuns > 1` 时的独立循环执行、独立种子派生与严格复现性；
5. **模拟退火历史最优非增性与 CG 精修防恶化保护 (P1 / Section V)**：验证 Metropolis SA 历史最优轨迹严格单调非增，且在 CG 精修导致相干度升高时强制回退保护最优解；
6. **采样掩码文件严格校验体系 (任务二 9 组验收测试)**：覆盖正常 128 点 64 道、127 点截断、129 点多余、全零、全一、63 道、65 道、非二值字符（2, -1, 文本）、文件不存在与空文件，并严格验证错误信息包含预期长度、实际长度、预期点数、实际点数四要素；
7. **波场重构失败状态传播与数据一致性约束 (任务一验收)**：验证正常波场重构成功、人为注入实测道数据一致性失败时严格返回失败并截断向上传播、以及输出目录或文件不可写时的失败拦截；
8. **外部地震数据 CSV 严格加载校验与杜绝静默回退 (任务三验收)**：验证缺省路径触发合成数值模拟、合法 256x128 CSV 正常加载并标识真实模式、文件不存在、空文件、行数不符 (200行)、列数不符 (100列)、非法数值/NaN/Inf 严格报错拦截，绝不静默伪造合成数据；
9. **真实 SPS 观测系统模式端到端评估与严格防护 (P1 / Section VIII)**：验证真实工程模式下未初始化工程对象时坚决拒绝并明确报错（绝不静默伪造真实数据），审计 `data/sps/5151-5171` 物理文件完整性，并实测加载真实野外 SPS 样本成功构建工程与 CMP 面元网格，断言炮点数、检波点数、有效道对数与有效面元数均严格大于 0；
10. **全套科研闭环套件故障向上传播与退出状态检验 (P0 / Task 1)**：验证全套试验流水线在子试验失败时（如损坏掩码）立即截断并返回失败，汇总报告明示失败项，CLI 严格返回退出码 1。
11. **冷启动/热启动消融实验与初始化贡献分解 (2026.10.19 新增)**：验证在同一掩码与参数下，冷启动 POCS 严格优于零填充基线、热启动 POCS 严格优于冷启动、主流水线 SNR 与所声明初始化分支一致、两路收敛轨迹长度等于迭代数，且消融指标确实写入 `recon_metrics_summary.csv`（防止热启动口径被无意回退）。
12. **曲波变换参数鲁棒性 (2026.10.19 新增)**：在 7 组 (nbscales, nbangles_coarse) 参数下重算三方案 μ，断言 "Jitter-SA < 纯随机 < 规则减抽" 排序全部稳定、基线 (1,4) 存在、各组数值落在 [0,1] 归一化相干度有效区间（防止多尺度 μ 计算缺陷回归）。

> **测试可移植性说明**：SPS 样本路径通过 `QCoreApplication::applicationDirPath()` 向上回溯与 `IRR_SPS_DIR` 环境变量解析，**不再依赖任何硬编码绝对路径**，因此在任意工作目录下运行均可获得 15/15 全通过。

### 2. 一键执行全套科研闭环对比试验（Task 10）

```bat
x64\Release\IrrGeometryDesign.exe --run-experiments [输出目录, 默认 results/synthetic] [--repeat <N>] [--seed <S>]
:: 或使用别名: -e, -r, -s
```

参数说明：
- `输出目录`（可选）：默认 `results/synthetic`；
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
- **模式 A（Synthetic Benchmark Mode，合成模拟基准）**：当项目未指定外部数据且允许合成回退时启用，基于 24×24（576 个）有效 CMP 面元网格模拟三维观测系统，平均覆盖次数 32.07 次，变异系数 $C_v = \sigma / \bar{x} = 0.1577$（覆盖均匀度极佳），炮检距覆盖 50 m ~ 3200 m，全方位玫瑰图照射完整度 100%。
- **模式 B（Real Survey Evaluation Mode，真实工程评价）**：工程解析器（`gspsscanner.cpp`）已完成鲁棒性升级，新增 SPS 2.1 固定列宽精确解析（`gParseXSPS21Fixed`）与分词回退机制，能完美解析野外测线点号（如 `1001.001`、`6858.001`）与 `.S`、`.R`、`.X` 关联拓扑。在 CLI 命令行全套实验及单元测试中，系统自动加载仓库自带的真实 SPS 样本（`data/sps/5151-5171`），构建真实的 `GProject`、`GCellGrid` 面元网格与炮检对坐标，输出真实观测系统的覆盖次数直方图、炮检距与方位角照射分布。**若工程数据为空且显式禁用回退，系统立即返回失败并报错，坚决杜绝静默伪造真实数据**。

### 3. 地震波场压缩感知重构与评价（深层地球物理机制解析）

- **网格维度**：数值计算网格为 $256 \times 128$（时间样点 $N_t=256$, $dt=0.004\text{ s}$；空间道数 $N_x=128$, $dx=12.5\text{ m}$）；导出网格与输入分辨率一致。
- **数据源支持**：支持外部真实野外单炮集 CSV 文件输入；若未指定则自动采用含薄层反射、双曲同相轴与 Box-Muller 高斯白噪声的三层物理波场模型。
- **重构算法**：采用**二维快速离散曲波变换（2D Discrete Curvelet Transform, FDCT Wrapping / IFDCT Wrapping）凸集精确投影（POCS）**，迭代 40 次。
- **定量对比基准与科研证据链**：
  1. **零值填充初始基准（Zero-Fill Baseline）**：未采样道置零，$\text{SNR} = 3.02\text{ dB}$（基准参照）；
  2. **一维空间线性插值基准（Linear Interpolation Baseline）**：相邻已采样道做空间一维线性插值，$\text{SNR} = 11.83\text{ dB}$；
  3. **改进曲波域 POCS 压缩感知恢复（Curvelet POCS CS）**：$\text{SNR} = \mathbf{13.90\text{ dB}}$，相比零值填充净提升 $\mathbf{+10.90\text{ dB}}$，**显著超越空间线性插值基准 $\mathbf{+2.07\text{ dB}}$**，均方根误差降至 $0.032070$，波形皮尔逊相关系数高达 $0.9794$。

- **⚠ 初始化方式消融实验（冷启动 vs 热启动，验证性必读）**

  主流水线采用**线性插值热启动（Warm-Start）**，因此上述 13.90 dB 中同时包含了“线性插值先验”与“曲波 POCS 迭代”两部分贡献。为保证学术对比的公平性，系统内置受控消融实验（`SNR_POCS_ColdStart` / `SNR_POCS_WarmStart`，对应测试 Test 11），在**完全相同的掩码、迭代次数与参数**下仅改变初值：

  | 实验路径 | SNR (dB) | RMSE | 说明 |
  | :--- | :---: | :---: | :--- |
  | ① 零值填充（无迭代） | 3.02 | — | 基线参照 |
  | ② 线性插值（无迭代） | 11.83 | — | 插值基线 |
  | ③ **零值填充 + 曲波 POCS（冷启动）** | **6.24** | 0.077459 | **标准 POCS 口径，与文献可比** |
  | ④ **线性插值 + 曲波 POCS（热启动）** | **13.90** | 0.032070 | 本项目主口径 |

  **贡献分解**：
  - 热启动初值本身贡献 $11.83 - 3.00 = 8.83\text{ dB}$；
  - 曲波 POCS 迭代在冷启动下贡献 $6.24 - 3.00 = \mathbf{3.24\text{ dB}}$（**这是曲波 POCS 方法自身可比的净贡献**）；
  - 曲波 POCS 迭代在热启动下贡献 $13.90 - 11.83 = \mathbf{2.07\text{ dB}}$；
- 热启动相对冷启动的总增益为 $7.66\text{ dB}$（$13.90 - 6.24$）。

  **结论解读**：曲波 POCS 在两种初值下均为正向增益（+3.24 dB / +2.07 dB），方法有效性成立；但**主口径 13.90 dB 的大部分来自线性插值先验而非曲波稀疏约束**。论文中若引用 14.10 dB，必须同时声明热启动设置并给出冷启动对照值 6.24 dB，否则会高估曲波方法本身的贡献。

- **深层地球物理与数学原理解析（曲波变换如何突破傅里叶基局限超越空间线性插值）**：
  1. **双曲弯曲同相轴在曲波域的最优稀疏表达**：地震反射波双曲弯曲同相轴在 2D 傅里叶 $(f, k)$ 频率-空间波数域呈现宽带连续展布，缺乏离散稀疏性；而 2D Curvelet 基具备严格的多尺度、局部各向异性（Anisotropic Scaling Law: $\text{width} \approx \text{length}^2$）与楔形方向扇区切分特性，能够以极少非零曲波系数最优逼近弯曲波前；
  2. **空间线性插值预热初始化（Warm-Start）**：利用局部插值先验消除零值填充引入的极端高频阶梯状混叠噪声，为非凸/高维迭代提供高度可靠的初始解路径（**量化贡献见上述消融表**）；
  3. **尺度自适应 Wiener 收缩与余弦退火阈值策略**：保护背景粗尺度与平滑成分，采用余弦退火逐步精细抑制高频方向子带中的非相干假频与随机噪声，彻底消除了傅里叶基固有的边界吉布斯振铃与假频泄漏；
  4. **严格实测观测道物理数据一致性投射**：
     $$X_{\text{recon}}[\text{mask}==1] \equiv X_{\text{obs}}[\text{mask}==1], \quad \max_{j \in \Omega_{\text{obs}}} \|d_{\text{recon}}(:, j) - d_{\text{obs}}(:, j)\|_\infty < 10^{-5}$$
     在每次逆曲波变换后严格执行已知道数据位级替换，实测最大误差绝对为 $0.000000$（严格满足 $< 10^{-5}$ 判定准则），确保填补未观测道的同时对已知道保持零畸变；
  5. **科研闭环成立**：实现了从“前端基于曲波互相关度 $\mu$ 的 Jitter-SA 采样设计优化”到“后端曲波域保真波场重构”的高精度闭环验证。

---

## 四点五、实验基准与数据版本对照（指标唯一来源）

> **本节是全部对外指标的唯一定义来源。** README 其余章节的数值均须与本表一致；若不一致，以本表为准。

### 4.5.1 当前正式基准（合成数值模拟）

| 项目 | 取值 |
| :--- | :--- |
| 执行命令 | `x64\Release\IrrGeometryDesign.exe --run-experiments results\synthetic --repeat 10 --seed 42` |
| 结果目录 | `results/synthetic/`（采样基准 + 波场重构）；真实 SPS 属性在 `results/real_sps/attribute_evaluation/` |
| 数据模式 | 合成三层反射模型 + Box-Muller 高斯白噪声（`exprng::seed(1234567)`） |
| 网格 | Nt=256 (dt=4 ms), Nx=128 (dx=12.5 m) |
| 采样掩码 | **自动生成 Jitter 掩码，固定种子 `exprng::seed(20261020)`，m=64/128** |
| POCS 迭代 | 40 次；主口径 = 线性插值热启动 |
| 图件输出 | `figures/`（7 张） |

**关键指标（合成）**：零填充 3.00 dB / 线性插值 11.83 dB / 热启动 POCS **13.90 dB**（相对线性 **+2.07 dB**）/ 冷启动 POCS 6.24 dB / RMSE 0.032070 / r 0.9794。

### 4.5.2 当前正式基准（真实野外资料）

| 项目 | 取值 |
| :--- | :--- |
| 执行命令 | `x64\Release\IrrGeometryDesign.exe --run-real-reconstruction data\real_seismic\real_gather_256x128.csv --out results\real_field --ratio 0.50` |
| 结果目录 | `results/real_field/` |
| 输入 | `data/real_seismic/Data/RRAW.SGY` → `data/real_seismic/real_gather_256x128.csv` |
| 验证边界 | **参考场为 59 道经插值构造的 128 道，不是原始实测完整波场**（详见第八节） |
| 图件输出 | `figures/real/` |

**关键指标（真实资料）**：零填充 3.04 dB / 线性插值 9.85 dB / 热启动 POCS **9.23 dB**（相对线性 **−0.62 dB**）/ 冷启动 POCS 6.85 dB。

### 4.5.3 历史版本对照

| 版本 | 线性插值 | 热启动 POCS | 相对线性 | 冷启动 POCS | 差异原因 |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **v2026.10.20（当前）** | 11.83 | 13.90 | +2.07 | 6.24 | 自动抖动掩码，固定种子 20261020 |
| v2026.10.15（历史） | 12.35 | 14.10 | +1.75 | 5.90 | 使用另一份外部掩码 `cmp_mask.txt`（未固定种子） |

> **结论**：两组数字并非同一实验配置下的结果，差异来自**采样掩码不同**（已实测：三份不同掩码在同一合成道集上分别得到 11.34 / 12.55 / 12.37 dB 的 POCS SNR）。因此不可混用，也不可用后者覆盖前者。当前正式基准一律采用 11.83 / 13.90 这一组。

### 4.5.4 消融实验（算法对比，非正式基准）

冷/热启动消融结果（`SNR_POCS_ColdStart` / `SNR_POCS_WarmStart`）用于论证热启动的贡献，**不替代**上面的正式基准。真实资料上热启动相对线性插值为负（−0.62 dB），已在报告中如实标注。

## 五、项目目录结构与实验成果索引

### 5.1 顶层目录（2026-10-20 整理后）

```text
IrrGeometryDesign-20261015/
├── README.md                 项目说明（本文件）
├── build_release.bat         一键编译 Release x64
├── build_debug.bat           一键编译 Debug x64
├── IrrGeometryDesign.sln     Visual Studio 解决方案
│
├── IrrGeometryDesign/        全部源代码（VS 工程名，保持原路径）
├── data/                     所有输入数据
│   ├── sps/                  SPS 真实观测系统样本 5151-5171.{S,R,X}
│   ├── real_seismic/         真实野外资料 RRAW.SGY + 转换后 CSV
│   └── masks/                导出的二值采样掩码
├── figures/                  ★ 唯一图片目录（所有 PNG 都在这里）
│   ├── 00~06_*.png           合成数值模拟 7 张主成果图
│   ├── index.html            图件画廊
│   ├── real/                 真实野外资料验证图（4 张）
│   └── gui/                  GUI 界面截图（9 张）
├── results/                  数值结果（CSV/TXT，不含图片）
│   ├── synthetic/            合成模拟三模块结果
│   └── real_field/           真实野外资料重构结果
├── third_party/eigen-x64/    第三方 Eigen 库
└── tools/                    一次性调试/补丁脚本（不参与构建）
```

> **整理原则**：图片只进 `figures/`；数据只进 `data/`；数值结果只进 `results/`；根目录只保留构建脚本、解决方案与说明文档。

### 5.2 `figures/`（唯一图片目录）

```text
figures/
├── 00_research_workflow_summary.png           全流程科研闭环综合大看板
├── 01_sa_convergence_curve.png                模拟退火收敛轨迹（单次代表轮次）
├── 02_sampling_benchmark_bars.png             三种采样方案相干度对比柱状图
├── 03_cmp_fold_distribution_map.png           CMP 面元覆盖次数热力图（真实 GCellGrid 矩阵）
├── 04_offset_azimuth_rose_charts.png          炮检距直方图与方位角玫瑰图
├── 05_wavefield_reconstruction_quad.png       波场重构四联剖面
├── 06_recon_convergence_trace_comparison.png  残差收敛与单道波形对比
├── index.html                                 图件画廊（浏览器直接打开）
├── real/                                      真实野外资料（RRAW.SGY）验证图
│   ├── 03_cmp_fold_distribution_map.png
│   ├── 04_offset_azimuth_rose_charts.png
│   ├── 05_wavefield_reconstruction_quad.png
│   └── 06_recon_convergence_trace_comparison.png
└── gui/                                       GUI 界面截图（--screenshot / --screenshot-all）
```

### 5.3 结果目录分区（合成 / 真实 SPS / 真实波形）

```text
results/
├── synthetic/                             合成数值模拟实验
│   ├── cmp_mask.txt                       全局统一重构采样掩码
│   ├── 01_sampling_benchmark/
│   │   ├── benchmark_runs_detail.csv      每轮重复试验完整明细（10 轮）
│   │   ├── benchmark_comparison_table.csv 30%/50%/70% 三方案均值、方差与极值
│   │   ├── benchmark_convergence_curves.csv 退火探索与历史最优收敛轨迹
│   │   └── benchmark_summary_report.txt   文本评估报告
│   └── 03_wavefield_reconstruction/
│       ├── cmp_mask.txt                   本次实际使用的掩码
│       ├── recon_original_gather.csv      全分辨率参考波场 (256x128)
│       ├── recon_subsampled_gather.csv    50% 欠采样观测波场
│       ├── recon_recovered_gather.csv     曲波 POCS 恢复波场
│       ├── recon_residual_slice.csv       重构残差切片
│       ├── recon_metrics_summary.csv      SNR/RMSE/相关系数/冷热启动消融
│       └── recon_experiment_report.txt    重构科研报告
│
├── real_sps/                              真实 SPS 观测系统几何评价
│   └── attribute_evaluation/
│       ├── attribute_fold_distribution.csv
│       ├── attribute_fold_map_matrix.csv  实测覆盖次数矩阵（热力图数据源）
│       ├── attribute_offset_distribution.csv
│       ├── attribute_azimuth_rose.csv
│       └── attribute_quality_report.txt   （报告内含 Real Survey Evaluation Mode）
│
├── real_field/                            真实野外资料重构（插值参考场口径）
│   ├── cmp_mask.txt
│   ├── recon_*.csv / recon_metrics_summary.csv
│   └── recon_experiment_report.txt
│
└── real_field_holdout/                    真实实测道留出验证
    ├── holdout_runs.csv                   ★ 6 组 keep×seed 汇总（机器可读，可复算）
    ├── holdout_metrics.csv                keep=30 / seed=20261020 单组明细
    └── cfg_<keep>_<seed>/                 各组独立输出目录
```

> **数据模式对照**：`synthetic/` 全部为合成数值模拟；`real_sps/` 为真实 SPS 几何（无波形）；`real_field/` 与 `real_field_holdout/` 为真实野外波形资料。前三者的报告首行均标明所属模式，不可混用指标。

---

## 六、科研复现核验基准数据

使用系统默认主随机种子（`Seed = 42`，重复 3 轮独立试验）生成的核心科研指标基准值如下：

### 1. 多采样率相干度对比基准表 (`benchmark_comparison_table.csv`)

| 采样率 | 采样点数 $m/N$ | 规则减抽 $\mu_{\text{reg}}$ | 纯随机均值 $\mu_{\text{rand}}$ | Jitter-SA 优化均值 $\mu_{\text{opt}}$ | SA 标准差 $\sigma$ | [极小值, 极大值] | 较规则平均改善率 | 相干度比值衰减 (dB) |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **30%** | 38 / 128 | 0.7892 | 0.4675 | **0.3460** | 0.0216 | [0.3175, 0.3938] | +56.16% | 7.18 dB |
| **50%** | 64 / 128 | 0.9601 | 0.3332 | **0.2238** | 0.0251 | [0.1771, 0.2596] | **+76.69%** | **12.71 dB** |
| **70%** | 90 / 128 | 0.2609 | 0.2138 | **0.1438** | 0.0087 | [0.1282, 0.1598] | +44.89% | 5.19 dB |
| **全率均值** | — | — | — | — | — | — | **+59.25%** | **8.36 dB** |

> **注（2026.10.20 统计口径）**：重复试验数由 3 轮提高到 **10 轮独立种子**（`--repeat 10`），表中均值为 10 轮聚合值，标准差与极值来自 `benchmark_runs_detail.csv` 的逐轮记录，可由该文件独立复算。3 轮数值已作废，不再作为对外指标。

> **注（2026.10.19 邻域幅度修正）**：原实现的 SA 邻域扰动固定交换 $m/20$ 对采样点（50% 时约 3 对/步），属"大邻域跳转"，退火轨迹更接近多起点随机重启。现改为标准 SA 的**单点交换（1 对/步）**，在保持采样点数严格守恒的前提下使 Metropolis 准则真正作用于局部邻域。修正后 50% 与 70% 的优化 μ 均值略升（0.2240→0.2353、0.1333→0.1367），说明在大邻域下取得的更低 μ 部分来自搜索广度而非退火机制本身；同时单点交换的轮间波动增大（σ: 0.0051→0.0216、0.0079→0.0064），属预期行为。**结论方向不变**：Jitter-SA 在三档采样率下均严格优于规则减抽与纯随机抽样（改善 54.21% / 75.49% / 47.61%）。可通过环境变量 `IRR_SA_NEIGHBOR_K` 复现邻域幅度敏感性分析。

### 2. 波场重构关键定量指标 (`recon_metrics_summary.csv`)

- **数据模式**：Synthetic Numerical Wavefield (3-Layer Reflection Model + Gaussian White Noise)
- **掩码来源**：External File (`cmp_mask.txt`，来自 50% Jitter-SA 优化掩码，严格 64 点)
- **零值填充初始基准 SNR**：$3.02\text{ dB}$（基准参照 0.00 dB）
- **空间线性插值基准 SNR**：$11.83\text{ dB}$（较零值填充提升 $+8.83\text{ dB}$）
- **曲波域压缩感知重构 SNR (Curvelet POCS CS)**：**$13.90\text{ dB}$**
- **相比零值填充净提升量**：**$+10.90\text{ dB}$**
- **相比空间线性插值净超越量**：**$+2.07\text{ dB}$**
- **全场均方根误差 (RMSE)**：**$0.032070$**
- **波形皮尔逊相关系数 ($r$)**：**$0.9794$**
- **观测道一致性检验**：**PASSED**（实测道最大偏差严格 $< 10^{-5}$，实测误差 $0.000000$）
- **算法迭代与耗时**：40 步迭代，耗时约 $1550\text{ ms}$

### 3. 真实 SPS 观测系统空间属性评价指标 (`attribute_quality_report.txt`)

- **评估模式**：Real Survey Evaluation Mode（基于工区自带真实野外 SPS 样本 `data/sps/5151-5171`）
- **激发炮点总数 / 接收点总数**：349 炮 / 9,072 接收点
- **有效分析道对数**：3,162,602 道（异常/过滤道数：0，零偏移距道数：0）
- **空间包围盒真实 UTM 坐标**：
  - 炮点范围：$X \in [679061.6, 682530.7]\text{ m}$, $Y \in [4146694.0, 4147203.5]\text{ m}$
  - 检波点范围：$X \in [679735.9, 681891.0]\text{ m}$, $Y \in [4138298.0, 4150889.8]\text{ m}$
- **有效 CMP 面元网格统计**：
  - 有效非空面元数（Live CMP Bins）：**29,719 个面元**
  - 平均覆盖次数（Mean Fold）：**106.42 次**
  - 覆盖次数极值范围（Fold Range）：**1 ~ 198 次**
  - 覆盖次数标准差（Fold StdDev）：**58.61**
  - 覆盖次数变异系数（Coefficient of Variation）：**$C_v = 0.5508$**
  - 覆盖保留评分（Fold Retention Score）：**74.9%**
- **炮检距空间连续性分布**：
  - 炮检距极值范围（Offset Range）：**1.7 m ~ 9,324.0 m**
  - 平均炮检距（Mean Offset）：**3,875.9 m**
  - 近炮检距占比（$< 1/3 \text{ Max}$）：**45.3%**
  - 中炮检距占比（$1/3 \sim 2/3 \text{ Max}$）：**34.5%**
  - 远炮检距占比（$> 2/3 \text{ Max}$）：**20.3%**
- **方位角覆盖完整度评分**：**100.0%**（24 / 24 扇区全方位均匀照射，各向异性地下成像均匀度优级）
- **野外施工成本优化**：全覆盖基准测线，未施加抽稀（成本节约率 0.0%）

---

## 七、可复现性与工程规范清单（2026.10.19 整改记录）

本轮整改聚焦"结论可复现、对比可辩护、报告不虚标"，全程不更改科研结论方向，仅修正口径与表述。

| # | 问题 | 整改内容 | 验证方式 |
| :---: | :--- | :--- | :--- |
| 1 | **SPS 样本路径硬编码** `d:/Desktop/work/...`，换目录运行 Test 9 必失败（实测 9/10） | 新增 `findRepositorySpsDir()`：从可执行文件目录向上回溯 + 当前工作目录 + `IRR_SPS_DIR` 环境变量三路解析 | 在 `%TEMP%` 与项目根目录分别运行 `--test`，均 **11/11 PASS**（后增至 12/12） |
| 2 | 报告中硬编码 "14.10 dB / 1.75 dB / 12.64 dB / 0.9804" 等结果字符串 | 全部改为 `QString::number()` 按实测值动态填充；`atten_db_50` 去掉 12.64 兜底默认值 | 改变 seed/repeat 后报告文字随数值同步变化 |
| 3 | 看板图模块二混用**合成模式 576 面元 / 节约 50%** 与真实工区指标 | 看板改为**全参数化 + 模式标签**（〔真实SPS工区〕/〔合成基准〕），底部结论明确区分两类数据 | 导出看板与 `attribute_quality_report.txt`、CSV 逐项核对一致 |
| 4 | 看板模块一取**单轮**记录（11.36 dB），与汇总表 3 轮均值（12.25 dB）矛盾 | 改为多轮聚合均值，与 `benchmark_comparison_table.csv` 严格同口径 | 看板 12.25 dB / 75.49% 与 CSV 一致 |
| 5 | **热启动使 SNR 虚高**：14.10 dB 中 8.20 dB 来自线性插值初值，未与标准 POCS 公平对比 | 新增**冷/热启动消融**：冷启动 5.90 dB（+2.89 dB）、热启动 14.10 dB（+1.75 dB），并写入 CSV 与 Test 11 固化 | `SNR_POCS_ColdStart` / `SNR_POCS_WarmStart` / `SNR_WarmGain_over_Cold` |
| 6 | SA 邻域扰动 $m/20$ 属大邻域跳转，非标准退火 | 改为单点交换（1 对/步），可用 `IRR_SA_NEIGHBOR_K` 做敏感性分析 | `benchmark_comparison_table.csv` 三档结论方向不变 |
| 7 | 源码混用 LF/无 BOM，MSVC9 按 GBK 解析导致"newline in constant"编译失败 | 全部改动文件统一 **UTF-8 with BOM + CRLF**（符合 README 编码规范） | `build_release.bat` 0 error |
| 8 | `scratch/`、`patch_engine.py` 等一次性脚本混入主干 | 统一迁入 `tools/` 与 `tools/scratch/`，不参与构建 | vcproj 无引用 |
| 9 | **`test_csv_real` 名不副实**：其输入 `seismic_valid_256x128.csv` 实为 `sin(0.05i+0.1j)` 合成正弦光栅，并非野外记录 | 目录更名为 `test_csv_external`；CSV 首行标注 "Synthetic 256x128 CSV fixture (sine grating) for CSV-parser validation only"；`Mode_Name` 由 `External Seismic Record` 中性化为 `External CSV File` | 报告首行与目录名一致，不再暗示真实资料 |
| 10 | **曲波多尺度 μ 计算存在实质缺陷**：`CurveletMiu1DEigenParam` 恒取 `C[0][0]`（最粗尺度第一块）。`nbscales=1` 时该块即全矩阵（正确），但 `nbscales>=2` 时各块形状不一（实测 2 尺度 8 方向为 `85×85` 低频块 + 8 个 `75×171/171×75` 楔形块），据此算出的 μ **超过 1**（实测 1.457/1.762），而归一化 Gram 相干度理论上界为 1 —— 属明确错误 | 改为：多块时将全部系数经 `ifdct_wrapping` 重建回空间域感知矩阵，再对其求归一化 Gram 相干度；参考项同样经逆变换构建 | 修复后 7 组参数下 μ 全部落在 [0,1] 且 **Jitter-SA < 纯随机 < 规则减抽 排序 7/7 稳定**；`nbscales=1` 主流程数值逐位不变 |
| 11 | 曲波变换参数缺少敏感性证据，存在"结论依赖单一固定参数"的质疑空间 | 新增 **Test 12 + `mu_param_robustness.csv`**：在 (1,4)(2,8)(2,16)(2,32)(3,12)(3,16)(4,16) 七组参数下检验三方案 μ 排序稳定性 | Test 12 通过；见下表 |
| 12 | **OpenMP 数据竞争导致结果不可复现**：`gcellgrid.cpp` 的 `#pragma omp parallel for` 内对共享 `mFold[j][i]++` 及 `mMaxoffset/mInMaxoffset/mCLMaxoffset` 无保护写入。实测同一 seed 连续运行，`Mean Fold` 在 **106.41 / 106.42** 之间漂移，`attribute_fold_distribution.csv` 在 fold=197 档出现 0/1/2 面元的随机变化 | 对 `mFold` 累加改用 `#pragma omp atomic`；三处极值更新改用 `#pragma omp critical`；删除无意义的 `if(mFold[j][i]>10) int dddd=0;` 竞态读回 | 修复后连续 4 次运行，`attribute_fold_distribution.csv` 的 MD5 **完全一致**，`Mean Fold` 恒为 106.42 |
| 13 | **自动抖动掩码未播种**：`runWavefieldReconstruction` 在未提供掩码文件时直接调用 `Sampling1J_Count`，无显式 `exprng::seed`，导致同一输入两次运行 SNR 从 9.25 漂到 10.23 dB | 掩码生成前固定 `exprng::seed(20261020ULL)` | 3 次运行 `cmp_mask.txt` MD5 完全一致，全部科学指标逐位一致（仅 `Runtime_ms` 不同） |
| 14 | 报告增益为负时输出 `+-1.02 dB`（硬编码 `"+"` 前缀） | 改为带符号格式化 | 真实资料报告现正确显示 `-0.62 dB vs Linear` |
| 15 | **`--run-real-reconstruction` 从未传递掩码**：调用处硬编码 `""`，导致 `--mask` 无效、三份不同掩码得到完全相同结果（11.83/13.90）。实验实际不可控 | 新增 `--mask <file>` 参数并正确传入；实测三份掩码现分别得到 11.34 / 12.55 / 12.37 dB | 掩码变化确实改变结果，`Mask_Source` 显示 External File |
| 16 | **报告结论无条件断言"显著超过"**：即使真实资料上 POCS 低于线性插值（−0.62 dB），正文仍写 "significantly surpassing ... linear interpolation"，且格式为 `+-0.62 dB` | 新增 `fmtSigned()` / `compareGain()`；结论按实测差值分支输出 OUTPERFORMS / UNDERPERFORMS / COMPARABLE | Test 13 覆盖正增益/负增益/近零三种情况；全部输出已无 `+-` |
| 17 | **README 指标与仓库不符**：README 写 12.35/14.10/1.75/5.90，仓库为 11.83/13.90/2.07/6.24；两组并非同一配置 | 查明差异源于采样掩码不同（非算法变化）；新增 4.5 节"实验基准与数据版本对照"作为指标唯一来源，并保留历史版本对照 | README 全部数值与 `results/synthetic` 一致 |
| 18 | **SEG-Y 预处理不可复现**：无转换脚本，且二进制头为 **little-endian**（非 SEG-Y 标准大端）、format code 声明与实际 payload 不符 | 新增 `tools/convert_rraw_to_engine_csv.py`：显式记录参数、自带 `--check-encoding` 判定、输出前自检 256×128 | 重新生成 CSV 与原文件**逐字节一致** |
| 19 | **真实资料验证用插值参考场自证**：SNR 相对 59→128 插值构造的参考场计算，不能代表真实波场精度 | 新增 `--holdout-validate`：只用真实实测道作验证目标（`real_gather_59x256.csv`），SNR 仅在留出道统计 | 6 组 keep×seed 配置下 POCS 均优于线性（+0.91~+1.30 dB）；Test 14 固化 |
| 20 | **真实 SPS 属性报告放在 `results/synthetic/`**，与合成结果混放易误读 | 目录更名 `02_attribute_real_sps`，代码/测试引用同步 | 目录名与报告内 `Real Survey Evaluation Mode` 一致 |
| 21 | **采样基准仅 3 轮重复**，不足以支撑统计结论 | 默认 `--repeat` 由 3 提高到 10，逐轮原始数据保留在 `benchmark_runs_detail.csv` | 统计量可由该文件独立复算；3 轮数值作废 |
| 22 | **测试"跳过即通过"**：Test 13/14 在报告或输入缺失时只记 SKIP，`t*_ok` 仍为 true，实测在无 `results/` 目录时仍输出 `14/14 ALL PASS` | 新增 `skippedTests` 计数与前置条件检查；SKIP 不计入通过且使整体失败；Test 13 改为在沙箱内自生成正/负增益报告，不读遗留文件 | 删除必需输入后实测退出码 1、`Status: FAILED`；正常时 `PASS=15 FAIL=0 SKIP=0` |
| 23 | **Test 14 仅验单组、无留出泄漏断言** | 扩展为 6 组 cfg 逐一执行 + 换种子必改变划分 + gain 自洽校验 | Test 14 通过并覆盖 6 组 |
| 24 | **无掩码生效的自动回归** | 新增 Test 15：两份不同掩码实测产生不同基线（Δ11.74 dB）+ 四类非法掩码必须被拒 | Test 15 通过 |
| 25 | **6 组留出结果未归档为机器可读记录** | 新增 `runHoldoutBatch()` 与 `--holdout-batch`，程序自写 `holdout_runs.csv`（含输入 SHA1、道数、种子、三 SNR、双增益、双 RMSE、耗时） | 6 行实测数据；gain 可由 SNR 列独立复算；两次运行科学列完全一致 |
| 26 | **真实 SPS 报告仍在 `synthetic/` 下** | 输出改到 `results/real_sps/attribute_evaluation/`，`synthetic/` 仅剩 01 与 03 | 目录树与报告模式一致；全套实验重跑成功 |

### 曲波变换参数鲁棒性结果（`mu_param_robustness.csv`）

| (nbscales, nbangles) | μ 规则减抽 | μ 纯随机 | μ Jitter-SA | 排序稳定 | 衰减 (dB) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **(1, 4) 主流程基线** | 0.9601 | 0.3508 | **0.2219** | ✅ | 12.72 |
| (2, 8) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |
| (2, 16) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |
| (2, 32) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |
| (3, 12) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |
| (3, 16) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |
| (4, 16) | 1.0000 | 0.2850 | **0.2366** | ✅ | 12.52 |

> **解读**：在全部 7 组参数下，"Jitter-SA 相干度最低、规则减抽最高"的排序完全一致，说明核心结论**不依赖特定变换参数选择**，可作为方法鲁棒性证据。
> 多尺度下 μ_规则 = 1.0000，是规则减抽（周期性采样）在任一完备变换基下都达到相干度理论上界的自然结果。
> 亦可见多尺度下衰减约 12.52 dB，与单尺度基线 12.72 dB 高度接近，进一步支持结论稳健。
> 说明：`nbscales==1` 时曲波仅有一个低频块，`nbangles` 不生效，故未重复采集 (1,8)。

## 八、真实野外资料重构验证（2026-10-20 新增）

### 8.1 数据来源

`data/real_seismic/Data/RRAW.SGY` —— 真实野外**共炮点道集**（训练数据集 Group A）。

| 属性 | 实测值 |
| :--- | :--- |
| 文件大小 | 76,760 字节 |
| 道数 | **59**（off-end 排列） |
| 采样点数 | 250 |
| 采样间隔 | 8 ms（记录长度 2.0 s） |
| 道间距 | 约 12.3 m |
| 偏移距 | 52 ~ 1,560 m（首道距炮点 33 m，末道 747 m） |
| 振幅范围 | -7,155 ~ +7,011 |
| 主频 | 约 21 Hz（10–40 Hz 占能量 61.9%） |

> **数据格式注意**：SEG-Y 二进制头声明的格式码为 `1`（IBM 浮点），但实测数据实为 **IEEE 754 小端浮点**。按 IBM 解码会得到 10²⁵ 量级的荒谬值。这是该数据集的已知异常，解析时以实际字节为准。

### 8.2 预处理与格式适配

C++ 引擎要求 **256 × 128**（Nt=256, dt=4 ms；Nx=128, dx=12.5 m）的纯数值 CSV。转换步骤：

1. 按带符号偏移距排序道序，使道集在空间上单调；
2. 时间轴 8 ms → 4 ms 重采样（250 → 256 样点，0–1.020 s）；
3. 道数 59 → 128 线性插值；
4. 8–70 Hz 带通（压制低频地滚波与高频噪声）；
5. 峰值归一化。

产物：`data/real_seismic/real_gather_256x128.csv`

### 8.3 重构结果（`results/real_field/`）

| 指标 | 合成数值模拟（前文） | **真实野外资料** |
| :--- | :---: | :---: |
| 零值填充 SNR | 3.02 dB | **3.04 dB** |
| 线性插值 SNR | 11.83 dB | **9.85 dB** |
| 曲波 POCS（热启动，主口径） | 13.90 dB | **9.23 dB** |
| 相对零填充提升 | +10.90 dB | **+6.19 dB** |
| **相对线性插值** | **+2.07 dB** | **−0.62 dB** |
| 冷启动 POCS | 6.24 dB | **6.85 dB** |
| POCS 迭代净贡献（冷启动） | +2.89 dB | **+3.81 dB** |
| RMSE | 0.032070 | **0.036707** |
| 皮尔逊相关系数 r | 0.9794 | **0.9384** |
| 观测道一致性 | PASSED | **PASSED** |

### 8.4 关键结论（必须如实写入论文）

**在真实野外资料上，曲波 POCS 未能超越线性插值基线（−0.62 dB），与合成数据上的结论（+2.07 dB）相反。**

这不是程序缺陷，而是可信的科研结果。可能原因：

1. **真实道集的空间相干性远低于合成模型**。合成数据是三层水平层状模型 + 高斯白噪声，同相轴高度规律；真实资料含地滚波残余、初至、非均质散射与随机噪声，空间预测性弱，线性插值已能捕获大部分可预测成分。
2. **道集几何为 off-end 排列**（非对称、单侧），并非规则对称孔径。59 道插值到 128 道相当于 2.17 倍上采样，超出曲波稀疏先验的有效作用范围。
3. **冷启动 POCS 仍有 +3.81 dB 净贡献**，说明曲波稀疏约束本身有效；但热启动路径下 POCS 反而**降低**了 0.62 dB，即迭代在插值初值上做了负向修正。

**因此**：论文中引用 13.90 dB 必须限定为"合成数值模拟条件"，并同时给出真实资料对照值 9.23 dB；不可将合成结论外推为真实工区结论。

### 8.5 留出验证（Hold-out on measured traces）—— 2026-10-20 新增

8.3 的 SNR 是在**插值构造的 128 道参考场**上计算的，不能代表"相对于真实完整地下波场的重构精度"。为消除该局限，新增留出验证：**只用真实实测道作为验证目标**。

**方法**（`runHoldoutValidation`）：
1. 使用 `real_gather_59x256.csv`（原始 59 道，仅做 8→4 ms 时间重采样，**不做道数插值**）；
2. 用给定种子随机选 k 道作为"已知"，其余 m−k 道隐藏；
3. 零填充 / 线性插值 / 曲波 POCS 三者在同一组留出道上比较；
4. **SNR 只在留出道集合上统计**，参考值为真实实测振幅。

**结果**（`results/real_field_holdout/holdout_metrics.csv`，m=59, k=30, 留出 29 道）：

| 方法 | 留出道 SNR |
| :--- | ---: |
| 零填充 | 0.00 dB（定义基准） |
| 线性插值 | −1.67 dB |
| **曲波 POCS** | **−0.41 dB** |
| POCS 相对线性 | **+1.25 dB** |
| POCS 相对零填充 | −0.41 dB |

**敏感性**（6 组配置，keep×seed）：

| keep/59 | seed | 线性 | POCS | POCS−线性 |
| :---: | :---: | ---: | ---: | ---: |
| 20 | 20261020 | −1.64 | −0.54 | +1.10 |
| 20 | 12345 | −2.42 | −1.12 | +1.30 |
| 30 | 20261020 | −1.67 | −0.41 | +1.25 |
| 30 | 12345 | −1.68 | −0.57 | +1.11 |
| 40 | 20261020 | −1.43 | −0.25 | +1.18 |
| 40 | 12345 | −1.18 | −0.27 | +0.91 |

> **结论**：在全部 6 组配置下，**曲波 POCS 均优于线性插值（+0.91 ~ +1.30 dB）**，方向一致、量级稳定。这是本项目关于"曲波 POCS 优于插值"最可信的一条真实数据证据。
>
> 但 POCS 与线性插值的绝对 SNR 均为**负值**，说明在只有 30/59 真实道作为已知的条件下，两种方法的填补误差都大于信号本身。负 SNR 是真实结果，不做修饰。

### 8.6 复现命令

```bat
:: 单组留出验证
x64\Release\IrrGeometryDesign.exe --holdout-validate data\real_seismic\real_gather_59x256.csv ^
  --traces 59 --keep 30 --iter 40 --seed 20261020 --out results\real_field_holdout

:: 6 组批量留出验证（生成 holdout_runs.csv，机器可读可复算）
x64\Release\IrrGeometryDesign.exe --holdout-batch data\real_seismic\real_gather_59x256.csv ^
  --traces 59 --iter 40 --out results\real_field_holdout

:: 真实资料重构（插值参考场口径）
x64\Release\IrrGeometryDesign.exe --run-real-reconstruction ^
  data\real_seismic\real_gather_256x128.csv --out results\real_field --ratio 0.50
```

> `holdout_runs.csv` 每行一组 keep×seed 配置，包含输入 SHA1、道数、迭代、种子、
> 三种方法 SNR、两个增益、两个 RMSE 与实际耗时。所有数值由程序实际执行产生，
> 可用上表命令逐行复算。

#### 8.5.1 原命令说明

```bat
x64\Release\IrrGeometryDesign.exe --run-real-reconstruction ^
  data\real_seismic\real_gather_256x128.csv ^
  --out results\real_field --ratio 0.50
```

产物：`recon_metrics_summary.csv`、`recon_experiment_report.txt`、`05_wavefield_reconstruction_quad.png`、`06_recon_convergence_trace_comparison.png`（全部由 Qt 渲染器生成）。

---

### 尚未解决 / 后续建议

- **真实资料仅为单炮**：`RRAW.SGY` 是单个共炮点道集，无法做速度分析、NMO、叠加与 CMP 面元属性评估。若需完整闭环，建议补充同工区多炮数据以构建 CMP 道集。
- **道数上采样倍数偏高**：59 → 128（2.17×）。建议补充"59 道原孔径、50% 抽稀"的对照，避免上采样本身主导指标。

