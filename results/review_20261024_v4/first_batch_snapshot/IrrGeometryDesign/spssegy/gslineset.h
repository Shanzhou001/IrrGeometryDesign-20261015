#ifndef GSLINE_SET_H
#define GSLINE_SET_H

#include <qobject.h>
#include "qfile.h"
#include "qdatastream.h"
#include "gptrvector.h"
#include "gvaluevector.h"

// GSPoint存盘时的填充空字节数
#define GSPOINT_PAD_SIZE 32

// GSLine存盘时填充空字节数
#define GSLINE_PAD_SIZE 64

// GSLineSet存盘时填充空字节数
#define GSLINESET_PAD_SIZE 64

class GSPSParam;

class GSPoint
{
friend class GShot;
friend class GShotSet;
friend class GSLineSet;
private:
	QString mLineName;						//炮线号
	QString mPointNum;						//点号
	int mRecNum;							// [3/16/2023 Liu Wei] 记录号（从关系文件中获取）
	float mSPSX, mSPSY, mSPSZ;				//SPS文件中X,Y,Z坐标
	float mDepth;							//SPS文件中井深
	float mBase;							//基准高
	float mX, mY;							//映射坐标
	float mSPSStatic;						//SPS野外静校正量
	float mTAO;								//TAO值（从SPS文件读入）
	GValueVector<Q_INT32> *mpShotList;		//对应炮关系列表-名称不同
	float mStatic;                          //静校正量
	bool mIsWellShot;
	/////////////延迟时法
	float mSurfaceDeph;
	float mPDelayTime[10];
	float mSVDelayTime[10];
	float mPDelayStatic;
	float mSVDelayStatic;
	float mLayerDepth[10];
	float mRefV[10];  //折射层速度数组
	int mRefCount;    //折射层数目
	int mFBTCount;    //可以使用的初至数目

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	float mRandomOffsetX;
	float mRandomOffsetY;
	//////////////////////////////////////////////////////////////////////////add by LiuWei
public:
	GSPoint();
	GSPoint(const QString &lineName, const QString &pointNum, 
			float spsX, float spsY, float spsZ, float spsStatic,
			float depth, float base, float tao);
	~GSPoint();

	//----得到炮线名称----
	QString getLineName()
	{ return mLineName; }
	//----得到点号----
	QString getPointNum()
	{ return mPointNum; }
	//----得到SPS文件X坐标----
	float getSPSX()
	{ return mSPSX; }
	// 设置SPS X坐标
	void setSPSX(float x)				{ mSPSX = x; }
	//----得到SPS文件Y坐标----
	float getSPSY()
	{ return mSPSY; }
	// 设置SPS Y坐标
	void setSPSY(float y)				{ mSPSY = y; }
	//----得到SPS文件Z坐标----
	float getSPSZ()
	{ return mSPSZ; }
	// 设置SPS Z坐标
	void setSPSZ(float z)				{ mSPSZ = z; }
	//----得到变换后X坐标----
	float getX()
	{ return mX; }
	//----得到变换后Y坐标----
	float getY()
	{ return mY; }
	//----得到基准面高度----
	float getBase()
	{ return mBase; }
	//----得到井深----
	float getDepth()
	{ return mDepth; }
	//----得到SPS野外静校正量----
	float getSPSStatic()
	{ return mSPSStatic; }
	//----得到TAO值----
	float getTAO()
	{ return mTAO; }
	//----得到炮关系数目----
	int count()
	{ return mpShotList->count(); }
	//----得到炮关系索引----
	int get(int index);
	//----添加炮关系索引（供GShot类调用）----
	bool append(Q_INT32 index)
	{ return mpShotList->append(index); }
	//----清空炮关系列表（供GShot类调用）----
	void clear()
	{ mpShotList->clear(); }

	// 设置和得到该炮对应的记录号[3/16/2023 Liu Wei]
	int getRecNum()	{ return mRecNum; }
	void setRecNum(int RecNum) { mRecNum=RecNum; } 

