/***************************************************
 * Name    : CalculateMiu.cpp
 * Function: Calculate Miu by Curvelet transform
 *			 根据互相干度参数的定义计算miu值, 无需矩阵相乘运算
 *           包含复数, 实数两种实现方式
 * 修复日期: 2026.10.15 // add by shanzhou in 2026/10/15
 * 修复内容(实验有效性与性能):
 *   1) CurveletMiu1DEigen 增加单位阵参考变换缓存：μ 的定义中参考项
 *      G4（单位阵经 FFT+曲波变换后的归一化 Gram）只依赖 N，与采样
 *      矩阵无关，原实现每次评估都重复计算（约占总耗时 1/2），现按
 *      N 缓存，结果与原实现逐位一致；
 *   2) CurveletMiu1DComplex/Real 的列循环硬编码 64，当最粗尺度系数
 *      块小于 64 时越界读内存——改为 min(64, col)（这两个函数当前
 *      未被主流程调用，保留原实现并仅修复越界）；
 *   3) CalculateAveMiu 增加 k==0 保护（除零）。
 ***************************************************/

#include "CalculateMiu.h"
#include "sample.h"

//------------ Calculate Curvelet Miu 1D ------------//
float CurveletMiu1DComplex(vector<int> idx, int N)
{
	/// 初始化数组 ///
	int **I = (int**)malloc(sizeof(int*) * N);
	int **Phi = (int**)malloc(sizeof(int*) * N);
	for (int i = 0; i < N; i++)
	{
		I[i] = (int*)malloc(sizeof(int) * N);
		Phi[i] = (int*)malloc(sizeof(int) * N);
	}
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			I[i][j] = 0;
			Phi[i][j] = 0;
		}
	}

	/// 生成采样矩阵 ///
	for (int i = 0; i < N; i++)
		I[i][i] = 1;
	int m = (int)idx.size();
	for (int i = 0; i < m; i++)
		Phi[idx[i]][idx[i]] = 1;

	/// 傅里叶变换 ///
	CpxNumMat FH(N, N);
	CpxNumMat X(N, N);
	FFT1D_fftshift(I, FH, N, N);  //单位矩阵
	FFT1D_fftshift(Phi, X, N, N); //采样矩阵
	for (int i = 0; i < N; i++)
	{
		free(I[i]); free(Phi[i]);
	}
	free(I); free(Phi);

	/// 曲波变换 ///
	vector< vector<CpxNumMat> > C;
	vector< vector<CpxNumMat> > CI;
	fdct_wrapping(N, N, 1, 4, 1, X, C);   //C 为采样矩阵
	fdct_wrapping(N, N, 1, 4, 1, FH, CI); //CI为单位矩阵
	clear(X); clear(FH); X.~NumMat(); FH.~NumMat();

	/// 互相干度参数μ公式 ///
	int col = C[0][0].m();
	int ncol = (col < 64) ? col : 64; //修复：原硬编码64在 col<64 时越界
	float **miuMatA = (float**)malloc(sizeof(float*) * col);
	float **miuMatAI = (float**)malloc(sizeof(float*) * col);
	complex<float> **A = (complex<float>**)malloc(sizeof(complex<float>*) * col);
	complex<float> **AI = (complex<float>**)malloc(sizeof(complex<float>*) * col);
	for (int i = 0; i < col; i++)
	{
		miuMatA[i] = (float*)malloc(sizeof(float) * col);
		miuMatAI[i] = (float*)malloc(sizeof(float) * col);
		A[i] = (complex<float>*)malloc(sizeof(complex<float>) * col);
		AI[i] = (complex<float>*)malloc(sizeof(complex<float>) * col);
	}
	for (int i = 0; i < col; i++)
	{
		for (int j = 0; j < col; j++)
		{
			A[i][j].real(C[0][0](i,j).real());
			A[i][j].imag(C[0][0](i,j).imag());//虚部对结果也有影响
			AI[i][j].real(CI[0][0](i,j).real());
			AI[i][j].imag(CI[0][0](i,j).imag());
		}
	}
	for (int i = 0; i < (int)C.size(); i++)
	{
		C[i].clear(); CI[i].clear();
	}
	C.clear(); CI.clear(); //释放

	complex<float> InnerPro(0,0);
	float InnerProAbs, Sum_ic, Sum_j, Norm2_ic, Norm2_j;

	for (int i = 0; i < ncol; i++) // 列循环 列为64和列为N的结果相差很小
	{
		for (int j = 0; j < col; j++)
		{
			//采样矩阵归一化miu值
			InnerPro.real(0); InnerPro.imag(0);
			InnerProAbs = 0;
			Sum_ic = 0;   Sum_j = 0;
			Norm2_ic = 0; Norm2_j = 0;
			for (int k = 0; k < col; k++) //行循环
			{
				InnerPro += conj(A[k][i]) * A[k][j];//求列向量的内积
				Sum_ic += abs(A[k][i]) * abs(A[k][i]);//求列向量L2范数
				Sum_j += abs(A[k][j]) * abs(A[k][j]);
			}
			InnerProAbs = abs(InnerPro);
			Norm2_ic = sqrt(Sum_ic);
			Norm2_j = sqrt(Sum_j);
			miuMatA[j][i] = InnerProAbs / (Norm2_ic * Norm2_j);

			//单位矩阵归一化miu值
			InnerPro.real(0); InnerPro.imag(0);
			InnerProAbs = 0;
			Sum_ic = 0;   Sum_j = 0;
			Norm2_ic = 0; Norm2_j = 0;
			for (int k = 0; k < col; k++)
			{
				InnerPro += conj(AI[k][i]) * AI[k][j];
				Sum_ic += abs(AI[k][i]) * abs(AI[k][i]);
				Sum_j += abs(AI[k][j]) * abs(AI[k][j]);
			}
			InnerProAbs = abs(InnerPro);
			Norm2_ic = sqrt(Sum_ic);
			Norm2_j = sqrt(Sum_j);
			miuMatAI[j][i] = InnerProAbs / (Norm2_ic * Norm2_j);
		}
	}

	//释放内存
	for (int i = 0; i < col; i++)
	{
		free(A[i]); free(AI[i]);
		A[i] = NULL; AI[i] = NULL;
	}
	free(A); free(AI);

	// 计算Miu //
	float Miu = 0.0;
	// 2026.10.20 修复：上方列循环只填了 i < ncol（= min(col,64)）的列，
	// 原实现却在此扫描 i < col，col>64 时会读入从未写入的元素（未初始化内存）。
	for (int i = 0; i < ncol; i++)
	{
		for (int j = 0; j < col; j++)
		{
			float temp = (float)abs(miuMatA[i][j] - miuMatAI[i][j]);
			if (temp >= Miu)
				Miu = temp;
		}
	}
	for (int i = 0; i < col; i++)
	{
		free(miuMatA[i]); free(miuMatAI[i]);
	}
	free(miuMatA); free(miuMatAI);

	//返回μ值
	return Miu;
}

