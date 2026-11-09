# 当前项目实验闭环验收

当前范围为用户于2026-11-08确认的 `public_field_applicability_v3`。Poland、Viking及Garner Valley承担正向实测应用，Soda Lake保留为指定接收点子集、采样比例与时间窗下的不适用案例。最终机器验收为 [project_current_closure.json](results/latest/project_current_closure.json)，可用 `python -B pipeline.py closure-status`重新核对。

原5151—5171测区保持几何案例。当前范围不主张原测区波形恢复、物理标定保幅、下游成像改善、实际现场费用降低或另一台机器认证。

## 实测结果

以下使用原始数字幅值，在每个炮点内先聚合窗口、采样比例、布局与掩码，再聚合炮点。零填充NMSE为1，越小越好。区间为20000次炮点聚类重采样的95%百分位区间，不代表独立工区的总体置信区间。

| 资料 | 留出炮点 | 优于零填充 | 平均NMSE | 95%区间 | 坐标IDW均值 |
|---|---:|---:|---:|---|---:|
| Poland | 4 | 4/4 | 0.956672 | [0.922515, 0.981883] | 1.636774 |
| Poland模型迁移至Viking | 10 | 10/10 | 0.651377 | [0.611462, 0.692938] | 1.649222 |
| Garner Valley原验证 | 4 | 4/4 | 0.872760 | [0.853746, 0.895192] | 1.222792 |
| Garner Valley新增确认 | 12 | 12/12 | 0.897964 | [0.871948, 0.922099] | 1.279989 |
| Soda Lake原方案 | 4 | 2/4 | 0.999277 | [0.998182, 1.000128] | 2.095107 |
| Soda Lake局部时移复验 | 4 | 1/4 | 1.000905 | [0.998220, 1.003024] | 2.635551 |

新增确认的炮点为41、47、64、35、52、58、72、95、120、78、89、114。在下载及读取新波形前冻结十二炮的选择、二十个布局种子、两个时间窗、30/50/70%保留率、四种布局与原模型。使用发布者196台站的真实ID与局部米制坐标，源力与垂直分量记录逐炮对应。八个开发炮、四个旧验证炮和十二个新确认炮分开登记，模型没有在新确认资料上重新拟合。

新增确认还评分**全部未观测道**，平均NMSE为 **0.890025**，95%炮点重采样区间为 **[0.872467, 0.908321]**，12/12炮优于零填充。相同目标的坐标IDW均值为 **1.177843**。逐案例保存完整缺失道预测、原始目标、IDW与掩码，独立原始数据评分最大差异为0。

