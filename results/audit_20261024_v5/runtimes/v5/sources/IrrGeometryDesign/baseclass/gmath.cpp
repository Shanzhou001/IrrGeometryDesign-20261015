#include "gmath.h"
#include "gvector3d.h"
//#include "gmatrix3d.h"
#include "gutils.h"

// AGC增益的系数缓存
#define GAGC_BUF_SIZE 10240
float gAGCBuf[GAGC_BUF_SIZE];

int gRound(double x)
{
	return x > 0 ? (int)(x+0.5) : (int)(x-0.5);
}

int gTrunc(double x)
{
	return (int)x;
}

double gSqr(double x)
{
	return x*x;
}

void gRotate(double sinAF, double cosAF, double sx, double sy, double &dx, double &dy)
{
	dx = sx*cosAF-sy*sinAF;
	dy = sx*sinAF+sy*cosAF;
}

bool gFittingLine( float *xArray, float *yArray, int firstIndex, int lastIndex,
				  float &a, float &b )
{
	int count = lastIndex-firstIndex+1;
    if(count < 2) return false;
    float s0 = (float)count, s1 = 0, s2 = 0, t0 = 0, t1 = 0;
    for(int i=firstIndex;i<=lastIndex;i++) {
		s1 += xArray[i];
		s2 += (xArray[i]*xArray[i]);
		t0 += yArray[i];
		t1 += (xArray[i]*yArray[i]);
    }
    float d = s0*s2-s1*s1;
    b = (s2*t0-s1*t1)/d;
    a = (s0*t1-s1*t0)/d;
    return true;
}

double gDistance2D( double x0, double y0, double x1, double y1 )
{
	return sqrt( (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0) );
}

double gDistance3D(const GPoint3d &v0, const GPoint3d &v1)
{
	return sqrt((v1.x()-v0.x())*(v1.x()-v0.x()) + 
				(v1.y()-v0.y())*(v1.y()-v0.y()) +
				(v1.z()-v0.z())*(v1.z()-v0.z()));
}

GVector3d gGetNormal(const GPoint3d &pt0, const GPoint3d &pt1, const GPoint3d &pt2)
{
	GVector3d v01, v21, n;
	v01.set(pt0.x()-pt1.x(), pt0.y()-pt1.y(), pt0.z()-pt1.z());
	v21.set(pt2.x()-pt1.x(), pt2.y()-pt1.y(), pt2.z()-pt1.z());
	n = v21.crossMult(v01);
	if(n.y()<0)
	{
		n=-n;
	}
	n.normalize();
	return n;
}

bool gInverseMatrix(double *m, int n)
{
    int i, j, k, l, u, v;
    double d, p;
    
    int *is = new int[n];
    int *js = new int[n];
    
    for(k=0; k<=n-1; k++)
    {
        d = 0;
        for(i=k; i<=n-1; i++)
        {
            for(j=k; j<=n-1; j++)
            {
                l = i*n+j;
                p = fabs(m[l]);
                if(p > d) { d = p;  is[k] = i;  js[k] = j; }
            }
        }
        
        if(IS_ZERO(d))
        {
            delete []is;
            delete []js;
            return false;
        }
        
        if(is[k] != k)
        {
            for(j=0; j<=n-1; j++)
            {
                u = k*n+j;
                v = is[k]*n+j;
                gSwap(m[u], m[v]);
            }
        }
        if(js[k] != k)
        {
            for(i=0; i<=n-1; i++)
            {
                u = i*n+k;
                v = i*n+js[k];
                gSwap(m[u], m[v]);
            }
        }
        
        l = k*n+k;
        m[l] = 1/m[l];
        for(j=0; j<=n-1; j++)
        {
            if(j != k)
            {
                u = k*n+j;
                m[u] = m[u]*m[l];
            }
        }
        
        for(i=0; i<=n-1; i++)
        {
            if(i != k)
            {
                for(j=0; j<=n-1; j++)
                {
                    if(j != k)
                    {
                        u = i*n+j;
                        m[u] = m[u] - m[i*n+k]*m[k*n+j];
                    }
                }
            }
        }
        
        for(i=0; i<=n-1; i++)
        {
            if(i != k)
            {
                u = i*n+k;
                m[u] = -m[u]*m[l];
            }
        }
    }
        
    for(k=n-1; k>=0; k--)
    {
        if(js[k] != k)
        {
            for(j=0; j<=n-1; j++)
            {
                u = k*n+j;
                v = js[k]*n+j;
                gSwap(m[u], m[v]);
            }
        }
        if(is[k] != k)
        {
            for(i=0; i<=n-1; i++)
            {
                u = i*n+k;
                v = i*n+is[k];
                gSwap(m[u], m[v]);
            }
        }
    }
    delete []is;
    delete []js;
    return true;
}