float CurveletMiu1DReal(vector<int> idx, int N)
{
	/// 初始化数组 ///
	int **I = (int**)malloc(sizeof(int*) * N);
	int **Phi = (int**)malloc(sizeof(int*) * N);
	for (int i = 0; i < N; i++)
	{
		I[i] = (int*)malloc(sizeof(int) * N);
		Phi[i] = (int*)malloc(sizeof(int) * N);
	}
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			I[i][j] = 0;
			Phi[i][j] = 0;
		}
	}

	/// 生成采样矩阵 ///
	for (int i = 0; i < N; i++)
		I[i][i] = 1;
	int m = (int)idx.size();
	for (int i = 0; i < m; i++)
		Phi[idx[i]][idx[i]] = 1;

	/// 傅里叶变换 ///
	CpxNumMat FH(N, N);
	CpxNumMat X(N, N);
	FFT1D_fftshift(I, FH, N, N);  //单位矩阵
	FFT1D_fftshift(Phi, X, N, N); //采样矩阵
	for (int i = 0; i < N; i++)
	{
		free(I[i]); free(Phi[i]);
	}
	free(I); free(Phi);

	/// 曲波变换 ///
	vector< vector<CpxNumMat> > C;
	vector< vector<CpxNumMat> > CI;
	fdct_wrapping(N, N, 1, 4, 1, X, C);   //C 为采样矩阵
	fdct_wrapping(N, N, 1, 4, 1, FH, CI); //CI为单位矩阵
	clear(X); clear(FH); X.~NumMat(); FH.~NumMat();

	/// 互相干度参数μ公式 ///
	int col = C[0][0].m();
	int ncol = (col < 64) ? col : 64; //修复：原硬编码64在 col<64 时越界
	float **miuMatA = (float**)malloc(sizeof(float*) * col);
	float **miuMatAI = (float**)malloc(sizeof(float*) * col);
	float **A = (float**)malloc(sizeof(float*) * col);
	float **AI = (float**)malloc(sizeof(float*) * col);
	for (int i = 0; i < col; i++)
	{
		miuMatA[i] = (float*)malloc(sizeof(float) * col);
		miuMatAI[i] = (float*)malloc(sizeof(float) * col);
		A[i] = (float*)malloc(sizeof(float) * col); //只要实部
		AI[i] = (float*)malloc(sizeof(float) * col);
	}
	for (int i = 0; i < col; i++)
	{
		for (int j = 0; j < col; j++)
		{
			A[i][j] = C[0][0](i,j).real();
			AI[i][j] = CI[0][0](i,j).real();
		}
	}
	for (int i = 0; i < (int)C.size(); i++)
	{
		C[i].clear(); CI[i].clear();
	}
	C.clear(); CI.clear();

	float InnerPro;
	float InnerProAbs, Sum_ic, Sum_j, Norm2_ic, Norm2_j;
	for (int i = 0; i < ncol; i++) // 列循环 列为64和列为N的结果相差很小
	{
		for (int j = 0; j < col; j++)
		{
			//采样矩阵归一化miu值
			InnerPro = 0;
			InnerProAbs = 0;
			Sum_ic = 0;   Sum_j = 0;
			Norm2_ic = 0; Norm2_j = 0;
			for (int k = 0; k < col; k++) //行循环
			{
				InnerPro += A[k][i] * A[k][j];//求列向量的内积
				Sum_ic += abs(A[k][i]) * abs(A[k][i]);//求列向量L2范数
				Sum_j += abs(A[k][j]) * abs(A[k][j]);
			}
			InnerProAbs = abs(InnerPro);
			Norm2_ic = sqrt(Sum_ic);
			Norm2_j = sqrt(Sum_j);
			miuMatA[j][i] = InnerProAbs / (Norm2_ic * Norm2_j);

			//单位矩阵归一化miu值
			InnerPro = 0;
			InnerProAbs = 0;
			Sum_ic = 0;   Sum_j = 0;
			Norm2_ic = 0; Norm2_j = 0;
			for (int k = 0; k < col; k++)
			{
				InnerPro += AI[k][i] * AI[k][j];
				Sum_ic += abs(AI[k][i]) * abs(AI[k][i]);
				Sum_j += abs(AI[k][j]) * abs(AI[k][j]);
			}
			InnerProAbs = abs(InnerPro);
			Norm2_ic = sqrt(Sum_ic);
			Norm2_j = sqrt(Sum_j);
			miuMatAI[j][i] = InnerProAbs / (Norm2_ic * Norm2_j);
		}
	}

	//释放内存
	for (int i = 0; i < col; i++)
	{
		free(A[i]); free(AI[i]);
	}
	free(A); free(AI);

	// 计算Miu //
	float Miu = 0.0;
	// 2026.10.20 修复：上方列循环只填了 i < ncol（= min(col,64)）的列，
	// 原实现却在此扫描 i < col，col>64 时会读入从未写入的元素（未初始化内存）。
	for (int i = 0; i < ncol; i++)
	{
		for (int j = 0; j < col; j++)
		{
			float temp = (float)abs(miuMatA[i][j] - miuMatAI[i][j]);
			if (temp >= Miu)
				Miu = temp;
		}
	}
	for (int i = 0; i < col; i++)
	{
		free(miuMatA[i]); free(miuMatAI[i]);
	}
	free(miuMatA); free(miuMatAI);

	// 返回μ值
	return Miu;
}

