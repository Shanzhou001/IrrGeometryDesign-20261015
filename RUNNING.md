# 编译、实验与验收

2026-11-05起默认完整范围包含公平预算和Parihaka阶段，修复记录见`HARDENING_REPORT.md`。可以在Qt中选择“全部实验”“基础实验”或“公平预算与外部验证”；批次与范围显示在图库窗口。

```powershell
python -B pipeline.py thesis --scope all --workers 16
python -B pipeline.py thesis --scope closure --workers 8 --out results/my_closure_workflow
```

独立核验会拒绝NaN/Inf评分、无成功证据的记录、原生步骤失败及登记输出丢失。失败计算保留记录并停止，不自动重试。

仅修复验收入口且计算代码保持一致时，可以显式复用已封存计算：先保留完整计算源码快照，再使用`research.validation.gui_workflow --retain-study <计算批次> --out <全新验收批次> --scope all`。工具核对源码归档、源文件集合及允许变化的验收函数，逐项检查源/输入/输出身份；算法、协议、数据加载、恢复、分析代码及原生程序必须一致。计算源码身份与验收源码身份分别记录。该参数不用于跳过失败科学计算。

当前研究范围及原SPS所需输入见`RESEARCH_SCOPE.md`。默认all统一执行基础5148例与补充2592例，当前验收见`HARDENING_REPORT.md`；初次前瞻验证的历史记录见`CLOSURE_REPORT.md`。

在IrrGeometryDesign目录执行。Python3.12，原生工具链为VS2008 SP1 x64、Qt4.7.4。

```powershell
python -m pip install -r requirements.txt
.\build.bat Release
.\build.bat Debug
python -B pipeline.py test
python -B pipeline.py thesis --workers 16
```

完整流程与Qt实验窗口使用同一个入口。它依次运行项目检查、历史冻结合成对照、Viking、F3重复迁移、独立数值复核、波形质量、20结构补充实验、Stratton/Volve确认、二维空间波形实验、贡献分解、公平预算与Parihaka复验，最后统一发布图件和报告。完成前不要修改源码、程序、协议或输入文件。

第二轮增加1296例空间跨块复验与适用条件报告，协议为`spatial_replication_protocol.json`。仅有Release运行包时可用`python -B pipeline.py thesis --workers 16 --modes Release`；Qt在未提供Debug程序时自动选用Release检查。

## 批次身份与结果路径

```powershell
Get-Content results/active.txt
python -B pipeline.py thesis --workers 16 --out results/my_fresh_batch
```

活动目录由`results/active.txt`记录。源码或程序改变时创建新身份目录。每次完整流程通过后自动删除其他结果批次，仅保留最新成功版本；此规则也适用于closure范围，因此运行它会取代此前all范围。失败时停止并保留诊断，下一次成功后清理。相同身份续跑会核对登记输入、原始输出及预测；发现缺失、修改或未完成阶段会停止。失败记录见活动批次的`workflow.json`、阶段日志与`results/failures`，不要通过修改哈希强行复用结果。

当前唯一结果位于`results/latest`。旧目录已按用户要求清理，历史文档的固定路径不再是可用入口。最新版本内的`calculation_compatibility.json`登记实际计算源码归档和历史路径映射；原清单与哈希不改写，迁移后的文件仍逐项核对原SHA-256。`retention.json`记录自动清理结果。

## 输入与新增协议

```powershell
python -B pipeline.py module research.data.confirmation --download
```

首次联网下载Stratton和Volve完整SEG-Y卷，保存完整本地SHA-256、HTTP身份、来源、许可/署名及协议冻结时间。Teapot下载拒绝访问的可用性修订见`confirmation_amendment.json`与数据目录的`amendment_freeze.json`。重新执行时先检查已下载卷的身份。

补充阶段也可分别运行到全新目录：

```powershell
python -B pipeline.py module research.experiments.confirmation_synthetic --out results/new_synthetic --workers 8
python -B pipeline.py module research.experiments.confirmation_external --survey stratton --out results/new_stratton --workers 16
python -B pipeline.py module research.experiments.confirmation_external --survey volve --out results/new_volve --workers 16
python -B pipeline.py module research.experiments.confirmation_spatial --out results/new_spatial
```

这些命令不按外层分数改变冻结方案。新实验不会使F3重新成为未见测试，也不会为缺少对应波形的9072点SPS补出成像证据。

## Qt与运行包

```powershell
python -B pipeline.py run -- --gui-gallery-test
python -B pipeline.py run -- --gui-workflow-test
```

第一条按当前批次清单检查图件加载、缩放、翻页与刷新；目前all为14张、legacy为13张、closure为1张。第二条从Qt启动所选范围；已有同身份结果时先核验再复用。研究报告通过“原图 / 网页”中的链接打开。运行中不能关闭实验窗口。

完整流程证明写入当前批次`gui_verification.json`，单独图库检查写`gui_gallery_verification.json`，避免后一次图库检查覆盖完整流程执行记录。

运行包工具自动收集Qt依赖、VC90 CRT、VC90 OpenMP、中文翻译和许可文本，并在移除QTDIR、限制PATH的环境下测试。创建新运行包时必须使用全新目标目录：

```powershell
python -B pipeline.py module research.validation.runtime_bundle --exe build/bin/Release/IrrGeometryDesign.exe --out output/runtime_new
```

