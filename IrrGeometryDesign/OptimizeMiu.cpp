/***************************************************
 * Name    : OptimizeMiu.cpp
 * Function: Optimize Miu by enumeration method and SA
 *			 根据模拟退火法优化μ值
 *           包括常规模拟退火和联合改进模拟退火算法和共轭梯度算法
 * 修复日期: 2025.12.24
 * 修复内容: 移除多线程冲突、增强边界检查、修复内存访问错误
 * 修复日期: 2026.10.15 // add by shanzhou in 2026/10/15
 * 修复内容(影响实验有效性的问题):
 *   1) UnOptiMiuSA 的马尔可夫链没有维护"当前解"状态——接受新解时
 *      只更新 μ 标量，解向量从不更新，导致退火搜索实际上不连续；
 *   2) "记忆最优解"保存的是最后一次候选解（无论是否被接受），与
 *      记录的最优 μ 值不对应；现维护链状态并保存真正的历史最优；
 *   3) OptiMiuSA 中 return 之后的释放语句为死代码、record 以
 *      sizeof(float*) 分配——已修正，并在返回前释放；
 *   4) 移除 Qt 头文件依赖（命令行实验框架复用本文件）；
 *   5) 随机源统一为 ExpRng（可播种复现，替代全局 rand()+srand）。
/***************************************************/

#include "CalculateMiu.h"
#include "sample.h"
#include "OptimizeMiu.h"
#include "ExpRng.h"
#include <time.h>

#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cstring>

#pragma warning(disable:4244)

using namespace std;

//*********** 候选解生成 - 前向声明 **********//
static int* gen_candidate_regen(int N, float c, const string& samptype);

static int* gen_candidate_neighbor(int N, const int* cur, const string& samptype);

//*********** 常规模拟退火算法（旧版接口，GUI 历史保留，未被调用）**********//
// 2026.9.18 重写：修正候选解泄漏、record 尺寸错误与链状态缺失。
// 返回 new[] 分配的优化矩阵，调用方 delete[] 释放。
int *OptiMiuSA(double T, double Tmin, int iter, double alpha, int N,float c, int *Phi, string samptype) // add by shanzhou in 2026/10/15
{
	if (Phi == NULL || N <= 0) return Phi;

	int steps = (int)(log(Tmin/T)/log(alpha)) + 1;//循环次数（+1 防止浮点误差越界）
	if (steps < 1) steps = 1;
	float *record = (float *)malloc(steps * sizeof(float)); //修复：原为 sizeof(float*)
	int *cur = new int[N];
	memcpy(cur, Phi, sizeof(int)*N); //修复：维护链状态（原实现仅按指针替换，被拒候选泄漏）

	float miu = TestCurveletMiu1D(N, cur, "EigenReal");
	int t = 0;
	while (T > Tmin && t < steps)
	{
		for (int k = 0; k < iter; k++)
		{
			int *phi_cand = gen_candidate_regen(N, c, samptype);
			float miu_new = TestCurveletMiu1D(N, phi_cand, "EigenReal");
			float delta = miu_new - miu;
			bool accept = (delta < 0) || (exp(-delta/T) > exprng::uniform());
			if (accept)
			{
				memcpy(cur, phi_cand, sizeof(int)*N); //经典 Metropolis 接受准则
				miu = miu_new;
			}
			delete[] phi_cand;
		}
		if (record) record[t] = miu;
		T = T*alpha;
		t++;
	}
	if (record) free(record);
	return cur;
}

//************ 候选解生成 ************//
// 根据 samptype 生成一个全新候选解（原行为：整体重新采样）
static int* gen_candidate_regen(int N, float c, const string& samptype) // add by shanzhou in 2026/10/15
{
	int m = (int)floor((double)N * c + 0.5);
	if (samptype == "F") return Sampling1F_Count(N, m);
	if (samptype == "J") return Sampling1J_Count(N, m);
	return Sampling1S_Count(N, m);
}

