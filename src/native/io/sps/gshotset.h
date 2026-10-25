#ifndef GSHOT_SET_H
#define GSHOT_SET_H

#include <qobject.h>
#include <qtextedit.h>
#include "gptrvector.h"
#include "qiodevice.h"
#include "qfile.h"
#include "qdatastream.h"
#include "qprogressdialog.h"
#include "grlineset.h"
#include "gslineset.h"
#include "../../core/gutils.h"

// GShotXArray存盘时的填充空字节数
#define GXARRAY_PAD_SIZE 16

// GShotXLine存盘时填充空字节数
#define GXLINE_PAD_SIZE 16

// GShot存盘时填充空字节数
#define GSHOT_PAD_SIZE 32

// GShotSet存盘时填充空字节数
#define GSHOT_SET_PAD_SIZE 64

class GProject;
class GShotSet;
class GShot;
class GShotXLine;
class GShotXArry;
class GRLine;
class GRPoint;
class GSPoint;

enum GShotState { siSaved, siUnSaved, siUpdated };	//炮关系存储状态
enum GSEvaluateState { NeverEvaluated, UnEvaluated, Evaluated }; //评价状态 
//------------------------------------------------------------------------------
class GShotXArray
{
friend GShotSet;
public:
	enum GShotXArrayType { gLeftXArray, gRightXArray };	//排列类型
private:
	bool mIsCOPXArray;						//是否是共偏移距结构
	GShotXArrayType mArrayType;				//左右排列标志
	Q_INT32 mMinTrcIndex, mMaxTrcIndex;		//最大、最小道下标
	Q_INT32 mFirstTrc, mLastTrc, mTrcInc;	//起始道号,终止道号,道号增量
	QString mFirstPt, mLastPt;				//起始点号,终止点号
	Q_INT32 mFirstPtIndex, mLastPtIndex;	//起始检波点下标,终止检波点下标
	Q_INT32 mTrcCount;						//道数 (mLastPt-mFirstPt)/mIncPt+1
	long long mFilePos;				//对应SEGY文件位置
	Q_INT32 *mPriTimeArray;					//初至数组
	float mFirstSDistance;		//排列第一道与炮点映射距离
	float mShotX;							//炮点映射坐标(仅对共偏移距有用)
    GShot  *mpShot;
	float mSArrayDistance;					//炮点与排列的距离
	/////////////////////////////////
	Q_INT32 *mPriTimeArraySV;					//转换波初至数组
	//////////////////////////////////////////////////////////////////////
	float *mFitDifference;                 //拟合差

private:
	//----将非单一排列，划分成两个排列，返回新生成的排列（仅供GShotSet调用）----
	GShotXArray * splitXArray(float sx, GRLine *pRLine);
public:
	GShotXArray();//构造函数 1
	GShotXArray(QString firstPt, QString lastPt, Q_INT32 firstTrc, Q_INT32 lastTrc,
				Q_INT32 trcInc, Q_INT32 trcCount);//构造函数 2
	~GShotXArray();
	float getSArrayDistance()
	{ return mSArrayDistance; }
	GShot *getShot(){return mpShot;}
	float getShotX(){return mShotX;}
	//----得到排列类型(左/右)
	GShotXArrayType getArrayType() { return mArrayType; }
	//----得到最小道下标----
	int getMinTrcIndex() { return mMinTrcIndex; }
	//----得到最大道下标----
	int getMaxTrcIndex() { return mMaxTrcIndex; }
	//----得到排列第一道----
	int getFirstTrc() { return mFirstTrc; }
	//----得到排列最后一道----
	int getLastTrc() { return mLastTrc; }
	//----得到道增量----
	int getTrcInc() { return mTrcInc; }
	//----得到排列第一检波点----
	QString getFirstPt() { return mFirstPt; }
	//----得到排列最后一个检波点----
	QString getLastPt() { return mLastPt; }
	//----得到第一检波点在GRLine中的下标----
	int getFirstPtIndex() { return mFirstPtIndex; }
	//----得到最后一个检波点在GRLine中的下标----
	int getLastPtIndex() { return mLastPtIndex; }
	//----得到排列总道数----
	int getTrcCount() { return mTrcCount; }
	//----得到第一道在SEG-Y文件中的位置----
	long long getFilePos() { return mFilePos; }
	//----设置第一道在SEG-Y文件中的位置----
	void setFilePos(long long pos) { mFilePos = pos; }
	//----得到初至数组----
	int * getPriTimeArray() { return mPriTimeArray; }
	//-----得到转换波初至数组
	int * getPriTimeArraySV() { return mPriTimeArraySV;}
	//----判断排列是否单一（仅是左支或右支）----
	bool isSingle(float sx, GRLine *pRLine);
	//----根据位置信息设置排列类型----
	void setArrayType(float sx, GRLine *pRLine);
	//----保存----
	void save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV);
	//----载入----
	void load(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV);
	//----仅保存初至----
	void savePriTime(QDataStream &tstream);
	//----保存转换波初至----
	void savePriTimeSV(QDataStream &tstream);
	//----保存初至外的其他信息----
	void save(QDataStream &stream);
	//
	float getFirstTrcDistance()
	{ return mFirstSDistance; }
	//////////////////////////////
	float *getFitDifference(){return mFitDifference;}

	// 载入
	bool load(QDataStream &stream);//add by zhaohu
};
//------------------------------------------------------------------------------
class GShotXLine
{
friend class GShotSet;
private:
	QString mLineName;						//接收线号
	Q_INT32 mLineIndex;						//接收线下标
	Q_INT32 mMinTrc, mMaxTrc;				//最小、最大道号
	Q_INT32 mMinIndex, mMaxIndex;			//最小、最大道下标
	Q_INT32 mTrcCount;						//道数 (mMaxIndex-mMinIndex)+1
	GPtrVector<GShotXArray> *mpXArrayList;	//关系排列列表(size <= 2)
public:
	GShotXLine();
	GShotXLine(const QString &lineName);
	~GShotXLine();

