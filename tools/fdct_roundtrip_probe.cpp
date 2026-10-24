// Independent analytic complex-array roundtrip probe; no threshold or observation projection.
#include "fdct_wrapping.hpp"
#include <cstdio>
#include <cmath>
using namespace fdct_wrapping_ns;
static cpx signal(int i,int j)
{return cpx(sin(.13*i+.37*j)+.3*cos(.07*i*j),.2*cos(.17*i-.19*j));}
int main()
{
    const int dimensions[5][2]={{64,32},{64,64},{250,128},{256,128},{499,128}};
    bool passed=true;printf("nt,nx,scales,angles,relative_l2,max_abs,coefficient_energy_ratio\n");
    for(int d=0;d<5;d++)for(int scales=3;scales<=4;scales++)for(int angles=8;angles<=16;angles*=2)
    {
        int nt=dimensions[d][0],nx=dimensions[d][1];CpxNumMat input(nt,nx),output;
        for(int i=0;i<nt;i++)for(int j=0;j<nx;j++)input(i,j)=signal(i,j);
        std::vector<std::vector<CpxNumMat> > coefficients;
        if(fdct_wrapping(nt,nx,scales,angles,0,input,coefficients)!=0)return 1;
        if(ifdct_wrapping(nt,nx,scales,angles,0,coefficients,output)!=0)return 1;
        double energy=0,error=0,maximum=0,coefficientEnergy=0;
        for(int i=0;i<nt;i++)for(int j=0;j<nx;j++)
        {double delta=std::abs(output(i,j)-signal(i,j));maximum=std::max(maximum,delta);error+=delta*delta;energy+=std::norm(signal(i,j));}
        for(size_t s=0;s<coefficients.size();s++)for(size_t a=0;a<coefficients[s].size();a++)
            for(int i=0;i<coefficients[s][a].m();i++)for(int j=0;j<coefficients[s][a].n();j++)coefficientEnergy+=std::norm(coefficients[s][a](i,j));
        double relative=sqrt(error/energy);if(relative>1e-10 || relative!=relative)passed=false;
        printf("%d,%d,%d,%d,%.15g,%.15g,%.15g\n",nt,nx,scales,angles,relative,maximum,coefficientEnergy/energy);
    }
    return passed?0:1;
}
