/***************************************************
 * Name    : spsloadcal.cpp
 * Function: 根据导入的sps文件，分别计算检波点和炮点的不相干参数μ值*
 * Author: 李汶骏(lwj)*
/***************************************************/

#include "spsloadcal.h"
int ArrayLineNum(QVector<QVector<float> > &Phi,string calctype)//寻找累加的最大间隔数值
{	
	if (calctype == "R")
	{
		int row=Phi.size();
		int maxcol=Phi.at(0).size();
		float Min=SearchMin(Phi);//寻找坐标数组当中最小值，作为原点
		// 数组初始化
		float **dx1 = (float**)malloc(sizeof(float*) * row); 
		float **dx2 = (float**)malloc(sizeof(float*) * row); 
		float **dx3 = (float**)malloc(sizeof(float*) * row); 
		for (int i = 0; i < row; i++) 
		{
			dx1[i] = (float*)malloc(sizeof(float) * maxcol);
			dx2[i] = (float*)malloc(sizeof(float) * maxcol);
			dx3[i] = (float*)malloc(sizeof(float) * maxcol);
		}
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < maxcol; j++)
			{
				dx1[i][j] = 0;
				dx2[i][j] = 0;
				dx3[i][j] = 0;
			}
		}
		for(int i=0;i<row;i++)
		{
			int j0=1;
			dx1[i][0]=Phi[i][0]-Min;
			for (int j=0;j<Phi[i].size()-1;j++)
			{			
				dx1[i][j0]=Phi[i][j+1]-Phi[i][j];j0=j0+1;	
			}	
		}
		float dx=10;
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				dx2[i][j]=(int)(dx1[i][j]/dx+0.5);			
			}
		}
		//数组累加
		for(int i=0;i<row;i++)
		{
			float sum=0.0;
			for (int j=0;j<Phi[i].size();j++)
			{
				sum=sum+dx2[i][j];
				dx3[i][j]=sum;
			}
		}
		//数组累加
		//查找最大值
		float Max=0.0;
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				float temp=dx3[i][j];
				if (temp>=Max)
				{
					Max=temp+1;
				}
			}
		}
		//释放内存
		for (int i=0;i<Phi.size();i++)
		{
			free(dx1[i]);free(dx2[i]);free(dx3[i]);
		}free(dx1);free(dx2);free(dx3);
		return Max;
	}
//****** 炮点 ******//
	else(calctype == "S");
	{
		// 数组初始化
		int row=Phi.size();
		int col=0;
		for (int i=0;i<row;i++)
		{
			if (Phi[i].size() > col)
			{
				col=Phi[i].size();//最大列
			}
		}
		float Min=Phi[0][1];float Min2=Phi[0][2];float Min3=Phi[1][1];
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				float temp=Phi[i][j];
				if (temp<Min && temp!=0)//寻找除开0的最小值
				{
					Min=temp;
				}
			}
		}
		// 数组初始化
		float **dx1 = (float**)malloc(sizeof(float*) * row); 
		float **dx2 = (float**)malloc(sizeof(float*) * row); 
		float **dx3 = (float**)malloc(sizeof(float*) * row); 
		for (int i = 0; i < row; i++) 
		{
			dx1[i] = (float*)malloc(sizeof(float) * col);
			dx2[i] = (float*)malloc(sizeof(float) * col);
			dx3[i] = (float*)malloc(sizeof(float) * col);
		}
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				dx1[i][j] = 0;
				dx2[i][j] = 0;
				dx3[i][j] = 0;
			}
		}
		for(int i=0;i<row;i++)
		{
			int j0=1;
			dx1[i][0]=Phi[i][0]-Min;
			for (int j=0;j<Phi[i].size()-1;j++)
			{			
				dx1[i][j0]=Phi[i][j+1]-Phi[i][j];j0=j0+1;	
			}	
		}
		float dx=10;
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				dx2[i][j]=(int)(dx1[i][j]/dx+0.5);			
			}
		}
		//数组累加
		for(int i=0;i<row;i++)
		{
			float sum=0.0;
			for (int j=0;j<col;j++)
			{
				sum=sum+dx2[i][j];
				dx3[i][j]=sum;
			}
		}
		//数组累加
		//查找最大值
		float Max=0.0;
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				float temp=dx3[i][j];
				if (temp>=Max)
				{
					Max=temp+1;
				}
			}
		}
		//释放内存
		for (int i=0;i<row;i++)
		{
			free(dx1[i]);free(dx2[i]);free(dx3[i]);
		}free(dx1);free(dx2);free(dx3);
		return Max;
	}
}
//可以用数组计算

