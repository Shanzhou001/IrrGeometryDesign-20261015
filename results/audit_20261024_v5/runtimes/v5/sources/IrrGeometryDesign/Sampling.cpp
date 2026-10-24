/***************************************************
 * Name    : Sampling.cpp
 * Function: 根据采样点和采样率形成三种一维和二维的采样方式；
 *			 根据道间距/炮距将0-1矩阵转化为实际坐标
 * 修复日期: 2026.10.15 // add by wushanzhou in 2026/10/15
 * 修复内容(影响实验有效性的问题):
 *   1) 移除各采样函数内部的 srand(time(NULL)) —— 该写法使模拟退火
 *      高频循环内一秒内多次调用时生成完全相同的候选解，随机源统一
 *      改为 ExpRng（可显式播种、可复现，见 ExpRng.h）；
 *   2) Jitter 采样（Sampling1J/2J）增加单元唯一性保证：原实现随机
 *      偏移可能命中同一格点，导致实际采样率偏离目标值；现按
 *      Hennenfent & Herrmann (2008) 的抖动网格方式，每个网格单元
 *      恰好一个点（均匀抖动、不重叠、不越界），采样点数精确等于
 *      int(col*c)（c<=0.5）或 col-int(col*c)（c>0.5，丢弃点数）；
 *   3) 分段随机采样（Sampling1F/2F）的剩余样点原来随机落到分段号
 *      1..M，两个剩余点可能落在同一段且只展开一个位置，导致实际
 *      采样点数少于目标；现保证 M 段每段恰有一点，剩余 L 点随机
 *      分给 L 个不同段，总点数精确等于 M。
/***************************************************/

#include "sample.h"
#include "ExpRng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

using namespace std;

/***************************************************
 * 内部工具：抖动网格取位
 * 在 [0,col) 内取 m 个互不相同的格点，将网格按采样因子
 * alpha=col/m 划分为 m 个单元，每个单元内均匀随机取一点。
 * 保证：点数恰为 m；无重复；不越界。
 ***************************************************/
static void jittered_positions(int col, int m, vector<int>& pos) // add by wushanzhou in 2026/10/15
{
	pos.clear();
	if (col <= 0 || m <= 0) return;
	if (m >= col) // 全采样
	{
		for (int i = 0; i < col; i++) pos.push_back(i);
		return;
	}
	double alpha = (double)col / (double)m;
	vector<char> used(col, 0);
	for (int i = 1; i <= m; i++)
	{
		// 第 i 个单元覆盖实数区间 [(i-1)*alpha, i*alpha)
		int lo = (int)ceil((i - 1) * alpha); // add by wushanzhou in 2026/10/23
		int hi = (int)ceil((double)i * alpha) - 1;
		if (lo < 0) lo = 0;
		if (hi > col - 1) hi = col - 1;
		int j;
		if (hi < lo) // 空单元（防御性处理：向右找第一个空位）
		{
			j = lo;
			while (j < col && used[j]) j++;
			if (j >= col)
			{
				j = lo;
				while (j >= 0 && used[j]) j--;
			}
		}
		else
		{
			int width = hi - lo + 1;
			j = lo + (int)(exprng::uniform() * width);
			if (j > hi) j = hi;
			if (j < lo) j = lo;
			// alpha 非整数时相邻单元的取整边界可能重叠一个格点，
			// 命中已用点时从 j 向右/左找最近空位
			if (used[j])
			{
				int k;
				for (k = j + 1; k <= hi && used[k]; k++) {}
				if (k <= hi) j = k;
				else
				{
					for (k = j - 1; k >= lo && used[k]; k--) {}
					j = (k >= lo) ? k : lo;
				}
			}
		}
		if (!used[j])
		{
			used[j] = 1;
			pos.push_back(j);
		}
	}
}

/**************方法二随机采样矩阵************/
//****** 一维随机采样矩阵 ******//
int *Sampling1S(int N,float c)//2023.4.16 李汶骏添加
{
	int num_one = (int)(N*c);
	int num_zero = N-num_one;
	int *arr = new int[N];
	for (int p=0;p<N;p++)
	{
		arr[p] = 0;
	}
	//使用二项分布生成随机数
	for(int i=0;i<N;i++)
	{
		if (i<num_zero)
		{
			arr[i] = 0;
		}
		else
		{
			arr[i] = 1;
		}
	}
	for (int i=N-1;i>=1;i--)
	{
		int j=exprng::integer()%(i+1);
		int temp=arr[j];
		arr[j]=arr[i];
		arr[i]=temp;
	}
	return arr;
}

