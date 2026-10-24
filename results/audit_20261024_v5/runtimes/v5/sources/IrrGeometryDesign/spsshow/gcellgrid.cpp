#include "gcellgrid.h"
#include "gutils.h"
#include "gspsparam.h"
#include "gpoint3d.h"
//#include "gaabbox.h"
#include "gutils.h"
#include "grlineset.h"
#include "gslineset.h"
#include "gproject.h"
#include "gspsparam.h"
//#include <qsqlquery.h>
//Added by qt3to4:
#include <Q3TextStream>

GCellGrid::GCellGrid()
{
	mGrid = NULL;
	mGridXY = NULL;// [3/20/2023 Liu Wei]
	mIlluEn = NULL;
	mRecvEn = NULL;
	mRecvOfsEn = NULL;
	mAccRecvOfsEn = NULL;
	mAccIlluEn = NULL;
	mAccRecvEn = NULL;
	mAccState = NULL;
	mRPtGrid = NULL;
	mMainRecvEn = NULL;
	mNumX = mNumY = 0;
	mGridX = mGridY = 0;
	mOfsNum = 0;

	mFold= NULL;
	setFold=0;

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	mMaxoffset=0;
	mInMaxoffset=0;//inline方向最大炮检距
	mCLMaxoffset=0;//Cline方向最大炮检距
	//////////////////////////////////////////////////////////////////////////add by LiuWei
}

GCellGrid::~GCellGrid()
{
	clear();
}

void GCellGrid::clear()
{
	// 删除CellGrid分配的空间
	if(mNumX > 0 && mNumY > 0)
	{
		gFree2(mGrid);
		gFree2(mGridXY);// [3/20/2023 Liu Wei]
		gFree2(mIlluEn);
		gFree2(mRecvEn);
		gFree2(mAccIlluEn);
		gFree2(mAccRecvEn);
		gFree2(mAccState);
		gFree2(mRPtGrid);
		gFree2(mRListGrid);
		gFree2(mMainRecvEn);

		//赵虎添加
		if(mFold!=NULL) gFree2(mFold);
		mNumX = mNumY = 0;
		setFold=0;
	}
}

void GCellGrid::zeroEn()
{
	gFill2(mIlluEn, 0.0f, mNumY, mNumX);
	gFill2(mRecvEn, 0.0f, mNumY, mNumX);
	//赵虎添加
	if(mFold!=NULL) gFill2(mFold,0, mNumY, mNumX);

}

void GCellGrid::zeroAccEn()
{
	gFill2(mAccIlluEn, 0.0f, mNumY, mNumX);
	gFill2(mAccRecvEn, 0.0f, mNumY, mNumX);
}

void GCellGrid::zeroAccState()
{
	memset(mAccState[0], 0, sizeof(int)*mNumX*mNumY);
}

void GCellGrid::nullRPtGrid()
{
	gFill2(mRPtGrid, (GRPoint *)NULL, mNumY, mNumX);
}

void GCellGrid::clearRListGrid()
{
	int i, j;
	for(i=0; i<mNumY; i++)
	{
		for(j=0; j<mNumX; j++)
		{
			mRListGrid[i][j].clear();
		}
	}
}

void GCellGrid::zeroMainRecvEn()
{
	gFill2(mMainRecvEn, 0.0f, mNumY, mNumX);
}

void GCellGrid::accIlluEn()
{
	for(int i=0; i<mNumY; i++)
	{
		for(int j=0; j<mNumX; j++)
		{
			mAccIlluEn[i][j] += mIlluEn[i][j];
		}
	}
}

void GCellGrid::accRecvEn()
{
	for(int i=0; i<mNumY; i++)
	{
		for(int j=0; j<mNumX; j++)
		{
			mAccRecvEn[i][j] += mRecvEn[i][j];
		}
	}
}