	//----得到检波线名称----
	QString getLineName() { return mLineName; }
	//----得到检波线下标----
	int getLineIndex() { return mLineIndex; }
	//----得到该线上最小道号----
	int getMinTrc() { return mMinTrc; }
	//----得到该线上最大道号----
	int getMaxTrc() { return mMaxTrc; }
	//----得到该线上最小道下标----
	int getMinIndex() { return mMinIndex; }
	//----得到该线上最大道下标----
	int getMaxIndex() { return mMaxIndex; }
	//----得到道数----
	int getTrcCount() { return mTrcCount; }
	//----得到排列个数----
	int count() { return mpXArrayList->count(); }
	//----通过下标访问关系排列指针----
	GShotXArray * get(int index);
	//----通过第一道道号访问关系排列----
	GShotXArray * getXArrayByTrc( Q_INT32 firstTrc );
	//----追加关系排列（供GXFileScanner调用)----
	bool append(GShotXArray *pXArray);
	//----保存----
	void save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV);
	//----载入----
	void load(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV);
	//----保存初至文件----
	void savePriTime(QDataStream &tstream);
	//----保存转换波初至文件---
	void savePriTimeSV(QDataStream &tstream);
	//----保存初至外的其他信息----
	void save(QDataStream &stream);

	// 载入
	bool load(QDataStream &stream);//add by zhaohu
};
//------------------------------------------------------------------------------
class GShot
{
friend class GShotSet;
friend class GEvaluateDoc;
friend class GAutoEvaluateDoc;
private:
	GProject *mpProject;
	//----炮基本属性------------------------------------------------------------
	GShotState mState;						//炮索引存储状态
	Q_INT32 mPosState;						//位置状态(1正确,0错误)
	GSEvaluateState mEvaluateState;			//跑评价状态
	Q_INT32 mDate;							//日期
	Q_INT32 mRecNum;						//野外记录号（主索引），一炮对应一个记录号
	Q_INT32 mRecNumZH;						//野外记录号（主索引），存储实际的野外记录号
	Q_INT32 mIndex;							//在GShotSet列表中的索引
	QString mLineName;						//炮线号
	QString mPointNum;						//点号
	Q_INT32 mLineIndex;						//炮线号索引（用于确定位置信息）
	Q_INT32 mPointIndex;					//炮点号索引（用于确定位置信息）
	Q_INT32 mTrcCount;						//道总数
	Q_INT32 mFileIndex;						//SEG-Y序号
	long long mFilePos;				//对应SEG-Y文件位置
//	QIODevice::Offset mDataPos;				//数据道起始位置
	long long mDataPos;				//数据道起始位置,add by zhaohu
	QIODevice::Offset mTBPos;				//TB道在SEGY文件中的开始位置
	QIODevice::Offset mTAOPos;				//TAO道在SEGY文件中的开始位置
	Q_INT32 mCPCount;						//采样点数，对于同一炮而言为恒值
	Q_INT32 mCPInv;							//采样间隔，对于同一炮而言为恒值
	GPtrVector<GShotXLine> *mpXLineList;	//关系线列表
	//--------------------------------------------------------------------------

