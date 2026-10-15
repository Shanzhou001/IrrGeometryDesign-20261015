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
#include "gspsparam.h"
#include "gutils.h"
#include "gmath.h"

#include "gAttrAnalyzeParam.h"

#define PROJECT_TAG			QString("CDUT SEIMIC EVALUATE SYSTEM PROJECT")
#define PROJECT_VERSION		QString("1.5")

enum GProjectType {gProject2D, gProject3D};

class GCellGrid;//add by zhaohu


class GProject : public QObject
{
	Q_OBJECT	

public:
	QString mPrjFileName;				// add by zhaohu ļ
	QString mPrjName;					// add by zhaohu 
	double mX0, mY0, mZ0;				// add by zhaohu ½(С)
	double mLength, mWidth, mHeight;	// Ի,ߴ

	bool mIsNewImfo;                        //Ƿ񷵻½Ĺ۲Ϣı־,add by zhaohu

private:
	//------------------------------------------------------------------
	//----<1>ݿ޹----
	float mAF;										//бǶȣȣ
	float mSinAF, mCosAF;							//бǲ
	float mMinX, mMinY, mMaxX, mMaxY;				//ӳ󹤳̷Χ
	float mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY;	//SPSļй̷Χ
	float mTransX0, mTransY0;						//ƽƱ任ǰСֵ
   	float mHCMPInv, mVCMPInv;						//CMP
	float mTrcInv, mLineInv;						//Ƶ,߼
	bool mIsNoSEGYFile;							    //SEGYļǷȱʧ
	static GProject * mpCurrentPtr;					//ǰָ
	static GProject * mpCurrentPtrSV;               //svϹָ

	//PWorkAreaList *mainFrmProList;				//еĿб,wyt
	//----<2>ݿ----
	GProjectType mProjectType;						//
	QString mProjectName;							//
	QString mProjectDir;							//Ŀ¼
	///////////////////////////////////////////////////////////
	QString mProjectNameSV;
	QString mProjectDirSV;
	////////////////////////////////////////////////////////////
	QString mWorkAreaNo;							//
	QString mLineBindNo;							//ߺ
	float mBase;									//׼߳
	float mVelocity;								//ٶ
	bool mIsBend;									//ǷΪ
//	GEvaluateGrade mEvaluateGrade;					//۵ȼ2ȼ/3ȼ
	bool mIsTFix;									//T־
	//--------------------------------------------------------------------------

	//----ݼ------------------------------------------------------------
	GShotSet *mpShotSet;							//ڹϵݼ
	GSLineSet *mpSLineSet;							//ݼ
	GRLineSet *mpRLineSet;							//첨ݼ
	QStringList mSEGYFileList;						//SEG-Yļб

	GCMPLineSet *mpCMPLineSet;                     //cmplineݼzhaohu
	GSPSParam *mpSPSParam;				// SPSָ룬zhaohu
	GCellGrid *mpCellGrid;				// Ԫzhaohu
	GShotSet *mpShotSetNew;				//ڹϵݼ(½ģzhaohu
	GSLineSet *mpSLineSetNew;				//ݼ(½ģzhaohu
	GRLineSet *mpRLineSetNew;				//첨ݼ(½ģzhaohu
	GCMPLineSet *mpCMPLineSetNew;                     //cmplineݼ(½ģzhaohu

	///////////////
	GShotSet *mpShotSetSV;							//ڹϵݼ
	GSLineSet *mpSLineSetSV;							//ݼ
	GRLineSet *mpRLineSetSV;							//첨ݼ
	QStringList mSEGYFileListSV;						//SEG-Yļб
	//--------------------------------------------------------------------------