// 邻域扰动幅度（交换的采样点对数）// add by shanzhou in 2026/10/19
// 原实现固定取 m/20（50% 采样率下每次交换约 3 对点），属于"大邻域跳转"，
// 退火轨迹更接近多起点随机重启而非标准 SA 的单点邻域搜索。
// 现默认改为标准 SA 的单点交换 (1 对)，既保持采样点数严格守恒，
// 又使 Metropolis 接受准则真正作用于"小步扰动"的局部邻域。
// 可通过环境变量 IRR_SA_NEIGHBOR_K 指定固定对数（>0），用于邻域幅度敏感性分析。
static int sa_neighbor_swaps()
{
	static int cached = -2; // -2 = 未初始化
	if (cached != -2) return cached;
	const char *envs = getenv("IRR_SA_NEIGHBOR_K");
	int k = 1;
	if (envs && envs[0])
	{
		int v = atoi(envs);
		if (v >= 1) k = v;
	}
	cached = k;
	return cached;
}

static int* gen_candidate_neighbor(int N, const int* cur, const string& samptype)
{
	(void)samptype;
	int *cand = new int[N];
	memcpy(cand, cur, sizeof(int)*N);
	vector<int> ones, zeros;
	ones.reserve(N/2); zeros.reserve(N/2);
	for (int i = 0; i < N; i++)
	{
		if (cur[i] == 1) ones.push_back(i);
		else zeros.push_back(i);
	}
	if (ones.empty() || zeros.empty()) return cand;
	// 标准 SA 单点交换 (默认 1 对)，保持采样点数严格守恒 // 2026.10.19
	int nm = sa_neighbor_swaps();
	if (nm < 1) nm = 1;
	for (int k = 0; k < nm; k++)
	{
		int a = ones[exprng::integer() % ones.size()];
		int b = zeros[exprng::integer() % zeros.size()];
		if (cand[a] == 1 && cand[b] == 0)
		{
			cand[a] = 0;
			cand[b] = 1;
		}
	}
	return cand;
}

//************ 联合改进模拟退火算法 - 修复版 2026.9.18 ************//
int *UnOptiMiuSAEx(int N, float c, int *Phi, string samptype,
				   bool neighbor_mode, SAStats *stats, int cg_max_iter)
{
	SAParams sap; //默认超参数（与原实现一致）
	return UnOptiMiuSAEx(N, c, Phi, samptype, neighbor_mode, stats, cg_max_iter, sap);
}

int *UnOptiMiuSAEx(int N, float c, int *Phi, string samptype, // add by shanzhou in 2026/10/15

				   bool neighbor_mode, SAStats *stats, int cg_max_iter,

				   const SAParams &sap)

