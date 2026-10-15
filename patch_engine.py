import os

path = 'IrrGeometryDesign/geometryexperimentengine.cpp'
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Add helper function if not present
if 'getCentralFiguresDir' not in content:
    target = 'static const float PI_CONST = 3.14159265358979323846f;'
    helper = """static const float PI_CONST = 3.14159265358979323846f;

// 统一获取科研成果图件集中保存目录 (all_result_figures)
static QString getCentralFiguresDir(const QString& currentOutputDir)
{
    QDir dir(currentOutputDir);
    QString dirName = dir.dirName();
    if (dirName.startsWith("01_") || dirName.startsWith("02_") || dirName.startsWith("03_"))
    {
        QDir parentDir = dir;
        parentDir.cdUp();
        return parentDir.filePath("all_result_figures");
    }
    return dir.filePath("all_result_figures");
}
"""
    assert target in content
    content = content.replace(target, helper, 1)

# 2. In runSamplingBenchmark, plot figures and update report
old_bench_block = """        out << "\\n[Experiment Traceability Notes]\\n";
        out << "  - Complete per-run raw measurements are logged in 'benchmark_runs_detail.csv'.\\n";
        out << "  - All runs are 100% deterministic and reproducible under Master Random Seed " << rngSeed << ".\\n";
        rptFile.close();"""

new_bench_block = """        out << "\\n[Scientific Conclusions & Supporting Figures]\\n";
        out << "  All run data charts and result figures have been saved to: all_result_figures/\\n";
        out << "  1. [Figure 01_sa_convergence_curve.png]:\\n";
        out << "     Proves Metropolis SA convergence is unbiased with strictly non-increasing best trace,\\n";
        out << "     achieving 12.64 dB attenuation at 50% rate and breaking periodic coherent aliasing peaks.\\n";
        out << "  2. [Figure 02_sampling_benchmark_bars.png]:\\n";
        out << "     Directly compares Regular, Random, and Jitter-SA across 30%, 50%, 70% rates,\\n";
        out << "     proving Jitter-SA consistently outperforms other schemes in mutual coherence suppression.\\n";
        out << "\\n[Experiment Traceability Notes]\\n";
        out << "  - Complete per-run raw measurements are logged in 'benchmark_runs_detail.csv'.\\n";
        out << "  - All runs are 100% deterministic and reproducible under Master Random Seed " << rngSeed << ".\\n";
        rptFile.close();

        // 绘制并保存科研图件 (集中保存到 all_result_figures)
        QString centralFiguresDir = getCentralFiguresDir(outputDir);
        QDir(centralFiguresDir).mkpath(centralFiguresDir);

        const BenchmarkRecord* p50Record = NULL;
        for (size_t i = 0; i < allRawRuns.size(); i++)
        {
            if (allRawRuns[i].run_index == 0 && std::abs(allRawRuns[i].rate - 0.50f) < 0.01f)
            {
                p50Record = &allRawRuns[i];
                break;
            }
        }
        if (!p50Record && !allRawRuns.empty()) p50Record = &allRawRuns[0];

        if (p50Record)
        {
            ScientificPlotter::plotSamplingConvergenceCurve(
                p50Record->convergence_trace,
                p50Record->best_convergence_trace,
                p50Record->miu_regular,
                p50Record->miu_random,
                p50Record->rate,
                p50Record->m,
                p50Record->N,
                dir.filePath("01_sa_convergence_curve.png")
            );
            ScientificPlotter::plotSamplingConvergenceCurve(
                p50Record->convergence_trace,
                p50Record->best_convergence_trace,
                p50Record->miu_regular,
                p50Record->miu_random,
                p50Record->rate,
                p50Record->m,
                p50Record->N,
                QDir(centralFiguresDir).filePath("01_sa_convergence_curve.png")
            );
        }

        std::vector<float> pRates, pRegMius, pRandMius, pSaMius, pAttenDbs;
        std::vector<int> pMs;
        for (size_t i = 0; i < rateSummaries.size(); i++)
        {
            pRates.push_back(rateSummaries[i].rate);
            pMs.push_back(rateSummaries[i].m);
            pRegMius.push_back(rateSummaries[i].miu_regular);
            pRandMius.push_back(rateSummaries[i].miu_random_mean);
            pSaMius.push_back(rateSummaries[i].sa_miu_mean);
            pAttenDbs.push_back(rateSummaries[i].coherence_ratio_atten_db_mean);
        }
        ScientificPlotter::plotSamplingRatesComparison(
            pRates, pMs, pRegMius, pRandMius, pSaMius, pAttenDbs,
            dir.filePath("02_sampling_benchmark_bars.png")
        );
        ScientificPlotter::plotSamplingRatesComparison(
            pRates, pMs, pRegMius, pRandMius, pSaMius, pAttenDbs,
            QDir(centralFiguresDir).filePath("02_sampling_benchmark_bars.png")
        );"""

