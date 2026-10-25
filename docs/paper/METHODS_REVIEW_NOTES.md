# 2026-10-25 论文方法独立核对记录

本记录用于本轮八页期刊稿的源码核对，不改变已归档的实验协议或数值。核对起点为
`8b9ffef2731f2210a8144900f155385d1dba266c`。这里的“与源码一致”不等于新增运行验证；
本轮实际测试状态以独立执行记录为准。

## 1. 核对范围与结果

| 主题 | 精确源码位置（项目相对路径） | 核对结论 |
| --- | --- | --- |
| Fourier 目标 | `src/native/optimization/OptimizeMiu.cpp:795`，`SamplingPhysicalCoherence()` | 字典频率为 k/L，L=N(x_last−x_first)/(N−1)，最大差频率为 N−1；均匀坐标退化到索引 PSF。 |
| 空间覆盖和软目标 | `src/native/optimization/OptimizeMiu.cpp:777`、`:783`、`:827`、`:865` | gap 包括左右边界；NN CV 使用总体标准差；物理软目标使用物理距离和 L/m 归一化。 |
| 精确道数与硬间隔 | `src/native/optimization/OptimizeMiu.cpp:839`，`SamplingPhysicalFeasibleMask()` | 有序候选 DAG 上的动态规划，保证回溯恰好 m 点；容量必要条件不能替代候选位置可行性。 |
| 传统搜索 | `src/native/optimization/OptimizeMiu.cpp:896`，`RunSamplingSearch()` | SAOnly 与旧 Hybrid 共用历史 SA 前缀；不能把新旧温度策略或缓存差异混作单一机制。 |
| Adaptive 状态机 | `src/native/optimization/adaptivesearch.inc:31`，`RunAdaptiveSamplingSearch()` | 当前、候选和精英分别维护；初值、扰动和重复掩码的完整目标评价计数；不可行提案只计尝试。 |
| 曲波迭代 | `src/native/reconstruction/wavefield_experiments.cpp:3`，`curveletPocsReconstruct()` | 固定迭代次数，粗尺度保留，其他尺度作 Wiener 型收缩，再进行数据投影。 |
| 实际坐标恢复 | `src/native/reconstruction/coordinateholdout.cpp:60`，`coordinateHoldoutReconstruct()` | 仅从 mask 内数据估计幅度尺度和初值，线性观测 Gram 用三对角求解。 |
| 内部选择 | `research/experiments/review_studies.py:104`，`validation_choice()`；`research/experiments/audit_joint_studies.py:129`，`nested()` | 增益和模型仅用保留集合内的验证道选择；最终用全部 K 重拟合。 |
| 共同外层 H | `research/experiments/audit_joint_studies.py:173`，`joint()` | H 为零基索引 4,12,…,124，共16道；允许集合112道，Fourier 域仍保留完整128个位置。 |
| 真实源身份 | `research/experiments/audit_joint_studies.py:58`，`real_raw()` | 该 v5 实验入口核对固定 SHA-256；底层旧转换函数本身只记录哈希。 |

## 2. 写作中必须保持的区别

### Fourier 几何目标不等于曲波字典互干度

当前 μ 来自所选位置上的 Fourier 列 Gram，而恢复使用 FDCT 曲波及插值观测算子。
它不是实际曲波观测字典 RΨ 的完整互干度。因此可以研究其与恢复精度的关联，
不能直接借用互干度稀疏恢复充分条件声称当前曲波恢复已有保证。
全保留不规则坐标也不必产生正交 Fourier 列。

### 目标评价预算不等于提案次数或墙钟

Adaptive 默认每全局阶段至少32次候选评价，并在精英连续12次未改善时切换局部。
局部交替使用最近空位及全域交换；12次局部失败后回收预算，重置到精英并尝试扰动。
扰动有 Metropolis 接受步骤，不能写作必然成功逃逸。
初值计一次，不能把未求 μ 的不可行提案记为完整目标评价。
30ms 实验的温度按评价比例推进，不能描述为温度进度也完全匹配的等时间比较。

