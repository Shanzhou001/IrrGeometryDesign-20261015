/*****************************************************
 * CALCULATEMIU.H
 *   Calculate Mutual Interference Parameter μ
 *   Based on Fourier transform (fftw_2.1.5)
 *   Based on Curvelet transform 
 * Created on Nov 10th, 2021
 *****************************************************/

#ifndef _CALCULATEMIU_H_
#define _CALCULATEMIU_H_

#include <complex>
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <time.h>
#include "fftw.h"

#include "fdct_wrapping.hpp"
#include "fdct_wrapping_inline.hpp"

#include <Eigen/Core>
using namespace Eigen;

using namespace std;
using namespace fdct_wrapping_ns;

//------ 复数二维数组的一维傅里叶变换(零频点在中心) Based on FFTW_2.1.5 -------//
void FFT1D_fftshift(int **inMat, CpxNumMat& bMat, int Nx, int Nt);

//float FourierMiu1D(vector<int> idx, int N); 
///*-----------------------------------------------------
// *	Function: 基于傅里叶变换（FFTW_2.1.5）计算μ值
// *	Input:
// *		idx -- 采样点位置的下标 
// *		N   -- 完整数据的长度
// *	Output:
// *		Miu -- float类型, μ值
// *	Example: miu = FourierMiu1D(idx, N);
// *-----------------------------------------------------*/
//float TestFourierMiu1D(int N, int *phi);
///*-----------------------------------------------------
// *	Function: 基于傅里叶变换（FFTW_2.1.5）计算μ值
// *	Input:
// *		phi -- 采样矩阵 
// *		N   -- 完整数据的长度
// *	Output:
// *		Miu -- float类型, μ值
// *	Example: miu = FourierMiu1D(N, phi);
// *-----------------------------------------------------*/

//float FourierAveMiu2D(int row, int col, int **Phi);
///*------------------------------------------------------
// *	Function: 基于傅里叶变换（FFTW_2.1.5）计算二维随机采样矩阵的μ值
// *	Input:
// *		row -- 采样矩阵行数
// *		col -- 采样矩阵列数
// *		Phi -- 采样矩阵
// *	Output:
// *		Miu -- float类型, μ值
// *	Example: miu = FourierMiu2D(row, col, Phi);
// *-------------------------------------------------------*/

float CurveletMiu1DComplex(vector<int> idx, int N); 
/*-------------------------------------------------------
 * Function: 基于曲波变换计算μ值, 复数
 * 输入输出同FourierMiu1D
 *------------------------------------------------------*/

float CurveletMiu1DReal(vector<int> idx, int N); 
/*-------------------------------------------------------
 * Function: 基于曲波变换计算μ值, 仅实数部分
 *------------------------------------------------------*/

float CurveletMiu1DEigen(vector<int> idx, int N, int datatype);
/*-------------------------------------------------------
 * Function: 基于曲波变换计算μ值, 采用Eigen开源库
 * Input   : datatype = 0为复数, datatype = 1为实数
 *           曲波参数固定为 (nbscales=1, nbangles_coarse=4)，与历史实现一致
 *------------------------------------------------------*/

float CurveletMiu1DEigenParam(vector<int> idx, int N, int datatype,
							  int nbscales, int nbangles_coarse);
/*-------------------------------------------------------
 * Function: 参数化μ计算：曲波 (nbscales, nbangles_coarse) 可配置
 * Purpose : μ 定义的变换参数鲁棒性实验（命令行实验框架使用）
 * Note    : nbscales=1, nbangles=4 时与 CurveletMiu1DEigen 完全一致
 *------------------------------------------------------*/

float TestCurveletMiu1D(int row, int *Phi, string calctype);
/*-------------------------------------------------------
 * Function: 基于曲波变换和一维采样矩阵计算μ值, 采用Eigen开源库
 * Input   : datatype = "EigenCpx"为复数, datatype = "EigenReal"为实数
 *		     row为一维矩阵总长，Phi为一维采样矩阵
 *------------------------------------------------------*/

float CurveletAveMiu2D(int row, int col, int **Phi, string calctype);
/*-------------------------------------------------------
 * Function: 基于曲波变换计算二维采样矩阵的平均μ值 
 * Input   : 输入输出同FourierMiu2D
 *           calctype -- "CCpx", "CReal", "EigenCpx", "EigenReal"四种类型
 *------------------------------------------------------*/

float CalculateAveMiu(int row,int N,int **Phi1);//实数计算曲波变换平均μ值

//------ μ 评估计数器（实验效率统计用）------//
void miu_eval_reset();//计数清零
long miu_eval_count();//当前累计μ评估次数（包含GUI与命令行全部调用）

#endif // CALCULATEMIU_H