//****** 二维随机采样矩阵 ******//
int **Sampling2S(int row,int col,float c)
{
	int num_one = (int)(col*c);
	int num_zero = col-num_one;
	int **phi = new int *[row];
	for (int i=0;i<row;i++)
	{
		phi[i] = new int[col];
	}
	//数组初始化
	for(int i=0;i < row;i++)
	{
		for (int j=0;j<col;j++)
		{
			phi[i][j] = 0;
		}
	}
	//使用二项分布生成随机数
	for (int i = 0;i < row; i++)
	{
		for (int j=0;j<col;j++)
		{

			if (j<num_zero)
			{
				phi[i][j] = 0;
			}
			else phi[i][j] = 1;
		}
	}
	for (int q=0;q<row;q++)
	{
		for (int i=col-1;i>=1;i--)
		{
			int j=exprng::integer()%(i+1);
			int temp=phi[q][j];
			phi[q][j]=phi[q][i];
			phi[q][i]=temp;
		}
	}
	return phi;
}

// *******************一维jitter采样********************//
int *Sampling1J(int col,float c)//n个采样点,c-采样率
{
	int *Phi = new int [col];
	if (c > 0.5)
	{
		int m = col-int(col*c);//需丢弃的点数（保持采样点数 = col-m = int(col*c)）
		for (int i=0;i<col;i++)
		{
			Phi[i] = 1;
		}
		if (m > 0)
		{
			vector<int> pos;
			jittered_positions(col, m, pos);//每个单元恰一点，精确丢弃 m 个
			for (size_t k=0;k<pos.size();k++)
			{
				Phi[pos[k]] = 0;
			}
		}
	}
	else
	{
		for (int i=0;i<col;i++)
		{
			Phi[i] = 0;
		}
		int m = int(col*c);//采样点数
		if (m > 0)
		{
			vector<int> pos;
			jittered_positions(col, m, pos);//每个单元恰一点，精确放置 m 个
			for (size_t k=0;k<pos.size();k++)
			{
				Phi[pos[k]] = 1;
			}
		}
	}
	return Phi;
}

// ****************二维Jitter采样 ****************//
int **Sampling2J(int row,int col,float c)//n个采样点,c-采样率
{
	int **Phi = new int *[row];
	for (int i=0;i<row;i++)
	{
		Phi[i] = new int[col];
	}
	if (c > 0.5)
	{
		int m = col-int(col*c);//需丢弃的点数
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				Phi[i][j] = 1;
			}
		}
		if (m > 0)
		{
			for (int k=0;k<row;k++)
			{
				vector<int> pos;
				jittered_positions(col, m, pos);//每行独立抖动
				for (size_t t=0;t<pos.size();t++)
				{
					Phi[k][pos[t]] = 0;
				}
			}
		}
	}
	else
	{
		int m = int(col*c);//采样点数
		for (int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				Phi[i][j] = 0;
			}
		}
		if (m > 0)
		{
			for (int k=0;k<row;k++)
			{
				vector<int> pos;
				jittered_positions(col, m, pos);//每行独立抖动
				for (size_t t=0;t<pos.size();t++)
				{
					Phi[k][pos[t]] = 1;
				}
			}
		}
	}
	return Phi;
}

// ************一维分段随机采样*************//
int *Sampling1F(int col,float c)
{
	int M = (int)(col*c);//分为M段（M 个采样点）
	int *Phi = new int [col];
	for (int i=0;i<col;i++)
	{
		Phi[i] = 0;
	}
	if (M <= 0) return Phi;  // c 过小，防御（原实现此处除零）
	if (M >= col)            // 全采样
	{
		for (int i=0;i<col;i++) Phi[i] = 1;
		return Phi;
	}
	int S = col/M;    //每一段的基础点数
	int L = col-S*M;  //剩余样点数（分给 L 个不同段，每段加宽 1）

	// 随机选出 L 个段（段号 0..M-1）加宽，等价于原来的
	// "剩余点随机放在 M 段中"，但保证每段至多加宽一次
	vector<int> seg(M);
	for (int i=0;i<M;i++) seg[i] = i;
	for (int i=M-1;i>=1;i--) //Fisher-Yates 洗牌
	{
		int j = (int)(exprng::integer()%(i+1));
		int tmp = seg[j]; seg[j] = seg[i]; seg[i] = tmp;
	}
	vector<char> extra(M, 0);
	for (int i=0;i<L;i++) extra[seg[i]] = 1;

	// 每段恰取一点，位置在段内均匀随机：保证总点数精确为 M 且无重复
	int start = 0;
	for (int t=0;t<M;t++)
	{
		int width = S + (extra[t] ? 1 : 0);
		int e = (int)(exprng::integer()%width); // [0,width)
		Phi[start+e] = 1;
		start += width;
	}
	return Phi;
}

