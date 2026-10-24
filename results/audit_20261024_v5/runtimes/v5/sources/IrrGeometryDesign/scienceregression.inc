// Included inside runUnitTests; assertions exercise the scientific contracts. // add by wushanzhou in 2026/10/23
    totalTests++; // add by wushanzhou in 2026/10/23
    bool searchOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int counts[3]={38,64,90}; // add by wushanzhou in 2026/10/23
        for(int r=0;r<3;r++) for(int seed=1;seed<=5;seed++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            exprng::seed(seed);int* jm=Sampling1J_Count(128,counts[r]); // add by wushanzhou in 2026/10/23
            std::vector<int> initial(jm,jm+128);delete[] jm; // add by wushanzhou in 2026/10/23
            if(fabs(SamplingUnitOccupancy(initial)-1.0f)>1e-7)searchOk=false; // add by wushanzhou in 2026/10/23
            ConstraintWeights w;ConstrainedResult sa,hybrid,repeat; // add by wushanzhou in 2026/10/23
            if(!RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_SA,sa,40) || // add by wushanzhou in 2026/10/23
               !RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_HYBRID,hybrid,40) || // add by wushanzhou in 2026/10/23
               !RunSamplingSearch(128,counts[r],initial,w,seed+1,120,SEARCH_HYBRID,repeat,40))searchOk=false; // add by wushanzhou in 2026/10/23
            if(sa.saPrefixMask!=hybrid.saPrefixMask || hybrid.mask!=repeat.mask || // add by wushanzhou in 2026/10/23
               hybrid.evals>120 || hybrid.evals<40 || sa.evals!=120 || // add by wushanzhou in 2026/10/23
               fabs(SamplingMaskCoherence(hybrid.mask)-hybrid.mu)>1e-7 || // add by wushanzhou in 2026/10/23
               hybrid.mu>SamplingMaskCoherence(hybrid.saPrefixMask)+1e-7)searchOk=false; // add by wushanzhou in 2026/10/23
            for(size_t i=1;i<hybrid.bestTrace.size();i++) // add by wushanzhou in 2026/10/23
                if(hybrid.bestTrace[i]>hybrid.bestTrace[i-1]+1e-7)searchOk=false; // add by wushanzhou in 2026/10/23
            if(hybrid.bestTrace.size()!=(size_t)hybrid.evals)searchOk=false; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 17: Shared PSF objective / cap / SA prefix / Jitter cells / reproducibility\n",searchOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(searchOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool hardOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        exprng::seed(9);int* jm=Sampling1J_Count(128,38);std::vector<int> initial(jm,jm+128);delete[] jm; // add by wushanzhou in 2026/10/23
        ConstraintWeights w;ConstrainedResult result;w.hardMaxGap=3; // add by wushanzhou in 2026/10/23
        if(RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result) || result.status!="infeasible_hard_gap")hardOk=false; // add by wushanzhou in 2026/10/23
        w.hardMaxGap=4; // add by wushanzhou in 2026/10/23
        if(!RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result) || result.maxGap>4 || // add by wushanzhou in 2026/10/23
            !ValidateSamplingMask(128,38,&result.mask[0]) || !result.hardConstraintSatisfied)hardOk=false; // add by wushanzhou in 2026/10/23
        w.hardMaxGap=0;initial[0]=2; // add by wushanzhou in 2026/10/23
        if(RunSamplingSearch(128,38,initial,w,1,100,SEARCH_HYBRID,result))hardOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 18: Infeasible hard constraints rejected; feasible initialization and saved optimum\n",hardOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(hardOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool fftOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const int n=31;std::vector<int> idx;idx.push_back(1);idx.push_back(7);idx.push_back(12);idx.push_back(25); // add by wushanzhou in 2026/10/23
        double expected=0; // add by wushanzhou in 2026/10/23
        for(int d=1;d<n;d++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            double re=0,im=0; // add by wushanzhou in 2026/10/23
            for(size_t k=0;k<idx.size();k++){double a=-2*3.14159265358979323846*d*idx[k]/n;re+=cos(a);im+=sin(a);} // add by wushanzhou in 2026/10/23
            expected=std::max(expected,sqrt(re*re+im*im)/idx.size()); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        if(fabs(SamplingMutualCoherence1D(idx,n)-expected)>1e-6)fftOk=false; // add by wushanzhou in 2026/10/23
        std::vector<int> flat(15,0);flat[0]=flat[3]=flat[7]=flat[11]=1; // add by wushanzhou in 2026/10/23
        expected=0; // add by wushanzhou in 2026/10/23
        for(int y=0;y<3;y++)for(int x=0;x<5;x++)if(x||y) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            double re=0,im=0; // add by wushanzhou in 2026/10/23
            for(int k=0;k<15;k++)if(flat[k]){double a=-2*3.14159265358979323846*(y*(k/5)/3.0+x*(k%5)/5.0);re+=cos(a);im+=sin(a);} // add by wushanzhou in 2026/10/23
            expected=std::max(expected,sqrt(re*re+im*im)/4); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        if(fabs(SamplingMutualCoherence2D(flat,3,5)-expected)>1e-6)fftOk=false; // add by wushanzhou in 2026/10/23
        ConstraintWeights w;ConstrainedResult result; // add by wushanzhou in 2026/10/23
        if(!RunSamplingSearch(15,4,flat,w,77,40,SEARCH_HYBRID,result,16,3) || // add by wushanzhou in 2026/10/23
            fabs(result.mu-SamplingMutualCoherence2D(result.mask,3,5))>1e-7 || result.mu>expected+1e-7)fftOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 19: FFT PSF matches independent DFT, including rectangular 2D search\n",fftOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(fftOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool parserOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        for(int variant=0;variant<7;variant++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            QString path=kTestSandbox+QString("/parser_%1.csv").arg(variant);QFile f(path); // add by wushanzhou in 2026/10/23
            if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){parserOk=false;continue;} // add by wushanzhou in 2026/10/23
            QTextStream os(&f);os << "# offsets_m=0,1,2,3\n"; // add by wushanzhou in 2026/10/23
            for(int row=0;row<32;row++) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                if(row==3 && variant==0)os << "1,2,3\n"; // add by wushanzhou in 2026/10/23
                else if(row==3 && variant==1)os << "1,NaN,3,4\n"; // add by wushanzhou in 2026/10/23
                else if(row==3 && variant==2)os << "1,Inf,3,4\n"; // add by wushanzhou in 2026/10/23
                else if(row==3 && variant==3)os << "1,,3,4\n"; // add by wushanzhou in 2026/10/23
                else if(row==3 && variant==4)os << "bad row\n"; // add by wushanzhou in 2026/10/23
                else os << "1,2,3,4\n"; // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            if(variant==5)os << "# offsets_m=0,1,2,3\n"; // add by wushanzhou in 2026/10/23
            os.flush();f.close(); // add by wushanzhou in 2026/10/23
            std::vector<std::vector<double> > data;std::vector<double> x;QString error; // add by wushanzhou in 2026/10/23
            const bool loaded=loadHoldoutGather(path,data,x,error); // add by wushanzhou in 2026/10/23
            if((variant<6 && loaded)||(variant==6 && !loaded))parserOk=false; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 20: Strict physical-coordinate CSV rejects ragged / NaN / Inf / empty / duplicate metadata\n",parserOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(parserOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool isolationOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const double locations[8]={0,2,4,6,9,12.8,13.8,18}; // add by wushanzhou in 2026/10/23
        std::vector<double> x(locations,locations+8);std::vector<int> mask(8,0); // add by wushanzhou in 2026/10/23
        std::vector<std::vector<float> > a(64,std::vector<float>(8,0)),b; // add by wushanzhou in 2026/10/23
        for(int j=0;j<8;j++){mask[j]=j!=3;for(int i=0;i<64;i++)a[i][j]=(float)(sin(i*0.2+j*0.1)*0.1);} // add by wushanzhou in 2026/10/23
        b=a;for(int j=0;j<8;j++)if(!mask[j])for(int i=0;i<64;i++)b[i][j]+=100; // add by wushanzhou in 2026/10/23
        std::vector<std::vector<float> > la,lb,ra,rb;double ea=0,eb=0; // add by wushanzhou in 2026/10/23
        if(!coordinateHoldoutReconstruct(a,mask,x,4,la,ra,ea)||!coordinateHoldoutReconstruct(b,mask,x,4,lb,rb,eb))isolationOk=false; // add by wushanzhou in 2026/10/23
        if(la!=lb||ra!=rb||ea>1e-5||eb>1e-5)isolationOk=false; // add by wushanzhou in 2026/10/23
        b=a;for(int j=0;j<8;j++)for(int i=0;i<64;i++)b[i][j]*=10; // add by wushanzhou in 2026/10/23
        if(!coordinateHoldoutReconstruct(b,mask,x,4,lb,rb,eb))isolationOk=false; // add by wushanzhou in 2026/10/23
        for(int j=0;j<8;j++)for(int i=0;i<64;i++)if(fabs(rb[i][j]-10*ra[i][j])>1e-5)isolationOk=false; // add by wushanzhou in 2026/10/23
        x.back()=1e12; // add by wushanzhou in 2026/10/23
        if(coordinateHoldoutReconstruct(a,mask,x,4,lb,rb,eb))isolationOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 21: Held-out amplitudes never affect solver; physical observation projection is consistent\n",isolationOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(isolationOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool chainOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        std::vector<int> selected,used;QString error; // add by wushanzhou in 2026/10/23
        if(!validateAndLoadMaskFile(kTestSandbox+"/test_suite_norm/01_sampling_benchmark/mask_50pct_sa_best.txt",128,64,selected,error) || // add by wushanzhou in 2026/10/23
           !validateAndLoadMaskFile(kTestSandbox+"/test_suite_norm/03_wavefield_reconstruction/cmp_mask.txt",128,64,used,error) || // add by wushanzhou in 2026/10/23
           selected!=used)chainOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 22: One-click reconstruction consumes the exact optimized benchmark mask\n",chainOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(chainOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool metadataOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        ReconstructionMetrics metrics;QString report; // add by wushanzhou in 2026/10/23
        if(!runWavefieldReconstruction("",kTestSandbox+"/physical_grid_metadata",0.5f,metrics,report, // add by wushanzhou in 2026/10/23
            "data/real_seismic/real_gather_256x128.csv",false,false,true,false))metadataOk=false; // add by wushanzhou in 2026/10/23
        if(fabs(metrics.dx-2990.0/127.0)>1e-4 || metrics.isSynthetic)metadataOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 23: Real reference-grid metadata uses actual offset spacing\n",metadataOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(metadataOk)passedTests++; // add by wushanzhou in 2026/10/23

    totalTests++; // add by wushanzhou in 2026/10/23
    bool physicalOk=true; // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        const double locations[8]={0,1,3,4,8,9,15,20}; // add by wushanzhou in 2026/10/23
        std::vector<double> x(locations,locations+8),uniform(8),shifted(8),scaled(8); // add by wushanzhou in 2026/10/23
        std::vector<int> mask(8,0);mask[0]=mask[3]=mask[5]=mask[7]=1; // add by wushanzhou in 2026/10/23
        for(int j=0;j<8;j++){uniform[j]=12.5*j;shifted[j]=1000+x[j];scaled[j]=10*x[j];} // add by wushanzhou in 2026/10/23
        if(fabs(SamplingPhysicalCoherence(mask,uniform)-SamplingMaskCoherence(mask))>1e-6)physicalOk=false; // add by wushanzhou in 2026/10/23
        const double period=20.0*8/7;double expected=0; // add by wushanzhou in 2026/10/23
        for(int d=1;d<8;d++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            std::complex<double> sum(0,0); // add by wushanzhou in 2026/10/23
            for(int j=0;j<8;j++)if(mask[j])sum+=std::polar(1.0,-6.28318530717958647692*d*x[j]/period); // add by wushanzhou in 2026/10/23
            expected=std::max(expected,std::abs(sum)/4); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        if(fabs(expected-SamplingPhysicalCoherence(mask,x))>1e-6 || // add by wushanzhou in 2026/10/23
           fabs(expected-SamplingPhysicalCoherence(mask,shifted))>1e-6 || // add by wushanzhou in 2026/10/23
           fabs(expected-SamplingPhysicalCoherence(mask,scaled))>1e-6)physicalOk=false; // add by wushanzhou in 2026/10/23
        ConstraintWeights w;w.lambdaGap=w.lambdaNN=0.3;ConstrainedResult a,b; // add by wushanzhou in 2026/10/23
        const double expectedCv=sqrt(8.5)/6; // add by wushanzhou in 2026/10/23
        const double goal=expected+0.3*(std::max(0.0,11/(period/4)-1)+expectedCv); // add by wushanzhou in 2026/10/23
        if(!RunSamplingSearch(8,4,mask,w,1,1,SEARCH_HYBRID,a,64,1,&x) || // add by wushanzhou in 2026/10/23
           !RunSamplingSearch(8,4,mask,w,1,1,SEARCH_HYBRID,b,64,1,&scaled) || // add by wushanzhou in 2026/10/23
           fabs(a.maxGapM-11)>1e-8 || fabs(a.nnCv-expectedCv)>1e-6 || fabs(a.objective-goal)>1e-6 || // add by wushanzhou in 2026/10/23
           fabs(a.objective-b.objective)>1e-6 || fabs(b.maxGapM-110)>1e-8)physicalOk=false; // add by wushanzhou in 2026/10/23
        x[1]=x[0]; // add by wushanzhou in 2026/10/23
        if(RunSamplingSearch(8,4,mask,w,1,1,SEARCH_HYBRID,a,64,1,&x))physicalOk=false; // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    log_test(logFp," [%s] Test 24: Physical Fourier dictionary / uniform equivalence / affine invariance / meter penalties\n",physicalOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/23
    if(physicalOk)passedTests++; // add by wushanzhou in 2026/10/23
    // add by wushanzhou in 2026/10/23
