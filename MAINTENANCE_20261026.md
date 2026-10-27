# 2026-10-26 修复、整理与交付记录

用户授权完善当前论文项目，并允许清理无用旧档。本轮保留原始资料、工程学位材料、当前科学结果、旧源码/程序冻结证据及仍被报告引用的历史研究。

## 当前入口整理

- `README.md` 改为 Viking 研究主线、当前检查及稿件导航，避免历史状态继续占据首页。
- `RUNNING.md` 区分当前 project/full、核心和可选 legacy RRAW 范围，加入多种子/双内层划分与冻结源码批次审计命令。
- `PROJECT_PROGRESS_20261026.md` 汇总当前批次、证据强度、稿件状态及剩余研究问题。
- 原首页全文保存于 [历史快照](docs/history/README_before_viking_primary_20261026.md)，增加历史标记并修正 Markdown 链接深度；原始文本 SHA-256 和调整后的归档 SHA-256 均记录在清理清单中。

批次的原始 manifest、源码/程序哈希和历史通过/不完整状态不因文档更新回写。

## 清理结果

已整体退役无外部引用、被首轮完整 v2 取代的 smoke 结果目录及对应图集：

| 退役目录 | 处理 |
| --- | --- |
| `results/viking_v2_smoke_20261026/` | 删除冗余预测、原生 CSV 和派生产物，先保存 manifest、计划、summary、隔离记录及逐文件哈希 |
| `figures/research/viking_v2_smoke_20261026/` | 删除对应冗余图集，保存逐文件哈希 |

共删除 **336 个文件、41,501,456 字节（约 39.6 MiB）**。所有递归删除目标先解析为绝对路径并验证位于 `D:/Desktop/sci` 内，拒绝目录/文件链接，使用 Python 单一流程完成。

详细记录为 [cleanup_manifest.json](checks/maintenance_20261026/cleanup_manifest.json)；旧 smoke 元数据位于 [retired_runs/viking_v2_smoke_20261026](checks/maintenance_20261026/retired_runs/viking_v2_smoke_20261026/)。该批次状态明确为退役、不可直接重放，不把残留 manifest 作为完整复现材料。

扫描未发现 Python 缓存或常见 tmp/bak 可删项。其他检查虽有重复，但属于历史证据链；当前和历史完整批次、原始 SPS/Viking、开题中期汇报、旧源代码/程序 ZIP 均保留。构建中间产物未在协作实验期间删除。

## 当前检查与科学证据

软件验证使用当前 Viking/SPS 范围，不再把旧 RRAW 缺失作为主流程阻塞。`--legacy` 保留独立旧资料诊断，缺项仍报告 incomplete。每个 Python 跳过记录原因，资料检查与科研效果分开描述。新稳健批次独立使用三个掩码种子和两个内层划分，原 v2 实验与清单完整保留。

最终运行规模、测试结果及交付包记录完成后汇入 [项目进展](PROJECT_PROGRESS_20261026.md)，避免将计划值误写为已完成。

最终稳定性批次已完成1215恢复、351唯一搜索和11725次成功调用。3个掩码种子、2个内层分组的全部条件保留，[独立归档核验](checks/maintenance_20261026/robust_archive_audit/verification.json)通过：86,144文件、约13.28GB、4860份预测，观测投影误差0。与首轮严格匹配的405案例固定恢复完全一致，单/双分组比较独立报告，见[VIKING_ROBUST_20261026.md](VIKING_ROBUST_20261026.md)。

当前正式项目检查已经通过：[检查清单](checks/project_current_20261026/checks.json)为project/passed，Python105项中101通过、4项旧RRAW依赖明确跳过；两构建登记Viking和SPS集成均通过，无缺失当前输入和源码变化。旧v2也已使用对应冻结源码通过[通用归档复核](checks/maintenance_20261026/v2_archive_audit/verification.json)。旧full清单没有回写。

## 可复现包入口

工具同时保存输入条款，不替原始资料新增开放许可。当前可创建可核验的本地精简复现包：

```powershell
python -B pipeline.py module research.validation.reproducibility_bundle create --out output/reproduction.zip --run results/viking_v2_20261026 --support results/viking_v2_support_20261026
python -B pipeline.py module research.validation.reproducibility_bundle verify output/reproduction.zip
```

工具附文件 SHA-256，包含代码、登记数据/来源条款、协议、汇总和图表，排除 GB 级原生预测和编译依赖。它是精简重跑与结果阅读包，不是原批次全部波形归档；完整原始预测仍保留在本机原批次目录。原生重建需要相同工具链或完整冻结程序环境。

当前英文/中文可编辑稿件与 PDF 由专用构建入口更新；历史英文 PDF 单列归档，稿件仍标为研究初稿。论文材料不得宣称未执行的外部确认、真实无噪真值或已完成投稿。

当前中文源稿位于`../thesis/thesis.md`，英文源稿位于`../paper-tgrs/manuscript.md`，`../paper-tgrs/build_manuscripts.py`从冻结CSV重建两份PDF和TeX/BibTeX。原大纲中的杂乱行清理前，完整字节已存于[原大纲](docs/history/THESIS_OUTLINE_before_full_draft_20261026.md)，身份见[大纲归档记录](checks/maintenance_20261026/outline_archive.json)。原8页英文PDF作为有引用的历史稿保留，未任意删除学位和来源材料。

最终英文13页、中文19页，合计32页已渲染检查，[视觉与机器记录](checks/maintenance_20261026/pdf_qa.json)通过。文献身份与主要一手元数据另有reference_audit，未将选择性的相关工作整理称为完整领域综述。
