#ifndef GPROJECT_H
#define GPROJECT_H

#include <qobject.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include "qfile.h"
#include "qdatastream.h"
#include "gshotset.h"
#include "gslineset.h"
#include "grlineset.h"
#include "gcmpset.h"
#include "../../gui/survey/gspsparam.h"
#include "../../core/gutils.h"
#include "../../core/gmath.h"

#include "../../geometry/gAttrAnalyzeParam.h"

#define PROJECT_TAG			QString("CDUT SEIMIC EVALUATE SYSTEM PROJECT")
#define PROJECT_VERSION		QString("1.5")

enum GProjectType {gProject2D, gProject3D};

class GCellGrid;//add by zhaohu


class GProject : public QObject
{
	Q_OBJECT	

public:
	QString mPrjFileName;				// add by zhaohu
	QString mPrjName;					// add by zhaohu 
	double mX0, mY0, mZ0;				// add by zhaohu ()
	double mLength, mWidth, mHeight;	// Ի,ߴ

	bool mIsNewImfo;                        // ,add by zhaohu

private:
	//------------------------------------------------------------------
	//----<1>ݿ޹----
	float mAF;										// 标定方位角（度）
	float mSinAF, mCosAF;							// 标定方位角的正/余弦
	float mMinX, mMinY, mMaxX, mMaxY;				// 工作区范围
	float mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY;	// SPS 坐标范围
	float mTransX0, mTransY0;						// 平移变换前的起始值
   	float mHCMPInv, mVCMPInv;						//CMP
	float mTrcInv, mLineInv;						// 道间距/线间距
	bool mIsNoSEGYFile;							    // SEG-Y 文件是否缺失
	static GProject * mpCurrentPtr;					// (original comment lost in an earlier encoding conversion)
	static GProject * mpCurrentPtrSV;               //sv

	//PWorkAreaList *mainFrmProList; // ,wyt
	//----<2>ݿ----
	GProjectType mProjectType;						//
	QString mProjectName;							//
	QString mProjectDir;							// (original comment lost in an earlier encoding conversion)
	///////////////////////////////////////////////////////////
	QString mProjectNameSV;
	QString mProjectDirSV;
	////////////////////////////////////////////////////////////
	QString mWorkAreaNo;							//
	QString mLineBindNo;							//ߺ
	float mBase;									//׼߳
	float mVelocity;								//ٶ
	bool mIsBend;									// (original comment lost in an earlier encoding conversion)
//	GEvaluateGrade mEvaluateGrade; // 2 /3
	bool mIsTFix;									//T־
	//--------------------------------------------------------------------------

	//----ݼ------------------------------------------------------------
	GShotSet *mpShotSet;							// (original comment lost in an earlier encoding conversion)
	GSLineSet *mpSLineSet;							//ݼ
	GRLineSet *mpRLineSet;							//첨ݼ
	QStringList mSEGYFileList;						//SEG-Y

	GCMPLineSet *mpCMPLineSet;                     //cmplineݼzhaohu
	GSPSParam *mpSPSParam;				// SPSָ룬zhaohu
	GCellGrid *mpCellGrid;				// Ԫzhaohu
	GShotSet *mpShotSetNew;				//( zhaohu
	GSLineSet *mpSLineSetNew;				//( zhaohu
	GRLineSet *mpRLineSetNew;				//( zhaohu
	GCMPLineSet *mpCMPLineSetNew;                     //cmpline( zhaohu

	///////////////
	GShotSet *mpShotSetSV;							// (original comment lost in an earlier encoding conversion)
	GSLineSet *mpSLineSetSV;							//ݼ
	GRLineSet *mpRLineSetSV;							//첨ݼ
	QStringList mSEGYFileListSV;						//SEG-Y
	//--------------------------------------------------------------------------

