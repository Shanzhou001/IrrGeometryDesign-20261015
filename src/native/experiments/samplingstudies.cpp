#include "experiment_internal.h"

// Formal studies share the ClassicPSF search, a 200-evaluation cap and saved masks. // add by wushanzhou in 2026/10/23
// Included by geometryexperimentengine.cpp to reuse reconstruction and export helpers. // add by wushanzhou in 2026/10/23

bool GeometryExperimentEngine::runConstrainedComparison(const std::vector<float>& rates, // add by wushanzhou in 2026/10/23
    const std::vector<unsigned long long>& seeds, const QString& outputDir, QString& report) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    QDir().mkpath(outputDir); QDir dir(outputDir); // add by wushanzhou in 2026/10/23
    QFile file(dir.filePath("constrained_comparison.csv")); // add by wushanzhou in 2026/10/23
    QFile sensitivity(dir.filePath("lambda_sensitivity.csv")); // add by wushanzhou in 2026/10/23
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text) || // add by wushanzhou in 2026/10/23
        !sensitivity.open(QIODevice::WriteOnly|QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&file), ls(&sensitivity); // add by wushanzhou in 2026/10/23
    os << "scheme,rate,seed,m,mu,max_gap,nn_cv,unit_occupancy,objective,evals,hard_ok,budget_cap,hard_max_gap,status,mask_sha1,runtime_ms\n"; // add by wushanzhou in 2026/10/23
    ls << "lambda,rate,seed,mu,max_gap,nn_cv,unit_occupancy,evals,budget_cap,mask_sha1\n"; // add by wushanzhou in 2026/10/23
    const char* names[5] = {"A_mu_only", "B_jitter_init", "C_soft_l0.3", "D_hard_adaptive", "E_soft_l3.0"}; // add by wushanzhou in 2026/10/23
    for (size_t ri=0; ri<rates.size(); ri++) for (size_t si=0; si<seeds.size(); si++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int n=128, m=(int)floor(n*(double)rates[ri]+0.5); // add by wushanzhou in 2026/10/23
        exprng::seed(seeds[si]); int* jm=Sampling1J_Count(n,m); // add by wushanzhou in 2026/10/23
        std::vector<int> initial(jm,jm+n); delete[] jm; // add by wushanzhou in 2026/10/23
        for (int method=0; method<5; method++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            ConstraintWeights w; // add by wushanzhou in 2026/10/23
            if (method==2) w.lambdaGap=w.lambdaNN=0.3; // add by wushanzhou in 2026/10/23
            if (method==4) w.lambdaGap=w.lambdaNN=3.0; // add by wushanzhou in 2026/10/23
            if (method==3) w.hardMaxGap=(n+m-1)/m; // add by wushanzhou in 2026/10/23
            ConstrainedResult result; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(n,m,initial,w,seeds[si]+1,method==1?1:200,SEARCH_HYBRID,result)) // add by wushanzhou in 2026/10/23
            { report=QString::fromStdString(result.status); return false; } // add by wushanzhou in 2026/10/23
            QString path=dir.filePath(QString("masks/%1_r%2_seed%3.txt").arg(names[method]) // add by wushanzhou in 2026/10/23
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seeds[si]))); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(path,result,seeds[si]+1,method==1?1:200,method==2||method==4?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false; // add by wushanzhou in 2026/10/23
            os << names[method] << "," << QString::number(rates[ri],'f',2) << "," << QString::number(seeds[si]) << "," << m // add by wushanzhou in 2026/10/23
               << "," << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv << "," << result.unitOccupancy // add by wushanzhou in 2026/10/23
               << "," << result.objective << "," << result.evals << "," << (result.hardConstraintSatisfied?1:0) // add by wushanzhou in 2026/10/23
               << "," << (method==1?1:200) << "," << w.hardMaxGap << "," << QString::fromStdString(result.status) << "," << samplingMaskHash(result.mask) // add by wushanzhou in 2026/10/23
               << "," << result.runtimeMs << "\n"; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        const double lambdas[6]={0,0.1,0.3,1,3,10}; // add by wushanzhou in 2026/10/23
        for (int li=0;li<6;li++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            ConstraintWeights w; w.lambdaGap=w.lambdaNN=lambdas[li]; ConstrainedResult result; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(n,m,initial,w,seeds[si]+1,200,SEARCH_HYBRID,result)) return false; // add by wushanzhou in 2026/10/23
            QString path=dir.filePath(QString("masks/lambda%1_r%2_seed%3.txt").arg(lambdas[li]) // add by wushanzhou in 2026/10/23
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seeds[si]))); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(path,result,seeds[si]+1,200,"ClassicPSF+SpatialPenalty")) return false; // add by wushanzhou in 2026/10/23
            ls << lambdas[li] << "," << QString::number(rates[ri],'f',2) << "," << QString::number(seeds[si]) // add by wushanzhou in 2026/10/23
               << "," << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv // add by wushanzhou in 2026/10/23
               << "," << result.unitOccupancy << "," << result.evals << ",200," << samplingMaskHash(result.mask) << "\n"; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    os.flush(); ls.flush(); report="ClassicPSF constrained study complete; adaptive feasible gap; all masks saved."; // add by wushanzhou in 2026/10/23
    return file.error()==QFile::NoError && sensitivity.error()==QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

