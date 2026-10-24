# 构建、验证与结果追溯报告

基线提交72240bbf2a9ad02fec97fd250789d188d9db50fa，初始工作区干净；分阶段提交和独立输出，没有覆盖历史科学文件。
本报告从实际日志/JSON生成，手工GUI、异机全实验和未执行工具不会被计入通过。

## 当前最终程序

|check|exit_code|seconds|passed|failed|skipped|
|---|---|---|---|---|---|
|release_build|0|2.59983|—|—|—|
|debug_build|0|2.90739|—|—|—|
|python|0|6.45577|—|—|—|
|release_regression|0|103.654|33|0|0|
|debug_regression|0|168.798|33|0|0|

Python日志为33项（原24+新增9），0失败/跳过。C++主套件仍33个不同测试，Release/Debug是两次构建验证，不能记为66个新增测试。
旧SEG-Y/矩阵工具类另有7个专项用例，Release与Debug共14次执行全部PASS；此前受控IBM零和IEEE文件测试确实超时，不能抹去。
Release程序SHA-256：`5947bacb58c30a3dca40aaa4c272154afbc447e7b0173053b3e5c1340298d30a`。Debug SHA-256及原生命令在final_all_checks/checks.json。
编译器文件版本15.00.30729.01/产品9.00.30729.01（SP1），Qt4.7.4，保留原FFTW/FDCT。旧vcvars的Beta2文字不作为版本判断。
构建退出码为0，但仍有历史告警，具体数量以原日志为准，没有声称零告警。

## 数学、恢复和兼容性

- 独立字典/覆盖/J及DP：588例通过；数值最大误差见final_numerics/verification.json。
- 稠密实际坐标投影：2例通过；最大差6.0090332e-08。
- 纯FDCT解析复数组正反变换：20个维度/参数组合，在Release/Debug分别通过；最大相对L2误差2.2934246e-15，系数能量比偏差约4.0e−14。
- 当前程序重放：930个完整非限时搜索案例、6个预设恢复案例通过；搜索掩码和非计时轨迹一致，预测/线性/收敛CSV字节一致。
- v4专用旧入口兼容：396行（两域、两预算、每档2种子）科学列、掩码和旧轨迹逐字节一致；不比较计时列。
- 隐藏干预在原始幅度链预测差0；历史6位CSV预处理存在约1.05e−6峰值比例的可测依赖，未冒充严格通过。
- 非法权重/窗口、外层H被选、唯一可选集合、原单位尺度等回归已执行；测试失败返回非零，C++跳过也令整体失败。

旧整数MulMatrix_1D另经受控零坐标用例复现失败：它按值非零压缩，导致合法零坐标丢失/顺序错位。最小修复为按mask选择及正确分配容量；正式筛选不调用此旧函数。safety_final中两种构建均通过对应回归。
最终转换脚本会在新CSV中明确标注未确认编码/几何，既有CSV字节未变。metadata_checks中AST等价、数值逐值相同和Python完整回归均通过。

## 冻结实验与历史

v4独立复核：5843条记录，2202步，16716个原生输出哈希，15393个历史文件通过。原核验器的唯一报告写入被显式重定向到本轮目录，v4/final不写。
v5四阶段：2344步，10180个原生输出哈希通过；其中1104条恢复记录和990个搜索/限时查询独立复算。
冻结基线34526个文件仍逐字节一致，包括全部v4/final、v2/v3历史及data。
正式v5运行版本80ca9303；runtimes/v5同时保存实际程序、依赖和匹配源码。随后仅旧工具类和Diagmul改变；搜索/恢复核心C++模块哈希不变。两转换脚本只修正说明/输出头，read_segy/convert的AST及实际数值与冻结版一致。
当前源码/程序与旧指纹不符时默认核验已实际失败，只有显式--runtime指定冻结快照才能核验旧批次，见stale_runtime_rejection.json。
没有在新程序下静默复用旧缓存；也没有声称修复后把全部2172个联合阶段步骤重新执行一遍。
最终独立核验见final_integrity_verification.json；汇总复核summary_verification_extended.json独立复算6模板、24配对的每项64个符号置换、Holm、图表均值和60组相同前缀。

## 失败、开发记录和未执行项

|记录|实际状态|解释|
|---|---|---|
|reproduction_raw_scale / pre_fix_tests|真实复现失败；开发专项首轮2失败|包含原幅度判据Bug，以及新唯一可选分支尚未重编译；最终重编译后33项通过|
|safety_reproduction / codec_before|原IBM零字循环超时，IEEE路径也超时|受控程序由超时器终止；原类误把IEEE当IBM，非规范零尾数循环|
|codec_after|编译失败，未执行测试|首次文本补丁因LF/CRLF匹配未生效；修正后codec_final的12次执行全通过|
|current_checks|主动中断，非完整验证|保存INTERRUPTED.md，不能用部分通过代替current_final_checks|
|verification_launch|工具读取UTF-8基线时失败|明确修正编码，verification_retry及冻结快照完整复核通过；失败日志保留|
|人工GUI / ASan / Valgrind / 异机全重跑|未执行|没有计为通过；操作清单及环境限制见UNRESOLVED_ISSUES|
|独立真实工区|未完成|外部访问记录存在，未下载或冒称新增数据|

## 可复现命令和文件

所有原生命令和输入/输出哈希保存在各阶段manifest中。主要入口：

```powershell
python -m research.validation.run_audit_checks --build --out results/new_validation
python -m research.validation.audit_numerics --out results/new_numerics
python -m research.validation.audit_projection --out results/new_projection
python -m research.validation.run_legacy_codec_checks --out results/new_codec_checks
python -m research.validation.verify_audit_v5 results/audit_20261024_v5 --runtime results/audit_20261024_v5/runtimes/v5 --report results/new_frozen_verification.json
python -m research.validation.replay_audit_science results/audit_20261024_v5 --out results/new_replay
```

完整重跑研究使用audit_joint_studies.py的search/joint/real/timing四阶段，各--out必须是新目录；先冻结协议和匹配程序/源码，计时阶段顺序单独运行。
本地精确快照复核及当前程序重放通过，不等于已经执行干净克隆/异机完整复现。新归档使用局部.gitattributes -text保护CSV/源码快照字节；旧历史行尾变更仍会被哈希拒绝。
最终图件有独立数据JSON/PNG/SVG及final_figures_manifest；统计模板层分析、初版条件分析和图件草稿全部保留。
