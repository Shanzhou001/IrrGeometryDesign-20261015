#include "experiment_internal.h"

// New experiments never share a directory or protocol label with archived v3 results. add by wushanzhou in 2026/10/24
bool GeometryExperimentEngine::runReviewUnitTests() // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    int totalTests=0,passedTests=0;FILE* logFp=NULL; // add by wushanzhou in 2026/10/24
    #include "../tests/reviewregression.inc" // add by wushanzhou in 2026/10/24
    return totalTests==passedTests; // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24
bool GeometryExperimentEngine::runReviewSearch(const QString& inputCsv,const QString& outputDir,int seeds,int budget, // add by wushanzhou in 2026/10/24
    unsigned long long baseSeed,bool tradeoff,QString& report) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    if(seeds<1 || seeds>1000 || budget<1 || budget>100000)return false; // add by wushanzhou in 2026/10/24
    std::vector<std::vector<double> > data; // add by wushanzhou in 2026/10/24
    std::vector<double> x; // add by wushanzhou in 2026/10/24
    if(!inputCsv.isEmpty() && !loadHoldoutGather(inputCsv,data,x,report))return false; // add by wushanzhou in 2026/10/24
    const bool physical=!inputCsv.isEmpty();const int n=physical?(int)x.size():128; // add by wushanzhou in 2026/10/24
    if(!QDir().mkpath(outputDir))return false; // add by wushanzhou in 2026/10/24
    QDir dir(outputDir);QFile file(dir.filePath("search.csv")); // add by wushanzhou in 2026/10/24
    if(file.exists()){report="refusing to overwrite an existing research table";return false;} // add by wushanzhou in 2026/10/24
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/24
    QTextStream os(&file);os.setRealNumberPrecision(12); // add by wushanzhou in 2026/10/24
    os << "domain,rate_index,n,m,seed,method,budget_cap,evals,mu,objective,max_gap,max_gap_m,nn_cv,lambda_gap,lambda_nn,hard_gap_m,runtime_ms,sa_evals,local_evals,restart_evals,switches,status,mask_sha1,mask_path\n"; // add by wushanzhou in 2026/10/24
    const char* names[21]={"Regular","Random","Jitter","SAOnly","Local","Hybrid","Restart","SA_Budget", // add by wushanzhou in 2026/10/24
        "Adaptive","FixedSwitch","NoRecycle","AdjacentOnly","W_0_01","W_0_03","W_01_0","W_01_01", // add by wushanzhou in 2026/10/24
        "W_01_03","W_03_0","W_03_01","W_03_03","HardGap"}; // add by wushanzhou in 2026/10/24
    const double gapWeights[8]={0,0,.1,.1,.1,.3,.3,.3}; // add by wushanzhou in 2026/10/24
    const double nnWeights[8]={.1,.3,0,.1,.3,0,.1,.3}; // add by wushanzhou in 2026/10/24
    const int counts[3]={physical?(n+2)/3:38,physical?(n+1)/2:64,physical?(2*n+2)/3:90}; // add by wushanzhou in 2026/10/24
    for(int ri=0;ri<3;ri++)for(int si=0;si<seeds;si++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const unsigned long long seed=baseSeed+(unsigned long long)si*179ULL; // add by wushanzhou in 2026/10/24
        const int m=counts[ri];exprng::seed(seed); // add by wushanzhou in 2026/10/24
        int* bits=Sampling1J_Count(n,m);std::vector<int> initial(bits,bits+n);delete[] bits; // add by wushanzhou in 2026/10/24
        for(int type=0;type<(tradeoff?21:12);type++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            ConstraintWeights w;ConstrainedResult result;AdaptiveSearchOptions options; // add by wushanzhou in 2026/10/24
            std::vector<int> selected=initial; // add by wushanzhou in 2026/10/24
            if(type<2) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                exprng::seed(seed+7);bits=type==0?Sampling1R_Count(n,m):Sampling1S_Count(n,m); // add by wushanzhou in 2026/10/24
                selected.assign(bits,bits+n);delete[] bits; // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            if(type>=12 && type<20){w.lambdaGap=gapWeights[type-12];w.lambdaNN=nnWeights[type-12];} // add by wushanzhou in 2026/10/24
            if(type==20) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                if(physical) // add by wushanzhou in 2026/10/24
                { // add by wushanzhou in 2026/10/24
                    double candidateGap=0;for(int j=1;j<n;j++)candidateGap=std::max(candidateGap,x[j]-x[j-1]); // add by wushanzhou in 2026/10/24
                    w.hardMaxGapM=std::max(candidateGap,2.0*(x.back()-x.front())*n/((n-1)*m)); // add by wushanzhou in 2026/10/24
                } // add by wushanzhou in 2026/10/24
                else w.hardMaxGap=(int)ceil(2.0*n/m); // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            bool ok=false; // add by wushanzhou in 2026/10/24
            if(type<7) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                SamplingSearchMethod method=type==3?SEARCH_SA:type==4?SEARCH_LOCAL:type==6?SEARCH_RESTART:SEARCH_HYBRID; // add by wushanzhou in 2026/10/24
                ok=RunSamplingSearch(n,m,selected,w,seed+1,type<3?1:budget,method,result,64,1,physical?&x:NULL); // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            else // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                if(type==7)options.localSearch=false; // add by wushanzhou in 2026/10/24
                if(type==9)options.adaptiveSwitch=false; // add by wushanzhou in 2026/10/24
                if(type==10)options.recycleBudget=false; // add by wushanzhou in 2026/10/24
                if(type==11)options.mixedNeighborhood=false; // add by wushanzhou in 2026/10/24
                ok=RunAdaptiveSamplingSearch(n,m,selected,w,seed+1,budget,options,result,physical?&x:NULL); // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            if(!ok){report="search failed: "+QString(names[type])+" "+QString::fromStdString(result.status);return false;} // add by wushanzhou in 2026/10/24
            QString relative=QString("masks/%1_r%2_seed%3.txt").arg(names[type]).arg(ri).arg(QString::number(seed)); // add by wushanzhou in 2026/10/24
            if(!saveSamplingMask(dir.filePath(relative),result,seed+1,budget, // add by wushanzhou in 2026/10/24
                physical?"PhysicalFourierPSF":"IndexFourierPSF","adaptive_hybrid_v4"))return false; // add by wushanzhou in 2026/10/24
            os << (physical?"physical":"uniform") << "," << ri << "," << n << "," << m << "," << QString::number(seed) // add by wushanzhou in 2026/10/24
               << "," << names[type] << "," << budget << "," << result.evals << "," << result.mu << "," << result.objective // add by wushanzhou in 2026/10/24
               << "," << result.maxGap << "," << result.maxGapM << "," << result.nnCv << "," << w.lambdaGap << "," << w.lambdaNN // add by wushanzhou in 2026/10/24
               << "," << w.hardMaxGapM << "," << result.runtimeMs << "," << result.saEvaluations << "," << result.localEvaluations // add by wushanzhou in 2026/10/24
               << "," << result.restartEvaluations << "," << result.switches << "," << QString::fromStdString(result.status) // add by wushanzhou in 2026/10/24
               << "," << samplingMaskHash(result.mask) << "," << relative << "\n"; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        os.flush(); // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    report="adaptive_hybrid_v4 search and prespecified ablations complete"; // add by wushanzhou in 2026/10/24
    return file.error()==QFile::NoError; // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24

bool GeometryExperimentEngine::runConfiguredReconstruction(const QString& inputCsv,const QString& maskFile, // add by wushanzhou in 2026/10/24
    const QString& outputDir,int iterations,double thresholdStart,double thresholdEnd,int scales,int angles,bool warmStart,QString& report) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    if(iterations<1 || iterations>1000)return false; // add by wushanzhou in 2026/10/24
    std::vector<std::vector<double> > input;std::vector<double> x; // add by wushanzhou in 2026/10/24
    if(!loadHoldoutGather(inputCsv,input,x,report))return false; // add by wushanzhou in 2026/10/24
    int m=0;QFile mf(maskFile); // add by wushanzhou in 2026/10/24
    if(!mf.open(QIODevice::ReadOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/24
    while(!mf.atEnd()){QByteArray line=mf.readLine().trimmed();if(line=="1")m++;}mf.close(); // add by wushanzhou in 2026/10/24
    std::vector<int> mask; // add by wushanzhou in 2026/10/24
    if(!validateAndLoadMaskFile(maskFile,(int)x.size(),m,mask,report))return false; // add by wushanzhou in 2026/10/24
    std::vector<std::vector<float> > truth(input.size(),std::vector<float>(x.size())); // add by wushanzhou in 2026/10/24
    for(size_t i=0;i<input.size();i++)for(size_t j=0;j<x.size();j++)truth[i][j]=(float)input[i][j]; // add by wushanzhou in 2026/10/24
    ReconstructionOptions options;options.thresholdStart=thresholdStart;options.thresholdEnd=thresholdEnd; // add by wushanzhou in 2026/10/24
    options.scales=scales;options.angles=angles;options.warmStart=warmStart; // add by wushanzhou in 2026/10/24
    std::vector<std::vector<float> > linear,reconstructed;std::vector<float> steps; // add by wushanzhou in 2026/10/24
    double error=0,scale=0;const clock_t started=clock(); // add by wushanzhou in 2026/10/24
    if(!coordinateHoldoutReconstruct(truth,mask,x,iterations,linear,reconstructed,error,&scale,&options,&steps)) // add by wushanzhou in 2026/10/24
    {report="invalid reconstruction configuration, geometry, or projection";return false;} // add by wushanzhou in 2026/10/24
    if(!QDir().mkpath(outputDir))return false;QDir dir(outputDir); // add by wushanzhou in 2026/10/24
    if(QFile::exists(dir.filePath("prediction.csv"))){report="refusing to overwrite a research prediction";return false;} // add by wushanzhou in 2026/10/24
    const char* files[2]={"linear.csv","prediction.csv"}; // add by wushanzhou in 2026/10/24
    for(int type=0;type<2;type++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        QFile file(dir.filePath(files[type]));if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/24
        QTextStream os(&file);os.setRealNumberPrecision(9); // add by wushanzhou in 2026/10/24
        const std::vector<std::vector<float> >& values=type?reconstructed:linear; // add by wushanzhou in 2026/10/24
        for(size_t i=0;i<values.size();i++){for(size_t j=0;j<x.size();j++){if(j)os << ",";os << values[i][j];}os << "\n";} // add by wushanzhou in 2026/10/24
        os.flush();if(file.error()!=QFile::NoError)return false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    QFile file(dir.filePath("solver.csv"));if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/24
    QTextStream os(&file);os.setRealNumberPrecision(12); // add by wushanzhou in 2026/10/24
    os << "known_scale,consistency_error,runtime_ms,iterations,threshold_start,threshold_end,scales,angles,warm_start,mask_sha1\n" // add by wushanzhou in 2026/10/24
       << scale << "," << error << "," << (double)(clock()-started)*1000/CLOCKS_PER_SEC << "," << iterations << "," // add by wushanzhou in 2026/10/24
       << thresholdStart << "," << thresholdEnd << "," << scales << "," << angles << "," << (warmStart?1:0) << "," << samplingMaskHash(mask) << "\n"; // add by wushanzhou in 2026/10/24
    os.flush();if(file.error()!=QFile::NoError)return false; // add by wushanzhou in 2026/10/24
    QFile convergence(dir.filePath("convergence.csv"));if(!convergence.open(QIODevice::WriteOnly|QIODevice::Text))return false; // add by wushanzhou in 2026/10/24
    QTextStream cs(&convergence);cs << "iteration,relative_step\n"; // add by wushanzhou in 2026/10/24
    for(size_t i=0;i<steps.size();i++)cs << (int)i+1 << "," << QString::number(steps[i],'g',9) << "\n"; // add by wushanzhou in 2026/10/24
    cs.flush();report="Configured solver completed without using hidden amplitudes"; // add by wushanzhou in 2026/10/24
    return convergence.error()==QFile::NoError; // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24
