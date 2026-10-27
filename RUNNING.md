# 构建、当前资料检查与实验复现

命令以 `IrrGeometryDesign` 为项目根目录。`pipeline.py` 支持从其他目录调用，研究模块的相对路径仍相对于项目根目录。每次检查或实验使用新输出目录，避免覆盖冻结证据。

## 1. 构建与检查范围

本地原生工具链为 Windows x64、VS2008 SP1、Qt 4.7.4；Python 为 3.12。构建配置位于 `src/build/`。

```powershell
python -m pip install -r requirements.txt
.\build.bat Release
.\build.bat Debug
python -B pipeline.py test
python -B pipeline.py check --out checks/new_core_check --build
python -B pipeline.py check --full --out checks/new_project_check
```

| 范围 | 入口 | 检查内容 |
| --- | --- | --- |
| 核心实现 | `check` | Python、Release/Debug 原生核心、独立数值、物理观测投影与合成端到端 |
| 当前项目 | `check --full` | 核心实现、登记 Viking 输入与观测/隐藏资料集成、SPS 工程集成 |
| 历史资料 | `check --legacy` | 显式检查旧 RRAW 输入、派生 CSV、来源声明及旧集成；缺项保留 incomplete |
| Python CI | `check --python-only` | 明确排除原生工具链及资料集成的 Python 检查 |

检查清单记录范围、命令、程序与源码身份、通过/失败以及每个跳过原因。当前项目范围只允许明确登记的旧 RRAW 依赖测试跳过；登记 Viking 或 SPS 资料缺失、其他意外跳过或执行失败不能标为当前项目通过。检查不等同于科学效果验证。

原生 `--test-core` 不依赖外部资料。旧原生 `--test` 包含历史工程测试，其范围不能与当前 `check --full` 混淆。单构建检查可加 `--mode Release` 或 `--mode Debug`。可单独执行底层检查：

```powershell
python -B pipeline.py module research.validation.run_legacy_codec_checks --out checks/new_codec
python -B pipeline.py module research.validation.run_fdct_probe --out checks/new_fdct
python -B pipeline.py module research.validation.registered_data_check --out checks/new_viking_integration
```

## 2. 当前真实资料：Viking 登记子集

默认数据目录为 `data/public/viking_line12_small/`。`src/research/data/viking_line12_subset.json` 固定 30 个数据文件与 2 份来源文档的身份。验证器独立解码大端 IBM SEG-Y，核对炮号、逐道采样间隔、原始样点、几何和裁窗 CSV，不能用目录内自行改写的哈希替代正式登记。

```powershell
python -B pipeline.py viking --validate-only --out checks/new_viking_inputs
```

数据包含 10 炮，各有 120 道；主研究窗口为 256 个原生 4 ms 样点。原始坐标单位码存在历史不一致，当前距离尺度依据公开 25 m 道间距、坐标增量和二进制头 measurement system 解释；绝对振幅物理标定未建立。本地小子集核验不等于核验完整大文件。评价对象为含噪实测道的隐藏预测。

## 3. Viking v2：当前方法对照

```powershell
python -B pipeline.py viking-v2 --out results/new_viking_v2 --workers 8
```

默认冻结计划为 `src/research/experiments/viking_v2_protocol.json`，运行时保存 `frozen_plan.json` 和哈希。首轮默认设置：FFID 53/453/953；窗口起始样点 256/512/768，对应 [1.024,2.048)、[2.048,3.072)、[3.072,4.096) 秒；一个周期 H 与两个分层随机 H，每组 15 道；保留 36/60/84 道；一个掩码种子；搜索上限 300。完整网格为 405 个恢复案例、117 个唯一搜索。

全部方法共享覆盖可行初值和 12 个先导位置 P，先导道计入保留数且始终保留在内层训练集。P 与外层 H 不交。比较方法如下：

| 方法 | 目标与约束 |
| --- | --- |
| Jitter | 共享初值，只记录一次评分 |
| SA_Mu | 纯 SA，全频 Fourier 互干度，完整评价预算 |
| Adaptive_Mu | 自适应混合，全频互干度 |
| Adaptive_Gap | 自适应混合，全频互干度；最大间隔 175/100/75 m |
| Adaptive_Signal | 同一间隔约束；用先导信号估计谱加权 PSF |

