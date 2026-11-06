# 第二轮完善与交付记录

> 2026-11-05更新：旧结果目录已按用户要求清理，仅保留最新版本。本文固定路径和执行数字属于历史记录；当前入口为`results/active.txt`，最新验收见[HARDENING_REPORT.md](HARDENING_REPORT.md)。


> 本文件是2026-11-03封存批次的历史记录。2026-11-04的公平预算、前瞻工区、程序标签及复现更新见[CLOSURE_REPORT.md](CLOSURE_REPORT.md)。这里的“当前源码/程序/运行包”均指当时封存身份，不代表后续版本已执行相同完整GUI工作流。

本轮按用户要求继续处理进展分析中的可落实问题，不执行Git提交。科学批次`results/completion_20261103`已由Qt实际启动并完整通过，5148例恢复及独立核验完成，耗时105.47分钟。当前交付代码的后续封装/身份检查补丁另行验收，未改写已封存的计算身份或结果。

## 已实施的改动

| 问题 | 本轮处理 | 验证方式 |
|---|---|---|
| GUI完整流程未在最新版本重新执行 | 使用实际Qt运行按钮启动新批次，不以图库检查替代流程执行 | 保留进程起止时间、程序与工作流哈希；完成后要求`workflow_executed=true` |
| 图库检查覆盖GUI完整流程记录 | 图库单独写入`gui_gallery_verification.json`，完整流程继续写`gui_verification.json` | 完整流程完成后再运行图库检查，检查完整流程证明仍在 |
| 测试、资源和独立脚本变化未完整纳入身份 | 批次身份覆盖非生成的src/tools资源、Qt UI/qrc/qm、入口脚本、协议、测试与构建文件，流程结束前再次复查 | 新回归检查资源修改和工具脚本新增必须被拒绝；当前交付范围604个文件 |
| 已有核验记录与manifest未严格绑定 | 复用时检查`manifest_sha256`、源码、程序及登记输入输出 | 修改manifest但不改输出的回归案例必须被拒绝 |
| 空间波形只验证一个数据块 | 在Stratton、Volve各选六个互不重叠空间块，两个时间窗、两个隐藏集、三个种子 | 1296例预检通过；显式Gram、预测重算、观测投影与未观测值NaN干预 |
| 互干度与恢复质量关系缺少条件解释 | 新增按工区、结构族、噪声、保留数、恢复器和隐藏条件分层的配对差值 | 完整CSV保留负结果，不把分层探索包装成确认性发现 |
| 恢复器变化容易被当成布局收益 | 单列候选选择分布，并统计相同恢复器下互干度降低但SNR降低的案例 | `recovery_selection.csv`、`objective_alignment.csv` |
| 缺少可携带的源码/输入/依赖交付 | 新增复现包工具，收集源码、现有登记数据、Release运行依赖和参考报告，并补齐完整流程必需的独立F3审计脚本 | 完整解压后核对文件哈希，在新路径运行核心与Release项目检查 |
| 运行包导入列表将已附带VC90列为系统依赖 | 根据实际附带DLL过滤该列表 | 运行包完整哈希与受限PATH核心检查 |
| 空间恢复参数无效时仍可继续 | 拒绝非正迭代数及越界阈值比例 | 回归测试覆盖无效控制参数 |

## 空间扩展设计与结果

协议为`src/research/experiments/spatial_replication_protocol.json`，在解码新增块/时间窗之前冻结，记录位于`data/public/spatial_replication_20261103/freeze.json`。

两份完整数据体此前已被项目访问，扩展不能声称增加未见工区。每工区取六个互不重叠的16×16道序号空间块，时间窗口为[128,256)、[768,896)；两组共同隐藏位置各32个。保留数76/128/179，三种种子，Jitter、SA、Hybrid三种布局，固定三维Fourier/FISTA恢复参数不变。

两个时间窗、隐藏集和种子先在块内聚合，再等权平均六块。空间块仍可能相关，不将1296例当作1296个独立样本，也不据两个已见工区宣称总体显著性。

同一网格、隐藏集、保留数和种子的布局不读取波形，几何搜索执行一次后在各数据块复验。1296是恢复案例数，不是1296次相互独立的几何搜索；实际搜索及评价次数保留在原生记录中。