int **SpsArrayTrans(QVector<QVector<float> > &Phi,string calctype)//寻找实际坐标之间的间隔，并转化为0-1矩阵
{
	//****** 检波点 ******//
	if (calctype == "R")
	{
		// 数组初始化
		int row=Phi.size();
		int maxcol=0;
		for (int i=0;i<row;i++)
		{
			if (Phi[i].size() > maxcol)
			{
				maxcol=Phi[i].size();//最大列
			}
		}
		float Min=SearchMin(Phi);//寻找坐标数组当中最小值，作为原点
		// 数组初始化
		float **dx1 = (float**)malloc(sizeof(float*) * row); 
		float **dx2 = (float**)malloc(sizeof(float*) * row); 
		float **dx3 = (float**)malloc(sizeof(float*) * row); 
		for (int i = 0; i < row; i++) 
		{
			dx1[i] = (float*)malloc(sizeof(float) * maxcol);
			dx2[i] = (float*)malloc(sizeof(float) * maxcol);
			dx3[i] = (float*)malloc(sizeof(float) * maxcol);
		}
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < maxcol; j++)
			{
				dx1[i][j] = 0;
				dx2[i][j] = 0;
				dx3[i][j] = 0;
			}
		}
		for(int i=0;i<row;i++)
		{
			int j0=1;
			dx1[i][0]=Phi[i][0]-Min;
			for (int j=0;j<Phi[i].size()-1;j++)
			{			
				dx1[i][j0]=Phi[i][j+1]-Phi[i][j];j0=j0+1;	
			}	
		}
		float dx=10;
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				dx2[i][j]=(int)(dx1[i][j]/dx+0.5);			
			}
		}
		//数组累加
		for(int i=0;i<row;i++)
		{
			float sum=0.0;
			for (int j=0;j<Phi[i].size();j++)
			{
				sum=sum+dx2[i][j];
				dx3[i][j]=sum;
			}
		}//数组累加
		//查找最大值
		float MaxInterval=0.0;
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				float temp=dx3[i][j];
				if (temp>=MaxInterval)
				{
					MaxInterval=temp+1;
				}
			}
		}
		//初始化0-1矩阵
		int **array= (int**)malloc(sizeof(int*) * row); 
		for (int i = 0; i < row; i++) 
		{
			array[i] = (int*)malloc(sizeof(int) * MaxInterval);
		}
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<MaxInterval;j++)
			{
				array[i][j] = 0;
			}
		}
		int k=0;
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<maxcol;j++)
			{
				if (dx3[i][j+1]-dx3[i][j]>0)
				{
					k=(int)dx3[i][j];	
					array[i][k]=1;//将dx4值对应下标赋值为1
				}
			}
		}
		//释放内存
		for (int i=0;i<Phi.size();i++)
		{
			free(dx1[i]);free(dx2[i]);free(dx3[i]);
		}free(dx1);free(dx2);free(dx3);
		return array;
	}
	//****** 炮点 ******//
	else(calctype == "S");
	{
		// 数组初始化
		int row=Phi.size();
		int col=0;
		for (int i=0;i<row;i++)
		{
			if (Phi[i].size() > col)
			{
				col=Phi[i].size();//最大列
			}
		}
		float Min=Phi[0][1];float Min2=Phi[0][2];float Min3=Phi[1][1];
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size();j++)
			{
				float temp=Phi[i][j];
				if (temp<Min && temp!=0)//寻找除开0的最小值
				{
					Min=temp;
				}
			}
		}
		// 数组初始化
		float **dx1 = (float**)malloc(sizeof(float*) * row); 
		float **dx2 = (float**)malloc(sizeof(float*) * row); 
		float **dx3 = (float**)malloc(sizeof(float*) * row); 
		for (int i = 0; i < row; i++) 
		{
			dx1[i] = (float*)malloc(sizeof(float) * col);
			dx2[i] = (float*)malloc(sizeof(float) * col);
			dx3[i] = (float*)malloc(sizeof(float) * col);
		}
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				dx1[i][j] = 0;
				dx2[i][j] = 0;
				dx3[i][j] = 0;
			}
		}
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<Phi[i].size()-1;j++)
			{			
				dx1[i][j]=-(Phi[i][j+1]-Phi[i][j]);
			}	
		}
		float a1=dx1[0][1];float a2=dx1[0][2];float a3=dx1[0][3];
		float dx=10;
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				dx2[i][j]=(int)(dx1[i][j]/dx+0.5);			
			}
		}
		//数组累加
		for(int i=0;i<row;i++)
		{
			float sum=0.0;
			for (int j=0;j<col;j++)
			{
				sum=sum+dx2[i][j];
				dx3[i][j]=sum;
			}
		}
		float a=dx3[0][1];float b=dx3[0][2];float c=dx3[1][2];float d=dx3[1][3];
		//数组累加
		//查找最大值
		float MaxInterval=0.0;
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				float temp=dx3[i][j];
				if (temp>=MaxInterval)
				{
					MaxInterval=temp+1;
				}
			}
		}
		//初始化0-1矩阵
		int **array= (int**)malloc(sizeof(int*) * row); 
		for (int i = 0; i < row; i++) 
		{
			array[i] = (int*)malloc(sizeof(int) * MaxInterval);
		}
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<MaxInterval;j++)
			{
				array[i][j] = 0;
			}
		}
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<MaxInterval;j++)
			{

					int k=(int)dx3[i][j];
					if (k>0)
					{
						array[i][k]=1;//将dx3值对应下标赋值为1	
					}
					else continue;
			}
		}
		//释放内存
		for (int i=0;i<row;i++)
		{
			free(dx1[i]);free(dx2[i]);free(dx3[i]);
		}free(dx1);free(dx2);free(dx3);
		return array;
	}
}

float SearchMin(QVector<QVector<float> > &Phi)//寻找数组中最小值
{
	int row=Phi.size();
	int col=Phi.at(0).size();
	float Min=Phi[0][1];
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<Phi[i].size();j++)
		{
			float temp=Phi[i][j];
			if (temp<Min && temp!=0)//寻找除开0的最小值
			{
				Min=temp;
			}
		}
	}
	return Min;
}
float SearchMax(QVector<QVector<float> > &Phi)//寻找数组中最大值
{
	int row=Phi.size();
	int col=Phi.at(0).size();
	float Max=0.0;
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<Phi[i].size();j++)
		{
			float temp=Phi[i][j];
			if (temp>=Max)
			{
				Max=temp;
			}
		}
	}
	return Max;
}