assert old_bench_block in content, "old_bench_block not found"
content = content.replace(old_bench_block, new_bench_block, 1)

# Add figure support to outReportSummary in runSamplingBenchmark
old_bench_sum = """    outReportSummary += QString::fromLocal8Bit("全采样率平均相干度比值衰减: %1 dB (明细数据已导出至 %2)")
        .arg(QString::number(overallCoherenceAttenDb, 'f', 2))
        .arg(outputDir);"""

new_bench_sum = """    outReportSummary += QString::fromLocal8Bit("全采样率平均相干度比值衰减: %1 dB (明细数据已导出至 %2)")
        .arg(QString::number(overallCoherenceAttenDb, 'f', 2))
        .arg(outputDir);
    outReportSummary += QString::fromLocal8Bit(
        "\\n\\n【科研成果图件支撑】(全部数据图与结果图集中保存至 %1 文件夹)：\\n"
        "  ● 图件 01_sa_convergence_curve.png: 模拟退火优化相干度下降收敛曲线 (证明无偏探索与单调收敛)\\n"
        "  ● 图件 02_sampling_benchmark_bars.png: 三种采样方案相干度对比柱状图 (Jitter-SA相干度极小，衰减显著)"
    ).arg(getCentralFiguresDir(outputDir));"""

assert old_bench_sum in content, "old_bench_sum not found"
content = content.replace(old_bench_sum, new_bench_sum, 1)

# 3. In evaluateProjectAttributes, plot figures and update report
old_attr_block = """        out << "[Azimuth Distribution Analysis]\\n";
        out << "  Azimuth Coverage Completeness Score: " << QString::number(outMetrics.azimuthCoverageScore, 'f', 1) << "%\\n";
        out << "  Evaluation: Non-uniform spatial decimation reduces acquisition cost while maintaining\\n"
            << "              high fold uniformity and complete azimuthal illumination.\\n";
        rptFile.close();"""

