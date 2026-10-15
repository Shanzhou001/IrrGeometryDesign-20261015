// // add by shanzhou in 2026/10/15
/*****************************************************
 * EXPRNG.CPP
 * xorshift64* 实现，见 ExpRng.h 说明
 *****************************************************/
#include "ExpRng.h"
#include <time.h>

namespace {

unsigned long long g_state = 0;      // 0 表示尚未播种
unsigned long long g_saved = 0;

inline unsigned long long next_u64()
{
	// xorshift64* (Vigna, 2014)
	unsigned long long x = g_state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	g_state = x;
	return x * 2685821657736338717ULL;
}

} // anonymous namespace

namespace exprng {

void seed(unsigned long long s)
{
	// 避免种子 0（xorshift 不动点）
	g_state = s == 0 ? 0x9E3779B97F4A7C15ULL : s;
	// 预热，消除弱种子的相关性
	for (int i = 0; i < 8; i++) next_u64();
	g_saved = g_state;
}

void seed_auto()
{
	seed((unsigned long long)time(NULL) ^ 0x2545F4914F6CDD1DULL);
}

bool seeded()
{
	return g_state != 0;
}

unsigned long integer()
{
	if (g_state == 0) seed_auto();
	// 取低 31 位，模拟 rand() 的取值范围
	return (unsigned long)(next_u64() & 0x7FFFFFFFULL);
}

double uniform()
{
	if (g_state == 0) seed_auto();
	// 53 位双精度，[0,1)
	return (double)(next_u64() >> 11) * (1.0 / 9007199254740992.0);
}

} // namespace exprng
