/*****************************************************
 * OPTIMIZEMIU.H
 *   Optimize Mutual Interference Parameter μ
 *   Based on Curvelet transform
 *   Based on CalculateMiu
 *	 Created on Nov 1st, 2022
 *   2026.10.15: 移除 Qt 依赖并完善状态跟踪 // add by wushanzhou in 2026/10/15（命令行实验框架复用本文件）；
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
#include "../../third_party/curvelet/Curvelet2d/fftw.h"
#include "../../third_party/curvelet/Curvelet2d/fdct_wrapping.hpp"
#include "../../third_party/curvelet/Curvelet2d/fdct_wrapping_inline.hpp"
#include "../sampling/CalculateMiu.h"
#include "../sampling/sample.h"

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

enum SAAcceptanceStrategy // add by wushanzhou in 2026/10/15

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



int *UnOptiMiuSAEx( // add by wushanzhou in 2026/10/15
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
 *  Function: 基于数值梯度的局部坐标搜索精修（原地修改并返回 Phi）
 *  Note    : 实现是"坐标贪心 ±1 移动"，并非标准共轭梯度法；
 *            名称保留以兼容既有接口与结果文件。
 *-----------------------------------------------------*/

// ---- 2026.10.20 新增：混合优化消融实验 ----
struct AblationResult // add by wushanzhou in 2026.10.20
{
	int n;                  // 掩码长度
	int m;                  // 采样点数
	double evalBudget;      // SA+CG 实际累计 μ 评估次数
	float mu_init;          // 初始抖动掩码 μ
	float mu_sa_only;       // 仅 SA（cg_max_iter = 0）
	float mu_cg_only;       // 仅局部搜索（跳过 SA）
	float mu_sa_cg;         // 当前主口径：SA + 局部搜索
	float mu_restart_best;  // 等预算抖动随机重启（K 次取最优）
	float mu_restart_mean;  // 等预算抖动随机重启的均值
	float mu_rand_best;     // 等预算纯随机掩码（K 次取最优）
	int   restart_trials;   // 随机重启次数 K
	AblationResult()
		: n(0), m(0), evalBudget(0.0), mu_init(0), mu_sa_only(0), mu_cg_only(0),
		  mu_sa_cg(0), mu_restart_best(0), mu_restart_mean(0), mu_rand_best(0),
		  restart_trials(0) {}
};

// =============================================================================
// 2026-10-21 新增：带空间约束的目标函数（任务书问题 2，方案 C/D/E）
// -----------------------------------------------------------------------------
// 算法审计（README.md §7.2）证明：互干度 μ 与空间均匀性是两个独立目标，
//   μ 甚至会奖励聚集。因此必须把均匀性作为显式项加入目标。
//
// 目标函数：
//   J(Φ) = μ(Φ) + λ_gap·P_gap(Φ) + λ_nn·P_nn(Φ)
//   P_gap = max(0, maxgap(Φ)/idealGap - 1)   idealGap = N/m
//   P_nn  = CV(最近邻间隔)                    无量纲
// 两个惩罚项均与采样率无关，可跨档比较。
// =============================================================================
struct ConstraintWeights // add by wushanzhou in 2026.10.21
{
    double lambdaGap;   // P_gap 权重
    double lambdaNN;    // P_nn 权重
    int    hardMaxGap;  // >0 时作为硬约束（方案 D）；0 表示仅软惩罚
    double hardMaxGapM; // Maximum physical gap, including boundary gaps; requires coordinates. add by wushanzhou in 2026/10/24
    ConstraintWeights() : lambdaGap(0.0), lambdaNN(0.0), hardMaxGap(0), hardMaxGapM(0) {} // add by wushanzhou in 2026/10/24
};