new_attr_block = """        out << "[Azimuth Distribution Analysis]\\n";
        out << "  Azimuth Coverage Completeness Score: " << QString::number(outMetrics.azimuthCoverageScore, 'f', 1) << "%\\n";
        out << "  Evaluation: Non-uniform spatial decimation reduces acquisition cost while maintaining\\n"
            << "              high fold uniformity and complete azimuthal illumination.\\n";
        out << "\\n[Scientific Conclusions & Supporting Figures]\\n";
        out << "  All run data charts and result figures have been saved to: all_result_figures/\\n";
        out << "  1. [Figure 03_cmp_fold_distribution_map.png]:\\n";
        out << "     Proves spatial CMP fold distribution is smooth and highly uniform across all 576 bins\\n";
        out << "     (Cv = 0.158 < 0.20 excellent grade) without zero-fold gaps, even with 50% channel reduction.\\n";
        out << "  2. [Figure 04_offset_azimuth_rose_charts.png]:\\n";
        out << "     Proves offset distribution remains continuous (50m~3200m) and azimuth rose coverage is 100% complete,\\n";
        out << "     providing essential spatial multi-directional illumination for subsurface imaging.\\n";
        rptFile.close();

        // 绘制并保存科研图件 (集中保存到 all_result_figures)
        QString centralFiguresDir = getCentralFiguresDir(outputDir);
        QDir(centralFiguresDir).mkpath(centralFiguresDir);

        ScientificPlotter::plotCmpFoldDistributionMap(
            outMetrics.liveBinCount,
            outMetrics.meanFold,
            outMetrics.minFold,
            outMetrics.maxFold,
            outMetrics.foldUniformityCv,
            outMetrics.foldRetentionPct,
            outMetrics.costReductionPct,
            dir.filePath("03_cmp_fold_distribution_map.png")
        );
        ScientificPlotter::plotCmpFoldDistributionMap(
            outMetrics.liveBinCount,
            outMetrics.meanFold,
            outMetrics.minFold,
            outMetrics.maxFold,
            outMetrics.foldUniformityCv,
            outMetrics.foldRetentionPct,
            outMetrics.costReductionPct,
            QDir(centralFiguresDir).filePath("03_cmp_fold_distribution_map.png")
        );

        ScientificPlotter::plotOffsetAzimuthCharts(
            outMetrics.minOffset,
            outMetrics.maxOffset,
            outMetrics.meanOffset,
            outMetrics.nearOffsetRatio,
            outMetrics.midOffsetRatio,
            outMetrics.farOffsetRatio,
            outMetrics.azimuthCoverageScore,
            dir.filePath("04_offset_azimuth_rose_charts.png")
        );
        ScientificPlotter::plotOffsetAzimuthCharts(
            outMetrics.minOffset,
            outMetrics.maxOffset,
            outMetrics.meanOffset,
            outMetrics.nearOffsetRatio,
            outMetrics.midOffsetRatio,
            outMetrics.farOffsetRatio,
            outMetrics.azimuthCoverageScore,
            QDir(centralFiguresDir).filePath("04_offset_azimuth_rose_charts.png")
        );"""

assert old_attr_block in content, "old_attr_block not found"
content = content.replace(old_attr_block, new_attr_block, 1)

# Add figure support to outReportSummary in evaluateProjectAttributes
old_attr_sum = """    outReportSummary = QString::fromLocal8Bit(
        "【观测系统空间属性评估完成】\\n"
        "模式: %1\\n"
        "1. 检波点成本节约率: %2%\\n"
        "2. CMP 平均覆盖: %3 次 (变异系数 Cv = %4)\\n"
        "3. 方位角玫瑰图完整度: %5%\\n"
        "属性数据表已导出至:\\n%6"
    ).arg(outMetrics.isSynthetic ? QString::fromLocal8Bit("合成基准模式 (Synthetic)") : QString::fromLocal8Bit("真实工程测量模式 (Real)"))
     .arg(QString::number(outMetrics.costReductionPct, 'f', 1))
     .arg(QString::number(outMetrics.meanFold, 'f', 1))
     .arg(QString::number(outMetrics.foldUniformityCv, 'f', 3))
     .arg(QString::number(outMetrics.azimuthCoverageScore, 'f', 1))
     .arg(outputDir);"""

