# 地震采样点智能筛选与缺失道恢复：最终整理版

课题：**基于混合优化算法的地震采样点智能筛选方法研究**。作者：吴善州；导师：王华锋；校外导师：范子梁。整理日期：2026-10-27。

本文是剩余材料的统一说明。工作区只保留一个当前代码主版本、中文/英文各一份当前研究稿，以及一个最终交付ZIP。分散的旧进展报告、旧PDF、旧交付包、重复检查和可再生成的构建缓存已清理。

不同实验批次仍被当前论文用于主结果、基线、诊断或匹配对照，**不是可以互相替代的旧代码版本**。这些原始实验、登记数据、匹配的冻结源码/程序和独立审计记录保留。开题、中期及工程汇报等唯一原始材料集中到工作区的reference_materials目录。

## 1. 先看这些文件

| 内容 | 入口 |
| --- | --- |
| 中文六章研究稿 | [阅读PDF](../thesis/thesis.pdf)；[主编辑源](manuscript_sources/chinese/thesis.md) |
| 英文研究稿 | [阅读PDF](../paper-tgrs/tgrs_trace_selection.pdf)；[主编辑源](manuscript_sources/english/manuscript.md) |
| 唯一最终交付包 | [paper_project_final.zip](../delivery/paper_project_final.zip) |
| 最终包实际运行验收 | [绑定ZIP哈希的验收报告](../delivery/paper_project_final.verification.json) |
| 运行细节 | [RUNNING.md](RUNNING.md) |
| 清理记录 | [删除路径、字节和SHA-256清单](checks/cleanup_final/cleanup_manifest.json) |
| 清理后保留内容盘点 | [目录清单](checks/cleanup_final/remaining_inventory.json) |

稿件为研究初稿，尚未套用学校或IEEE最终模板，也不代表导师审定、期刊投稿或学位评审通过。两种语言稿件的实测表格来自同一套冻结CSV；本文不把软件通过与研究效果成立混为一谈。

## 2. 剩余工作区的全部组成

```text
sci/
  README.md                  工作区导航，指向本说明
  IrrGeometryDesign/         唯一当前工程、实验与主编辑源
  paper-tgrs/                当前英文PDF、TeX、BibTeX、展开稿、图片及构建记录
  thesis/                    当前中文PDF、TeX及展开稿
  delivery/                  唯一最终ZIP、SHA-256、实际解压验收报告和当前索引
  reference_materials/       唯一原始开题、中期、汇报与会议材料
  package_delivery.py        最终打包/校验兼容入口
```

### 2.1 当前工程及源码

| 路径 | 剩余内容及作用 |
| --- | --- |
| `src/native/` | C++/Qt应用、命令行、核心数值、搜索、恢复、几何、SPS输入输出、绘图、GUI与原生测试；一个当前实现 |
| `src/research/` | Python数据读取和登记、协议与实验编排、f-k/FISTA、恢复选择、统计、波形诊断和校验；`common/`集中路径、哈希、环境与可选Git信息 |
| `src/tests/` | Python回归与原生独立检查，不将缺失旧RRAW的跳过视为执行通过 |
| `src/resources/` | Qt资源清单、界面语言文件等原始构建资源 |
| `src/third_party/` | 当前工程实际依赖的第三方代码、库与原始许可说明，不擅自删除或改许可 |
| `src/build/` | VS2008/Qt4构建配置；其中legacy指兼容工具链格式，是当前构建所需文件 |
| `.github/workflows/` | Python CI配置；本地通过不代表已执行远程CI |
| `build/bin/Release/`、`build/bin/Debug/` | 当前两种构建的EXE、必需DLL与语言资源，分别用于运行和双构建数值验证 |
| `output/runtime_20261027/` | 当前发布运行依赖：16个登记程序/依赖/许可文件及清单；供无需外部Qt路径的命令行实验使用 |
| `tools/` | 最终打包器、独立解压实测器、F3归档复核器 |
| `pipeline.py`、`build.bat`、`IrrGeometryDesign.sln` | 统一Python入口、Windows构建入口与当前解决方案 |
| `requirements.txt`、`.gitignore`、`.gitattributes`、`.git/` | Python依赖、版本管理规则与原仓库记录；未重置用户暂存内容或擅自提交 |

已删除obj、生成代码、链接/调试中间产物和临时Python环境；构建命令会按需重新生成。源码中的可选历史协议/测试仍用于核对信息隔离、边界条件与回归，不是另一套并行维护的主版本。