double gUVectorMul(int i, int j, double **matrix)
{
    int k;
	double v=0.0f;
    for(k=0;k<i;k++) v = v+matrix[i][k]*matrix[k][j];
	return v;
}

double gLVectorMul(int i, int j, double **matrix)
{
    int k;
	double v = 0.0f;
	for(k=0; k<j; k++) v = v+matrix[i][k] * matrix[k][j];
	return v;
}

void gSelSlove(double **matrix, double *result, int dim)		//有问题待解决
{
    double *temp, s1, s2;
	int i, j;
	for(i=1; i<dim; i++)  //选第一列主元
	{
		if(fabs(matrix[0][0]) < fabs(matrix[i][0]))
		{
			temp = matrix[0];
			matrix[0] = matrix[i];
			matrix[i] = temp;
		}
	}

	for(i=1; i<dim; i++) //算第一列
		matrix[i][0] = matrix[i][0] / matrix[0][0];

	for(i=1; i<dim; i++)
	{
		s1 = matrix[i][i] - gUVectorMul(i, i, matrix);
		for(j=i+1; j<dim; j++)
		{
            s2 = matrix[j][i] - gLVectorMul(j, i, matrix);
			if(fabs(s2) > fabs(s1))
			{
				temp = matrix[i];
				matrix[i] = matrix[j];
				matrix[j] = temp;
				s1 = s2;
			}
		}//选主元
		matrix[i][i] = s1;
		for(j=i+1; j<=dim; j++)
			matrix[i][j] = matrix[i][j] - gUVectorMul(i, j, matrix);
		for(j=i+1; j<dim; j++)
			matrix[j][i] = (matrix[j][i]-gLVectorMul(j, i, matrix)) / matrix[i][i];
	}
	double sum = 0; //余项和
	for(i=dim-1; i>=0; i--)
	{
		for(j=dim-1;j>i;j--) sum += matrix[i][j]*result[j];
		result[i] = (matrix[i][dim]-sum) / matrix[i][i];
		sum = 0;
	}
}

void gTridagSlove( double *aArray, double *bArray,
					double *cArray, double *dArray,
					int count, double *rArray )
{
	int i;
	for(i=0;i<count-1;i++) 
	{
		aArray[i] = aArray[i]/bArray[i];
		bArray[i+1] = bArray[i+1]-aArray[i]*cArray[i];
		dArray[i+1] = dArray[i+1]-aArray[i]*dArray[i];
	}
	rArray[count-1] = dArray[count-1]/bArray[count-1];
	for(i=count-2;i>=0;i--) 
	{
		rArray[i] =(dArray[i]-cArray[i]*rArray[i+1])/bArray[i];
	}
}

// 解一元二次方程
int gSolveTwoeTimeEqu(double a[3], double r[2])
{
	if(IS_ZERO(a[0])) //一元一次方程
	{
		if(a[1]==0) return 0;
		r[0] = -a[2]/a[1];
		return 1;
	}

	double det = a[1]*a[1] - 4*a[0]*a[2];
	if(det < 0) return 0;

	if(det < GMATH_ZERO)
	{
		r[0] = -a[1]/2/a[0];
		return 1;
	}
	det = sqrt(det);
	r[0] = (-a[1]+det)/2/a[0];
	r[1] = (-a[1]-det)/2/a[0];
	return 2;
}