void GCellGrid::allocOfsEn(int nofs)
{
	freeOfsEn();
	gAlloc3(mRecvOfsEn, nofs, mNumY, mNumX);
	gAlloc3(mAccRecvOfsEn, nofs, mNumY, mNumX);
	mOfsNum = nofs;
}

void GCellGrid::freeOfsEn()
{
	if(mRecvOfsEn != NULL)
	{
		gFree3(mRecvOfsEn);
	}
	if(mAccRecvOfsEn != NULL)
	{
		gFree3(mAccRecvOfsEn);
	}
	mOfsNum = 0;
}

void GCellGrid::accOfsEn()
{
	int i, j, k;
	for(i=0; i<mOfsNum; i++)
	{
		for(j=0; j<mNumY; j++)
		{
			for(k=0; k<mNumX; k++)
			{
				mAccRecvOfsEn[i][j][k] += mRecvOfsEn[i][j][k];
			}
		}
	}
}

// 构造照明面元网格
// pSPSParam:			SPS设计参数指针
// tx0, ty0:			面元网格偏离第一中心点的相对距离分量
// numX, numY:			面元网格数目
// gridX, gridY:		面元网格尺寸
void GCellGrid::create(GSPSParam *pSPSParam, float tx0, float ty0, int numX, int numY,
					   float gridX, float gridY)
{
	clear();

	// 分配CellGrid空间
	mNumX = numX;		mNumY = numY;
	mGridX = gridX;		mGridY = gridY;
	gAlloc2(mGrid, numY+1, numX+1);
	gAlloc2(mGridXY, numY+1, numX+1);// [3/20/2023 Liu Wei]
	gAlloc2(mIlluEn, mNumY, mNumX);
	gAlloc2(mRecvEn, mNumY, mNumX);
	gAlloc2(mAccIlluEn, mNumY, mNumX);
	gAlloc2(mAccRecvEn, mNumY, mNumX);
	gAlloc2(mAccState, mNumY, mNumX);
	gAlloc2(mRPtGrid, mNumY, mNumX);
	gAlloc2(mRListGrid, mNumY, mNumX);
	gAlloc2(mMainRecvEn, mNumY, mNumX);
//赵虎添加
	gAlloc2(mFold,mNumY,mNumX);
	setFold=0;

	// 将相关参数清0
	zeroEn();
	zeroAccEn();
	zeroAccState();
	nullRPtGrid();
	zeroMainRecvEn();

	int i, j;
	float tx, ty, spsX, spsY;
	ty = ty0;

	// 循环计算面元网格点的SPS坐标
	for(i=0; i<=mNumY; i++)
	{
		tx = tx0;
		for(j=0; j<=mNumX; j++)
		{
			mGridXY[i][j].set(tx,-1e10,ty);
			// 将相对坐标变换为SPS坐标
			pSPSParam->transTosps(tx, ty, spsX, spsY);
			// 为网格点设置坐标
			mGrid[i][j].set(spsX, -1e10, spsY);
			tx += mGridX;
		}
		ty += mGridY;
	}

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	mMaxoffset=0;
	mInMaxoffset=0;//inline方向最大炮检距
	mCLMaxoffset=0;//Cline方向最大炮检距
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	// 寻找面元网格坐标范围 [Liu Wei 3/29/2023]
	float minbinX, minbinY, maxbinX, maxbinY;

	minbinX = minbinY = HUGE_VAL;
	maxbinX = maxbinY = -HUGE_VAL;

	for(i=0; i<=mNumY; i++)
	{
		for(j=0; j<=mNumX; j++)
		{
			if(mGrid[i][j].x() < minbinX) minbinX = mGrid[i][j].x();
			if(mGrid[i][j].x() > maxbinX) maxbinX = mGrid[i][j].x();
			if(mGrid[i][j].z() < minbinY) minbinY = mGrid[i][j].z();
			if(mGrid[i][j].z() > maxbinY) maxbinY = mGrid[i][j].z();
		}
	}
	mXmin=minbinX; mYmin=minbinY;
	mXmax=maxbinX; mYmax=maxbinY;
}

