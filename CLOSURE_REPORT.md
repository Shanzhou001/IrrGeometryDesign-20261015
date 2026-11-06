# 第三轮闭环完善与验收

> 2026-11-05更新：旧结果目录已按用户要求清理，仅保留最新版本。本文固定路径和执行数字属于历史记录；当前入口为`results/active.txt`，最新验收见[HARDENING_REPORT.md](HARDENING_REPORT.md)。


> 本文件为2026-11-04的封存交付记录。后续代码、Qt范围整合及异常路径修复见[HARDENING_REPORT.md](HARDENING_REPORT.md)。下文的当前程序和610文件身份仅指该次封存版本。

2026-11-04完成公平预算随机搜索、一个前瞻新工区、独立Python环境完整重算、程序标签修正和报告身份检查。本轮不提交Git。研究范围见[RESEARCH_SCOPE.md](RESEARCH_SCOPE.md)。

## 完成的问题

| 问题 | 本轮处理与验收 |
|---|---|
| 缺少同预算随机搜索 | SA、Hybrid、RandomBudget均为300次完整目标评价；18组随机搜索共5400个候选全部由原生目标评分并留存 |
| 条件收益缺少前瞻工区 | 在Parihaka波形读取前冻结协议，128道主条件、三种对照与六块一致性标准不随结果改变 |
| 原空间验证仅两个已见工区 | 新工区单列为1个，项目波形工区合计5个；Stratton/Volve继续作为已见资料，不重复计工区 |
| 互干度下降被误作恢复保证 | 同时报告有限64原子与全空间Fourier目标，以及目标降低但SNR下降的配对案例 |
| 信息隔离只抽查部分掩码 | 全部2592例执行未观测值NaN干预，观测投影及独立评分误差均为0 |
| 报告可以读取变更后的结果表 | 新报告核对recovery.csv与已验收manifest中的SHA-256，新增篡改回归测试 |
| 点数减少被标作实际成本节约 | 修正原生报告和图件标签，兼容字段costReductionPct仅解释为点数减少比例 |
| 复现仅限路径检查 | 从复制输入、新目录、新venv完整重算2592例，全部布局相同，最大指标差0 |
| 项目题目与方法范围不一致 | README、研究范围及7页汇报统一到预算约束布局与恢复适用条件；CG为兼容名称，CNN/GCN不列为已完成 |

## 正式实验与主要结果

新批次为`results/closure_20261104`，共2592例、36个空间—时间块、18个空间块。其中1296例是已有条件重新计算，最大数值差0；432例为新增随机搜索对照，864例来自新工区。不能将2592例全计为新增独立样本。两个批次及独立环境重算也不能累加成独立工区数。

Parihaka保留128/256道的预定主要比较：

| 比较 | 平均SNR差/dB | 正差块 | 预定六块均正标准 |
|---|---:|---:|---|
| Hybrid-Jitter | +0.373739 | 6/6 | 满足 |
| Hybrid-SA | +0.283340 | 6/6 | 满足 |
| Hybrid-RandomBudget | +0.168718 | 5/6 | 未满足 |

该标准是一个新工区内的描述性一致性判断，不是总体显著性检验。对同预算随机搜索的未满足结果完整保留，未调整窗口、阈值、样本或检验方式以改变结论。其他保留数和已见工区均作为次要描述性结果，见[完整报告](results/closure_20261104/report/report.html)。

## 源码、程序与检查

计算源树610个文件已存`source_snapshot.zip`。封存后仅补充报告结果表身份检查及其回归测试，当前交付另存`delivery_source.zip`，没有更改采样搜索、恢复、协议或数据选择。独立环境重算对应封存计算源树；后续报告维护由当前检查单独覆盖。

当前Python测试138项：134通过、4项缺少可选RRAW数据跳过，0失败、0错误。Release与Debug项目检查通过，两个模式的增量构建均0错误、0警告，不宣称旧工具链完整重建的既有警告全部消除。

新旧原生程序执行同一5436条空间查询，5437个结果/轨迹文件逐字节相同。新程序的实际GUI控件及13图加载、缩放、导航已在隔离目录检查。历史Qt完整工作流证明继续对应历史封存程序，不把本轮图库检查写成完整流程重跑。

默认运行包为`output/runtime_current`，19个登记文件核对通过。上一版运行包保存在`results/completion_20261103/runtime_current`，旧批次结果与原始汇报保留。

正式包为`output/reproduction_closure_20261104/reproduction_kit.zip`，包含当前源码、现有输入、Parihaka源字节缓存、运行依赖及新旧参考报告。实际文件数、大小与SHA-256以该目录`verification.json`为准。完整逐案例原始预测继续保存在原工作区和独立复现目录，参考报告没有伪装成可续跑的passed批次。

新环境依赖版本见`results/closure_20261104/report/environment.lock.txt`，复现包中同一文件位于`evidence/closure/environment.lock.txt`。需要精确复用该依赖组合时，可用`pip install -r requirements.txt -c evidence/closure/environment.lock.txt`。

最终源码/报告/运行包身份见[delivery_verification.json](results/closure_20261104/delivery_verification.json)，独立环境重算见[clean_reproduction.json](results/closure_20261104/clean_reproduction.json)。

## 数据传输与结论边界

Parihaka来源为https://zenodo.org/records/18988527。保留Jorge Leonardo、Chevron U.S.A. Inc.及New Zealand Petroleum and Minerals署名和编译数据的CC BY 4.0许可。使用发布者整理的数值体、道序号几何和未标定数字幅度。

原协议先于波形下载冻结。完整下载较慢，登记仅涉及传输的修订后获取同一文件中六个预定块。严格核对HTTP206、Content-Range、字节长度、下载前后发布者版本/MD5与本地逐段SHA-256。接收源字节6180992字节，保留解码块与原字节的一致性核验；未声称局部缓存已核对完整卷MD5。中断的全卷下载和失败探测记录保留，没有覆盖为passed。

本轮完成可执行的补充实验、核验和交付。原9072点SPS仍缺对应波形，物理保幅、下游成像、实际现场降本和全新Windows机器认证仍无证据。当前结论为条件性恢复效果和预算利用机制，普遍优越性尚未建立。待输入要求已在研究范围文档中列出。