// 解一元三次方程
int gSolveThreeTimeEqu(double a[4], double r[3])
{
	if(IS_ZERO(a[0]))	//二元一次方程
	{
		return gSolveTwoeTimeEqu(&(a[1]), r);
	}
	for(int i=1; i<4; i++)
	{
		a[i] /= a[0];
	}
	a[0] = 1;

	double p, q;
	p = a[2]-a[1]*a[1]/3;
	q = 2*a[1]*a[1]*a[1]/27 - a[1]*a[2]/3+a[3];

	if(IS_ZERO(p))
	{
		if(q > 0)
		{
			r[0] = -pow(q, 1.0/3.0);
			r[0] -= a[1]/3;
		}
		else
		{
			r[0] = pow(-q, 1.0/3.0);
			r[0] -= a[1]/3;
		}
		return 1;
	}

	if(IS_ZERO(q))
	{
		r[0] = 0;
		r[0] -= a[1]/3;
		if(p < 0)
		{
			r[1] = sqrt(-p);
			r[1] -= a[1]/3;
			return 2;
		}
		return 1;
	}

	double d = q*q/4+p*p*p/27;
	double t = -q/2;
	if(d>0)
	{
		if(t+sqrt(d)>0)
		{
			r[0] = pow(t+sqrt(d),1./3.);
		}
		else
		{
			r[0] = -pow(-t-sqrt(d),1./3.);
		}
		if(t-sqrt(d)>0)
		{
			r[0] += pow(t-sqrt(d),1./3.);
		}
		else
		{
			r[0] -= pow(-t+sqrt(d),1./3.);
		}
		r[0] -= a[1]/3;
		return 1;
	}
	else if(d==0)
	{
		double temp = pow(4*q,1./3.);;
		r[0] = -temp;
		r[0] -= a[1]/3;
		r[1] = temp/2;
		r[1] -= a[1]/3;
		return 2;
	}
	else
	{
		double temp = sqrt(-3*p);
		double arc = acos(-3*q*temp/2/p/p);
		r[0]= (2*temp*cos(arc/3)/3);
		r[0] -= a[1]/3;
		r[1] = (-temp *(cos(arc/3) + sqrt(3.)*sin(arc/3))/3);
		r[1] -= a[1]/3;
		r[2] = (-temp *(cos(arc/3) - sqrt(3.)*sin(arc/3))/3);
		r[2] -= a[1]/3;
		return 3;
	}
}

double gNewtonSolveThreeTimeEqu(double a[4], double r, 
							  double errLimit, int itCount)
{
	int i;
	double x, err;

	err = 1000;
	x = r;
	for(i=0; i<itCount && err > errLimit; i++)
	{
		double f = a[0]*x*x*x + a[1]*x*x + a[2]*x + a[3];
		double df = 3*a[0]*x*x + 2*a[1]*x + a[2];
		if(IS_ZERO(df)) break ;
		x = x - f / df;
		err = fabs(r - x);
		r = x;
	}
	return r;
}

void gFixedGain(float *destData, const float *srcData, int pointCount, float fixed )
{
	int i;
	fixed = (float)pow(10, fixed/20);
	
	for( i=0; i<pointCount; i++ )
	{
		destData[i] = fixed*srcData[i];
	}
}

void gAmpequGain(float *destData, const float *srcData, int pointCount, int npowin, int step)
{
	int i, j, k, npo, t;
	float sum, preSum, *w;
	
	w = gAGCBuf;
	npowin = npowin/2;
	t = step <= 0 ? npowin : step;
	if(npowin < t) t = npowin;
	npo = pointCount;
	
	for(i=0; i<npo; i++)  w[i] = 1;
	
	// 起始部分系数计算
	sum = 0;
	for(i=0; i<npowin*2; i++)
		sum += (float)fabs( srcData[i] );
	preSum = sum;
	sum = sum/(npowin*2);
	
	if(sum > 1e-8f) 
	{
		w[0] = 1.0f / sum;
		for(i=1; i<=npowin; i++)
			w[i] = w[0];
	}
	
	// 中间部分系数计算
	for(i=npowin+t; i<npo-npowin; i+=t)
	{
		sum = preSum;
		for(j=i-npowin-t; j<i-npowin; j++)
			sum -= (float)fabs(srcData[j]);
		for(j=i+npowin-t; j<i+npowin; j++)
			sum += (float)fabs(srcData[j]);
		preSum = sum;
		sum /= npowin*2;
		if(sum > 1e-8f) w[i] = 1.0f/sum;
		for(k=t-1; k>0; k--)
		{
			w[i-k] = w[i-t] + (w[i]-w[i-t])*(t-k)/t;
		}
	}
	
	// 将最后一个可计算系数填充到余下的系数数组中
	for(j=i-t; j<npo; j++) w[j] = w[i-t];
	
	// 根据系数进行增益处理
	for( i=0; i<npo; i++ ) 
	{
		destData[i] = srcData[i]*w[i];
	}
}