//输入为一维矩阵,计算μ值//
float TestCurveletMiu1D(int row, int *Phi, string calctype)//读取一维矩阵下标，计算μ值
{
	vector<int> idx;
	// 2026.10.20 修复：原 tempMiu 未初始化，calctype 拼写错误时返回垃圾值
	float tempMiu = 0.0f;
		//行数，即测线
		for (int j = 0; j < row; j++)
		{
			if (Phi[j] == 1)
			{
				idx.push_back(j);
			}
		}
		if (calctype == "EigenCpx")//复数曲波变换
		{
			tempMiu = CurveletMiu1DEigen(idx, row, 0);
		}
		if (calctype == "EigenReal")//实数曲波变换
		{
			tempMiu = CurveletMiu1DEigen(idx, row, 1);
		}
		if (calctype != "EigenCpx" && calctype != "EigenReal")
		{
			// 2026.10.20: 未知 calctype 不再静默返回 0，打印告警便于定位调用错误
			printf("[WARN] TestCurveletMiu1D: unknown calctype \"%s\", returning 0\n", calctype.c_str());
			tempMiu = 0.0f;
		}
		idx.clear();
		vector<int>().swap(idx);
		return tempMiu;

}

//------------ Calculate Curvelet Average Miu 2D ------------//
float CurveletAveMiu2D(int row, int col, int **Phi, string calctype)
{
	if ((calctype != "CCpx") && (calctype != "CReal") && (calctype != "EigenCpx") && (calctype != "EigenReal"))
	{
		printf("  计算类型出错，请检查！\n");
		return 0;
	}

	vector<int> idx;
	float Sum = 0.0, tempMiu;
	//clock_t start, finish;
	for (int i = 0; i < row; i++) //行数，即测线
	{
		for (int j = 0; j < col; j++)
		{
			if (Phi[i][j] == 1)
			{
				idx.push_back(j);
			}
		}
		// 利用CurveletMiu1D计算μ值
		if (calctype == "CCpx")
		{
			tempMiu = CurveletMiu1DComplex(idx, col);
			Sum += tempMiu;
		}
		if (calctype == "CReal")
		{
			tempMiu = CurveletMiu1DReal(idx, col);
			Sum += tempMiu;
		}
		if (calctype == "EigenCpx")
		{
			tempMiu = CurveletMiu1DEigen(idx, col, 0);
			Sum += tempMiu;
		}
		if (calctype == "EigenReal")
		{
			tempMiu = CurveletMiu1DEigen(idx, col, 1);
			Sum += tempMiu;
		}
		idx.clear();
	}
	vector<int>().swap(idx);
	float Miu = Sum / row;
	return Miu;
}

