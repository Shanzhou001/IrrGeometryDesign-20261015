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

// =============================================================================
// 2026-10-21 新增：经典互干度（Mutual Coherence），修正实现
// -----------------------------------------------------------------------------
// 背景：算法审计（见 README.md §7.2）证明原 CurveletMiu1DEigenParam 度量的
//   是“采样算子 Gram 相对全采样 Gram 的偏离”，实测会奖励聚集、惩罚均匀
//   （极端集中掩码 mu=0.0427 < 均匀抽稀 mu=0.9601），与采样质量方向相反。
//
// 本函数按经典定义实现：
//   设采样位置集合 Ω ⊂ {0..N-1}，|Ω|=m，
//   r(Δ) = Σ_{k∈Ω} exp(-2πi·Δ·k/N)，Δ=0..N-1
//   mu = max_{Δ≠0} |r(Δ)| / r(0)
// 这是采样点集的点扩散函数（PSF）峰值旁瓣，等价于傅里叶字典下的最大互干度。
//
// 性质（均已在 Test 16 中断言）：
//   ① 全采样 (m=N)      -> mu = 0        （酉字典，非对角内积为 0）
//   ② 单点 (m=1)        -> mu = 1        （无法区分任何两个原子）
//   ③ 规则 2 抽 1       -> mu = 1        （Nyquist 假频，最差）
//   ④ 均匀优于聚集      -> mu(jitter) < mu(clustered)
//   ⑤ 值域              -> mu ∈ [0,1]，无需截断
//   ⑥ 计算量            -> O(N·m)，远低于原实现的 FDCT
// =============================================================================
// =============================================================================
// 2026-10-21 新增：真正的二维互干度（任务书问题 6）
// -----------------------------------------------------------------------------
// 背景：原 2D 基准的 μ 是“逐行 1D μ 的均值”，**不包含跨行（跨线）相干性**，
//   不能代表完整二维采样算子的相干性。
//
// 本函数按二维点扩散函数（PSF）峰值旁瓣实现：
//   设二维掩码 M ∈ {0,1}^{Ny×Nx}，|M|=m，
//   r(Δy,Δx) = Σ_{(y,x)∈Ω} exp(-2πi(Δy·y/Ny + Δx·x/Nx))
//   mu2D = max_{(Δy,Δx)≠(0,0)} |r| / m
// 这是二维傅里叶字典下的最大互干度，**包含跨行相干性**。
// 性质：全采样 -> 0；单点 -> 1；值域 [0,1]。
// 计算量：O(Ny·Nx·m)，对 32×32/16×16 规模可接受。
// =============================================================================
float SamplingMutualCoherence2D(const std::vector<int>& maskFlat, int Ny, int Nx);
/*-----------------------------------------------------
 *  Function: 二维采样掩码的经典互干度（2D PSF 峰值旁瓣）
 *  Input   : maskFlat -- 长度 Ny*Nx 的 0/1 掩码，索引 = y*Nx + x
 *            Ny, Nx   -- 网格尺寸
 *  Output  : mu ∈ [0,1]；m=0 或尺寸非法时返回 0
 *-----------------------------------------------------*/
float SamplingMutualCoherence1D(const std::vector<int>& idx, int N);
/*-----------------------------------------------------
 *  Function: 采样点集的经典互干度（PSF 峰值旁瓣）
 *  Input   : idx -- 采样位置（升序或无序均可，内部不去重）
 *            N   -- 空间总点数
 *  Output  : mu ∈ [0,1]；m=0 或 N<=0 时返回 0
 *-----------------------------------------------------*/

//------ μ 评估计数器（实验效率统计用）------//
void miu_eval_reset();//计数清零
long miu_eval_count();//当前累计μ评估次数（包含GUI与命令行全部调用）

#endif // CALCULATEMIU_H