bool GCellGrid::save(QDataStream &stream)
{
	// 存入基本参数
	stream << mGridX << mGridY;
	stream << mNumX << mNumY;
	stream << mDiffX << mDiffY;
	// 跳过填充字节
	gSkipPadBytes(stream, GCELLGRID_PAD_SIZE);
	// 存入网格点坐标
	if(mNumX > 0 && mNumY > 0)
	{
		int i, j;
		
		for(i=0; i<=mNumY; i++)
		{
			for(j=0; j<=mNumX; j++)
			{
				stream << mGrid[i][j].x() << mGrid[i][j].z();
			}
		}
	}

	return true;
}

bool GCellGrid::load(QDataStream &stream)
{
	clear();

	// 读入基本参数
	stream >> mGridX >> mGridY;
	stream >> mNumX >> mNumY;
	stream >> mDiffX >> mDiffY;
	// 跳过填充字节
	gSkipPadBytes(stream, GCELLGRID_PAD_SIZE);
	
	if(mNumX > 0 && mNumY > 0)
	{
		// 分配空间
		gAlloc2(mGrid, mNumY+1, mNumX+1);
		gAlloc2(mIlluEn, mNumY, mNumX);
		gAlloc2(mRecvEn, mNumY, mNumX);
		gAlloc2(mAccIlluEn, mNumY, mNumX);
		gAlloc2(mAccRecvEn, mNumY, mNumX);
		gAlloc2(mAccState, mNumY, mNumX);
		gAlloc2(mRPtGrid, mNumY, mNumX);
		gAlloc2(mRListGrid, mNumY, mNumX);
		gAlloc2(mMainRecvEn, mNumY, mNumX);
		//赵虎添加
		gAlloc2(mFold,mNumY,mNumX);
		// 数据清0
		zeroEn();
		zeroAccEn();
		zeroAccState();
		nullRPtGrid();
		zeroMainRecvEn();
		// 读入网格坐标
		int i, j;
		for(i=0; i<=mNumY; i++)
		{
			for(j=0; j<=mNumX; j++)
			{
				stream >> mGrid[i][j][0] >> mGrid[i][j][2];
				mGrid[i][j][1] = -1e10;
			}
		}
	}

	return true;
}

bool GCellGrid::exportInEn(const QString &fileName, uint surfaceID)
{
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	Q3TextStream stream(&file);
	stream.setEncoding(Q3TextStream::Locale);
	QString line;

	stream << QString::fromLocal8Bit("// 面元入射能量文件\r\n");
	stream << QString::fromLocal8Bit("// 目的层ID\r\n");
	stream << QString::fromLocal8Bit("// 面元X尺寸\t面元Y尺寸\r\n");
	stream << QString::fromLocal8Bit("// X面元数目\tY面元数目\r\n");
	stream << QString::fromLocal8Bit("// 面元网格左上角X\tY\t面元网格角度\r\n");
	stream << QString::fromLocal8Bit("// 面元X\t面元Y\t面元Z\t面元入射能量\r\n");

	line.sprintf("%d\r\n", surfaceID);
	stream << line;
	line.sprintf("%f\t%f\r\n", mGridX, mGridY);
	stream << line;
	line.sprintf("%d\t%d\r\n", mNumX, mNumY);
	stream << line;

	if(mNumX <= 0 || mNumY <= 0)
	{
		stream << "0\t0";		// 网格第一点x, y坐标
		file.close();
		return true;
	}
	
	// 记录第一网格左上角坐标
	GProject *pProject = GProject::getCurrentProject();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	line.sprintf("%f\t%f\t%f\r\n", mGrid[0][0].x(), mGrid[0][0].z(), pSPSParam->mAf*RADIAN_TO_ANGLE);
	stream << line;
	int i, j;
	float hx, hy;
	hx = mGridX / 2;
	hy = mGridY / 2;
	
	for(i=0; i<mNumY; i++)
	{
		for(j=0; j<mNumX; j++)
		{
			if(mAccIlluEn[i][j] > 0 || mAccRecvEn[i][j] > 0)
			{
				line.sprintf("%.2f\t%.2f\t%.2f\t%f\r\n", mGrid[i][j].x()+hx,
					mGrid[i][j].z()+hy, mGrid[i][j].y(), mAccIlluEn[i][j]);
				stream << line;
			}
		}
	}

	file.close();

	return true;
}