	GSPoint *mpSPoint;						// add by zhaohu 所关联的炮指针
	
//	PPORPSV mPSVFlag;
	////////////////////////////
	float *mQCFBT;                           //初至质量数组
	int *mTrcNum;                            //道号数组
	int *mRLIndex;                            //检波线下标列表
	int *mRPIndex;                            //检波点下标列表
protected:
	//----初始化变量----
	void init();
public:
	void setProject(GProject *pProject){mpProject = pProject;}
	GShot();
	GShot(Q_INT32 recNum, const QString &lineName, const QString &pointNum);
	~GShot();

	//----炮基本属性函数--------------------------------------------------------
	//----得到位置状态----
	int getPosState()				{ return mPosState; }
	//----设置位置状态----
	void setPosState(int state)		{ mPosState = state; }
	//----得到存盘状态----
	GShotState getState()			{ return mState; }
	//----设置存盘状态----
	void setState(GShotState state) { mState = state; }
	//----SPS文件中施工日期----
    int getDate()					{ return mDate;}
	//----得到该炮在GShotSet中的下标
	int getIndex()					{ return mIndex; }
	//---得到该炮的线号索引索引
	int getLtIndex()				{ return mLineIndex; }//add by zhaohu

	//---得到该炮的点号索引
	int getPtIndex()				{ return mPointIndex; }
	//----得到野外记录号----
	int getRecNum()					{ return mRecNum; }
	//----得到线号----
	int getRecNumZH()					{ return mRecNumZH; }//zhaohu 添加
	//----得到线号----zhaohu 添加
	QString getLineName()			{ return mLineName; }
	//----得到点号----
	QString getPointNum()			{ return mPointNum; }
	//---得到炮桩号
	QString getStackNum()			{ return mLineName+mPointNum; }
	//----得到炮点对象----
	GSPoint * getSPoint();
	// 设置炮点对象
	void setSPoint(GSPoint *pSPt)	{ mpSPoint = pSPt; }//add by zhaohu

	//----得到采样点数----
	int getCPCount()				{ return mCPCount; }
	//----设置采样点数----
	void setCPCount(int cpCount);
	//----得到采样间隔----
	int getCPInv()					{ return mCPInv; }
	//----设置采样间隔----
	void setCPInv(int cpInv)		{ mCPInv = cpInv; }
	//----得到道总数----
	int getTrcCount()				{ return mTrcCount; }
	//----得到SEG-Y文件索引----
	int getFileIndex()				{ return mFileIndex; }
	//----设置SEG-Y文件索引----
	//	void setFileIndex(int index)	{ mFileIndex = index; }
	void setFileIndex(int index);
	//----得到SEG-Y文件位置----
	long long getFilePos()	{ return mFilePos; }
	//----设置SEG-Y文件位置
	void setFilePos(long long pos)	{ mFilePos = pos; }
	//----得到数据道在SEG-Y文件中的位置----
//	QIODevice::Offset getDataPos()	{ return mDataPos; }
	long long  getDataPos()	{ return mDataPos; }
	//----设置数据道在SEG-Y文件中的位置----add by zhaohu
//	void setDataPos(QIODevice::Offset pos)	{ mDataPos = pos; }
	void setDataPos(long long pos)	{ mDataPos = pos; }
	//----得到TB道在SEG-Y文件中的位置----add by zhaohu
	QIODevice::Offset getTBPos()	{ return mTBPos; }
	//----设置TB道在SEG-Y文件中的位置----
	void setTBPos(QIODevice::Offset pos)	{ mTBPos = pos; }
	//----得到TAO道在SEG-Y文件中的位置----
	QIODevice::Offset getTAOPos()	{ return mTAOPos; }
	//----设置TB道在SEG-Y文件中的位置----
	void setTAOPos(QIODevice::Offset pos)	{ mTAOPos = pos; }
	//----获取静校正（单位ms）----
	float getStatic();
	//----获取SPS静校正（单位ms）----
	float getSPSStatic()			{ return getSPoint()->getSPSStatic(); }
	//----得到TAO值----
	float getTAO()					{ return getSPoint()->getTAO(); }
	//--------------------------------------------------------------------------