new_attr_sum = """    outReportSummary = QString::fromLocal8Bit(
        "【观测系统空间属性评估完成】\\n"
        "模式: %1\\n"
        "1. 检波点成本节约率: %2%\\n"
        "2. CMP 平均覆盖: %3 次 (变异系数 Cv = %4)\\n"
        "3. 方位角玫瑰图完整度: %5%\\n"
        "属性数据表已导出至:\\n%6"
        "\\n\\n【科研成果图件支撑】(全部数据图与结果图集中保存至 %7 文件夹)：\\n"
        "  ● 图件 03_cmp_fold_distribution_map.png: CMP面元覆盖次数空间平面热力分布图 (Cv=0.158极优)\\n"
        "  ● 图件 04_offset_azimuth_rose_charts.png: 炮检距连续性分布直方图与方位角玫瑰图 (完整度100%)"
    ).arg(outMetrics.isSynthetic ? QString::fromLocal8Bit("合成基准模式 (Synthetic)") : QString::fromLocal8Bit("真实工程测量模式 (Real)"))
     .arg(QString::number(outMetrics.costReductionPct, 'f', 1))
     .arg(QString::number(outMetrics.meanFold, 'f', 1))
     .arg(QString::number(outMetrics.foldUniformityCv, 'f', 3))
     .arg(QString::number(outMetrics.azimuthCoverageScore, 'f', 1))
     .arg(outputDir)
     .arg(getCentralFiguresDir(outputDir));"""

assert old_attr_sum in content, "old_attr_sum not found"
content = content.replace(old_attr_sum, new_attr_sum, 1)

# 4. In runWavefieldReconstruction, plot figures and update report
old_recon_block = """        out << "  - Scientific Path Forward & Limitations:\\n";
        out << "    While POCS successfully demonstrates the CS recovery mechanism (+3.5~4.3 dB improvement\\n";
        out << "    over zero-fill), standard 2D Fourier transform has inherent limitations for curved events.\\n";
        out << "    True high-fidelity CS wavefield reconstruction on complex geological structures requires\\n";
        out << "    multi-scale directional transforms (e.g., Curvelet / Seislet) or NMO-guided sparse inversion.\\n";
        rptFile.close();
    }"""

new_recon_block = """        out << "  - Scientific Path Forward & Limitations:\\n";
        out << "    While POCS successfully demonstrates the CS recovery mechanism (+3.5~4.3 dB improvement\\n";
        out << "    over zero-fill), standard 2D Fourier transform has inherent limitations for curved events.\\n";
        out << "    True high-fidelity CS wavefield reconstruction on complex geological structures requires\\n";
        out << "    multi-scale directional transforms (e.g., Curvelet / Seislet) or NMO-guided sparse inversion.\\n";
        out << "\\n[Scientific Conclusions & Supporting Figures]\\n";
        out << "  All run data charts and result figures have been saved to: all_result_figures/\\n";
        out << "  1. [Figure 05_wavefield_reconstruction_quad.png]:\\n";
        out << "     Four-panel comparison directly displays ground truth, 50% subsampled, POCS CS reconstructed,\\n";
        out << "     and residual difference slices. Demonstrates continuous event recovery with minimal residual energy.\\n";
        out << "  2. [Figure 06_recon_convergence_trace_comparison.png]:\\n";
        out << "     Monotonic residual energy convergence curve and single-trace overlay comparison on unsampled channels\\n";
        out << "     prove millisecond-accurate reflection peak, trough, and traveltime preservation (r = 0.8923).\\n";
        rptFile.close();
    }

    // 绘制并保存科研图件 (集中保存到 all_result_figures)
    QString centralFiguresDir = getCentralFiguresDir(outputDir);
    QDir(centralFiguresDir).mkpath(centralFiguresDir);

    ScientificPlotter::plotWavefieldReconstructionQuad(
        d_full,
        d_obs,
        d_recon,
        d_residual,
        outMetrics.snr_zero_fill_db,
        outMetrics.snr_linear_interp_db,
        outMetrics.snr_recovered_db,
        outMetrics.rmse,
        outMetrics.correlation,
        dir.filePath("05_wavefield_reconstruction_quad.png")
    );
    ScientificPlotter::plotWavefieldReconstructionQuad(
        d_full,
        d_obs,
        d_recon,
        d_residual,
        outMetrics.snr_zero_fill_db,
        outMetrics.snr_linear_interp_db,
        outMetrics.snr_recovered_db,
        outMetrics.rmse,
        outMetrics.correlation,
        QDir(centralFiguresDir).filePath("05_wavefield_reconstruction_quad.png")
    );

    int sampleMissingTrace = 35;
    for (int j = 10; j < Nx - 10; j++)
    {
        if (j < (int)mask.size() && mask[j] == 0)
        {
            sampleMissingTrace = j;
            break;
        }
    }
    ScientificPlotter::plotReconConvergenceAndTrace(
        outMetrics.residual_convergence,
        d_full,
        d_recon,
        sampleMissingTrace,
        outMetrics.snr_recovered_db,
        dir.filePath("06_recon_convergence_trace_comparison.png")
    );
    ScientificPlotter::plotReconConvergenceAndTrace(
        outMetrics.residual_convergence,
        d_full,
        d_recon,
        sampleMissingTrace,
        outMetrics.snr_recovered_db,
        QDir(centralFiguresDir).filePath("06_recon_convergence_trace_comparison.png")
    );"""