bool GCellGrid::exportRecvEn(const QString &fileName, uint surfaceID)
{
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	Q3TextStream stream(&file);
	stream.setEncoding(Q3TextStream::Locale);
	QString line;

	stream << QString::fromLocal8Bit("// 面元接收能量文件\r\n");
	stream << QString::fromLocal8Bit("// 目的层ID\r\n");
	stream << QString::fromLocal8Bit("// 面元X尺寸\t面元Y尺寸\r\n");
	stream << QString::fromLocal8Bit("// X面元数目\tY面元数目\r\n");
	stream << QString::fromLocal8Bit("// 面元网格左上角X\tY\t面元角度\r\n");
	stream << QString::fromLocal8Bit("// 面元X\t面元Y\t面元Z\t面元接收能量\r\n");

	line.sprintf("%d\r\n", surfaceID);
	stream << line;
	line.sprintf("%f\t%f\r\n", mGridX, mGridY);
	stream << line;
	line.sprintf("%d\t%d\r\n", mNumX, mNumY);
	stream << line;

	if(mNumX <= 0 || mNumY <= 0)
	{
		stream << "0\t0";		// 网格第一点x, y坐标
		file.close();
		return true;
	}
	
	// 记录第一网格左上角坐标及角度
	GProject *pProject = GProject::getCurrentProject();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	line.sprintf("%f\t%f\t%f\r\n", mGrid[0][0].x(), mGrid[0][0].z(), pSPSParam->mAf*RADIAN_TO_ANGLE);
	stream << line;
	int i, j;
	float hx, hy;
	hx = mGridX / 2;
	hy = mGridY / 2;
	
	for(i=0; i<mNumY; i++)
	{
		for(j=0; j<mNumX; j++)
		{
			if(mAccIlluEn[i][j] > 0 || mAccRecvEn[i][j] > 0)
			{
				line.sprintf("%.2f\t%.2f\t%.2f\t%f\r\n", mGrid[i][j].x()+hx,
					mGrid[i][j].z()+hy, mGrid[i][j].y(), mAccRecvEn[i][j]);
				stream << line;
			}
		}
	}

	file.close();

	return true;
}

bool GCellGrid::exportMainRecvEn(Q3TextStream &stream, GSPoint *pSPoint)
{
	int i, j;
	float hx, hy;
	hx = mGridX / 2;
	hy = mGridY / 2;

	QString line;

	for(i=0; i<mNumY; i++)
	{
		for(j=0; j<mNumX; j++)
		{
			GRPoint *pRPoint = mRPtGrid[i][j];
			if(pRPoint != NULL)
			{
				line.sprintf("%-16s%-8s%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%-16s%-8s%.2f\t%.2f\t%.2f\t%f\r\n",
					(const char *)pSPoint->getLineName(), (const char *)pSPoint->getPointNum(),
					pSPoint->getSPSX(), pSPoint->getSPSY(), pSPoint->getSPSZ(),
					mGrid[i][j].x()+hx, mGrid[i][j].z()+hy, mGrid[i][j].y(),
					(const char *)pRPoint->getLineName(), (const char *)pRPoint->getPointNum(),
					pRPoint->getSPSX(), pRPoint->getSPSY(), pRPoint->getSPSZ(),
					mMainRecvEn[i][j]);
				stream << line;
			}
		}
	}
	return true;
}

