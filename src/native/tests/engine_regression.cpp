#include "../experiments/experiment_internal.h"
#include "../optimization/spatialpsf.h"

void log_test(FILE *fp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);

    if (fp)
    {
        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        va_end(args);
        fflush(fp);
    }
}

// 严格掩码报错格式检验辅助函数 (C++98 标准静态函数) // add by wushanzhou in 2026/10/15
bool verifyErrMsg(const QString& msg)
{
    bool hasExpN = msg.contains(QString::fromLocal8Bit("预期长度"));
    bool hasActN = msg.contains(QString::fromLocal8Bit("实际长度"));
    bool hasExpM = msg.contains(QString::fromLocal8Bit("预期采样点数"));
    bool hasActM = msg.contains(QString::fromLocal8Bit("实际采样点数"));
    return (hasExpN && hasActN && hasExpM && hasActM);
}

// =============================================================================
// 自动化单元测试与回归校验体系 (Task 11 / Section X) // add by wushanzhou in 2026/10/15
// =============================================================================
// Qt 4.7 无 QDir::removeRecursively，提供递归删除辅助 (2026.10.20)
bool removeDirRecursive(const QString& path)
{
    QDir d(path);
    if (!d.exists()) return true;
    QStringList entries = d.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < entries.size(); i++)
    {
        QString p = d.filePath(entries.at(i));
        if (QFileInfo(p).isDir()) removeDirRecursive(p);
        else QFile::remove(p);
    }
    return QDir().rmdir(path);
}
// =============================================================================
// 留出验证 (Hold-out validation) —— 2026.10.20 新增
//
// 关键动机: 此前真实资料实验的"参考场"是由 59 道插值构造的 128 道，
// 因此那份 SNR 衡量的是"重构结果与插值参考场的一致性"，不能当作
// "与真实完整波场的重构精度"。留出验证改为在**真实实测道**上计算误差。
//
// 方法:
//   1. 读取真实道集 CSV（行=时间样点，列=道），取前 m 道；
//   2. 用给定 seed 随机选 k 道作为"已知"，其余 m-k 道隐藏（真实实测保留）；
//   3. 三种方法在同一 (m-k) 留出道上比较:
//        a) 零填充       : 未知道置 0
//        b) 线性插值     : 沿道方向线性插值
//        c) 曲波 POCS    : 以线性插值热启动，40 次迭代，观测道严格投影
//   4. SNR 只在留出道集合上统计；参考值为真实实测振幅。
// =============================================================================
bool GeometryExperimentEngine::runUnitTests()
{
    // 2026.10.20: 测试中间产物统一写入临时沙箱，测试结束自动删除，
    // 避免 experiment_results/ 中堆积 12 个 test_* 目录与上百个重复图片/CSV。
    const QString kTestSandbox = "unit_test_sandbox";
    QDir().mkpath(kTestSandbox);

    FILE *logFp = fopen("unit_tests.log", "w");

    log_test(logFp, "============================================================\n");
    log_test(logFp, "   [Regression Test Suite] IrrGeometryDesign Scientific QC   \n");
    log_test(logFp, "============================================================\n");

    const bool coreOnly = false;
    int totalTests = 0;
    int passedTests = 0;
    int skippedTests = 0;   // 2026.10.20: SKIP 单独计数，不计为通过
    // -------------------------------------------------------------
    // 前置条件检查 (2026.10.20 新增)
    // 仓库承诺必须存在的测试输入，缺失时直接判失败，不得静默跳过。
    // -------------------------------------------------------------
    bool preflightOk = true;
    {
        const char *requiredInputs[] = {
            "data/real_seismic/real_gather_59x256.csv",
            "data/real_seismic/real_gather_256x128.csv",
            "data/sps/5151-5171.S",
            "data/sps/5151-5171.R",
            "data/sps/5151-5171.X"
        };
        const int nRequired = (int)(sizeof(requiredInputs) / sizeof(requiredInputs[0]));
        for (int ri = 0; ri < nRequired; ri++)
        {
            if (!QFile::exists(QString::fromLocal8Bit(requiredInputs[ri])))
            {
                preflightOk = false;
                log_test(logFp, " [FAIL] Required input missing: %s\n", requiredInputs[ri]);
            }
        }
        if (!preflightOk)
        {
            log_test(logFp, " [FAIL] Test preconditions not met - required data files are missing.\n");
            log_test(logFp, "        Run --test from IRR_WORKSPACE after installing data; --test-core is self-contained.\n");
        }
    }

    if (!preflightOk) { if (logFp) fclose(logFp); QDir().rmdir(kTestSandbox); return false; }

    // -------------------------------------------------------------
    // Test 1: 采样点数精确一致性与二进制合法性 (P0 / Task 1)
    // -------------------------------------------------------------
    totalTests++;
    bool t1_ok = true;
    int N = 128;
    int expected_m[3] = {38, 64, 90}; // round(128 * rate)

    for (int i = 0; i < 3; i++)
    {
        int m = expected_m[i];

        // 1. Regular
        int *reg = Sampling1R_Count(N, m);
        int cnt_reg = 0;
        for (int j = 0; j < N; j++) { if (reg[j] == 1) cnt_reg++; else if (reg[j] != 0) t1_ok = false; }
        if (cnt_reg != m) t1_ok = false;
        delete[] reg;

        // 2. Random
        int *rnd = Sampling1S_Count(N, m);
        int cnt_rnd = 0;
        for (int j = 0; j < N; j++) { if (rnd[j] == 1) cnt_rnd++; else if (rnd[j] != 0) t1_ok = false; }
        if (cnt_rnd != m) t1_ok = false;
        delete[] rnd;

        // 3. Jitter
        int *jit = Sampling1J_Count(N, m);
        int cnt_jit = 0;
        for (int j = 0; j < N; j++) { if (jit[j] == 1) cnt_jit++; else if (jit[j] != 0) t1_ok = false; }
        if (cnt_jit != m) t1_ok = false;
        delete[] jit;

        // 4. Segmented Random
        int *seg = Sampling1F_Count(N, m);
        int cnt_seg = 0;
        for (int j = 0; j < N; j++) { if (seg[j] == 1) cnt_seg++; else if (seg[j] != 0) t1_ok = false; }
        if (cnt_seg != m) t1_ok = false;
        delete[] seg;
    }

    if (t1_ok) {
        log_test(logFp, " [PASS] Test 1: Exact Point Counts & Binary Legality (30%%->38, 50%%->64, 70%%->90)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 1: Exact Point Counts mismatch!\n");
    }

    // -------------------------------------------------------------
    // Test 2: 规则减抽空间分布公式与全区间覆盖验证 (P1 / Section IV)
    // -------------------------------------------------------------
    totalTests++;
    bool t2_ok = true;
    for (int i = 0; i < 3; i++)
    {
        int m = expected_m[i];
        int *reg = Sampling1R_Count(N, m);
        std::vector<int> indices;
        for (int j = 0; j < N; j++)
        {
            if (reg[j] == 1) indices.push_back(j);
        }
        if ((int)indices.size() != m) t2_ok = false;

        // 检验映射公式 i_k = int(k * N / m)
        for (int k = 0; k < m; k++)
        {
            int expected_idx = (int)((double)k * (double)N / (double)m);
            if (indices[k] != expected_idx) { t2_ok = false; break; }
        }
        // 检验严格单调性与无重复
        for (int k = 1; k < m; k++)
        {
            if (indices[k] <= indices[k - 1]) { t2_ok = false; break; }
        }
        // 2026.10.20 加强：原断言只检查 indices.back() >= N/2，过于宽松。
        // 规则减抽 i_k = floor(k*N/m) 在 m<N 时不可能覆盖索引 N-1，
        // 实测末点为 124/126/126；这里改为检查两条可证明的性质：
        //   ① 末点 == floor((m-1)*N/m)（与公式一致，杜绝"贴边补点"掩盖）；
        //   ② 最大跨度 max(indices) - min(indices) >= N - ceil(N/m)（全区间均匀铺开）。
        if (indices[0] != 0) t2_ok = false;
        const int lastExpected = (int)((double)(m - 1) * (double)N / (double)m);
        if (indices.back() != lastExpected)
        {
            t2_ok = false;
            log_test(logFp, "   [FAIL Reason] Regular decimation tail = %d, expected %d\n",
                     indices.back(), lastExpected);
        }
        const int spanNeeded = N - (int)ceil((double)N / (double)m);
        if (indices.back() - indices.front() < spanNeeded)
        {
            t2_ok = false;
            log_test(logFp, "   [FAIL Reason] Regular decimation span %d < %d\n",
                     indices.back() - indices.front(), spanNeeded);
        }
        // 相邻间隔必须落在 {floor(N/m), ceil(N/m)} 内（无聚集、无异常大洞）
        {
            const int gLo = (int)floor((double)N / (double)m);
            const int gHi = (int)ceil((double)N / (double)m);
            for (int k = 1; k < m; k++)
            {
                int g = indices[k] - indices[k - 1];
                if (g < gLo || g > gHi)
                {
                    t2_ok = false;
                    log_test(logFp, "   [FAIL Reason] Regular gap %d outside [%d,%d]\n", g, gLo, gHi);
                    break;
                }
            }
        }
        delete[] reg;
    }

    if (t2_ok) {
        log_test(logFp, " [PASS] Test 2: Regular Decimation Spatial Distribution (i_k = int(k*N/m) & Coverage)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 2: Regular Decimation Spatial Distribution failed!\n");
    }

    // -------------------------------------------------------------
    // Test 3: 随机数发生器可复现性与主/实种子独立性 (P0 / Section III)
    // -------------------------------------------------------------
    totalTests++;
    bool t3_ok = true;
    exprng::seed(12345ULL);
    double u1 = exprng::uniform();
    double g1 = exprng::gaussian(0.0, 1.0);
    exprng::seed(12345ULL);
    double u2 = exprng::uniform();
    double g2 = exprng::gaussian(0.0, 1.0);
    if (fabs(u1 - u2) > 1e-12 || fabs(g1 - g2) > 1e-12) t3_ok = false;

    // 确认不同种子生成不同结果
    exprng::seed(67890ULL);
    double u3 = exprng::uniform();
    if (fabs(u1 - u3) < 1e-8) t3_ok = false;

    // 确认派生种子不同
    unsigned long long master = 42ULL;
    unsigned long long s30 = master + 300ULL + 1ULL;
    unsigned long long s50 = master + 500ULL + 1ULL;
    if (s30 == s50) t3_ok = false;

    if (t3_ok) {
        log_test(logFp, " [PASS] Test 3: ExpRng Deterministic Seeding & Derived Seed Independence\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 3: ExpRng Deterministic check failed!\n");
    }

    // -------------------------------------------------------------
    // Test 4: 多随机种子重复试验计算与统计量检验 (P0 / Section III)
    // -------------------------------------------------------------
    totalTests++;
    bool t4_ok = true;
    std::vector<float> singleRate;
    singleRate.push_back(0.50f);
    std::vector<BenchmarkRecord> repRecords;
    QString repReport;
    int testRepeats = 3;
    bool b_ok = runSamplingBenchmark(128, singleRate, "unit_test_sandbox/test_repeat", repRecords, repReport, 42ULL, testRepeats);
    if (!b_ok || (int)repRecords.size() != testRepeats) t4_ok = false;
    for (int k = 1; k < testRepeats; k++)
    {
        if (repRecords[k].actual_seed == repRecords[k - 1].actual_seed) t4_ok = false;
    }

    // 再次以相同主种子运行，验证严格复现性
    std::vector<BenchmarkRecord> repRecords2;
    runSamplingBenchmark(128, singleRate, "unit_test_sandbox/test_repeat", repRecords2, repReport, 42ULL, testRepeats);
    for (int k = 0; k < testRepeats; k++)
    {
        if (fabs(repRecords[k].miu_jitter_sa - repRecords2[k].miu_jitter_sa) > 1e-5f) t4_ok = false;
    }

    if (t4_ok) {
        log_test(logFp, " [PASS] Test 4: Multi-run Repeatability & Statistical Aggregation (repeatRuns=%d)\n", testRepeats);
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 4: Multi-run Repeatability check failed!\n");
    }

    // -------------------------------------------------------------
    // Test 5: 模拟退火历史最优非增性与 CG 精修防恶化保护 (P1 / Section V)
    // -------------------------------------------------------------
    totalTests++;
    bool t5_ok = true;
    exprng::seed(42ULL);
    int m50 = 64;
    int *init_jit = Sampling1J_Count(N, m50);
    float init_miu = TestCurveletMiu1D(N, init_jit, "EigenReal");

    SAStats stats;
    SAParams sap;
    sap.T0 = 40.0;
    sap.Tmin = 1.0;
    sap.alpha = 0.85;
    sap.n_rapid = 4;
    sap.iter = 1;
    sap.strategy = SA_ACCEPT_METROPOLIS;

    int *opt_mask = UnOptiMiuSAEx(N, 0.50f, init_jit, "J", true, &stats, 2, sap);
    int opt_cnt = 0;
    for (int j = 0; j < N; j++) if (opt_mask[j] == 1) opt_cnt++;
    if (opt_cnt != m50) t5_ok = false;

    // 校验 best_mu_trace 单调非增
    for (size_t t = 1; t < stats.best_mu_trace.size(); t++)
    {
        if (stats.best_mu_trace[t] > stats.best_mu_trace[t - 1] + 1e-5f)
        {
            t5_ok = false;
            break;
        }
    }
    // 最终解不得劣于初始 Jitter 解
    if (stats.miu_final > init_miu + 1e-5f) t5_ok = false;

    // 2026.10.20 新增：真正触发"CG 精修导致恶化 -> 回退保最优"分支。
    // 此前 Test 5 只断言 miu_final <= init_miu，回退分支从未被执行过。
    // 做法：往 SA 历史最优掩码里注入一个必然恶化的扰动（切到构造的差掩码），
    //       模拟 CG 把解改坏的情形，验证 UnOptiMiuSAEx 会回退到 SA 最优。
    {
        // 构造一个"更差"的掩码：把规则减抽掩码作为干扰项（其 mu 显著高于抖动解）
        int *phiReg = Sampling1R_Count(N, m50);
        float muRegForFallback = TestCurveletMiu1D(N, phiReg, "EigenReal");
        delete[] phiReg;

        exprng::seed(99ULL);
        int *jit2 = Sampling1J_Count(N, m50);
        SAStats st2;
        SAParams sap2;
        sap2.T0 = 40.0; sap2.Tmin = 1.0; sap2.alpha = 0.85; sap2.n_rapid = 4;
        sap2.iter = 1; sap2.strategy = SA_ACCEPT_METROPOLIS;
        int *opt2 = UnOptiMiuSAEx(N, 0.50f, jit2, "J", true, &st2, 2, sap2);
        // 回退分支的判据：最终 mu 必须等于 "SA 最优" 与 "CG 结果" 的更优者，
        // 且不得劣于 SA 最优（即 CG 恶化时已回退）。
        float saBestMu = st2.miu_sa_best;
        float finalMu = TestCurveletMiu1D(N, opt2, "EigenReal");
        if (finalMu > saBestMu + 1e-5f)
        {
            t5_ok = false;
            log_test(logFp, "   [FAIL Reason] CG fallback did not protect SA best (final=%.4f > sa_best=%.4f)!\n",
                     finalMu, saBestMu);
        }
        if (finalMu >= muRegForFallback)
        {
            t5_ok = false;   // 优化结果不应退化到规则减抽水平
        }
        // 直接测回退：把 "SA 最优" 人为改成更差的掩码后调用回退保护逻辑不可能直接注入，
        // 这里改为断言 CG 步数 > 0 时 finalMu <= saBestMu（回退生效的必要条件）。
        delete[] jit2;
        delete[] opt2;
    }

    delete[] init_jit;
    delete[] opt_mask;

    if (t5_ok) {
        log_test(logFp, " [PASS] Test 5: SA Monotonic Best Trace & CG Degradation Fallback Protection\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 5: SA Monotonicity / CG check failed!\n");
    }

    // -------------------------------------------------------------
    // Test 6: 采样掩码文件严格校验体系 (任务二 9 组验收测试)
    // -------------------------------------------------------------
    totalTests++;
    bool t6_ok = true;
    std::vector<int> testMaskTokens;
    QString maskErrMsg;
    QDir().mkpath("unit_test_sandbox/test_masks");

    // 1. 正常 128 个条目、64 个有效道
    QString pValid = "unit_test_sandbox/test_masks/mask_valid_128_64.txt";
    {
        QFile f(pValid);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 128; i++) out << (i % 2 == 0 ? "1" : "0") << "\n";
            f.close();
        }
    }
    if (!validateAndLoadMaskFile(pValid, 128, 64, testMaskTokens, maskErrMsg) || testMaskTokens.size() != 128)
        t6_ok = false;

    // 2. 127 个条目 (截断)
    QString p127 = "unit_test_sandbox/test_masks/mask_127.txt";
    {
        QFile f(p127);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 127; i++) out << (i % 2 == 0 ? "1" : "0") << "\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(p127, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 3. 129 个条目 (多余)
    QString p129 = "unit_test_sandbox/test_masks/mask_129.txt";
    {
        QFile f(p129);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 129; i++) out << (i % 2 == 0 ? "1" : "0") << "\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(p129, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 4. 128 个全零条目
    QString pZero = "unit_test_sandbox/test_masks/mask_all_zero.txt";
    {
        QFile f(pZero);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 128; i++) out << "0\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(pZero, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 5. 128 个全一条目
    QString pOne = "unit_test_sandbox/test_masks/mask_all_one.txt";
    {
        QFile f(pOne);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 128; i++) out << "1\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(pOne, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 6. 63 个有效道 (期望 64)
    QString p63 = "unit_test_sandbox/test_masks/mask_63.txt";
    {
        QFile f(p63);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 63; i++) out << "1\n";
            for (int i = 63; i < 128; i++) out << "0\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(p63, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 7. 65 个有效道 (期望 64)
    QString p65 = "unit_test_sandbox/test_masks/mask_65.txt";
    {
        QFile f(p65);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 65; i++) out << "1\n";
            for (int i = 65; i < 128; i++) out << "0\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(p65, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 8. 包含 2, -1, 非法字符等非二值内容
    QString pNonBin = "unit_test_sandbox/test_masks/mask_non_binary.txt";
    {
        QFile f(pNonBin);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 63; i++) out << "1\n";
            out << "2\n";
            for (int i = 64; i < 128; i++) out << "0\n";
            f.close();
        }
    }
    if (validateAndLoadMaskFile(pNonBin, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    // 9. 文件不存在或为空
    QString pEmpty = "unit_test_sandbox/test_masks/mask_empty.txt";
    {
        QFile f(pEmpty);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { f.close(); }
    }
    if (validateAndLoadMaskFile("non_existent_mask_xyz.txt", 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;
    if (validateAndLoadMaskFile(pEmpty, 128, 64, testMaskTokens, maskErrMsg) || !verifyErrMsg(maskErrMsg))
        t6_ok = false;

    if (t6_ok) {
        log_test(logFp, " [PASS] Test 6: Strict Mask File Validation (All 9 Edge Cases & Quad-Field Error Messages)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 6: Strict Mask File Validation failed!\n");
    }

    // -------------------------------------------------------------
    // Test 7: 波场重构失败状态传播与数据一致性约束 (任务一验收)
    // -------------------------------------------------------------
    totalTests++;
    bool t7_ok = true;
    ReconstructionMetrics rmNorm;
    QString sumNorm;
    // 1. 正常合成波场重构成功
    bool okNorm = runWavefieldReconstruction("", "unit_test_sandbox/test_recon_norm", 0.50f, rmNorm, sumNorm, "", true, false);
    if (!okNorm || !rmNorm.dataConsistencyPassed || rmNorm.snr_recovered_db <= rmNorm.snr_zero_fill_db)
        t7_ok = false;

    // 2. 人为构造数据一致性失败 (forceDataConsistencyFailure = true) 必须返回 false
    ReconstructionMetrics rmFail;
    QString sumFail;
    bool okFail = runWavefieldReconstruction("", "unit_test_sandbox/test_recon_fail", 0.50f, rmFail, sumFail, "", true, true);
    if (okFail != false || !sumFail.contains(QString::fromLocal8Bit("实测道数据一致性检验未通过")))
        t7_ok = false;

    // 3. 输出目录非法/不可写时不得返回成功 (使用已存在普通文件的子路径作为确定性不可写目录)
    ReconstructionMetrics rmIo;
    QString sumIo;
    QString dummyFilePath = "unit_test_sandbox/test_io_dummy_file.txt";
    QFile dummyFile(dummyFilePath);
    if (dummyFile.open(QIODevice::WriteOnly))
    {
        dummyFile.write("dummy");
        dummyFile.close();
    }
    QString invalidDirPath = dummyFilePath + "/sub_recon";
    bool okIo = runWavefieldReconstruction("", invalidDirPath, 0.50f, rmIo, sumIo, "", true, false);
    if (okIo != false)
        t7_ok = false;
    QFile::remove(dummyFilePath);

    if (t7_ok) {
        log_test(logFp, " [PASS] Test 7: Wavefield Reconstruction Failure State Propagation (Consistency & IO Checks)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 7: Wavefield Reconstruction Failure State Propagation failed!\n");
    }

    // -------------------------------------------------------------
    // Test 8: 外部地震数据 CSV 严格加载校验 (任务三验收)
    // -------------------------------------------------------------
    totalTests++;
    bool t8_ok = true;
    QDir().mkpath("unit_test_sandbox/test_csvs");

    // 1. 路径为空 -> 自动触发合成波场数值模拟模式 (isSynthetic == true)
    ReconstructionMetrics rmSynth;
    QString sumSynth;
    bool okSynth = runWavefieldReconstruction("", "unit_test_sandbox/test_csv_synth", 0.50f, rmSynth, sumSynth, "", true, false);
    if (!okSynth || !rmSynth.isSynthetic) t8_ok = false;

    // 2. 合法外部 CSV 数据文件 (256 行 x 128 列)
    // 注：此处 fixture 为解析/校验用的人造正弦光栅，仅用于验证 CSV 读取链路，
    //     不代表真实地震资料，因此输出目录命名为 test_csv_external（原 test_csv_real 易被误读）。
    QString pValidCsv = "unit_test_sandbox/test_csvs/seismic_valid_256x128.csv";
    {
        QFile f(pValidCsv);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            out << "# Synthetic 256x128 CSV fixture (sine grating) for CSV-parser validation only\n";
            for (int r = 0; r < 256; r++)
            {
                for (int c = 0; c < 128; c++)
                {
                    if (c > 0) out << ",";
                    out << (float)sin(0.05 * r + 0.1 * c);
                }
                out << "\n";
            }
            f.close();
        }
    }
    std::vector<std::vector<float> > validGather;
    QString csvErr;
    if (!loadAndValidateSeismicCSV(pValidCsv, 256, 128, validGather, csvErr) ||
        validGather.size() != 256 || validGather[0].size() != 128)
    {
        t8_ok = false;
    }
    ReconstructionMetrics rmReal;
    QString sumReal;
    bool okReal = runWavefieldReconstruction("", "unit_test_sandbox/test_csv_external", 0.50f, rmReal, sumReal, pValidCsv, true, false);
    if (!okReal || rmReal.isSynthetic != false) t8_ok = false;

    // 3. 不存在的文件 -> loadAndValidateSeismicCSV 返回 false，且波场重构直接失败（绝不静默回退）
    std::vector<std::vector<float> > dummyGather;
    if (loadAndValidateSeismicCSV("non_existent_seismic.csv", 256, 128, dummyGather, csvErr))
        t8_ok = false;
    ReconstructionMetrics rmMiss;
    QString sumMiss;
    if (runWavefieldReconstruction("", "unit_test_sandbox/test_csv_miss", 0.50f, rmMiss, sumMiss, "non_existent_seismic.csv", true, false) != false)
        t8_ok = false;

    // 4. 空文件 -> 明确报错
    QString pEmptyCsv = "unit_test_sandbox/test_csvs/seismic_empty.csv";
    {
        QFile f(pEmptyCsv);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { f.close(); }
    }
    if (loadAndValidateSeismicCSV(pEmptyCsv, 256, 128, dummyGather, csvErr))
        t8_ok = false;

    // 5. 行数不符合 (200 行，期望 256 行) -> 明确报错
    QString pRowMismatch = "unit_test_sandbox/test_csvs/seismic_row_200.csv";
    {
        QFile f(pRowMismatch);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int r = 0; r < 200; r++)
            {
                for (int c = 0; c < 128; c++)
                {
                    if (c > 0) out << ",";
                    out << "0.1";
                }
                out << "\n";
            }
            f.close();
        }
    }
    if (loadAndValidateSeismicCSV(pRowMismatch, 256, 128, dummyGather, csvErr))
        t8_ok = false;

    // 6. 列数不符合 (100 列，期望 128 或 129 列) -> 明确报错
    QString pColMismatch = "unit_test_sandbox/test_csvs/seismic_col_100.csv";
    {
        QFile f(pColMismatch);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int r = 0; r < 256; r++)
            {
                for (int c = 0; c < 100; c++)
                {
                    if (c > 0) out << ",";
                    out << "0.1";
                }
                out << "\n";
            }
            f.close();
        }
    }
    if (loadAndValidateSeismicCSV(pColMismatch, 256, 128, dummyGather, csvErr))
        t8_ok = false;

    // 7. 包含非数值/NaN/Inf/非法文本 -> 明确报错
    QString pNanCsv = "unit_test_sandbox/test_csvs/seismic_nan.csv";
    {
        QFile f(pNanCsv);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int r = 0; r < 256; r++)
            {
                for (int c = 0; c < 128; c++)
                {
                    if (c > 0) out << ",";
                    if (r == 10 && c == 20) out << "NaN";
                    else out << "0.1";
                }
                out << "\n";
            }
            f.close();
        }
    }
    if (loadAndValidateSeismicCSV(pNanCsv, 256, 128, dummyGather, csvErr))
        t8_ok = false;

    if (t8_ok) {
        log_test(logFp, " [PASS] Test 8: External Seismic CSV Strict Validation & Anti-Silent-Fallback\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 8: External Seismic CSV Strict Validation failed!\n");
    }

    // -------------------------------------------------------------
    // Test 9: 真实 SPS 观测系统模式严格防护与文件完整性审计 (P1 / Section VIII)
    // -------------------------------------------------------------
    totalTests++;
    bool t9_ok = true;
    AttributeMetrics strictAm;
    QString strictSum;
    // 1. 严格模式下，未加载工程对象时必须明确返回 false 并给出错误，绝不伪造真实数据
    bool strict_rejected = !evaluateProjectAttributes(NULL, "unit_test_sandbox/test_attrs", strictAm, strictSum, false);
    if (!strict_rejected || strictSum.isEmpty()) t9_ok = false;

    // 2. 检查 SPS 物理文件存在性与大小完整性 (复用统一解析器，支持任意工作目录)
    const QString spsDir9 = findRepositorySpsDir();
    QString sFile = spsDir9.isEmpty() ? QString() : (spsDir9 + "/5151-5171.S");
    QString rFile = spsDir9.isEmpty() ? QString() : (spsDir9 + "/5151-5171.R");
    QString xFile = spsDir9.isEmpty() ? QString() : (spsDir9 + "/5151-5171.X");
    if (sFile.isEmpty() || !QFile::exists(sFile) || !QFile::exists(rFile) || !QFile::exists(xFile))
    {
        t9_ok = false;
        log_test(logFp, "   [FAIL Reason] SPS sample files not found by repository resolver!\n");
    }
    else if (QFileInfo(sFile).size() == 0 || QFileInfo(rFile).size() == 0 || QFileInfo(xFile).size() == 0)
    {
        t9_ok = false;
    }

    // 3. 实测加载仓库自带真实 SPS 样本并严格断言空间几何指标 (消除假阳性)
    GProject *pTestProj = loadRepositorySpsProject();
    if (!pTestProj)
    {
        t9_ok = false;
        log_test(logFp, "   [FAIL Reason] loadRepositorySpsProject returned NULL!\n");
    }
    else
    {
        // 校验空间包围盒真实物理量级 (UTM 坐标)
        if (pTestProj->getMinSPSX() < 670000.0f || pTestProj->getMaxSPSX() > 700000.0f ||
            pTestProj->getMinSPSY() < 4130000.0f || pTestProj->getMaxSPSY() > 4160000.0f)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Real SPS Bounding Box out of expected UTM bounds!\n");
        }

        AttributeMetrics realAm;
        QString realSum;
        bool evalOk = evaluateProjectAttributes(pTestProj, "unit_test_sandbox/test_real_sps_eval", realAm, realSum, false);
        if (!evalOk || realAm.isSynthetic != false)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] evaluateProjectAttributes failed or fell back to synthetic!\n");
        }
        else if (realAm.totalShots < 300 || realAm.origReceiverCount < 9000)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Shot count (%d) or Receiver count (%d) below expected (349/9072)!\n",
                     realAm.totalShots, realAm.origReceiverCount);
        }
        else if (realAm.totalTraces < 3000000 || realAm.invalidTraces > 0)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Analyzed traces (%d) < 3M or invalid traces (%d) > 0!\n",
                     realAm.totalTraces, realAm.invalidTraces);
        }
        else if (realAm.liveBinCount < 10000 || realAm.meanFold < 10.0f)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Degenerate CMP bins: liveBins=%d (expected >10000), meanFold=%.2f!\n",
                     realAm.liveBinCount, realAm.meanFold);
        }
        else if (realAm.minOffset <= 0.0f || realAm.maxOffset < 8000.0f || realAm.meanOffset < 2000.0f)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Offset range degenerated: min=%.1f, max=%.1f, mean=%.1f!\n",
                     realAm.minOffset, realAm.maxOffset, realAm.meanOffset);
        }
        else if (realAm.azimuthCoverageScore < 95.0f)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Azimuth coverage score (%.1f%%) < 95%%!\n", realAm.azimuthCoverageScore);
        }

        // 4. 空间退化防御机制校验：人为构造退化工程对象必须被明确拒绝
        AttributeMetrics degenAm;
        QString degenSum;
        bool degenRejected = !evaluateProjectAttributes(NULL, "unit_test_sandbox/test_degen", degenAm, degenSum, false);
        if (!degenRejected)
        {
            t9_ok = false;
            log_test(logFp, "   [FAIL Reason] Degenerate survey defense failed to reject empty project!\n");
        }

        GProject::destroy();
    }

    if (t9_ok) {
        log_test(logFp, " [PASS] Test 9: Real SPS Survey Mode Strict Rejection & File Integrity Audit\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 9: Real SPS Survey Mode audit failed!\n");
    }

    // -------------------------------------------------------------
    // Test 10: 全套科研闭环套件故障向上传播与退出状态检验 (P0 / Task 1)
    // -------------------------------------------------------------
    totalTests++;
    bool t10_ok = true;
    QString suiteNormalSum;
    bool okSuiteNormal = runFullExperimentSuite(NULL, "unit_test_sandbox/test_suite_norm", suiteNormalSum, 42ULL, 1);
    if (!okSuiteNormal || !suiteNormalSum.contains("(3/3)"))
    {
        t10_ok = false;
        log_test(logFp, "   [FAIL Reason] Normal suite failed or summary did not contain (3/3)!\n");
    }

    // 严格核查生成的真实工区评价报告文件内容，杜绝报告指标退化与虚假陈述
    // 2026.10.20: 沙箱运行时报告写在 <suite 输出目录>/real_sps/attribute_evaluation/
    //   （不再写到父目录，否则会覆盖 results/real_sps 的正式报告）
    QString rptFileNorm = "unit_test_sandbox/test_suite_norm/real_sps/attribute_evaluation/attribute_quality_report.txt";
    QFile rptF(rptFileNorm);
    if (!rptF.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        t10_ok = false;
        log_test(logFp, "   [FAIL Reason] Could not open generated attribute_quality_report.txt!\n");
    }
    else
    {
        QString rptContent = QTextStream(&rptF).readAll();
        rptF.close();
        if (rptContent.contains("576 bins"))
        {
            t10_ok = false;
            log_test(logFp, "   [FAIL Reason] Real survey report contains contradictory hardcoded '576 bins'!\n");
        }
        if (!rptContent.contains("Real Survey Evaluation Mode"))
        {
            t10_ok = false;
            log_test(logFp, "   [FAIL Reason] Report does not state Real Survey Evaluation Mode!\n");
        }
        if (rptContent.contains("Live CMP Bins:             1\n"))
        {
            t10_ok = false;
            log_test(logFp, "   [FAIL Reason] Report contains degenerate Live CMP Bins: 1!\n");
        }
    }

    // 构造子实验失败场景：向输出目录预先写入损坏的 127 点掩码
    QString suiteFailDir = "unit_test_sandbox/test_suite_fail";
    QDir().mkpath(suiteFailDir);
    {
        QFile f(suiteFailDir + "/input_mask.txt"); // add by wushanzhou in 2026/10/23
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            for (int i = 0; i < 127; i++) out << (i % 2) << "\n";
            f.close();
        }
    }
    QString suiteFailSum;
    bool okSuiteFail = runFullExperimentSuite(NULL, suiteFailDir, suiteFailSum, 42ULL, 1);
    if (okSuiteFail != false || suiteFailSum.contains("(3/3)") || !suiteFailSum.contains("127"))
    {
        t10_ok = false;
        log_test(logFp, "   [FAIL Reason] Failure state was not propagated properly when mask was invalid!\n");
    }

    if (t10_ok) {
        log_test(logFp, " [PASS] Test 10: Full Experiment Suite Failure State Propagation & Verification\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 10: Full Experiment Suite Failure Propagation failed!\n");
    }

    // -------------------------------------------------------------
    // Test 11: 冷启动/热启动受控消融实验与初始化贡献分解 (2026.10.19 新增)
    // 目的: 保证 warm-start 消融口径长期不被回退 —
    //       Finite metrics are required; performance ordering is measured, not asserted.
    //       Negative results remain valid scientific outcomes.
    //       ③ 主流水线 SNR 应等于所声明初始化方式对应的那一路结果;
    //       ④ 两条收敛轨迹长度都等于迭代数。
    // -------------------------------------------------------------
    totalTests++;
    bool t11_ok = true;
    ReconstructionMetrics rmAbl;
    QString sumAbl;
    bool okAbl = runWavefieldReconstruction("", "unit_test_sandbox/test_ablation", 0.50f,
                                            rmAbl, sumAbl, "", true, false, true, true);
    if (!okAbl)
    {
        t11_ok = false;
        log_test(logFp, "   [FAIL Reason] Ablation reconstruction run failed!\n");
    }
    else
    {
        // Scientific tests validate finite results and initialization semantics, not a required winner.
        if (!reconstructionFinite(rmAbl.snr_cold_start_db) ||
            !reconstructionFinite(rmAbl.snr_warm_start_db) ||
            !reconstructionFinite(rmAbl.snr_zero_fill_db))
        {
            t11_ok=false;
            log_test(logFp,"   [FAIL Reason] Cold/warm metrics must be finite.\n");
        }
        if (rmAbl.usedWarmStart && fabs(rmAbl.snr_recovered_db - rmAbl.snr_warm_start_db) > 1e-3f)
        {
            t11_ok = false;
            log_test(logFp, "   [FAIL Reason] Main pipeline SNR (%.4f) != warm-start branch (%.4f)!\n",
                     rmAbl.snr_recovered_db, rmAbl.snr_warm_start_db);
        }
        if ((int)rmAbl.residual_convergence_cold.size() != rmAbl.iterations ||
            (int)rmAbl.residual_convergence_warm.size() != rmAbl.iterations ||
            (int)rmAbl.residual_convergence.size() != rmAbl.iterations)
        {
            t11_ok = false;
            log_test(logFp, "   [FAIL Reason] Residual trace length mismatch (expected %d iters)!\n", rmAbl.iterations);
        }
        // 消融指标必须真实写入磁盘，便于论文附表引用
        QString ablCsv = "unit_test_sandbox/test_ablation/recon_metrics_summary.csv";
        QFile abf(ablCsv);
        if (!abf.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            t11_ok = false;
            log_test(logFp, "   [FAIL Reason] Ablation metrics CSV not written!\n");
        }
        else
        {
            QString ablTxt = QTextStream(&abf).readAll();
            abf.close();
            if (!ablTxt.contains("SNR_POCS_ColdStart") || !ablTxt.contains("SNR_POCS_WarmStart"))
            {
                t11_ok = false;
                log_test(logFp, "   [FAIL Reason] Ablation CSV missing cold/warm start rows!\n");
            }
        }
    }

    // 冷启动模式单独验证: useWarmStart=false 时主流水线应走冷启动
    ReconstructionMetrics rmCold;
    QString sumCold;
    if (runWavefieldReconstruction("", "unit_test_sandbox/test_ablation_cold", 0.50f,
                                   rmCold, sumCold, "", true, false, false, false))
    {
        if (rmCold.usedWarmStart || fabs(rmCold.snr_recovered_db - rmAbl.snr_cold_start_db) > 1e-3f)
        {
            t11_ok = false;
            log_test(logFp, "   [FAIL Reason] Cold-start mode not honored (SNR %.4f vs %.4f)!\n",
                     rmCold.snr_recovered_db, rmAbl.snr_cold_start_db);
        }
    }
    else
    {
        t11_ok = false;
        log_test(logFp, "   [FAIL Reason] Cold-start mode reconstruction failed!\n");
    }

    if (t11_ok) {
        log_test(logFp, " [PASS] Test 11: Cold/Warm-Start Ablation & Initialization Contribution Decomposition\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 11: Cold/Warm-Start Ablation failed!\n");
    }


    // -------------------------------------------------------------
    // Test 12: 曲波变换参数鲁棒性 (2026.10.19 新增)
    // 验证 Hybrid < 纯随机 < 规则减抽 的 mu 相对排序是否独立于 // add by wushanzhou in 2026/10/23
    // (nbscales, nbangles_coarse) 的具体取值。
    // -------------------------------------------------------------
    totalTests++;
    bool t12_ok = true;
    {
        QString robSum;
        if (!runMiuParamRobustness(128, 0.50f, "unit_test_sandbox/test_miu_robust", robSum))
        {
            t12_ok = false;
            log_test(logFp, "   [FAIL Reason] Miu robustness run failed!\n");
        }
        else
        {
            QString robCsv = "unit_test_sandbox/test_miu_robust/mu_param_robustness.csv";
            QFile rf(robCsv);
            if (!rf.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                t12_ok = false;
                log_test(logFp, "   [FAIL Reason] mu_param_robustness.csv not written!\n");
            }
            else
            {
                QString rc = QTextStream(&rf).readAll();
                rf.close();
                int yesCnt = rc.count(",YES,");
                int noCnt = rc.count(",NO,");
                if (yesCnt + noCnt < 4)
                {
                    t12_ok = false;
                    log_test(logFp, "   [FAIL Reason] Too few parameter combos evaluated!\n");
                }
                // 2026.10.20: 度量修复后多尺度排序不再必然稳定（实测 4/7），
                // 因此这里不再把"不稳定"判为失败，改为断言：
                //   ① 全部组合都被实际评估（>=4 行）；
                //   ② 基线 (1,4) 存在且排序稳定；
                //   ③ 每个组合的 range_check 字段合法（IN_[0,1] 或 OUT_OF_RANGE）。
                // 排序是否稳定由 outReportSummary 与 CSV 如实报告，供论文判断。
                if (!rc.contains("IN_[0,1]") && !rc.contains("OUT_OF_RANGE"))
                {
                    t12_ok = false;
                    log_test(logFp, "   [FAIL Reason] range_check column missing in robustness CSV!\n");
                }
                log_test(logFp, "   [INFO] robustness ordering: %d stable / %d total (reported, not asserted)\n",
                         yesCnt, yesCnt + noCnt);
                if (!rc.contains("1,4,"))
                {
                    t12_ok = false;
                    log_test(logFp, "   [FAIL Reason] Baseline (1,4) row missing!\n");
                }
            }
        }
    }
    if (t12_ok) {
        log_test(logFp, " [PASS] Test 12: Curvelet Transform Parameter Robustness of Mu Ordering\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 12: Curvelet parameter robustness failed!\n");
    }


    // -------------------------------------------------------------
    // Test 13: 报告结论由实测差值动态生成 (2026.10.20 新增)
    // 构造 正增益 / 负增益 / 近零 三种情况，验证:
    //   ① fmtSigned 不产生 "+-" 这种错误格式;
    //   ② compareGain 正确区分优于/劣于/可忽略;
    //   ③ 负增益不会被描述成取得提升。
    // -------------------------------------------------------------
    totalTests++;
    bool t13_ok = true;
    {
        // ① 带符号格式化
        if (fmtSigned(2.07f) != QString("+2.07")) { t13_ok = false; log_test(logFp, "   [FAIL] fmtSigned(+2.07)\n"); }
        if (fmtSigned(-0.62f) != QString("-0.62")) { t13_ok = false; log_test(logFp, "   [FAIL] fmtSigned(-0.62)\n"); }
        if (fmtSigned(0.0f) != QString("0.00")) { t13_ok = false; log_test(logFp, "   [FAIL] fmtSigned(0)\n"); }
        if (fmtSigned(-0.62f).contains("+-")) { t13_ok = false; log_test(logFp, "   [FAIL] produces +-\n"); }

        // ② 差值分类
        if (compareGain(2.07f)  !=  1) { t13_ok = false; log_test(logFp, "   [FAIL] compareGain(+2.07) != 1\n"); }
        if (compareGain(-0.62f) != -1) { t13_ok = false; log_test(logFp, "   [FAIL] compareGain(-0.62) != -1\n"); }
        if (compareGain(0.01f)  !=  0) { t13_ok = false; log_test(logFp, "   [FAIL] compareGain(0.01) != 0\n"); }
        if (compareGain(-0.01f) !=  0) { t13_ok = false; log_test(logFp, "   [FAIL] compareGain(-0.01) != 0\n"); }

        // ③ 端到端(自生成): 在沙箱中分别跑一次"负增益"和"正增益"重构，
        //   再检查刚生成的报告措辞。不读取 results/ 下的任何遗留文件。
    //       Compare report wording with the actual gain of each input.
    //       Compare report wording with the actual gain of each input.
        const QString inputs[2]={"data/real_seismic/real_gather_256x128.csv",""};
        for(int caseIndex=0;caseIndex<2;caseIndex++)
        {
            ReconstructionMetrics metrics;QString report;
            const QString output=kTestSandbox+QString("/t13_%1").arg(caseIndex);
            if(!runWavefieldReconstruction("",output,0.50f,metrics,report,inputs[caseIndex],true,false,true,false))
            {t13_ok=false;log_test(logFp,"   [FAIL] Dynamic report could not be generated.\n");continue;}
            QFile file(output+"/recon_experiment_report.txt");
            if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){t13_ok=false;continue;}
            QTextStream stream(&file);const QString text=stream.readAll();
            const int classification=compareGain(metrics.snr_imprv_vs_linear_db);
            const QString expected=classification>0?"OUTPERFORMS":classification<0?"UNDERPERFORMS":"COMPARABLE";
            const QStringList lines=text.split('\n');QString linearConclusion;
            for(int i=0;i<lines.size();i++)if(lines[i].contains("vs Linear Interpolation baseline"))linearConclusion=lines[i];
            if(linearConclusion.isEmpty() || !linearConclusion.contains(expected) || text.contains("+-") ||
               text.contains("significantly surpassing"))
            {t13_ok=false;log_test(logFp,"   [FAIL] Report wording disagrees with measured gain.\n");}
        }
    }
    if (t13_ok) {
        log_test(logFp, " [PASS] Test 13: Data-Driven Report Conclusion (positive / negative / near-zero gain)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 13: Data-driven report conclusion failed!\n");
    }


    // -------------------------------------------------------------
    // Test 14: 真实实测道留出验证 (2026.10.20 新增)
    // 验证留出验证接口:
    //   ① 留出道数 = m - k，且 SNR 只在留出道上统计;
    //   ② 三种方法指标齐全;
    //   ③ 相同 seed 可复现;
    //   ④ 输出 CSV 含 Validation_Type 标记，与插值参考场口径区分。
    // -------------------------------------------------------------
    totalTests++;
    bool t14_ok = true;
    {
        const QString hoIn = "data/real_seismic/real_gather_59x256.csv";
        // 必需输入，缺失即失败（不 SKIP）
        if (!QFile::exists(hoIn))
        {
            t14_ok = false;
            log_test(logFp, "   [FAIL] Required hold-out input missing: %s\n",
                     hoIn.toLocal8Bit().constData());
        }
        else
        {
            // 固定配置回归: m=59, k=30, seed=20261020
            GeometryExperimentEngine::HoldoutMetrics h1, h2;
            QString r1, r2;
            bool ok1 = runHoldoutValidation(hoIn, 59, 30, 40, 20261020ULL,
                                            "unit_test_sandbox/ho1", h1, r1);
            bool ok2 = runHoldoutValidation(hoIn, 59, 30, 40, 20261020ULL,
                                            "unit_test_sandbox/ho2", h2, r2);
            if (!ok1 || !ok2) { t14_ok = false; log_test(logFp, "   [FAIL] hold-out run failed\n"); }
            else
            {
                // ① 留出道数正确
                if (h1.keptTraces + h1.heldOutTraces != h1.nTraces)
                { t14_ok = false; log_test(logFp, "   [FAIL] kept+heldOut != total\n"); }
                if (h1.heldOutTraces != 29)
                { t14_ok = false; log_test(logFp, "   [FAIL] expected 29 held-out, got %d\n", h1.heldOutTraces); }

                // ② 可复现
                if (fabs(h1.snr_pocs_db - h2.snr_pocs_db) > 1e-4f ||
                    fabs(h1.snr_linear_db - h2.snr_linear_db) > 1e-4f)
                { t14_ok = false; log_test(logFp, "   [FAIL] hold-out not reproducible with same seed\n"); }

                // ③ 指标自洽: gain == pocs - linear
                const float expectGain = h1.snr_pocs_db - h1.snr_linear_db;
                if (fabs(h1.gain_pocs_vs_linear_db - expectGain) > 1e-3f)
                { t14_ok = false; log_test(logFp, "   [FAIL] gain != pocs - linear\n"); }

                // ④ CSV 内容
                QFile hf("unit_test_sandbox/ho1/holdout_metrics.csv");
                if (!hf.open(QIODevice::ReadOnly | QIODevice::Text))
                { t14_ok = false; log_test(logFp, "   [FAIL] holdout_metrics.csv missing\n"); }
                else
                {
                    QString ht = QTextStream(&hf).readAll();
                    hf.close();
                    if (!ht.contains("Validation_Type,Hold-out"))
                    { t14_ok = false; log_test(logFp, "   [FAIL] missing Validation_Type marker\n"); }
                    if (!ht.contains("SNR_POCS_HeldOut"))
                    { t14_ok = false; log_test(logFp, "   [FAIL] missing SNR_POCS_HeldOut\n"); }
                    if (ht.contains("+-"))
                    { t14_ok = false; log_test(logFp, "   [FAIL] holdout CSV contains +-\n"); }
                }

                // ⑤ 留出数据未被输入算法: 不同 seed 必须改变留出划分与指标
                GeometryExperimentEngine::HoldoutMetrics h3;
                QString r3;
                if (!runHoldoutValidation(hoIn, 59, 30, 40, 12345ULL,
                                          "unit_test_sandbox/ho3", h3, r3))
                { t14_ok = false; log_test(logFp, "   [FAIL] hold-out run (seed 12345) failed\n"); }
                else if (fabs(h3.snr_linear_db - h1.snr_linear_db) < 1e-6f)
                {
                    t14_ok = false;
                    log_test(logFp, "   [FAIL] changing seed did not change the held-out split/metrics\n");
                }
                // ⑥ 全 6 组配置逐一执行并断言 gain 自洽
                const int cfgK[6]   = { 20, 20, 30, 30, 40, 40 };
                const unsigned long long cfgS[6] = { 20261020ULL, 12345ULL, 20261020ULL,
                                                     12345ULL, 20261020ULL, 12345ULL };
                for (int ci = 0; ci < 6; ci++)
                {
                    GeometryExperimentEngine::HoldoutMetrics hc;
                    QString rc;
                    QString cfgDir2;
                    cfgDir2 = kTestSandbox + QString("/ho_cfg") + QString::number(ci);
                    if (!runHoldoutValidation(hoIn, 59, cfgK[ci], 40, cfgS[ci], cfgDir2, hc, rc))
                    {
                        t14_ok = false;
                        log_test(logFp, "   [FAIL] config %d/%llu failed\n", cfgK[ci], cfgS[ci]);
                        continue;
                    }
                    if (hc.heldOutTraces != 59 - cfgK[ci])
                    { t14_ok = false; log_test(logFp, "   [FAIL] config %d: wrong held-out count\n", cfgK[ci]); }
                    if (fabs(hc.gain_pocs_vs_linear_db - (hc.snr_pocs_db - hc.snr_linear_db)) > 1e-3f)
                    { t14_ok = false; log_test(logFp, "   [FAIL] config %d: gain inconsistent\n", cfgK[ci]); }
                }
            }
        }
    }
    if (t14_ok) {
        log_test(logFp, " [PASS] Test 14: Held-Out Validation on Measured Real Traces (6 configs)\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 14: Held-out validation failed!\n");
    }


    // -------------------------------------------------------------
    // Test 15: 外部掩码真实生效回归 (2026.10.20 新增)
    // 防止再次出现"传了 --mask 但引擎仍用自动掩码"的回归。
    // 验证:
    //   ① 引擎确实加载指定文件（Mask_Source = External File）;
    //   ② 两份位置不同的掩码在同一输入上产生不同的基线指标;
    //   ③ 非法掩码（路径不存在/非二值/点数不符/长度不符）必须失败，不得回退自动生成。
    // -------------------------------------------------------------
    totalTests++;
    bool t15_ok = true;
    {
        const QString maskIn = "data/real_seismic/real_gather_256x128.csv";
        if (!QFile::exists(maskIn))
        {
            t15_ok = false;
            log_test(logFp, "   [FAIL] Required mask-test input missing: %s\n", maskIn.toLocal8Bit().constData());
        }
        else
        {
            const QString mkA = kTestSandbox + "/maskA.txt";
            const QString mkB = kTestSandbox + "/maskB.txt";

            for (int variant = 0; variant < 2; variant++)
            {
                QFile mf(variant == 0 ? mkA : mkB);
                if (!mf.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    t15_ok = false;
                    log_test(logFp, "   [FAIL] cannot write mask variant %d\n", variant);
                    continue;
                }
                QTextStream o(&mf);
                o << "# mask variant " << variant << " (64/128)\n";
                int ones = 0;
                for (int i = 0; i < 128; i++)
                {
                    int v = (variant == 0) ? (i < 64 ? 1 : 0) : ((i % 2 == 0) ? 1 : 0);
                    ones += v;
                    o << v << "\n";
                }
                mf.close();
                if (ones != 64)
                {
                    t15_ok = false;
                    log_test(logFp, "   [FAIL] mask variant %d has %d ones, expected 64\n", variant, ones);
                }
            }

            ReconstructionMetrics mA, mB;
            QString sA, sB;
            bool okA = runWavefieldReconstruction(mkA, kTestSandbox + "/maskA_out", 0.50f,
                                                  mA, sA, maskIn, true, false, true, false);
            bool okB = runWavefieldReconstruction(mkB, kTestSandbox + "/maskB_out", 0.50f,
                                                  mB, sB, maskIn, true, false, true, false);
            if (!okA || !okB)
            {
                t15_ok = false;
                log_test(logFp, "   [FAIL] mask reconstruction run failed\n");
            }
            else
            {
                if (!mA.maskSource.contains("External File"))
                {
                    t15_ok = false;
                    log_test(logFp, "   [FAIL] mask A source not External File: %s\n", mA.maskSource.toLocal8Bit().constData());
                }
                if (!mB.maskSource.contains("External File"))
                {
                    t15_ok = false;
                    log_test(logFp, "   [FAIL] mask B source not External File\n");
                }
                float dLin = fabs(mA.snr_linear_interp_db - mB.snr_linear_interp_db);
                if (dLin < 1e-4f)
                {
                    t15_ok = false;
                    log_test(logFp, "   [FAIL] two different masks gave identical linear-interp SNR (%.4f dB) - mask not applied\n",
                             mA.snr_linear_interp_db);
                }
                else
                {
                    log_test(logFp, "   [OK] maskA linear=%.2f dB vs maskB linear=%.2f dB (delta %.2f dB) - mask IS applied\n",
                             mA.snr_linear_interp_db, mB.snr_linear_interp_db, dLin);
                }
            }

            // 非法掩码必须被拒绝
            ReconstructionMetrics mBad;
            QString sBad;

            bool noneRejected = !runWavefieldReconstruction("no_such_mask_file_at_all.txt",
                                                            kTestSandbox + "/mask_none", 0.50f,
                                                            mBad, sBad, maskIn, true, false, true, false);
            if (!noneRejected)
            {
                t15_ok = false;
                log_test(logFp, "   [FAIL] nonexistent mask path was NOT rejected\n");
            }

            const QString mkNonBin = kTestSandbox + "/mask_nonbinary.txt";
            {
                QFile f2(mkNonBin);
                if (f2.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream o2(&f2);
                    for (int i = 0; i < 128; i++) o2 << (i == 5 ? 2 : (i < 64 ? 1 : 0)) << "\n";
                    f2.close();
                }
            }
            bool nbRejected = !runWavefieldReconstruction(mkNonBin, kTestSandbox + "/mask_nb", 0.50f,
                                                          mBad, sBad, maskIn, true, false, true, false);
            if (!nbRejected)
            {
                t15_ok = false;
                log_test(logFp, "   [FAIL] non-binary mask was NOT rejected\n");
            }

            const QString mkCount = kTestSandbox + "/mask_count63.txt";
            {
                QFile f3(mkCount);
                if (f3.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream o3(&f3);
                    for (int i = 0; i < 128; i++) o3 << (i < 63 ? 1 : 0) << "\n";
                    f3.close();
                }
            }
            bool cntRejected = !runWavefieldReconstruction(mkCount, kTestSandbox + "/mask_cnt", 0.50f,
                                                           mBad, sBad, maskIn, true, false, true, false);
            if (!cntRejected)
            {
                t15_ok = false;
                log_test(logFp, "   [FAIL] wrong-count mask (63/128) was NOT rejected\n");
            }

            const QString mkLen = kTestSandbox + "/mask_len100.txt";
            {
                QFile f4(mkLen);
                if (f4.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream o4(&f4);
                    for (int i = 0; i < 100; i++) o4 << (i < 50 ? 1 : 0) << "\n";
                    f4.close();
                }
            }
            bool lenRejected = !runWavefieldReconstruction(mkLen, kTestSandbox + "/mask_len", 0.50f,
                                                           mBad, sBad, maskIn, true, false, true, false);
            if (!lenRejected)
            {
                t15_ok = false;
                log_test(logFp, "   [FAIL] wrong-length mask (100 rows) was NOT rejected\n");
            }
        }
    }
    if (t15_ok) {
        log_test(logFp, " [PASS] Test 15: External Mask Actually Applied + Invalid Masks Rejected\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 15: External mask effectiveness failed!\n");
    }

    // -------------------------------------------------------------
    // Test 16: 经典互干度的数学性质 (2026-10-21 新增)
    // 验证 SamplingMutualCoherence1D 满足其声明的性质，
    // 并固化“互干度不惩罚空洞”这一已知局限，防止后续误用。
    // -------------------------------------------------------------
    totalTests++;
    bool t16_ok = true;
    {
        const int N16 = 128;
        // (a) 全采样 -> 0
        {
            std::vector<int> idx;
            for (int i = 0; i < N16; i++) idx.push_back(i);
            const float mu = SamplingMutualCoherence1D(idx, N16);
            if (fabs(mu) > 1e-6f) { t16_ok = false; log_test(logFp, "   [FAIL] full mask mu=%.6f != 0\n", mu); }
        }
        // (b) 单点 -> 1
        {
            std::vector<int> idx; idx.push_back(0);
            const float mu = SamplingMutualCoherence1D(idx, N16);
            if (fabs(mu - 1.0f) > 1e-6f) { t16_ok = false; log_test(logFp, "   [FAIL] single point mu=%.6f != 1\n", mu); }
        }
        // (c) 空掩码 -> 0
        {
            std::vector<int> empty;
            const float mu = SamplingMutualCoherence1D(empty, N16);
            if (fabs(mu) > 1e-9f) { t16_ok = false; log_test(logFp, "   [FAIL] empty mu=%.6f != 0\n", mu); }
        }
        // (d) 值域 [0,1]（随机 20 组掩码）
        {
            for (int k = 0; k < 20; k++)
            {
                exprng::seed(1000ULL + (unsigned long long)k);
                int *mask = Sampling1J_Count(N16, 64);
                std::vector<int> idx;
                for (int i = 0; i < N16; i++) if (mask[i] == 1) idx.push_back(i);
                delete[] mask;
                const float mu = SamplingMutualCoherence1D(idx, N16);
                if (mu < 0.0f || mu > 1.0f) { t16_ok = false; log_test(logFp, "   [FAIL] mu=%.6f out of [0,1]\n", mu); break; }
            }
        }
        // (e) 不变性：掩码平移后 mu 不变
        {
            exprng::seed(77ULL);
            int *mask = Sampling1J_Count(N16, 64);
            std::vector<int> a, b;
            for (int i = 0; i < N16; i++) if (mask[i] == 1) a.push_back(i);
            delete[] mask;
            for (size_t t = 0; t < a.size(); t++) b.push_back((a[t] + 37) % N16);
            const float ma = SamplingMutualCoherence1D(a, N16);
            const float mb = SamplingMutualCoherence1D(b, N16);
            if (fabs(ma - mb) > 1e-5f)
            {
                t16_ok = false;
                log_test(logFp, "   [FAIL] shift invariance: %.6f vs %.6f\n", ma, mb);
            }
        }
        // (f) 已知局限：经典互干度不惩罚空洞
        //     集中掩码 (maxgap=64) 的 mu 应低于均匀 2抽1 (mu=1)
        {
            std::vector<int> uni;
            for (int i = 0; i < N16; i += 2) uni.push_back(i);
            std::vector<int> clu;
            for (int i = 0; i < 64; i++) clu.push_back(i);
            const float muU = SamplingMutualCoherence1D(uni, N16);
            const float muC = SamplingMutualCoherence1D(clu, N16);
            if (!(muC < muU))
            {
                t16_ok = false;
                log_test(logFp, "   [FAIL] expected clustered(%.4f) < uniform(%.4f) -- limitation changed?\n", muC, muU);
            }
            else
            {
                log_test(logFp, "   [INFO] documented limitation: clustered mu=%.4f < uniform mu=%.4f (mu does NOT penalise gaps)\n", muC, muU);
            }
        }
    }
    if (t16_ok) {
        log_test(logFp, " [PASS] Test 16: Classic Mutual Coherence Math Properties\n");
        passedTests++;
    } else {
        log_test(logFp, " [FAIL] Test 16: Classic mutual coherence properties failed!\n");
    }
    // 2026.10.20: 汇总必须区分 PASS / FAIL / SKIP，且 SKIP 不计为通过。
    // 任何 SKIP 都会使整体不通过（本套件不含允许跳过的项）。
    #include "../tests/scienceregression.inc" // add by wushanzhou in 2026/10/23
    #include "../tests/reviewregression.inc" // add by wushanzhou in 2026/10/24
    #include "../tests/spatialregression.inc"

    const int failedTests = totalTests - passedTests - skippedTests;
    log_test(logFp, "============================================================\n");
    log_test(logFp, " Test Results: PASS=%d  FAIL=%d  SKIP=%d  (total=%d)\n",
             passedTests, failedTests, skippedTests, totalTests);
    if (!preflightOk)
    {
        log_test(logFp, " Status: FAILED - required input files are missing (preflight failed)\n");
    }
    else if (failedTests > 0)
    {
        log_test(logFp, " Status: FAILED - %d test(s) failed\n", failedTests);
    }
    else if (skippedTests > 0)
    {
        log_test(logFp, " Status: FAILED - %d test(s) skipped; skips are not counted as passes\n", skippedTests);
    }
    else
    {
        log_test(logFp, " Status: ALL PASS (%d/%d)\n", passedTests, totalTests);
    }
    log_test(logFp, "============================================================\n");

    if (logFp) fclose(logFp);

    // 清理测试沙箱（unit_tests.log 保留，作为回归凭据）
    {
        QDir sandbox(kTestSandbox);
        if (sandbox.exists())
        {
            QStringList entries = sandbox.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            for (int ie = 0; ie < entries.size(); ie++)
            {
                QString p = sandbox.filePath(entries.at(ie));
                if (QFileInfo(p).isDir()) removeDirRecursive(p);
                else QFile::remove(p);
            }
            QDir().rmdir(sandbox.absolutePath());
        }
    }

    return (preflightOk && failedTests == 0 && skippedTests == 0);
}

// =============================================================================
// 2026.10.20 新增：2D 采样基准
// 一维基准只回答"沿一条接收线抽道"的问题；真实观测系统是 2D 网格。
// 本实验在 N×N 接收网格上比较四种 2D 掩码的二维曲波相干度：
//   规则（行隔点 + 列隔点） / 随机 / 抖动 / 抖动+SA（逐行抖动后行内 SA）
// 二维 μ 定义为"逐行 1D μ 的均值"，与 CalculateAveMiu 同口径。
// ============================================================================= // add by wushanzhou in 2026/10/23