数据来源为Vantassel、Cox和Crocker发布的[Garner Valley资料](https://doi.org/10.17603/ds2-ngdh-af80)。50份CSV的大小与SHA-256位于 `src/research/data/gvda_inputs.json`，下载回执、发布者许可与署名在 `data/public/gvda_2019_field`。这是已用工区内的新炮点确认，不算新的未见工区，也不替代深部三维成像验证。

数值与图件见 [应用报告](results/latest/field_applications/report/report.html)、[炮点统计](results/latest/field_applications/report/source_statistics.csv)、[逐例表](results/latest/field_applications/gvda_confirmation/recovery.csv)。

## 实现及审计修复

- 局部时移、Viking迁移、Garner Valley下载读取及恢复迁入 `src/research`，默认 `thesis --scope all` 与Qt全部实验统一接入，不依赖临时实验脚本。
- 布局保持每个实际坐标空间单元恰好一个接收点。SA、Hybrid与RandomBudget均使用200次几何目标评价，保存互干度、覆盖界限、实际掩码与预测。
- 新确认使用十二个新炮点、二十个布局种子，分别评价共同隐藏道和全部未观测道。九项次要布局比较使用炮点级精确双侧符号翻转检验及Holm校正。
- 独立审计核对原始波形、坐标、应有案例全集、隐藏集合隔离、空间单元约束、IDW和评分，拒绝缺例、NaN与目标变更。
- 协议原件与执行副本分别绑定哈希，同时核对JSON内容。允许排版差异，拒绝预算、种子、模型等参数变更。
- Qt工作流、源码身份、图库及分数证据保存在当前结果内部。执行目录只作来源说明，迁移后不依赖原绝对路径。
- 当前650文件源码的工程检查发现205项Python测试，201通过，4项因可选旧RRAW.SGY未安装而跳过。Release、Debug、数值、投影、注册数据及SPS集成检查通过，SKIP不计为通过。

## 算法结论

正向波形恢复与布局优化器的优势分别评价。几何感知恢复采用时移预测与开发炮回归，布局目标采用实际坐标Fourier Gram互干度。旧曲波单尺度接口保留历史协议，不声称它们是同一算子。

新增Garner Valley上，Hybrid相对Jitter、RandomBudget及SA的配对均差区间均跨零，不能主张普遍恢复优势。Viking上Hybrid相对Jitter改善0.015369 NMSE，Holm p=0.017578；相对SA反而恶化0.004434，Holm p=0.046875。九项次要探索比较保留全部方向和不确定性。降低互干度不能保证降低当前恢复器误差。

Soda Lake原方案及局部时移复验均未达到预设稳定改善标准。失败不被抹去，也不由其他资料冒充成功。历史八项范围的失败记录 `results/latest/recovery_project_closure.json`保持原样。当前十项范围加入明确的适用条件、边界报告、独立应用复算和Qt验收。

## 复现与交付

全流程包含基础7740例、公开炮集1728例、原始几何感知恢复1536例及应用阶段14016例，共25020条评价记录。应用阶段包含11520例新增确认及2496例既有方案复算。重复配置不能充当独立样本数。

历史计算由原源码快照、程序及manifest封存。与11月6日快照对比确认629个既有计算源文件一致，仅增加列明的应用模块并修改验收和流程接入。复用历史结果时逐文件核验原身份，新增应用阶段重新计算，不声称本轮从零重算全部25020例。

工程检查为 `results/latest/project_current_checks/checks.json`。独立应用复算证明为 `application_reproduction.json`，Qt证明为 `project_current_gui.json`，必要证据在本轮 `reproduction_evidence` 和 `qt_evidence` 中。同主机独立目录复算不等于另一台机器认证。完整图库为19图、7个报告入口。

本轮独立目录的14016条应用分数表与主目录逐字节一致，表中包含逐例预测文件的SHA-256。新增应用先在独立目录重新计算并独立评分，随后作为完整结果由Qt全部实验核验复用。Qt流程于2026-11-08实际通过，加载19图和7报告，并完成导航、缩放和刷新检查；原始过程日志随 `qt_evidence/process.log`保留。

最终交付身份为 `results/latest/project_current_delivery.json`。复现包由维护中的 `research.validation.reproduction_kit`生成，包含源码、注册数据、Release依赖及参考结果；解压逐文件核验后，在受控PATH下执行原生及完整Release检查。完整逐例预测保留在本机 `results/latest`，包内参考摘要不作为已完成或可续跑的结果批次。

结果根目录仅保留 `results/latest`一版。旧范围的封存清单及负结果作为审计证据保留在其内部。没有创建Git提交。

## 最终验收与交付索引

2026-11-08最终联合验收为 **passed，10/10项必验通过**。正式记录为 `results/latest/project_current_closure.json`。历史v2的效果失败保持原样。

主复现包为 `output/application_kit_20261108/reproduction_kit.zip`，大小1487621296字节，SHA-256为 `29ab7be4ce4a2d85a663498c5b525edd740cc86358a7d9e03aa87a9512cdb859`。1378个登记文件解压核验通过，受控PATH下的原生核心及完整Release工程检查通过。

配套 `output/application_kit_20261108/acceptance_evidence.zip`保存正式验收、独立复算、Qt证明、历史计算源码归档、当前结果表及本文。交付索引为 `results/latest/project_current_handoff.json`。

本节在主包验收后写入。主包内文档是打包时快照，配套证据包提供最终验收记录，避免主包哈希自引用。