	//----以下为关系线相关函数--------------------------------------------------
	//----得到关系线条数----
	int count()
	{ return mpXLineList->count(); }
	//----通过下标访问关系线----
	GShotXLine * get(int index);
	//----通过检波线名得到关系线----
	GShotXLine * getXLineByName(const QString &lineName);
	//----追加炮关系线（供GXFileScanner调用)----
	bool append(GShotXLine *pXLine)
	{ return mpXLineList->append(pXLine); }
	//--------------------------------------------------------------------------

	//----以下为炮、道关系函数--------------------------------------------------
	//----通过道下标得到关系线----
	GShotXLine * getXLineByTrcIndex(int index);
	//----通过道下标得到关系排列----
	GShotXArray * getXArrayByTrcIndex(int index);
	//----根据道下标得道关系（关系线、关系排列）----
	bool getTrcInfoByIndex(int index, int &trcNum, GShotXLine *pXLine, GShotXArray *pXArray);
	//----通过道下标得到对应检波点----
	GRPoint * getRPointByTrcIndex(int  index );
	GRPoint * getRPointByTrcIndex(int  index, GRLineSet *pRLineSet );
//	GRPoint * getRPointByTrcIndex(int index,GDataStream *dataStream);
	//----通过道号获得道文件位置（为以前版本兼容，建议使用getTrcOffsetByIndex）----
	long long getTrcOffset(int trc);
	//----通过道下标获得道文件位置----
	long long getTrcOffsetByIndex(int index);
	//----根据道下标得道号----
	bool getTrcByIndex(int index, int &trcNum);
	//----根据道号得道下标----
	bool getIndexByTrc(int &index, int trcNum);
	//----根据检波点得道下标----
	bool getTrcIndexByRPoint(GRPoint *pRPoint, int &trcIndex);
	//----根据道下标获取炮道距离----
	float getTrcDistance(int index);
	//----得到下标处初至指针----
	Q_INT32 * getPriTimePtr(int index);
	Q_INT32 *getFBTByIndex(int index){return getPriTimePtr(index);}
	void setFBTDataByIndex(int index,int data)
	{int *fbt = getPriTimePtr(index); *fbt = data;}
	//
	Q_INT32 * getPriTimePtrSV(int index);
	//
	float *getFitDiference(int index);
	//--------------------------------------------------------------------------

	//----文件、数据库函数组----------------------------------------------------
	//----保存到流----
	void save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV);
	//----保存初至到流----
	void savePriTime( QDataStream &tstream );
	//
	void savePriTimeSV( QDataStream &tstream);
	//----保存炮点基本信息----
	void save(QDataStream &stream);
	//----从流中加载----
	void load(QDataStream &stream, QDataStream &tstream, QDataStream &tstreamSV);	

//	void setPSVFlag(PPORPSV psvFlag){mPSVFlag = psvFlag;}
	////////////////////////////
	float getQCFBTByIndex(int index);
	void setQCFBTByIndex(int index,float data);
	int getTrcIndexByRPoint(GRPoint * rPoint);
    bool getWellShotFlag(){return getSPoint()->getWellShotFlag();}
	int getTrcNumByIndex(int index);
	void setTrcNumByIndex(int index,int trcNum);
    //add by psl 110119
	int setRLineAndPointIndex(QDataStream *dataStream);

	// 从流中加载
	bool load(QDataStream &stream, GSLineSet *pSLineSet);//add by zhaohu
};
//------------------------------------------------------------------------------

class GShotLine		//炮关系线类（临时数据结构，保存抽取的部分炮关系）
{
private:
	GPtrVector<GShot> *mpShotList;
public:
	GShotLine();	//与GSLine不同，不执行自动删除功能
	~GShotLine();