//*************** 二维分段随机采样 ****************//
int **Sampling2F(int row, int col, float c)//二维
{
	int M = (int)(col*c);
	int **Phi = new int *[row];
	for (int i = 0;i < row;i++)
	{
		Phi[i] = new int[col];
	}
	for(int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			Phi[i][j] = 0;
		}
	}//初始化
	if (M <= 0) return Phi;
	if (M >= col)
	{
		for(int i=0;i<row;i++)
		{
			for (int j=0;j<col;j++)
			{
				Phi[i][j] = 1;
			}
		}
		return Phi;
	}
	int S = col/M;
	int L = col-S*M;

	// 每行独立做一维分段随机（与 Sampling1F 相同逻辑）
	for (int k=0;k<row;k++)
	{
		vector<int> seg(M);
		for (int i=0;i<M;i++) seg[i] = i;
		for (int i=M-1;i>=1;i--)
		{
			int j = (int)(exprng::integer()%(i+1));
			int tmp = seg[j]; seg[j] = seg[i]; seg[i] = tmp;
		}
		vector<char> extra(M, 0);
		for (int i=0;i<L;i++) extra[seg[i]] = 1;
		int start = 0;
		for (int t=0;t<M;t++)
		{
			int width = S + (extra[t] ? 1 : 0);
			int e = (int)(exprng::integer()%width);
			Phi[k][start+e] = 1;
			start += width;
		}
	}
	return Phi;
}

