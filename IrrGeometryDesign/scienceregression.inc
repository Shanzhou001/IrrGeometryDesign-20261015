// Included inside runUnitTests; assertions exercise the scientific contracts.
    totalTests++;
    bool searchOk=true;
    {
        const int counts[3]={38,64,90};
        for(int r=0;r<3;r++) for(int seed=1;seed<=5;seed++)
        {
            exprng::seed(seed);int* jm=Sampling1J_Count(128,counts[r]);
            std::vector<int> initial(jm,jm+128);delete[] jm;
            if(fabs(SamplingUnitOccupancy(initial)-1.0f)>1e-7)searchOk=false;
            ConstraintWeights w;ConstrainedResult sa,hybrid,repeat;
            if(!RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_SA,sa,40) ||
               !RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_HYBRID,hybrid,40) ||
               !RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_HYBRID,repeat,40))searchOk=false;
            if(sa.saPrefixMask!=hybrid.saPrefixMask || hybrid.mask!=repeat.mask ||
               hybrid.evals>120 || hybrid.evals<40 || sa.evals!=120 ||
               fabs(SamplingMaskCoherence(hybrid.mask)-hybrid.mu)>1e-7 ||
               hybrid.mu>SamplingMaskCoherence(hybrid.saPrefixMask)+1e-7)searchOk=false;
            for(size_t i=1;i<hybrid.bestTrace.size();i++)
                if(hybrid.bestTrace[i]>hybrid.bestTrace[i-1]+1e-7)searchOk=false;
            if(hybrid.bestTrace.size()!=(size_t)hybrid.evals)searchOk=false;
        }
    }
    log_test(logFp," [%s] Test 17: Shared PSF objective / cap / SA prefix / Jitter cells / reproducibility\n",searchOk?"PASS":"FAIL");
    if(searchOk)passedTests++;

    totalTests++;
    bool hardOk=true;
    {
        exprng::seed(9);int* jm=Sampling1J_Count(128,38);std::vector<int> initial(jm,jm+128);delete[] jm;
        ConstraintWeights w;ConstrainedResult result;w.hardMaxGap=3;
        if(RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result) || result.status!="infeasible_hard_gap")hardOk=false;
        w.hardMaxGap=4;
        if(!RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result) || result.maxGap>4 ||
            !ValidateSamplingMask(128,38,&result.mask[0]) || !result.hardConstraintSatisfied)hardOk=false;
        w.hardMaxGap=0;initial[0]=2;
        if(RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result))hardOk=false;
    }
    log_test(logFp," [%s] Test 18: Infeasible hard constraints rejected; feasible initialization and saved optimum\n",hardOk?"PASS":"FAIL");
    if(hardOk)passedTests++;

    totalTests++;
    bool fftOk=true;
    {
        const int n=31;std::vector<int> idx;idx.push_back(1);idx.push_back(7);idx.push_back(12);idx.push_back(25);
        double expected=0;
        for(int d=1;d<n;d++)
        {
            double re=0,im=0;
            for(size_t k=0;k<idx.size();k++){double a=-2*3.14159265358979323846*d*idx[k]/n;re+=cos(a);im+=sin(a);}
            expected=std::max(expected,sqrt(re*re+im*im)/idx.size());
        }
        if(fabs(SamplingMutualCoherence1D(idx,n)-expected)>1e-6)fftOk=false;
        std::vector<int> flat(15,0);flat[0]=flat[3]=flat[7]=flat[11]=1;
        expected=0;
        for(int y=0;y<3;y++)for(int x=0;x<5;x++)if(x||y)
        {
            double re=0,im=0;
            for(int k=0;k<15;k++)if(flat[k]){double a=-2*3.14159265358979323846*(y*(k/5)/3.0+x*(k%5)/5.0);re+=cos(a);im+=sin(a);}
            expected=std::max(expected,sqrt(re*re+im*im)/4);
        }
        if(fabs(SamplingMutualCoherence2D(flat,3,5)-expected)>1e-6)fftOk=false;
        ConstraintWeights w;ConstrainedResult result;
        if(!RunSamplingSearch(15,4,flat,w,77,40,SEARCH_HYBRID,result,16,3) ||
            fabs(result.mu-SamplingMutualCoherence2D(result.mask,3,5))>1e-7 || result.mu>expected+1e-7)fftOk=false;
    }
    log_test(logFp," [%s] Test 19: FFT PSF matches independent DFT, including rectangular 2D search\n",fftOk?"PASS":"FAIL");
    if(fftOk)passedTests++;

    totalTests++;
    bool parserOk=true;
    {
        for(int variant=0;variant<7;variant++)
        {
            QString path=kTestSandbox+QString("/parser_%1.csv").arg(variant);QFile f(path);
            if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){parserOk=false;continue;}
            QTextStream os(&f);os << "# offsets_m=0,1,2,3\n";
            for(int row=0;row<32;row++)
            {
                if(row==3 && variant==0)os << "1,2,3\n";
                else if(row==3 && variant==1)os << "1,NaN,3,4\n";
                else if(row==3 && variant==2)os << "1,Inf,3,4\n";
                else if(row==3 && variant==3)os << "1,,3,4\n";
                else if(row==3 && variant==4)os << "bad row\n";
                else os << "1,2,3,4\n";
            }
            if(variant==5)os << "# offsets_m=0,1,2,3\n";
            os.flush();f.close();
            std::vector<std::vector<double> > data;std::vector<double> x;QString error;
            const bool loaded=loadHoldoutGather(path,data,x,error);
            if((variant<6 && loaded)||(variant==6 && !loaded))parserOk=false;
        }
    }
    log_test(logFp," [%s] Test 20: Strict physical-coordinate CSV rejects ragged / NaN / Inf / empty / duplicate metadata\n",parserOk?"PASS":"FAIL");
    if(parserOk)passedTests++;

    totalTests++;
    bool isolationOk=true;
    {
        const double locations[8]={0,2,4,6,9,12.8,13.8,18};
        std::vector<double> x(locations,locations+8);std::vector<int> mask(8,0);
        std::vector<std::vector<float> > a(64,std::vector<float>(8,0)),b;
        for(int j=0;j<8;j++){mask[j]=j!=3;for(int i=0;i<64;i++)a[i][j]=(float)(sin(i*0.2+j*0.1)*0.1);}
        b=a;for(int j=0;j<8;j++)if(!mask[j])for(int i=0;i<64;i++)b[i][j]+=100;
        std::vector<std::vector<float> > la,lb,ra,rb;double ea=0,eb=0;
        if(!coordinateHoldoutReconstruct(a,mask,x,4,la,ra,ea)||!coordinateHoldoutReconstruct(b,mask,x,4,lb,rb,eb))isolationOk=false;
        if(la!=lb||ra!=rb||ea>1e-5||eb>1e-5)isolationOk=false;
        b=a;for(int j=0;j<8;j++)for(int i=0;i<64;i++)b[i][j]*=10;
        if(!coordinateHoldoutReconstruct(b,mask,x,4,lb,rb,eb))isolationOk=false;
        for(int j=0;j<8;j++)for(int i=0;i<64;i++)if(fabs(rb[i][j]-10*ra[i][j])>1e-5)isolationOk=false;
        x.back()=1e12;
        if(coordinateHoldoutReconstruct(a,mask,x,4,lb,rb,eb))isolationOk=false;
    }
    log_test(logFp," [%s] Test 21: Held-out amplitudes never affect solver; physical observation projection is consistent\n",isolationOk?"PASS":"FAIL");
    if(isolationOk)passedTests++;

    totalTests++;
    bool chainOk=true;
    {
        std::vector<int> selected,used;QString error;
        if(!validateAndLoadMaskFile(kTestSandbox+"/test_suite_norm/01_sampling_benchmark/mask_50pct_sa_best.txt",128,64,selected,error) ||
           !validateAndLoadMaskFile(kTestSandbox+"/test_suite_norm/03_wavefield_reconstruction/cmp_mask.txt",128,64,used,error) ||
           selected!=used)chainOk=false;
    }
    log_test(logFp," [%s] Test 22: One-click reconstruction consumes the exact optimized benchmark mask\n",chainOk?"PASS":"FAIL");
    if(chainOk)passedTests++;

    totalTests++;
    bool metadataOk=true;
    {
        ReconstructionMetrics metrics;QString report;
        if(!runWavefieldReconstruction("",kTestSandbox+"/physical_grid_metadata",0.5f,metrics,report,
            "data/real_seismic/real_gather_256x128.csv",false,false,true,false))metadataOk=false;
        if(fabs(metrics.dx-2990.0/127.0)>1e-4 || metrics.isSynthetic)metadataOk=false;
    }
    log_test(logFp," [%s] Test 23: Real reference-grid metadata uses actual offset spacing\n",metadataOk?"PASS":"FAIL");
    if(metadataOk)passedTests++;