{

	// 参数防御性校验与约束

	if (Phi == NULL || N <= 0) return Phi;



	double T0 = sap.T0 > 0.0 ? sap.T0 : 100.0;

	double Tmin = (sap.Tmin > 0.0 && sap.Tmin < T0) ? sap.Tmin : 1.0;

	double alpha = (sap.alpha > 0.01 && sap.alpha < 0.999) ? sap.alpha : 0.80;

	int n = sap.n_rapid >= 1 ? sap.n_rapid : 10;

	int iter = sap.iter >= 1 ? sap.iter : 1;



	double T = T0;

	double T1 = T0 * exp(-alpha * pow((double)n, 0.5));

	int r2 = (int)(log(Tmin / T1) / log(alpha));

	if (r2 < 1) r2 = 1;

	double Time = n + r2;



	float miu = 0.0f, miu_new = 0.0f;

	int max_steps = n + r2 + 100;

	float *record = new float[max_steps];



	// 维护马尔可夫链当前解与历史最优解

	int *cur = new int[N];

	int *best = new int[N];

	for (int k = 0; k < N; k++)

	{

		cur[k] = Phi[k];

		best[k] = Phi[k];

	}

	for (int k = 0; k < max_steps; k++) record[k] = 0.0f;



	int t = 0;

	float miu_best = 1e9f;



	// 修复：清空所有统计容器，避免历史旧数据残留

	if (stats)

	{

		stats->miu0 = 0.0f;

		stats->miu_sa_best = 0.0f;

		stats->miu_final = 0.0f;

		stats->sa_time = 0.0;

		stats->cg_time = 0.0;

		stats->sa_steps = 0;

		stats->mu_evals = 0;

		stats->mu_trace.clear();

		stats->best_mu_trace.clear();

		stats->eval_trace.clear();

	}



	clock_t sa_begin = clock();

	long evals_begin = miu_eval_count();



	miu = TestCurveletMiu1D(N, Phi, "EigenReal");

	miu_best = miu;

	if (stats)

	{

		stats->miu0 = miu;

		stats->mu_trace.push_back(miu);

		stats->best_mu_trace.push_back(miu_best);

		stats->eval_trace.push_back(1);

	}



	// 根据初始 miu 标定 Metropolis 温度标度（目标量级自适应）

	double scale = (sap.temp_scale > 0.0) ? sap.temp_scale : 1.0;

	if (scale == 1.0 && miu > 0.0f && T0 > 10.0)

	{

		scale = (double)miu / T0;

	}



	// SA 算法主循环

	while (T > Tmin && t < max_steps - 1)

	{

		for (int i = 0; i < iter; i++)

		{

			int *phi_new_i = neighbor_mode ? gen_candidate_neighbor(N, cur, samptype)

										   : gen_candidate_regen(N, c, samptype);

			if (phi_new_i == NULL) continue;



			miu_new = TestCurveletMiu1D(N, phi_new_i, "EigenReal");

			float delta = miu_new - miu;

			bool accept = false;



			if (delta <= 0.0f)

			{

				accept = true; // 新解更优或等优，直接接受

			}

			else

			{

				if (sap.strategy == SA_ACCEPT_METROPOLIS)

				{

					// 经典 Metropolis 准则: p = exp(-delta / (T * scale))

					double eff_T = T * scale;

					if (eff_T < 1e-12) eff_T = 1e-12;

					double exponent = -(double)delta / eff_T;

					if (exponent < -50.0) exponent = -50.0;

					float p = (float)exp(exponent);

					if (p >= exprng::uniform())

					{

						accept = true;

					}

				}

				else

				{

					// 自定义迭代进度准则

					float p = (float)exp(-20.0 * pow((double)t / Time, 5.0));

					if (p >= exprng::uniform())

					{

						accept = true;

					}

				}

			}



			if (accept)

			{

				memcpy(cur, phi_new_i, sizeof(int) * N);

				miu = miu_new;

			}

			delete[] phi_new_i;

		}



		record[t] = miu;

		if (miu < miu_best)

		{

			miu_best = miu;

			memcpy(best, cur, sizeof(int) * N);

		}



		if (stats)

		{

			stats->mu_trace.push_back(miu);

			stats->best_mu_trace.push_back(miu_best);

			stats->eval_trace.push_back(miu_eval_count() - evals_begin);

		}



		t = t + 1;

		if (t < n + 1)

		{

			T = T0 * exp(-alpha * pow((double)t, 0.5));

		}

		else

		{

			T = T * alpha;

		}

	}



	if (stats)

	{

		stats->sa_steps = t;

		stats->miu_sa_best = miu_best;

		stats->sa_time = (double)(clock() - sa_begin) / CLOCKS_PER_SEC;

		stats->mu_evals = miu_eval_count() - evals_begin;

	}



	// 混合优化策略：CG 局部精修（若 cg_max_iter <= 0 则跳过）(add by shanzhou in 2026/10/15)
	clock_t cg_begin = clock();
	if (cg_max_iter > 0)
	{
		int *sa_best_copy = new int[N];
		memcpy(sa_best_copy, best, sizeof(int) * N);
		float sa_best_miu = miu_best;

		OptiMiuCG(N, best, cg_max_iter);
		float cg_miu = TestCurveletMiu1D(N, best, "EigenReal");

		if (cg_miu <= sa_best_miu)
		{
			miu_best = cg_miu;
		}
		else
		{
			// 若 CG 精修导致相干度升高或变差，则严格回退保留模拟退火历史最优解
			memcpy(best, sa_best_copy, sizeof(int) * N);
		}
		delete[] sa_best_copy;
	}

	if (stats)
	{
		stats->cg_time = (double)(clock() - cg_begin) / CLOCKS_PER_SEC;
		stats->miu_final = miu_best;
	}

	if (record != NULL) delete[] record;
	if (cur != NULL) delete[] cur;

	return best;
}



