// Formal studies share the ClassicPSF search, a 200-evaluation cap and saved masks.
// Included by geometryexperimentengine.cpp to reuse reconstruction and export helpers.

bool GeometryExperimentEngine::runConstrainedComparison(const std::vector<float>& rates,
    const std::vector<unsigned long long>& seeds, const QString& outputDir, QString& report)
{
    QDir().mkpath(outputDir); QDir dir(outputDir);
    QFile file(dir.filePath("constrained_comparison.csv"));
    QFile sensitivity(dir.filePath("lambda_sensitivity.csv"));
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text) ||
        !sensitivity.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream os(&file), ls(&sensitivity);
    os << "scheme,rate,seed,m,mu,max_gap,nn_cv,unit_occupancy,objective,evals,hard_ok,budget_cap,hard_max_gap,status,mask_sha1,runtime_ms\n";
    ls << "lambda,rate,seed,mu,max_gap,nn_cv,unit_occupancy,evals,budget_cap,mask_sha1\n";
    const char* names[5] = {"A_mu_only", "B_jitter_init", "C_soft_l0.3", "D_hard_adaptive", "E_soft_l3.0"};
    for (size_t ri=0; ri<rates.size(); ri++) for (size_t si=0; si<seeds.size(); si++)
    {
        const int n=128, m=(int)floor(n*(double)rates[ri]+0.5);
        exprng::seed(seeds[si]); int* jm=Sampling1J_Count(n,m);
        std::vector<int> initial(jm,jm+n); delete[] jm;
        for (int method=0; method<5; method++)
        {
            ConstraintWeights w;
            if (method==2) w.lambdaGap=w.lambdaNN=0.3;
            if (method==4) w.lambdaGap=w.lambdaNN=3.0;
            if (method==3) w.hardMaxGap=(n+m-1)/m;
            ConstrainedResult result;
            if (!RunSamplingSearch(n,m,initial,w,seeds[si]+1,method==1?1:200,SEARCH_HYBRID,result))
            { report=QString::fromStdString(result.status); return false; }
            QString path=dir.filePath(QString("masks/%1_r%2_seed%3.txt").arg(names[method])
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seeds[si])));
            if (!saveSamplingMask(path,result,seeds[si]+1,method==1?1:200,method==2||method==4?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false;
            os << names[method] << "," << QString::number(rates[ri],'f',2) << "," << QString::number(seeds[si]) << "," << m
               << "," << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv << "," << result.unitOccupancy
               << "," << result.objective << "," << result.evals << "," << (result.hardConstraintSatisfied?1:0)
               << "," << (method==1?1:200) << "," << w.hardMaxGap << "," << QString::fromStdString(result.status) << "," << samplingMaskHash(result.mask)
               << "," << result.runtimeMs << "\n";
        }
        const double lambdas[6]={0,0.1,0.3,1,3,10};
        for (int li=0;li<6;li++)
        {
            ConstraintWeights w; w.lambdaGap=w.lambdaNN=lambdas[li]; ConstrainedResult result;
            if (!RunSamplingSearch(n,m,initial,w,seeds[si]+1,200,SEARCH_HYBRID,result)) return false;
            QString path=dir.filePath(QString("masks/lambda%1_r%2_seed%3.txt").arg(lambdas[li])
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seeds[si])));
            if (!saveSamplingMask(path,result,seeds[si]+1,200,"ClassicPSF+SpatialPenalty")) return false;
            ls << lambdas[li] << "," << QString::number(rates[ri],'f',2) << "," << QString::number(seeds[si])
               << "," << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv
               << "," << result.unitOccupancy << "," << result.evals << ",200," << samplingMaskHash(result.mask) << "\n";
        }
    }
    os.flush(); ls.flush(); report="ClassicPSF constrained study complete; adaptive feasible gap; all masks saved.";
    return file.error()==QFile::NoError && sensitivity.error()==QFile::NoError;
}

