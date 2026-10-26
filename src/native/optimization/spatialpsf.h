#ifndef IRR_SPATIAL_PSF_H
#define IRR_SPATIAL_PSF_H

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "../core/ExpRng.h"

// SpatialFourier2D_v1 is separate from the frozen 1D v4/v5 protocols.
// Atoms: exp(-2*pi*i*(kx*(x-x0)/Lx+ky*(y-y0)/Ly))/sqrt(m).
// All atom-pair differences are evaluated; conjugates share one cached sum.
struct SpatialPsfPoint
{
    double x,y;
    SpatialPsfPoint(double px=0,double py=0):x(px),y(py){}
};

struct SpatialPsfFrequency
{
    int dx,dy;
    double fx,fy;
    SpatialPsfFrequency(int px,int py,double x,double y):dx(px),dy(py),fx(x),fy(y){}
};

class SpatialPsfObjective
{
    std::vector<std::complex<double> > cache;
    int n;
    static bool finite(double v)
    { return v==v && v<=std::numeric_limits<double>::max() && v>=-std::numeric_limits<double>::max(); }
    static double period(std::vector<double>& axis, int& atoms, int requested)
    {
        std::sort(axis.begin(),axis.end());
        axis.erase(std::unique(axis.begin(),axis.end()),axis.end());
        atoms=std::min(requested,(int)axis.size());
        if(axis.size()==1)return 1.0; // Constant axis has only its DC atom.
        std::vector<double> gaps;
        for(size_t i=1;i<axis.size();i++)gaps.push_back(axis[i]-axis[i-1]);
        std::sort(gaps.begin(),gaps.end());
        double median=gaps[gaps.size()/2];
        if(gaps.size()%2==0)median=.5*(median+gaps[gaps.size()/2-1]);
        return axis.back()-axis.front()+median;
    }
public:
    double originX,originY,periodX,periodY;
    int atomsX,atomsY;
    std::vector<SpatialPsfFrequency> differences;
    SpatialPsfObjective():n(0),originX(0),originY(0),periodX(0),periodY(0),atomsX(0),atomsY(0){}
    int size()const{return n;}
    size_t cachedValues()const{return cache.size();}
    bool initialize(const std::vector<SpatialPsfPoint>& points,std::string& error,int requestedX=8,int requestedY=8)
    {
        n=0;cache.clear();differences.clear();
        if(points.size()<2 || points.size()>100000 || requestedX<1 || requestedX>16 || requestedY<1 || requestedY>16)
        {error="invalid_spatial_grid_or_size";return false;}
        std::vector<double> x,y;
        std::set<std::pair<double,double> > unique;
        for(size_t i=0;i<points.size();i++)
        {
            if(!finite(points[i].x)||!finite(points[i].y))
            {error="nonfinite_spatial_coordinate";return false;}
            if(!unique.insert(std::make_pair(points[i].x,points[i].y)).second)
            {error="duplicate_spatial_coordinate";return false;}
            x.push_back(points[i].x);y.push_back(points[i].y);
        }
        originX=*std::min_element(x.begin(),x.end());originY=*std::min_element(y.begin(),y.end());
        periodX=period(x,atomsX,requestedX);periodY=period(y,atomsY,requestedY);
        if(!finite(periodX)||!finite(periodY)||periodX<=0||periodY<=0||atomsX*atomsY<2)
        {error="degenerate_spatial_dictionary";return false;}
        for(int dy=0;dy<atomsY;dy++)for(int dx=-(atomsX-1);dx<atomsX;dx++)
            if(dy>0 || dx>0)differences.push_back(SpatialPsfFrequency(dx,dy,dx/periodX,dy/periodY));
        if(points.size()*differences.size()>16000000)
        {error="spatial_cache_limit";differences.clear();return false;}
        n=(int)points.size();cache.resize(points.size()*differences.size());
        const double twoPi=6.28318530717958647692;
        for(size_t f=0;f<differences.size();f++)for(int i=0;i<n;i++)
        {
            const double phase=-twoPi*(differences[f].fx*(points[i].x-originX)+differences[f].fy*(points[i].y-originY));
            if(!finite(phase)){error="nonfinite_spatial_phase";n=0;cache.clear();return false;}
            cache[f*n+i]=std::complex<double>(cos(phase),sin(phase));
        }
        error.clear();return true;
    }
    bool sums(const std::vector<int>& mask,std::vector<std::complex<double> >& out,int& keep)const
    {
        if(!n || mask.size()!=(size_t)n)return false;
        keep=0;
        for(int i=0;i<n;i++){if(mask[i]!=0&&mask[i]!=1)return false;keep+=mask[i];}
        if(!keep)return false;
        out.assign(differences.size(),std::complex<double>(0,0));
        for(size_t f=0;f<differences.size();f++)for(int i=0;i<n;i++)if(mask[i])out[f]+=cache[f*n+i];
        return true;
    }
    double coherence(const std::vector<std::complex<double> >& sum,int keep)const
    {
        if(!n || keep<1 || sum.size()!=differences.size())return -1;
        double peak=0;
        for(size_t f=0;f<sum.size();f++)peak=std::max(peak,std::abs(sum[f])/keep);
        return std::min(1.0,peak);
    }
    double measure(const std::vector<int>& mask)const
    {
        int keep=0;std::vector<std::complex<double> > value;
        return sums(mask,value,keep)?coherence(value,keep):-1;
    }
    bool exchange(const std::vector<std::complex<double> >& current,int remove,int add,
                  std::vector<std::complex<double> >& candidate)const
    {
        if(remove<0||remove>=n||add<0||add>=n||remove==add||current.size()!=differences.size())return false;
        candidate.resize(current.size());
        for(size_t f=0;f<current.size();f++)candidate[f]=current[f]-cache[f*n+remove]+cache[f*n+add];
        return true;
    }
};

