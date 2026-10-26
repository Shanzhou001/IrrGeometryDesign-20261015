# 构建、验证与实验复现

源码、输入、构建和结果均以当前项目根目录为工作目录。`pipeline.py` 可从其他目录调用；传给研究模块的相对输入、输出路径均相对于项目根目录。

## 构建与检查

原生程序需要 Windows x64、VS2008、Qt 4.7.4；Python 使用 3.12。环境查找与原生构建配置位于 `src/build/`。GitHub Actions 仅执行 Python 回归，原生和外部资料检查仍需本地工具链。

```powershell
python -m pip install -r requirements.txt
.\build.bat Release
.\build.bat Debug
python -B pipeline.py test
python -B pipeline.py check --out checks/new_core_run --build
```

`test` 只运行 Python 测试。`check` 默认执行 Python、Release/Debug 科学核心回归、588 项数值对照、物理投影对照和合成端到端检查。检查目录必须不存在；`checks.json` 保存命令、程序与源码哈希、Python 通过/跳过数量和最终状态。单构建检查可加 `--mode Release` 或 `--mode Debug`。

`--test-core` 是不需要外部资料的原生科学检查。完整 `--test` 还包含历史工程集成检查；缺少输入会明确失败，不能用 core 通过代替完整验证。

```powershell
python -B pipeline.py check --out checks/new_full_run --full --source-confirmation data/real_seismic/source_confirmation.json
```

完整检查要求原始 SEG-Y、两份派生 CSV 和 SPS 的 S/R/X 文件，同时要求真实来源声明有效、Python 测试无跳过。缺输入、跳过或未确认来源会将结果标为 `incomplete`，并返回非零退出码。

独立底层检查也可以单独运行：

```powershell
python -B pipeline.py module research.validation.run_legacy_codec_checks --out checks/new_codec
python -B pipeline.py module research.validation.run_fdct_probe --out checks/new_fdct
```

## 自包含的合成研究

以下命令产生新结果，不能覆盖现有批次。搜索采用 20 个配对种子、200/600 次评价上限；共同隐藏集恢复包含 12 个预设模型/噪声条件、3 个掩码种子和 5 个主方法。

```powershell
python -B pipeline.py module research.experiments.review_studies --out results/new_batch/search --stage search --synthetic-only --workers 4
python -B pipeline.py module research.experiments.audit_joint_studies --out results/new_batch/joint --stage joint --workers 4
python -B pipeline.py module research.analysis.synthetic_results --v4-root results/new_batch/search --joint-root results/new_batch/joint --out results/new_batch/analysis --group research/new_batch
```

两个实验应使用同一个冻结原生程序；实验期间不要修改源码或重新构建。记录器核对源码、输入和程序身份，分析器核对原始表与轨迹哈希。

分析按 6 个干净结构模板聚合噪声条件和掩码种子，再计算配对差、置信区间及多重比较修正；不把全部重复当成独立地质场景。恢复 SNR 只在共同隐藏道计算，保留负值；内层选择包含零预测、线性插值和曲波候选。结果不会预设 Adaptive 或 Curvelet 必须胜出。

输出包含原始 CSV、掩码、轨迹、预测、逐次日志、manifest、统计表和三张新图。`figures` 命令只刷新图库，不会补跑实验：

```powershell
python -B pipeline.py figures
```

## 小规模 Viking 真实资料实验

`pipeline.py viking` 是独立于历史 RRAW 的新入口，默认读取 `data/public/viking_line12_small/`。外部数据及目录名含 viking 的新运行产物不纳入 Git；正式代码中的 `src/research/data/viking_line12_subset.json` 固定 10 炮原始 SEG-Y、派生 CSV、坐标表和来源文档的身份。数据准备阶段的脚本仍在下载包内。

先核验全部输入，再执行实验；输出目录和图件分组必须为新位置：

```powershell
python -B pipeline.py viking --validate-only --out checks/new_viking_dataset
python -B pipeline.py viking --out results/new_viking_run
```

默认设置为 10 炮、每炮 120 道、256 个原生 4 ms 时间样点；保留数为 36/60/84，3 个配对掩码种子，搜索评价上限 300，恢复迭代 40 次。共同外层隐藏位置为零起始 4、12、...、116，共 15 道；选点只能使用其余 105 个位置。Jitter、SAOnly、旧 Hybrid、SA_Budget、Adaptive 共用初始布局。相同相对检波点几何只搜索一次，45 组几何掩码复用于各炮，共形成 450 组恢复对照。

