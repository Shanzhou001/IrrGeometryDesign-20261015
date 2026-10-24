// Research regression: numerical contracts, never a requirement that a new method must win. add by wushanzhou in 2026/10/24
    totalTests++;bool adaptiveOk=true; // add by wushanzhou in 2026/10/24
    for(int variant=0;variant<5;variant++)for(int cap=1;cap<=200;cap+=199) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        exprng::seed(9001);int* bits=Sampling1J_Count(32,12);std::vector<int> initial(bits,bits+32);delete[] bits; // add by wushanzhou in 2026/10/24
        AdaptiveSearchOptions options;ConstraintWeights w;ConstrainedResult a,b; // add by wushanzhou in 2026/10/24
        if(variant==1)options.adaptiveSwitch=false; // add by wushanzhou in 2026/10/24
        if(variant==2)options.recycleBudget=false; // add by wushanzhou in 2026/10/24
        if(variant==3)options.mixedNeighborhood=false; // add by wushanzhou in 2026/10/24
        if(variant==4)options.localSearch=false; // add by wushanzhou in 2026/10/24
        if(!RunAdaptiveSamplingSearch(32,12,initial,w,42,cap,options,a) || // add by wushanzhou in 2026/10/24
           !RunAdaptiveSamplingSearch(32,12,initial,w,42,cap,options,b))adaptiveOk=false; // add by wushanzhou in 2026/10/24
        if(a.mask!=b.mask || a.bestTrace!=b.bestTrace || a.phaseTrace!=b.phaseTrace || a.evals>cap || // add by wushanzhou in 2026/10/24
           !ValidateSamplingMask(32,12,&a.mask[0]) || fabs(a.mu-SamplingMaskCoherence(a.mask))>1e-6 || // add by wushanzhou in 2026/10/24
           a.bestTrace.size()!=(size_t)a.evals || a.phaseTrace.size()!=(size_t)a.evals || // add by wushanzhou in 2026/10/24
           a.saEvaluations+a.localEvaluations+a.restartEvaluations!=a.evals-1)adaptiveOk=false; // add by wushanzhou in 2026/10/24
        if(options.recycleBudget && a.evals!=cap)adaptiveOk=false; // add by wushanzhou in 2026/10/24
        for(size_t i=1;i<a.bestTrace.size();i++)if(a.bestTrace[i]>a.bestTrace[i-1]+1e-8f)adaptiveOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 25: Adaptive determinism / counts / caps / objective / monotone incumbent\n",adaptiveOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(adaptiveOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool physicalHardOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const double positions[7]={0,1,3,4,8,9,12};std::vector<double> x(positions,positions+7); // add by wushanzhou in 2026/10/24
        for(int m=1;m<7;m++)for(int bound=1;bound<=12;bound++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            bool expected=false;std::vector<int> selected; // add by wushanzhou in 2026/10/24
            for(int code=1;code<128;code++) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                std::vector<int> mask(7);int count=0; // add by wushanzhou in 2026/10/24
                for(int j=0;j<7;j++){mask[j]=(code>>j)&1;count+=mask[j];} // add by wushanzhou in 2026/10/24
                if(count==m && SamplingPhysicalMaxGap(mask,x)<=bound)expected=true; // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            bool feasible=SamplingPhysicalFeasibleMask(x,m,bound,selected); // add by wushanzhou in 2026/10/24
            if(feasible!=expected || (feasible && (!ValidateSamplingMask(7,m,&selected[0]) || SamplingPhysicalMaxGap(selected,x)>bound+1e-8)))physicalHardOk=false; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        std::vector<int> initial(7,0);initial[0]=initial[1]=initial[2]=1; // add by wushanzhou in 2026/10/24
        ConstraintWeights w;w.hardMaxGapM=4;ConstrainedResult a,b;AdaptiveSearchOptions options; // add by wushanzhou in 2026/10/24
        if(!RunAdaptiveSamplingSearch(7,3,initial,w,55,80,options,a,&x) || a.maxGapM>4+1e-8)physicalHardOk=false; // add by wushanzhou in 2026/10/24
        for(int j=0;j<7;j++)x[j]=100+10*x[j];w.hardMaxGapM=40; // add by wushanzhou in 2026/10/24
        if(!RunAdaptiveSamplingSearch(7,3,initial,w,55,80,options,b,&x) || a.mask!=b.mask || fabs(a.mu-b.mu)>1e-6)physicalHardOk=false; // add by wushanzhou in 2026/10/24
        w.hardMaxGapM=1; // add by wushanzhou in 2026/10/24
        if(RunAdaptiveSamplingSearch(7,3,initial,w,55,80,options,b,&x))physicalHardOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 26: Physical hard-gap DP matches exhaustive feasibility and unit scaling\n",physicalHardOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(physicalHardOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool finiteOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        std::vector<int> mask(8,0);mask[0]=mask[2]=mask[4]=mask[6]=1; // add by wushanzhou in 2026/10/24
        ConstraintWeights w;ConstrainedResult result;w.lambdaGap=std::numeric_limits<double>::quiet_NaN(); // add by wushanzhou in 2026/10/24
        if(RunSamplingSearch(8,4,mask,w,1,20,SEARCH_HYBRID,result))finiteOk=false; // add by wushanzhou in 2026/10/24
        w.lambdaGap=0;w.lambdaNN=std::numeric_limits<double>::infinity(); // add by wushanzhou in 2026/10/24
        if(RunSamplingSearch(8,4,mask,w,1,20,SEARCH_HYBRID,result))finiteOk=false; // add by wushanzhou in 2026/10/24
        w.lambdaNN=0;w.hardMaxGap=-1; // add by wushanzhou in 2026/10/24
        if(RunSamplingSearch(8,4,mask,w,1,20,SEARCH_HYBRID,result))finiteOk=false; // add by wushanzhou in 2026/10/24
        w.hardMaxGap=0;w.hardMaxGapM=2; // add by wushanzhou in 2026/10/24
        if(RunSamplingSearch(8,4,mask,w,1,20,SEARCH_HYBRID,result))finiteOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 27: Nonfinite weights / invalid constraint units are rejected\n",finiteOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(finiteOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool configOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const double locations[8]={0,1,3,4,8,9,15,20};std::vector<double> x(locations,locations+8); // add by wushanzhou in 2026/10/24
        std::vector<int> mask(8,0);mask[0]=mask[3]=mask[7]=1; // add by wushanzhou in 2026/10/24
        std::vector<std::vector<float> > data(64,std::vector<float>(8)),changed,linear,a,b; // add by wushanzhou in 2026/10/24
        for(int i=0;i<64;i++)for(int j=0;j<8;j++)data[i][j]=(float)(sin(i*.2)*(.5+.01*x[j])); // add by wushanzhou in 2026/10/24
        changed=data;for(int i=0;i<64;i++)for(int j=0;j<8;j++)if(!mask[j])changed[i][j]+=100; // add by wushanzhou in 2026/10/24
        ReconstructionOptions options;double ea,eb; // add by wushanzhou in 2026/10/24
        for(int warm=0;warm<2;warm++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            options.warmStart=warm!=0;options.scales=3;options.angles=8;options.thresholdStart=.05;options.thresholdEnd=.002; // add by wushanzhou in 2026/10/24
            if(!coordinateHoldoutReconstruct(data,mask,x,4,linear,a,ea,NULL,&options) || // add by wushanzhou in 2026/10/24
               !coordinateHoldoutReconstruct(changed,mask,x,4,linear,b,eb,NULL,&options) || a!=b)configOk=false; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        std::vector<std::vector<float> > empty; // add by wushanzhou in 2026/10/24
        if(coordinateHoldoutReconstruct(empty,mask,x,4,linear,a,ea))configOk=false; // add by wushanzhou in 2026/10/24
        changed=data;changed[1].pop_back(); // add by wushanzhou in 2026/10/24
        if(coordinateHoldoutReconstruct(changed,mask,x,4,linear,a,ea))configOk=false; // add by wushanzhou in 2026/10/24
        options.scales=99; // add by wushanzhou in 2026/10/24
        if(coordinateHoldoutReconstruct(data,mask,x,4,linear,a,ea,NULL,&options))configOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 28: Configurable solver isolates hidden values and rejects malformed inputs\n",configOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(configOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool zeroThresholdOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const double locations[8]={0,1,3,4,8,9,15,20};std::vector<double> x(locations,locations+8); // add by wushanzhou in 2026/10/24
        std::vector<int> mask(8,0);mask[0]=mask[3]=mask[7]=1; // add by wushanzhou in 2026/10/24
        std::vector<std::vector<float> > data(64,std::vector<float>(8)),linear,prediction,legacy; // add by wushanzhou in 2026/10/24
        for(int i=0;i<64;i++)for(int j=0;j<8;j++)data[i][j]=(float)(sin(i*.2)*(.5+.01*x[j])); // add by wushanzhou in 2026/10/24
        ReconstructionOptions options;options.thresholdStart=options.thresholdEnd=0;double error; // add by wushanzhou in 2026/10/24
        if(!coordinateHoldoutReconstruct(data,mask,x,2,linear,prediction,error,NULL,&options))zeroThresholdOk=false; // add by wushanzhou in 2026/10/24
        for(int i=0;i<64;i++)for(int j=0;j<8;j++)if(fabs(prediction[i][j]-data[i][j])>2e-6)zeroThresholdOk=false; // add by wushanzhou in 2026/10/24
        options=ReconstructionOptions(); // add by wushanzhou in 2026/10/24
        if(!coordinateHoldoutReconstruct(data,mask,x,4,linear,prediction,error,NULL,&options) || // add by wushanzhou in 2026/10/24
           !coordinateHoldoutReconstruct(data,mask,x,4,linear,legacy,error))zeroThresholdOk=false; // add by wushanzhou in 2026/10/24
        for(int i=0;i<64;i++)for(int j=0;j<8;j++)if(fabs(prediction[i][j]-legacy[i][j])>2e-6)zeroThresholdOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 29: Zero-threshold FDCT/projection roundtrip and legacy default equivalence\n",zeroThresholdOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(zeroThresholdOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool endpointOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const double locations[8]={-1560,-1352,-936,-104,312,520,728,1144}; // add by wushanzhou in 2026/10/24
        std::vector<double> x(locations,locations+8);std::vector<int> mask(8,0);mask[3]=mask[4]=mask[6]=1; // add by wushanzhou in 2026/10/24
        std::vector<std::vector<float> > data(64,std::vector<float>(8)),linear,prediction; // add by wushanzhou in 2026/10/24
        for(int i=0;i<64;i++)for(int j=0;j<8;j++)data[i][j]=(float)(sin(i*.2+j*.1)); // add by wushanzhou in 2026/10/24
        double error;ReconstructionOptions options; // add by wushanzhou in 2026/10/24
        if(!coordinateHoldoutReconstruct(data,mask,x,2,linear,prediction,error,NULL,&options))endpointOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 30: Physical grid endpoint rounding stays within the observation domain\n",endpointOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(endpointOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool shiftOk=true; // add by wushanzhou in 2026/10/24
    const int dimensions[4][2]={{3,5},{4,5},{3,4},{4,6}}; // add by wushanzhou in 2026/10/24
    for(int test=0;test<4;test++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        int nr=dimensions[test][0],nc=dimensions[test][1]; // add by wushanzhou in 2026/10/24
        std::vector<std::vector<int> > data(nr,std::vector<int>(nc));std::vector<int*> pointers(nr); // add by wushanzhou in 2026/10/24
        for(int i=0;i<nr;i++){pointers[i]=&data[i][0];for(int j=0;j<nc;j++)data[i][j]=3*i+j*j;} // add by wushanzhou in 2026/10/24
        fdct_wrapping_ns::CpxNumMat result;FFT1D_fftshift(&pointers[0],result,nr,nc); // add by wushanzhou in 2026/10/24
        for(int i=0;i<nr;i++)for(int k=0;k<nc;k++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            std::complex<double> expected(0,0);for(int j=0;j<nc;j++)expected+=(double)data[i][j]*std::polar(1.0,-6.28318530717958647692*k*j/nc); // add by wushanzhou in 2026/10/24
            if(std::abs(result((i+nr/2)%nr,(k+nc/2)%nc)-expected)>1e-8)shiftOk=false; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 31: Legacy FFT shift matches direct DFT for rectangular odd/even grids\n",shiftOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(shiftOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool spsBoundsOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        QVector<QVector<float> > values;QVector<float> a,b;a << 0 << 10 << 30;b << 20;values << a << b; // add by wushanzhou in 2026/10/24
        for(int mode=0;mode<2;mode++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            std::string type=mode?"S":"R";int** mask=SpsArrayTrans(values,type); // add by wushanzhou in 2026/10/24
            if(ArrayLineNum(values,type)!=4 || !mask)spsBoundsOk=false; // add by wushanzhou in 2026/10/24
            if(mask){if(!mask[0][0] || !mask[0][1] || mask[0][2] || !mask[0][3] || !mask[1][2])spsBoundsOk=false;for(int i=0;i<2;i++)free(mask[i]);free(mask);} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        std::reverse(values[0].begin(),values[0].end());if(ArrayLineNum(values,"S")!=4)spsBoundsOk=false; // add by wushanzhou in 2026/10/24
        QVector<QVector<float> > empty;if(ArrayLineNum(empty,"R") || SpsArrayTrans(empty,"R") || SpsArrayTrans(values,"bad"))spsBoundsOk=false; // add by wushanzhou in 2026/10/24
        values[0][0]=std::numeric_limits<float>::quiet_NaN();if(ArrayLineNum(values,"R") || SpsArrayTrans(values,"R"))spsBoundsOk=false; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 32: Legacy SPS ragged/empty/descending/zero/nonfinite geometry is safe\n",spsBoundsOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(spsBoundsOk)passedTests++; // add by wushanzhou in 2026/10/24

    totalTests++;bool legacyGramOk=true; // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        std::vector<int> indices;for(int i=0;i<80;i++)if(i==0 || i%3==1)indices.push_back(i); // add by wushanzhou in 2026/10/24
        std::vector<std::vector<int> > identity(80,std::vector<int>(80,0)),sampled=identity;std::vector<int*> ip(80),sp(80); // add by wushanzhou in 2026/10/24
        for(int i=0;i<80;i++){identity[i][i]=1;ip[i]=&identity[i][0];sp[i]=&sampled[i][0];} // add by wushanzhou in 2026/10/24
        for(size_t i=0;i<indices.size();i++)sampled[indices[i]][indices[i]]=1; // add by wushanzhou in 2026/10/24
        fdct_wrapping_ns::CpxNumMat fullTransform,sampleTransform;std::vector<std::vector<fdct_wrapping_ns::CpxNumMat> > ci,cs; // add by wushanzhou in 2026/10/24
        FFT1D_fftshift(&ip[0],fullTransform,80,80);FFT1D_fftshift(&sp[0],sampleTransform,80,80); // add by wushanzhou in 2026/10/24
        fdct_wrapping_ns::fdct_wrapping(80,80,1,4,1,fullTransform,ci);fdct_wrapping_ns::fdct_wrapping(80,80,1,4,1,sampleTransform,cs); // add by wushanzhou in 2026/10/24
        const int size=cs[0][0].m();MatrixXcf a(size,size),b(size,size); // add by wushanzhou in 2026/10/24
        for(int i=0;i<size;i++)for(int j=0;j<size;j++){a(i,j)=std::complex<float>((float)cs[0][0](i,j).real(),(float)cs[0][0](i,j).imag());b(i,j)=std::complex<float>((float)ci[0][0](i,j).real(),(float)ci[0][0](i,j).imag());} // add by wushanzhou in 2026/10/24
        MatrixXcf ga=a.adjoint()*a,gb=b.adjoint()*b;MatrixXf ar=a.real(),br=b.real(),ra=ar.transpose()*ar,rb=br.transpose()*br; // add by wushanzhou in 2026/10/24
        double expectedComplex=0,expectedReal=0; // add by wushanzhou in 2026/10/24
        for(int i=0;i<std::min(64,size);i++)for(int j=0;j<size;j++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            expectedComplex=std::max<double>(expectedComplex,fabs(std::abs(ga(i,j))/sqrt(ga(i,i).real()*ga(j,j).real())-std::abs(gb(i,j))/sqrt(gb(i,i).real()*gb(j,j).real()))); // add by wushanzhou in 2026/10/24
            expectedReal=std::max<double>(expectedReal,fabs(fabs(ra(i,j))/sqrt(ra(i,i)*ra(j,j))-fabs(rb(i,j))/sqrt(rb(i,i)*rb(j,j)))); // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        for(int repeat=0;repeat<3;repeat++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            float actualComplex=CurveletMiu1DComplex(indices,80),actualReal=CurveletMiu1DReal(indices,80); // add by wushanzhou in 2026/10/24
            if(fabs(actualComplex-expectedComplex)>2e-5 || fabs(actualReal-expectedReal)>2e-5){legacyGramOk=false;log_test(logFp,"legacy/reference complex %.8g/%.8g real %.8g/%.8g\n",actualComplex,expectedComplex,actualReal,expectedReal);} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    log_test(logFp," [%s] Test 33: Legacy Gram scan above 64 columns matches explicit normalized magnitude Gram\n",legacyGramOk?"PASS":"FAIL"); // add by wushanzhou in 2026/10/24
    if(legacyGramOk)passedTests++; // add by wushanzhou in 2026/10/24
