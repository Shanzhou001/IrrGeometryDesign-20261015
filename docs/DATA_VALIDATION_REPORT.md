# RRAW.SGY 独立验证报告（2026-10-24）

**结论：结构和道头读取已得到强字节证据支持；payload 的 IEEE-LE 解释仍未被来源方证实。**
本轮没有把 IBM 声明自动改成 IEEE，也未根据 SNR 选码。真实恢复结论全部条件于显式
`RRAW_IEEE_LE_UNCONFIRMED` 假设。不能回答“全部真实数据均已按证实方式解码”。

## 1. 原始文件和复现

原文件 `data/real_seismic/Data/RRAW.SGY`，76,760 bytes，SHA-256：
`ddcb311b2406af17cf375b570b3eb8ea7ca2d81cdeb002f972e6d668cd7b2bc2`。
只读程序 `tools/audit_segy_bytes.py`，命令：

```powershell
python tools/audit_segy_bytes.py --out results/new_byte_audit
```

实际输出归档 `results/audit_20261024_v5/segy_bytes/`。`trace_headers.csv` 保存全部
59 个道头原始 hex、偏移、哈希及 LE/BE 对照；`audit.json` 保存完整二进制头 hex、
ASCII/cp500/cp037 文本解释、字段集合、顺序映射和六种 payload 描述。
IBM 公式用独立 float64 位解码，与既有读取器不共享解码实现；解析字已通过单元测试。

## 2. 已证明的字节事实

3200 字节文本头+400 字节二进制头+59×(240+250×4)=76,760，无尾余字节。
LE 二进制 ns=250、dt=8000 μs、format=1、sorting=5、measurement_system=1；
revision/fixed-length/extended-text/新 endian-test 字段均为 0。全部道头 ns=250、dt=8000。
道序号 line 为 1…59；file 序号不连续，不能按连续编号补造缺道。
BE 相同字段得到 ns=64000、format=256，不能解释此完整固定四字节布局。
故当前 LE 头解析有结构支持；这不同时证明 payload 格式。

CDP 237/238/239/240/241 分别 8/15/15/14/7 道；source-point 字段有 15 个不同值，
field-record 全为 0，source Y 有 15 个不同值。排序码虽然为 common-source，
不能据此确认它真是一个单炮。offset 排序后每一对邻道 CDP 均不同，共 58 次变化。
header offset 为 59 个唯一值 −1560…1430，最小相邻差 26，最大差 182。
真实邻域的横向连续性因此不能仅由 offset 单轴保证。

coordinate scalar=0、coordinate units=0；source/receiver X=0，Y 非零；全部 59 道的
`(receiverY-sourceY)/offset=100`。不能自行补上 −100 scalar 并声称几何已校正。
measurement_system=1 支持 offset 的“头声明米”口径，实际坐标比例和 CRS 仍需确认。
振幅单位=0，trace weighting=0，延迟=0，time scalar=0；没有已知物理振幅标定。

## 3. 来源材料冲突

文本头含“2.0 SEC”“SR 2MS”“FEBRUARY 1988”“NMO TEST”“26 GI”“SOURCE INT 104M”。
随附 `data/real_seismic/Readme.pdf` 描述 59 channel off-end common-shot、约 12.3 m 道距、
33…747 m 距离。这与上述头中的 source、offset、CDP 和时间字段不一致。
PDF 文本只读提取到 `provider_readme_extracted.txt`；其哈希在基线清单中。
该 PDF 可能不适配当前文件或记录的是前期版本，**原因未知**。
不能以资料名相同来消除冲突，也不能把 PDF 的几何直接移植到本文件。

## 4. Payload 六种解释

|解释|有限样点|零值|abs 中位数|abs 最大值|其他格式证据|
|---|---:|---:|---:|---:|---|
|IEEE-LE|14750/14750|5|436.3333|7155|无 subnormal|
|IBM-LE|14750/14750|75|3098|915840|1233 个非零尾数首十六进制位为 0；70 个非规范零字|
|IEEE-BE|14750/14750|5|2.2047e−38|3.3809e13|4865 个 subnormal|
|IBM-BE|14750/14750|5|8.0988e−78|1.8587e25|3453 个非规范化非零尾数|
|IEEE pair-swapped BE|14750/14750|5|3.0510e−13|2.6641e36|4379 个 subnormal|
|IBM pair-swapped BE|14750/14750|5|2.1678e−27|4.6310e69|7457 个非规范化非零尾数|

这些统计支持继续显式保留历史 IEEE-LE 解释进行条件性复现，**不证明其来源语义正确**。
特别是 IBM-LE 也全有限，且其最大幅度并非旧“约 1e25”的 IBM-BE 反例。
IBM 可以表示非规范化值；异常尾数比例只是一项描述性证据。没有重构 SNR 参与解码判断。

## 5. 与标准及既有读取代码的核对

