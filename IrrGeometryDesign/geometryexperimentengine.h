// add by wushanzhou in 2026/10/15
#ifndef GEOMETRY_EXPERIMENT_ENGINE_H
#define GEOMETRY_EXPERIMENT_ENGINE_H

#include <QString>
#include <QList>
#include <vector>
#include <complex>
#include "gproject.h"

// add by wushanzhou in 2026/10/15
class GeometryExperimentEngine
{
public:
    // =============================================================
    // Task 1: 多采样率对比试验 (30%, 50%, 70%)
    // =============================================================
    struct BenchmarkRecord
    {
        float rate;               // 采样比例，如 0.30, 0.50, 0.70
        int N;                    // 空间道数/总检波点数
        int m;                    // 抽取后保留点数
        int run_index;            // 试验重复序号 (0, 1, ..., repeatRuns-1)
        unsigned long long master_seed; // 实验主随机种子
        unsigned long long actual_seed; // 本次实际使用的派生随机种子
        unsigned long long seed;  // 兼容旧字段 (= actual_seed)
        float miu_regular;        // 规则减抽相干度
        float miu_random;         // 纯随机抽样相干度
        float miu_jitter_init;    // 初始抖动抽样相干度
        float miu_jitter_sa;      // 优化后 Jitter-SA 相干度
        float imprv_vs_regular;   // 相比规则减抽改善率(%)
        float imprv_vs_random;    // 相比随机抽样改善率(%)
        float alias_suppress_db;  // 相干度比值衰减指标 (20*log10(mu_reg/mu_opt) dB)
        // ---- 2026.10.20 新增：空间均匀性三指标（衡量"非规则但均匀"） ----
        int max_gap_regular;      // 规则减抽最大相邻采样间隔（格）
        int max_gap_random;       // 纯随机最大间隔
        int max_gap_jitter_init;  // 初始抖动最大间隔
        int max_gap_sa;           // SA 优化后最大间隔
        float nn_cv_regular;      // 规则减抽最近邻间隔变异系数
        float nn_cv_random;
        float nn_cv_jitter_init;
        float nn_cv_sa;
        float jitter_unit_occupancy; // SA 后"每抖动单元恰一点"的比例(0~1)
        std::vector<float> convergence_trace;      // SA 当前解探索轨迹
        std::vector<float> best_convergence_trace; // SA 历史最优收敛曲线 (单调非增)
        std::vector<long> eval_trace;              // 逐步累计评估次数
    };

    struct MultiSeedSummary
    {
        float rate;
        int N;
        int m;
        int repeatRuns;
        unsigned long long master_seed;
        float miu_regular;
        float miu_random_mean;
        float miu_random_std;
        float sa_miu_mean;
        float sa_miu_std;
        float sa_miu_best;
        float sa_miu_worst;
        float imprv_vs_reg_mean;
        float imprv_vs_rand_mean;
        float coherence_ratio_atten_db_mean;
    };

    // 执行多采样率对比试验（支持指定随机种子与多随机种子重复试验）
    static bool runSamplingBenchmark(int N,
                                     const std::vector<float>& rates,
                                     const QString& outputDir,
                                     std::vector<BenchmarkRecord>& outResults,
                                     QString& outReportSummary,
                                     unsigned long long rngSeed,
                                     int repeatRuns);

    // =============================================================
    // Task 2: 观测系统空间属性质量评估与导出
    // =============================================================
    struct AttributeMetrics
    {
        bool isSynthetic;            // 是否为模拟/合成模式 (true=模拟基准, false=真实观测系统)
        QString modeName;            // 模式名称描述
        int totalShots;              // 总炮点数
        int origReceiverCount;       // 原始满覆盖检波点数
        int optReceiverCount;        // 优化稀疏后检波点数
        float costReductionPct;      // 野外设备与采集成本节约率 (%)
        int totalTraces;             // 参与统计的有效炮检道对数
        int invalidTraces;           // 无效/过滤记录数
        int liveBinCount;            // 有效覆盖面元数 (Live CMP Bins)
        float meanFold;              // 平均覆盖次数 (Mean Fold)
        int minFold;                 // 最小覆盖次数
        int maxFold;                 // 最大覆盖次数
        float foldStdDev;            // 覆盖次数标准差 (Fold StdDev)
        float foldUniformityCv;      // 覆盖次数变异系数 (Cv = sigma / mean)
        float foldRetentionPct;      // 覆盖保留度评估值 (%)
        float minOffset;             // 最小炮检距 (m)
        float maxOffset;             // 最大炮检距 (m)
        float meanOffset;            // 平均炮检距 (m)
        float nearOffsetRatio;       // 近炮检距占比 (0~1/3 MaxOffset)
        float midOffsetRatio;        // 中炮检距占比 (1/3~2/3 MaxOffset)
        float farOffsetRatio;        // 远炮检距占比 (2/3~1.0 MaxOffset)
        float azimuthCoverageScore;  // 方位角玫瑰图完整度评分 (0-100)
    };

