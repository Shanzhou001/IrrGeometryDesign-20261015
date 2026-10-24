# 执行入口

日常入口统一为 `python pipeline.py <命令>`，详见 [README](../README.md)。
原生程序位于 `build/bin/Release/IrrGeometryDesign.exe`，原 GUI 和下列 CLI 开关继续保留：

`--test` / `--run-tests` / `-t`、`--run-experiments` / `-e`、`--run-real-reconstruction`、
`--holdout-validate`、`--holdout-batch`、`--miu-robustness`、`--ablation`、`--sampling-2d`、
`--mu-probe`、`--fft-probe`、`--mu-discriminate`、`--mc-probe`、`--constrained`、
`--ablation-multiseed`、`--holdout-study`、`--mask-recon`、`--review-tests`、
`--review-search`、`--review-reconstruct`、`--audit-queries`、`--screenshot`、
`--screenshot-all`、`--screenshot-survey`。

新增 `--replay-reference <新输出目录> <已有掩码> <比例> [输入CSV]` 用于恢复历史示例的矩阵
及实际迭代轨迹；没有输入 CSV 时使用原有确定性解析模型。它不重新搜索掩码或选择超参数。

## 研究模块

|用途|入口|输出要求|
|---|---|---|
|v5 搜索/联合/真实/限时实验|`python -m research.experiments.audit_joint_studies --help`|`--out` 新目录；阶段分开|
|v4 正式研究|`python -m research.experiments.review_studies --help`|保留原协议及独立输出|
|v3 研究和快速流水线|`python -m research.experiments.run_revised_studies --help`|新目录，`--quick` 为软件验证|
|真实记录窗口研究|`python -m research.experiments.review_window_study --help`|新目录|
|历史掩码对照|`python -m research.experiments.compare_masks --help`|显式给定道集和掩码|
|SEG-Y 字节审计|`python -m research.data.audit_segy_bytes --help`|独立输出，不自动判定争议编码|
|128 道参考插值场|`python -m research.data.convert_rraw_to_engine_csv --help`|明确是插值参考场|
|59 道转换|`python -m research.data.make_59x256 --help`|IEEE-LE 假设仍未确认|

## 验证与图表

|用途|入口|
|---|---|
|两构建/完整自动回归|`python -m research.validation.run_audit_checks --help`|
|独立目标和硬约束枚举|`python -m research.validation.audit_numerics --help`|
|稠密坐标投影对照|`python -m research.validation.audit_projection --help`|
|FDCT 正反变换|`python -m research.validation.run_fdct_probe --help`|
|旧 SEG-Y/零坐标专项|`python -m research.validation.run_legacy_codec_checks --help`|
|原始预处理干预|`python -m research.validation.audit_preprocessing_isolation --help`|
|当前程序搜索/预测重放|`python -m research.validation.replay_audit_science --help`|
|迁移后的完整历史核验|`python -m research.validation.archives --help`|
|代码/资源/图片检查|`python -m research.validation.layout --help`|
|仅重绘论文 10 图|`python -m research.analysis.render_audit_figures results/audit_20261024_v5`|
|全部图件与界面|`python pipeline.py figures`|

`analysis` 中的统计函数与图表入口已分开。完整图片集中生成时读取归档统计表，
不会把计划实验写为结果或重新选择权重。历史核验使用显式快照，当前程序重放使用当前构建。

`research.maintenance` 的冻结/归档/日期注释工具是上一轮审计的专用维护工具，
不是日常流水线步骤；保留其防覆盖、指纹匹配和一次性归档限制。
`scripts/migrate_layout.py`、`scripts/consolidate_figures.py` 是本轮已执行的一次性迁移，
不应在已迁移的树上再次执行；其映射和基线已归档。
