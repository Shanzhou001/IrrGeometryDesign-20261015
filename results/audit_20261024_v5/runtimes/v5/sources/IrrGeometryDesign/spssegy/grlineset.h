#ifndef GRLINE_SET_H
#define GRLINE_SET_H

#include <qprogressdialog.h>
#include "qfile.h"
#include "qdatastream.h"
#include "gmath.h"
#include "gvaluevector.h"
#include "gptrvector.h"
#include <QTextStream>



enum GRPointState { rpSaved, rpUnSaved, rpUpdated };	//检波点存储状态
enum GREvaluateState { rpNeverEvaluate, rpEvaluate };	//检波点参评状态


// GRPoint存盘时的填充空字节数
#define GRPOINT_PAD_SIZE 32

// GRLine存盘时填充空字节数
#define GRLINE_PAD_SIZE 64

// GRLineSet存盘时填充空字节数
#define GRLINESET_PAD_SIZE 64

class GSPSParam;//add by zhaohu

class GProject;
class GRPoint
{
friend class GRLineSet;
friend class GShotSet;
private:
	GRPointState mState;					//存储状态
	Q_INT32 mPosState;						//1位置正确, 0位置不正确
	QString mLineName;						//线号
	QString mPointNum;						//点号
	float mSPSX, mSPSY, mSPSZ;				//SPS文件中X,Y,Z
	float mX, mY;							//映射坐标
	float mBase;							//基准高
	GValueVector<Q_INT32> *mpShotList;		//炮关系索引列表
    GREvaluateState mEvaluateState;			//参评状态
	float mSPSStatic;						//野外静校正量(ms)
	float mStatic;
	GProject *mpProject;
	//////////////初至拟合静校正量
	float mFitFBTStatic;
    float mPSVTimeDiff;
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
public:
	GRPoint();
	GRPoint(const QString &lineName, const QString &pointNum, 
			float spsX, float spsY, float spsZ, float spsStatic, float base);
	~GRPoint();
	float getStatica(){return mStatic;}
	void setStatic(float sta){mStatic = sta;}
	
	//----属性函数组------------------------------------------------------------
	//----得到存盘状态----
	GRPointState getState()		{ return mState; }
	//----设置存盘状态----
	void setState(GRPointState state) { mState = state; }
	//----得到检波线名称----
	QString getLineName()		{ return mLineName; }
	//----得到点号----
	QString getPointNum()		{ return mPointNum; }
	//----得到检波点桩号----
	QString getStackNum()		{ return mLineName+mPointNum; }
	//----得到SPS文件X坐标----
	float getSPSX()				{ return mSPSX; }
	//----得到SPS文件Y坐标----
	float getSPSY()				{ return mSPSY; }
	//----得到SPS文件Z坐标----
	float getSPSZ()				{ return mSPSZ; }
	//----得到变换后X坐标----
	float getX()				{ return mX; }
	//----得到变换后Y坐标----
	float getY()				{ return mY; }
	//----得到基准面高度----
	float getBase()				{ return mBase; }
	//----得到炮关系数目----
	int count()					{ return mpShotList->count(); }
	//----得到炮关系索引----
	int get(int index);
	//----添加炮关系索引（供GShot类调用）----
	bool append(Q_INT32 index)	{ return mpShotList->append(index); }
	//----清空炮关系列表（供GShot类调用）----
	void clear()				{ mpShotList->clear(); }
	//----得到静校量（单位ms）----
	float getStatic();
	//----得到野外静校正量（单位ms）----
	float getSPSStatic()		{ return mSPSStatic; }
	//----得到位置状态----
	int getPosState()			{ return mPosState; }
	//----设置位置状态----
	void setPosState(int state) { mPosState = state; }
	//----获取参评状态----
	int getEvaluateState()		{ return mEvaluateState; }
	//----设置参评状态----
	void setEvaluateState(GREvaluateState state) { mEvaluateState = state; }
	//--------------------------------------------------------------------------

	//----坐标变换函数组--------------------------------------------------------
	//----旋转坐标----
	void rotate(float sinAF, float cosAF);
	//----平移坐标----
	void translate(float x, float y);
	//----保存----
	bool save(QDataStream &stream);
	//----载入----
	bool load(QDataStream &stream);
	//--------------------------------------------------------------------------
	void setProject(GProject *pProject){mpProject = pProject;}
	////////
	float getFitFBTStatic(){return mFitFBTStatic;}
	void setFitFBTStatic(float fitFBTStaitc){mFitFBTStatic = fitFBTStaitc;}
	//////////为了初至拾取，提供纵波和转换波之间的延迟时差
	void setTimeDiff(float diff){mPSVTimeDiff=diff;}
    float getTimeDiff(){return mPSVTimeDiff; }
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
};
//------------------------------------------------------------------------------
class GRLine								//检波线类
{
friend class GRLineSet;
friend class G3DExtractView;
private:
	QString mLineName;						//检波线号
	GPtrVector<GRPoint> *mpRPointList;		//检波点列表
public:
	GRLine(bool isAtuoDel = true);
	GRLine(const QString &lineName, bool isAutoDel = true);
	~GRLine();