bool GCellGrid::exportOfsEn(Q3TextStream *streams, GSPoint *pSPoint)
{
	int i, j, k;
	float hx, hy, en;
	hx = mGridX / 2;
	hy = mGridY / 2;
	
	QString line, str;
	
	if(pSPoint != NULL)
	{
		for(i=0; i<mNumY; i++)
		{
			for(j=0; j<mNumX; j++)
			{
				line.sprintf("%-16s%-8s%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
					(const char *)pSPoint->getLineName(), (const char *)pSPoint->getPointNum(),
					pSPoint->getSPSX(), pSPoint->getSPSY(), pSPoint->getSPSZ(),
							mGrid[i][j].x()+hx, mGrid[i][j].z()+hy, mGrid[i][j].y());
				for(k=0; k<mOfsNum; k++)
				{
					en = mRecvOfsEn[k][i][j];
					if(en > 1e-7)
					{
						streams[k] << line;
						str.sprintf("\t%.f\r\n", en);
						streams[k] << str;
					}
				}
			}
		}
	}
	else
	{
		for(i=0; i<mNumY; i++)
		{
			for(j=0; j<mNumX; j++)
			{
				line.sprintf("%.2f\t%.2f\t%.2f",
					mGrid[i][j].x()+hx, mGrid[i][j].z()+hy, mGrid[i][j].y());
				for(k=0; k<mOfsNum; k++)
				{
					en = mAccRecvOfsEn[k][i][j];
					if(en > 1e-7)
					{
						streams[k] << line;
						str.sprintf("\t%.f\r\n", en);
						streams[k] << str;
					}
				}
			}
		}
	}
	return true;
}

