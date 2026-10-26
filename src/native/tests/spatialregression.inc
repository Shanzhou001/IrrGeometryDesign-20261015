// Included inside both full and self-contained regression entry points.
    totalTests++;bool spatialGramOk=true;
    {
        std::vector<SpatialPsfPoint> points;
        points.push_back(SpatialPsfPoint(0,0));points.push_back(SpatialPsfPoint(1,2));
        points.push_back(SpatialPsfPoint(3,1));points.push_back(SpatialPsfPoint(4,5));
        points.push_back(SpatialPsfPoint(8,3));
        SpatialPsfObjective objective;std::string error;
        if(!objective.initialize(points,error,2,2))spatialGramOk=false;
        else for(int bits=1;bits<32;bits++)
        {
            std::vector<int> mask(5,0);int keep=0;
            for(int i=0;i<5;i++){mask[i]=(bits>>i)&1;keep+=mask[i];}
            // Independent explicit complex Gram of the four dictionary atoms.
            double expected=0;
            for(int a=0;a<4;a++)for(int b=a+1;b<4;b++)
            {
                std::complex<double> gram(0,0);
                for(int i=0;i<5;i++)if(mask[i])
                {
                    const double fa=(a%2)*(points[i].x-objective.originX)/objective.periodX
                                   +(a/2)*(points[i].y-objective.originY)/objective.periodY;
                    const double fb=(b%2)*(points[i].x-objective.originX)/objective.periodX
                                   +(b/2)*(points[i].y-objective.originY)/objective.periodY;
                    gram+=std::polar(1.0,6.28318530717958647692*(fa-fb));
                }
                expected=std::max(expected,std::abs(gram)/keep);
            }
            if(fabs(objective.measure(mask)-expected)>2e-12)spatialGramOk=false;
            std::vector<std::complex<double> > sums,incremental;int m=0;
            if(!objective.sums(mask,sums,m))spatialGramOk=false;
            for(int i=0;i<5;i++)if(mask[i])for(int j=0;j<5;j++)if(!mask[j])
            {
                std::vector<int> changed=mask;changed[i]=0;changed[j]=1;
                if(!objective.exchange(sums,i,j,incremental) ||
                   fabs(objective.coherence(incremental,m)-objective.measure(changed))>2e-12)spatialGramOk=false;
            }
            std::vector<SpatialPsfPoint> translated=points;
            for(size_t i=0;i<translated.size();i++){translated[i].x=7*translated[i].x+1000;translated[i].y=13*translated[i].y-500;}
            SpatialPsfObjective transformed;
            if(!transformed.initialize(translated,error,2,2) || fabs(transformed.measure(mask)-expected)>2e-12)spatialGramOk=false;
            std::reverse(translated.begin(),translated.end());std::reverse(mask.begin(),mask.end());
            if(!transformed.initialize(translated,error,2,2) || fabs(transformed.measure(mask)-expected)>2e-12)spatialGramOk=false;
        }
        std::vector<SpatialPsfPoint> line;
        for(int i=0;i<5;i++)line.push_back(SpatialPsfPoint(i*2,100));
        SpatialPsfObjective flat;
        if(!flat.initialize(line,error)||flat.atomsY!=1||fabs(flat.measure(std::vector<int>(5,1)))>1e-12)spatialGramOk=false;
        line.push_back(line[0]);
        if(flat.initialize(line,error)||error!="duplicate_spatial_coordinate")spatialGramOk=false;
        line[5]=SpatialPsfPoint(std::numeric_limits<double>::quiet_NaN(),100);
        if(flat.initialize(line,error)||error!="nonfinite_spatial_coordinate")spatialGramOk=false;
        if(flat.initialize(points,error,1,1)||error!="degenerate_spatial_dictionary")spatialGramOk=false;
    }
    log_test(logFp," [%s] Test 35: SpatialFourier2D matches explicit Gram, incremental swaps and coordinate invariants\n",spatialGramOk?"PASS":"FAIL");
    if(spatialGramOk)passedTests++;

    totalTests++;bool spatialSearchOk=true;
    {
        std::vector<SpatialPsfPoint> points;
        std::vector<int> initial;
        for(int y=0;y<4;y++)for(int x=0;x<5;x++)
        {points.push_back(SpatialPsfPoint(x*12.5,y*25));initial.push_back((x+5*y)%3==0?1:0);}
        SpatialPsfObjective objective;std::string error;
        if(!objective.initialize(points,error))spatialSearchOk=false;
        else
        {
            for(int budget=1;budget<=200;budget+=199)
            {
                SpatialPsfSearchResult a,b,sa;
                if(!RunSpatialPsfSearch(objective,initial,42,budget,a) ||
                   !RunSpatialPsfSearch(objective,initial,42,budget,b) ||
                   !RunSpatialPsfSearch(objective,initial,42,budget,sa,budget))spatialSearchOk=false;
                else
                {
                    if(a.mask!=b.mask||a.bestTrace!=b.bestTrace||a.evaluations!=budget||sa.evaluations!=budget||
                       a.bestTrace.size()!=(size_t)budget||a.mu>objective.measure(initial)+1e-12||
                       fabs(objective.measure(a.mask)-a.mu)>2e-12)spatialSearchOk=false;
                    int count=0,wanted=0;
                    for(size_t i=0;i<initial.size();i++){count+=a.mask[i];wanted+=initial[i];}
                    if(count!=wanted)spatialSearchOk=false;
                    for(size_t i=1;i<a.bestTrace.size();i++)
                        if(a.bestTrace[i]>a.bestTrace[i-1]+1e-14 || a.bestTrace[i]>a.acceptedTrace[i]+1e-14)spatialSearchOk=false;
                    for(size_t i=0;i<std::min<size_t>(64,a.bestTrace.size());i++)
                        if(a.bestTrace[i]!=sa.bestTrace[i] || a.candidateTrace[i]!=sa.candidateTrace[i])spatialSearchOk=false;
                }
            }
            SpatialPsfSearchResult full,invalid;
            if(!RunSpatialPsfSearch(objective,std::vector<int>(20,1),42,200,full)||full.evaluations!=1||full.status!="single_feasible_mask")spatialSearchOk=false;
            if(RunSpatialPsfSearch(objective,std::vector<int>(20,0),42,200,invalid)||RunSpatialPsfSearch(objective,initial,42,0,invalid))spatialSearchOk=false;
            initial[0]=2;if(RunSpatialPsfSearch(objective,initial,42,200,invalid))spatialSearchOk=false;
        }
    }
    log_test(logFp," [%s] Test 36: Spatial search preserves cardinality, exact budgets, shared SA prefix and reproducibility\n",spatialSearchOk?"PASS":"FAIL");
    if(spatialSearchOk)passedTests++;

    totalTests++;bool spatialScaleOk=true;
    {
        const clock_t started=clock();
        std::vector<SpatialPsfPoint> points;std::vector<int> mask;
        for(int y=0;y<72;y++)for(int x=0;x<126;x++)
        {points.push_back(SpatialPsfPoint(600000+x*25,3000000+y*50));mask.push_back((x+y)%2);}
        SpatialPsfObjective objective;SpatialPsfSearchResult result;std::string error;
        if(!objective.initialize(points,error)||objective.cachedValues()!=9072*112 ||
           !RunSpatialPsfSearch(objective,mask,77,200,result)||result.evaluations!=200||
           fabs(objective.measure(result.mask)-result.mu)>3e-12)spatialScaleOk=false;
        log_test(logFp," SpatialFourier2D 9072 points: cache=%lu complex values; CPU seconds=%.4f\n",
            (unsigned long)objective.cachedValues(),(double)(clock()-started)/CLOCKS_PER_SEC);
    }
    log_test(logFp," [%s] Test 37: Spatial incremental search handles 9072 actual coordinate pairs\n",spatialScaleOk?"PASS":"FAIL");
    if(spatialScaleOk)passedTests++;