	//----得到线号----
	QString getLineName()		{ return mLineName; }

	// 设置检波线名称
	void setLineName(const QString &name)// add by zhaohu
	{ mLineName = name; }

	//----得到检波点数目----
	int count()					{ return mpRPointList->count(); }
	//----得到指定下标的检波点----
	GRPoint * get(int index)	{ return mpRPointList->at(index); }
	//----根据点号得到检波点索引----
	int getIndexByPointNum(const QString &pointNum);
	//----根据点号得到检波点----
	GRPoint * getByPointNum(const QString &pointNum);
	//----追加检波点----
	bool append(GRPoint *pSPoint)
	{ return mpRPointList->append(pSPoint); }
	//----删除指定下标检波点----
	bool remove(int index)
	{ return mpRPointList->remove(index); }
	//----清空检波点列表----
	void clear()
	{ mpRPointList->clear(); }
	//----清空炮关系表----
	void clearShotList();
	//----得到SPS文件坐标的范围（供GRLineSet类调用）----
	void getSPSRange(float &minX, float &minY, float &maxX, float &maxY);
	//----得到变换后坐标范围（供GRLineSet类调用）----
	void getRange(float &minX, float &minY, float &maxX, float &maxY);
	//----旋转坐标系，并返回旋转后最小坐标----
	void rotate(float sinAF, float cosAF, float &minX, float &minY);
	//----平移坐标系 ----
	void translate(float x, float y);
	//----求检波线倾角所成sinAF，和cosAF----
	bool getAngleParam(float &sinAF, float &cosAF);
	//----重新按旋转X坐标排序----
	void sortByX();
	//----重新按旋转Y坐标排序----
	void sortByY();
	//----保存----
	void save(QDataStream &stream);
	//----载入----
	void load(QDataStream &stream);
	void sortByPointNum();


};
//------------------------------------------------------------------------------
class GRLineSet							//检波线数据集类
{
private:
	GPtrVector<GRLine> *mpRLineList;	//检波线列表
public:
	GRLineSet();
	~GRLineSet();

	//----得到检波线数目----
	int count()					{ return mpRLineList->count(); }
	//----得到指定下标的检波线----
	GRLine * get(int index)		{ return mpRLineList->at(index); }
	//----根据检波线号得到检波线下标----
	int getIndexByLineName(const QString &lineName);
	//----根据检波线号得到检波线----
	GRLine * getByLineName(const QString &lineName);
	//----追加检波线----
	bool append(GRLine *pSLine)	{ return mpRLineList->append(pSLine); }
	//----清空检波线列表----
	void clear()				{ mpRLineList->clear(); }
	//----清空炮关系表----
	void clearShotList();
	//----得到SPS文件坐标的范围（供GProject类调用）----
	void getSPSRange(float &minX, float &minY, float &maxX, float &maxY);
	//----得到变换后坐标范围（供GProject类调用）----
	void getRange(float &minX, float &minY, float &maxX, float &maxY);
	//----旋转坐标系，并返回旋转后最小坐标----
	void rotate(float sinAF, float cosAF, float &minX, float &minY);
	//----平移坐标系 ----
	void translate(float x, float y);
	//----求检波线列表平均倾角所成sinAF，和cosAF----
	bool getAngleParam(float &sinAF, float &cosAF);
	//----保存----
	bool save(const QString &fileName);
	//----将报表信息存入数据库（北京石油大学接口函数）----
	bool saveToDB(const QString &workAreaNo, const QString &lineBindNo,
				  QProgressDialog *dlg);
	//----载入----
	bool load(const QString &fileName);
	//----抽取inline（该函数实际是为pSrcRLine复制副本）----
	void extractInline(GRLine *pSrcRLine, GRLine *pDestRLine);
	//----抽取crossline----
	void extractCrossline(float x, float diffx, GRLine *pRLine);
	//----载入野外静校正量----
	bool loadStatic(const QString &fileName);

	// 创建检波线
	void create(GSPSParam *pSPSParam);//add by zhaohu
	void create(QVector<QVector<float> > &xcrd, QVector<QVector<float> > &ycrd, GSPSParam *pSPSParam);//add by liuwei
	// 保存
	bool save(QDataStream &stream);//add by zhaohu 

	// 载入
	bool load(QDataStream &stream);//add by zhaohu

	// 保存为R文件
	bool saveRFile2(const QString &fileName);//sps2.1格式add by zhaohu


};

#endif
