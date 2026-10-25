// // add by wushanzhou in 2026/10/15
/*****************************************************
 * EXPRNG.H
 * 统一伪随机数源（xorshift64*），解决以下实验有效性问题：
 *  1) 原实现每次调用 Sampling1J/1F/2J/2F 都执行 srand(time(NULL))，
 *     导致模拟退火高频循环内一秒内多次调用时生成完全相同的候选解；
 *  2) 全局 rand() 状态不可控，实验不可复现。
 * 用法：
 *   - 命令行实验：每个实验配置开始前显式调用 exprng::seed(seed)，
 *     保证同 seed 结果完全一致；
 *   - GUI 交互：不调用 seed() 时首次取数会按系统时间自动播种一次，
 *     行为与原程序一致。
 * 单线程使用（现有算法均为单线程）。
 *****************************************************/
#ifndef _EXPRNG_H_
#define _EXPRNG_H_

namespace exprng {

// 显式播种（之后的所有取数序列确定）
void seed(unsigned long long s);

// 按系统时间播种一次（仅当从未播种过时才会自动调用）
void seed_auto();

// 是否已经播种过
bool seeded();

// 返回 [0, 2147483647] 区间整数，语义与 rand() 对齐
unsigned long integer();

// 返回 [0,1) 双精度均匀随机数（53 位精度）
double uniform();

// 返回满足 N(mean, stddev^2) 的正态分布高斯随机数 (Box-Muller) // add by wushanzhou in 2026/10/15
double gaussian(double mean = 0.0, double stddev = 1.0);

// 获取最近一次显式播种的种子值
unsigned long long current_seed();

} // namespace exprng

#endif // _EXPRNG_H_