### 缓存减少三角函数调用，不改变搜索评价数

均匀目标每次 FFT 评分约 O(N log N)。物理原子缓存构造和存储为 O(N²)，
当前物理评分仍遍历 N(N−1) 个位置，最坏时间 O(N²)；非零累加为 O(Nm)
不代表整个实现的总评分开销只有 O(Nm)。硬间隔 DP 最坏 O(mN²) 时间、O(mN) 存储。

### v4 与 v5 内选候选池不同

`research/experiments/review_studies.py:28` 的 v4 池为零预测、线性及11个曲波配置。
`research/experiments/audit_joint_studies.py:129` 的 v5 池仅为零、线性、默认和 cold，
曲波均用40次迭代。不能把 v4 大候选池用于解释 v5 的新结果。
最终增益来自内验证，应用于未知道，已知道保持观测；恢复器选择的收益不归因采样优化。

## 3. 合成数据的评价真值与统计单位

`research/experiments/audit_joint_studies.py:153` 的 `synthetic()` 使用256×128数组、
4ms时间步和12.5m道距。三家族 dipping/curved/faulted，各有两结构、两噪声条件；
12条件只包含6个干净结构模板。

三层 Ricker 事件 t0 为 .22/.46/.70s，频率22/25/28Hz，振幅因子为
`(1−.18*layer)*(1+.1*sin(x/400+layer))`。倾斜时差斜率为35/120µs/m，中层反号；
曲线模型采用双曲时距式、速度2600/1800m/s；断层在中心坐标>50m增加25/70ms。
噪声 SD 为 .01/.05，种子为65100加条件索引。

**噪声先加到 data，再保存到名为 truth 的文件，并以该 data 计算隐藏道误差。**
因此当前合成 SNR 衡量隐藏含噪道恢复，不是相对于无噪地下响应的去噪 SNR。

90个搜索掩码跨12模型复用；1080条运行记录不是1080个独立结构样本。
模板层分析先平均3掩码，再平均同模板2噪声条件，以6个模板作探索性配对。
这些模板仍来自3类确定性解析模型，不能代表随机地下总体。

## 4. 真实资料与预处理边界

只读运行 `read_segy()` 得到250×59数组，假定 IEEE-LE 下的峰值为7155，
header offset范围−1560…1430；CDP237…241的道数依次为8/15/15/14/7。
该运行核对数值解释的内部一致性，不证实编码、振幅单位或几何语义。

v5 用原幅度、59实测列，调用原生恢复前将未观测列置零，仅按当次训练道定尺度。
四种窗口/步长各自预设，时间插值不能增加独立观测，跨窗 SNR 目标也不同。
FDCT 处理数组索引，时间和空间像素尺度变化影响表示先验；这只是待区分的影响因素，
不能单凭负 SNR 确定其为主因。

真实源 SHA-256：
`ddcb311b2406af17cf375b570b3eb8ea7ca2d81cdeb002f972e6d668cd7b2bc2`。
IBM 头声明与 IEEE-LE payload 假设的冲突仍需提供方确认，不能写成已验证的真实解码。

## 5. 本轮确认并局部修正的材料问题

1. `docs/AUDIT_ISSUES.md` D2：0.007482原幅度单位的预测差，约为原始峰值7155的
   1.05e−6倍；不能把相对比值误读为原始峰值。
2. `docs/DATA_VALIDATION_REPORT.md` §5：固定源 SHA 拒绝保护限定于 v5 `real_raw()`；
   旧转换脚本只记录 SHA、检查部分布局，不自动鉴定编码。
3. 已通知负责方法材料的审查者修正 `THESIS_METHODS.md` 中同一峰值错误和过时图件描述。

本轮未发现必须重写核心公式或搜索状态机的证据。尚未证实的编码、独立真实来源、
切换/邻域/扰动普适优势及高保真恢复，继续保留为限制，不因完成八页稿而标记完成。
