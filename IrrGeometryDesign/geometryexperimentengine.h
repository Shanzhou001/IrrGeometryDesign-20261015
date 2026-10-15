// add by shanzhou in 2026/10/15
#ifndef GEOMETRY_EXPERIMENT_ENGINE_H
#define GEOMETRY_EXPERIMENT_ENGINE_H

#include <QString>
#include <vector>
#include <complex>
#include "gproject.h"

// add by shanzhou in 2026/10/15
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
        float miu_regular;        // 规则减抽相干度
        float miu_random;         // 纯随机抽样相干度
        float miu_jitter_init;    // 初始抖动抽样相干度
        float miu_jitter_sa;      // 优化后 Jitter-SA 相干度
        float imprv_vs_regular;   // 相比规则减抽改善率(%)
        float imprv_vs_random;    // 相比随机抽样改善率(%)
        float alias_suppress_db;  // 相干假频与噪声抑制比 (dB)
        std::vector<float> convergence_trace; // SA 迭代收敛曲线
    };

    // 执行多采样率对比试验
    static bool runSamplingBenchmark(int N,
                                     const std::vector<float>& rates,
                                     const QString& outputDir,
                                     std::vector<BenchmarkRecord>& outResults,
                                     QString& outReportSummary);

    // =============================================================
    // Task 2: 观测系统空间属性质量评估与导出
    // =============================================================
    struct AttributeMetrics
    {
        int origReceiverCount;       // 原始满覆盖检波点数
        int optReceiverCount;        // 优化稀疏后检波点数
        float costReductionPct;      // 野外设备与采集成本节约率 (%)
        int liveBinCount;            // 有效覆盖面元数 (Live CMP Bins)
        float meanFold;              // 平均覆盖次数 (Mean Fold)
        int minFold;                 // 最小覆盖次数
        int maxFold;                 // 最大覆盖次数
        float foldStdDev;            // 覆盖次数标准差
        float foldUniformityCv;      // 覆盖次数不均匀度系数 (Cv = sigma / mean)
        float foldRetentionPct;      // 覆盖均匀性保有率 (%)
        float minOffset;             // 最小炮检距 (m)
        float maxOffset;             // 最大炮检距 (m)
        float meanOffset;            // 平均炮检距 (m)
        float nearOffsetRatio;       // 近炮检距占比 (0~1/3 MaxOffset)
        float midOffsetRatio;        // 中炮检距占比 (1/3~2/3 MaxOffset)
        float farOffsetRatio;        // 远炮检距占比 (2/3~1.0 MaxOffset)
        float azimuthCoverageScore;  // 方位角玫瑰图完整度评分 (0-100)
    };

    // 执行观测系统空间属性评估
    static bool evaluateProjectAttributes(GProject *pProject,
                                          const QString& outputDir,
                                          AttributeMetrics& outMetrics,
                                          QString& outReportSummary);

    // =============================================================
    // Task 3: 地震波场压缩感知重构试验引擎
    // =============================================================
    struct ReconstructionMetrics
    {
        int nTraces;                 // 总道数 (Nx)
        int nSamples;                // 时间样点数 (Nt)
        int sampledTraces;           // 采样道数
        float samplingRate;          // 实际采样率
        int iterations;              // 重构迭代次数
        float snr_zero_fill_db;      // 零值填充初始 SNR (dB)
        float snr_recovered_db;      // 重构恢复后 SNR (dB)
        float snr_improvement_db;    // SNR 提升幅度 (dB)
        float rmse;                  // 均方根误差 (RMSE)
        float correlation;           // 皮尔逊相关系数 r (0~1)
        double runtimeMs;            // 算法耗时 (毫秒)
    };

    // 执行地震波场压缩感知重构试验
    static bool runWavefieldReconstruction(const QString& maskFilePath,
                                          const QString& outputDir,
                                          float customRatio,
                                          ReconstructionMetrics& outMetrics,
                                          QString& outReportSummary);

    // 辅助工具：一键执行全部三个试验并导出完整成果
    static bool runFullExperimentSuite(GProject *pProject,
                                       const QString& outputDir,
                                       QString& fullReportSummary);
};

#endif // GEOMETRY_EXPERIMENT_ENGINE_H