### 2.2 登记输入

| 路径 | 数据与边界 |
| --- | --- |
| `data/public/viking_line12_small/` | Mobil AVO Viking Graben Line12小子集，10炮、每炮120道；包含原始SEG-Y、裁窗CSV、来源文档和准备说明，登记30个数据文件及2份来源文档 |
| `data/public/f3_transfer_small/` | F3叠后资料的3个小剖面，分别255×120；登记数组和文档约0.70MB，另保留约6.36MB来源字节范围与哈希记录用于追溯 |
| `data/sps/` | S/R/X三份SPS几何输入；9072个检波点、3162602条有效炮检关系、29719个活动CMP面元 |
| `data/sps_recovery_20261026.json` | SPS输入身份和恢复来源说明 |
| `src/research/data/*subset.json` | Viking与F3的权威登记清单，不能用数据目录内自行修改的manifest替换 |

Viking原始坐标单位码有历史冲突，当前尺度依据公开道间距和头段解释；绝对物理振幅未标定。F3使用道/采样点索引，不把名义25单位网格声称为物理25米。两个资料源的完整大文件未在本地整包核验，本地子集和获取范围分别校验。SPS几何集成不代表真实波形成像结果，来源和分发权利按原始材料处理。

### 2.3 当前论文依赖的实验与冻结支持

| 目录（均位于`results/`） | 规模与保留原因 |
| --- | --- |
| `repair_20261026/` | 修复后的合成基准：1980行搜索、1080行恢复，6个独立结构模板；保留search、joint、analysis与匹配runtime，支撑预算机制和合成对照 |
| `viking_20261026/` | Viking问题发现批次：10炮、450恢复、45搜索、1770次调用；支撑论文v1诊断与阶段比较 |
| `viking_diagnosis_20261026/` | 覆盖、频带、暖/冷启动、阈值和周期H的事后诊断；用于解释失败原因，不用于外层调参的正式优越性主张 |
| `viking_v2_20261026/` | 首轮开发对照：405恢复、117搜索、2299次调用；一个种子/一个内层分组，作为双分组匹配比较的基线 |
| `viking_robust_20261026/` | 当前主稳定性批次：1215恢复、351搜索、11725次调用；3种子/2个不重叠内层验证分组 |
| `viking_quality_20261027/` | 对4860份Viking预测的事后RMS比、相关性、频带相位误差与覆盖率诊断，未用于模型选择 |
| `f3_transfer_20261027/` | 新增外部叠后工区的冻结迁移：270恢复、126搜索、2496次调用；方案在读取波形值前固定，没有按F3外层分数重新调参 |
| `viking_v2_support_20261026/`、`viking_robust_support_20261026/` | 被引用批次匹配的冻结源码ZIP、程序/DLL和哈希清单；源快照服务于旧批次核验，不作为新的活动代码版本 |

这些目录的原始CSV、观测输入、掩码、选择记录、预测和清单形成完整证据链，主要磁盘占用来自这些必要实验档案。未为了缩小目录而删除独立核验仍要读取的原始文件。轻量ZIP只带汇总和必要输入，完整逐次预测留在工作区。

### 2.4 最终保留的检查

| 路径（位于`checks/`） | 用途 |
| --- | --- |
| `closure_20261027/project/` | 当前完整项目检查：113项Python、Release/Debug核心/数值/投影/端到端、登记Viking和SPS集成 |
| `closure_20261027/f3_archive_audit/` | F3独立核验：18580文件、1080预测数组、约3.08GB字节；投影误差0，分数误差约3.55e-15 |
| `closure_20261027/external_plan_identity.json`、`f3_source.zip` | F3读取波形前的固定方案身份与202份匹配源码快照 |
| `closure_20261027/pdf_qa.json`及`pdf_qa_final/` | 当前PDF的逐页边界/字形检查及最终联系表；大张逐页PNG可再生成，不作为旧版本保留 |
| `closure_20261027/clean_environment_requirements.txt` | 实际独立虚拟环境的精确Python依赖版本，临时环境在验收后删除 |
| `maintenance_20261026/` | 只保留仍有用途的单/双分组匹配表与脚本、预设示例记录、Viking完整归档审计和示例图生成脚本 |
| `repair_20261026_codec/`、`repair_20261026_fdct/` | 不同数值契约的编码与FDCT往返检查，并非重复完整项目检查 |
| `repair_20261026_sps/` | 论文SPS表格、空间目标与显式Gram核对的直接证据 |
| `cleanup_final/` | 本次删除清单、保留目录盘点；Git元数据、原始资料和论文所需文件未被当作缓存清理 |
| `final_delivery_replay/` | 唯一最终ZIP的实际解压重跑、独立预测核验与论文重建结果 |