信号目标只用 `data[:, P]` 的 5–60 Hz 频带特征，保存派生权重，同时报告全频 μ。它是先导观测辅助的两阶段设计，不是无观测信号设计，也不是实际曲波算子的互干度。

恢复候选包含零预测、线性插值、两档正则化 f-k/FISTA、暖启动曲波 40 次、冷启动曲波 40 次、强阈值冷启动曲波 40 次和冷启动曲波 80 次。候选与 [0,1] 幅度系数只用观测道内层验证选择，再在全部 K 上重拟合。固定冷启动曲波、固定弱正则 f-k、线性与零预测独立报告。FISTA 在原生网格求解 `0.5||P_K x-y_K||² + λ||FFT2_ortho(x)||_1`，保存优化目标轨迹，再在输出中恢复已知观测。

单条件快速运行和双内层划分示例：

```powershell
python -B pipeline.py viking-v2 --out results/new_viking_pilot --shots 53 --windows 512 --counts 60 --hidden-sets r271828 --budget 80 --inner-folds 2
```

本轮增加三个掩码种子与双内层划分的独立稳健性批次，复现命令为：

```powershell
python -B pipeline.py viking-v2 --out results/new_viking_robust --seeds 84011 94217 105953 --inner-folds 2 --workers 16
```

完整网格计划为 1215 个恢复案例和 351 个唯一搜索；实际完成状态、调用数和结果见 [当前结果总览](README.md)。它仍是同一工区的开发稳健性检查，不能升级为独立工区验证。

正式运行前冻结程序、源码、输入、协议和候选池；运行期间不重建程序、不改参数。批次实际进行非先导振幅干预与外层 H 振幅干预，核对权重、掩码、模型选择及预测隔离。

输出含 `recovery.csv`、`search.csv`、掩码、预测、逐次调用、特征、选择记录和 manifest。分析先平均种子，再聚合每炮的窗口/H，最后按炮汇总，保留逐条件配对差。它们仍属于同一工区，不能按独立工区做显著性检验。相同目标评价次数不等于相同总计算量或墙钟时间。

重新分析必须使用新的目录与图件分组：

```powershell
python -B pipeline.py module research.analysis.viking_v2_results --root results/new_viking_v2 --out results/new_viking_v2_analysis --group research/new_viking_v2_analysis
```

首轮结果与局限见 [当前结果与材料总览](README.md)。新增批次不回写首轮清单。

通用批次复核器可使用对应的冻结源码 ZIP，避免把旧批次与后续当前源码误当作同一实现：

```powershell
python -B pipeline.py module research.validation.verify_viking_batch --root results/viking_robust_20261026 --out checks/new_robust_archive_audit --source-archive results/viking_robust_support_20261026/implementation_source.zip
```

复核首轮 v2 时改用该批次的结果目录与冻结源码包。旧的一次性核验脚本已随重复检查目录清理，当前统一使用上述复核器及匹配的冻结源码。

## 4. 合成研究与二维 SPS

合成研究使用同一个冻结程序，先生成原始结果再分析：

```powershell
python -B pipeline.py module research.experiments.review_studies --out results/new_synthetic/search --stage search --synthetic-only --workers 4
python -B pipeline.py module research.experiments.audit_joint_studies --out results/new_synthetic/joint --stage joint --workers 4
python -B pipeline.py module research.analysis.synthetic_results --v4-root results/new_synthetic/search --joint-root results/new_synthetic/joint --out results/new_synthetic/analysis --group research/new_synthetic
```

合成分析以 6 个干净结构模板为统计单位，先聚合噪声和掩码重复。保留负 SNR；不把重复当作独立地质模型，不预设某种搜索或恢复器必须胜出。

GUI 逐线模板与实际 SPS 二维目标分别实现。SPS 使用 `SpatialFourier2D_v1`、实际 `(x,y)`、有限二维字典和增量交换评价。已有 9072 检波点的真实几何集成及显式 Gram 复核；这不能替代二维真实波形或成像质量评价。历史记录见 [实现与验证总览](README.md)。

## 5. 可编辑稿件与材料归档

