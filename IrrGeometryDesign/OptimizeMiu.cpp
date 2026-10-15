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
	int nm = (int)ones.size() / 20;
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



	// 混合优化策略：CG 局部精修（若 cg_max_iter <= 0 则跳过）

	clock_t cg_begin = clock();

	int *result = best;

	if (cg_max_iter > 0)

	{

		result = OptiMiuCG(N, best, cg_max_iter);

	}

	if (stats)

	{

		stats->cg_time = (double)(clock() - cg_begin) / CLOCKS_PER_SEC;

		stats->miu_final = TestCurveletMiu1D(N, result, "EigenReal");

	}



	if (record != NULL) delete[] record;

	if (cur != NULL) delete[] cur;



	return result;

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