	//---- ----------------------------------------------------------
	//----<1>----
	float mAmpWidth, mTrcWidth, mTrcHeight;			// (original comment lost in an earlier encoding conversion)
	float mEquWin, mFixedGain;						// (original comment lost in an earlier encoding conversion)
//	GGainMode mGainMode; //
	Q_INT32 mFromTimeIndex, mToTimeIndex;			// (original comment lost in an earlier encoding conversion)
    float mR;
	float mH;
	//---------------------------------------------------------------------------
//	GRPointVList *mpRPointVList; //
//	GRPointVList *mpRPointPVList; //
	//////////
//	GRPointVList *mpRPointVListSV; //
//	GRPointVList *mpRPointPVListSV; //
//    PPORPSV mPSVFlag;    
	// (original comment lost in an earlier encoding conversion)
	//////////////////////////
	float mSinA;                                  //
	float mCosA;                                   //
    int mShotIndexCount;                          //
	float mMaxReceiverLineZLong;                  //첨߳
	float mLineInterv;   
	int mMinRecordNum;
	int mMaxRecordNum;
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	int mbeginfoldi;int mendfoldi;// (original comment lost in an earlier encoding conversion)
	int mbeginfoldj;int mendfoldj;// (original comment lost in an earlier encoding conversion)
	GAttrAnalyzeParam *mpAttrAnalyzeParam;          //?
	//////////////////////////////////////////////////////////////////////////add by LiuWei
public:
	GProject();
	~GProject();
	float getR(){return mR;}
	float getH(){return mH;}
	//------------------------------------------------
    static	GProject * create();		// (original comment lost in an earlier encoding conversion)
	static void destroy();			// (original comment lost in an earlier encoding conversion)
	static GProject * getCurrentProject(){return mpCurrentPtr;}	
	/////////////////////
	static GProject * createSV();		// (original comment lost in an earlier encoding conversion)
	static void destroySV();			// (original comment lost in an earlier encoding conversion)
	static GProject * getCurrentProjectSV()	{ return mpCurrentPtrSV; }
	////////////////////////////////////////
//    void setPSVFlag(PPORPSV flag);
	void setFlag();//Ի
	/////////////////////////////////////
	//---- ----
	void clear();
	//--------------------------------------------------------------------------

	//----̻Ժ----------------------------------------------------
	GProjectType getProjectType();
	float getAF()				{ return mAF; }
	float getSinAF()			{ return mSinAF; }
	float getCosAF()			{ return mCosAF; }
	float getMinX()				{ return mMinX; }
	float getMaxX()				{ return mMaxX; }
	float getMinY()				{ return mMinY; }
	float getMaxY()				{ return mMaxY; }
	float getMinSPSX()			{ return mMinSPSX; }
	float getMaxSPSX()			{ return mMaxSPSX; }
	float getMinSPSY()			{ return mMinSPSY; }
	float getMaxSPSY()			{ return mMaxSPSY; }
	float getTransX0()			{ return mTransX0; }
	float getTransY0()			{ return mTransY0; }
	void  setTransX0(float x0)	{mTransX0=x0;		}// ֵ [Liu Wei 6/20/2023]
	void  setTransY0(float y0)	{mTransY0=y0;		}// ֵ [Liu Wei 6/20/2023]
    float getHCMPInv()			{ return mHCMPInv; }
	float getVCMPInv()			{ return mVCMPInv; }
   // float getBase()				{ return mBase; }
//	float getVelocity()			{ return mVelocity; }
	float getBase()				{ return 550; }
	float getVelocity()			{ return 5200; }
	float getLineInv()			{ return mLineInv;}
	float getTrcInv()			{ return mTrcInv;}
	QString getWorkAreaNo()		{ return mWorkAreaNo; }
	QString getLineBindNo()		{ return mLineBindNo; }
//	GEvaluateGrade getEvaluateGrade() { return mEvaluateGrade; }
	bool getIsTFix()			{ return mIsTFix; }
	bool getIsNoSEGYFile()		{ return mIsNoSEGYFile; }

	void setBase( float base ) { mBase = base; }
	void setVelocity( float velocity ) { mVelocity = velocity; }

	QString getSPSFileName();//Ի
	double getX0()		{ return mX0; }//Ի
	double getY0()		{ return mY0; }//Ի
	double getZ0()		{ return mZ0; }//Ի
	double getLength()	{ return mLength; }//Ի
	double getWidth()	{ return mWidth; }//Ի
	double getHeight()	{ return mHeight; }//Ի

	//--------------------------------------------------------------------------

	//----ݼ----------------------------------------------------------
	// add by wushanzhou in 2026/10/15
	GShotSet * getShotSet()
	{
		if(mIsNewImfo && mpShotSetNew && mpShotSetNew->count() > 0)
			return mpShotSetNew;
		return mpShotSet;
	}
	GSLineSet * getSLineSet()
	{ 
		if(mIsNewImfo && mpSLineSetNew && mpSLineSetNew->count() > 0)
			return mpSLineSetNew;
		return mpSLineSet;
	}
	GRLineSet * getRLineSet()
	{ 
		if(mIsNewImfo && mpRLineSetNew && mpRLineSetNew->count() > 0)
			return mpRLineSetNew;
		return mpRLineSet;
	}
	GCMPLineSet *getCMPLineSet()
	{
		return mpCMPLineSet;    //cmplineݼzhaohu                 
	}