### 2.5 稿件、图件与交付文件

`manuscript_sources/english/`与`manuscript_sources/chinese/`是唯一主编辑源，包含正文、稳定性/外部迁移补充、参考文献JSON、引用核验和构建/校验脚本。相邻paper-tgrs、thesis中的Markdown、展开稿、TeX、BibTeX与图片是同一版本的生成产物，不是另一份独立维护的稿件。`paper-tgrs/build_manifest.json`逐项绑定表格、图、文字与输出PDF。

`figures/assets/`是Qt资源清单引用的图标，`figures/gui/`是程序界面材料，`figures/research/`只保留论文相关合成、Viking、F3图件及其数值记录。`figures/aliases.tsv`是原生绘图路径路由配置，`figures/index.html`是当前图库；删除旧图后已刷新。

`delivery/`只保留最终ZIP、对应SHA-256、与ZIP实际身份绑定的验收报告、README及LATEST索引。ZIP不含GB级预测及完整历史逐次输出，也不打包原始开题/中期材料；这些唯一材料仍保留在本地工作区。实际验收报告独立置于ZIP旁，避免ZIP包含自身哈希报告的循环依赖。

## 3. 已验证结果与实际结论

当前完整软件检查通过：Python共113项，109通过、4项因未安装可选旧RRAW资料跳过；双构建核心、每构建588个独立数值案例、2个物理投影案例、端到端、Viking与SPS均通过。已有Viking大批次86144文件/4860预测的独立审计，新增F3审计通过。

| 保留道数 | Viking固定冷曲波 | Viking固定弱f-k | Viking内部选择 | F3固定冷曲波 | F3固定弱f-k | F3内部选择 | F3线性插值 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 36 | 1.8655 | 1.0802 | 2.3532 | 3.9336 | 3.9230 | 9.7933 | 9.6650 |
| 60 | 3.1042 | 3.3786 | 3.9891 | 6.7644 | 7.4948 | 12.5366 | 12.5503 |
| 84 | 4.2125 | 5.5544 | 5.4869 | 9.2706 | 10.4070 | 14.3428 | 14.3912 |

以上为各自冻结协议下五种布局等权的共同H平均SNR/dB。不同资料绝对分数不能相减作为算法增益。主要结论如下：

- 合成搜索中，Adaptive相对旧Hybrid六档比较有四档通过多重比较校正，尚未稳定超越SA。共同实际评价次数的对照更支持继续使用剩余预算的解释。
- 几何互干度下降不保证恢复变好。覆盖约束能强制控制最大间隔，恢复收益依赖密度、波形和恢复器。
- Viking内部选择相对固定冷曲波改善平均SNR，最高密度下固定f-k仍略高；先导谱加权没有稳定综合优势。
- F3中Jitter三档选择后平均SNR均最高；270案有215案选线性插值、55案选暖曲波。外部迁移保留了负结果，没有证明混合搜索普遍更好。
- Viking内部选择的三档预测/目标RMS比约0.497/0.662/0.764，相位MAE约40.06/25.85/18.68度。相关性改善仍伴随振幅衰减和相位误差，不能宣称已保幅或已改善成像。

Viking是开发测线，F3是一个新增叠后工区。炮、窗口、邻近剖面、隐藏集和随机种子都不增加独立工区数量。两份资料不构成普遍跨工区有效的证明；物理标定、动校正、倾角一致性、下游成像、学校格式与作者/导师审阅仍有明确边界。

## 4. 从当前保留内容运行

以下命令在IrrGeometryDesign目录执行，Python为3.12，原生平台为Windows x64。

```powershell
python -m venv .venv
.venv/Scripts/python.exe -m pip install -r requirements.txt -r manuscript_sources/english/requirements.txt
.venv/Scripts/python.exe -B pipeline.py test
.venv/Scripts/python.exe -B pipeline.py viking-v2 --out results/new_run --exe output/runtime_20261027/IrrGeometryDesign.exe --shots 53 --windows 512 --counts 60 --seeds 84011 --hidden-sets r271828 --budget 80 --inner-folds 2 --workers 4
```