bool GeometryExperimentEngine::runMultiSeedAblationExperiment(const std::vector<float>& rates,
    int count, unsigned long long baseSeed, const QString& outputDir, QString& report)
{
    if (count<1) return false;
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("ablation_multiseed_raw.csv"));
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream os(&file);
    os << "rate,seed_index,seed,mu_jitter_init,mu_sa_only,mu_local_only,mu_sa_local,mu_restart_best,mu_constrained,"
          "evals_sa_only,evals_local_only,evals_sa_local,evals_restart,evals_constrained,max_gap_sa_local,max_gap_constrained,"
          "nn_cv_sa_local,nn_cv_constrained,protocol,budget_cap,sa_prefix_mu,sa_prefix_equal,sa_prefix_sha1,"
          "runtime_sa_ms,runtime_local_ms,runtime_hybrid_ms,runtime_restart_ms,runtime_constrained_ms\n";
    const char* names[5]={"SA","Local","Hybrid","Restart","Constrained"};
    const SamplingSearchMethod methods[5]={SEARCH_SA,SEARCH_LOCAL,SEARCH_HYBRID,SEARCH_RESTART,SEARCH_HYBRID};
    for (size_t ri=0;ri<rates.size();ri++) for (int k=0;k<count;k++)
    {
        const int n=128,m=(int)floor(n*(double)rates[ri]+0.5);
        const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL;
        exprng::seed(seed); int* jm=Sampling1J_Count(n,m);
        std::vector<int> initial(jm,jm+n); delete[] jm;
        ConstrainedResult result[5], init; ConstraintWeights noWeights;
        if (!RunSamplingSearch(n,m,initial,noWeights,seed+1,1,SEARCH_LOCAL,init) ||
            !saveSamplingMask(dir.filePath(QString("masks/Init_r%1_seed%2.txt").arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed))),init,seed,1,"ClassicPSF")) return false;
        for (int j=0;j<5;j++)
        {
            ConstraintWeights w; if (j==4) w.lambdaGap=w.lambdaNN=0.3;
            if (!RunSamplingSearch(n,m,initial,w,seed+1,200,methods[j],result[j])) return false;
            QString path=dir.filePath(QString("masks/%1_r%2_seed%3.txt").arg(names[j])
                .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed)));
            if (!saveSamplingMask(path,result[j],seed+1,200,j==4?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false;
        }
        bool same=result[0].saPrefixMask==result[2].saPrefixMask;
        if (!same) { report="SA prefixes diverged"; return false; }
        ConstrainedResult prefix=result[2];prefix.mask=result[2].saPrefixMask;
        prefix.mu=SamplingMaskCoherence(prefix.mask);prefix.objective=prefix.mu;
        prefix.maxGap=SamplingMaxGap(prefix.mask);prefix.nnCv=SamplingNNCv(prefix.mask);
        prefix.unitOccupancy=SamplingUnitOccupancy(prefix.mask);prefix.evals=64;
        prefix.bestTrace.resize(64);prefix.currentTrace.resize(64);prefix.status="shared_sa_prefix";
        if(!saveSamplingMask(dir.filePath(QString("masks/SAPrefix_r%1_seed%2.txt")
            .arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed))),prefix,seed+1,64,"ClassicPSF"))return false;
        os << QString::number(rates[ri],'f',2) << "," << k << "," << QString::number(seed) << "," << QString::number(init.mu,'g',9);
        for (int j=0;j<5;j++) os << "," << QString::number(result[j].mu,'g',9);
        for (int j=0;j<5;j++) os << "," << result[j].evals;
        os << "," << result[2].maxGap << "," << result[4].maxGap << "," << result[2].nnCv << "," << result[4].nnCv
           << ",classic_psf_v2,200," << SamplingMaskCoherence(result[2].saPrefixMask) << ",1," << samplingMaskHash(result[2].saPrefixMask);
        for (int j=0;j<5;j++) os << "," << result[j].runtimeMs;
        os << "\n";
    }
    os.flush(); report="ClassicPSF ablation complete; common cap=200; shared SA prefix=64; constrained objective reported separately.";
    return file.error()==QFile::NoError;
}