	GSPSParam * getSPSParam()
	{ return mpSPSParam; }//add by zhaohu

	GCellGrid * getCellGrid()
	{ return mpCellGrid; }//add by zhaohu

	GShotSet * getShotSetNew()	//add by zhaohu		
	{
		return mpShotSetNew;
	}
	GSLineSet * getSLineSetNew()	//add by zhaohu
	{ 
		return mpSLineSetNew;
	}
	GRLineSet * getRLineSetNew()//add by zhaohu
	{ 
		return mpRLineSetNew; 
	}
	GCMPLineSet *getCMPLineSetNew()
	{
		return mpCMPLineSetNew;    //cmpline zhaohu
	}
	GRLineSet * getOriginalRLineSet() { return mpRLineSet; } // add by wushanzhou in 2026/10/15
	GSLineSet * getOriginalSLineSet() { return mpSLineSet; }


	//--------------------------------------------------------------------------

	//------------------------------------------------------
		//---- ()----
    QString	getCMPVFileName(){return mProjectDir+"/"+mProjectName+".v";}
	QString getRStackFileName()
	{ 
//		if(mPSVFlag==PPDATA)
			return mProjectDir+"/"+mProjectName+".rstack";
//		else
//			return mProjectDirSV+"/"+mProjectNameSV+".rstack";
	}
	//---- ----
	QString getProjectFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		if(mProjectDir.isEmpty()) return NULL;
	    return mProjectDir+"/"+mProjectName+".ppr";
 //       else
//			return mProjectDirSV+"/"+mProjectNameSV+".ppr";
	}
	//---- ----
	QString getXFileName()
	{
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".x"; 
//		else
 //           return mProjectDirSV+"/"+mProjectNameSV+".x"; 
	}

	//---- ----
	QString getSFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".s";
//		else
 //         return mProjectDirSV+"/"+mProjectNameSV+".s";
	}
	//---- ----
	QString getRFileName()
	{
//		if(mPSVFlag==PPDATA)
		  return mProjectDir+"/"+mProjectName+".r";
//		else
//	     return mProjectDirSV+"/"+mProjectNameSV+".r"; 
	}
	//---- ----
	QString getFBTFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".fbt"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".fbt"; 
	}

	//---- ----
	QString getFBTFileNameSV()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".fbtSV"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".fbtSV"; 
	}
	//--- ----
	QString getStackFileName()
	{
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".segy";
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".segy";

	}

	//---- SPS----
	QString getSPSLogFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+"sps_log.txt"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+"sps_log.txt"; 
	}
	//---- SEG-Y----
	QString getSGYLogFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		    return mProjectDir+"/"+mProjectName+"sgy_log.txt"; 
//		else
//			return mProjectDirSV+"/"+mProjectNameSV+"sgy_log.txt"; 
	}
	//---- ----
	QString getImagePath()
	{ 
//		if(mPSVFlag==PPDATA)
			return mProjectDir+"/image"; 
//		else
 //    		return mProjectDirSV+"/image";
	}
	//---- SEG-Y ----
	int getSEGYFileCount()
	{ 
//		if(mPSVFlag==PPDATA)
			return mSEGYFileList.count(); 
//		else
//			return mSEGYFileListSV.count();
	}
	//---- SEG-Y----
	QString getSEGYFileName(int index);
	//----SEG-Y----
	void appendSEGYFileName(QString fileName);
	//----SEG-Y----
	void clearSEGYFileList();
	//---- ----
	bool createFolders(const QString &projectName, const QString &projectDir);
	bool  createFolders();
	//------------- -------------------------------------------------------------

	//----ݵ뺯--------------------------------------------------------
	//----SPS----
	bool appendSPSInfo(const QString &sFileName, const QString &rFileName, 
					   const QString &xFileName, QTextEdit *spsMemo = NULL,
					   QProgressBar *progressBar= NULL);
	//----SEG-Y----
	bool appendSEGYInfo(const QString &fileName, QProgressBar *progressBar = NULL, 
						QTextEdit *sgyMemo = NULL);
	//----
	void refresh();

	//----+-----------------------------------
	//--------
	bool saveProjectFile();
	//--------
	bool save();

	// SPS
	bool saveSPS(const QString &spsFileName);//add by wushanzhou in 2026/10/23



	//-------
	bool saveTime();
	//--------
	bool saveToDB(QProgressDialog *dlg);

	//--------
	bool loadProjectFile(const QString &fileName);
	//--------
	bool load(const QString &fileName);
	//--------
	bool reLoad();
	//----,----
	bool loadFromDB( const QString &lineBindNo );
	//--------------------------------------------------------------------------

	//------------------------------------------
	//void setMainFrmProList( PWorkAreaList *list )	{ mainFrmProList = list; }
	//PWorkAreaList *getMainFrmProList()				{ return mainFrmProList; }
	//--------------------------------------------------------------------------
	
	//---- --------------------------------------------------------
	//----<1> ----
	float getAmpWidth()			{ return mAmpWidth; }
	float getTrcWidth()			{ return mTrcWidth; }
	float getTrcHeight()		{ return mTrcHeight; }