验证器按已登记身份核验 30 个数据文件和 2 份来源文档，用独立 IBM 实现重解原始 SEG-Y，并逐值检查 CSV 裁窗和坐标表。不会根据振幅外观猜测编码，不进行空间扩道、时间重采样、滤波或全场归一化。原始坐标单位码 3 与距离单位解释不一致，按公开采集资料的 25 m 道间距、坐标增量和二进制头 measurement system 解释距离；该限制进入数据报告、manifest、图件元数据和总结。

恢复沿用共同 H 与内层验证流程，只向原生进程序列化已观测振幅；候选为 zero/linear/default curvelet/cold curvelet，并仅用内层数据选幅度系数。每个批次实际执行一次 `H <- 13*H+7` 干预，要求参数选择、固定恢复和选择后预测均不改变。

需要控制计算量时可明确缩小配置，例如保留全部 10 炮和三档密度，只做一个配对种子：

```powershell
python -B pipeline.py viking --out results/new_viking_pilot --seed-count 1
```

`--shots 53 453 953` 可指定登记炮号；`--counts`、`--budget`、`--iterations` 和 `--workers` 可调整配置，实际设置均记录在清单。输出包含逐次日志、查询、掩码、收敛轨迹、原生预测、内层选择、逐案例分数、来源与隔离报告，以及 `analysis/summary.md`、5 张统计表和 3 张新图。

统计先平均同一炮的掩码种子，再描述炮间差异。10 炮来自一个工区，可能空间相关；不将炮数或掩码次数当作独立工区样本，不生成独立工区显著性 p 值或伪置信区间，负 SNR 保留。图中分别呈现几何目标、实际评价次数与共同 H 恢复质量。

分析可脱离原生程序重做，仍需保持原始结果表、轨迹及清单完整；不能覆盖已有分析或图件：

```powershell
python -B pipeline.py module research.analysis.viking_results --root results/new_viking_run --out results/new_viking_analysis --group research/new_viking_analysis
```

此入口不要求旧 RRAW 来源声明，也不冒充旧资料复现。下文 `check --full` 仍指历史 RRAW/SPS 全部集成检查，其缺项不能用 Viking 结果标成已通过。

## 历史 RRAW 真实资料与来源声明

真实输入位置：

```text
data/real_seismic/Data/RRAW.SGY
data/real_seismic/real_gather_59x256.csv
data/real_seismic/real_gather_256x128.csv
data/sps/5151-5171.S
data/sps/5151-5171.R
data/sps/5151-5171.X
```

保留的 RRAW 解码配置存在 IBM 头字段与 IEEE 小端载荷解释的冲突，不能靠幅值合理性推断提供方意图。`src/research/data/source_confirmation.example.json` 是未填写的声明模板；来源 SHA-256、编码、8 ms 采样间隔、offset 米单位、振幅单位、几何说明、确认人及证据引用必须按真实材料填写。模板本身不是确认。

正式真实实验使用 `--source-confirmation <json>`；只有用于敏感性诊断时才显式使用 `--allow-unconfirmed-real`。后者的 manifest、报告和图件保留未确认标记，不具备已确认真实资料结论资格。现有解码器不支持的编码应先实现并验证相应解码器，不能修改声明迁就当前实现。

v4 的正式路径保留原始振幅，逐次只向重建进程写入已知道。旧全场归一化与空间插值链仅用于历史诊断，不能混入严格隔离实验。

完整英文论文四图需原始 v4/v5 表与来源记录；输入缺失时一次列出所有缺项，并在写图前停止：

```powershell
python -B pipeline.py module research.analysis.paper_figures --v4-root results/v4 --v5-root results/v5 --group paper
```

不应从历史 PNG 反造原始结果表。没有真实资料时使用前述合成研究分支。

## GUI 与二维 SPS 目标

GUI 模板采样与优化使用精确保留数和正式 Fourier/PSF 自适应搜索，每个非退化搜索最多 600 次评价；线间与线内分别优化，取消时不提交计算结果。任意导入二维几何不能按规则模板坐标改写。

真实 SPS 属性实验另采用 `SpatialFourier2D_v1`：在实际 `(x,y)` 坐标上定义有限二维 Fourier 字典，默认每轴最多 8 个原子，周期为该轴孔径加正间距中位数。缓存共轭半平面差频并增量评价交换；SA 与 Hybrid 共用初值、种子和 64 次 SA 前缀，评价上限均为 200。

该协议导出坐标、字典频率与周期、掩码、每次评价轨迹及属性对照。它与旧一维 v4/v5 是不同协议；有限频率目标改善不等于无条件抗混叠保证或恢复 SNR 改善。