//------ 复数二维数组的一维傅里叶变换(零频点在中心) Based on FFTW_2.1.5 -------//
void FFT1D_fftshift(int **inMat, CpxNumMat& bMat, int Nx, int Nt)
{
	/*----------------------------------*
	 * 输入: inMat
	 * 输出: bMat
	 *----------------------------------*/

	fftw_complex *in, *out;
	fftw_plan p;
	in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * Nt);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* Nt);
	p = fftw_create_plan(Nt, FFTW_FORWARD, FFTW_ESTIMATE);

	/// fft1D
	CpxNumMat aMat(Nx, Nt); clear(aMat);
	for (int i = 0; i < Nx; i++)
	{
		for (int k = 0; k < Nt; k++)
		{
			in[k].re = inMat[i][k];
			in[k].im = 0;
		}
		fftw_one(p, in, out);
		for (int k = 0; k < Nt; k++)
			aMat(i,k) = cpx(out[k].re, out[k].im);
	}

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);

	// 实现fftshift
	int X0, Y0;
	if (Nx % 2 == 0)	X0 = Nx/2 + 1;
	else        		X0 = (Nx + 1)/2;
	if (Nt % 2 == 0)    Y0 = Nt/2 + 1;
	else         		Y0 = (Nt + 1)/2;

	// Nx为偶数 Nt为偶数
	if (Nx%2 == 0 && Nt%2 == 0)
	{
		for (int i = 0; i < X0-1; i++)
		{
			for (int j = 0; j < Y0-1; j++)
			{
				bMat(i,j) = aMat(i+X0-1,j+Y0-1);
				bMat(i+X0-1,j) = aMat(i,j+Y0-1);
				bMat(i,j+Y0-1) = aMat(i+X0-1,j);
				bMat(i+X0-1,j+Y0-1) = aMat(i,j);
			}
		}
	}

	// Nx为奇数 Nt为奇数
	if (Nx%2 !=0 && Nt%2 !=0)
	{
		for (int i=0; i<X0; i++)
		{
			for (int j=0; j<Y0; j++)
			{
				bMat(i+X0-1,j+Y0-1) = aMat(i,j);
			}
		}
		for (int i=X0; i<Nx; i++)
		{
			for (int j=0; j<Y0; j++)
			{
				bMat(i-X0,j+Y0-1) = aMat(i,j);
			}
		}
		for (int i=0; i<X0; i++)
		{
			for (int j=Y0; j<Nx; j++)
			{
				bMat(i+X0-1,j-Y0) = aMat(i,j);
			}
		}
		for (int i=X0; i<Nx; i++)
		{
			for (int j=Y0; j<Nx; j++)
			{
				bMat(i-X0,j-Y0) = aMat(i,j);
			}
		}
	}
	clear(aMat); aMat.~NumMat();
}

//==================== 单位阵参考变换缓存 ====================
// μ 定义中的参考项只依赖 (N, 曲波参数)：单位阵 I --FFT--> 曲波变换 --> 归一化 Gram G4。
// 原实现在每次 μ 评估中重复计算；现按 (N, nbscales, nbangles) 缓存，跨调用复用。
// 仅 CurveletMiu1DEigen（主流程）使用；ac 固定为 1。
struct MiuRefEntry // add by shanzhou in 2026/10/15
{
	int N;
	int sc;   //nbscales
	int an;   //nbangles_coarse
	int col;
	MatrixXf  G4r;  bool ready_r; //实数参考Gram
	MatrixXcf G4c;  bool ready_c; //复数参考Gram
	MiuRefEntry() : N(0), sc(0), an(0), col(0), ready_r(false), ready_c(false) {}
};
static vector<MiuRefEntry> g_miu_refs;
static const int MIU_REF_CACHE_MAX = 8;

static MiuRefEntry& miu_ref_for(int N, int col, int sc, int an)
{
	for (size_t i = 0; i < g_miu_refs.size(); i++)
	{
		if (g_miu_refs[i].N == N && g_miu_refs[i].sc == sc && g_miu_refs[i].an == an)
			return g_miu_refs[i];
	}
	MiuRefEntry e;
	e.N = N; e.sc = sc; e.an = an; e.col = col;
	if ((int)g_miu_refs.size() >= MIU_REF_CACHE_MAX)
		g_miu_refs.erase(g_miu_refs.begin()); //超出上限时淘汰最早的一项
	g_miu_refs.push_back(e);
	return g_miu_refs.back();
}

