/*	文件名:gcmpset.h
 *	摘要:gcmplist.h和gcmplist.cpp文件描述检波线列表数据结构,表结构如下:
 *
 *	一级表(1)CMPLine列表
 *	二级表(1)CMPPoint列表,(2)inline方向坐标
 *	三级表(1)CMP号,(2)CMP的X,Y坐标,(3)最近炮号,(4)覆盖次数
 *	四级表(1)炮号,(2)道号,(3)炮检距,(4)道在SEGY文件位置
 *
 *	编制单位:成都理工大学物探研究室
 *	作者:邓飞	校审:邓飞
 *	完成日期:2003.10.17 */
//
#ifndef GCMP_SET_H
#define GCMP_SET_H

#include "qfile.h"
#include "qdatastream.h"
#include "gshotset.h"
#include "grlineset.h"
#include "gslineset.h"
#include "gptrvector.h"

class GProject;
class GCMPItem;
class GCMPoint;
class GCMPLine;
class GCMPLineSet;

class GCMPItem	//CMP子项(炮检对)对应三级表
{
friend class GCMPPoint;
private:
	GShotSet *mpShotSet;			//炮关系数据集
	Q_INT32 mShotIndex;				//炮关系索引
	Q_INT32 mTrcIndex;				//道号道下标
	float mSTRange;					//炮道距
	short mAngle;                   //方位角
	long long mFilePos;		//对应SEGY文件位置
	//////////
	int mCMPPointNum;
public:
	GCMPItem();
	GCMPItem(GShot *pShot, GRPoint *pRPoint, int trcIndex);

	//----得到炮关系----
	GShot * getShot()			{ return mpShotSet->get(mShotIndex); }
	//----得到炮关系索引----
	int getShotIndex()			{ return mShotIndex; }
	//----得到炮号(野外记录号)----
	int getRecNum()				{ return getShot()->getRecNum(); }
	//----得到炮桩号----
	QString getSStackNum()		{ return getShot()->getStackNum(); }
	//----得到道号下标----
	int getTrcIndex()			{ return mTrcIndex; }
	//----得到炮道距离----
	float getSTRange()			{ return mSTRange; }
	//----得到SEG-Y文件下标----
	Q_INT32 getFileIndex()		{ return getShot()->getFileIndex(); }
	//----得到道在SEGY文件中的位置----
	long long getFilePos()	{ return mFilePos; }

	//----得到炮道方位角----
	short getAngle()			{ return mAngle; }//add by zhaohu

	//----保存到流----
	void save(QDataStream &stream);
	//----从流中加载----
	void load(QDataStream &stream);
	//////////////////////
	void setCMPPointNum(int cmpNum){ mCMPPointNum = cmpNum;}
	int getCMPPointNum(){return mCMPPointNum;}
};

class GCMPPoint								//CMP点(含多个炮检对子项)对应二级表
{
friend class GCMPLine;
friend class GCMPLineSet;
private:
	GShotSet *mpShotSet;
	Q_INT32 mCrossLineNum;					//垂直测线方向编号
	float mX, mY;							//对应旋转后的坐标
	Q_INT32 mNearShotIndex;					//近炮关系下标
	float mNearRange;						//近炮距
	GPtrVector<GCMPItem> *mpCMPItemList;	//炮检对列表
private:
	void insert(GShot *pShot, GRPoint *pRPoint, int trcIndex, float xCMP, float yCMP);
public:
	GCMPPoint();
	~GCMPPoint();

	//----得到共中心点X坐标(旋转坐标系)----
	float getX()			{ return mX; }
	//----得到共中心点Y坐标(旋转坐标系)----
	float getY()			{ return mY; }
	//----得到垂直于测线方向编号(中心点编号)----
	int getCrossLineNum()	{ return mCrossLineNum; }
	//----得到近炮关系----
	GShot * getNearShot()	{ return mpShotSet->get(mNearShotIndex); }
	//----得到近炮关系下标----
	int getNearShotIndex()	{ return mNearShotIndex; }
	//----得到近炮距离----
	float getNearRange()	{ return mNearRange; }
	//----得到CMP子项个数(炮检对个数)----
	int count()				{ return mpCMPItemList->count(); }
	//----按下标得到CMP子项(炮检对)指针----
	GCMPItem * get(int index) { return mpCMPItemList->at( index ); }
	//----按炮检距排序----
	void sort();

	//----保存到流----
	void save(QDataStream &stream);
	//----从流中加载----
	void load(QDataStream &stream);
	GPtrVector<GCMPItem> *getCMPItemList(){return mpCMPItemList;}
	void setX(float x){mX = x;}
	void setY(float y){mY = y;}
	void setCrossNum(int num){mCrossLineNum=num;}
};

class GCMPLine								//CMPLine,对应二级表
{
friend class GCMPLineSet;
private:
	Q_INT32 mInLineNum;						//沿测线方向编号
	float mY;								//该CMP线旋转Y坐标
	GShotSet *mpShotSet;					//炮索引列表指针
	GRLineSet *mpRLineSet;					//检波点列表指针
	GPtrVector<GCMPPoint> *mpCMPPointList;	//CMP列表
	float mHalfHCMPInv, mHalfVCMPInv;		//横,纵半CMP点距
private:
	void insert(GShot *pShot, GRPoint *pRPoint, int trcIndex);
public:
	GCMPLine();
	~GCMPLine();

	//----得到中心线Y坐标(旋转坐标系)----
	float getY()				{ return mY; }
	//----得到沿测线方向编号(共中心线编号)----
	Q_INT32 getInLineNum()		{ return mInLineNum; }
	//----得到共中心点数量----
	int count()					{ return mpCMPPointList->count(); }
	//----按下标得到共中心点指针----
	GCMPPoint * get(int index)  { return mpCMPPointList->at(index); }
	//----得到该线最大覆盖次数----
	int getMaxCoverCount();
	//----按炮检距对每个共中心面元排序----
	void sort();
	//----保存到流----
	void save(QDataStream &stream);
	//----从流中加载----
	void load(QDataStream &stream);
	void insertCMPPoint(GCMPPoint *point){mpCMPPointList->append(point);}
};

class GCMPLineSet								//CMPLine列表,对应一级表
{
private:
	GPtrVector<GCMPLine> *mpCMPLineList;		//CMPLine列表
private:
	void insert( GShot *pShot, GShotXLine *pXLine, GRLine *pRLine );
public:
	GCMPLineSet();
	~GCMPLineSet();

	//----得到共中心线数量----
	int count()					{ return mpCMPLineList->count(); }
	//----按下标得到共中心线指针----
	GCMPLine * get(int index)	{ return mpCMPLineList->at(index); }
	//----进行共中线、心点抽取----
	void extract(GShotSet *ShotSet,GRLineSet *RLineSet);
	//----进行全区共中心点编号----
	void enCode();
	//----得到全区最大覆盖次数----
	int getMaxCoverCount();

	//----清空数据----
	void clear();
	//----按炮检距为每个共中心面元排序----
	void sort();

	//----保存到文件----
	bool save(const QString &fileName);
	//----从文件中加载----
	bool load(const QString &fileName);
};
		

#endif