    // 执行观测系统空间属性评估（严格区分合成数据与真实工程数据）
    static bool evaluateProjectAttributes(GProject *pProject,
                                          const QString& outputDir,
                                          AttributeMetrics& outMetrics,
                                          QString& outReportSummary,
                                          bool allowSyntheticFallback);

    // =============================================================
    // Task 3: 地震波场压缩感知重构试验引擎
    // =============================================================
    struct ReconstructionMetrics
    {
        bool isSynthetic;            // 是否为合成波场数据
        QString modeName;            // 数据源模式说明
        QString maskSource;          // 掩码来源说明
        int nTraces;                 // 计算网格总道数 (Nx)
        int nSamples;                // 计算网格时间样点数 (Nt)
        int exportNx;                // 导出网格道数
        int exportNt;                // 导出网格样点数
        float calcDt;                // 计算网格采样间隔 (s)
        float exportDt;              // 导出网格采样间隔 (s)
        float dx;                    // 空间道间距 (m)
        int sampledTraces;           // 采样实测道数
        float samplingRate;          // 实际采样比例
        int iterations;              // 重构迭代次数
        float snr_zero_fill_db;      // 基准1：零值填充初始 SNR (dB)
        float snr_linear_interp_db;  // 基准2：线性插值基准 SNR (dB)
        float snr_recovered_db;      // 本算法：压缩感知恢复后 SNR (dB)
        float snr_improvement_db;    // 相比零值填充净提升 (dB)
        float snr_imprv_vs_linear_db;// 相比线性插值净提升 (dB)
        float rmse;                  // 均方根误差 (RMSE)
        float correlation;           // 皮尔逊相关系数 r (-1~1) // add by wushanzhou in 2026/10/23
        // 2026.10.20 新增：缺失道口径 SNR（全幅 SNR 含已精确置回的道，差值由参考能量分布决定） // add by wushanzhou in 2026/10/23
        float snr_linear_missing_db; // 线性插值在未采样道上的 SNR (dB)
        float snr_pocs_missing_db;   // POCS 在未采样道上的 SNR (dB)
        bool dataConsistencyPassed;  // 观测道数据一致性检验是否通过
        double runtimeMs;            // 算法耗时 (毫秒)
        // ---- 冷启动/热启动对照 (用于学术公平性论证, 2026.10.19 新增) ----
        bool usedWarmStart;          // 本次 POCS 是否使用线性插值热启动初始化
        float snr_cold_start_db;     // 零填充冷启动 POCS 恢复 SNR (dB)
        float rmse_cold_start;       // 零填充冷启动 POCS RMSE
        float corr_cold_start;       // 零填充冷启动 POCS 相关系数
        float snr_warm_start_db;     // 线性插值热启动 POCS 恢复 SNR (dB)
        float rmse_warm_start;       // 线性插值热启动 POCS RMSE
        float corr_warm_start;       // 线性插值热启动 POCS 相关系数
        std::vector<float> residual_convergence; // 迭代残差收敛轨迹
        std::vector<float> residual_convergence_cold; // 冷启动迭代残差轨迹
        std::vector<float> residual_convergence_warm; // 热启动迭代残差轨迹
    };

    // 严格校验采样掩码文件合法性 (长度严格为 expectedN, 采样点数严格为 expectedSampledPoints, 取值只能严格为 0 或 1) // add by wushanzhou in 2026/10/15
    static bool validateAndLoadMaskFile(const QString& maskFilePath,
                                        int expectedN,
                                        int expectedSampledPoints,
                                        std::vector<int>& outMask,
                                        QString& outErrorMsg);

    static bool validateAndLoadMaskFile(const QString& maskFilePath,
                                        int expectedN,
                                        std::vector<int>& outMask,
                                        QString& outErrorMsg);

    // 严格校验并读取外部地震数据 CSV 文件 (Nt x Nx 矩阵，杜绝静默回退) // add by wushanzhou in 2026/10/15
    static bool loadAndValidateSeismicCSV(const QString& filePath,
                                         int expectedNt,
                                         int expectedNx,
                                         std::vector<std::vector<float> >& outGather,
                                         QString& outErrorMsg);

