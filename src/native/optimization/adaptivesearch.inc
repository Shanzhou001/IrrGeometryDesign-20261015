// Progress-driven SA/local search, separately versioned as adaptive_hybrid_v4. add by wushanzhou in 2026/10/24
class CachedSamplingObjective // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    int n,m; // add by wushanzhou in 2026/10/24
    std::vector<std::complex<double> > atoms; // add by wushanzhou in 2026/10/24
public: // add by wushanzhou in 2026/10/24
    CachedSamplingObjective(int count,int kept,const std::vector<double>* x,bool forceAtoms=false):n(count),m(kept),atoms((x||forceAtoms)?count*(count-1):0)
    { // add by wushanzhou in 2026/10/24
        if(!x && !forceAtoms)return; // Legacy uniform candidates retain FFT scoring.
        const double period=x?(x->back()-x->front())*n/(n-1):(double)n; // add by wushanzhou in 2026/10/24
        for(int d=1;d<n;d++)for(int j=0;j<n;j++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            double position=x?(*x)[j]-x->front():(double)j; // add by wushanzhou in 2026/10/24
            atoms[(d-1)*n+j]=std::polar(1.0,-6.28318530717958647692*d*position/period); // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    float coherence(const std::vector<int>& mask,const std::vector<double>* weights=NULL,float* full=NULL)const
    { // add by wushanzhou in 2026/10/24
        if(atoms.empty()){float value=SamplingMaskCoherence(mask);if(full)*full=value;return value;}
        double largest=0,weightedLargest=0;
        for(int d=1;d<n;d++) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            std::complex<double> sum(0,0); // add by wushanzhou in 2026/10/24
            for(int j=0;j<n;j++)if(mask[j])sum+=atoms[(d-1)*n+j]; // add by wushanzhou in 2026/10/24
            double value=std::abs(sum)/m;
            largest=std::max(largest,value);
            weightedLargest=std::max(weightedLargest,value*(weights?(*weights)[d-1]:1.0));
        } // add by wushanzhou in 2026/10/24
        if(full)*full=(float)largest;
        return (float)(weights?weightedLargest:largest);
    }
    bool hasAtoms()const { return !atoms.empty(); }
    void computeSums(const std::vector<int>& mask,std::vector<std::complex<double> >& sums)const
    {
        sums.assign(n>1?n-1:0,std::complex<double>(0.0,0.0));
        if(atoms.empty())return;
        for(int d=1;d<n;d++)
        {
            std::complex<double> sum(0,0);
            for(int j=0;j<n;j++)if(mask[j])sum+=atoms[(d-1)*n+j];
            sums[d-1]=sum;
        }
    }
    float exchange1(const std::vector<std::complex<double> >& baseSums,int remIdx,int addIdx,
                    std::vector<std::complex<double> >& candSums,const std::vector<double>* weights=NULL,float* full=NULL)const
    {
        candSums.resize(n>1?n-1:0);
        double largest=0,weightedLargest=0;
        for(int d=1;d<n;d++)
        {
            const std::complex<double> s=baseSums[d-1]-atoms[(d-1)*n+remIdx]+atoms[(d-1)*n+addIdx];
            candSums[d-1]=s;
            const double value=std::abs(s)/m;
            largest=std::max(largest,value);
            weightedLargest=std::max(weightedLargest,value*(weights?(*weights)[d-1]:1.0));
        }
        if(full)*full=(float)largest;
        return (float)(weights?weightedLargest:largest);
    }
    float exchange2(const std::vector<std::complex<double> >& baseSums,int rem1,int add1,int rem2,int add2,
                    std::vector<std::complex<double> >& candSums,const std::vector<double>* weights=NULL,float* full=NULL)const
    {
        candSums.resize(n>1?n-1:0);
        double largest=0,weightedLargest=0;
        for(int d=1;d<n;d++)
        {
            const std::complex<double> s=baseSums[d-1]-atoms[(d-1)*n+rem1]+atoms[(d-1)*n+add1]-atoms[(d-1)*n+rem2]+atoms[(d-1)*n+add2];
            candSums[d-1]=s;
            const double value=std::abs(s)/m;
            largest=std::max(largest,value);
            weightedLargest=std::max(weightedLargest,value*(weights?(*weights)[d-1]:1.0));
        }
        if(full)*full=(float)largest;
        return (float)(weights?weightedLargest:largest);
    } // add by wushanzhou in 2026/10/24
}; // add by wushanzhou in 2026/10/24

