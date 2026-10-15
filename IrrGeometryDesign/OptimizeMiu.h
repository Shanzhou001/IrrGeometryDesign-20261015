/*****************************************************
 * OPTIMIZEMIU.H
 *   Optimize Mutual Interference Parameter μ
 *   Based on Curvelet transform
 *   Based on CalculateMiu
 *	 Created on Nov 1st, 2022
 *   2026.10.15: 移除 Qt 依赖并完善状态跟踪 // add by shanzhou in 2026/10/15（命令行实验框架复用本文件）；
 *              删除只有声明没有实现的 GeneticOptimizeMiu；
 *              新增 UnOptiMiuSAEx（带收敛轨迹输出的扩展版）。
 *****************************************************/
#ifndef _OPTIMIZEMIU_H_
#define _OPTIMIZEMIU_H_
#include <complex>
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <time.h>
#include "fftw.h"
#include "fdct_wrapping.hpp"
#include "fdct_wrapping_inline.hpp"
#include "CalculateMiu.h"
#include "sample.h"

using namespace std;
using namespace fdct_wrapping_ns;

int *UnOptiMiuSA(int N, float c, int *Phi, string samptype);//2023.5.16修改
/*-----------------------------------------------------
 *  Function: 联合改进模拟退火算法优化μ值（SA 全局搜索 + CG 局部精修）
 *  Input:
 *		N -- 矩阵长度  c -- 采样率  Phi--采样矩阵 samptype--采样类型
 *	Output:
 *		Phi -- 优化之后μ值对应的矩阵（new[] 分配，调用方 delete[]）
 *	Note : 本函数为 UnOptiMiuSAEx 的简化包装（无统计输出）
 *-----------------------------------------------------*/

enum SAAcceptanceStrategy // add by shanzhou in 2026/10/15

{

	SA_ACCEPT_METROPOLIS = 0, // 经典 Metropolis 准则: p = exp(-delta / T)

	SA_ACCEPT_PROGRESS   = 1  // 自定义迭代进度准则: p = exp(-20 * (t/Time)^5)

};



struct SAStats

/*-----------------------------------------------------

 *  优化过程统计信息（UnOptiMiuSAEx 输出，实验记录用）

 *-----------------------------------------------------*/

{

	float miu0;                      //初始μ值

	float miu_sa_best;               //SA 结束时的历史最优μ值

	float miu_final;                 //CG 精修后的最终μ值

	double sa_time;                  //SA 耗时（秒）

	double cg_time;                  //CG 耗时（秒）

	int sa_steps;                    //SA 实际迭代步数

	long mu_evals;                   //本阶段μ评估次数（效率对比用）

	std::vector<float> mu_trace;     //当前解逐步μ值（探索轨迹）

	std::vector<float> best_mu_trace;//历史最优逐步μ值（单调非增收敛曲线）

	std::vector<long> eval_trace;    //逐步累计μ评估次数（收敛曲线的效率横轴）

	SAStats() : miu0(0), miu_sa_best(0), miu_final(0), sa_time(0), cg_time(0),

				sa_steps(0), mu_evals(0) {}

};



struct SAParams

/*-----------------------------------------------------

 *  模拟退火超参数配置

 *-----------------------------------------------------*/

{

	double T0;        //初始温度（默认 100）

	double Tmin;      //终止温度（默认 1.0）

	double alpha;     //降温系数（默认 0.80）

	int n_rapid;      //急速降温阶段步数（默认 10）

	int iter;         //每温度内循环次数（默认 1）

	SAAcceptanceStrategy strategy; //退火接受策略 (默认 Metropolis)

	double temp_scale;             //温度标度系数（默认 1.0）

	SAParams() : T0(100), Tmin(1.0), alpha(0.80), n_rapid(10), iter(1),

	             strategy(SA_ACCEPT_METROPOLIS), temp_scale(1.0) {}

};



int *UnOptiMiuSAEx( // add by shanzhou in 2026/10/15
    int N, float c, int *Phi, string samptype,
				   bool neighbor_mode, SAStats *stats, int cg_max_iter = 2);
/*-----------------------------------------------------
 *  Function: 联合改进模拟退火算法优化μ值（扩展版，默认超参数）
 *  （见下方带 SAParams 的完整版；本函数保持原签名，行为不变）
 *-----------------------------------------------------*/

int *UnOptiMiuSAEx(int N, float c, int *Phi, string samptype,
				   bool neighbor_mode, SAStats *stats, int cg_max_iter,
				   const SAParams &sap);
/*-----------------------------------------------------
 *  Function: 联合改进模拟退火算法优化μ值（扩展版，自定义超参数）
 *  Input:
 *		N -- 矩阵长度  c -- 采样率  Phi--采样矩阵 samptype--采样类型
 *		neighbor_mode -- false: 候选解整体重新采样（原行为，保持与旧实验可比）
 *						 true : 候选解由当前解邻域扰动生成（标准 SA 邻域搜索）
 *		stats -- 可为 NULL；非 NULL 时回填收敛轨迹与耗时
 *		cg_max_iter -- CG 精修最大迭代轮数（0=跳过 CG，即纯 SA）
 *		sap -- SA 超参数（T0/Tmin/alpha/n_rapid/iter）
 *	Output:
 *		返回历史最优解经 CG 精修后的矩阵（new[] 分配，调用方 delete[]）
 *	Note : 2026.9.18 修复 —— 原实现的马尔可夫链未维护"当前解"状态
 *			（只更新 μ 标量），且"记忆最优解"保存的是最后一次候选而非
 *			最优 μ 对应的矩阵；现维护链状态并记录真正的历史最优。
 *-----------------------------------------------------*/

int *OptiMiuCG(int N, int *Phi, int max_iter = 2);//2025.12.13修改
/*-----------------------------------------------------
 *  Function: 基于数值梯度的共轭梯度局部精修（原地修改并返回 Phi）
 *-----------------------------------------------------*/

/* 辅助函数：计算数值梯度 */
void ComputeNumericalGradient(int N, int *Phi, float *grad);

#endif // OPTIMIZEMIU_H
