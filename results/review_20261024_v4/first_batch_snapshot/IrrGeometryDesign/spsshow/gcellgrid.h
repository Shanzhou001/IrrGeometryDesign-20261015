#ifndef GCELL_GRID_H
#define GCELL_GRID_H

#include <qdatastream.h>
//Added by qt3to4:
#include <Q3TextStream>
#include "gvaluevector.h"

class GSPSParam;
class GPoint3d;
//class GAABBoxSet;
class GRPoint;
class GSPoint;

#define GCELLGRID_PAD_SIZE	512

// 检波点面元类
// 用于导出检波点能量
struct GCellRPt
{
	GRPoint *mpRPoint;			// 检波点指针
	float mEn;					// 检波点能量

	GCellRPt()
	{
		mpRPoint = NULL;
		mEn = 0;
	}

	GCellRPt(GRPoint *pRPt, float en)
	{
		mpRPoint = pRPt;
		mEn = en;
	}
};

// 面元网格类
class GCellGrid
{
public:
	GPoint3d **mGrid;		// 网格点坐标SPS
	GPoint3d **mGridXY;		// 网格点坐标NEW
	int mNumX, mNumY;		// 单元数目
	float mGridX, mGridY;	// 单元尺寸
	float mDiffX, mDiffY;	// 第一网格单元左上角偏离第一中心点位置的距离

	float **mIlluEn;		// 一条高斯束入射目的层面元的照明能量
	float **mRecvEn;		// 一条高斯束被目的层面元反射到地表后, 检波器接收的能量 
	float **mAccIlluEn;		// 累计照明能量
	float **mAccRecvEn;		// 累计接收能量
	int **mAccState;		// 访问状态
	GRPoint ***mRPtGrid;	// 对应每个面元的主要能量接收检波点网格
	float ***mRecvOfsEn;	// 按偏移距记录的面元接收能量
	float ***mAccRecvOfsEn;	// 累计偏移距面元接收能量
	float **mMainRecvEn;	// 每个面元的主要接收能量

	int mOfsNum;			// 偏移距数目
	GValueVector<GCellRPt> **mRListGrid;

	//赵虎添加
	int **mFold;//覆盖次数，同 int **mCoverCount;
	int maxFold;//最大的覆盖次数
	int setFold;//显示时的最大覆盖次数

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	float mXmin;//面元网格最小X坐标
	float mYmin;//面元网格最小Y坐标
	float mXmax, mYmax;//面元网格最大X、Y坐标
	int mMaxoffset;//最大炮检距
	int mInMaxoffset;//inline方向最大炮检距-最大纵向炮检距
	int mCLMaxoffset;//Cline方向最大炮检距-最大非纵炮检距
	//////////////////////////////////////////////////////////////////////////add by LiuWei
public:
	GCellGrid();
	~GCellGrid();

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	GPoint3d **getCellList(){return mGrid;}
	int **getCoverCount(){return mFold;}
	int getNumX(){return mNumX;}
	int getNumY(){return mNumY;}
	//////////////////////////////////////////////////////////////////////////add by LiuWei

	// 清空CellGrid
	void clear();

	// 将照明能量和接收能量矩阵清0
	void zeroEn();
	// 将累计照明能量和累计接收能量矩阵清0
	void zeroAccEn();
	// 将访问状态矩阵清0
	void zeroAccState();

	// 将检波点网格清NULL
	void nullRPtGrid();
	// 将面元主能量清0
	void zeroMainRecvEn();
	// 将RListGrid清空
	void clearRListGrid();


	// 累加照明能量
	void accIlluEn();
	// 累加接收能量
	void accRecvEn();

	// 分配偏移据能量数组
	void allocOfsEn(int nofs);
	// 释放偏移据能量数组
	void freeOfsEn();
	// 累计偏移距能量
	void accOfsEn();
	
	// 构造照明面元网格
	void create(GSPSParam *pSPSParam, float tx0, float ty0, int numX, int numY,
				float gridX, float girdY);
	//计算覆盖次数
	void  calculateCoverCount();

	// 保存到流中
	bool save(QDataStream &stream);

	// 从流中载入
	bool load(QDataStream &stream);

	// 保存入射能量和网格基本信息到文本文件
	bool exportInEn(const QString &fileName, uint surfaceID);

	// 保存接收能量和网格基本信息到文件
	bool exportRecvEn(const QString &fileName, uint surfaceID);

	// 保存每炮的网格接收主能量
	bool exportMainRecvEn(Q3TextStream &stream, GSPoint *pSPoint);

	// 保存偏移距接收能量
	bool exportOfsEn(Q3TextStream *streams, GSPoint *pSPoint);

	// 从文本文件中导入网格
	bool import(const QString &fileName);
};

#endif