// 兼容旧接口
int *UnOptiMiuSA(int N, float c, int *Phi, string samptype)
{
	return UnOptiMiuSAEx(N, c, Phi, samptype, false, NULL, 2);
}

// ==========================================================
//  基于数值梯度的共轭梯度算法  2025.12.24 by 吴善州
//  用于混合优化策略的局部精修
//  修复版本：移除多线程、增强边界检查、修复内存访问
// ==========================================================

// 获取采样点的位置索引
vector<int> GetIndices(int N, int *Phi) {
	if (Phi == NULL || N <= 0) return vector<int>();

	vector<int> idx;
	for (int i = 0; i < N; i++) {
		if (Phi[i] == 1) idx.push_back(i);
	}
	return idx;
}

// 1. 计算数值梯度 (Numerical Gradient) - 修复版：移除多线程
void ComputeNumericalGradient(int N, int *Phi, float *grad) {
	if (Phi == NULL || grad == NULL || N <= 0) return;

	float current_miu = TestCurveletMiu1D(N, Phi, "EigenReal");
	vector<int> idx = GetIndices(N, Phi);
	int num_indices = (int)idx.size();

	if (num_indices == 0) return;

	// 初始化梯度
	for(int i=0; i<num_indices; i++) grad[i] = 0.0;

	// 移除多线程并行计算，改为串行计算以避免冲突
	// 原来的 #pragma omp parallel for 已被移除
		int *local_Phi = new int[N];
		if (local_Phi == NULL) return;

		for (int k = 0; k < num_indices; k++) {
			memcpy(local_Phi, Phi, N * sizeof(int));

		int original_pos = idx[k];

		// 边界检查
		if (original_pos < 0 || original_pos >= N) {
			continue;
		}

		// 向右试探
		if (original_pos + 1 < N && local_Phi[original_pos + 1] == 0) {
			local_Phi[original_pos] = 0;
			local_Phi[original_pos + 1] = 1;
			float miu_right = TestCurveletMiu1D(N, local_Phi, "EigenReal");
			// 恢复
			local_Phi[original_pos + 1] = 0;
			local_Phi[original_pos] = 1;
			grad[k] = (miu_right - current_miu);
		}
		// 向左试探
		else if (original_pos - 1 >= 0 && local_Phi[original_pos - 1] == 0) {
			local_Phi[original_pos] = 0;
			local_Phi[original_pos - 1] = 1;
			float miu_left = TestCurveletMiu1D(N, local_Phi, "EigenReal");
			// 恢复
			local_Phi[original_pos - 1] = 0;
			local_Phi[original_pos] = 1;

			if (grad[k] == 0.0) grad[k] = (current_miu - miu_left);
			else grad[k] = (grad[k] + (current_miu - miu_left)) / 2.0;
		}
	}
	delete[] local_Phi;// 释放临时内存
}