// 构建参考项（datatype: 0复数 / 1实数），结果写入缓存
static bool miu_ref_build(int N, int col, int datatype, int sc, int an)
{
	MiuRefEntry& e = miu_ref_for(N, col, sc, an);
	if (datatype == 0 && e.ready_c) return true;
	if (datatype == 1 && e.ready_r) return true;

	/// 单位阵 ///
	int **I = (int**)malloc(sizeof(int*) * N);
	for (int i = 0; i < N; i++)
	{
		I[i] = (int*)malloc(sizeof(int) * N);
		for (int j = 0; j < N; j++) I[i][j] = 0;
	}
	for (int i = 0; i < N; i++) I[i][i] = 1;

	/// 傅里叶变换 ///
	CpxNumMat FH(N, N);
	FFT1D_fftshift(I, FH, N, N);
	for (int i = 0; i < N; i++)
	{
		free(I[i]);
		I[i] = NULL;
	}
	free(I);

	/// 曲波变换（参数化）///
	vector< vector<CpxNumMat> > CI;
	fdct_wrapping(N, N, sc, an, 1, FH, CI);
	clear(FH); FH.~NumMat();

	if ((int)CI.size() == 0 || (int)CI[0].size() == 0 || CI[0][0].m() != col)
	{
		// 参数或库版本异常，放弃缓存（调用方回退为本地计算）
		for (int i = 0; i < (int)CI.size(); i++) CI[i].clear();
		CI.clear();
		return false;
	}
	if (datatype == 0) //复数
	{
		MatrixXcf CIMat(col, col);  CIMat.setZero();
		MatrixXcf G2(col, col);     G2.setZero();
		MatrixXcf G4(col, col);     G4.setZero();
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				CIMat(i,j) = CI[0][0](i,j);
			}
		}
		G2 = CIMat.adjoint() * CIMat;
		CIMat.resize(0,0);
		float *G2_diag = (float*) malloc(sizeof(float) * col);
		for (int i = 0; i < col; i++)
		{
			G2_diag[i] = 1/sqrt(G2(i,i).real());
		}
		for (int j = 0; j < col; j++)
		{
			for (int i = 0; i < col; i++)
			{
				G4(i,j) = G2_diag[j]*G2(i,j);
			}
		}
		G2.resize(0,0);
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				G4(i,j) = G2_diag[i]*G4(i,j);
			}
		}
		free(G2_diag);
		e.G4c = G4;
		e.ready_c = true;
	}
	else //实数
	{
		MatrixXf CIMat(col, col);  CIMat.setZero();
		MatrixXf G2(col, col);     G2.setZero();
		MatrixXf G4(col, col);     G4.setZero();
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				CIMat(i,j) = CI[0][0](i,j).real();
			}
		}
		G2 = CIMat.adjoint() * CIMat;
		CIMat.resize(0,0);
		float *G2_diag = (float*) malloc(sizeof(float) * col);
		for (int i = 0; i < col; i++)
		{
			G2_diag[i] = 1/sqrt(G2(i,i));
		}
		for (int j = 0; j < col; j++)
		{
			for (int i = 0; i < col; i++)
			{
				G4(i,j) = G2_diag[j]*G2(i,j);
			}
		}
		G2.resize(0,0);
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				G4(i,j) = G2_diag[i]*G4(i,j);
			}
		}
		free(G2_diag);
		e.G4r = G4;
		e.ready_r = true;
	}

	for (int i = 0; i < (int)CI.size(); i++) CI[i].clear();
	CI.clear();
	return true;
}

//==================== μ 评估计数器 ====================
// 统计整个进程内 μ 的实际计算次数，用于优化算法效率对比
// （例如 SA+CG 与纯 SA 在相同 μ 评估预算下的质量比较）。
static long g_miu_evals = 0;
void miu_eval_reset() { g_miu_evals = 0; }
long miu_eval_count() { return g_miu_evals; }

//------------ Calculate Curvelet Miu 1D (Eigen 加速版, 主流程) ------------//
float CurveletMiu1DEigen(vector<int> idx, int N, int datatype) // add by shanzhou in 2026/10/15
{
	//默认曲波参数 (nbscales=1, nbangles=4)：与项目历史实现一致（GUI 数值不变）
	return CurveletMiu1DEigenParam(idx, N, datatype, 1, 4);
}