struct SpatialPsfSearchResult
{
    std::vector<int> mask,phase;
    std::vector<double> candidateTrace,acceptedTrace,bestTrace;
    double mu;
    int evaluations;
    std::string status;
    SpatialPsfSearchResult():mu(-1),evaluations(0){}
};

// Fixed SA prefix followed by greedy random-swap local search. Initial scoring
// and every complete candidate score count once, including rejected candidates.
inline bool RunSpatialPsfSearch(const SpatialPsfObjective& objective,const std::vector<int>& initial,
    unsigned long long seed,int budget,SpatialPsfSearchResult& out,int saPrefix=64)
{
    out=SpatialPsfSearchResult();int keep=0;
    std::vector<std::complex<double> > currentSum,bestSum,candidateSum;
    if(budget<1||budget>1000000||saPrefix<1||!objective.sums(initial,currentSum,keep))
    {out.status="invalid_spatial_search_input";return false;}
    std::vector<int> current=initial,ones,zeros;
    for(size_t i=0;i<current.size();i++)(current[i]?ones:zeros).push_back((int)i);
    out.mask=initial;out.mu=objective.coherence(currentSum,keep);bestSum=currentSum;
    double accepted=out.mu;
    out.evaluations=1;out.bestTrace.push_back(out.mu);out.acceptedTrace.push_back(accepted);
    out.candidateTrace.push_back(accepted);out.phase.push_back(0);
    if(zeros.empty()){out.status="single_feasible_mask";return true;}
    exprng::seed(seed);
    const int prefix=std::min(saPrefix,budget);
    while(out.evaluations<budget)
    {
        const bool local=out.evaluations>=prefix;
        if(out.evaluations==prefix)
        {
            current=out.mask;currentSum=bestSum;accepted=out.mu;ones.clear();zeros.clear();
            for(size_t i=0;i<current.size();i++)(current[i]?ones:zeros).push_back((int)i);
        }
        const size_t oi=exprng::integer()%ones.size(),zi=exprng::integer()%zeros.size();
        const int remove=ones[oi],add=zeros[zi];
        objective.exchange(currentSum,remove,add,candidateSum);
        const double candidate=objective.coherence(candidateSum,keep),delta=candidate-accepted;
        // Both the SA baseline and hybrid use the same temperature schedule,
        // so changing the switching point preserves their common SA prefix.
        const double fraction=(double)out.evaluations/63.0;
        const double temperature=.05*pow(.001/.05,std::min(1.0,fraction));
        if(delta<0 || (!local && exprng::uniform()<exp(-delta/temperature)))
        {
            current[remove]=0;current[add]=1;ones[oi]=add;zeros[zi]=remove;
            currentSum=candidateSum;accepted=candidate;
            if(candidate<out.mu){out.mu=candidate;out.mask=current;bestSum=currentSum;}
        }
        out.evaluations++;
        out.candidateTrace.push_back(candidate);out.acceptedTrace.push_back(accepted);
        out.bestTrace.push_back(out.mu);out.phase.push_back(local?2:1);
    }
    out.status="evaluation_limit";return true;
}

#endif