// 2. 共轭梯度主函数 - 修复版：增强边界检查和NULL指针检查
int *OptiMiuCG(int N, int *Phi, int max_iter) {
	if (Phi == NULL || N <= 0) return Phi;
	if (max_iter < 1) max_iter = 1;

	vector<int> idx = GetIndices(N, Phi);
	int num_samples = (int)idx.size();

	if (num_samples == 0) return Phi; // 没有采样点，直接返回

	float *g_k = new float[N];
	float *g_new = new float[N];
	float *d_k = new float[N];

	memset(g_k, 0, N * sizeof(float));
	memset(g_new, 0, N * sizeof(float));
	memset(d_k, 0, N * sizeof(float));

	ComputeNumericalGradient(N, Phi, g_k);
	for(int i=0; i<num_samples; i++) d_k[i] = -g_k[i];

	float current_miu = TestCurveletMiu1D(N, Phi, "EigenReal");

	for (int iter = 0; iter < max_iter; iter++) {
		bool improved = false;

		// 线性搜索
		for (int i = 0; i < num_samples; i++) {
			if (fabs(d_k[i]) > 1e-6) {
				int old_pos = idx[i];

				// 边界检查
				if (old_pos < 0 || old_pos >= N) continue;

				int shift = (d_k[i] > 0) ? 1 : -1;
				int new_pos = old_pos + shift;

				if (new_pos >= 0 && new_pos < N && Phi[new_pos] == 0) {
					Phi[old_pos] = 0;
					Phi[new_pos] = 1;
					float new_miu = TestCurveletMiu1D(N, Phi, "EigenReal");

					if (new_miu < current_miu) {
						current_miu = new_miu;
						idx[i] = new_pos;
						improved = true;
					} else {
						Phi[new_pos] = 0;
						Phi[old_pos] = 1;
					}
				}
			}
		}
		if (!improved) break;

		ComputeNumericalGradient(N, Phi, g_new);
		float num = 0.0, den = 0.0;
		for(int i=0; i<num_samples; i++) {
			num += g_new[i] * g_new[i];
			den += g_k[i] * g_k[i];
		}
		float beta = (den > 1e-9) ? num / den : 0.0;
		for(int i=0; i<num_samples; i++) {
			d_k[i] = -g_new[i] + beta * d_k[i];
			g_k[i] = g_new[i];
		}
	}

	delete[] g_k;
	delete[] g_new;
	delete[] d_k;
	return Phi;
}

// ==========================================================
//  2026.10.20 新增：混合优化消融（SA-only / CG-only / SA+CG / 等预算随机重启）
//  目的：此前所有对外表格只给"Jitter-SA"一个数，无法回答
//        "SA 与局部搜索各自的贡献有多大""混合是否优于等预算随机重启"。
//  所有策略共享同一初始抖动掩码与同一随机种子，保证可比。
// ==========================================================
bool RunHybridAblation(int N, float c, unsigned long long seed,
					   const SAParams &sap, AblationResult &out, int restartTrials)
{
	out = AblationResult();
	if (N <= 0) return false;
	int m = (int)floor((double)N * (double)c + 0.5);
	if (m < 2) m = 2;
	if (m > N) m = N;
	out.n = N;
	out.m = m;

	// ---- 共享的初始抖动掩码 ----
	exprng::seed(seed);
	int *phi0 = Sampling1J_Count(N, m);
	if (!ValidateSamplingMask(N, m, phi0)) { delete[] phi0; return false; }
	out.mu_init = TestCurveletMiu1D(N, phi0, "EigenReal");

	// ---- 主口径：SA + 局部搜索（同时得到真实评估预算） ----
	long evalsBefore = miu_eval_count();
	SAStats stMain;
	{
		int *phiIn = new int[N];
		memcpy(phiIn, phi0, sizeof(int) * N);
		int *phiOut = UnOptiMiuSAEx(N, c, phiIn, "J", true, &stMain, 2, sap);
		out.mu_sa_cg = stMain.miu_final;
		delete[] phiOut;
		delete[] phiIn;
	}
	out.evalBudget = (double)(miu_eval_count() - evalsBefore);

	// ---- SA-only：跳过局部搜索 ----
	{
		int *phiIn = new int[N];
		memcpy(phiIn, phi0, sizeof(int) * N);
		SAStats st;
		int *phiOut = UnOptiMiuSAEx(N, c, phiIn, "J", true, &st, 0, sap);
		out.mu_sa_only = st.miu_final;
		delete[] phiOut;
		delete[] phiIn;
	}

	// ---- CG-only：从同一初始掩码直接做局部搜索 ----
	{
		int *phiIn = new int[N];
		memcpy(phiIn, phi0, sizeof(int) * N);
		OptiMiuCG(N, phiIn, 2);
		out.mu_cg_only = TestCurveletMiu1D(N, phiIn, "EigenReal");
		delete[] phiIn;
	}

	// ---- 等预算随机重启：K 个抖动掩码取最优，K = SA+CG 的评估次数 ----
	int K = (restartTrials > 0) ? restartTrials : (int)out.evalBudget;
	if (K < 1) K = 1;
	if (K > 2000) K = 2000;   // 防御：避免极端配置下耗时爆炸
	out.restart_trials = K;
	{
		double sum = 0.0;
		float best = 1e9f;
		for (int k = 0; k < K; k++)
		{
			int *phi = Sampling1J_Count(N, m);
			if (phi == NULL) continue;
			float mu = TestCurveletMiu1D(N, phi, "EigenReal");
			delete[] phi;
			sum += mu;
			if (mu < best) best = mu;
		}
		out.mu_restart_best = (best < 1e9f) ? best : 0.0f;
		out.mu_restart_mean = (float)(sum / (double)K);
	}

	// ---- 等预算纯随机掩码：K 次取最优（第二种对照，排除"抖动先验"的贡献） ----
	{
		float best = 1e9f;
		for (int k = 0; k < K; k++)
		{
			int *phi = Sampling1S_Count(N, m);
			if (phi == NULL) continue;
			float mu = TestCurveletMiu1D(N, phi, "EigenReal");
			delete[] phi;
			if (mu < best) best = mu;
		}
		out.mu_rand_best = (best < 1e9f) ? best : 0.0f;
	}

	delete[] phi0;
	return true;
}

