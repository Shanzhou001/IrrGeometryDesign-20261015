#include "experiment_internal.h"

bool GeometryExperimentEngine::runFullExperimentSuite(GProject *pProject,
                                                     const QString& outputDir,
                                                     QString& fullReportSummary,
                                                     unsigned long long rngSeed,
                                                     int repeatRuns)
{
    QDir dir(outputDir);
    QDir().mkpath(outputDir);   // 2026.10.20: 原 dir.mkpath(outputDir) 会把 outputDir 重复拼接到 dir 之后

    QString dir1 = dir.filePath("01_sampling_benchmark");
    // 2026.10.20: 真实 SPS 几何评价不属于"合成"实验，放到输出目录的兄弟目录
    // real_sps/attribute_evaluation/ 下，避免与 synthetic/ 混淆。
    // 2026.10.20 修复：属性评价目录原来固定写到 <输出目录>/../real_sps，导致
    //   --test 的 Test 10 在沙箱中运行时会覆盖 results/real_sps 下的正式报告。
    //   现改为：沙箱运行写到沙箱内部；否则仍写到 results/real_sps/attribute_evaluation。
    QString dir2;
    if (outputDir.contains("unit_test_sandbox", Qt::CaseInsensitive))
    {
        dir2 = dir.filePath("real_sps/attribute_evaluation");
    }
    else
    {
        dir2 = QDir(dir.filePath("..")).filePath("real_sps/attribute_evaluation");
    }

    QString dir3 = dir.filePath("03_wavefield_reconstruction");
    // 2026.10.20 修复: dir.mkpath(dir1) 会把 dir1 当作相对于 dir 的路径，
    // 而 dir1 已含 outputDir 前缀，结果创建出 experiment_results/experiment_results/01_...
    // 的空壳目录。改用 QDir() 以绝对/完整路径创建。
    QDir().mkpath(dir1);
    QDir().mkpath(dir2);
    QDir().mkpath(dir3);

    QString s1, s2, s3;
    std::vector<float> rates;
    rates.push_back(0.30f);
    rates.push_back(0.50f);
    rates.push_back(0.70f);

    // 2026.10.20: 掩码溯源（见下方复用 cmp_mask.txt 处的说明）
    QString provenanceSource; // add by wushanzhou in 2026/10/23

    std::vector<BenchmarkRecord> bResults;
    bool ok1 = runSamplingBenchmark(128, rates, dir1, bResults, s1, rngSeed, repeatRuns);

    GProject *pProjToUse = pProject;
    bool ownProject = false;
    if (pProjToUse == NULL)
    {
        pProjToUse = loadRepositorySpsProject();
        if (pProjToUse != NULL) ownProject = true;
    }

    AttributeMetrics aMetrics;
    bool ok2 = evaluateProjectAttributes(pProjToUse, dir2, aMetrics, s2, true);

    if (ownProject)
    {
        GProject::destroy();
        pProjToUse = NULL;
    }

    QString maskPath = QDir(dir1).filePath("mask_50pct_sa_best.txt"); // add by wushanzhou in 2026/10/23
    provenanceSource = "Benchmark optimized ClassicPSF mask (50%, best across declared runs)"; // add by wushanzhou in 2026/10/23
    if (QFile::exists(dir.filePath("input_mask.txt"))) // add by wushanzhou in 2026/10/23
    { maskPath = dir.filePath("input_mask.txt"); provenanceSource = "Explicit suite input mask"; } // add by wushanzhou in 2026/10/23
    ReconstructionMetrics rMetrics;
    bool ok3 = runWavefieldReconstruction(maskPath, dir3, 0.50f, rMetrics, s3, "", true,
                                          false, true, true, provenanceSource);

    // 同步将生成的 cmp_mask.txt 复制或导出到 experiment_results 根目录供外部调用
    if (QFile::exists(dir3 + "/cmp_mask.txt")) // add by wushanzhou in 2026/10/23
    {
        QFile::remove(dir.filePath("cmp_mask.txt")); // add by wushanzhou in 2026/10/23
        QFile::copy(dir3 + "/cmp_mask.txt", dir.filePath("cmp_mask.txt"));
    }

    // 严格核验预期输出文件存在且非空
    bool f1_ok = checkFileExistsAndNonEmpty(dir.filePath("01_sampling_benchmark/benchmark_comparison_table.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("01_sampling_benchmark/benchmark_convergence_curves.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("01_sampling_benchmark/benchmark_summary_report.txt"));

    bool f2_ok = checkFileExistsAndNonEmpty(QDir(dir2).filePath("attribute_fold_distribution.csv")) &&
                 checkFileExistsAndNonEmpty(QDir(dir2).filePath("attribute_offset_distribution.csv")) &&
                 checkFileExistsAndNonEmpty(QDir(dir2).filePath("attribute_azimuth_rose.csv")) &&
                 checkFileExistsAndNonEmpty(QDir(dir2).filePath("attribute_quality_report.txt"));

    bool f3_ok = checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_original_gather.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_subsampled_gather.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_recovered_gather.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_residual_slice.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_metrics_summary.csv")) &&
                 checkFileExistsAndNonEmpty(dir.filePath("03_wavefield_reconstruction/recon_experiment_report.txt"));

    bool allSuccess = ok1 && ok2 && ok3 && f1_ok && f2_ok && f3_ok;

    QString statusHeader = allSuccess
        ? QString::fromLocal8Bit("【全套科研闭环对比试验执行完毕 - 全部成功 (3/3)】\n")
        : QString::fromLocal8Bit("【全套科研闭环对比试验执行完毕 - 存在失败项】\n");

    QString subStatus = QString::fromLocal8Bit(
        "子试验 1 (多采样率基准对比): %1 (文件核验: %2)\n"
        "子试验 2 (空间属性质量评估): %3 (文件核验: %4)\n"
        "子试验 3 (波场压缩感知重构): %5 (文件核验: %6)\n\n"
    ).arg(ok1 ? QString::fromLocal8Bit("成功") : QString::fromLocal8Bit("失败"))
     .arg(f1_ok ? QString::fromLocal8Bit("正常") : QString::fromLocal8Bit("缺失"))
     .arg(ok2 ? QString::fromLocal8Bit("成功") : QString::fromLocal8Bit("失败"))
     .arg(f2_ok ? QString::fromLocal8Bit("正常") : QString::fromLocal8Bit("缺失"))
     .arg(ok3 ? QString::fromLocal8Bit("成功") : QString::fromLocal8Bit("失败"))
     .arg(f3_ok ? QString::fromLocal8Bit("正常") : QString::fromLocal8Bit("缺失"));

        QString centralFiguresDir = getCentralFiguresDir(outputDir);   // 2026.10.20: 统一输出到 figures/
    QDir().mkpath(centralFiguresDir);

    // 50% 采样率相干度衰减与 beta/μ 指标一律取多轮重复试验的聚合均值（下方计算），
    // 不在任何采样率缺失时虚构默认值。
    float atten_db_50 = 0.0f;

    if (allSuccess)
    {
        // 50% 采样率指标一律取"多轮重复试验的聚合均值"，与 benchmark_comparison_table.csv 口径严格一致；
        // 修复(2026.10.19): 原实现取 bResults 中首个 50% 记录（单轮值），
        // 会与汇总表(3 轮均值)出现 11.36 dB vs 12.25 dB 的自相矛盾。
        int n50 = 0;
        double sumReg50 = 0.0, sumOpt50 = 0.0, sumImprv50 = 0.0, sumAtten50 = 0.0;
        for (size_t i = 0; i < bResults.size(); i++)
        {
            if (std::abs(bResults[i].rate - 0.50f) < 0.01f)
            {
                sumReg50 += bResults[i].miu_regular;
                sumOpt50 += bResults[i].miu_jitter_sa;
                sumImprv50 += bResults[i].imprv_vs_regular;
                sumAtten50 += bResults[i].alias_suppress_db;
                n50++;
            }
        }
        float miuReg50 = 0.0f, miuOpt50 = 0.0f, imprvPct50 = 0.0f;
        if (n50 > 0)
        {
            miuReg50 = (float)(sumReg50 / n50);
            miuOpt50 = (float)(sumOpt50 / n50);
            imprvPct50 = (float)(sumImprv50 / n50);
            atten_db_50 = (float)(sumAtten50 / n50);
        }

        // 真实/合成模式标记与属性指标透传，杜绝模块间模式混用
        const bool attrIsReal = !aMetrics.isSynthetic;
        const bool reconIsReal = !rMetrics.isSynthetic;

        ScientificPlotter::plotWorkflowSummaryDashboard(
            aMetrics.meanFold,
            aMetrics.foldUniformityCv,
            rMetrics.snr_improvement_db,
            atten_db_50,
            QDir(centralFiguresDir).filePath("00_research_workflow_summary.png"),
            rMetrics.snr_recovered_db,
            rMetrics.rmse,
            rMetrics.correlation,
            attrIsReal,
            aMetrics.liveBinCount,
            aMetrics.costReductionPct,
            aMetrics.minOffset,
            aMetrics.maxOffset,
            aMetrics.azimuthCoverageScore,
            miuReg50,
            miuOpt50,
            imprvPct50,
            rMetrics.snr_linear_interp_db,
            rMetrics.snr_zero_fill_db,
            reconIsReal,
            rMetrics.usedWarmStart
        );
        ScientificPlotter::exportFiguresGalleryHtml(
            centralFiguresDir,
            QDir(centralFiguresDir).filePath("index.html")
        );
    }

    QString figuresConclusion = QString::fromLocal8Bit(
        "\n========================================================================\n"
        "【全套科研成果图件集中保存与支撑清单】\n"
        "本套科研闭环对比试验生成的所有数据图与结果图，均已统一保存至文件夹：\n"
        "--> %1\n\n"
        "图件支撑清单与科学结论：\n"
        "1. [00_research_workflow_summary.png] 全流程科研闭环综合大看板 (采样-属性-重构总览)\n"
        "2. [01_sa_convergence_curve.png] 模拟退火当前解/历史最优收敛轨迹 (历史最优单调非增)\n"
        "3. [02_sampling_benchmark_bars.png] 三种采样方案相干度对比柱状图 (Hybrid相干度衰减%2 dB)\n" // add by wushanzhou in 2026/10/23
        "4. [03_cmp_fold_distribution_map.png] CMP面元空间覆盖次数平面热力图 (展示覆盖次数在二维CMP网格上的连续分布)\n"
        "5. [04_offset_azimuth_rose_charts.png] 炮检距连续性分布直方图与方位角玫瑰图 (完整度%3)\n"
        "6. [05_wavefield_reconstruction_quad.png] 地震波场欠采样与曲波POCS恢复四联剖面 (SNR恢复达%4 dB，较线性插值提升%5 dB)\n"
        "7. [06_recon_convergence_trace_comparison.png] 曲波POCS残差收敛与抽稀道波形拟合细节 (波形相关系数r=%6)\n"
        "8. [index.html] 科研成果图件集中浏览画廊 (可直接在网页浏览器中查看)\n"
        "========================================================================\n"
    ).arg(centralFiguresDir)
     .arg(QString::number(atten_db_50, 'f', 2))
     .arg(QString::number(aMetrics.azimuthCoverageScore, 'f', 1) + "%")
     .arg(QString::number(rMetrics.snr_recovered_db, 'f', 2))
     .arg(QString::number(rMetrics.snr_imprv_vs_linear_db, 'f', 2))
     .arg(QString::number(rMetrics.correlation, 'f', 4));

    fullReportSummary = statusHeader + subStatus +
                        s1 + "\n----------------------------------------\n" +
                        s2 + "\n----------------------------------------\n" +
                        s3 + figuresConclusion;

    return allSuccess;
}

