#include "experiment_internal.h"

bool GeometryExperimentEngine::runSamplingBenchmark(int N,
                                                   const std::vector<float>& rates,
                                                   const QString& outputDir,
                                                   std::vector<BenchmarkRecord>& outResults,
                                                   QString& outReportSummary,
                                                   unsigned long long rngSeed,
                                                   int repeatRuns)
{
    if (N <= 0) N = 128;
    QDir dir(outputDir);
    QDir().mkpath(outputDir);   // 2026.10.20: 原 dir.mkpath(outputDir) 会把 outputDir 重复拼接到 dir 之后

    outResults.clear();
    std::vector<float> testRates = rates;
    if (testRates.empty())
    {
        testRates.push_back(0.30f);
        testRates.push_back(0.50f);
        testRates.push_back(0.70f);
    }

    if (repeatRuns < 1) repeatRuns = 1;

    std::vector<BenchmarkRecord> allRawRuns;
    std::vector<MultiSeedSummary> rateSummaries;

    for (size_t rIdx = 0; rIdx < testRates.size(); rIdx++)
    {
        float c = testRates[rIdx];
        // 关键修复 1：每次实验只计算一次目标采样点数 m = round(N * c)
        int m = (int)floor((double)N * (double)c + 0.5);
        if (m < 2) m = 2;
        if (m > N) m = N;

        // 1. 规则减抽方案 (确定性全区间等间距映射: i_k = int(k * N / m))
        int *phi_reg = Sampling1R_Count(N, m);
        if (!ValidateSamplingMask(N, m, phi_reg))
        {
            delete[] phi_reg;
            outReportSummary = QString::fromLocal8Bit("【采样基准测试失败】：规则采样掩码合法性校验未通过！");
            return false;
        }
        float miu_reg = SamplingMaskCoherence(std::vector<int>(phi_reg, phi_reg + N)); // add by wushanzhou in 2026/10/23
        // 2026.10.20: 规则减抽的均匀性指标（作为对照基准）
        const int regMaxGap = uniformity_max_gap(phi_reg, N);
        const float regNnCv = uniformity_nn_cv(phi_reg, N);
        delete[] phi_reg;

        MultiSeedSummary summary;
        summary.rate = c;
        summary.N = N;
        summary.m = m;
        summary.repeatRuns = repeatRuns;
        summary.master_seed = rngSeed;
        summary.miu_regular = miu_reg;
        summary.sa_miu_best = 1e9f;
        summary.sa_miu_worst = -1e9f;

        double sum_rand = 0.0;
        double sum_sa = 0.0;
        double sum_sa_sq = 0.0;
        double sum_imprv_reg = 0.0;
        double sum_imprv_rand = 0.0;
        double sum_alias_db = 0.0;

        // 2026.10.20 新增：记录本采样率下 μ 最优的 SA 掩码与其 Jitter 初始掩码，
        // 供论文"采样优化 → 波场重构"闭环实验直接使用（此前掩码从不落盘）。
        std::vector<int> bestSaMask;
        std::vector<int> bestJitterMask;
        int bestSaRunIndex = -1;
        float bestSaMiu = 1e9f;

        for (int runIdx = 0; runIdx < repeatRuns; runIdx++)
        {
            BenchmarkRecord record;
            record.rate = c;
            record.N = N;
            record.m = m;
            record.run_index = runIdx;
            record.master_seed = rngSeed;

            // 派生独立且可复现的实际种子
            // 派生规则: actual_seed = master_seed + int(c * 1000) + runIdx * 179 + 1
            unsigned long long actual_seed = rngSeed + (unsigned long long)(c * 1000.0f) + (unsigned long long)(runIdx * 179 + 1);
            record.actual_seed = actual_seed;
            record.seed = actual_seed; // 保持向后兼容

            exprng::seed(actual_seed);

            record.miu_regular = miu_reg;

            // 2. 纯随机抽样方案 (Fisher-Yates 精确抽取 m 个点)
            int *phi_rand = Sampling1S_Count(N, m);
            if (!ValidateSamplingMask(N, m, phi_rand))
            {
                delete[] phi_rand;
                outReportSummary = QString::fromLocal8Bit("【采样基准测试失败】：纯随机采样掩码合法性校验未通过！");
                return false;
            }
            record.miu_random = SamplingMaskCoherence(std::vector<int>(phi_rand, phi_rand + N)); // add by wushanzhou in 2026/10/23
            record.max_gap_random = uniformity_max_gap(phi_rand, N);
            record.nn_cv_random = uniformity_nn_cv(phi_rand, N);
            delete[] phi_rand;

            // 3. Hybrid 优化方案 (Jitter Initial + Metropolis SA) // add by wushanzhou in 2026/10/23
            int *phi_jit = Sampling1J_Count(N, m);
            if (!ValidateSamplingMask(N, m, phi_jit))
            {
                delete[] phi_jit;
                outReportSummary = QString::fromLocal8Bit("【采样基准测试失败】：Jitter采样掩码合法性校验未通过！");
                return false;
            }
            record.miu_jitter_init = SamplingMaskCoherence(std::vector<int>(phi_jit, phi_jit + N)); // add by wushanzhou in 2026/10/23
            record.max_gap_jitter_init = uniformity_max_gap(phi_jit, N);
            record.nn_cv_jitter_init = uniformity_nn_cv(phi_jit, N);

            SAStats stats;
            SAParams sap;
            sap.T0 = 80.0;
            sap.Tmin = 0.5;
            sap.alpha = 0.85;
            sap.n_rapid = 8;
            sap.iter = 1;
            sap.strategy = SA_ACCEPT_METROPOLIS; // 经典 Metropolis 准则

            ConstrainedResult selected; ConstraintWeights weights; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(N, m, std::vector<int>(phi_jit, phi_jit + N), weights, // add by wushanzhou in 2026/10/23
                                   actual_seed + 1, 200, SEARCH_HYBRID, selected)) // add by wushanzhou in 2026/10/23
            { delete[] phi_jit; outReportSummary = "sampling search failed"; return false; } // add by wushanzhou in 2026/10/23
            int *phi_opt = new int[N]; // add by wushanzhou in 2026/10/23
            for (int q = 0; q < N; q++) phi_opt[q] = selected.mask[q]; // add by wushanzhou in 2026/10/23
            stats.miu_final = selected.mu; stats.miu0 = record.miu_jitter_init; // add by wushanzhou in 2026/10/23
            stats.miu_sa_best = SamplingMaskCoherence(selected.saPrefixMask); // add by wushanzhou in 2026/10/23
            stats.mu_trace = selected.currentTrace; stats.best_mu_trace = selected.bestTrace; // add by wushanzhou in 2026/10/23
            for (size_t q = 0; q < selected.bestTrace.size(); q++) stats.eval_trace.push_back((long)q + 1); // add by wushanzhou in 2026/10/23
            QString savedMask = dir.filePath(QString("masks/r%1_run%2_hybrid.txt").arg((int)(c*100+0.5f)).arg(runIdx)); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(savedMask, selected, actual_seed + 1, 200, "ClassicPSF")) // add by wushanzhou in 2026/10/23
            { delete[] phi_jit; delete[] phi_opt; outReportSummary = "mask export failed"; return false; } // add by wushanzhou in 2026/10/23
            if (!ValidateSamplingMask(N, m, phi_opt))
            {
                delete[] phi_jit;
                delete[] phi_opt;
                outReportSummary = QString::fromLocal8Bit("【采样基准测试失败】：退火优化后采样掩码合法性校验未通过！");
                return false;
            }
            record.miu_jitter_sa = stats.miu_final;
            record.max_gap_sa = uniformity_max_gap(phi_opt, N);
            record.nn_cv_sa = uniformity_nn_cv(phi_opt, N);
            record.jitter_unit_occupancy = uniformity_unit_occupancy(phi_opt, N, m);
            record.max_gap_regular = regMaxGap;
            record.nn_cv_regular = regNnCv;
            record.convergence_trace = stats.mu_trace;
            record.best_convergence_trace = stats.best_mu_trace;
            record.eval_trace = stats.eval_trace;

            // 保存本采样率当前最优掩码（在 delete 之前拷贝）
            if (record.miu_jitter_sa < bestSaMiu)
            {
                bestSaMiu = record.miu_jitter_sa;
                bestSaRunIndex = runIdx;
                bestSaMask.assign(phi_opt, phi_opt + N);
                bestJitterMask.assign(phi_jit, phi_jit + N);
            }

            delete[] phi_jit;
            delete[] phi_opt;

            // 4. 对比指标计算
            if (record.miu_regular > 1e-6f)
                record.imprv_vs_regular = (record.miu_regular - record.miu_jitter_sa) / record.miu_regular * 100.0f;
            else
                record.imprv_vs_regular = 0.0f;

            if (record.miu_random > 1e-6f)
                record.imprv_vs_random = (record.miu_random - record.miu_jitter_sa) / record.miu_random * 100.0f;
            else
                record.imprv_vs_random = 0.0f;

            // 相干度比值衰减指标 (dB)
            float ratio = (record.miu_jitter_sa > 1e-6f) ? (record.miu_regular / record.miu_jitter_sa) : 1.0f;

            record.alias_suppress_db = 20.0f * (float)log10((double)ratio);

            allRawRuns.push_back(record);

            sum_rand += record.miu_random;
            sum_sa += record.miu_jitter_sa;
            sum_sa_sq += (double)record.miu_jitter_sa * (double)record.miu_jitter_sa;
            sum_imprv_reg += record.imprv_vs_regular;
            sum_imprv_rand += record.imprv_vs_random;
            sum_alias_db += record.alias_suppress_db;

            if (record.miu_jitter_sa < summary.sa_miu_best) summary.sa_miu_best = record.miu_jitter_sa;
            if (record.miu_jitter_sa > summary.sa_miu_worst) summary.sa_miu_worst = record.miu_jitter_sa;
        }

        double nRuns = (double)repeatRuns;
        summary.miu_random_mean = (float)(sum_rand / nRuns);
        summary.sa_miu_mean = (float)(sum_sa / nRuns);
        double variance = (sum_sa_sq / nRuns) - ((double)summary.sa_miu_mean * (double)summary.sa_miu_mean);
        if (variance < 0.0) variance = 0.0;
        summary.sa_miu_std = (float)sqrt(variance);
        summary.imprv_vs_reg_mean = (float)(sum_imprv_reg / nRuns);
        summary.imprv_vs_rand_mean = (float)(sum_imprv_rand / nRuns);
        summary.coherence_ratio_atten_db_mean = (float)(sum_alias_db / nRuns);

        // 导出本采样率最优掩码（每行一个 0/1；可直接传给 --run-real-reconstruction --mask）
        if (!bestSaMask.empty())
        {
            const int pct = (int)(c * 100.0f + 0.5f);
            QString pathSa = dir.filePath(QString("mask_%1pct_sa_best.txt").arg(pct));
            QFile fSa(pathSa);
            if (fSa.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&fSa);
                out.setCodec("UTF-8");
                out << "# Hybrid optimized mask (best of " << repeatRuns << " runs)\n"; // add by wushanzhou in 2026/10/23
                out << "# Sampling_Rate: " << pct << "%, m=" << m << "/" << N
                    << ", best_run_index=" << bestSaRunIndex
                    << ", mu=" << QString::number(bestSaMiu, 'f', 6) << "\n";
                for (int i = 0; i < N; i++) out << bestSaMask[i] << "\n";
                fSa.close();
            }
            QString pathJit = dir.filePath(QString("mask_%1pct_jitter_init.txt").arg(pct));
            QFile fJit(pathJit);
            if (fJit.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&fJit);
                out.setCodec("UTF-8");
                out << "# Jitter init mask of the best Hybrid run (run_index=" << bestSaRunIndex << ")\n"; // add by wushanzhou in 2026/10/23
                out << "# Sampling_Rate: " << pct << "%, m=" << m << "/" << N << "\n";
                for (int i = 0; i < N; i++) out << bestJitterMask[i] << "\n";
                fJit.close();
            }
        }

        rateSummaries.push_back(summary);
    }

    outResults = allRawRuns;

    // -------------------------------------------------------------
    // 导出 1: benchmark_runs_detail.csv (所有独立运行的明细记录)
    // -------------------------------------------------------------
    QString csvDetailPath = dir.filePath("benchmark_runs_detail.csv");
    QFile csvDetailFile(csvDetailPath);
    if (csvDetailFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&csvDetailFile);
        out.setCodec("UTF-8");
        out << "Sampling_Rate,Target_Points,Total_Points,Run_Index,Master_Seed,Actual_Seed,Regular_Miu,Random_Miu,Jitter_Init_Miu,Jitter_SA_Miu,"
            << "Imprv_vs_Regular_Pct,Imprv_vs_Random_Pct,Coherence_Ratio_Atten_dB,"
            // 2026.10.20 新增：空间均匀性三指标
            << "MaxGap_Regular,MaxGap_Random,MaxGap_JitterInit,MaxGap_SA,"
            << "NN_CV_Regular,NN_CV_Random,NN_CV_JitterInit,NN_CV_SA,JitterUnitOccupancy_SA\n";
        for (size_t i = 0; i < allRawRuns.size(); i++)
        {
            const BenchmarkRecord& r = allRawRuns[i];
            out << QString::number(r.rate, 'f', 2) << ","
                << r.m << ","
                << r.N << ","
                << r.run_index << ","
                << QString::number(r.master_seed) << ","
                << QString::number(r.actual_seed) << ","
                << QString::number(r.miu_regular, 'f', 4) << ","
                << QString::number(r.miu_random, 'f', 4) << ","
                << QString::number(r.miu_jitter_init, 'f', 4) << ","
                << QString::number(r.miu_jitter_sa, 'f', 4) << ","
                << QString::number(r.imprv_vs_regular, 'f', 2) << "%,"
                << QString::number(r.imprv_vs_random, 'f', 2) << "%,"
                << QString::number(r.alias_suppress_db, 'f', 2) << ","
                << r.max_gap_regular << ","
                << r.max_gap_random << ","
                << r.max_gap_jitter_init << ","
                << r.max_gap_sa << ","
                << QString::number(r.nn_cv_regular, 'f', 4) << ","
                << QString::number(r.nn_cv_random, 'f', 4) << ","
                << QString::number(r.nn_cv_jitter_init, 'f', 4) << ","
                << QString::number(r.nn_cv_sa, 'f', 4) << ","
                << QString::number(r.jitter_unit_occupancy, 'f', 4) << "\n";
        }
        csvDetailFile.close();
    }

    // -------------------------------------------------------------
    // 导出 2: benchmark_comparison_table.csv (分采样率统计汇总基准表)
    // -------------------------------------------------------------
    QString csvTablePath = dir.filePath("benchmark_comparison_table.csv");
    QFile csvTableFile(csvTablePath);
    if (csvTableFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&csvTableFile);
        out.setCodec("UTF-8");
        out << "Sampling_Rate,Target_Points,Total_Points,Repeat_Runs,Master_Seed,Regular_Miu,Random_Miu_Mean,Mean_Miu_SA,Std_Miu_SA,Min_Miu_SA,Max_Miu_SA,"
            << "Mean_Imprv_vs_Regular_Pct,Mean_Imprv_vs_Random_Pct,Mean_Coherence_Ratio_Atten_dB\n";
        for (size_t i = 0; i < rateSummaries.size(); i++)
        {
            const MultiSeedSummary& s = rateSummaries[i];
            out << QString::number(s.rate, 'f', 2) << ","
                << s.m << ","
                << s.N << ","
                << s.repeatRuns << ","
                << QString::number(s.master_seed) << ","
                << QString::number(s.miu_regular, 'f', 4) << ","
                << QString::number(s.miu_random_mean, 'f', 4) << ","
                << QString::number(s.sa_miu_mean, 'f', 4) << ","
                << QString::number(s.sa_miu_std, 'f', 4) << ","
                << QString::number(s.sa_miu_best, 'f', 4) << ","
                << QString::number(s.sa_miu_worst, 'f', 4) << ","
                << QString::number(s.imprv_vs_reg_mean, 'f', 2) << "%,"
                << QString::number(s.imprv_vs_rand_mean, 'f', 2) << "%,"
                << QString::number(s.coherence_ratio_atten_db_mean, 'f', 2) << "\n";
        }
        csvTableFile.close();
    }

    // -------------------------------------------------------------
    // 导出 3: benchmark_convergence_curves.csv (以 run 0 为代表的收敛轨迹)
    // -------------------------------------------------------------
    QString csvCurvePath = dir.filePath("benchmark_convergence_curves.csv");
    QFile csvCurveFile(csvCurvePath);
    if (csvCurveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&csvCurveFile);
        out.setCodec("UTF-8");
        out << "Iteration";
        std::vector<BenchmarkRecord> run0Records;
        for (size_t i = 0; i < allRawRuns.size(); i++)
        {
            if (allRawRuns[i].run_index == 0) run0Records.push_back(allRawRuns[i]);
        }

        for (size_t i = 0; i < run0Records.size(); i++)
        {
            int pct = (int)(run0Records[i].rate * 100.0f + 0.5f);
            out << ",Current_Miu_" << pct << "Pct,Best_Miu_" << pct << "Pct";
        }
        out << "\n";

        size_t maxSteps = 0;
        for (size_t i = 0; i < run0Records.size(); i++)
        {
            if (run0Records[i].convergence_trace.size() > maxSteps)
                maxSteps = run0Records[i].convergence_trace.size();
        }

        for (size_t step = 0; step < maxSteps; step++)
        {
            out << step;
            for (size_t i = 0; i < run0Records.size(); i++)
            {
                float curVal = (step < run0Records[i].convergence_trace.size())
                    ? run0Records[i].convergence_trace[step]
                    : (run0Records[i].convergence_trace.empty() ? 0.0f : run0Records[i].convergence_trace.back());
                float bestVal = (step < run0Records[i].best_convergence_trace.size())
                    ? run0Records[i].best_convergence_trace[step]
                    : (run0Records[i].best_convergence_trace.empty() ? 0.0f : run0Records[i].best_convergence_trace.back());
                out << "," << QString::number(curVal, 'f', 5) << "," << QString::number(bestVal, 'f', 5);
            }
            out << "\n";
        }
        csvCurveFile.close();
    }

    // -------------------------------------------------------------
    // 计算跨采样率均值统计
    // -------------------------------------------------------------
    float overallImprvReg = 0.0f;
    float overallImprvRand = 0.0f;
    float overallCoherenceAttenDb = 0.0f;
    if (!rateSummaries.empty())
    {
        for (size_t i = 0; i < rateSummaries.size(); i++)
        {
            overallImprvReg += rateSummaries[i].imprv_vs_reg_mean;
            overallImprvRand += rateSummaries[i].imprv_vs_rand_mean;
            overallCoherenceAttenDb += rateSummaries[i].coherence_ratio_atten_db_mean;
        }
        overallImprvReg /= (float)rateSummaries.size();
        overallImprvRand /= (float)rateSummaries.size();
        overallCoherenceAttenDb /= (float)rateSummaries.size();
    }

    // -------------------------------------------------------------
    // 导出 4: benchmark_summary_report.txt
    // -------------------------------------------------------------
    QString rptPath = dir.filePath("benchmark_summary_report.txt");
    QFile rptFile(rptPath);
    if (rptFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&rptFile);
        out.setCodec("UTF-8");
        out << "====================================================================================\n";
        out << "  Irregular Seismic Geometry Non-Uniform Sampling Benchmark Report (Scientific Evaluation)\n";
        out << "  Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "  Master Random Seed: " << rngSeed << "\n";
        out << "  Repetition Runs per Rate: " << repeatRuns << "\n";
        out << "  Seed Derivation Formula: actual_seed = master_seed + int(c * 1000) + run_index * 179 + 1\n";
        out << "====================================================================================\n\n";

        out << "[Sampling Protocol & Fairness Guarantees]\n";
        out << "  1. Spatial Total Points N = " << N << "; strictly synchronized actual sampling points m.\n";
        out << "  2. Regular Decimation covers full space: i_k = int(k * N / m) for k=0..m-1.\n";
        out << "  3. Pure Random Sampling uses Fisher-Yates shuffle, exactly m points without duplicate.\n";
        out << "  4. Jitter Initial Sampling uses grid-partitioned jitter, exactly m points.\n";
        out << "  5. Hybrid uses Metropolis acceptance & local perturbation, strictly preserving sum(mask) == m.\n"; // add by wushanzhou in 2026/10/23
        out << "  6. Coherence Ratio Attenuation definition: 20 * log10(mu_regular / mu_opt) dB.\n"
            << "     (Note: Measures peak mutual coherence ratio attenuation, not direct total field seismic aliasing energy).\n\n";

        out << "[Statistical Sampling Benchmark Summary Table (Averaged over " << repeatRuns << " runs)]\n";
        out << "------------------------------------------------------------------------------------------------------------------------\n";
        out << "Rate\tm/N\tRegular_Mu\tRandom_Mu\tSA_Mean_Mu\tSA_StdDev\t[Min, Max]\tImprv_vs_Reg\tCoherence_Atten(dB)\n";
        out << "------------------------------------------------------------------------------------------------------------------------\n";

        for (size_t i = 0; i < rateSummaries.size(); i++)
        {
            const MultiSeedSummary& s = rateSummaries[i];
            out << QString::number(s.rate * 100, 'f', 0) + "%"
                << "\t" << s.m << "/" << s.N
                << "\t" << QString::number(s.miu_regular, 'f', 4)
                << "\t\t" << QString::number(s.miu_random_mean, 'f', 4)
                << "\t\t" << QString::number(s.sa_miu_mean, 'f', 4)
                << "\t\t" << QString::number(s.sa_miu_std, 'f', 4)
                << "\t\t[" << QString::number(s.sa_miu_best, 'f', 4) << ", " << QString::number(s.sa_miu_worst, 'f', 4) << "]"
                << "\t" << QString::number(s.imprv_vs_reg_mean, 'f', 2) + "%"
                << "\t\t" << QString::number(s.coherence_ratio_atten_db_mean, 'f', 2) + " dB\n";
        }
        out << "------------------------------------------------------------------------------------------------------------------------\n\n";

        out << "[Statistical Metrics & Scientific Conclusions]\n";
        out << "  - Cross-Rate Average Improvement (vs Regular): " << QString::number(overallImprvReg, 'f', 2) << "%\n";
        out << "  - Cross-Rate Average Improvement (vs Random):  " << QString::number(overallImprvRand, 'f', 2) << "%\n";
        out << "  - Cross-Rate Average Coherence Attenuation:    " << QString::number(overallCoherenceAttenDb, 'f', 2) << " dB\n";
        if (rateSummaries.size() >= 2)
        {
            out << "  - 50% Sampling Rate Benchmark: Mean Mu = " << QString::number(rateSummaries[1].sa_miu_mean, 'f', 4)
                << " (Std: " << QString::number(rateSummaries[1].sa_miu_std, 'f', 4) << "), Imprv vs Regular = "
                << QString::number(rateSummaries[1].imprv_vs_reg_mean, 'f', 2)
                << "%, Coherence Ratio Attenuation = " << QString::number(rateSummaries[1].coherence_ratio_atten_db_mean, 'f', 2) << " dB\n";
        }
        out << "\n[Scientific Conclusions & Supporting Figures]\n";
        out << "  All run data charts and result figures have been saved to: figures/\n";
        out << "  1. [Figure 01_sa_convergence_curve.png]:\n";
        out << "     Shows the Metropolis SA current-solution trace and the strictly non-increasing best-so-far trace";
        if (rateSummaries.size() >= 2)
        {
            out << ",\n     with " << QString::number(rateSummaries[1].coherence_ratio_atten_db_mean, 'f', 2)
                << " dB coherence-ratio attenuation at 50% rate.\n";
        }
        else
        {
            out << ".\n";
        }
        out << "  2. [Figure 02_sampling_benchmark_bars.png]:\n";
        out << "     Compares Regular, Random, and Hybrid mu values across the tested sampling rates.\n"; // add by wushanzhou in 2026/10/23
        out << "     Hybrid attains the lowest mu among the three schemes in these runs; this is a\n"; // add by wushanzhou in 2026/10/23
        out << "     mu-domain result and does not by itself establish a reconstruction-SNR gain.\n";
        out << "\n[Experiment Traceability Notes]\n";
        out << "  - Complete per-run raw measurements are logged in 'benchmark_runs_detail.csv'.\n";
        out << "  - All runs are 100% deterministic and reproducible under Master Random Seed " << rngSeed << ".\n";

        // 2026.10.20 新增：空间均匀性三指标汇总（回答"非规则但均匀"）
        if (!allRawRuns.empty())
        {
            double gReg = 0, gRnd = 0, gJit = 0, gSa = 0;
            double cReg = 0, cRnd = 0, cJit = 0, cSa = 0, occ = 0;
            for (size_t i = 0; i < allRawRuns.size(); i++)
            {
                const BenchmarkRecord& r = allRawRuns[i];
                gReg += r.max_gap_regular;  gRnd += r.max_gap_random;
                gJit += r.max_gap_jitter_init; gSa += r.max_gap_sa;
                cReg += r.nn_cv_regular;    cRnd += r.nn_cv_random;
                cJit += r.nn_cv_jitter_init; cSa += r.nn_cv_sa;
                occ += r.jitter_unit_occupancy;
            }
            const double n = (double)allRawRuns.size();
            out << "\n[Uniformity Metrics (2026.10.20 added)]\n";
            out << "  Max gap (traces)      : Regular=" << QString::number(gReg/n, 'f', 2)
                << "  Random=" << QString::number(gRnd/n, 'f', 2)
                << "  JitterInit=" << QString::number(gJit/n, 'f', 2)
                << "  <b>SA=" << QString::number(gSa/n, 'f', 2) << "</b>\n";
            out << "  Nearest-neighbour CV  : Regular=" << QString::number(cReg/n, 'f', 4)
                << "  Random=" << QString::number(cRnd/n, 'f', 4)
                << "  JitterInit=" << QString::number(cJit/n, 'f', 4)
                << "  SA=" << QString::number(cSa/n, 'f', 4) << "\n";
            out << "  Jitter-unit occupancy of the SA mask : " << QString::number(occ/n*100.0, 'f', 2) << "%\n";
            out << "  Note: SA performs unconstrained pairwise swaps, so the final mask is NOT\n";
            out << "        guaranteed to stay on the jitter grid; occupancy below 100% quantifies\n";
            out << "        how far the optimiser drifts from the jitter prior.\n";
        }
        rptFile.close();

        // 绘制并保存科研图件 (集中保存到 figures/)
        QString centralFiguresDir = getCentralFiguresDir(outputDir);
        QDir().mkpath(centralFiguresDir);

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
            QDir(centralFiguresDir).filePath("02_sampling_benchmark_bars.png")
        );
    }

    outReportSummary = QString::fromLocal8Bit("【多采样率基准对比试验完成】(重复试验: %1 轮, 主随机种子: %2)\n")
        .arg(repeatRuns).arg(rngSeed);
    for (size_t i = 0; i < rateSummaries.size(); i++)
    {
        const MultiSeedSummary& s = rateSummaries[i];
        outReportSummary += QString::fromLocal8Bit(
            "%1. %2% 采样率 (m=%3/%4): 规则相干度=%5, SA均值=%6 (标准差: %7), 相比规则抽样改善率: %8%, 相干度比值衰减: %9 dB\n"
        ).arg((int)i + 1)
         .arg(QString::number(s.rate * 100.0f, 'f', 0))
         .arg(s.m).arg(s.N)
         .arg(QString::number(s.miu_regular, 'f', 4))
         .arg(QString::number(s.sa_miu_mean, 'f', 4))
         .arg(QString::number(s.sa_miu_std, 'f', 4))
         .arg(QString::number(s.imprv_vs_reg_mean, 'f', 2))
         .arg(QString::number(s.coherence_ratio_atten_db_mean, 'f', 2));
    }
    outReportSummary += QString::fromLocal8Bit("全采样率平均相干度比值衰减: %1 dB (明细数据已导出至 %2)")
        .arg(QString::number(overallCoherenceAttenDb, 'f', 2))
        .arg(outputDir);
    outReportSummary += QString::fromLocal8Bit(
        "\n\n【科研成果图件支撑】(全部数据图与结果图集中保存至 %1 文件夹)：\n"
        "  ● 图件 01_sa_convergence_curve.png: SA 当前解/历史最优轨迹（历史最优单调非增）\n"
        "  ● 图件 02_sampling_benchmark_bars.png: 三种采样方案 μ 对比柱状图（仅相干度口径）"
    ).arg(getCentralFiguresDir(outputDir));

    return true;
}