    // 执行地震波场压缩感知重构试验（支持外部地震文件输入、真实掩码校验、基线对比与全分辨率导出）
    // useWarmStart: true=以空间线性插值为初值 (热启动)；false=以零填充为初值 (冷启动，标准 POCS)
    // runStartupComparison: true=额外跑一次相反初始化并填入冷/热启动对照指标
    static bool runWavefieldReconstruction(const QString& maskFilePath,
                                          const QString& outputDir,
                                          float customRatio,
                                          ReconstructionMetrics& outMetrics,
                                          QString& outReportSummary,
                                          const QString& inputSeismicFilePath,
                                          bool exportFullResolution,
                                          bool forceDataConsistencyFailure = false,
                                          bool useWarmStart = true,
                                          bool runStartupComparison = true,
                                          // 2026.10.20: 复用落盘掩码时的原始来源标签（避免被误标为 External File）
                                          const QString& maskSourceOverride = QString());

    // 辅助工具：一键执行全部三个试验并导出完整成果（具备子试验状态核验与错误追溯）
    static bool runFullExperimentSuite(GProject *pProject,
                                       const QString& outputDir,
                                       QString& fullReportSummary,
                                       unsigned long long rngSeed,
                                       int repeatRuns);

    static bool runFullExperimentSuite(GProject *pProject,
                                       const QString& outputDir,
                                       QString& fullReportSummary,
                                       unsigned long long rngSeed);

    // 兼容原有无扩展参数签名的重载函数 (add by wushanzhou in 2026/10/15)
    static bool runSamplingBenchmark(int N,
                                     const std::vector<float>& rates,
                                     const QString& outputDir,
                                     std::vector<BenchmarkRecord>& outResults,
                                     QString& outReportSummary);

    static bool evaluateProjectAttributes(GProject *pProject,
                                          const QString& outputDir,
                                          AttributeMetrics& outMetrics,
                                          QString& outReportSummary);

    static bool runWavefieldReconstruction(const QString& maskFilePath,
                                          const QString& outputDir,
                                          float customRatio,
                                          ReconstructionMetrics& outMetrics,
                                          QString& outReportSummary);

    static bool runFullExperimentSuite(GProject *pProject,
                                       const QString& outputDir,
                                       QString& fullReportSummary);

    // =============================================================
    // 留出验证 (Hold-out validation) —— 2026.10.20 新增
    // 目的: 在**真实实测道**上评估插值/重构精度，而不是在插值构造的
    //       128 道参考场上自证。做法:
    //   ① 取 m 道真实实测数据作为输出网格 (Nx=m)；
    //   ② 随机保留其中 k 道作为"已知输入"，其余 m-k 道**真实实测**作为验证目标；
    //   ③ 分别计算 零填充 / 线性插值 / 曲波 POCS 在留出道上的误差；
    //   ④ 报告留出道 SNR，而非全网格 SNR。
    // 注意: 该接口复用 curveletPocsReconstruct（与尺寸无关），
    //       不受主流程 Nt=256/Nx=128 固定尺寸的限制。
    // =============================================================
    struct HoldoutMetrics
    {
        int    nTraces;          // 使用的真实道数 m
        int    heldOutTraces;    // 留出道数 m-k
        int    keptTraces;       // 已知道数 k
        int    nSamples;         // 时间样点数
        int    iterations;       // POCS 迭代次数
        unsigned long long seed; // 随机种子（决定 k 道位置）
        float  snr_zero_fill_db; // 留出道上的零填充 SNR
        float  snr_linear_db;    // 留出道上的线性插值 SNR
        float  snr_pocs_db;      // 留出道上的曲波 POCS SNR
        float  gain_pocs_vs_linear_db;
        float  gain_pocs_vs_zero_db;
        float  rmse_linear;
        float  rmse_pocs;
        bool   dataConsistencyPassed;
        double runtimeMs;        // 2026.10.20: 本组实际运行耗时
        QString inputSha1;       // 2026.10.20: 输入 CSV 的 SHA1（Qt4.7 无 SHA256，独立复核用）
    };

    // 输入 CSV: 每行 Nt 个时间样点? 否 —— 与引擎一致: 行=时间, 列=道。
    // 使用真实道集 CSV (例如 data/real_seismic 转换产物)，按 --traces 取前 m 道。
    static bool runHoldoutValidation(const QString& inputCsvPath,
                                     int useTraces,
                                     int keepTraces,
                                     int iterations,
                                     unsigned long long seed,
                                     const QString& outputDir,
                                     HoldoutMetrics& outMetrics,
                                     QString& outReport,
                                     const QString& externalMaskPath = QString());