struct ConstrainedResult // add by wushanzhou in 2026.10.21
{
    float mu;            // 终止时的经典互干度
    float objective;     // 终止时的 J 值
    int   maxGap;
    float nnCv;
    float unitOccupancy;
    long  evals;         // 实际 μ 评估次数
    bool  hardConstraintSatisfied;
    std::vector<int> mask; // add by wushanzhou in 2026/10/23
    std::vector<int> saPrefixMask; // add by wushanzhou in 2026/10/23
    std::vector<float> bestTrace; // add by wushanzhou in 2026/10/23
    std::vector<float> currentTrace; // add by wushanzhou in 2026/10/23
    long saPrefixEvals; // add by wushanzhou in 2026/10/23
    double runtimeMs; // add by wushanzhou in 2026/10/23
    double maxGapM; // Physical-coordinate gap when coordinates are supplied. add by wushanzhou in 2026/10/23
    std::string status; // add by wushanzhou in 2026/10/23
    int saEvaluations, localEvaluations, restartEvaluations, switches; // add by wushanzhou in 2026/10/24
    std::vector<int> phaseTrace; // 0 initial, 1 SA, 2 local, 3 perturbation. add by wushanzhou in 2026/10/24
    // v5 audit fields: v4 currentTrace is retained as the candidate trace for compatibility.
    long long proposalAttempts;
    std::vector<float> acceptedTrace;
    std::vector<double> temperatureTrace, wallTraceMs;
    double runtimeWallMs;
    ConstrainedResult() : mu(0), objective(0), maxGap(0), nnCv(0),
                           unitOccupancy(0), evals(0), hardConstraintSatisfied(false), // add by wushanzhou in 2026/10/23
                           saPrefixEvals(0), runtimeMs(0), maxGapM(0), saEvaluations(0), // add by wushanzhou in 2026/10/24
                           localEvaluations(0), restartEvaluations(0), switches(0),
                           proposalAttempts(0), runtimeWallMs(0) {} // add by wushanzhou in 2026/10/24
};

// =============================================================================
// 2026-10-21 新增：多种子等预算消融（任务书问题 3）
// 每个种子都保留原始结果，不筛选。
// =============================================================================
struct AblationRow // add by wushanzhou in 2026.10.21
{
    int    seedIndex;
    unsigned long long seed;
    float  muJitterInit;   // ① 初始解
    float  muSaOnly;       // ② SA-only
    float  muLocalOnly;    // ③ 局部搜索-only
    float  muSaLocal;      // ④ SA+局部搜索
    float  muRestartBest;  // ⑤ 等预算随机重启最优
    float  muConstrained;  // ⑥ 带空间约束的混合
    long   evalsSaOnly;
    long   evalsLocalOnly;
    long   evalsSaLocal;
    long   evalsRestart;
    long   evalsConstrained;
    int    maxGapSaLocal;
    int    maxGapConstrained;
    float  nnCvSaLocal;
    float  nnCvConstrained;
    AblationRow() : seedIndex(0), seed(0), muJitterInit(0), muSaOnly(0), muLocalOnly(0),
        muSaLocal(0), muRestartBest(0), muConstrained(0), evalsSaOnly(0), evalsLocalOnly(0),
        evalsSaLocal(0), evalsRestart(0), evalsConstrained(0), maxGapSaLocal(0),
        maxGapConstrained(0), nnCvSaLocal(0), nnCvConstrained(0) {}
};

// Formal experiments: one PSF objective, identical evaluation caps, explicit constraints. // add by wushanzhou in 2026/10/23
enum SamplingSearchMethod { SEARCH_SA, SEARCH_LOCAL, SEARCH_HYBRID, SEARCH_RESTART }; // add by wushanzhou in 2026/10/23
float SamplingMaskCoherence(const std::vector<int>& mask); // add by wushanzhou in 2026/10/23
int SamplingMaxGap(const std::vector<int>& mask); // add by wushanzhou in 2026/10/23
float SamplingNNCv(const std::vector<int>& mask); // add by wushanzhou in 2026/10/23
float SamplingUnitOccupancy(const std::vector<int>& mask); // add by wushanzhou in 2026/10/23
bool RunSamplingSearch(int N, int m, const std::vector<int>& initial, // add by wushanzhou in 2026/10/23
                       const ConstraintWeights& weights, unsigned long long seed, // add by wushanzhou in 2026/10/23
                       int evalBudget, SamplingSearchMethod method, // add by wushanzhou in 2026/10/23
                       ConstrainedResult& out, int saPrefixBudget = 64, int gridRows = 1, // add by wushanzhou in 2026/10/23
                       const std::vector<double>* coordinates = NULL,
                       const std::vector<int>* eligibility = NULL); // add by wushanzhou in 2026/10/23