完整复现Viking稳定性和F3方案：

```powershell
.venv/Scripts/python.exe -B pipeline.py viking-v2 --out results/new_robust --exe output/runtime_20261027/IrrGeometryDesign.exe --seeds 84011 94217 105953 --inner-folds 2 --workers 12
.venv/Scripts/python.exe -B pipeline.py module research.experiments.f3_transfer --out results/new_f3 --exe output/runtime_20261027/IrrGeometryDesign.exe --workers 12
```

每次使用新输出目录。冻结运行期间不要改源码、协议、输入或程序。重新跑已见F3资料只是重复计算，不能把已用于后续开发的F3再次当作未见外部测试。

源码编译与完整项目检查需要本地VS2008 SP1/Qt4.7.4工具链：

```powershell
.\build.bat Release
.\build.bat Debug
python -B pipeline.py check --full --mode both --out checks/new_full_check
```

`--full`检查当前公开资料和SPS；`--legacy`只用于另外具备旧RRAW资料及来源声明的历史范围。当前没有旧RRAW，不伪造其通过状态。原生冻结运行、Git缺失容错和源码编译是不同层面的能力。

## 5. 稿件构建、打包与独立验收

从sci工作区执行：

```powershell
IrrGeometryDesign/.venv/Scripts/python.exe -B paper-tgrs/build_manuscripts.py
IrrGeometryDesign/.venv/Scripts/python.exe -B paper-tgrs/check_manuscripts.py --final
python -B package_delivery.py create --out delivery/paper_project_final.zip
python -B package_delivery.py verify delivery/paper_project_final.zip
```

构建器读取保存的CSV，核验引用表及文件身份，生成当前两份PDF。使用ReportLab与Windows系统字体Times/Consolas/STSong/SimHei；TeX只是导出，本环境未执行XeLaTeX编译。打包器要求新目标文件，替换已有最终包时应先验收新包，再删除旧包；不另留旧版本目录。

从项目目录实际检验交付包：

```powershell
python -B tools/verify_delivery.py ../delivery/paper_project_final.zip --python .venv/Scripts/python.exe --out checks/new_delivery_replay
```

校验器在临时非Git目录解压，清除外部Qt/Git路径，使用指定独立虚拟环境执行原生核心测试、五方法缩小实验、信息隔离、预测独立核对、登记F3读取和两份论文重建。它与仅验证ZIP的逐文件哈希不同；受控验证在同一Windows主机完成，不代表全新操作系统或交互GUI全流程验收。

完整本地Viking/F3归档的独立复核命令见RUNNING。全部方法、恢复候选、8候选内层选择与分组规则保持原冻结协议，清理不重写历史实验manifest或结果。

## 6. 原始学术材料

`../reference_materials/`集中保留原有开题、中期材料目录，论文相关汇报PPT和CGU会议文稿。经哈希检查未发现可直接按字节判重的副本，因此保留各份唯一材料；它们是原始学术记录，不视作本次代码或论文的旧版本。逐项文件清单附于下方，移动前后路径和哈希另见清理记录。

## 7. 本次清理的范围

已实际删除7644个旧文件/缓存，合计473098795字节（约451.18MiB）。另将约196.94MiB的验收虚拟环境移出工作区，在最终解压检查结束后清除，不作为保留内容。25份唯一原始学术材料完整保留。

删除：旧交付包及旧精简包、被替代的论文PDF、旧报告/大纲、重复完整检查和smoke检查、无当前证据支撑的v4/v5旧图、重复输入ZIP、构建缓存、调试链接产物和临时环境。相应说明已合并进本README，稿件中失效的旧档保存表述已修正。

保留：Git仓库与用户暂存内容、当前全部源码/构建资源、登记数据、论文引用的不同实验、与实验绑定的冻结程序/源码、最新或具有独立用途的校验、许可和唯一原始学术材料。没有把仍在用于独立核验的掩码、预测或输入当作无用文件删除。

删除清单先记录路径、字节和SHA-256，再执行；所有删除/移动目标均验证位于D:/Desktop/sci或明确建立的任务临时目录中。最终ZIP重新实测，其自身哈希报告在ZIP外单独保存。

## 附：原始学术材料逐项清单

