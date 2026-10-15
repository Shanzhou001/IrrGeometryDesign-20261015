/***************************************************
 * Name    : Diagmul.cpp
 * Function: 一维矩阵转化为对角矩阵，实现和一个二维矩阵相乘
 *			 对一个二维矩阵行间采样/对线间进行采样           
/***************************************************/
#include "DiagMulmatrix.h"
#include <stdlib.h>
#include <string.h>

//****** 对二维矩阵的行进行采样,数据类型为int ******
// 修复 2026.10.15 // add by shanzhou in 2026/10/15：原实现按"对角矩阵×二维矩阵"的矩阵乘法计算，
// 复杂度 O(m^2*col)，被标记弃用但仍是主流程的现行调用。
// 对角矩阵乘法在数学上等价于"按行掩码选择"，此处直接选择，
// 输出与原实现逐位一致（phi[i]==1 保留第 i 行，否则置零），
// 分配/释放约定不变（malloc 分配，调用方 free）。
int **MulMatrix(int m,int *phi,int col,int **Phi) // add by shanzhou in 2026/10/15 //对Phi的行进行采样
{
	int **c = (int**)malloc(sizeof(int*) * m);
	for (int i = 0; i < m; i++)
	{
		c[i] = (int*)malloc(sizeof(int) * col);
		if (phi[i] == 1)
		{
			memcpy(c[i], Phi[i], sizeof(int) * col);
		}
		else
		{
			memset(c[i], 0, sizeof(int) * col);
		}
	}
	return c;
}

//****** 对一维矩阵进行采样,数据类型为int,采用寻找下标的方式 ******//
int *MulMatrix_1D(int N,int *phi,int *Phi)
{
	vector<int> idx;
	// 得到采样矩阵0点的下标
	for (int i = 0; i < N; i++)
	{
		if (phi[i] == 0)
		{
			idx.push_back(i);
		}
	}
	int m = (int)idx.size();

	for (int j=0;j<m;j++)
	{
		Phi[idx[j]]=0;//完成采样
	}

	int *d = (int*)malloc(sizeof(int) * N-m);//申请动态空间
	for (int i=0;i<N-m;i++)//初始化-可以省略
	{
		d[i]=0;
	}
	int t=0;
	for (int i=0;i<N;i++)
	{
		if (Phi[i]!=0)
		{
			d[t]=Phi[i];t++;//去掉等于0的数据，得到采样后的实际数据
		}
	}
	//释放内存
	return d;
}

//****** 对一维矩阵进行采样,数据类型为float,采用寻找下标的方式 ******//
 float *MulMatrix_1D_float(int N, int *phi, float *Phi)
 {
	 vector<int> idx;
	 // 得到采样矩阵0点的下标
	 for (int i = 0; i < N; i++)
	 {
		 if (phi[i] == 1)
		 {
			 idx.push_back(i);
		 }
	 }
	 int m = (int)idx.size();
	 float *d = (float*)malloc(sizeof(float) * m);//申请动态空间
	 int t=0;
	 for (int i=0;i<N;i++)
	 {
		 if (phi[i] == 1)
		 {
			 d[t]=Phi[i]; t++;//去掉等于0的数据，得到采样后的实际数据
		 }
	 }
	 //释放内存
	 return d;
 }

//****** 规则观测系统---检波点横坐标 ******//
float *RegularMR(int row,int col, float d, float X0)//d为道间距
{
	int N = row*col;
	float **x = (float**)malloc(sizeof(float*)*row);
	for (int i=0;i<row;i++)
	{
		x[i] = (float*)malloc(sizeof(float)*col);
	}
	
	for (int k=0;k<row;k++)
	{
		for (int j=0;j<col;j++)
		{
			x[k][j]=j*d+X0;//实际坐标是从X0开始
		}
	}
	float *y=Convert1D_float(row,col,x);//二维转化为一维
	
	for (int i=0;i<row;i++)
	{
		free(x[i]);
	}
	free(x);
	return y;
}

//****** 规则观测系统---检波点纵坐标 ******//
float *RegularNR(int row,int col, float d, float Y0)//d--线距,row--测线数,col--每条线上点数
{
	int N = row*col;
	float **x = (float**)malloc(sizeof(float*)*row);
	for (int i=0;i<row;i++)
	{
		x[i] = (float*)malloc(sizeof(float)*col);
	}
	for (int k=0;k<row;k++)
	{
		for (int j=0;j<col;j++)
		{
			x[k][j]=k*d+Y0;//实际纵坐标是从Y0开始
		}
	}
	float *y=Convert1D_float(row,col,x);//二维转化为一维

	for (int i=0;i<row;i++)
	{
		free(x[i]);
	}
	free(x);
	return y;
}
//****** 规则观测系统---炮点横坐标 ******//
float *RegularMS(int row,int col, float d, float SX0)//d为道间距
{
	int N = row*col;
	float **x = (float**)malloc(sizeof(float*)*row);
	for (int i=0;i<row;i++)
	{
		x[i] = (float*)malloc(sizeof(float)*col);
	}
	for (int k=0;k<row;k++)
	{
		for (int j=0;j<col;j++)
		{
			x[k][j]=k*d+SX0;//实际坐标是从SX0开始
		}
	}
	float *y=Convert1D_float(row,col,x);//二维转化为一维

	for (int i=0;i<row;i++)
	{
		free(x[i]);
	}
	free(x);
	return y;
}

// ******规则观测系统---炮点纵坐标 ******//
float *RegularNS(int row,int col, float d, float SY0)//d--线距,row--测线数,col--每条线上点数
{
	int N = row*col;
	float **x = (float**)malloc(sizeof(float*)*row);
	for (int i=0;i<row;i++)
	{
		x[i] = (float*)malloc(sizeof(float)*col);
	}
	for (int k=0;k<row;k++)
	{
		for (int j=0;j<col;j++)
		{
			x[k][j]=j*d+SY0;//实际纵坐标是从SY0开始
		}
	}
	float *y=Convert1D_float(row,col,x);//二维转化为一维

	for (int i=0;i<row;i++)
	{
		free(x[i]);
	}
	free(x);
	return y;
}

//****** 二维数组转化为一维,数据类型为int,按行读取排列 ******//
int *Convert1D(int row,int col,int **phi)
{
	int N=row*col;
	int *y = (int*)malloc(sizeof(int)*N);
	int t=0;
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			y[t] = phi[i][j];t++;//将二维转为一维数组
		}
	}
	return y;
}

//****** 二维数组转化为一维,数据类型为float,按行读取排列 ******//
float *Convert1D_float(int row,int col,float **phi)
{
	int N=row*col;
	float *y = (float*)malloc(sizeof(float)*N);
	int t=0;
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			y[t] = phi[i][j];t++;//将二维转为一维数组
		}
	}
	return y;
}