bool GeometryExperimentEngine::runMultiSeedAblationExperiment(const std::vector<float>& rates, // add by wushanzhou in 2026/10/23
    int count, unsigned long long baseSeed, const QString& outputDir, QString& report) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    if (count<1) return false; // add by wushanzhou in 2026/10/23
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("ablation_multiseed_raw.csv")); // add by wushanzhou in 2026/10/23
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&file); // add by wushanzhou in 2026/10/23
    os << "rate,seed_index,seed,mu_jitter_init,mu_sa_only,mu_local_only,mu_sa_local,mu_restart_best,mu_constrained," // add by wushanzhou in 2026/10/23
          "evals_sa_only,evals_local_only,evals_sa_local,evals_restart,evals_constrained,max_gap_sa_local,max_gap_constrained," // add by wushanzhou in 2026/10/23
          "nn_cv_sa_local,nn_cv_constrained,protocol,budget_cap,sa_prefix_mu,sa_prefix_equal,sa_prefix_sha1," // add by wushanzhou in 2026/10/23
          "runtime_sa_ms,runtime_local_ms,runtime_hybrid_ms,runtime_restart_ms,runtime_constrained_ms\n"; // add by wushanzhou in 2026/10/23
    const char* names[5]={"SA","Local","Hybrid","Restart","Constrained"}; // add by wushanzhou in 2026/10/23
    const SamplingSearchMethod methods[5]={SEARCH_SA,SEARCH_LOCAL,SEARCH_HYBRID,SEARCH_RESTART,SEARCH_HYBRID}; // add by wushanzhou in 2026/10/23
    for (size_t ri=0;ri<rates.size();ri++) for (int k=0;k<count;k++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int n=128,m=(int)floor(n*(double)rates[ri]+0.5); // add by wushanzhou in 2026/10/23
        const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL; // add by wushanzhou in 2026/10/23
        exprng::seed(seed); int* jm=Sampling1J_Count(n,m); // add by wushanzhou in 2026/10/23
        std::vector<int> initial(jm,jm+n); delete[] jm; // add by wushanzhou in 2026/10/23
        ConstrainedResult result[5], init; ConstraintWeights noWeights; // add by wushanzhou in 2026/10/23
        if (!RunSamplingSearch(n,m,initial,noWeights,seed+1,1,SEARCH_LOCAL,init) || // add by wushanzhou in 2026/10/23
            !saveSamplingMask(dir.filePath(QString("masks/Init_r%1_seed%2.txt").arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed))),init,seed,1,"ClassicPSF")) return false; // add by wushanzhou in 2026/10/23
        for (int j=0;j<5;j++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            ConstraintWeights w; if (j==4) w.lambdaGap=w.lambdaNN=0.3; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(n,m,initial,w,seed+1,200,methods[j],result[j])) return false; // add by wushanzhou in 2026/10/23
            QString path=dir.filePath(QString("masks/%1_r%2_seed%3.txt").arg(names[j]) // add by wushanzhou in 2026/10/23
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed))); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(path,result[j],seed+1,200,j==4?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        bool same=result[0].saPrefixMask==result[2].saPrefixMask; // add by wushanzhou in 2026/10/23
        if (!same) { report="SA prefixes diverged"; return false; } // add by wushanzhou in 2026/10/23
        ConstrainedResult prefix=result[2];prefix.mask=result[2].saPrefixMask; // add by wushanzhou in 2026/10/23
        prefix.mu=SamplingMaskCoherence(prefix.mask);prefix.objective=prefix.mu; // add by wushanzhou in 2026/10/23
        prefix.maxGap=SamplingMaxGap(prefix.mask);prefix.nnCv=SamplingNNCv(prefix.mask); // add by wushanzhou in 2026/10/23
        prefix.unitOccupancy=SamplingUnitOccupancy(prefix.mask);prefix.evals=64; // add by wushanzhou in 2026/10/23
        prefix.bestTrace.resize(64);prefix.currentTrace.resize(64);prefix.status="shared_sa_prefix"; // add by wushanzhou in 2026/10/23
        if(!saveSamplingMask(dir.filePath(QString("masks/SAPrefix_r%1_seed%2.txt") // add by wushanzhou in 2026/10/23
            .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed))),prefix,seed+1,64,"ClassicPSF"))return false; // add by wushanzhou in 2026/10/23
        os << QString::number(rates[ri],'f',2) << "," << k << "," << QString::number(seed) << "," << QString::number(init.mu,'g',9); // add by wushanzhou in 2026/10/23
        for (int j=0;j<5;j++) os << "," << QString::number(result[j].mu,'g',9); // add by wushanzhou in 2026/10/23
        for (int j=0;j<5;j++) os << "," << result[j].evals; // add by wushanzhou in 2026/10/23
        os << "," << result[2].maxGap << "," << result[4].maxGap << "," << result[2].nnCv << "," << result[4].nnCv // add by wushanzhou in 2026/10/23
           << ",classic_psf_v2,200," << SamplingMaskCoherence(result[2].saPrefixMask) << ",1," << samplingMaskHash(result[2].saPrefixMask); // add by wushanzhou in 2026/10/23
        for (int j=0;j<5;j++) os << "," << result[j].runtimeMs; // add by wushanzhou in 2026/10/23
        os << "\n"; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    os.flush(); report="ClassicPSF ablation complete; common cap=200; shared SA prefix=64; constrained objective reported separately."; // add by wushanzhou in 2026/10/23
    return file.error()==QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