float SamplingPhysicalCoherence(const std::vector<int>& mask, const std::vector<double>& coordinates); // add by wushanzhou in 2026/10/23

// Separately versioned adaptive method; legacy search defaults remain available. add by wushanzhou in 2026/10/24
struct AdaptiveSearchOptions // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    bool adaptiveSwitch, recycleBudget, mixedNeighborhood, localSearch; // add by wushanzhou in 2026/10/24
    int minimumSa, stagnationWindow; // add by wushanzhou in 2026/10/24
    // New protocol only. Empty eligibility preserves v4; no amplitude enters search.
    std::vector<int> eligible;
    // Pilot-assisted v2 only: required observations and nonzero Fourier difference weights.
    std::vector<int> required;
    std::vector<double> frequencyWeights;
    double wallLimitMs;
    bool perturbation;
    AdaptiveSearchOptions() : adaptiveSwitch(true), recycleBudget(true), mixedNeighborhood(true), // add by wushanzhou in 2026/10/24
        localSearch(true), minimumSa(32), stagnationWindow(12), wallLimitMs(0), perturbation(true) {} // add by wushanzhou in 2026/10/24
}; // add by wushanzhou in 2026/10/24
double SamplingPhysicalMaxGap(const std::vector<int>& mask,const std::vector<double>& x); // add by wushanzhou in 2026/10/24
bool SamplingPhysicalFeasibleMask(const std::vector<double>& x,int m,double bound,std::vector<int>& mask); // add by wushanzhou in 2026/10/24
bool RunAdaptiveSamplingSearch(int N,int m,const std::vector<int>& initial,const ConstraintWeights& weights, // add by wushanzhou in 2026/10/24
    unsigned long long seed,int budget,const AdaptiveSearchOptions& options,ConstrainedResult& out, // add by wushanzhou in 2026/10/24
    const std::vector<double>* coordinates=NULL); // add by wushanzhou in 2026/10/24

bool RunMultiSeedAblation(int N, float c, int nSeeds, unsigned long long baseSeed,
                          std::vector<AblationRow>& outRows);
/*-----------------------------------------------------
 *  Function: 多种子等预算消融
 *  Note    : 每个种子独立生成初始掩码，三种策略共享同一初始解，
 *            预算按 μ 评估次数对齐。
 *-----------------------------------------------------*/

bool RunConstrainedSA(int N, int m, const std::vector<int>& initMask,
                      const ConstraintWeights& w, unsigned long long seed,
                      int maxSteps, ConstrainedResult& out);
/*-----------------------------------------------------
 *  Function: 带空间约束的 SA（方案 C/D）
 *  Input   : N,m -- 掩码长度与点数；initMask -- 初始掩码（0/1）
 *            w   -- 惩罚权重；hardMaxGap>0 时启用硬约束
 *            seed -- 复现种子；maxSteps -- 温度步数上限
 *  Output  : out -- 终止时的 μ / J / 均匀性 / 评估次数
 *  Note    : 硬约束方案下，不满足 maxgap 的候选解直接被拒绝（可行性投影）。
 *-----------------------------------------------------*/
bool RunHybridAblation(int N, float c, unsigned long long seed,
					   const SAParams &sap, AblationResult &out, int restartTrials = 0);
/*-----------------------------------------------------
 *  Function: 混合优化贡献分解消融
 *  策略  : SA-only / CG-only / SA+CG / 等预算抖动随机重启 / 等预算纯随机
 *  Input : N, c -- 掩码长度与采样率；seed -- 复现种子
 *          sap -- SA 超参数（应与基准一致）
 *          restartTrials -- 重启次数；<=0 时自动取 SA+CG 的评估次数
 *  Note  : 所有策略共享同一初始抖动掩码，确保可比。
 *-----------------------------------------------------*/

/* 辅助函数：计算数值梯度 */
void ComputeNumericalGradient(int N, int *Phi, float *grad);

#endif // OPTIMIZEMIU_H