“固定恢复”指迭代数80与阈值比例0.01的规则固定。归一化尺度及实际阈值仍由各布局的已观测数据计算，并非所有布局共用同一个绝对阈值；结果解释为相同观测驱动恢复规则下的布局比较，不作对采样点本身的因果识别。

本轮正式结果与预检一致。1296例评分独立重算差0，观测投影和未观测值NaN干预差0，显式Gram最大差7.77e-16。Hybrid相对Jitter的工区内块平均SNR差如下：

| 保留数 | Stratton/dB | Volve/dB |
|---:|---:|---:|
| 76 | +0.390754 | +0.383461 |
| 128 | +0.382927 | +0.344675 |
| 179 | -0.043148 | -0.008773 |

76/128道时两个工区的六个块均为正差；179道时收益不稳定。Hybrid相对SA在128道约有+0.202/+0.197dB均值差，其余保留数不呈现一致的相对优势。这是需要保持条件限定的结果，不作普遍优势结论。

## 科学版本与交付版本

科学流程记录在[workflow.json](results/completion_20261103/workflow.json)，实际Qt启动记录在[gui_full_run.json](results/completion_20261103/gui_full_run.json)，封存验收在[final_verification.json](results/completion_20261103/final_verification.json)。完整GUI测试后又执行单独图库测试，`gui_verification.json`中的`workflow_executed=true`仍保留。

计算批次原251文件身份保持原样，另存251文件`frozen_source.zip`及603文件的`scientific_full_source.zip`。封存后只修改了复现包的工具收集、验收文件枚举、工作流身份复查三个维护文件，新增一个回归测试文件。原生程序、GUI、采样搜索、恢复器、数据选取与协议的源码及字节身份均未改变，没有将封装补丁冒充成重新计算的科研结果。

当前交付代码包含604个检查文件，验收与差异范围见[delivery_verification.json](results/completion_20261103/delivery_verification.json)，当前源码另存`delivery_source.zip`。完整项目检查见`delivery_checks_final/checks.json`。132项Python测试中128通过、4项可选RRAW跳过；Release、Debug核心、数值、投影、流程和登记数据检查通过。先前暴露旧测试夹具兼容问题的失败记录保留在`delivery_checks`，没有覆盖成passed。

历史2565个案例与原批次的关键数值最大差0，候选选择一致。原有补充实验和1296例空间扩展全部通过独立核验。基础13图与5张补充图、7页汇报均已更新并检查。

## 运行与复现交付

Release、Debug本轮增量构建均为0错误、0警告；这不等于消除了旧工具链完整重建时的既有警告。默认启动程序为`output/runtime_current/IrrGeometryDesign.exe`，仍与实际完整GUI测试的程序字节一致。旧包归档在`results/revision_20261103/runtime_current`，迁移后19个登记文件核验见[runtime_handoff.json](results/completion_20261103/runtime_handoff.json)。

复现包预检约371MB，746个登记文件解压后逐一核对，核心与Python/登记数据的Release项目检查通过。该检查发生在本机新目录，未冒称一台全新Windows机器的兼容性认证。复现包中的参考报告放在`evidence`目录，不复制成一个缺少原始预测却标为passed的可续跑结果批次。

正式交付包位于`output/reproduction_completion_20261103/reproduction_kit.zip`，完成状态、实际文件数、包大小和SHA-256以同目录`verification.json`为准。正式包包含F3独立审计脚本，预检版仅作为过程记录。适用条件报告见[applicability_report/report.html](results/completion_20261103/applicability_report/report.html)；新版汇报位于`output/reports/研究进展汇报_完善版.pptx`，原汇报和上一版汇报保留。

## 交付边界

源码和运行包将按当前身份保存，不依赖Git提交。历史批次、原始汇报和已保存预测继续保留。复现包包含现有输入和参考汇总；完整逐案例原始预测继续保存在原工作区批次中，完整再现按包内命令重新运行。

原9072点SPS仍无对应实测波形；实际采集降本、物理保幅和下游成像仍缺证据。首次历史偶发退出的底层根因无法从缺失的旧日志恢复。Qt4.7.4/VS2008源码构建工具链保留，当前实现范围不含开题中的CNN/GCN系统。