	void clear()
	{ mpShotList->clear(); }

	int count()
	{ return mpShotList->count(); }

	GShot * get(int index)
	{ return mpShotList->at(index); }

	int getIndexByRecNum(int recNum);
	GShot * getByRecNum(int recNum);

	bool append(GShot *pShot)
	{ return mpShotList->append(pShot); }

	bool remove(int index)
	{ return mpShotList->remove(index); }

	void sortByX();
	void sortByY();
};
//------------------------------------------------------------------------------
class GShotSet : public QObject
{
private:
	GPtrVector<GShot> *mpShotList;
//	PPORPSV mPSVFlag;
	GProject *mpProject;
public:
	GShotSet();
	~GShotSet();

	void clear();

	int count()
	{ return mpShotList->count(); }

	GShot * get(int index);
	int getIndexByRecNum(Q_INT32 recNum);
	GShot * getByRecNum(Q_INT32 recNum);
	GShot * getByLinePoint(const QString &lineName, const QString &pointNum);  //2026.3.31 by Wushanzhou

	//----追加炮关系（供GXFileScanner调用)----
	bool append(GShot *pShot)
	{ return mpShotList->append(pShot); }

	//----检查炮点信息完整性，自动删除信息不完整的炮关系----
	//----记录炮线号、点号索引，为炮点GSPoint追加炮关系----
	void checkSInfo( QTextEdit *spsMemo = NULL );

	//----检查检波点信息完整性，自动删除信息不完整的炮关系----
	//----计算炮道总数，填充关系线对应线序号，填充关系排列点序号----
	void checkRInfo(QTextEdit *spsMemo = NULL,QProgressBar *progressBar= NULL );

	//----检查SEG-Y信息完整性，自动删除信息不完整的炮关系----
	void checkSEGYInfo(QTextEdit *sgyMemo = NULL);

	//----抽取inline方向炮线----
	void extractInline(float y, float diffy, GShotLine *pSLine);
	//----2D抽取,将所有炮索引放入pSLine中----
	void extractInline2D(GShotLine *pSLine);
	//----抽取crossline方向炮线----
	void extractCrossline(float x, float diffx, GShotLine *pSLine);
	//----按合成桩号排序（以线号、点号有序）----
	void sort();
	//----抽取共检波点----
	void extractCRP(GRLineSet *pRLineSet);
	//----抽取共炮点（可能在同一点放多炮，造成同以SPoint对应多个Shot）----
	void extractCSP(GSLineSet *pSLineSet);

	bool save(const QString &fileName);

	bool save(const QString &fileName, const QString &tfileName, 
		       const QString &tfileNameSV);
	bool savePriTime(const QString &tfileName);

	bool savePriTimeSV(const QString &tfileName);

	bool load(const QString &fileName, const QString &tfileName,
		              const QString &tfileNameSV);
	bool saveEvaluate(const QString &efileName);
	bool saveToDB(const QString &workAreaNo, const QString &lineBindNo,
				  QProgressDialog *dlg );
//	void setPSVFlag(PPORPSV psvFlag){mPSVFlag = psvFlag;}
	void setProject(GProject *pProject){mpProject = pProject;}
	void sortByLineAndPoint();
	void sortByPointAndLine();
	void sortByRec();
	///add by psl 110119
	void  calculateRIndex();


	// 直接创建炮集
	void create(GSPSParam *pSPSParam, GSLineSet *pSLineSet, GRLineSet *pRLineSet,GShotSet *pShotSet);//add by zhaohu

	void create(GSPSParam *pSPSParam, GSLineSet *pSLineSet, GRLineSet *pRLineSet);//  [Liu Wei 5/31/2023]

	// 保存炮集索引文件
	bool save(QDataStream &stream);//add by zhaohu

	// 载入炮集索引文件
	bool load(QDataStream &stream, GSLineSet *pSLineSet);//add by zhaohu

	int getRecNumByLineNamePointName(QString LineName,QString PointName);//根据炮线号和炮点好返回ffid号add by zhaohu

	// 输出.x文件
	bool saveXFile2(const QString &fileName,GSLineSet *pSLineSet, GRLineSet *pRLineSet);//sps2.1版本add by zhaohu

};

#endif
