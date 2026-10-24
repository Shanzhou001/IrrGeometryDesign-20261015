//#include <qsqlquery.h>
#include "gspsparam.h"
#include "../../core/gutils.h"
#include "../../core/gmath.h"

GSPSParam::GSPSParam()
{
	reset();
}

void GSPSParam::reset()
{
	//  [Liu Wei 6/21/2023]排列（分割）组织参数
	mMaxLOffset = 1225;
	mMinLOffset = 0;
	mMaxROffset = 1225;
	mMinROffset = 0;
	//  [Liu Wei 6/21/2023]排列（分割）组织参数
	// 接收线参数 [Liu Wei 6/21/2023]
	mAf = 0;
	mRLineCount = 12;
	mRLineInv = 200;
	mRPtCount = 145;
	mRPtInv = 50;
	mRX0 = 400000;// 原代码：mRX0=0; [Liu Wei 6/19/2023]
	mRY0 = 3000000;// 原代码：mRY0=0; [Liu Wei 6/19/2023]
	mRZ0 = 0;

	// 炮点原点坐标相对于检波器原点坐标的偏移量 [Liu Wei 5/29/2023]
	mMoveX0 =0 ;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	mMoveY0 =0 ;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）

	mOrginalByShot=false;           //add by zhaohu 以第一个炮点为原点设计观测系统
	mOrginalByReceiver=true;       //add by zhaohu 以第一个检波点为原点设计观测系统

	// 炮线参数 [Liu Wei 6/21/2023]
	//mSAf = 0;//Inline方向为0度，对应gInLine
	//mSLineType = gInLine;//炮线平行于测线
	mSAf=PI/2;//Crossline方向为90度，对应gCrossLine
	mSLineType = gCrossLine;//炮线垂直与测线
	mSLineCount = 20;
	mSLineInv = 250;
	mSPtCount = 8;
	mSPtInv = 50;
	mSX0 = mRX0 + mMaxLOffset;
	mSY0 = mRY0 + ((mRLineCount-1)*mRLineInv-(mSPtCount-1)*mSPtInv)/2;
	mSZ0 = 0;

	// add by liuwei [Liu Wei 5/29/2023]
	mSinAf = 0;
	mCosAf = 1;
	// add by liuwei [Liu Wei 5/29/2023]
	mBaseCMP = false;
	mBaseCRP = true;
}

void GSPSParam::transTosps(float tx, float ty, float &sx, float &sy)
{
	tx += mTransX0;
	ty += mTransY0;
	double x, y;
	gRotate(mSinAf, mCosAf, tx, ty, x, y);
	sx = x;		sy = y;
}

void GSPSParam::spsTotrans(float sx, float sy, float &tx, float &ty)
{
	double x, y;
	gRotate(-mSinAf, mCosAf, sx, sy, x, y);
	tx = x;		ty = y;
	tx -= mTransX0;
	ty -= mTransY0;
}

void GSPSParam::spsTotrans2(float SinAf,float CosAf, float sx, float sy, float &tx, float &ty)
{
	double x, y;
	gRotate(SinAf, CosAf, sx, sy, x, y);
	tx = x;		ty = y;
	tx -= mTransX0;
	ty -= mTransY0;
}

bool GSPSParam::save(QDataStream &stream)
{
	stream << mAf;
	stream << mRLineCount;
	stream << mRLineInv;
	stream << mRX0;
	stream << mRY0;
	stream << mRZ0;
	stream << mRPtCount;
	stream << mRPtInv;
	stream << mMoveX0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	stream << mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	stream << mOrginalByReceiver;       //add by zhaohu 以第一个检波点为原点设计观测系统
	stream << mOrginalByShot;           //add by zhaohu 以第一个炮点为原点设计观测系统


	stream << mSAf;
	stream << (Q_INT32)mSLineType;
	stream << mSLineCount;
	stream << mSLineInv;
	stream << mSX0;
	stream << mSY0;
	stream << mSZ0;
	stream << mSPtCount;
	stream << mSPtInv;

	stream << mMaxLOffset;
	stream << mMinLOffset;
	stream << mMaxROffset;
	stream << mMinROffset;

	stream << mTransX0;
	stream << mTransY0;
	stream << mSinAf;
	stream << mCosAf;

	stream << mBaseCRP;
	stream << mBaseCMP;

	gSkipPadBytes(stream, GSPSPARAM_PAD_SIZE);
	return true;
}

bool GSPSParam::save()
{
	/*QSqlQuery query;
	query.prepare("UPDATE nsps SET sps_param = ?");
	QByteArray ba;
	QDataStream stream(ba, QIODevice::WriteOnly);
	save(stream);
	query.bindValue(0, ba);
	if(!query.exec()) return false;*/
	
	return true;
}

bool GSPSParam::load(QDataStream &stream)
{
	Q_INT32 i32;

	stream >> mAf;
	stream >> mRLineCount;
	stream >> mRLineInv;
	stream >> mRX0;
	stream >> mRY0;
	stream >> mRZ0;
	stream >> mRPtCount;
	stream >> mRPtInv;
	stream >> mMoveX0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	stream >> mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	stream >> mOrginalByReceiver;       //add by zhaohu 以第一个检波点为原点设计观测系统
	stream >> mOrginalByShot;           //add by zhaohu 以第一个炮点为原点设计观测系统

	stream >> mSAf;
	stream >> i32;
	mSLineType = (GSLineType)i32;
	stream >> mSLineCount;
	stream >> mSLineInv;
	stream >> mSX0;
	stream >> mSY0;
	stream >> mSZ0;
	stream >> mSPtCount;
	stream >> mSPtInv;

	stream >> mMaxLOffset;
	stream >> mMinLOffset;
	stream >> mMaxROffset;
	stream >> mMinROffset;

	stream >> mTransX0;
	stream >> mTransY0;
	stream >> mSinAf;
	stream >> mCosAf;

	stream >> mBaseCRP;
	stream >> mBaseCMP;

	gSkipPadBytes(stream, GSPSPARAM_PAD_SIZE);

	return true;
}

bool GSPSParam::load()
{
	/*reset();

	QSqlQuery query;
	query.prepare("SELECT sps_param FROM nsps");
	if(!query.exec()) return false;
	
	if(!query.first()) return false;
	
	QByteArray ba = query.value(0).toByteArray();
	if(ba.size() > 0)
	{
		QDataStream stream(ba, QIODevice::ReadOnly);
		load(stream);
	}
	*/
	return true;
}