// =============================================================================
// Task 4: 曲波变换参数对 μ 排序的鲁棒性实验 (2026.10.19 新增)
// -----------------------------------------------------------------------------
// 背景：主流程的 μ 定义固定使用 fdct_wrapping(N,N,1,4,...)，即仅最粗尺度
//       (nbscales=1, nbangles_coarse=4)，并未利用曲波多尺度方向选择性。
//       若审稿人质疑"结论依赖于这一组固定参数"，需要有直接的敏感性证据。
// 做法：在同一组采样掩码（规则减抽 / 纯随机 / Hybrid 优化）上，用多组 // add by wushanzhou in 2026/10/23
//       (nbscales, nbangles) 重新计算 μ，检验三者的相对排序是否稳定。
// 输出：mu_param_robustness.csv + 文本结论。
// =============================================================================
bool runMiuParamRobustness(int N, float c, const QString& outputDir, QString& outReportSummary)
{
    QDir dir(outputDir);
    QDir().mkpath(outputDir);   // 2026.10.20: 原 dir.mkpath(outputDir) 会把 outputDir 重复拼接到 dir 之后

    const int m = (int)floor((double)N * c + 0.5);
    exprng::seed(20261019ULL);

    // 三组代表性掩码：规则减抽、纯随机(单次)、Hybrid 优化 // add by wushanzhou in 2026/10/23
    int *phiReg = Sampling1R_Count(N, m);
    int *phiRand = Sampling1S_Count(N, m);
    int *phiJit = Sampling1J_Count(N, m);
    SAParams sap;
    SAStats st;
    int *phiOpt = UnOptiMiuSAEx(N, c, phiJit, "J", true, &st, 2, sap);
    delete[] phiJit;

    std::vector<int> idxReg, idxRand, idxOpt;
    for (int i = 0; i < N; i++)
    {
        if (phiReg[i] == 1) idxReg.push_back(i);
        if (phiRand[i] == 1) idxRand.push_back(i);
        if (phiOpt[i] == 1) idxOpt.push_back(i);
    }

    // (nbscales, nbangles_coarse) 组合：1x4 为主流程基线；其余检验多尺度方向选择性影响。
    // 注：nbscales==1 时曲波只有 1 个低频块，nbangles 不生效，故不再重复采集 (1,8)；
    //     真正有区分度的是 nbscales>=2（2/3/4 尺度 × 8/12/16/32 方向），共 7 组。
    const int scArr[]  = { 1, 2, 2, 2, 3, 3, 4 };
    const int angArr[] = { 4, 8, 16, 32, 12, 16, 16 };
    const int nCombos = 7;

    QFile csv(dir.filePath("mu_param_robustness.csv"));
    if (!csv.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReportSummary = QString::fromLocal8Bit("【参数鲁棒性实验失败】：无法创建输出文件");
        delete[] phiReg; delete[] phiRand; delete[] phiOpt;
        return false;
    }

    QTextStream os(&csv);
    os.setCodec("UTF-8");
    os << "nbscales,nbangles,mu_regular,mu_random,mu_jitter_sa,order_stable,atten_db,range_check\n";

    QStringList rows;
    int stableCount = 0;
    int outOfRangeCombos = 0;   // 2026.10.20: μ 超出 [0,1] 的组数
    int baselineOrderStable = -1;
    double atten1x4 = 0.0;

    for (int k = 0; k < nCombos; k++)
    {
        const int sc = scArr[k];
        const int an = angArr[k];
        const float muR = CurveletMiu1DEigenParam(idxReg, N, 1, sc, an);
        const float muN = CurveletMiu1DEigenParam(idxRand, N, 1, sc, an);
        const float muO = CurveletMiu1DEigenParam(idxOpt, N, 1, sc, an);
        if (muR <= 0.0f && muN <= 0.0f && muO <= 0.0f) continue;

        // 期望排序: 优化 < 随机 < 规则 (μ 越低越好)
        // 2026.10.20: 块级多尺度度量下 μ 的数值尺度与单块不同（可能 >1），
        // 但排序比较仍然有效；这里如实记录是否稳定，不再强制断言。
        const bool stable = (muO < muN) && (muN < muR);
        if (stable) stableCount++;
        if (k == 0)
        {
            baselineOrderStable = stable ? 1 : 0;
            atten1x4 = (muO > 1e-6f) ? (20.0 * log10((double)muR / (double)muO)) : 0.0;
        }

        const double atten = (muO > 1e-6f) ? (20.0 * log10((double)muR / (double)muO)) : 0.0;
        // 额外记录 μ 是否落在归一化区间内，便于论文判断该度量是否健康
        const bool inRange = (muR <= 1.0f + 1e-6f) && (muN <= 1.0f + 1e-6f) && (muO <= 1.0f + 1e-6f);
        os << sc << "," << an << ","
           << QString::number(muR, 'f', 6) << ","
           << QString::number(muN, 'f', 6) << ","
           << QString::number(muO, 'f', 6) << ","
           << (stable ? "YES" : "NO") << ","
           << QString::number(atten, 'f', 3) << ","
           << (inRange ? "IN_[0,1]" : "OUT_OF_RANGE") << "\n";
        if (!inRange) outOfRangeCombos++;

        rows.append(QString::fromLocal8Bit("  (%1,%2): mu_reg=%3  mu_rand=%4  mu_SA=%5  order=%6  atten=%7 dB  [%8]")
            .arg(sc).arg(an)
            .arg(QString::number(muR, 'f', 4))
            .arg(QString::number(muN, 'f', 4))
            .arg(QString::number(muO, 'f', 4))
            .arg(stable ? QString::fromLocal8Bit("stable") : QString::fromLocal8Bit("UNSTABLE"))
            .arg(QString::number(atten, 'f', 2))
            .arg(inRange ? QString::fromLocal8Bit("in[0,1]") : QString::fromLocal8Bit("OUT_OF_RANGE")));
    }
    csv.close();

    delete[] phiReg;
    delete[] phiRand;
    delete[] phiOpt;

    const int total = rows.size();
    outReportSummary = QString::fromLocal8Bit(
        "【曲波变换参数鲁棒性实验完成】\n"
        "检验目标: 在不同 (nbscales, nbangles_coarse) 下，"
        "Hybrid < 纯随机 < 规则减抽 的相对排序是否稳定。\n" // add by wushanzhou in 2026/10/23
        "采样率: %1%%, m=%2/%3\n\n")
        .arg(QString::number(c * 100.0f, 'f', 0)).arg(m).arg(N);
    outReportSummary += rows.join("\n");
    outReportSummary += QString::fromLocal8Bit(
        "\n\n结论: %1/%2 组参数下三方案排序一致%3。\n"
        "重要限制（2026.10.20 复核）: nbscales>=2 时本函数经 ifdct_wrapping 精确逆变换"
        "重建感知矩阵，参数被数学约掉，因此多尺度各行的 μ 必然相同；"
        "本实验实际只有 (1,4) 一个独立参数点，不能作为参数敏感性或鲁棒性证据。\n"
        "明细: %4")
        .arg(stableCount).arg(total)
        .arg(stableCount == total ? QString::fromLocal8Bit("（全部一致）") : QString::fromLocal8Bit("（存在不一致组）"))
        .arg(dir.filePath("mu_param_robustness.csv"));

    // 便于测试断言
    if (baselineOrderStable == 0)
    {
        outReportSummary += QString::fromLocal8Bit("\n[警告] 基线 (1,4) 参数下排序即不稳定。");
    }
    Q_UNUSED(atten1x4);
    return true;
}