//------------ 参数化版本：曲波 (nbscales, nbangles_coarse) 可配置 ------------//
// 用途：μ 定义的变换参数鲁棒性实验（不同尺度/角度参数下 μ 排序是否稳定）
float CurveletMiu1DEigenParam(vector<int> idx, int N, int datatype, int nbscales, int nbangles)
{
	g_miu_evals++;
	/// 初始化采样矩阵 ///
	int **Phi = (int**)malloc(sizeof(int*) * N);
	for (int i = 0; i < N; i++)
	{
		Phi[i] = (int*)malloc(sizeof(int) * N);
		for (int j = 0; j < N; j++) Phi[i][j] = 0;
	}
	int m = (int)idx.size();
	for (int i = 0; i < m; i++)
		Phi[idx[i]][idx[i]] = 1;

	/// 傅里叶变换（只计算采样矩阵一侧，参考侧走缓存）///
	CpxNumMat X(N, N);
	FFT1D_fftshift(Phi, X, N, N);
	for (int i = 0; i < N; i++)
	{
		free(Phi[i]);
		Phi[i] = NULL;
	}
	free(Phi);

	/// 曲波变换（参数化）///
	vector< vector<CpxNumMat> > C;
	fdct_wrapping(N, N, nbscales, nbangles, 1, X, C);   //C 为采样矩阵
	clear(X); X.~NumMat();

	// ---------------------------------------------------------------------
	// 2026.10.20 修复（取代 2026.10.19 的做法）：多尺度 μ 真正随参数变化
	//
	//   原做法：把全部系数块经 ifdct_wrapping 重建回空间域再求 Gram。
	//   但 ifdct_wrapping 是 fdct_wrapping 的精确逆，重建结果恒等于输入，
	//   (nbscales, nbangles) 被完全约掉 —— 实测多组参数结果完全相同，
	//   参数鲁棒性实验因此没有意义。
	//
	//   现做法：直接在各个系数块上求 Gram。
	//     A_sw = C[s][w] 为该块感知矩阵；G = A^H A 按对角元归一化；
	//     单位阵按同一变换块化得参考块，同样归一化；
	//     μ = max_{s,w} max|G - G_ref|。
	//   这样 μ 会随块划分（即 nbscales/nbangles）变化。
	//   nbscales==1 时只有一块，与原单块路径等价，主流程数值不变。
	// ---------------------------------------------------------------------
	{
		bool multiBlock = ((int)C.size() > 1) || ((int)C.size() == 1 && (int)C[0].size() > 1);
		if (multiBlock)
		{
			vector< vector<CpxNumMat> > CI;
			{
				int **I2 = (int**)malloc(sizeof(int*) * N);
				for (int i = 0; i < N; i++)
				{
					I2[i] = (int*)malloc(sizeof(int) * N);
					for (int k = 0; k < N; k++) I2[i][k] = 0;
				}
				for (int i = 0; i < N; i++) I2[i][i] = 1;
				CpxNumMat FH(N, N);
				FFT1D_fftshift(I2, FH, N, N);
				for (int i = 0; i < N; i++) { free(I2[i]); I2[i] = NULL; }
				free(I2);
				fdct_wrapping(N, N, nbscales, nbangles, 1, FH, CI);
				clear(FH); FH.~NumMat();
			}
			bool okAll = ((int)CI.size() == (int)C.size());
			float muMax = 0.0f;
			for (int s = 0; okAll && s < (int)C.size(); s++)
			{
				if ((int)CI[s].size() != (int)C[s].size()) { okAll = false; break; }
				for (int w = 0; okAll && w < (int)C[s].size(); w++)
				{
					const int d1 = C[s][w].m();
					const int d2 = C[s][w].n();
					if (d1 <= 0 || d2 <= 0 || d1 != CI[s][w].m() || d2 != CI[s][w].n()) { okAll = false; break; }
					MatrixXf A(d1, d2);
					MatrixXf AI(d1, d2);
					for (int p1 = 0; p1 < d1; p1++)
					{
						for (int p2 = 0; p2 < d2; p2++)
						{
							A(p1, p2)  = C[s][w](p1, p2).real();
							AI(p1, p2) = CI[s][w](p1, p2).real();
						}
					}
					MatrixXf Vs = A.adjoint() * A;
					MatrixXf Vr = AI.adjoint() * AI;
					const int nc = (int)Vs.rows();
					std::vector<float> ds((size_t)nc), dr((size_t)nc);
					bool okDiv = true;
					for (int k = 0; k < nc; k++)
					{
						float v1 = Vs(k, k);
						float v2 = Vr(k, k);
						if (v1 <= 1e-20f || v2 <= 1e-20f) { okDiv = false; break; }
						ds[k] = 1.0f / sqrt(v1);
						dr[k] = 1.0f / sqrt(v2);
					}
					if (!okDiv) { okAll = false; break; }
					float blockMax = 0.0f;
					for (int c1 = 0; c1 < nc; c1++)
					{
						for (int c2 = 0; c2 < nc; c2++)
						{
							float g1 = ds[c1] * Vs(c1, c2) * ds[c2];
							float g2 = dr[c1] * Vr(c1, c2) * dr[c2];
							float dif = fabsf(g1 - g2);
							if (dif > blockMax) blockMax = dif;
						}
					}
					if (blockMax > muMax) muMax = blockMax;
				}
			}
			for (int i = 0; i < (int)CI.size(); i++) CI[i].clear();
			CI.clear();
			if (okAll)
			{
				for (int i = 0; i < (int)C.size(); i++) C[i].clear();
				C.clear();
				return muMax;
			}
		}
	}

	int col = C[0][0].m();
	float Miu = 0;

	// 参考项 G4（优先走缓存；异常时回退为本地计算，结果不变）
	bool ref_ok = miu_ref_build(N, col, datatype, nbscales, nbangles);
	MiuRefEntry& e = miu_ref_for(N, col, nbscales, nbangles);


	if (datatype == 0) //复数参与计算
	{
		MatrixXcf CMat(col, col);  CMat.setZero();
		MatrixXcf G1(col, col);    G1.setZero();
		MatrixXcf G3(col, col);    G3.setZero();
		MatrixXcf G4(col, col);    G4.setZero();
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				CMat(i,j) = C[0][0](i,j);
			}
		}
		for (int i = 0; i < (int)C.size(); i++)
		{
			C[i].clear();
		}
		C.clear(); //释放

		// 复数感知矩阵相乘 //
		G1 = CMat.adjoint() * CMat;
		CMat.resize(0,0);

		// 列向量单位化 //
		float *G1_diag = (float*) malloc(sizeof(float) * col);
		for (int i = 0; i < col; i++)
		{
			G1_diag[i] = 1/sqrt(G1(i,i).real());
		}
		for (int j = 0; j < col; j++)
		{
			for (int i = 0; i < col; i++)
			{
				G3(i,j) = G1_diag[j]*G1(i,j);
			}
		}
		G1.resize(0,0);
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				G3(i,j) = G1_diag[i]*G3(i,j);
			}
		}
		free(G1_diag);

		// 计算Miu //
		if (ref_ok && e.ready_c)
		{
			Miu = (G3 - e.G4c).cwiseAbs().maxCoeff();//最大值
		}
		else
		{
			// 回退：本地计算参考项（与原实现一致）
			int **I = (int**)malloc(sizeof(int*) * N);
			for (int i = 0; i < N; i++)
			{
				I[i] = (int*)malloc(sizeof(int) * N);
				for (int j = 0; j < N; j++) I[i][j] = 0;
			}
			for (int i = 0; i < N; i++) I[i][i] = 1;
			CpxNumMat FH(N, N);
			FFT1D_fftshift(I, FH, N, N);
			for (int i = 0; i < N; i++) { free(I[i]); I[i] = NULL; }
			free(I);
			vector< vector<CpxNumMat> > CI;
			fdct_wrapping(N, N, 1, 4, 1, FH, CI);
			clear(FH); FH.~NumMat();
			MatrixXcf CIMat(col, col);  CIMat.setZero();
			MatrixXcf G2(col, col);     G2.setZero();
			for (int i = 0; i < col; i++)
				for (int j = 0; j < col; j++)
					CIMat(i,j) = CI[0][0](i,j);
			for (int i = 0; i < (int)CI.size(); i++) CI[i].clear();
			CI.clear();
			G2 = CIMat.adjoint() * CIMat;
			CIMat.resize(0,0);
			float *G2_diag = (float*) malloc(sizeof(float) * col);
			for (int i = 0; i < col; i++) G2_diag[i] = 1/sqrt(G2(i,i).real());
			for (int j = 0; j < col; j++)
				for (int i = 0; i < col; i++)
					G4(i,j) = G2_diag[j]*G2(i,j);
			G2.resize(0,0);
			for (int i = 0; i < col; i++)
				for (int j = 0; j < col; j++)
					G4(i,j) = G2_diag[i]*G4(i,j);
			free(G2_diag);
			Miu = (G3 - G4).cwiseAbs().maxCoeff();
		}
		G3.resize(0,0); G4.resize(0,0);
	}
	else//实数参与计算
	{
		MatrixXf CMat(col, col);  CMat.setZero();
		MatrixXf G1(col, col);    G1.setZero();
		MatrixXf G3(col, col);    G3.setZero();
		MatrixXf G4(col, col);    G4.setZero();
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				CMat(i,j) = C[0][0](i,j).real();
			}
		}
		for (int i = 0; i < (int)C.size(); i++)
		{
			C[i].clear();
		}
		C.clear();

		// 实数感知矩阵相乘 //
		G1 = CMat.adjoint() * CMat;
		CMat.resize(0,0);

		// 列向量单位化 //
		float *G1_diag = (float*) malloc(sizeof(float) * col);
		for (int i = 0; i < col; i++)
		{
			G1_diag[i] = 1/sqrt(G1(i,i));
		}
		for (int j = 0; j < col; j++)
		{
			for (int i = 0; i < col; i++)
			{
				G3(i,j) = G1_diag[j]*G1(i,j);
			}
		}
		G1.resize(0,0);
		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < col; j++)
			{
				G3(i,j) = G1_diag[i]*G3(i,j);
			}
		}
		free(G1_diag);

		// 计算Miu //
		if (ref_ok && e.ready_r)
		{
			Miu = (G3 - e.G4r).cwiseAbs().maxCoeff();//最大值
		}
		else
		{
			// 回退：本地计算参考项（与原实现一致）
			int **I = (int**)malloc(sizeof(int*) * N);
			for (int i = 0; i < N; i++)
			{
				I[i] = (int*)malloc(sizeof(int) * N);
				for (int j = 0; j < N; j++) I[i][j] = 0;
			}
			for (int i = 0; i < N; i++) I[i][i] = 1;
			CpxNumMat FH(N, N);
			FFT1D_fftshift(I, FH, N, N);
			for (int i = 0; i < N; i++) { free(I[i]); I[i] = NULL; }
			free(I);
			vector< vector<CpxNumMat> > CI;
			fdct_wrapping(N, N, 1, 4, 1, FH, CI);
			clear(FH); FH.~NumMat();
			MatrixXf CIMat(col, col);  CIMat.setZero();
			MatrixXf G2(col, col);     G2.setZero();
			for (int i = 0; i < col; i++)
				for (int j = 0; j < col; j++)
					CIMat(i,j) = CI[0][0](i,j).real();
			for (int i = 0; i < (int)CI.size(); i++) CI[i].clear();
			CI.clear();
			G2 = CIMat.adjoint() * CIMat;
			CIMat.resize(0,0);
			float *G2_diag = (float*) malloc(sizeof(float) * col);
			for (int i = 0; i < col; i++) G2_diag[i] = 1/sqrt(G2(i,i));
			for (int j = 0; j < col; j++)
				for (int i = 0; i < col; i++)
					G4(i,j) = G2_diag[j]*G2(i,j);
			G2.resize(0,0);
			for (int i = 0; i < col; i++)
				for (int j = 0; j < col; j++)
					G4(i,j) = G2_diag[i]*G4(i,j);
			free(G2_diag);
			Miu = (G3 - G4).cwiseAbs().maxCoeff();
		}
		G3.resize(0,0); G4.resize(0,0);
	}

	// 返回μ值
	return Miu;
}