// =============================================================================
// 2026-10-21 新增：带空间约束的模拟退火（任务书问题 2）
// =============================================================================

static int cons_max_gap(const std::vector<int>& idx, int N)
{
    if (idx.empty()) return N;
    int g = 0;
    for (size_t k = 1; k < idx.size(); k++)
    {
        int d = idx[k] - idx[k - 1];
        if (d > g) g = d;
    }
    if (idx.front() > g) g = idx.front();
    if (N - 1 - idx.back() > g) g = N - 1 - idx.back();
    return g;
}

static float cons_nn_cv(const std::vector<int>& idx)
{
    const int n = (int)idx.size();
    if (n < 3) return 0.0f;
    std::vector<double> d;
    d.reserve(n);
    for (int k = 0; k < n; k++)
    {
        int left  = (k > 0)     ? idx[k] - idx[k - 1] : (idx[1] - idx[0]);
        int right = (k + 1 < n) ? idx[k + 1] - idx[k] : (idx[n - 1] - idx[n - 2]);
        d.push_back((double)((left < right) ? left : right));
    }
    double mean = 0.0;
    for (size_t i = 0; i < d.size(); i++) mean += d[i];
    mean /= (double)d.size();
    if (mean <= 1e-9) return 0.0f;
    double var = 0.0;
    for (size_t i = 0; i < d.size(); i++) { double t = d[i] - mean; var += t * t; }
    var /= (double)d.size();
    return (float)(sqrt(var) / mean);
}

static float cons_unit_occupancy(const std::vector<int>& idx, int N, int m)
{
    if (m <= 0 || m >= N) return 1.0f;
    const double alpha = (double)N / (double)m;
    std::vector<int> cnt((size_t)m, 0);
    for (size_t k = 0; k < idx.size(); k++)
    {
        int u = (int)((double)idx[k] / alpha);
        if (u < 0) u = 0;
        if (u >= m) u = m - 1;
        cnt[(size_t)u]++;
    }
    int ok = 0;
    for (int u = 0; u < m; u++) if (cnt[(size_t)u] == 1) ok++;
    return (float)ok / (float)m;
}

// J = mu + λ_gap·P_gap + λ_nn·P_nn
static double cons_objective(float mu, int maxGap, float nnCv, int N, int m,
                             const ConstraintWeights& w)
{
    const double idealGap = (m > 0) ? (double)N / (double)m : (double)N;
    double pGap = (double)maxGap / idealGap - 1.0;
    if (pGap < 0.0) pGap = 0.0;
    double pNn = (double)nnCv;
    return (double)mu + w.lambdaGap * pGap + w.lambdaNN * pNn;
}