//三个点求面积，赵虎添加
double gArea(GPoint3d p1,GPoint3d p2,GPoint3d p3)
{
	double s;
	s=0.5*(p1.x()*(p2.y()-p3.y())+p2.x()*(p3.y()-p1.y())+p3.x()*(p1.y()-p2.y()));//需要有方向

//	s=0.5*(p1.x()*p2.y()+p2.x()*p3.y()+p3.x()*p1.y()-p1.x()*p3.y()-p2.x()*p1.y()-p3.x()*p2.y());

// 	double abx=p2.x()-p1.x();
// 	double aby=p2.y()-p1.y();
// 	double bcx=p3.x()-p2.x();
// 	double bcy=p3.y()-p2.y();
// 	s=0.5*(abx*bcy-aby*bcx);

// 	double a=gDistance2D(p1.x(),p1.y(),p2.x(),p2.y());
// 	double b=gDistance2D(p2.x(),p2.y(),p3.x(),p3.y());
// 	double c=gDistance2D(p3.x(),p3.y(),p1.x(),p1.y());
// 	double p=(a+b+c)*0.5;
// 	s=sqrt(p*(p-a)*(p-b)*(p-c));

	return fabs(s);
// 面积S=abs((x1*y2   +   x2*y3   +   x3*y1   -   x1*y3   -   x2*y1   -   x3*y2)/2) 
//	海伦公式：AB=a BC =b AC=c p=(a+b+c)/2 面积S=sqrt(p*(p-a)*(p-b)*(p-c)).

}
//多项式拟合，赵虎添加
// n 表示给定数据点的个数
// m 表示多项式的项数，m-1阶
// x[] 表示n个数据点的x坐标
// y[] 表示n个数据点的y坐标
// a[] 表示多项式的系数向量
// dt[] dt[0] 表示返回多项式与数据点误差平方和，dt[1]表示多项式与数据点误差绝对值；dt[2]返回多项式与数据点误差绝对值的最大值
/*
void gPolyfit(int n,int m,double x[],double y[],double a[],double dt[])
{
	int i,j,k;
	double alpha,p,q,g,w,beta,d1,d2,s[20],t[20],b[20];
	for(i=0;i<=m-1;i++) a[i]=0.0;
	if(m>n) m=n;
	if(m>20) m=20;
	b[0]=1.0;d1=n;alpha=0.0;q=0.0;
	for(i=0;i<=n-1;i++) {alpha=alpha+x[i];q=q+y[i];}
	q=q/d1;
	alpha=alpha/d1;
	a[0]=q*b[0];
	if(m>1)
	{
		t[1]=1.0;t[0]=-alpha;
		d2=0.0;q=0.0;g=0.0;
		for(i=0;i<=n-1;i++)
		{
			w=x[i]-alpha;
			d2=d2+w*w;
			q=q+y[i]*w;
			g=g+x[i]*w*w;
		}
		q=q/d2;
		alpha=g/d2;
		beta=d2/d1;
		d1=d2;
		a[1]=q*t[1];
		a[0]=q*t[0]+a[0];
	}
	for(j=2;j<=m-1;j++)
	{
		s[j]=t[j-1];
		s[j-1]=-alpha*t[j-1]+t[j-2];
		if(j>=3)		
			for(k=j-2;k>=1;k--) s[k]=-alpha*t[k]+t[k-1]-beta*b[k];
		s[0]=-alpha*t[0]-beta*b[0];
		d2=0.0;q=0.0;g=0.0;
		for(i=0;i<=n-1;i++)
		{
			w=s[j];
			for(k=j-1;k>=0;k--) w=w*x[i]+s[k];
			d2=d2+w*w;q=q+y[i]*w;
			g=g+x[i]*w*w;
		}
		q=q/d2;
		alpha=g/d2;
		beta=d2/d1;
		a[j]=q*s[j];t[j]=s[j];
		for(k=j-1;k>=0;k--)
		{
			a[k]=q*s[k]+a[k];
			b[k]=t[k];t[k]=s[k];
		}

	}
	dt[0]=0.0;
	dt[1]=0.0;
	dt[2]=0.0;
	for(i=0;i<=n-1;i++)
	{
		w=a[m-1];
		for(k=m-2;k>=0;k--) w=a[k]+w*x[i];
		p=w-y[i];
		if(fabs(p)>dt[2]) dt[2]=fabs(p);
		dt[0]=dt[0]+p*p;
		dt[1]=dt[1]+fabs(p);
	}
	return;
}
*/
void gPolyfit(int m,vector<double> x,vector <double> y,double *a,double dt[])
{
	int i,j,k;
	double alpha,p,q,g,w,beta,d1,d2,s[20],t[20],b[20];
	int n=x.size();
	for(i=0;i<=m-1;i++) a[i]=0.0;
	if(m>n) m=n;
	if(m>20) m=20;
	b[0]=1.0;d1=n;alpha=0.0;q=0.0;
	for(i=0;i<=n-1;i++) {alpha=alpha+x[i];q=q+y[i];}
	q=q/d1;
	alpha=alpha/d1;
	a[0]=q*b[0];
	if(m>1)
	{
		t[1]=1.0;t[0]=-alpha;
		d2=0.0;q=0.0;g=0.0;
		for(i=0;i<=n-1;i++)
		{
			w=x[i]-alpha;
			d2=d2+w*w;
			q=q+y[i]*w;
			g=g+x[i]*w*w;
		}
		q=q/d2;
		alpha=g/d2;
		beta=d2/d1;
		d1=d2;
		a[1]=q*t[1];
		a[0]=q*t[0]+a[0];
	}
	for(j=2;j<=m-1;j++)
	{
		s[j]=t[j-1];
		s[j-1]=-alpha*t[j-1]+t[j-2];
		if(j>=3)		
			for(k=j-2;k>=1;k--) s[k]=-alpha*t[k]+t[k-1]-beta*b[k];
		s[0]=-alpha*t[0]-beta*b[0];
		d2=0.0;q=0.0;g=0.0;
		for(i=0;i<=n-1;i++)
		{
			w=s[j];
			for(k=j-1;k>=0;k--) w=w*x[i]+s[k];
			d2=d2+w*w;q=q+y[i]*w;
			g=g+x[i]*w*w;
		}
		q=q/d2;
		alpha=g/d2;
		beta=d2/d1;
		a[j]=q*s[j];t[j]=s[j];
		for(k=j-1;k>=0;k--)
		{
			a[k]=q*s[k]+a[k];
			b[k]=t[k];t[k]=s[k];
		}

	}
	dt[0]=0.0;
	dt[1]=0.0;
	dt[2]=0.0;
	for(i=0;i<=n-1;i++)
	{
		w=a[m-1];
		for(k=m-2;k>=0;k--) w=a[k]+w*x[i];
		p=w-y[i];
		if(fabs(p)>dt[2]) dt[2]=fabs(p);
		dt[0]=dt[0]+p*p;
		dt[1]=dt[1]+fabs(p);
	}
	return;
}

//求方位角
int gAngle(float sx,float sy,float rx,float ry)//add by zhaohu
{
	//求方位角
	float temp1=(float)(ry-sy);
	float temp2=(float)(rx-sx);
	double temp3;
	if(temp2==0)
		temp3 = tan(PI/2);
	else
		temp3=temp1/temp2;

	short temp=atanf(temp3)*RADIAN_TO_ANGLE;//方位角角度	

	short tarray=0;
	if(rx<sx)
	{
		if(ry<sy)	
		{tarray=270-abs(temp);}
		else 
		{tarray=270+abs(temp);}
	}
	else
	{
		if(ry<sy)	
		{tarray=90+abs(temp);}
		else 
		{tarray=90-abs(temp);}//与cmp线夹角，顺时针方向
	}
	//求方位角
	return tarray;
}
//求两个正整数的最大公约数
int gcd(int m,int n)
{
	int r=m%n;
	while(r!=0)   //不用比较大小，若m小于n，则会在第一遍循环交换位置 
	{
		m=n;
		n=r;
		r=m%n;
	}
	return n;
}