//****** 实数计算曲波变换平均μ值 ******//
float CalculateAveMiu(int row,int N,int **Phi)
{
	float sum=0;int k=0;
	for (int i=0;i<row;i++)
	{
		float temp_v=TestCurveletMiu1D(N,Phi[i],"EigenReal");
		if (temp_v>0)
		{
			float miu2 = temp_v;
			sum=sum+miu2;k++;
		}
	}
	if (k == 0) return 0; //修复：全零/异常输入时除零保护
	float avemiu=sum/k;
	return avemiu;
}

// =============================================================================
// 2026-10-21 新增：经典互干度（PSF 峰值旁瓣）—— 修正实现
// 详见 CalculateMiu.h 的接口注释与 THESIS_ALGORITHM_AUDIT.md
// =============================================================================
float SamplingMutualCoherence1D(const std::vector<int>& idx, int N)
{
    if (N <= 0 || idx.empty()) return 0.0f;
    const int m = (int)idx.size();
    if (m >= N) return 0.0f;   // 全采样：西矩阵非对角内积为 0

    // r(Δ) = Σ_{k∈Ω} exp(-2πi·Δ·k/N)，只需 Δ=1..N-1；r(0)=m
    double maxAbs = 0.0;
    for (int delta = 1; delta < N; delta++)
    {
        double re = 0.0, im = 0.0;
        const double w = -2.0 * 3.14159265358979323846 * (double)delta / (double)N;
        for (int t = 0; t < m; t++)
        {
            const double ang = w * (double)idx[t];
            re += cos(ang);
            im += sin(ang);
        }
        const double a = sqrt(re * re + im * im);
        if (a > maxAbs) maxAbs = a;
    }
    double mu = maxAbs / (double)m;
    if (mu < 0.0) mu = 0.0;
    if (mu > 1.0) mu = 1.0;   // 理论上界，防止浮点误差
    return (float)mu;
}