bool RunConstrainedSA(int N, int m, const std::vector<int>& initMask,
                      const ConstraintWeights& w, unsigned long long seed,
                      int maxSteps, ConstrainedResult& out)
{
    out = ConstrainedResult();
    if (N <= 0 || m <= 0 || m >= N || (int)initMask.size() != N) return false;

    exprng::seed(seed);

    // 当前解与最优解
    std::vector<int> cur = initMask;
    std::vector<int> best = initMask;
    std::vector<int> idxCur, idxBest;
    for (int i = 0; i < N; i++) if (cur[i] == 1) idxCur.push_back(i);
    idxBest = idxCur;

    float muCur  = SamplingMutualCoherence1D(idxCur, N);
    int   gapCur = cons_max_gap(idxCur, N);
    float cvCur  = cons_nn_cv(idxCur);
    double objCur = cons_objective(muCur, gapCur, cvCur, N, m, w);

    float muBest = muCur;
    int   gapBest = gapCur;
    float cvBest = cvCur;
    double objBest = objCur;
    long evals = 1;

    // 温度设定（与主流程一致的两段降温）
    const double T0 = 0.5, Tmin = 0.005, alpha = 0.92;
    double T = T0;
    const int nRapid = 8;

    std::vector<int> ones, zeros;
    int t = 0;
    while (T > Tmin && t < maxSteps)
    {
        // 单点交换邻域（与主流程一致）
        ones.clear(); zeros.clear();
        for (int i = 0; i < N; i++) { if (cur[i] == 1) ones.push_back(i); else zeros.push_back(i); }
        if (ones.empty() || zeros.empty()) break;
        int a = ones[exprng::integer() % ones.size()];
        int b = zeros[exprng::integer() % zeros.size()];

        std::vector<int> cand = cur;
        cand[a] = 0; cand[b] = 1;
        std::vector<int> idxCand;
        for (int i = 0; i < N; i++) if (cand[i] == 1) idxCand.push_back(i);

        const int   gapCand = cons_max_gap(idxCand, N);
        // 硬约束：不满足则直接拒绝（方案 D）
        if (w.hardMaxGap > 0 && gapCand > w.hardMaxGap)
        {
            t++;
            if (t < nRapid) T = T0 * exp(-alpha * sqrt((double)t));
            else T = T * alpha;
            continue;
        }

        const float muCand = SamplingMutualCoherence1D(idxCand, N);
        evals++;
        const float cvCand = cons_nn_cv(idxCand);
        const double objCand = cons_objective(muCand, gapCand, cvCand, N, m, w);

        const double delta = objCand - objCur;
        bool accept = false;
        if (delta <= 0.0) accept = true;
        else
        {
            double effT = T;
            if (effT < 1e-12) effT = 1e-12;
            double expo = -delta / effT;
            if (expo < -50.0) expo = -50.0;
            if (exp(-expo * -1.0) >= exprng::uniform()) accept = true;
        }
        if (accept)
        {
            cur = cand; idxCur = idxCand;
            muCur = muCand; gapCur = gapCand; cvCur = cvCand; objCur = objCand;
            if (objCur < objBest)
            {
                best = cur; idxBest = idxCur;
                muBest = muCur; gapBest = gapCur; cvBest = cvCur; objBest = objCur;
            }
        }

        t++;
        if (t < nRapid) T = T0 * exp(-alpha * sqrt((double)t));
        else T = T * alpha;
    }

    out.mu = muBest;
    out.objective = (float)objBest;
    out.maxGap = gapBest;
    out.nnCv = cvBest;
    out.unitOccupancy = cons_unit_occupancy(idxBest, N, m);
    out.evals = evals;
    out.hardConstraintSatisfied = (w.hardMaxGap <= 0) || (gapBest <= w.hardMaxGap);
    return true;
}

