/*****************************************************
 * SAMPLE.H
 * Function: 由采样点和采样率得到几种一维/二维的采样方式
 *			 将0-1矩阵转化为实际坐标
 *			 矩阵相乘，用于对二维矩阵进行线间采样
 * Created on Nov 10th, 2021
 *****************************************************/
#ifndef _SAMPLE_H_
#define _SAMPLE_H_

#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include <vector>

#include "CalculateMiu.h"

using namespace std;

int *Sampling1S(int N,float c);
/*-----------------------------------------------------
 * 方法二：生成一维随机矩阵(实际采样率约等于c，运算时间快)
 *  n -- 总采样点 c -- 采样率
 *-----------------------------------------------------*/

int **Sampling2S(int row,int col,float c);
/*-----------------------------------------------------
 *  方法二：生成二维随机矩阵(采样率约等于c) -- row行col列的二维矩阵
 *  col -- 每条线总采样点 c -- 采样率 row -- 矩阵行数
 *-----------------------------------------------------*/
int *Sampling1J(int col,float c);
/*-----------------------------------------------------
 *  生成一维jitter采样随机矩阵
 *  col -- 总采样点 c -- 采样率
 *-----------------------------------------------------*/

int **Sampling2J(int row,int col,float c);
/*-----------------------------------------------------
 *  生成二维jitter随机矩阵 -- row行col列的二维矩阵
 *  col -- 每条线总采样点 c -- 采样率 row -- 矩阵行数
 *-----------------------------------------------------*/

int *Sampling1F(int col,float c);
/*-----------------------------------------------------
 *  生成一维分段随机矩阵
 *  col -- 总采样点 c -- 采样率
 *-----------------------------------------------------*/

int **Sampling2F(int row,int col,float c);//二维分段随机采样，分成M段
/*-----------------------------------------------------
 *  生成二维分段随机矩阵 -- row行col列的二维矩阵
 *  col -- 每条线总采样点 c -- 采样率 row -- 矩阵行数
 *-----------------------------------------------------*/

// 精确采样点数与区间全覆盖规则抽样接口 // add by wushanzhou in 2026/10/15
int *Sampling1S_Count(int N, int m);
int *Sampling1J_Count(int col, int m);
int *Sampling1F_Count(int col, int m);
int *Sampling1R_Count(int N, int m);
bool ValidateSamplingMask(int N, int m, const int *mask);

int *X_coor(int col, float c,int d, int *phi);
/*-----------------------------------------------------
 *	将一维0-1矩阵化为坐标形式
 *	col-矩阵长度, d-最小道间距,phi-采样矩阵
 *-----------------------------------------------------*/

int **X_coor_2(int row, int col, int d, int **phi);
/*-----------------------------------------------------
 *	将二维0-1矩阵化为实际坐标形式
 *	row -- 行数 col -- 列数 d -- 道间距 phi -- 二维矩阵
 *-----------------------------------------------------*/

void bubble_sort(int arr[], int len);// 冒泡法排序 [lwj 6/5/2023]

//int **MulMatrix(int m,int *phi,int col,int **Phi);
///*-----------------------------------------------------
// * 矩阵相乘，得到线间采样后的二维矩阵
// * phi -- 一维矩阵 m -- 一维矩阵总长/二维矩阵的行数
// * Phi -- 二维矩阵 col -- 二维矩阵的列长
// *-----------------------------------------------------*/
//float CalculateAveMiu(int row,int N,int **Phi1);//计算并返回μ值
int* Sampling1F_Positions(int col, float c);//遗传算法


#endif