	//----Ĭ----------------------------------------------------------
	//----<1>ͼͼ----
	float mAmpWidth, mTrcWidth, mTrcHeight;			//ͼ
	float mEquWin, mFixedGain;						//ʱС
//	GGainMode mGainMode;							//淽ʽ
	Q_INT32 mFromTimeIndex, mToTimeIndex;			//ͼʱ
    float mR;
	float mH;
	//---------------------------------------------------------------------------
//	GRPointVList *mpRPointVList;                 //ݼ첨㶨ĵٶϢ
//	GRPointVList *mpRPointPVList;                 //ݼ첨㶨ݲٶϢ
	//////////
//	GRPointVList *mpRPointVListSV;                 //ݼ첨㶨ĵٶϢ
//	GRPointVList *mpRPointPVListSV;                 //ݼ첨㶨ݲٶϢ
//    PPORPSV mPSVFlag;    
	//־ǰǷת
	//////////////////////////
	float mSinA;                                  //
	float mCosA;                                   //
    int mShotIndexCount;                          //
	float mMaxReceiverLineZLong;                  //첨߳
	float mLineInterv;   
	int mMinRecordNum;
	int mMaxRecordNum;
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	int mbeginfoldi;int mendfoldi;//ǵķΧ
	int mbeginfoldj;int mendfoldj;//ǵķΧ
	GAttrAnalyzeParam *mpAttrAnalyzeParam;          //ڼ༰ַλԷ?
	//////////////////////////////////////////////////////////////////////////add by LiuWei
public:
	GProject();
	~GProject();
	float getR(){return mR;}
	float getH(){return mH;}
	//----̹졢١ʺ--------------------------------------------
    static	GProject * create();		//ʯʹѧӿڣ
	static void destroy();			//ʯʹѧӿڣ
	static GProject * getCurrentProject(){return mpCurrentPtr;}	
	/////////////////////
	static GProject * createSV();		//ʯʹѧӿڣ
	static void destroySV();			//ʯʹѧӿڣ
	static GProject * getCurrentProjectSV()	{ return mpCurrentPtrSV; }
	////////////////////////////////////////
//    void setPSVFlag(PPORPSV flag);
	void setFlag();//Ի
	/////////////////////////////////////
	//----Ŀݣʯʹѧӿڣ----
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
	// add by shanzhou in 2026/10/15
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
		return mpCMPLineSetNew;    //cmplineݼ½ģzhaohu                 
	}
	GRLineSet * getOriginalRLineSet() { return mpRLineSet; } // add by shanzhou in 2026/10/15
	GSLineSet * getOriginalSLineSet() { return mpSLineSet; }


	//--------------------------------------------------------------------------

	//----ļ--------------------------------------------------
		//----õ첨ļ(ԶУ)----
    QString	getCMPVFileName(){return mProjectDir+"/"+mProjectName+".v";}
	QString getRStackFileName()
	{ 
//		if(mPSVFlag==PPDATA)
			return mProjectDir+"/"+mProjectName+".rstack";
//		else
//			return mProjectDirSV+"/"+mProjectNameSV+".rstack";
	}
	//----õļ----
	QString getProjectFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		if(mProjectDir.isEmpty()) return NULL;
	    return mProjectDir+"/"+mProjectName+".ppr";
 //       else
//			return mProjectDirSV+"/"+mProjectNameSV+".ppr";
	}
	//----õڹϵļ----
	QString getXFileName()
	{
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".x"; 
//		else
 //           return mProjectDirSV+"/"+mProjectNameSV+".x"; 
	}

	//----õڵļ----
	QString getSFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".s";
//		else
 //         return mProjectDirSV+"/"+mProjectNameSV+".s";
	}
	//----õ첨ļ----
	QString getRFileName()
	{
//		if(mPSVFlag==PPDATA)
		  return mProjectDir+"/"+mProjectName+".r";
//		else
//	     return mProjectDirSV+"/"+mProjectNameSV+".r"; 
	}
	//----õļ----
	QString getFBTFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".fbt"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".fbt"; 
	}

	//----õתļ----
	QString getFBTFileNameSV()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".fbtSV"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".fbtSV"; 
	}
	//---õļ----
	QString getStackFileName()
	{
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+".segy";
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+".segy";

	}

	//----õSPSļ־ļ----
	QString getSPSLogFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		   return mProjectDir+"/"+mProjectName+"sps_log.txt"; 
