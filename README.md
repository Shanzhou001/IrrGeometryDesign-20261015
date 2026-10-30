# 地震采样点智能筛选项目

目标：基于混合优化算法的地震采样点智能筛选方法研究。当前维护代码、实测输入、完整实验及Qt界面，不生成论文稿件。

详细修复记录、目录职责、科研图来源、验收结果及完整Qt操作路线见 [ACCEPTANCE.md](ACCEPTANCE.md)。

## 启动与GUI路线

1. 双击 `output/runtime_current/IrrGeometryDesign.exe`；也可运行 `python -B pipeline.py run`。
2. 点击工具栏的电脑图标，或菜单“空间优化与科研试验 → 毕业实验与结果图库”。此流程不要求先手工导入SPS。
3. 在“Python 3.12”填写解释器完整路径。本机为 `C:/Users/48198/AppData/Local/Programs/Python/Python312/python.exe`。并行数默认16，可选1—16；仅改变并行执行，不改变算法参数。
4. 点击“运行 / 核验完整实验”。窗口实时显示日志；运行中不能关闭实验窗口。首次完整运行可能需要较长时间。
5. 等待日志出现“完整实验及独立核验通过”。失败时查看 `results/current/workflow.json` 和对应阶段日志；失败不会显示为通过。
6. 点击“刷新最新图件”。用下拉框、“上一张”“下一张”查看13张当前图件；缩放窗口时图片保持比例。下方说明对应本轮数据。
7. 点击“原图 / 网页”查看可放大的原图；点击“打开结果目录”查看CSV、预测、协议和核验报告。

已完成的相同源码/程序批次会先逐项核对登记文件身份，再复用结果。源码或可执行文件变化后不允许混用旧结果。需要从零重算时，关闭程序，将 `results/current`、`figures/research/current`、`figures/current.tsv` 及 `figures/index.html` 移出工程备份，再重新启动并执行；不要在运行中改文件。

## 冻结实验参数

- 合成搜索：N=128，保留38/64/90道，预算200/600，20个配对掩码种子，包含旧Hybrid、SA、Adaptive及消融对照。
- 合成恢复：6个干净结构模板，各两个噪声条件；共同H和补集诊断分开，统计先聚合噪声和掩码重复。
- Viking：FFID53/453/953，原生4ms；窗口起点256/512/768、每窗256点；三个H；保留36/60/84道；种子84011/94217/105953；搜索预算300；12个先导道计入成本；2个不重叠内层验证分组。
- 五种布局：Jitter、SA_Mu、Adaptive_Mu、Adaptive_Gap、Adaptive_Signal。共享可行初值；后两者最大间隔175/100/75m。信号权重仅使用先导观测。
- 恢复：8候选（零、线性、两档f-k、四档曲波）；候选及幅度系数只使用已采集道内层验证，外层H完全隔离。固定基线独立报告。
- F3：3个相邻剖面、3个H、3种保留数、2个种子，沿用冻结历史协议。当前是已见资料的重复迁移，不宣称新的未见测试。
- SPS：实际9072个检波点及S/R/X关系；二维有限Fourier字典与显式Gram独立核对。几何通过不等于波形成像改善。

基础协议位于 `src/research/experiments/*protocol.json`；完整流程显式将Viking扩为3个种子、2个内层分组，本轮最终执行参数以 `results/current/viking/frozen_plan.json` 及各阶段manifest为准。直接单独运行模块的基础默认值不能代替上述完整流程。不要根据外层H分数挑选参数或删除负结果。

采样比例无效时输入框标红并拒绝执行，面元尺寸同样拒绝无效输入；模板道数或道间距修改后首炮位置自动更新。

## 输出与目录

- `src/native`：C++/Qt应用、命令行、搜索、恢复、几何、GUI、原生测试。
- `src/research`：数据、协议、实验、f-k恢复、统计和核验。
- `src/tests`：Python回归与原生独立探针；`src/resources`为Qt语言/资源清单，使用系统图标，不依赖已删除assets。
- `src/third_party`：曲波、Eigen、Qt绘图依赖及许可；`src/build`为VS2008/Qt4构建配置。
- `data`：必要的Viking、F3原始子集、来源文件和SPS。
- `results/current`：唯一当前实验；原始预测和审计输入完整保留。重复数据使用NTFS硬链接共享，大型CSV透明压缩；详见ACCEPTANCE.md。
- `figures/research/current`：13张当前科研图及数值来源；`figures/current.tsv`为Qt图库清单。
- `figures/gui`：实际GUI截图；其中OVT/聚焦能量等部分截图使用界面测试数据，不作为实测研究结果。
- `build/bin`：Release/Debug程序；`output/runtime_current`：可移植运行依赖。
- `tools`：归档核验工具；具体运行命令见RUNNING.md。

## 结果解释

SNR/NMSE只在对应隐藏道评价，保留负SNR。Viking和F3各算一个工区，炮、相邻剖面、窗口、H和种子不增加独立工区数。Fourier互干度是几何代理，不能保证曲波恢复。幅度RMS比、相关性和相位误差同时报告；原始资料没有物理振幅标定，未做动校正或下游成像验证。恢复选择的收益不能全部归因于混合搜索。旧RRAW未安装，依赖它的四项可选测试明确跳过。