英文和中文主源文件、构建器及校验器统一放在项目内 `manuscript_sources/`；项目相邻的 `paper-tgrs/`、`thesis/` 为兼容输出目录。编辑项目内主源，再运行原构建入口。图件必须由已保存数值生成，不能从 PNG 反造结果表。运行 `python -B pipeline.py figures` 只刷新图库，不补做实验。

从sci工作区执行：

```powershell
python -m pip install -r paper-tgrs/requirements.txt
python -B paper-tgrs/build_manuscripts.py
```

Markdown为主编辑源，表格由冻结CSV生成并核对分析清单。阅读PDF使用ReportLab及本地Windows字体；TeX/BibTeX可编辑导出，但本环境未执行XeLaTeX编译。中文稿是完整六章研究初稿，尚未套用学校模板。

大体积数据、程序与 Viking 结果不全部受 Git 管理。交付时需要同时准备正式登记数据、冻结程序/源码、协议、原始结果及清单依赖，再验证哈希。新增可复现包工具的准确命令和本轮交付记录见 [最终整理说明](README.md)。

## 6. 轻量包、独立迁移与实际解压验收

2026-10-27交付补充了原生命令行运行依赖。解压后可直接指定冻结程序，不要求安装Qt或配置Git；编译源码和Debug全范围检查仍需要原工具链。

```powershell
python -m venv .venv
.venv/Scripts/python.exe -m pip install -r requirements.txt
.venv/Scripts/python.exe -B pipeline.py viking-v2 --out results/new_zip_run --exe output/runtime_20261027/IrrGeometryDesign.exe --shots 53 --windows 512 --counts 60 --seeds 84011 --hidden-sets r271828 --budget 80 --inner-folds 2 --workers 4
.venv/Scripts/python.exe -B pipeline.py module research.experiments.f3_transfer --out results/new_f3_transfer --exe output/runtime_20261027/IrrGeometryDesign.exe --workers 12
```

F3命令是冻结方案的重复计算，不是重新获得一个独立测试集；若根据已见F3结果修改方法，需另找外部评价资料。F3使用道/采样点索引，不把名义25单位网格当作已确认的物理坐标。

对完整交付ZIP的独立目录实测（项目根目录执行）：

```powershell
python tools/verify_delivery.py ../delivery/paper_project_final.zip --python .venv/Scripts/python.exe --out checks/new_delivery_replay
```

`--python`应指向新建虚拟环境，先安装项目与`manuscript_sources/english/requirements.txt`依赖。精确版本见`checks/closure_20261027/clean_environment_requirements.txt`。校验器清除外部Qt/Git路径，解压到临时非Git目录，执行核心测试、五方法缩小案例、预测独立复核和两份论文重建。验收在同一Windows主机上进行，不代表全新物理操作系统安装。

完整F3本地归档可另核验：

```powershell
python -B tools/verify_f3_transfer.py --root results/f3_transfer_20261027 --out checks/new_f3_audit
```

事后波形诊断入口为`research.analysis.waveform_quality`，读取已完成Viking批次，核对冻结预测后计算RMS比、相关性及频带相位误差。它不修改旧批次，不参与参数选择，不推定物理标定或成像效果。最新验收依据见[当前结果与材料总览](README.md)。

## 7. 历史可选入口

Viking v1 的复现入口仍为：

```powershell
python -B pipeline.py viking --out results/new_viking_v1 --workers 8
```

它使用 10 炮、单窗口、单周期 H、3 个掩码种子和旧恢复候选，适合重放问题发现阶段，不能与 v2 均值直接相减作为同任务收益。

旧 RRAW 另需 `data/real_seismic/Data/RRAW.SGY`、`real_gather_59x256.csv`、`real_gather_256x128.csv` 和绑定原始文件 SHA-256 的来源声明。当前缺项不阻塞 Viking 主流程。正式旧资料运行需真实证据填写 `source_confirmation.example.json`；模板不是确认。未确认资料只可显式标为敏感性诊断。

```powershell
python -B pipeline.py check --legacy --out checks/new_legacy_check --source-confirmation data/real_seismic/source_confirmation.json
```

旧 v4/v5 论文四图依赖原始表与来源记录。输入缺失时应停止，不能从图片推造数据。未使用的旧快照及旧图已清理。历史RRAW的缺失不被重新标为通过；仍被当前论文引用的独立实验和审计记录保留。