//		else
//		   return mProjectDirSV+"/"+mProjectNameSV+"sps_log.txt"; 
	}
	//----õSEG-Yļ־ļ----
	QString getSGYLogFileName()
	{ 
//		if(mPSVFlag==PPDATA)
		    return mProjectDir+"/"+mProjectName+"sgy_log.txt"; 
//		else
//			return mProjectDirSV+"/"+mProjectNameSV+"sgy_log.txt"; 
	}
	//----õͼƬ·----
	QString getImagePath()
	{ 
//		if(mPSVFlag==PPDATA)
			return mProjectDir+"/image"; 
//		else
 //    		return mProjectDirSV+"/image";
	}
	//----õSEG-YļĿ----
	int getSEGYFileCount()
	{ 
//		if(mPSVFlag==PPDATA)
			return mSEGYFileList.count(); 
//		else
//			return mSEGYFileListSV.count();
	}
	//----ͨ±SEG-Yļ----
	QString getSEGYFileName(int index);
	//----׷SEG-Yļ----
	void appendSEGYFileName(QString fileName);
	//----SEG-Yļб----
	void clearSEGYFileList();
	//----Ŀ¼ʯʹѧӿڣ----
	bool createFolders(const QString &projectName, const QString &projectDir);
	bool  createFolders();
	//------------- -------------------------------------------------------------

	//----ݵ뺯--------------------------------------------------------
	//----SPSļϢ----
	bool appendSPSInfo(const QString &sFileName, const QString &rFileName, 
					   const QString &xFileName, QTextEdit *spsMemo = NULL,
					   QProgressBar *progressBar= NULL);
	//----SEG-Yļ----
	bool appendSEGYInfo(const QString &fileName, QProgressBar *progressBar = NULL, 
						QTextEdit *sgyMemo = NULL);
	//----ˢݽṹ򡢳ȡݽṹ
	void refresh();

	//----洢飨ļ+ݿ⣩-----------------------------------
	//----湤ļ----
	bool saveProjectFile();
	//----湤Ϣ----
	bool save();

	// SPSϢ
	bool saveSPS(const QString &spsFileName);//add by Wushanzhou



	//-------
	bool saveTime();
	//----汨Ϣݿ⣨ʯʹѧӿڣ----
	bool saveToDB(QProgressDialog *dlg);

	//----빤ļ----
	bool loadProjectFile(const QString &fileName);
	//----ļ----
	bool load(const QString &fileName);
	//--------
	bool reLoad();
	//----ݿжϢ,ݿݼļʯʹѧӿڣ----
	bool loadFromDB( const QString &lineBindNo );
	//--------------------------------------------------------------------------

	//----빤غʯʹѧӿڣ--------------------------------------
	//void setMainFrmProList( PWorkAreaList *list )	{ mainFrmProList = list; }
	//PWorkAreaList *getMainFrmProList()				{ return mainFrmProList; }
	//--------------------------------------------------------------------------
	
	//----ĬԺ--------------------------------------------------------
	//----<1>ͼĬ----
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
	void setFoldArea(int beginfoldi,int endfoldi,int beginfoldj,int endfoldj)//ǵķΧ
	{
		mbeginfoldi=beginfoldi;mendfoldi=endfoldi;mbeginfoldj=beginfoldj;mendfoldj=endfoldj;
	}
	void getFoldArea(int &beginfoldi,int &endfoldi,int &beginfoldj,int &endfoldj)//ǵķΧ
	{
		beginfoldi=mbeginfoldi;endfoldi=mendfoldi;beginfoldj=mbeginfoldj;endfoldj=mendfoldj;
	}

	void setAttrAnalyzeParam(GAttrAnalyzeParam *param) { mpAttrAnalyzeParam=param;}
	GAttrAnalyzeParam *getAttrAnalyzeParam() {return mpAttrAnalyzeParam;}
	static double getAngle(double sinAF, double cosAF);
	//////////////////////////////////////////////////////////////////////////add by LiuWei
};

#endif