void GCellGrid::calculateCoverCount()
{
	GProject *pProject = GProject::getCurrentProject();
	if (!pProject || !mFold || mNumX <= 0 || mNumY <= 0 || !mGridXY) return;

	int r,s; 
	for(r=0; r<mNumY; r++)
		for(s=0; s<mNumX; s++)
			mFold[r][s] = 0;
	mMaxoffset = 0;
	mInMaxoffset = 0;
	mCLMaxoffset = 0;
	GRLineSet *rLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
	                      ? pProject->getRLineSetNew()
	                      : ((pProject->getRLineSet() && pProject->getRLineSet()->count() > 0) ? pProject->getRLineSet() : pProject->getRLineSetNew());
	GShotSet *tsl = (pProject->mIsNewImfo && pProject->getShotSetNew() && pProject->getShotSetNew()->count() > 0)
	                ? pProject->getShotSetNew()
	                : ((pProject->getShotSet() && pProject->getShotSet()->count() > 0) ? pProject->getShotSet() : pProject->getShotSetNew());
	if (!rLineSet || !tsl || tsl->count() == 0) return;

#pragma omp parallel for //并行计算代码
	for(s=0; s<tsl->count(); s++)
	{
		GShot *sl = tsl->get(s);
		float offsetx = sl->getSPoint()->getRandomOffsetX();
		float offsety = sl->getSPoint()->getRandomOffsetY();

		//float sx = sl->getSPoint()->getSPSX()+offsetx;//赵老师的原始代码
		//float sy = sl->getSPoint()->getSPSY()+offsety;//赵老师的原始代码

		float sx = sl->getSPoint()->getX()+offsetx;// [3/20/2023 Liu Wei]
		float sy = sl->getSPoint()->getY()+offsety;// [3/20/2023 Liu Wei]

		int test_count=sl->getTrcCount();// 跟踪监视 [Liu Wei 6/21/2023]
		for(int si=0; si<sl->getTrcCount(); si++)
		{
			
			GRPoint *rpoint = sl->getRPointByTrcIndex(si,rLineSet);
			if(rpoint==NULL)
				continue;
			//float rx = rpoint->getSPSX()+offsetx;//赵老师的原始代码
			//float ry = rpoint->getSPSY()+offsety;//赵老师的原始代码

			float rx = rpoint->getX()+offsetx;// [3/20/2023 Liu Wei]
			float ry = rpoint->getY()+offsety;// [3/20/2023 Liu Wei]

			float tempcmpx = (rx+sx)/2;
			float tempcmpy = (ry+sy)/2;

			int i = (tempcmpx-mGridXY[0][0].x())/mGridX;// [3/20/2023 Liu Wei]
			int j = (tempcmpy-mGridXY[0][0].z())/mGridY;// [3/20/2023 Liu Wei]

			//赵虎添加
			int offset=sqrt((float)((sx-rx)*(sx-rx)+(sy-ry)*(sy-ry)));//炮检距
			#pragma omp critical(gcell_maxoff)
			{ if(offset>mMaxoffset) mMaxoffset=offset; }//求最大炮检距
			int tInoffset=abs(sx-rx);
			#pragma omp critical(gcell_maxoff)
			{ if(tInoffset>mInMaxoffset) mInMaxoffset=tInoffset; }
			int tCLoffset=abs(sy-ry);
			#pragma omp critical(gcell_maxoff)
			{ if(tCLoffset>mCLMaxoffset) mCLMaxoffset=tCLoffset; }
			//赵虎添加

			if(i<0||j<0)
				continue; 	
			if(i>mNumX-1||j>mNumY-1)
				continue; 
			#pragma omp atomic
			mFold[j][i]++;
			// (removed racy debug read-back of mFold[j][i])
		}
	}

	float max = 0;
	for(r=0; r<mNumY; r++)
	{
		for(s=0; s<mNumX; s++)
		{
			if(max<mFold[r][s])
				max = mFold[r][s];
		}
	}

	//赵虎添加
	//找到满覆盖次数的范围
	// 对于非规则观测系统而言，最大覆盖次数可能不是理论的满覆盖次数！！ [Liu Wei 4/8/2023]
	int target_folds=max*0.9;
	int begini=0;int endi=mNumX-1;
	int beginj=0;int endj=mNumY-1;
	for(r=1; r<mNumY-1; r++)
	{
		for(s=1; s<mNumX-1; s++)
		{
			if(mFold[r][s]==target_folds && mFold[r-1][s-1]<target_folds && begini==0 && beginj==0) 
			{
				begini=s; beginj=r;
			}
			if(mFold[r][s]==target_folds && mFold[r+1][s+1]<target_folds) //|| mFold[r][s+1]<max || mFold[r+1][s]<max
			{// 也不能保证百分之百找到满覆盖区域 [Liu Wei 4/6/2023]
				if (s>begini && r>beginj)
				{
					endi=s; endj=r;
				}
			}
			//if(mFold[r][s]==max && mFold[r+1][s+1]<max) //赵老师的源码：会出现开始结束范围相等的Bug情况。
			//{
			//	endi=s; endj=r;
			//}
		}
	}
	pProject->setFoldArea(begini,endi,beginj,endj);//满覆盖的范围
	//找到满覆盖次数的范围

	//赵虎添加
	pProject->getAttrAnalyzeParam()->mBeginAreaOffset=mMaxoffset/3;//炮检距分区域分析的区间边界初始化
	pProject->getAttrAnalyzeParam()->mEndAreaOffset=2*mMaxoffset/3;
	pProject->getAttrAnalyzeParam()->mBeginAreaInOffset=mInMaxoffset/3;//OVT分析的inline区间边界初始化
	pProject->getAttrAnalyzeParam()->mEndAreaInOffset=2*mInMaxoffset/3;
	pProject->getAttrAnalyzeParam()->mBeginAreaCLOffset=mCLMaxoffset/3;//OVT分析的Crline区间边界初始化
	pProject->getAttrAnalyzeParam()->mEndAreaCLOffset=2*mCLMaxoffset/3;
	pProject->getAttrAnalyzeParam()->mInMaxOffset=mInMaxoffset;
	pProject->getAttrAnalyzeParam()->mCLMaxOffset=mCLMaxoffset;
	//赵虎添加
}