//****** 数组排序 ******//
void bubble_sort(int arr[], int len)//将长度为len的数组元素从小到大排序
{
	int i, j, temp;
	for (i = 0; i < len - 1; i++)
	{
		for (j = 0; j < len - 1 - i; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
}

//一维随机矩阵化为坐标
int *X_coor(int col, float c,int d, int *phi)//col-列长，d-间隔
{
	int m = 0;
	for (int j=0;j<col;j++)
	{
		if (phi[j]==1) m++;//按实际矩阵统计点数（修复：原实现按 c 反推，与去重后的矩阵可能不符）
	}
	int *x = new int [m];
	int k=0;
		for(int j=0;j<col;j++)
		{
			if (phi[j]==1)
			{
				x[k]=j*d+1000;k++;
			}
			else
			{
				continue;
			}
		}
	return x;
}

//二维随机矩阵转化为坐标
int **X_coor_2(int row, int col, int d, int **phi)
{

	int **y = new int *[row];
	for (int i=0;i<row;i++)
	{
		y[i] = new int[col];
	}
	for (int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			y[i][j] = 0;
		}
	}
	for (int i=0;i<row;i++)//将0-1矩阵转化为坐标
	{
		for(int j=0;j<col;j++)
		{
			if (phi[i][j]==1)
			{
				y[i][j]=j*d+1000;
			}
			else
			{
				continue;
			}
		}
	}
	return y;
}

int* Sampling1F_Positions(int col, float c) //0-1掩码转采样点索引
{
    int* mask = Sampling1F(col, c);  // 原函数，返回0-1掩码
    int count = 0;

    for (int i = 0; i < col; ++i)
        if (mask[i] == 1) count++;

    int* phi = new int[count];
    int idx = 0;
    for (int i = 0; i < col; ++i)
    {
        if (mask[i] == 1)
            phi[idx++] = i;
    }

    delete[] mask;  // 释放原始掩码内存
    return phi;     // 返回采样点索引数组
}

// =============================================================================
// 精确采样点数与合法性校验新实现 (Task 1 & 2) // add by wushanzhou in 2026/10/15
// =============================================================================

bool ValidateSamplingMask(int N, int m, const int *mask)
{
	if (!mask || N <= 0 || m < 0 || m > N) return false;
	int count = 0;
	for (int i = 0; i < N; i++)
	{
		if (mask[i] != 0 && mask[i] != 1) return false;
		if (mask[i] == 1) count++;
	}
	return (count == m);
}

int *Sampling1R_Count(int N, int m)
{
	if (N <= 0) return NULL;
	int *Phi = new int[N];
	for (int i = 0; i < N; i++) Phi[i] = 0;
	if (m <= 0) return Phi;
	if (m >= N)
	{
		for (int i = 0; i < N; i++) Phi[i] = 1;
		return Phi;
	}

	// 规则减抽采用均匀覆盖全空间的等间距步长: i_k = int(k * N / m)
	// 在 50% 下严格等于纯偶数抽取 (0, 2, ..., 126)，在 30% 和 70% 下均匀分布全区间且严格等于 m 点
	for (int k = 0; k < m; k++)
	{
		int idx = (int)((double)k * (double)N / (double)m);
		if (idx < 0) idx = 0;
		if (idx >= N) idx = N - 1;
		Phi[idx] = 1;
	}
	int current_cnt = 0;
	for (int i = 0; i < N; i++) if (Phi[i] == 1) current_cnt++;
	for (int i = 0; i < N && current_cnt < m; i++)
	{
		if (Phi[i] == 0) { Phi[i] = 1; current_cnt++; }
	}
	for (int i = N - 1; i >= 0 && current_cnt > m; i--)
	{
		if (Phi[i] == 1) { Phi[i] = 0; current_cnt--; }
	}
	return Phi;
}

int *Sampling1S_Count(int N, int m)
{
	if (N <= 0) return NULL;
	int *arr = new int[N];
	for (int i = 0; i < N; i++) arr[i] = 0;
	if (m <= 0) return arr;
	if (m >= N)
	{
		for (int i = 0; i < N; i++) arr[i] = 1;
		return arr;
	}
	int num_zero = N - m;
	for (int i = 0; i < N; i++)
	{
		arr[i] = (i < num_zero) ? 0 : 1;
	}
	for (int i = N - 1; i >= 1; i--)
	{
		int j = (int)(exprng::integer() % (i + 1));
		int temp = arr[j];
		arr[j] = arr[i];
		arr[i] = temp;
	}
	return arr;
}

int *Sampling1J_Count(int col, int m)
{
	if (col <= 0) return NULL;
	int *Phi = new int[col];
	if (m <= 0)
	{
		for (int i = 0; i < col; i++) Phi[i] = 0;
		return Phi;
	}
	if (m >= col)
	{
		for (int i = 0; i < col; i++) Phi[i] = 1;
		return Phi;
	}

	for (int i = 0; i < col; i++) Phi[i] = 0; // add by wushanzhou in 2026/10/23
	vector<int> pos; // add by wushanzhou in 2026/10/23
	jittered_positions(col, m, pos); // add by wushanzhou in 2026/10/23
	for (size_t k = 0; k < pos.size(); k++) Phi[pos[k]] = 1; // add by wushanzhou in 2026/10/23

	int count = 0;
	for (int i = 0; i < col; i++) if (Phi[i] == 1) count++;
	for (int i = 0; i < col && count < m; i++)
	{
		if (Phi[i] == 0) { Phi[i] = 1; count++; }
	}
	for (int i = col - 1; i >= 0 && count > m; i--)
	{
		if (Phi[i] == 1) { Phi[i] = 0; count--; }
	}
	return Phi;
}

int *Sampling1F_Count(int col, int m)
{
	if (col <= 0) return NULL;
	int *Phi = new int[col];
	for (int i = 0; i < col; i++) Phi[i] = 0;
	if (m <= 0) return Phi;
	if (m >= col)
	{
		for (int i = 0; i < col; i++) Phi[i] = 1;
		return Phi;
	}

	int S = col / m;
	int L = col - S * m;

	vector<int> seg(m);
	for (int i = 0; i < m; i++) seg[i] = i;
	for (int i = m - 1; i >= 1; i--)
	{
		int j = (int)(exprng::integer() % (i + 1));
		int tmp = seg[j]; seg[j] = seg[i]; seg[i] = tmp;
	}
	vector<char> extra(m, 0);
	for (int i = 0; i < L; i++) extra[seg[i]] = 1;

	int start = 0;
	for (int t = 0; t < m; t++)
	{
		int width = S + (extra[t] ? 1 : 0);
		if (width < 1) width = 1;
		int e = (int)(exprng::integer() % width);
		int pos = start + e;
		if (pos >= col) pos = col - 1;
		Phi[pos] = 1;
		start += width;
	}

	int current_cnt = 0;
	for (int i = 0; i < col; i++) if (Phi[i] == 1) current_cnt++;
	for (int i = 0; i < col && current_cnt < m; i++)
	{
		if (Phi[i] == 0) { Phi[i] = 1; current_cnt++; }
	}
	for (int i = col - 1; i >= 0 && current_cnt > m; i--)
	{
		if (Phi[i] == 1) { Phi[i] = 0; current_cnt--; }
	}
	return Phi;
}