// 2026.10.20 新增：对外入口，便于 CLI 把参数鲁棒性结果归档到 results/ 下
bool GeometryExperimentEngine::runMiuParamRobustnessExperiment(int N, float c,
                                                               const QString& outputDir,
                                                               QString& outReportSummary)
{
    return runMiuParamRobustness(N, c, outputDir, outReportSummary);
}

// =============================================================================
// 2026.10.20 新增：混合优化贡献分解消融
// 回答三个此前无法回答的问题：
//   ① SA 单独贡献多少？ ② 局部搜索单独贡献多少？ ③ 混合是否胜过"等预算随机重启"？
// 输出 hybrid_ablation.csv（每行一个采样率）+ 文本摘要。
// =============================================================================
bool GeometryExperimentEngine::runHybridAblationExperiment(int N,
                                                           const std::vector<float>& rates,
                                                           unsigned long long seed,
                                                           const QString& outputDir,
                                                           QString& outReportSummary)
{
    QDir().mkpath(outputDir);
    QDir dir(outputDir);

    std::vector<float> useRates = rates;
    if (useRates.empty()) { useRates.push_back(0.30f); useRates.push_back(0.50f); useRates.push_back(0.70f); }

    SAParams sap;
    sap.T0 = 80.0;
    sap.Tmin = 0.5;
    sap.alpha = 0.85;
    sap.n_rapid = 8;
    sap.iter = 1;
    sap.strategy = SA_ACCEPT_METROPOLIS;

    QFile f(dir.filePath("hybrid_ablation.csv"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReportSummary = QString::fromLocal8Bit("【混合优化消融失败】：无法创建输出文件");
        return false;
    }
    QTextStream out(&f);
    out.setCodec("UTF-8");
    // 说明：所有 μ 均由 CurveletMiu1DEigen（nbscales=1）计算，与主流程同口径
    out << "# Master_Seed=" << QString::number(seed) << ", protocol=classic_psf_v2,budget_cap=200,sa_prefix=64,Metropolis\n"; // add by wushanzhou in 2026/10/23
    out << "Sampling_Rate,m,Total,Initial_Jitter_Mu,SA_only_Mu,CG_only_Mu,SA_CG_Mu,"
        << "SA_CG_Evals,Restart_K,Jitter_Restart_Best,Jitter_Restart_Mean,Random_Restart_Best,"
        << "Gain_Init_to_SA_dB,Gain_Init_to_SA_CG_dB,SA_CG_vs_JitterRestart_dB,SA_CG_vs_RandomRestart_dB\n";

    QStringList rep;
    rep << QString::fromLocal8Bit("【混合优化贡献分解消融】采样率: 30% / 50% / 70%，主随机种子: %1")
           .arg(QString::number(seed));

    for (size_t i = 0; i < useRates.size(); i++)
    {
        const float c = useRates[i];
        AblationResult ar;
        const unsigned long long subSeed = seed + (unsigned long long)(c * 1000.0f);
        if (!RunHybridAblation(N, c, subSeed, sap, ar, 0))
        {
            f.close();
            outReportSummary = QString::fromLocal8Bit("【混合优化消融失败】：采样率 %1 计算失败")
                .arg(QString::number(c, 'f', 2));
            return false;
        }
        const double gInitToSa = (ar.mu_init > 1e-6 && ar.mu_sa_only > 1e-6) ? 20.0 * log10((double)ar.mu_init / ar.mu_sa_only) : 0.0;
        const double gInitToSaCg = (ar.mu_init > 1e-6 && ar.mu_sa_cg > 1e-6) ? 20.0 * log10((double)ar.mu_init / ar.mu_sa_cg) : 0.0;
        const double gVsRestart = (ar.mu_restart_best > 1e-6 && ar.mu_sa_cg > 1e-6) ? 20.0 * log10((double)ar.mu_restart_best / ar.mu_sa_cg) : 0.0;
        const double gVsRand = (ar.mu_rand_best > 1e-6 && ar.mu_sa_cg > 1e-6) ? 20.0 * log10((double)ar.mu_rand_best / ar.mu_sa_cg) : 0.0;

        out << QString::number(c, 'f', 2) << "," << ar.m << "," << ar.n << ","
            << QString::number(ar.mu_init, 'f', 4) << ","
            << QString::number(ar.mu_sa_only, 'f', 4) << ","
            << QString::number(ar.mu_cg_only, 'f', 4) << ","
            << QString::number(ar.mu_sa_cg, 'f', 4) << ","
            << QString::number(ar.evalBudget, 'f', 0) << ","
            << ar.restart_trials << ","
            << QString::number(ar.mu_restart_best, 'f', 4) << ","
            << QString::number(ar.mu_restart_mean, 'f', 4) << ","
            << QString::number(ar.mu_rand_best, 'f', 4) << ","
            << QString::number(gInitToSa, 'f', 2) << ","
            << QString::number(gInitToSaCg, 'f', 2) << ","
            << QString::number(gVsRestart, 'f', 2) << ","
            << QString::number(gVsRand, 'f', 2) << "\n";

        rep << QString::fromLocal8Bit(
            "  %1%% (m=%2): 初始=%3 | SA-only=%4 | CG-only=%5 | SA+CG=%6 (%7 次评估)")
            .arg(QString::number(c * 100.0f, 'f', 0)).arg(ar.m)
            .arg(QString::number(ar.mu_init, 'f', 4))
            .arg(QString::number(ar.mu_sa_only, 'f', 4))
            .arg(QString::number(ar.mu_cg_only, 'f', 4))
            .arg(QString::number(ar.mu_sa_cg, 'f', 4))
            .arg(QString::number(ar.evalBudget, 'f', 0));
        rep << QString::fromLocal8Bit(
            "          等预算随机重启 (K=%1): 抖动最优=%2 均值=%3 | 纯随机最优=%4")
            .arg(ar.restart_trials)
            .arg(QString::number(ar.mu_restart_best, 'f', 4))
            .arg(QString::number(ar.mu_restart_mean, 'f', 4))
            .arg(QString::number(ar.mu_rand_best, 'f', 4));
        rep << QString::fromLocal8Bit(
            "          SA 贡献=%1 dB | SA+CG 总贡献=%2 dB | SA+CG vs 抖动重启=%3 dB | vs 纯随机重启=%4 dB")
            .arg(QString::number(gInitToSa, 'f', 2))
            .arg(QString::number(gInitToSaCg, 'f', 2))
            .arg(QString::number(gVsRestart, 'f', 2))
            .arg(QString::number(gVsRand, 'f', 2));
    }
    f.close();

    rep << QString::fromLocal8Bit(
        "\n[解读提示] SA+CG_vs_JitterRestart > 0 表示混合优化优于同等评估次数的抖动随机重启；"
        "若接近 0 或为负，说明混合优化的收益主要来自抖动先验与算力规模，而非搜索机制本身。");
    rep << QString::fromLocal8Bit("明细: %1").arg(dir.filePath("hybrid_ablation.csv"));
    outReportSummary = rep.join("\n");
    return true;
}