Qt4.7.4/VS2008仍是旧工具链。本机验证不等于所有Windows系统兼容；未通过测试时不得标为已验收。

## 复现包

```powershell
python -B pipeline.py module research.validation.reproduction_kit --out output/my_reproduction_kit --runtime output/runtime_current
```

工具打包源码、现有登记输入、Release依赖和参考汇总，解压到独立目录核对全部文件，再运行核心与Release项目检查。包内`START_HERE.md`列出完整重算命令。原始逐案例预测保留在工作区，不把只有汇总文件的包标成可续跑的passed批次。该工具的迁移验证范围是本机新路径。

## 公平预算与前瞻外部验证

新协议`src/research/experiments/closure_protocol.json`预先冻结Parihaka主要条件。SA、Hybrid与随机搜索各300次目标评价，随机搜索保存全部候选，按几何目标选择。Jitter为共同初始布局，不宣称其与优化器计算量相同。评价次数、实际运行时间和现场成本分别解释。

工作区和最新复现包均已带有预定Parihaka源字节与解码块。直接重算到新目录：

```powershell
python -B pipeline.py module research.experiments.closure_validation --out results/my_closure --workers 4
python -B pipeline.py module research.analysis.closure_report --study results/my_closure --out results/my_closure/report
```

仅在尚无缓存的新环境中下载预定块：

```powershell
python -B pipeline.py module research.data.closure_holdout --ranges
```

字节范围模式核对HTTP206、精确范围与长度、发布者前后版本及逐段哈希；不把局部缓存声称为完整卷MD5已核验。失败或部分缓存保留供检查，不静默重试科学计算，也不按结果替换块。

创建同时包含新验证参考报告的交付包：

```powershell
python -B pipeline.py module research.validation.reproduction_kit --out output/my_closure_kit --runtime output/runtime_current --study results/latest --extension results/latest/closure
```

## 正式交付与复现工具

这些入口均在`src/research/validation`中，随源码身份和复现包归档，不依赖`output/`中的历史辅助脚本。

```powershell
python -B pipeline.py module research.validation.gui_workflow --out results/my_gui_study --scope all
python -B pipeline.py module research.validation.delivery seal --study results/my_gui_study --runtime output/runtime_current --require-gui
python -B pipeline.py module research.validation.reproduction_kit --out output/my_kit --runtime output/runtime_current --study results/my_gui_study --extension results/my_gui_study/closure
python -B pipeline.py module research.validation.delivery verify --study results/my_gui_study --runtime output/runtime_current --kit output/my_kit
python -B pipeline.py module research.validation.reproduce --kit output/my_kit/reproduction_kit.zip --out output/my_independent_run --scope closure --create-env
```

`reproduce --scope all`可重算所有阶段；`closure`重算补充阶段并执行完整Release项目检查。两者明确记录实际覆盖范围。独立目录或Python环境验证均不等同另一台Windows机器认证。

## 数据冷启动和完整卷

```powershell
python -B pipeline.py module research.data.closure_holdout --prepare --directory data/public/my_holdout_cache
python -B pipeline.py module research.experiments.closure_validation --data-directory data/public/my_holdout_cache --out results/my_cache_study
```

范围下载会自行获取发布者元数据；不再要求先执行完整下载。已有完整卷和旧完整卷来源记录可通过同一`load_blocks`接口读取。网络读取最多尝试三次，响应范围或校验不匹配立即失败。部分缓存保留供检查，需要重新尝试时使用明确的新目录，不覆盖失败证据。

历史七页汇报的参数化重建源码位于`tools/presentation/rebuild_reference.mjs`，依赖模板与F3图件位于`data/templates`。运行`--help`查看参数，并按安装的Presentations技能完成最终导出和逐页检查。它重建历史研究参考，不冒充本轮GUI验收记录。

## 公开炮集联合验证与完整状态

`python -B pipeline.py field --workers 8`执行冻结的公开实测炮集流程；默认输出results/latest/mapped_field，拒绝覆盖已有结果。已有结果使用`python -B pipeline.py field --verify`独立核验。

`python -B pipeline.py original-site --out results/latest/original_site_application/registered`核验原测区几何及候选输入；可用--waveforms附加候选文件。尚缺同测区证据时返回2，不能据此认为程序崩溃或应用已通过。默认不自动猜测分量、坐标系统及外工区身份。

`python -B pipeline.py closure-status --out results/latest/project_closure.json`汇总当前检查、保留实验、新炮集与原测区状态。公开实测恢复为当前项目必验范围；原测区、物理标定、成像和实际成本分别列为可选扩展。Qt全部实验及thesis --scope all现统一包含原7740例与field阶段1728例及其报告。新版本必须先验证和归档后再替换旧文件，程序拒绝覆盖实验目录。

## 当前项目闭环验收范围

运行python -B pipeline.py closure-status --out results/latest/project_closure.json。必验项为基础公共/合成实验、配套公开炮集恢复、分层报告、当前源码检查、独立复算及交付包。全部通过返回0；缺必验项返回2。可选原测区入口仍返回其自身真实状态，不更改公共实测工区身份。

当前有效源码检查为results/latest/project_checks/checks.json。195项Python测试191通过、4项可选历史RRAW跳过。默认全部实验包括9468例及16图5报告。历史源版本、验收和清理记录保留其当时含义。