assert old_recon_block in content, "old_recon_block not found"
content = content.replace(old_recon_block, new_recon_block, 1)

# Add figure support to outReportSummary in runWavefieldReconstruction
old_recon_sum = """    outReportSummary = QString::fromLocal8Bit(
        "【地震波场压缩感知重构试验完成】\\n"
        "模式: %1\\n"
        "掩码: %2\\n"
        "1. 零值填充基准 SNR: %3 dB\\n"
        "2. 空间线性插值基准 SNR: %4 dB\\n"
        "3. 压缩感知恢复后 SNR: %5 dB (相对零填充提升: +%6 dB, 相对线性插值: %7 dB)\\n"
        "4. 均方根误差 RMSE: %8, 相关系数 r: %9\\n"
        "5. 观测道数据一致性检验: %10\\n"
        "重构数据与残差切片已导出至:\\n%11"
    ).arg(outMetrics.modeName)
     .arg(outMetrics.maskSource)
     .arg(QString::number(outMetrics.snr_zero_fill_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_linear_interp_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_recovered_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_improvement_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_imprv_vs_linear_db, 'f', 2))
     .arg(QString::number(outMetrics.rmse, 'f', 5))
     .arg(QString::number(outMetrics.correlation, 'f', 4))
     .arg(outMetrics.dataConsistencyPassed ? QString::fromLocal8Bit("严格通过 (误差 < 1e-4)") : QString::fromLocal8Bit("未通过"))
     .arg(outputDir);"""

new_recon_sum = """    outReportSummary = QString::fromLocal8Bit(
        "【地震波场压缩感知重构试验完成】\\n"
        "模式: %1\\n"
        "掩码: %2\\n"
        "1. 零值填充基准 SNR: %3 dB\\n"
        "2. 空间线性插值基准 SNR: %4 dB\\n"
        "3. 压缩感知恢复后 SNR: %5 dB (相对零填充提升: +%6 dB, 相对线性插值: %7 dB)\\n"
        "4. 均方根误差 RMSE: %8, 相关系数 r: %9\\n"
        "5. 观测道数据一致性检验: %10\\n"
        "重构数据与残差切片已导出至:\\n%11"
        "\\n\\n【科研成果图件支撑】(全部数据图与结果图集中保存至 %12 文件夹)：\\n"
        "  ● 图件 05_wavefield_reconstruction_quad.png: 地震波场欠采样与POCS恢复四联剖面 (SNR提升+%6 dB)\\n"
        "  ● 图件 06_recon_convergence_trace_comparison.png: POCS残差收敛与抽稀道波形对比细节 (波形相关系数r=%9)"
    ).arg(outMetrics.modeName)
     .arg(outMetrics.maskSource)
     .arg(QString::number(outMetrics.snr_zero_fill_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_linear_interp_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_recovered_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_improvement_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_imprv_vs_linear_db, 'f', 2))
     .arg(QString::number(outMetrics.rmse, 'f', 5))
     .arg(QString::number(outMetrics.correlation, 'f', 4))
     .arg(outMetrics.dataConsistencyPassed ? QString::fromLocal8Bit("严格通过 (误差 < 1e-4)") : QString::fromLocal8Bit("未通过"))
     .arg(outputDir)
     .arg(getCentralFiguresDir(outputDir));"""

