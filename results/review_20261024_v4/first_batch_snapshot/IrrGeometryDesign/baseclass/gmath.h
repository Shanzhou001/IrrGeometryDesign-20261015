#ifndef GMATH_H
#define GMATH_H

#include <math.h>
#include <stdio.h>
#include <vector>

#include <algorithm>

#include <numeric>

using namespace std;

class GPoint3d;
class GVector3d;
class GMatrix3d;

#define PI 3.1415926535898

#define RADIAN_TO_ANGLE 57.29577985
#define ANGLE_TO_RADIAN 0.01745329
#define GMATH_ZERO		1.0E-6
#define IS_ZERO(x)		(fabs(x) < GMATH_ZERO)

#define MAX_UINT32 0xffffffff
#define MAX_INT32  0x7fffffff
#define MIN_INT32  0x80000000
#define MAX_UINT16 0xffff
#define MAX_INT16  -32768
#define MIN_INT16  32767
#define MAX_FLOAT  3.2767e38f
#define MIN_FLOAT  -3.2768e38f

// 求两个数中较小者
template <class TData> TData gGetMin(const TData &x, const TData &y)
{
	return x < y ? x : y;
}

// 求两个数中较大者
template <class TData> TData gGetMax(const TData &x, const TData &y)
{
	return x > y ? x : y;
}

// 求数组中最小元素的下标
template <class TData> int gGetMin(const TData *data, int begin, int end)
{
	int index = begin;
	TData min = data[index];
	for(int i=begin+1; i<=end; i++)
	{
		if(data[i] < min)
		{
			min = data[i];
			index = i;
		}
	}
	return index;
}


// 求数组中最大元素的下标
template <class TData> int gGetMax(const TData *data, int begin, int end)
{
	int index = begin;
	TData max = data[index];
	for(int i=begin+1; i<=end; i++)
	{
		if(data[i] > max)
		{
			max = data[i];
			index = i;
		}
	}
	return index;
}

// 四舍五入
int gRound( double x );

// 取整
int gTrunc( double x );

// 平方
double gSqr( double x );

bool gFittingLine(float *xArray, float *yArray, int firstIndex, int lastIndex,
				  float &a, float &b);	//最小二乘法拟合直线

void gRotate(double sinAF, double cosAF, double sx, double sy, double &dx, double &dy);

// 2D距离
double gDistance2D(double x0, double y0, double x1, double y1); 

// 3D距离
double gDistance3D(const GPoint3d &v0, const GPoint3d &v1);

GVector3d gGetNormal(const GPoint3d &pt0, const GPoint3d &pt1, const GPoint3d &pt2);

// 求你矩阵
bool gInverseMatrix(double *m, int n);

// 选主元三角分解法
void gSelSlove(double **matrix, double *result, int dim);	

// 追赶法求解三对角矩阵
void gTridagSlove( double *aArray, double *bArray,
					double *cArray, double *dArray,
					int count, double *rArray );

// 解一元二次方程
int gSolveTwoeTimeEqu(double a[3], double r[2]);

// 解一元三次方程
int gSolveThreeTimeEqu(double a[4], double r[3]);

// 牛顿迭代法解一元三次方程
double gNewtonSolveThreeTimeEqu(double a[4], double r, 
							  double errLimit = 0.01, int itCount = 32);

//快速时窗均衡处理
void gAmpequGain(float *destData, const float *srcData, int pointCount, int npowin=400, int step=0);

//固定增益
void gFixedGain(float *destData, const float *srcData, int pointCount, float fixed );

//三个点求面积，赵虎添加
double gArea(GPoint3d p1,GPoint3d p2,GPoint3d p3);
//多项式拟合，赵虎添加
//void gPolyfit(int n,int m,double x[],double y[],double a[],double dt[]);
void gPolyfit(int m,vector<double> x,vector <double> y,double *a,double dt[]);

//求方位角
int gAngle(float sx,float sy,float rx,float ry);//add by zhaohu

//求最大公约数
int gcd(int m,int n);	//add by LiuWei

#endif
