# 基于混合优化算法的地震采样点智能筛选方法研究

本项目用于硕士论文的采样点筛选、几何评价与曲波重构实验。C++ 桌面工程保持 VS2008 SP1 x64、Qt 4.7.4 和仓库内 FFTW/FDCT 环境兼容；Python 负责资料转换、批处理、统计和独立核验。

当前新增研究协议为 **`adaptive_hybrid_v4`**，正式输出入口为 `results/review_20261024_v4/final/`。此前正式协议 **`physical_psf_v3`** 的原始结果继续保存在 `results/enhanced_20261023/`，`classic_psf_v2` 保存在 `results/revised_20261023/`。三个版本分别归档，不能混合其中不同目标、模型或评价流程的数字。

旧 GUI 和原有 CLI 的 Hybrid 仍保留固定 SA 前缀，保证历史入口可用。新增自适应算法通过专用研究入口调用；新版图件通过本页链接打开。

<!-- add by wushanzhou in 2026/10/24 -->

## 方法和代码入口

|模块|实现|说明|
|---|---|---|
|定点数采样|`IrrGeometryDesign/Sampling.cpp`|Regular、Random、Jitter，精确保留 `round(Nc)` 个位置。|
|指标与历史诊断|`IrrGeometryDesign/CurveletMiu.cpp`|正式 Fourier/PSF 目标；另保留历史曲波 Gram 诊断，两者定义不同。|
|搜索与空间约束|`IrrGeometryDesign/OptimizeMiu.cpp/.h`|原 SA、Local、Hybrid、Restart；物理坐标软惩罚与最大间隔硬约束。|
|自适应混合搜索|`IrrGeometryDesign/adaptivesearch.inc`|停滞触发切换、混合交换邻域、局部停滞后的预算回收与扰动。|
|曲波重构|`geometryexperimentengine.cpp`、`coordinateholdout.inc`、`reconstructionoptions.h`|实际坐标观测投影、已知道幅度标定、可配置尺度／方向／阈值／迭代和初始化。|
|新实验接口|`IrrGeometryDesign/reviewexperiments.inc`|搜索消融和可配置重构；输出明确标注 v4。|
|研究与独立复核|`tools/review_studies.py`、`verify_review_results.py`|固定配置批次、缓存日志、逐输出哈希、独立指标和内层选择复算。|
|汇总与失败诊断|`summarize_review.py`、`review_diagnostics.py`|统计、共同实际评价次数比较、几何—质量关联、真实误差分解和图件。|

新混合方法与旧方法共享固定点数和目标定义，但不预设它必须胜过 SA-only。正式比较同时报告评价上限、实际评价次数和运行时间。`SA_Budget` 使用与新方法相同的预算温度日程和缓存后端，以区分温度／计算后端改变与混合机制的作用。

Local 是坐标贪心搜索，历史 CG 命名不代表标准共轭梯度。重构为非线性曲波收缩与观测投影的 POCS 式迭代，不宣称经典凸 POCS 的收敛保证。

<!-- add by wushanzhou in 2026/10/24 -->

## 构建与回归

在仓库根目录运行：

```bat
build_release.bat
build_debug.bat
x64\Release\IrrGeometryDesign.exe --test
x64\Release\IrrGeometryDesign.exe --review-tests
x64\Debug\IrrGeometryDesign.exe --review-tests
python tools\test_science_tools.py
python tools\test_review_science.py
```

构建脚本支持 `QTDIR`、`VCVAUTO`、`VCARCH`，并保留自动探测。Python 科学工具使用 3.12；基础依赖见 `requirements.txt`，图件额外依赖见 `requirements-research.txt`。

完整 C++ 回归包括原有测试与本轮边界、物理硬约束、FDCT／DFT 数值检查。`--review-tests` 只执行本轮新增专项，便于 Debug 下快速复核。历史编译器告警尚未全部清理。

<!-- add by wushanzhou in 2026/10/24 -->

## 新实验复现

```bat
python tools\review_studies.py --out results\review_20261024_v4\final --workers 6
python tools\verify_review_results.py results\review_20261024_v4\final --check-archives
python tools\summarize_review.py results\review_20261024_v4\final
```