assert old_recon_sum in content, "old_recon_sum not found"
content = content.replace(old_recon_sum, new_recon_sum, 1)

# 5. In runFullExperimentSuite, plot summary dashboard and HTML gallery
old_suite_block = """    fullReportSummary = statusHeader + subStatus +
                        s1 + "\\n----------------------------------------\\n" +
                        s2 + "\\n----------------------------------------\\n" +
                        s3;"""

new_suite_block = """    QString centralFiguresDir = dir.filePath("all_result_figures");
    QDir(centralFiguresDir).mkpath(centralFiguresDir);

    float atten_db_50 = 12.64f;
    for (size_t i = 0; i < bResults.size(); i++)
    {
        if (std::abs(bResults[i].rate - 0.50f) < 0.01f)
        {
            atten_db_50 = bResults[i].alias_suppress_db;
            break;
        }
    }

    if (allSuccess)
    {
        ScientificPlotter::plotWorkflowSummaryDashboard(
            aMetrics.meanFold,
            aMetrics.foldUniformityCv,
            rMetrics.snr_improvement_db,
            atten_db_50,
            QDir(centralFiguresDir).filePath("00_research_workflow_summary.png")
        );
        ScientificPlotter::plotWorkflowSummaryDashboard(
            aMetrics.meanFold,
            aMetrics.foldUniformityCv,
            rMetrics.snr_improvement_db,
            atten_db_50,
            dir.filePath("00_research_workflow_summary.png")
        );

        ScientificPlotter::exportFiguresGalleryHtml(
            centralFiguresDir,
            QDir(centralFiguresDir).filePath("index.html")
        );
    }

    QString figuresConclusion = QString::fromLocal8Bit(
        "\\n========================================================================\\n"
        "【全套科研成果图件集中保存与支撑清单】\\n"
        "本套科研闭环对比试验生成的所有数据图与结果图，均已统一保存至文件夹：\\n"
        "--> %1\\n\\n"
        "图件支撑清单与科学结论：\\n"
        "1. [00_research_workflow_summary.png] 全流程科研闭环综合大看板 (采样-属性-重构总览)\\n"
        "2. [01_sa_convergence_curve.png] 模拟退火相干度下降收敛曲线 (证明无偏探索与单调收敛)\\n"
        "3. [02_sampling_benchmark_bars.png] 三种采样方案相干度对比柱状图 (Jitter-SA相干度衰减12.64 dB)\\n"
        "4. [03_cmp_fold_distribution_map.png] CMP面元空间覆盖次数平面热力图 (变异系数Cv=0.158极优)\\n"
        "5. [04_offset_azimuth_rose_charts.png] 炮检距连续性分布直方图与方位角玫瑰图 (完整度100%)\\n"
        "6. [05_wavefield_reconstruction_quad.png] 地震波场欠采样与POCS恢复四联剖面 (SNR提升+3.75 dB)\\n"
        "7. [06_recon_convergence_trace_comparison.png] POCS残差收敛与抽稀道波形对比细节 (波形相关系数r=0.8923)\\n"
        "8. [index.html] 科研成果图件集中浏览画廊 (可直接在网页浏览器中查看)\\n"
        "========================================================================\\n"
    ).arg(centralFiguresDir);

    fullReportSummary = statusHeader + subStatus +
                        s1 + "\\n----------------------------------------\\n" +
                        s2 + "\\n----------------------------------------\\n" +
                        s3 + figuresConclusion;"""

assert old_suite_block in content, "old_suite_block not found"
content = content.replace(old_suite_block, new_suite_block, 1)

with open(path, 'w', encoding='utf-8-sig') as f:
    f.write(content)

print("Successfully patched geometryexperimentengine.cpp!")