- [2024级硕士研究生中期答辩-吴善州/中期报告.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/中期报告.doc)（69.3 KiB）。
- [2024级硕士研究生中期答辩-吴善州/中期报告.pdf](../reference_materials/2024级硕士研究生中期答辩-吴善州/中期报告.pdf)（261.2 KiB）。
- [2024级硕士研究生中期答辩-吴善州/吴善州中期报告ppt.pptx](../reference_materials/2024级硕士研究生中期答辩-吴善州/吴善州中期报告ppt.pptx)（6695.1 KiB）。
- [2024级硕士研究生中期答辩-吴善州/吴善州中期检查表.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/吴善州中期检查表.doc)（29.0 KiB）。
- [2024级硕士研究生中期答辩-吴善州/吴善州中期检查表.pdf](../reference_materials/2024级硕士研究生中期答辩-吴善州/吴善州中期检查表.pdf)（85.0 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包/1-提交材料清单.docx](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包/1-提交材料清单.docx)（18.0 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包/2-研究生中期答辩卡.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包/2-研究生中期答辩卡.doc)（38.0 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包/3-研究生中期反馈卡.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包/3-研究生中期反馈卡.doc)（30.5 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包/4-答辩流程-2024级.docx](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包/4-答辩流程-2024级.docx)（17.4 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包/5-中期答辩成绩.xlsx](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包/5-中期答辩成绩.xlsx)（9.2 KiB）。
- [2024级硕士研究生中期答辩-吴善州/材料包(1).rar](../reference_materials/2024级硕士研究生中期答辩-吴善州/材料包(1).rar)（53.2 KiB）。
- [2024级硕士研究生中期答辩-吴善州/硕士专业学位研究生专业实践记录.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/硕士专业学位研究生专业实践记录.doc)（32.5 KiB）。
- [2024级硕士研究生中期答辩-吴善州/硕士专业学位研究生实践总结报告.doc](../reference_materials/2024级硕士研究生中期答辩-吴善州/硕士专业学位研究生实践总结报告.doc)（3205.5 KiB）。
- [2024级硕士研究生开题答辩-吴善州/吴善州开题ppt.pptx](../reference_materials/2024级硕士研究生开题答辩-吴善州/吴善州开题ppt.pptx)（6492.0 KiB）。
- [2024级硕士研究生开题答辩-吴善州/吴善州开题卡.doc](../reference_materials/2024级硕士研究生开题答辩-吴善州/吴善州开题卡.doc)（22.5 KiB）。
- [2024级硕士研究生开题答辩-吴善州/吴善州开题反馈卡.doc](../reference_materials/2024级硕士研究生开题答辩-吴善州/吴善州开题反馈卡.doc)（17.5 KiB）。
- [2024级硕士研究生开题答辩-吴善州/吴善州开题评审表.doc](../reference_materials/2024级硕士研究生开题答辩-吴善州/吴善州开题评审表.doc)（37.0 KiB）。
- [2024级硕士研究生开题答辩-吴善州/开题报告.doc](../reference_materials/2024级硕士研究生开题答辩-吴善州/开题报告.doc)（79.5 KiB）。
- [2024级硕士研究生开题答辩-吴善州/开题报告.docx](../reference_materials/2024级硕士研究生开题答辩-吴善州/开题报告.docx)（68.4 KiB）。
- [2024级硕士研究生开题答辩-吴善州/开题评审表.doc](../reference_materials/2024级硕士研究生开题答辩-吴善州/开题评审表.doc)（36.5 KiB）。
- [CGU2025-基于Jitter采样的混合优化算法在观测系统中的应用研究-吴善州.doc](../reference_materials/CGU2025-基于Jitter采样的混合优化算法在观测系统中的应用研究-吴善州.doc)（12.3 KiB）。
- [《基于混合优化算法采样矩阵优化技术研究》.pptx](../reference_materials/《基于混合优化算法采样矩阵优化技术研究》.pptx)（3011.7 KiB）。
- [基于混合优化算法的非规则观测系统优化技术研究-结题验收（验收版）.pptx](../reference_materials/基于混合优化算法的非规则观测系统优化技术研究-结题验收（验收版）.pptx)（23895.9 KiB）。
- [基于混合优化算法采样矩阵优化技术研究--中期汇报.pptx](../reference_materials/基于混合优化算法采样矩阵优化技术研究--中期汇报.pptx)（7745.8 KiB）。
- [基于混合优化算法采样矩阵优化技术研究--开题报告书.pptx](../reference_materials/基于混合优化算法采样矩阵优化技术研究--开题报告书.pptx)（4869.1 KiB）。