bool GeometryExperimentEngine::runMaskReconStudy(const QString& outputDir,int count,
    unsigned long long baseSeed,QString& report,const QString& inputCsv)
{
    if (count<1) return false;
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("mask_recon_study.csv"));
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream os(&file);
    os << "rate,mask_type,instance,m,mu,max_gap,nn_cv,snr_zero_full,snr_linear_full,snr_pocs_full,"
          "snr_linear_missing,snr_pocs_missing,gain_vs_linear_missing,seed,budget_cap,evals,objective,mask_sha1,input_sha1\n";
    const float rates[3]={0.30f,0.50f,0.70f};
    const char* names[6]={"JitterInit","SA","Restart","Constrained","Random","Regular"};
    QString inputHash="builtin_three_layer_v1";
    if (!inputCsv.isEmpty())
    {
        QFile input(inputCsv); if (!input.open(QIODevice::ReadOnly)) return false;
        inputHash=QString(QCryptographicHash::hash(input.readAll(),QCryptographicHash::Sha1).toHex());
    }
    for (int ri=0;ri<3;ri++) for (int k=0;k<count;k++)
    {
        const int n=128,m=(int)floor(n*(double)rates[ri]+0.5);
        const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL;
        exprng::seed(seed); int* jm=Sampling1J_Count(n,m);
        std::vector<int> initial(jm,jm+n); delete[] jm;
        for (int type=0;type<6;type++)
        {
            ConstraintWeights w; ConstrainedResult result; std::vector<int> selected=initial;
            if (type==3) w.lambdaGap=w.lambdaNN=0.3;
            if (type==4 || type==5)
            {
                exprng::seed(seed+7); int* mask=type==4?Sampling1S_Count(n,m):Sampling1R_Count(n,m);
                selected.assign(mask,mask+n); delete[] mask;
            }
            const int budget=(type==0||type>=4)?1:200;
            const SamplingSearchMethod method=type==2?SEARCH_RESTART:SEARCH_HYBRID;
            if (!RunSamplingSearch(n,m,selected,w,seed+1,budget,method,result)) return false;
            QString tag=QString("%1_r%2_seed%3").arg(names[type]).arg((int)(rates[ri]*100+0.5f)).arg(QString::number(seed));
            QString path=dir.filePath("masks/"+tag+".txt");
            if (!saveSamplingMask(path,result,seed+1,budget,type==3?"ClassicPSF+SpatialPenalty":"ClassicPSF")) return false;
            ReconstructionMetrics metrics; QString summary;
            if (!runWavefieldReconstruction(path,dir.filePath("run_"+tag),rates[ri],metrics,summary,inputCsv,false,false,true,false))
            { report=summary; return false; }
            os << QString::number(rates[ri],'f',2) << "," << names[type] << "," << k << "," << m << ","
               << QString::number(result.mu,'g',9) << "," << result.maxGap << "," << result.nnCv << ","
               << metrics.snr_zero_fill_db << "," << metrics.snr_linear_interp_db << "," << metrics.snr_recovered_db << ","
               << metrics.snr_linear_missing_db << "," << metrics.snr_pocs_missing_db << ","
               << metrics.snr_pocs_missing_db-metrics.snr_linear_missing_db << "," << QString::number(seed) << ","
               << budget << "," << result.evals << "," << result.objective << "," << samplingMaskHash(result.mask) << "," << inputHash << "\n";
            os.flush();
        }
    }
    report="Six mask types reconstructed, including the actual constrained optimum. See mask_recon_study.csv.";
    return file.error()==QFile::NoError;
}

bool GeometryExperimentEngine::runSampling2DBenchmark(int grid,const std::vector<float>& rates,
    unsigned long long seed,const QString& outputDir,QString& report)
{
    if (grid<4 || grid>256) { report="grid must be in [4,256]"; return false; }
    QDir().mkpath(outputDir); QDir dir(outputDir); QFile file(dir.filePath("sampling_2d_benchmark.csv"));
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream os(&file); os << "grid,rate,points,method,true_2d_mu,evals,budget_cap,mask_sha1\n";
    const char* names[5]={"Regular","Random","JitterFlattened","HybridTrue2D","RestartTrue2D"};
    for (size_t ri=0;ri<rates.size();ri++)
    {
        const int n=grid*grid,m=(int)floor(n*(double)rates[ri]+0.5);
        exprng::seed(seed+ri); int* jm=Sampling1J_Count(n,m);
        std::vector<int> initial(jm,jm+n); delete[] jm;
        for (int type=0;type<5;type++)
        {
            std::vector<int> selected=initial;
            if (type<2)
            {
                exprng::seed(seed+ri); int* bits=type==0?Sampling1R_Count(n,m):Sampling1S_Count(n,m);
                selected.assign(bits,bits+n); delete[] bits;
            }
            ConstraintWeights w; ConstrainedResult result; const int budget=type<3?1:200;
            if (!RunSamplingSearch(n,m,selected,w,seed+ri+1,budget,type==4?SEARCH_RESTART:SEARCH_HYBRID,result,64,grid)) return false;
            QString path=dir.filePath(QString("masks/%1_r%2.txt").arg(names[type]).arg((int)(rates[ri]*100+0.5f)));
            if (!saveSamplingMask(path,result,seed+ri+1,budget,"ClassicPSF_True2D")) return false;
            os << grid << "," << QString::number(rates[ri],'f',2) << "," << m << "," << names[type]
               << "," << QString::number(result.mu,'g',9) << "," << result.evals << "," << budget << "," << samplingMaskHash(result.mask) << "\n";
        }
    }
    os.flush(); report="True two-dimensional objective used for selection and reporting; masks are identical to the measured masks.";
    return file.error()==QFile::NoError;
}