    // 自动化单元测试与回归校验体系 (Task 11) (add by wushanzhou in 2026/10/15)
    static bool runUnitTests();
    static bool runReviewUnitTests(); // add by wushanzhou in 2026/10/24

    static bool runReviewSearch(const QString& inputCsv,const QString& outputDir,int seeds,int budget, // add by wushanzhou in 2026/10/24
        unsigned long long seed,bool tradeoff,QString& report); // add by wushanzhou in 2026/10/24
    static bool runConfiguredReconstruction(const QString& inputCsv,const QString& maskFile,const QString& outputDir, // add by wushanzhou in 2026/10/24
        int iterations,double thresholdStart,double thresholdEnd,int scales,int angles,bool warmStart,QString& report); // add by wushanzhou in 2026/10/24

    // 批量留出验证并归档 (2026.10.20 新增)
    // 对 keep x seed 组合逐一执行，写出机器可读的汇总 CSV，
    // 所有数值均由实际执行产生，便于第三方复算。
    static bool runHoldoutBatch(const QString& inputCsvPath,
                                int useTraces,
                                const QList<int>& keeps,
                                const QList<unsigned long long>& seeds,
                                int iterations,
                                const QString& outputDir,
                                QString& outReport);

    // 曲波变换参数鲁棒性实验（2026.10.20 新增 CLI 入口，结果可归档）
    static bool runMiuParamRobustnessExperiment(int N, float c,
                                                const QString& outputDir,
                                                QString& outReportSummary);

    // 混合优化贡献分解消融（2026.10.20 新增 CLI 入口）
    // 输出 hybrid_ablation.csv：SA-only / CG-only / SA+CG / 等预算随机重启
    static bool runHybridAblationExperiment(int N,
                                            const std::vector<float>& rates,
                                            unsigned long long seed,
                                            const QString& outputDir,
                                            QString& outReportSummary);

    // 2D 采样基准（2026.10.20 新增 CLI 入口）
    // 在 N×N 接收网格上比较规则 / 随机 / Jitter / Jitter-SA 的二维相干度，
    // 弥补"只有一维道抽取基准、2D 仅停留在 GUI"的缺口。
    static bool runSampling2DBenchmark(int N2D,
                                       const std::vector<float>& rates,
                                       unsigned long long seed,
                                       const QString& outputDir,
                                       QString& outReportSummary, int repeats = 1); // add by wushanzhou in 2026/10/23

    // =========================================================================
    // 2026-10-21 新增：目标函数数学性质探针（问题 1 取证用）
    // 输出每个用例的: 用例名, N, m, nbscales, nbangles, mu, 对角项最大值偏差,
    //                 G 的对角范围, ref 对角范围, 是否触发多块路径, 备注
    // =========================================================================
    static bool runMiuProbe(const QString& outCsvPath, QString& outReport);
    static bool runFftProbe(const QString& outCsvPath, QString& outReport);
    static bool runMiuDiscriminateProbe(const QString& outCsvPath, QString& outReport);
    static bool runMcProbe(const QString& outCsvPath, QString& outReport);

    // 2026-10-21 新增：空间约束优化 A–E 对比 + λ 敏感性
    // 2026-10-21 新增：多种子等预算消融
    // 2026-10-21 新增：优化掩码对重构效果的独立验证（10 掩码 × 3 档）
    static bool runRealMaskStudy(const QString& inputCsv,int count,int iterations, // add by wushanzhou in 2026/10/23
                                  unsigned long long baseSeed,const QString& outputDir,QString& report); // add by wushanzhou in 2026/10/23
    static bool runMaskReconStudy(const QString& outputDir,
                                  int masksPerType,
                                  unsigned long long baseSeed,
                                  QString& outReportSummary, // add by wushanzhou in 2026/10/23
                                  const QString& inputCsv = QString()); // add by wushanzhou in 2026/10/23
    static bool runMultiSeedAblationExperiment(const std::vector<float>& rates,
                                              int nSeeds,
                                              unsigned long long baseSeed,
                                              const QString& outputDir,
                                              QString& outReportSummary);
    static bool runConstrainedComparison(const std::vector<float>& rates,
                                         const std::vector<unsigned long long>& seeds,
                                         const QString& outputDir,
                                         QString& outReportSummary);
};

#endif // GEOMETRY_EXPERIMENT_ENGINE_H