核对 [SEG 官方 rev 2 标准，Tables 2/3、§3.3、Appendix E](https://seg.org/wp-content/uploads/2025/11/seg_y_rev2_0_mar2017.pdf)：
ns/dt/format 及 offset/CDP 的字节位置符合定义；format 1 为 IBM，5 为 IEEE。
rev 2 允许 LE 与 pairwise swapped，因此“SEG-Y 标准永远只允许 BE”的旧注释不准确；
本文件 revision=0 且无 endian marker，不能按标准标志自动确认这些扩展。
坐标 scalar 用于坐标字段，不能任意用于 offset；零坐标 scalar 按 1，零单位表示未说明。

既有 `read_segy` 是 RRAW 专用 profile：硬编码 LE 四字节 IEEE、ns/dt/长度/有限值检查。
它不是通用 SEG-Y 标准解码器。原 GUI 的 SEG-Y 扫描路径也不等于此 Python profile。
目前没有证据授权更换真实波形解码。既有 `make_59x256.py` 的“payload 实为 IEEE-LE”
及 128 道 CSV 头中的确定性措辞已改正；新生成 CSV 明示未确认 profile，既有源数据/历史 CSV 字节不改。
两个转换脚本的 read_segy/convert 函数 AST 与冻结版一致，实际导出的全部数值与原 CSV 逐值相同，
证据为 `metadata_checks/conversion_metadata_audit.json`。带通原注释“余弦斜坡”也改为实际的 FFT 分箱线性斜坡。
新转换先绑定原始 SHA-256 并显式记录未确认 profile，不接受其他文件静默套用假设。

## 6. 预处理和信息隔离

原始 250×59（时间×道），按二进制/道头的8 ms解释为125 Hz，若以延迟字段0为起点则最后样点1.992 s；实际采集/重采样历史尚需确认。
4 ms 重采样的全记录是 499 点；256 点仅覆盖 0…1.020 s。插值不增加独立时间样点，
8 ms 的 Nyquist 为 62.5 Hz；上采样后带通标称至 70 Hz 不意味着新增有效频率信息。
59→128 的空间插值不增加独立实测道；该参考场不应充当实测隐藏道恢复的证据。
既有滤波器频带固定，无测试拟合参数，但先空间插值会在参考场内混合原始道，
全场峰值归一化也先接触了隐藏幅度。将其当作严格前瞻留出数据链是不充分的。

59 道留出链只做逐道时间插值、排序及全场峰值；恢复内部再只按 K 的峰值缩放，
线性运算下全场乘数可能抵消，但不能用代数说明代替对 float/CSV 舍入的实测。
本轮原始幅度实验删除此前全场归一化，内层/外层每次调用前将全部未观测列置零；
阈值尺度只由当次训练道确定。隐藏干预证据见 `real/isolation.json`（运行完成后读取）。

窗口四方案预先固定，不按测试值选方案：完整 native 250@8 ms、crop 128@8 ms
至 1.016 s、完整 upsample 499@4 ms、crop 256@4 ms 至 1.020 s。它们的目标能量不同，
不能直接以跨窗 dB 差作为同一任务的算法提升。新输出 provenance 记录原幅度、dt、
时间窗、形状和哈希。空间没有插值、数据自适应滤波、NMO 或测试标定。

## 7. 已复现的软件问题与修复

原幅度（峰值约 7155）进入实际坐标恢复器时，40 次迭代调用失败；归一化后同一观测成功。
证据 `reproduction_raw_scale/run.json` 退出码 1、`pre_fix_tests.log`；原因已定位到
`coordinateHoldoutReconstruct` 在反归一化后仍用固定绝对 `1e-5` 判据，浮点落盘误差随单位放大。
最小修复将终止判据改为 `1e-5*max(1,knownScale)`，和求解时的已知道尺度一致，仍记录
原幅度单位的绝对一致性误差。没有修改 FDCT、阈值、投影公式或历史输入。
这解释一个输入单位相关的失败状态，**不解释此前归一化资料负 SNR**。
新增测试同时比较原单位与归一化的预测等变性，当前通过。

旧 Qt `CSegyIO` 工具类另确认：非零指数/符号且零尾数的 IBM 字进入无限规范化循环；
format=5 的受控 IEEE 文件也被无条件当成 IBM 转换。独立 Release/Debug 测试都出现超时。
现已处理零尾数、区分支持的 IBM/IEEE BE、拒绝非法长度/未支持格式，修正数组 delete[]、
重复关闭/重新分配和生命周期。它仍不是通用 SEG-Y 解码器，也不自动读取争议 RRAW LE profile。
该工具类没有被本批正式恢复调用，其修复不能用来解释旧真实恢复数值。

## 8. 提供方必须确认

1. 与上述 SHA-256 匹配的来源、使用授权、导出软件/版本/选项及原始未重导出文件。
2. 头和样点各自的字节序、真实浮点格式、错误 format 字段的成因；提供若干确定样点的数值对。
3. 原始/当前采样间隔、是否从 2 ms 抽样至 8 ms、抗混叠处理、延迟/裁窗/滤波历史。
4. 这是单炮、CDP 集合还是其他重排；source/CDP/trace-id 的真实含义及完整分组映射。
5. offset 的符号/单位；Y 坐标比例为何相差 100；scalar、CRS、坐标单位及真实道距。
6. 振幅单位、AGC/增益/静校正/NMO/静音/叠加记录；PDF 是否与当前文件匹配。

在这些条件解决前，真实实验可复现其数值流程，不能升级为已验证的实测地球物理解释。