bool GeometryExperimentEngine::runRealMaskStudy(const QString& input,int count,int iterations,
    unsigned long long baseSeed,const QString& outputDir,QString& report)
{
    if(count<1 || iterations<1)return false;
    std::vector<std::vector<double> > data;std::vector<double> x;
    if(!loadHoldoutGather(input,data,x,report))return false;
    const int n=(int)x.size();
    QDir().mkpath(outputDir);QDir dir(outputDir);QFile file(dir.filePath("real_mask_study.csv"));
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return false;
    QTextStream os(&file);
    os << "keep,total,seed,mask_type,mu_index_psf,max_gap_m,nn_cv_index,snr_zero_heldout,snr_linear_heldout,"
          "snr_pocs_heldout,gain_vs_linear,gain_vs_zero,rmse_pocs,consistency_passed,mask_sha1,input_sha1,evals,budget_cap\n";
    const char* names[6]={"JitterInit","Hybrid","Restart","Constrained","Random","Regular"};
    const int keeps[3]={(n+2)/3,(n+1)/2,(2*n+2)/3};
    for(int ri=0;ri<3;ri++)for(int k=0;k<count;k++)
    {
        const int m=keeps[ri];const unsigned long long seed=baseSeed+(unsigned long long)k*179ULL;
        exprng::seed(seed);int* jm=Sampling1J_Count(n,m);std::vector<int> initial(jm,jm+n);delete[] jm;
        for(int type=0;type<6;type++)
        {
            std::vector<int> selected=initial;ConstraintWeights w;ConstrainedResult result;
            if(type==3)w.lambdaGap=w.lambdaNN=0.3;
            if(type>=4)
            {
                exprng::seed(seed+7);int* bits=type==4?Sampling1S_Count(n,m):Sampling1R_Count(n,m);
                selected.assign(bits,bits+n);delete[] bits;
            }
            const int budget=type==0||type>=4?1:200;
            if(!RunSamplingSearch(n,m,selected,w,seed+1,budget,type==2?SEARCH_RESTART:SEARCH_HYBRID,result))return false;
            QString tag=QString("%1_keep%2_seed%3").arg(names[type]).arg(m).arg(QString::number(seed));
            QString path=dir.filePath("masks/"+tag+".txt");
            if(!saveSamplingMask(path,result,seed+1,budget,type==3?"ClassicPSF_IndexOrder+IndexSpatialPenalty":"ClassicPSF_IndexOrder"))return false;
            HoldoutMetrics metrics;QString summary;
            if(!runHoldoutValidation(input,n,m,iterations,seed,dir.filePath("run_"+tag),metrics,summary,path))
            {report=summary;return false;}
            double gap=0,last=x.front();
            for(int j=0;j<n;j++)if(result.mask[j]){gap=std::max(gap,x[j]-last);last=x[j];}
            gap=std::max(gap,x.back()-last);
            os << m << "," << n << "," << QString::number(seed) << "," << names[type] << "," << result.mu
               << "," << gap << "," << result.nnCv << "," << metrics.snr_zero_fill_db << "," << metrics.snr_linear_db
               << "," << metrics.snr_pocs_db << "," << metrics.gain_pocs_vs_linear_db << "," << metrics.gain_pocs_vs_zero_db
               << "," << metrics.rmse_pocs << "," << (metrics.dataConsistencyPassed?1:0) << "," << samplingMaskHash(result.mask)
               << "," << metrics.inputSha1 << "," << result.evals << "," << budget << "\n";
            os.flush();
        }
    }
    report="Measured-trace mask study complete. Selection objective uses sorted trace indices; reconstruction uses actual offsets. One independent real source.";
    return file.error()==QFile::NoError;
}