bool GeometryExperimentEngine::runMaskReconStudy(const QString& outputDir,int count, // add by wushanzhou in 2026/10/23
    unsigned long long baseSeed,QString& report,const QString& inputCsv) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    if (count<1) return false; // add by wushanzhou in 2026/10/23
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("mask_recon_study.csv")); // add by wushanzhou in 2026/10/23
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&file); // add by wushanzhou in 2026/10/23
    os << "rate,mask_type,instance,m,mu,max_gap,nn_cv,snr_zero_full,snr_linear_full,snr_pocs_full," // add by wushanzhou in 2026/10/23
          "snr_linear_missing,snr_pocs_missing,gain_vs_linear_missing,seed,budget_cap,evals,objective,mask_sha1,input_sha1\n"; // add by wushanzhou in 2026/10/23
    const float rates[3]={0.30f,0.50f,0.70f}; // add by wushanzhou in 2026/10/23
    const char* names[7]={"JitterInit","Hybrid","Restart","Constrained","Random","Regular","SAOnly"}; // add by wushanzhou in 2026/10/23
    QString inputHash="builtin_three_layer_v1"; // add by wushanzhou in 2026/10/23
    if (!inputCsv.isEmpty()) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        QFile input(inputCsv); if (!input.open(QIODevice::ReadOnly)) return false; // add by wushanzhou in 2026/10/23
        inputHash=QString(QCryptographicHash::hash(input.readAll(),QCryptographicHash::Sha1).toHex()); // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    for (int ri=0;ri<3;ri++) for (int k=0;k<count;k++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int n=128,m=(int)floor(n*(double)rates[ri]+0.5); // add by wushanzhou in 2026/10/23
        const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL; // add by wushanzhou in 2026/10/23
        exprng::seed(seed); int* jm=Sampling1J_Count(n,m); // add by wushanzhou in 2026/10/23
        std::vector<int> initial(jm,jm+n); delete[] jm; // add by wushanzhou in 2026/10/23
        for (int type=0;type<7;type++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            ConstraintWeights w; ConstrainedResult result; std::vector<int> selected=initial; // add by wushanzhou in 2026/10/23
            if (type==3) w.lambdaGap=w.lambdaNN=0.3; // add by wushanzhou in 2026/10/23
            if (type==4 || type==5) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                exprng::seed(seed+7); int* mask=type==4?Sampling1S_Count(n,m):Sampling1R_Count(n,m); // add by wushanzhou in 2026/10/23
                selected.assign(mask,mask+n); delete[] mask; // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            const int budget=(type==0||type==4||type==5)?1:200; // add by wushanzhou in 2026/10/23
            const SamplingSearchMethod method=type==6?SEARCH_SA:type==2?SEARCH_RESTART:SEARCH_HYBRID; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(n,m,selected,w,seed+1,budget,method,result)) return false; // add by wushanzhou in 2026/10/23
            QString tag=QString("%1_r%2_seed%3").arg(names[type]).arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed)); // add by wushanzhou in 2026/10/23
            QString path=dir.filePath("masks/"+tag+".txt"); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(path,result,seed+1,budget,type==3?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false; // add by wushanzhou in 2026/10/23
            ReconstructionMetrics metrics; QString summary; // add by wushanzhou in 2026/10/23
            if (!runWavefieldReconstruction(path,dir.filePath("run_"+tag),rates[ri],metrics,summary,inputCsv,false,false,true,false)) // add by wushanzhou in 2026/10/23
            { report=summary; return false; } // add by wushanzhou in 2026/10/23
            os << QString::number(rates[ri],'f',2) << "," << names[type] << "," << k << "," << m << "," // add by wushanzhou in 2026/10/23
               << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv << "," // add by wushanzhou in 2026/10/23
               << metrics.snr_zero_fill_db << "," << metrics.snr_linear_interp_db << "," << metrics.snr_recovered_db << "," // add by wushanzhou in 2026/10/23
               << metrics.snr_linear_missing_db << "," << metrics.snr_pocs_missing_db << "," // add by wushanzhou in 2026/10/23
               << metrics.snr_pocs_missing_db-metrics.snr_linear_missing_db << "," << QString::number(seed) << "," // add by wushanzhou in 2026/10/23
               << budget << "," << result.evals << "," << result.objective << "," << samplingMaskHash(result.mask) << "," << inputHash << "\n"; // add by wushanzhou in 2026/10/23
            os.flush(); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    report="Seven mask types reconstructed, including distinct SAOnly, Hybrid and constrained optima. See mask_recon_study.csv."; // add by wushanzhou in 2026/10/23
    return file.error()==QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

bool GeometryExperimentEngine::runSampling2DBenchmark(int grid,const std::vector<float>& rates, // add by wushanzhou in 2026/10/23
    unsigned long long seed,const QString& outputDir,QString& report,int repeats) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    if (grid<4 || grid>256 || repeats<1) { report="grid must be in [4,256], repeats >= 1"; return false; } // add by wushanzhou in 2026/10/23
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("sampling_2d_benchmark.csv")); // add by wushanzhou in 2026/10/23
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&file); os << "grid,rate,points,method,true_2d_mu,evals,budget_cap,mask_sha1,seed,instance\n"; // add by wushanzhou in 2026/10/23
    const char* names[5]={"Regular","Random","JitterFlattened","HybridTrue2D","RestartTrue2D"}; // add by wushanzhou in 2026/10/23
    for(int instance=0;instance<repeats;instance++) for (size_t ri=0;ri<rates.size();ri++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int n=grid*grid,m=(int)floor(n*(double)rates[ri]+0.5); // add by wushanzhou in 2026/10/23
        const unsigned long long runSeed=seed+(unsigned long long)instance*179ULL; // add by wushanzhou in 2026/10/23
        exprng::seed(runSeed+ri); int* jm=Sampling1J_Count(n,m); // add by wushanzhou in 2026/10/23
        std::vector<int> initial(jm,jm+n); delete[] jm; // add by wushanzhou in 2026/10/23
        for (int type=0;type<5;type++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            std::vector<int> selected=initial; // add by wushanzhou in 2026/10/23
            if (type<2) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                exprng::seed(runSeed+ri); int* bits=type==0?Sampling1R_Count(n,m):Sampling1S_Count(n,m); // add by wushanzhou in 2026/10/23
                selected.assign(bits,bits+n); delete[] bits; // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            ConstraintWeights w; ConstrainedResult result; const int budget=type<3?1:200; // add by wushanzhou in 2026/10/23
            if (!RunSamplingSearch(n,m,selected,w,runSeed+ri+1,budget,type==4?SEARCH_RESTART:SEARCH_HYBRID,result,64,grid)) return false; // add by wushanzhou in 2026/10/23
            QString path=dir.filePath(QString("masks/%1_r%2_seed%3.txt").arg(names[type]).arg((int)(rates[ri]*100+0.5f)).arg(QString::number(runSeed))); // add by wushanzhou in 2026/10/23
            if (!saveSamplingMask(path,result,runSeed+ri+1,budget,"ClassicPSF_True2D")) return false; // add by wushanzhou in 2026/10/23
            os << grid << "," << QString::number(rates[ri],'f',2) << "," << m << "," << names[type] // add by wushanzhou in 2026/10/23
               << "," << QString::number(result.mu,'g',9) << "," << result.evals << "," << budget << "," << samplingMaskHash(result.mask) // add by wushanzhou in 2026/10/23
               << "," << QString::number(runSeed) << "," << instance << "\n"; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    os.flush(); report="True two-dimensional objective used for selection and reporting; masks are identical to the measured masks."; // add by wushanzhou in 2026/10/23
    return file.error()==QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

bool GeometryExperimentEngine::runRealMaskStudy(const QString& input,int count,int iterations, // add by wushanzhou in 2026/10/23
    unsigned long long baseSeed,const QString& outputDir,QString& report) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    if(count<1 || iterations<1)return false; // add by wushanzhou in 2026/10/23
    std::vector<std::vector<double> > data;std::vector<double> x; // add by wushanzhou in 2026/10/23
    if(!loadHoldoutGather(input,data,x,report))return false; // add by wushanzhou in 2026/10/23
    const int n=(int)x.size(); // add by wushanzhou in 2026/10/23
    QDir().mkpath(outputDir);QDir dir(outputDir);QFile file(dir.filePath("real_mask_study.csv")); // add by wushanzhou in 2026/10/23
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&file); // add by wushanzhou in 2026/10/23
    os << "keep,total,seed,mask_type,mu_physical_psf,max_gap_m,nn_cv_physical,snr_zero_heldout,snr_linear_heldout," // add by wushanzhou in 2026/10/23
          "snr_pocs_heldout,gain_vs_linear,gain_vs_zero,rmse_pocs,consistency_passed,mask_sha1,input_sha1,evals,budget_cap,objective,period_m,frequency_step_per_m\n"; // add by wushanzhou in 2026/10/23
    const char* names[7]={"JitterInit","Hybrid","Restart","Constrained","Random","Regular","SAOnly"}; // add by wushanzhou in 2026/10/23
    const int keeps[3]={(n+2)/3,(n+1)/2,(2*n+2)/3}; // add by wushanzhou in 2026/10/23
    for(int ri=0;ri<3;ri++)for(int k=0;k<count;k++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int m=keeps[ri];const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL; // add by wushanzhou in 2026/10/23
        exprng::seed(seed);int* jm=Sampling1J_Count(n,m);std::vector<int> initial(jm,jm+n);delete[] jm; // add by wushanzhou in 2026/10/23
        for(int type=0;type<7;type++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            std::vector<int> selected=initial;ConstraintWeights w;ConstrainedResult result; // add by wushanzhou in 2026/10/23
            if(type==3)w.lambdaGap=w.lambdaNN=0.3; // add by wushanzhou in 2026/10/23
            if(type==4||type==5) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                exprng::seed(seed+7);int* bits=type==4?Sampling1S_Count(n,m):Sampling1R_Count(n,m); // add by wushanzhou in 2026/10/23
                selected.assign(bits,bits+n);delete[] bits; // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            const int budget=type==0||type==4||type==5?1:200; // add by wushanzhou in 2026/10/23
            if(!RunSamplingSearch(n,m,selected,w,seed+1,budget,type==6?SEARCH_SA:type==2?SEARCH_RESTART:SEARCH_HYBRID,result,64,1,&x))return false; // add by wushanzhou in 2026/10/23
            QString tag=QString("%1_keep%2_seed%3").arg(names[type]).arg(m).arg(QString::number(seed)); // add by wushanzhou in 2026/10/23
            QString path=dir.filePath("masks/"+tag+".txt"); // add by wushanzhou in 2026/10/23
            if(!saveSamplingMask(path,result,seed+1,budget,type==3?"PhysicalFourierPSF+PhysicalSpatialPenalty":"PhysicalFourierPSF"))return false; // add by wushanzhou in 2026/10/23
            HoldoutMetrics metrics;QString summary; // add by wushanzhou in 2026/10/23
            if(!runHoldoutValidation(input,n,m,iterations,seed,dir.filePath("run_"+tag),metrics,summary,path)) // add by wushanzhou in 2026/10/23
            {report=summary;return false;} // add by wushanzhou in 2026/10/23
            const double period=(x.back()-x.front())*n/(n-1); // add by wushanzhou in 2026/10/23
            os << m << "," << n << "," << QString::number(seed) << "," << names[type] << "," << result.mu // add by wushanzhou in 2026/10/23
               << "," << QString::number(result.maxGapM,'g',12) << "," << result.nnCv << "," << metrics.snr_zero_fill_db << "," << metrics.snr_linear_db // add by wushanzhou in 2026/10/23
               << "," << metrics.snr_pocs_db << "," << metrics.gain_pocs_vs_linear_db << "," << metrics.gain_pocs_vs_zero_db // add by wushanzhou in 2026/10/23
               << "," << metrics.rmse_pocs << "," << (metrics.dataConsistencyPassed?1:0) << "," << samplingMaskHash(result.mask) // add by wushanzhou in 2026/10/23
               << "," << metrics.inputSha1 << "," << result.evals << "," << budget << "," << result.objective // add by wushanzhou in 2026/10/23
               << "," << QString::number(period,'g',15) << "," << QString::number(1.0/period,'g',15) << "\n"; // add by wushanzhou in 2026/10/23
            os.flush(); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    report="Measured-trace study uses physical-coordinate Fourier PSF, physical soft constraints, and actual-offset reconstruction. Seven methods; one independent real source."; // add by wushanzhou in 2026/10/23
    return file.error()==QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23