//	int getEquWin()				{ return gTrunc(mEquWin); }
	float getFixedGain()		{ return mFixedGain; }
//	GGainMode getGainMode()		{ return mGainMode; }
	int getFromTimeIndex()		{ return mFromTimeIndex; }
	int getToTimeIndex()		{ return mToTimeIndex; }

	void setAmpWidth(float ampWidth)		{ mAmpWidth = ampWidth; }
	void setTrcWidth(float trcWidth)		{ mTrcWidth = trcWidth; }
	void setTrcHeight(float trcHeight)		{ mTrcHeight = trcHeight; }
	void setEquWin(float equWin)			{ mEquWin = equWin; }
	void setFixedGain(float fixed)			{ mFixedGain = fixed; }
//	void setGainMode(GGainMode mode)		{ mGainMode = mode; }
	void setFromTimeIndex(int index)		{ mFromTimeIndex = index; }
	void setToTimeIndex(int index)			{ mToTimeIndex = index; }

	void setHCMPInv(float HCMPInv)			{ mHCMPInv=HCMPInv; }//add by zhaohu
	void setVCMPInv(float VCMPInv)			{ mVCMPInv=VCMPInv; }//add by zhaohu

	//--------------------------------------------------------------------------
/*	GRPointVList *getRPointVList()
	{
		if(mPSVFlag==PPDATA)
			return mpRPointVList;
		else
			return mpRPointVListSV;
	}
	GRPointVList *getRPointPVList()
	{
		if(mPSVFlag==PPDATA)
		  return mpRPointPVList;
		else
		  return mpRPointPVListSV;
	}
	*/
	bool saveFBT();
//	bool saveFBT2();
	bool loadFBT();
//	bool loadFBT2();
	//////////////////////////
/*	void setProjectDir(QString projectDir)
	{
		if(mPSVFlag==PPDATA)
		 mProjectDir = projectDir;
		else
		 mProjectDirSV = projectDir;
	}
	void setProjectName(QString projectName)
	{
		if(mPSVFlag==PPDATA)
		 mProjectName = projectName;
		else
		 mProjectNameSV = projectName;
	}
*/	void saveProjectFileName(QString fileName);
	void loadProjectFileName(QString fileName);
	void setTrcInterv(float interv)
	{
	    mTrcInv = interv;
	}
//	void addRStatic();
//	void addSStatic();
//	void saveSStatic();
//	void saveRStatic();
//	void zeroRStatic();
	///////////////////////////////////////////////////
	void addRDiff();
	float getMaxRLineLong();
	int getShotIndexCount();
    void setRecorderNum(int n1,int n2){	mMinRecordNum = n1; mMaxRecordNum = n2;}
	int getMinRecordNum(){return mMinRecordNum;}
	int getMaxRecordNum(){return mMaxRecordNum;}
	bool scanForShotRecorderRange(const QString &fileName, 
												QProgressBar *progressBar, 
			                           QTextEdit *sgyMemo );
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	void setFoldArea(int beginfoldi,int endfoldi,int beginfoldj,int endfoldj)// (original comment lost in an earlier encoding conversion)
	{
		mbeginfoldi=beginfoldi;mendfoldi=endfoldi;mbeginfoldj=beginfoldj;mendfoldj=endfoldj;
	}
	void getFoldArea(int &beginfoldi,int &endfoldi,int &beginfoldj,int &endfoldj)// (original comment lost in an earlier encoding conversion)
	{
		beginfoldi=mbeginfoldi;endfoldi=mendfoldi;beginfoldj=mbeginfoldj;endfoldj=mendfoldj;
	}

	void setAttrAnalyzeParam(GAttrAnalyzeParam *param) { mpAttrAnalyzeParam=param;}
	GAttrAnalyzeParam *getAttrAnalyzeParam() {return mpAttrAnalyzeParam;}
	static double getAngle(double sinAF, double cosAF);
	//////////////////////////////////////////////////////////////////////////add by LiuWei
};

#endif