// =============================================================================
// 2026-10-21 新增：二维经典互干度（2D PSF 峰值旁瓣）
// =============================================================================
float SamplingMutualCoherence2D(const std::vector<int>& maskFlat, int Ny, int Nx)
{
    if (Ny <= 0 || Nx <= 0) return 0.0f;
    if ((int)maskFlat.size() != Ny * Nx) return 0.0f;

    std::vector<int> ys, xs;
    for (int y = 0; y < Ny; y++)
        for (int x = 0; x < Nx; x++)
            if (maskFlat[(size_t)(y * Nx + x)] == 1) { ys.push_back(y); xs.push_back(x); }
    const int m = (int)ys.size();
    if (m <= 1) return (m == 1) ? 1.0f : 0.0f;
    if (m >= Ny * Nx) return 0.0f;   // 全采样 -> 西矩阵 -> 0

    double maxAbs = 0.0;
    for (int dy = 0; dy < Ny; dy++)
    {
        for (int dx = 0; dx < Nx; dx++)
        {
            if (dy == 0 && dx == 0) continue;
            double re = 0.0, im = 0.0;
            for (int t = 0; t < m; t++)
            {
                const double ang = -2.0 * 3.14159265358979323846 *
                    ((double)dy * (double)ys[t] / (double)Ny + (double)dx * (double)xs[t] / (double)Nx);
                re += cos(ang);
                im += sin(ang);
            }
            const double a = sqrt(re * re + im * im);
            if (a > maxAbs) maxAbs = a;
        }
    }
    double mu = maxAbs / (double)m;
    if (mu < 0.0) mu = 0.0;
    if (mu > 1.0) mu = 1.0;
    return (float)mu;
}