// =============================================================================
// 2026-10-21 新增：多种子等预算消融（任务书问题 3）
// =============================================================================
bool RunMultiSeedAblation(int N, float c, int nSeeds, unsigned long long baseSeed,
                          std::vector<AblationRow>& outRows)
{
    outRows.clear();
    if (N <= 0 || nSeeds <= 0) return false;
    const int m = (int)floor((double)N * (double)c + 0.5);
    if (m < 2 || m >= N) return false;

    for (int k = 0; k < nSeeds; k++)
    {
        AblationRow row;
        row.seedIndex = k;
        row.seed = baseSeed + (unsigned long long)(k * 179);

        // 共享初始解
        exprng::seed(row.seed);
        int *jm = Sampling1J_Count(N, m);
        std::vector<int> initMask(jm, jm + N);
        delete[] jm;
        std::vector<int> idx0;
        for (int i = 0; i < N; i++) if (initMask[i] == 1) idx0.push_back(i);
        row.muJitterInit = SamplingMutualCoherence1D(idx0, N);

        // ② SA-only（cg_max_iter=0）
        {
            long e0 = miu_eval_count();
            exprng::seed(row.seed + 1);
            int *in = new int[N];
            for (int i = 0; i < N; i++) in[i] = initMask[i];
            SAStats st; SAParams sp;
            sp.T0 = 80.0; sp.Tmin = 0.5; sp.alpha = 0.85; sp.n_rapid = 8; sp.iter = 1;
            sp.strategy = SA_ACCEPT_METROPOLIS;
            int *outm = UnOptiMiuSAEx(N, c, in, "J", true, &st, 0, sp);
            std::vector<int> idxo;
            for (int i = 0; i < N; i++) if (outm[i] == 1) idxo.push_back(i);
            row.muSaOnly = SamplingMutualCoherence1D(idxo, N);
            row.evalsSaOnly = miu_eval_count() - e0;
            delete[] outm; delete[] in;
        }
        // ③ 局部搜索-only
        {
            long e0 = miu_eval_count();
            int *work = new int[N];
            for (int i = 0; i < N; i++) work[i] = initMask[i];
            OptiMiuCG(N, work, 2);
            std::vector<int> idxo;
            for (int i = 0; i < N; i++) if (work[i] == 1) idxo.push_back(i);
            row.muLocalOnly = SamplingMutualCoherence1D(idxo, N);
            row.evalsLocalOnly = miu_eval_count() - e0;
            delete[] work;
        }
        // ④ SA+局部搜索
        {
            long e0 = miu_eval_count();
            exprng::seed(row.seed + 2);
            int *in = new int[N];
            for (int i = 0; i < N; i++) in[i] = initMask[i];
            SAStats st; SAParams sp;
            sp.T0 = 80.0; sp.Tmin = 0.5; sp.alpha = 0.85; sp.n_rapid = 8; sp.iter = 1;
            sp.strategy = SA_ACCEPT_METROPOLIS;
            int *outm = UnOptiMiuSAEx(N, c, in, "J", true, &st, 2, sp);
            std::vector<int> idxo;
            for (int i = 0; i < N; i++) if (outm[i] == 1) idxo.push_back(i);
            row.muSaLocal = SamplingMutualCoherence1D(idxo, N);
            row.maxGapSaLocal = cons_max_gap(idxo, N);
            row.nnCvSaLocal = cons_nn_cv(idxo);
            row.evalsSaLocal = miu_eval_count() - e0;
            delete[] outm; delete[] in;
        }
        // ⑤ 等预算随机重启（K = SA+局部搜索的评估次数）
        {
            int K = (int)row.evalsSaLocal;
            if (K < 1) K = 1;
            if (K > 1000) K = 1000;
            float best = 1e9f;
            long ev = 0;
            for (int t = 0; t < K; t++)
            {
                exprng::seed(row.seed + 1000ULL + (unsigned long long)t);
                int *rm = Sampling1J_Count(N, m);
                std::vector<int> idxr;
                for (int i = 0; i < N; i++) if (rm[i] == 1) idxr.push_back(i);
                delete[] rm;
                float mu = SamplingMutualCoherence1D(idxr, N);
                ev++;
                if (mu < best) best = mu;
            }
            row.muRestartBest = best;
            row.evalsRestart = ev;
        }
        // ⑥ 带空间约束的混合（λ=0.3 软惩罚）
        {
            ConstraintWeights w;
            w.lambdaGap = 0.3; w.lambdaNN = 0.3; w.hardMaxGap = 0;
            ConstrainedResult cr;
            if (RunConstrainedSA(N, m, initMask, w, row.seed + 3, 60, cr))
            {
                row.muConstrained = cr.mu;
                row.maxGapConstrained = cr.maxGap;
                row.nnCvConstrained = cr.nnCv;
                row.evalsConstrained = cr.evals;
            }
        }
        outRows.push_back(row);
    }
    return true;
}