	//----旋转坐标----
	void rotate(float sinAF, float cosAF);
	//----平移坐标----
	void translate(float x, float y);
	//----保存----
	bool save(QDataStream &stream);
	//----载入----
	bool load(QDataStream &stream);
	void setStatic(float sta){mStatic = sta;}
	float getStatic(){return mStatic;}
	void setWellShotFlag(bool flag){mIsWellShot = flag;}
	bool getWellShotFlag(){return mIsWellShot;}
	////	/////////////延迟时法
	void setSurfaceDepth(float depth){mSurfaceDeph=depth;}
	float getSurfaceDepth(){return mSurfaceDeph;}
	void setDepthByIndex(int index,float depth){mLayerDepth[index] = depth;}
	float getDepthByIndex(int index){return mLayerDepth[index];}
	void setPDelayTimeByIndex(int index,float pdelay){mPDelayTime[index] = pdelay;}
	float getPDelayTimeByIndex(int index){return mPDelayTime[index];}
	void setSVDelayTimeByIndex(int index,float svdelay){mSVDelayTime[index] = svdelay;}
	float getSVDelayTimeByIndex(int index){return mSVDelayTime[index];}
    void setPDelayStatic(float pdelay){mPDelayStatic = pdelay;}
	float getPDelayStatic(){return mPDelayStatic;}
	void setSVDelayStatic(float svdelay){mSVDelayStatic = svdelay;}
	float getSVDelayStatic(){return mSVDelayStatic;}
	void setRefVByIndex(int index,float v){mRefV[index] = v;}
	float getRefVByIndex(int index){return mRefV[index];}
    void setRefCount(int count){mRefCount = count;}
	int getRefCount(){return mRefCount;}
    void setFBTCount(int count){mFBTCount = count;}
	int getFBTCount(){return mFBTCount;}
	////	/////////////延迟时法
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	void setRandomOffsetX(float offset){mRandomOffsetX = offset;}
	void setRandomOffsetY(float offset){mRandomOffsetY = offset;}
	float getRandomOffsetX(){return mRandomOffsetX;}//没有初始值，就是一个随机数啊？
	float getRandomOffsetY(){return mRandomOffsetY;}
	//////////////////////////////////////////////////////////////////////////add by LiuWei
};
//------------------------------------------------------------------------------
class GSLine
{
friend class GSLineSet;//add by zhaohu
private:
	QString mLineName;						//炮线号
	GPtrVector<GSPoint> *mpSPointList;		//炮点列表
public:
	GSLine();
	GSLine(const QString &lineName);
	~GSLine();

	//----得到线号----
	QString getLineName()
	{ return mLineName; }
	//----得到炮点数目----
	int count()
	{ return mpSPointList->count(); }
	//----得到指定下标的炮点----
	GSPoint * get(int index);
	//----根据点号得到炮点索引----
	int getIndexByPointNum(const QString &pointNum);
	//----根据点号得到炮点----
	GSPoint * getByPointNum(const QString &pointNum);
	//----追加炮点----
	bool append(GSPoint *pSPoint)
	{ return mpSPointList->append(pSPoint); }
	//----删除炮点----
	bool remove(int index)
	{ return mpSPointList->remove(index); }
	//----清空炮点列表----
	void clear()
	{ mpSPointList->clear(); }
	//----得到SPS文件坐标的范围（供GSLineSet类调用）----
	void getSPSRange(float &minX, float &minY, float &maxX, float &maxY);
	//----得到变换后坐标范围（供GSLineSet类调用）----
	void getRange(float &minX, float &minY, float &maxX, float &maxY);
	//----旋转坐标系，并返回旋转后最小坐标----
	void rotate(float sinAF, float cosAF, float &minX, float &minY);
	//----平移坐标系 ----
	void translate(float x, float y);
	//----清空所有炮点的炮关系列表----
	void clearShotList();
	//----按点号排序----
	void sort();
	//----保存----
	void save(QDataStream &stream);
	//----载入----
	void load(QDataStream &stream);
};
//------------------------------------------------------------------------------
class GSLineSet : public QObject
{
private:
	GPtrVector<GSLine> *mpSLineList;
public:
	GSLineSet();
	~GSLineSet();

	//----得到炮线数目----
	int count()
	{ return mpSLineList->count(); }
	//----得到指定下标的炮线----
	GSLine * get(int index)
	{ return mpSLineList->at(index); }
	//----根据炮线号得到炮线下标----
	int getIndexByLineName(const QString &lineName);
	//----根据炮线号得到炮线----
	GSLine * getByLineName(const QString &lineName);
	//----追加炮线----
	bool append(GSLine *pSLine)
	{ return mpSLineList->append(pSLine); }
	//----清空炮线列表----
	void clear()
	{ mpSLineList->clear(); }
	//----清除空炮点----
	void clearVainShots();
	//----得到SPS文件坐标的范围（供GProject类调用）----
	void getSPSRange(float &minX, float &minY, float &maxX, float &maxY);
	//----得到变换后坐标范围（供GProject类调用）----
	void getRange(float &minX, float &minY, float &maxX, float &maxY);
	//----旋转坐标系，并返回旋转后最小坐标----
	void rotate(float sinAF, float cosAF, float &minX, float &minY);
	//----平移坐标系 ----
	void translate(float x, float y);
	//----清空所有炮点的炮关系列表----
	void clearShotList();
	//----按点线号排序----
	void sort();
	//----保存----
	bool save(const QString &fileName);
	//----载入----
	bool load(const QString &fileName);
	//----载入野外静校正量----
	bool loadStatic(const QString &fileName);

	// 直接创建二维观测系统
	void create(GSPSParam *pSPSParam);//add by zhaohu
	void create(QVector<QVector<float> > &sxcrd, QVector<QVector<float> > &sycrd, GSPSParam *pSPSParam);//add by liuwei

	void createByold(GSLineSet *SLineSet);//add by zhaohu

	// 保存
	bool save(QDataStream &stream);//add by zhaohu
	// 载入
	bool load(QDataStream &stream);//add by zhaohu

	bool saveSFile2(const QString &fileName);//sps2.1格式 add by zhaohu
};

#endif