已归档的正式目录不能用作新一轮覆盖运行；重新实验应将 `--out` 改为未使用的版本目录。全记录辅助研究命令为 `python tools\review_window_study.py --out results\review_20261024_v4\my_full_window`，仍只使用同一真实来源。

复用已完成步骤必须添加 `--resume`；程序、源码、脚本、配置和输入须匹配，归档输出也要通过哈希检查。程序在批次运行期间变化会明确拒绝继续。失败或版本不一致的输出应保留，并改用新目录；不能覆盖历史实验以制造通过记录。

小规模检查使用独立目录：

```bat
python tools\review_studies.py --quick --out results\review_20261024_v4\my_smoke
python tools\verify_review_results.py results\review_20261024_v4\my_smoke
```

新批次配置：

|实验|配置|
|---|---|
|搜索与消融|均匀候选／实际 offset，200／600 评价上限，3 档，20 种子；200 档另做分离权重与硬约束研究。|
|合成重构|3 类反射事件模型、6 个独立参数／噪声实现；主要对照各 5 个掩码种子，权重扩展各 3 个。|
|真实选点—重构|同一 59 道资料，保留 20／30／40 道，保留完整基线与消融。|
|真实参数选择|外层固定测试道；参数与预测幅度系数仅使用保留道内部验证，30 个外层划分。|
|CDP 诊断|5 个子集的固定划分分析，不增加独立来源。|

计算图件保存 PNG 和 SVG，入口为 [新版图件](results/review_20261024_v4/final/figures/index.html)。首轮开发和小规模批次不作为正式论文结果。

<!-- add by wushanzhou in 2026/10/24 -->

## 数据与科学结论边界

真实波形只有 `data/real_seismic/Data/RRAW.SGY` 一份：59 道、250 样点，二进制头和各道头均给出 8000 μs；包含五个 CDP。文件声明的浮点格式与既有 IEEE-LE 解释冲突，读取器明确使用当前资料专用配置；不能仅以振幅合理性证明一种有争议的编码。原“二进制 dt=64”的注释错误已纠正。

`real_gather_59x256.csv` 保留真实空间位置，只做时间重采样、裁窗和固定幅度单位转换；用于实测道留出。`real_gather_256x128.csv` 含空间插值，是参考场。SPS 几何来自另一份资料，未建立同工区波形对应。不同时间窗、CDP、随机划分或合成场景不增加真实波形来源数。

必须分别讨论：**降低互干度、改善空间覆盖、提高缺失道重构 SNR**。零填充的缺失道 SNR 为 0 dB；负值原样保留。内层模型选择允许选择零预测，这表示当前恢复模型的局限，不能写成曲波恢复成功。

物理硬约束当前限定为一维候选位置的最大间隔（含边界）。尚未实现自由连续布点、施工约束、二维实测坐标硬约束、NMO／叠加或成像质量验证。新合成资料是解析反射事件叠加，不是复杂地下模型的完整波动方程正演。

<!-- add by wushanzhou in 2026/10/24 -->

## 论文与审查材料

本次实际验收：Release 完整 C++ 回归 33/33，新增专项在 Debug 下 9/9，Python 24 项通过；正式批次 5,843 行及 16,716 个输出文件哈希通过独立核验。15,393 个历史结果文件及原始 SEG-Y 保持不变，详见 [机器验收记录](results/review_20261024_v4/validation_summary.json)。

- [方法、实验与讨论](docs/THESIS_METHODS.md)
- [审查计划与优先级](docs/REVIEW_PLAN_20261024.md)
- [本轮审查及实际结果](docs/REVIEW_REPORT_20261024.md)
- [当前进展](docs/PROJECT_PROGRESS_20261024.md)
- [新批次原始汇总](results/review_20261024_v4/final/RESULTS.md)
- [此前 v3 结果](results/enhanced_20261023/RESULTS.md)

代码修改按实际日期添加 `add by wushanzhou in 2026/10/24`。数值 CSV、严格 JSON 和二进制图件不插入源码式注释。

<!-- add by wushanzhou in 2026/10/24 -->