bool RunAdaptiveSamplingSearch(int N,int m,const std::vector<int>& initial,const ConstraintWeights& w, // add by wushanzhou in 2026/10/24
    unsigned long long seed,int budget,const AdaptiveSearchOptions& options,ConstrainedResult& out, // add by wushanzhou in 2026/10/24
    const std::vector<double>* x) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    const clock_t started=clock(); // add by wushanzhou in 2026/10/24
    const double wallStarted=samplingWallMs();
    if(N>2048 || options.minimumSa<1 || options.stagnationWindow<1 || budget<1 || budget>10000000 ||
       !search_finite(options.wallLimitMs) || options.wallLimitMs<0) // add by wushanzhou in 2026/10/24
    {out=ConstrainedResult();out.status="invalid_adaptive_options";return false;} // add by wushanzhou in 2026/10/24
    if(!options.eligible.empty())
    {
        int available=0;
        if((int)options.eligible.size()!=N || (int)initial.size()!=N)return false;
        for(int j=0;j<N;j++)
        {
            if((options.eligible[j]!=0 && options.eligible[j]!=1) || (initial[j] && !options.eligible[j]))return false;
            available+=options.eligible[j];
        }
        if(available<m)return false;
    }
    int requiredCount=0;
    if(!options.required.empty())
    {
        if((int)options.required.size()!=N || (int)initial.size()!=N)return false;
        for(int j=0;j<N;j++)
        {
            if((options.required[j]!=0 && options.required[j]!=1) ||
                (options.required[j] && (!initial[j] || (!options.eligible.empty() && !options.eligible[j]))))return false;
            requiredCount+=options.required[j];
        }
        if(requiredCount>m)return false;
    }
    if(!options.frequencyWeights.empty())
    {
        if((int)options.frequencyWeights.size()!=N-1)return false;
        for(int d=0;d<N-1;d++)if(!search_finite(options.frequencyWeights[d]) ||
            options.frequencyWeights[d]<=0 || options.frequencyWeights[d]>1)return false;
    }
    std::vector<int> seedInitial=initial;
    if(w.hardMaxGapM>0 && x && !search_gap_ok(seedInitial,w,x))
    {
        if(!SamplingPhysicalFeasibleMask(*x,m,w.hardMaxGapM,seedInitial,
            options.eligible.empty()?NULL:&options.eligible,
            options.required.empty()?NULL:&options.required))
        {out=ConstrainedResult();out.status="infeasible_physical_hard_gap";return false;}
    }
    if(!RunSamplingSearch(N,m,seedInitial,w,seed,1,SEARCH_SA,out,1,1,x,
        options.eligible.empty()?NULL:&options.eligible))return false; // add by wushanzhou in 2026/10/24
    for(int j=0;j<N;j++)if(out.mask[j] && !options.eligible.empty() && !options.eligible[j])return false;
    for(int j=0;j<N;j++)if(!options.required.empty() && options.required[j] && !out.mask[j])return false;
    // Precomputation time is included; every complete candidate objective is counted. // add by wushanzhou in 2026/10/24
    CachedSamplingObjective objective(N,m,x,!options.frequencyWeights.empty());
    const std::vector<double>* frequencyWeights=options.frequencyWeights.empty()?NULL:&options.frequencyWeights;
    if(frequencyWeights)
    {
        float full=0,weighted=objective.coherence(out.mask,frequencyWeights,&full);
        out.objective+=weighted-out.mu;out.mu=full;
        out.bestTrace.assign(1,out.objective);out.currentTrace.assign(1,out.objective);
    }
    std::vector<int> cur=out.mask; // add by wushanzhou in 2026/10/24
    std::vector<std::complex<double> > curSums,bestSums,candSums;
    if(objective.hasAtoms()){objective.computeSums(cur,curSums);bestSums=curSums;}
    double current=out.objective; // add by wushanzhou in 2026/10/24
    int phase=1,phaseEvals=0,stagnant=0,localFailures=0; // add by wushanzhou in 2026/10/24
    bool perturb=false; // add by wushanzhou in 2026/10/24
    out.phaseTrace.push_back(0); // add by wushanzhou in 2026/10/24
    out.acceptedTrace.push_back(out.objective);out.temperatureTrace.push_back(0);
    out.wallTraceMs.push_back(samplingWallMs()-wallStarted);
    if(out.status=="single_feasible_mask" || requiredCount==m)
    {out.status="single_feasible_mask";out.runtimeWallMs=samplingWallMs()-wallStarted;return true;}
    exprng::seed(seed); // add by wushanzhou in 2026/10/24
    const long long attemptLimit=(long long)budget*100; // add by wushanzhou in 2026/10/24
    long long attempts=0; // add by wushanzhou in 2026/10/24
    while(out.evals<budget && attempts<attemptLimit) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        if(options.wallLimitMs>0 && samplingWallMs()-wallStarted>=options.wallLimitMs)break;
        attempts++;
        std::vector<int> candidate=phase==2?out.mask:cur; // add by wushanzhou in 2026/10/24
        std::vector<int> ones,zeros; // add by wushanzhou in 2026/10/24
        for(int j=0;j<N;j++)if(options.eligible.empty() || options.eligible[j])
        {
            if(candidate[j]){if(options.required.empty() || !options.required[j])ones.push_back(j);}
            else zeros.push_back(j);
        }
        const int source=ones[exprng::integer()%ones.size()]; // add by wushanzhou in 2026/10/24
        int target=zeros[exprng::integer()%zeros.size()]; // add by wushanzhou in 2026/10/24
        const int evaluatedPhase=perturb?3:phase; // add by wushanzhou in 2026/10/24
        if(phase==2 && (!options.mixedNeighborhood || phaseEvals%2==0)) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            // Symmetric nearest-empty moves alternate with full exchanges; no free objective screening. // add by wushanzhou in 2026/10/24
            int direction=exprng::integer()%2?1:-1; // add by wushanzhou in 2026/10/24
            target=source+direction; // add by wushanzhou in 2026/10/24
            while(target>=0 && target<N && (candidate[target] || (!options.eligible.empty() && !options.eligible[target])))target+=direction; // add by wushanzhou in 2026/10/24
            if(target<0 || target>=N)continue; // add by wushanzhou in 2026/10/24
            if(!options.mixedNeighborhood && abs(target-source)!=1)continue; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        candidate[source]=0;candidate[target]=1; // add by wushanzhou in 2026/10/24
        int source2=-1,target2=-1;
        if(perturb && ones.size()>1 && zeros.size()>1) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            int a=ones[exprng::integer()%ones.size()],b=zeros[exprng::integer()%zeros.size()]; // add by wushanzhou in 2026/10/24
            if(a!=source && b!=target){candidate[a]=0;candidate[b]=1;source2=a;target2=b;} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        if(!search_gap_ok(candidate,w,x))continue; // add by wushanzhou in 2026/10/24
        ConstrainedResult measured; // add by wushanzhou in 2026/10/24
        float full=0,score=0;
        if(objective.hasAtoms())
        {
            const std::vector<std::complex<double> >& baseSums=phase==2?bestSums:curSums;
            score=(source2>=0 && target2>=0)
                ?objective.exchange2(baseSums,source,target,source2,target2,candSums,frequencyWeights,&full)
                :objective.exchange1(baseSums,source,target,candSums,frequencyWeights,&full);
        }
        else score=objective.coherence(candidate,frequencyWeights,&full);
        search_measure(candidate,w,m,measured,1,x,score);measured.mu=full;
        if(measured.mu<0 || !search_finite(measured.objective)){out.status="objective_failure";return false;} // add by wushanzhou in 2026/10/24
        out.evals++;phaseEvals++; // add by wushanzhou in 2026/10/24
        if(evaluatedPhase==3)out.restartEvaluations++; // add by wushanzhou in 2026/10/24
        else if(phase==1)out.saEvaluations++;else out.localEvaluations++; // add by wushanzhou in 2026/10/24
        const double delta=measured.objective-current; // add by wushanzhou in 2026/10/24
        const double progress=(double)(out.evals-2)/std::max(1,budget-2); // add by wushanzhou in 2026/10/24
        double temperature=0.1*pow(0.0001,progress); // add by wushanzhou in 2026/10/24
        if(perturb)temperature=std::max(temperature,0.01*(1.0-progress)); // add by wushanzhou in 2026/10/24
        if(phase==1 && (delta<=0 || exprng::uniform()<exp(-delta/std::max(temperature,1e-8)))) // add by wushanzhou in 2026/10/24
        {cur=candidate;current=measured.objective;if(objective.hasAtoms())curSums=candSums;} // add by wushanzhou in 2026/10/24
        bool improved=measured.objective+1e-8f<out.objective; // add by wushanzhou in 2026/10/24
        if(improved) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            out.mask=measured.mask;out.mu=measured.mu;out.objective=measured.objective; // add by wushanzhou in 2026/10/24
            out.maxGap=measured.maxGap;out.maxGapM=measured.maxGapM;out.nnCv=measured.nnCv; // add by wushanzhou in 2026/10/24
            out.unitOccupancy=measured.unitOccupancy;stagnant=0;localFailures=0; // add by wushanzhou in 2026/10/24
            if(objective.hasAtoms())bestSums=candSums;
            if(phase==2){cur=out.mask;current=out.objective;if(objective.hasAtoms())curSums=bestSums;} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        else{stagnant++;if(phase==2)localFailures++;} // add by wushanzhou in 2026/10/24
        out.bestTrace.push_back(out.objective);out.currentTrace.push_back(measured.objective); // add by wushanzhou in 2026/10/24
        out.phaseTrace.push_back(evaluatedPhase);perturb=false; // add by wushanzhou in 2026/10/24
        out.acceptedTrace.push_back((float)current);out.temperatureTrace.push_back(temperature);
        out.wallTraceMs.push_back(samplingWallMs()-wallStarted);
        if(options.localSearch && phase==1 && // add by wushanzhou in 2026/10/24
           (options.adaptiveSwitch?(phaseEvals>=options.minimumSa && stagnant>=options.stagnationWindow):phaseEvals>=64)) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            phase=2;phaseEvals=0;localFailures=0;out.switches++;cur=out.mask;current=out.objective;if(objective.hasAtoms()){objective.computeSums(out.mask,bestSums);curSums=bestSums;} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        else if(phase==2 && localFailures>=options.stagnationWindow) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            if(!options.recycleBudget)break; // add by wushanzhou in 2026/10/24
            // Local stagnation returns remaining budget to SA with a counted two-exchange escape. // add by wushanzhou in 2026/10/24
            phase=1;phaseEvals=0;stagnant=0;out.switches++;cur=out.mask;current=out.objective;perturb=options.perturbation;if(objective.hasAtoms()){objective.computeSums(out.mask,bestSums);curSums=bestSums;} // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    out.hardConstraintSatisfied=search_gap_ok(out.mask,w,x); // add by wushanzhou in 2026/10/24
    out.runtimeMs=(double)(clock()-started)*1000.0/CLOCKS_PER_SEC; // add by wushanzhou in 2026/10/24
    out.proposalAttempts=attempts;out.runtimeWallMs=samplingWallMs()-wallStarted;
    out.status=out.evals>=budget?"budget_exhausted":attempts>=attemptLimit?"feasible_proposals_exhausted":"local_stagnation"; // add by wushanzhou in 2026/10/24
    if(out.evals<budget && options.wallLimitMs>0 && out.runtimeWallMs>=options.wallLimitMs)out.status="wall_limit";
    return out.hardConstraintSatisfied && ValidateSamplingMask(N,m,&out.mask[0]); // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24
