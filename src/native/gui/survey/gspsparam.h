#ifndef GSPS_PARAM_H
#define GSPS_PARAM_H

#include <qfile.h>
#include <qdatastream.h>

#define GSPSPARAM_PAD_SIZE 512

// 创建SPS基本参数
class GSPSParam
{
public:
	// 炮线编线方式
	enum GSLineType {gInLine, gCrossLine};
public:
	float mAf;						// 检波线倾角
	Q_INT32 mRLineCount;			// 检波线数目
	float mRLineInv;				// 检波线间距 
	float mRX0, mRY0, mRZ0;			// No.1检波点坐标
	Q_INT32 mRPtCount;				// 检波点数目
	float mRPtInv;					// 检波点间隔

	float mMoveX0;	               //add by zhaohu 检波点X方向平移量（旋转成水平坐标时的值）
	float mMoveY0;	               //add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
	bool mOrginalByShot;           //add by zhaohu 以第一个炮点为原点设计观测系统
	bool mOrginalByReceiver;       //add by zhaohu 以第一个检波点为原点设计观测系统

	float mSAf;						// 炮线倾角
	GSLineType mSLineType;			// 编线方式
	Q_INT32 mSLineCount;			// 炮线数目，注意：作用于多条炮线时的模板，默认值1
	float mSLineInv;				// 炮线间隔，注意：作用于多条炮线时的模板，默认值250。
	float mSX0, mSY0, mSZ0;			// No.1炮点坐标
	Q_INT32 mSPtCount;				// 炮点数目/炮线（模板的单线炮点数）
	float mSPtInv;					// 炮点间隔

	float mMaxLOffset;				// 左支最大偏移距
	float mMinLOffset;				// 左支最小偏移距
	float mMaxROffset;				// 右支最大偏移距
	float mMinROffset;				// 右支最小偏移距

	float mSinAf, mCosAf;			// 测线旋转三角函数值
	float mTransX0, mTransY0;		// 旋转后的SPS平移量
	float mTransX02, mTransY02;		// 旋转后的SPS平移量

	bool mBaseCMP;bool mBaseCRP;//反演方式
public:
	GSPSParam();

	// 重置参数到默认状态
	void reset();

	// 变换后相对坐标到SPS坐标
	void transTosps(float tx, float ty, float &sx, float &sy);
	// SPS坐标到变换后相对坐标
	void spsTotrans(float sx, float sy, float &tx, float &ty);

	// 保存
	bool save(QDataStream &stream);
	// 保存到数据库
	bool save();
	// 载入
	bool load(QDataStream &stream);
	// 从数据库载入
	bool load();

	//赵虎添加
	void spsTotrans2(float SinAf,float CosAf, float sx, float sy, float &tx, float &ty);
};

#endif