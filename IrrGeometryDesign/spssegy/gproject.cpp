#include "gproject.h"
#include "gspsscanner.h"
#include "gsegyfilescanner.h"
//#include "tchoicefbtform.h"
#include "math.h"
#include "gcellgrid.h"

#define MAX_FLOAT  3.2767e38f
#define MIN_FLOAT  -3.2768e38f

GProject * GProject::mpCurrentPtr = NULL;
GProject * GProject::mpCurrentPtrSV = NULL;

GProject::GProject()
{
	mProjectType = gProject2D;
	mAF = 0;
	mSinAF = 0;		mCosAF = 0;
	mMinSPSX = 0;	mMaxSPSX = 0;
	mMinSPSY = 0;	mMaxSPSY = 0;
	mMinX = 0;		mMaxX = 0;
	mMinY = 0;		mMaxY = 0;
	mProjectName = QString::null;
	mProjectDir = QString::null;

	mX0=mY0=mZ0=0;//add by zhaohu
	mLength = 8000;//add by zhaohu
	mWidth = 8000;	//add by zhaohu
	mHeight = 0;//add by zhaohu


	mIsBend = false;
//	mEvaluateGrade = gGrade2;
	//mEvaluateGrade = gGrade3;
	mIsTFix = true;
	mIsNoSEGYFile = false;

    mHCMPInv = 20;			mVCMPInv = 50;
	mTrcInv = 50;			mLineInv = 100;
	mAmpWidth = 3.0f;
	mTrcWidth = 5.0f;		mTrcHeight = 1.0f;
	mEquWin = 400; 
	mFixedGain = 0;
//	mGainMode = gAGC;
	mFromTimeIndex = 0;		mToTimeIndex = -1;		//-1ʾȫʱ
	mBase = 1000;			mVelocity = 2200;  

	mpShotSet = NULL;
	mpSLineSet = NULL;
	mpRLineSet = NULL;
	mpCMPLineSet = NULL;    //cmplineݼzhaohu
	mpSPSParam = NULL;// add by zhaohu
	mpCellGrid = NULL;// add by zhaohu
	mpShotSetNew = NULL;// add by zhaohu
	mpSLineSetNew = NULL;// add by zhaohu
	mpRLineSetNew = NULL;// add by zhaohu
	mpCMPLineSetNew = NULL;    //cmplineݼzhaohu
	mIsNewImfo=false;

//	mpRPointVList = NULL;
//	mpRPointPVList = NULL;
	////////////////////////////////////////////
	mpShotSetSV = NULL;
	mpSLineSetSV = NULL;
	mpRLineSetSV = NULL;
//	mpRPointVListSV = NULL;
//	mpRPointPVListSV = NULL;

	mR = 1.8;
	mH = 300;
//	mPSVFlag = PPDATA;

	setFlag();
	//////////////////////////////////////////////////////////////////////////add by LiuWei
	mbeginfoldi=0;mendfoldi=0;mbeginfoldj=0;mendfoldj=0;
	mpAttrAnalyzeParam=new GAttrAnalyzeParam();
}

GProject::~GProject()
{
//	if(mPSVFlag==PPDATA)
//	{
	if(mpSLineSet != NULL) delete mpSLineSet; 
	if(mpRLineSet != NULL) delete mpRLineSet;
	if(mpShotSet != NULL) delete mpShotSet;
	if(mpCMPLineSet != NULL)  delete mpCMPLineSet; //cmplineݼzhaohu
	if(mpSPSParam !=NULL) delete mpSPSParam;//add by zhaohu
	if(mpCellGrid !=NULL) delete mpCellGrid;//add by zhaohu
	if(mpSLineSetNew != NULL) delete mpSLineSetNew; 
	if(mpRLineSetNew != NULL) delete mpRLineSetNew;
	if(mpShotSetNew != NULL) delete mpShotSetNew;// add by zhaohu
	if(mpCMPLineSetNew != NULL)  delete mpCMPLineSetNew; //cmplineݼzhaohu

//	if(mpRPointVList !=NULL) delete mpRPointVList;
//	if(mpRPointPVList!=NULL) delete mpRPointPVList;
//	}
	/////////////////////////////////////////////////
//	else
//	{
	if(mpSLineSetSV != NULL) delete mpSLineSetSV; 
	if(mpRLineSetSV != NULL) delete mpRLineSetSV;
	if(mpShotSetSV != NULL) delete mpShotSetSV;
//	if(mpRPointVListSV !=NULL) delete mpRPointVListSV;
//	if(mpRPointPVListSV!=NULL) delete mpRPointPVListSV;
//	}
}  

void GProject::setFlag()//Ի
{
	mpShotSet = new GShotSet();
	mpShotSet->setProject(mpCurrentPtr);
	mpSLineSet = new GSLineSet();
	mpRLineSet = new GRLineSet();
	mpCMPLineSet = new GCMPLineSet(); //cmplineݼzhaohu
	mpSPSParam = new GSPSParam(); // add by zhaohu
	mpCellGrid = new GCellGrid(); // add by zhaohu
	mpShotSetNew = new GShotSet();
	mpShotSetNew->setProject(mpCurrentPtr);
	mpSLineSetNew = new GSLineSet();
	mpRLineSetNew = new GRLineSet(); // add by zhaohu
	mpCMPLineSetNew = new GCMPLineSet(); //cmplineݼzhaohu
}

//void GProject::setPSVFlag(PPORPSV flag)
//{
//	mPSVFlag  = flag;
//	if(mPSVFlag==PPDATA)
//	{
//	mpShotSet = new GShotSet();
//	mpShotSet->setProject(mpCurrentPtr);
//	mpSLineSet = new GSLineSet();
//	mpRLineSet = new GRLineSet();
//	mpRPointVList = new	GRPointVList();
//	mpRPointPVList = new GRPointVList();
//	}
	////////////////////////////////////////////
//	else
//	{
//	mpShotSetSV = new GShotSet();
//	mpShotSetSV->setProject(mpCurrentPtrSV);
//	mpSLineSetSV = new GSLineSet();
//	mpRLineSetSV = new GRLineSet();
//	mpRPointVListSV = new	GRPointVList();
//	mpRPointPVListSV = new GRPointVList();
//	}
//}
GProject * GProject::create()
{
     GProject *pProject = new GProject;
	 mpCurrentPtr = pProject;	
     return pProject;	

}

void GProject::destroy()
{
	if(mpCurrentPtr != NULL)
	{
		delete mpCurrentPtr;
		mpCurrentPtr = NULL;
	}
}

GProject * GProject::createSV()
{
     GProject *pProject = new GProject;
	 mpCurrentPtrSV = pProject;	
     return pProject;	

}

void GProject::destroySV()
{
	if(mpCurrentPtrSV != NULL)
	{
		delete mpCurrentPtrSV;
		mpCurrentPtrSV = NULL;
	}
}
void GProject::clear()
{
//	if(mPSVFlag==PPDATA)
//	{
	if(mpSLineSet!=NULL) mpSLineSet->clear();
	if(mpRLineSet!=NULL) mpRLineSet->clear();
	if(mpShotSet!=NULL ) mpShotSet->clear();

	if(mpCMPLineSet != NULL)    mpCMPLineSet->clear(); //cmplineݼzhaohu
	if(mpSPSParam !=NULL) mpSPSParam->reset();//add by zhaohu
	if(mpCellGrid !=NULL) mpCellGrid->clear();//add by zhaohu
	if(mpSLineSetNew!=NULL) mpSLineSetNew->clear();
	if(mpRLineSetNew!=NULL) mpRLineSetNew->clear();
	if(mpShotSetNew!=NULL ) mpShotSetNew->clear();
	if(mpCMPLineSetNew != NULL)    mpCMPLineSetNew->clear(); //cmplineݼzhaohu

 //   }
//	else
//	{
//	mpSLineSetSV->clear();
//	mpRLineSetSV->clear();
//	mpShotSetSV->clear();
	mProjectName = QString::null;
	mProjectDir = QString::null;
//	}
	clearSEGYFileList();
	mWorkAreaNo = QString::null;
	//mLineBindNo = QString::null;
	mLineBindNo = QString("LineBind_1");	//for debug

	mHCMPInv = 20;	mVCMPInv = 50;
	mTrcInv = 50;	mLineInv = 100;
	mIsBend = false;
	mIsNoSEGYFile = false;

	mMinSPSX = MAX_FLOAT;
	mMinSPSY = MAX_FLOAT;
	mMaxSPSX = MIN_FLOAT;
	mMaxSPSY = MIN_FLOAT;

	mAmpWidth = 1.0f;
	mTrcWidth = 5.0f;
	mTrcHeight = 1.0f;
	mEquWin = 400; 
	mFixedGain = -50;
//	mGainMode = gAGC;
	mFromTimeIndex = -1;
	mToTimeIndex = -1;		//-1ʾȫʱ

	mPrjName = QString::null;//add by zhaohu
	mPrjFileName = QString::null;//add by zhaohu
	mX0 = mY0 = mZ0 = 0;//add by zhaohu
	mLength = 0;//add by zhaohu
	mWidth = 0;	//add by zhaohu
	mHeight = 0;//add by zhaohu
}

GProjectType GProject::getProjectType()
{
//	if(mPSVFlag==PPDATA)
//	{
	if(mpRLineSet->count() > 1) return gProject3D;
	else return gProject2D;
//	}
//	else
//	{
//    if(mpRLineSetSV->count() > 1) return gProject3D;
//	else return gProject2D;
//	}
}
/**/

void GProject::clearSEGYFileList()
{
//	if(mPSVFlag==PPDATA)
	mSEGYFileList.clear();
//	else
//	mSEGYFileListSV.clear();
}

QString GProject::getSEGYFileName(int index)
{
	
//	if(mPSVFlag==PPDATA)
//	{
		if(0 <= index && index < (int)mSEGYFileList.count())
			return mSEGYFileList[index];
		else return QString::null;
//	}
//	else
//	{
//	  if(0 <= index && index < (int)mSEGYFileListSV.count())
//			return mSEGYFileListSV[index];
//		else return QString::null;
//	}
}

void GProject::appendSEGYFileName(QString fileName)
{
//	if(mPSVFlag==PPDATA)
	mSEGYFileList.append(fileName);
//	else
//	mSEGYFileListSV.append(fileName);
}

bool GProject::appendSPSInfo(const QString &sFileName, const QString &rFileName, 
							 const QString &xFileName, QTextEdit *spsMemo,QProgressBar *progressBar )
{

	GXFileScanner xScanner;
	xScanner.setProject(this);
	if(!xScanner.scan(xFileName, mpShotSet, spsMemo))
	{
		if(mpShotSet) mpShotSet->clear();
		return false;
	}

	GSFileScanner sScanner;
	if(!sScanner.scan(sFileName, mpSLineSet, mpShotSet, spsMemo))
	{
		if(mpShotSet) mpShotSet->clear();
		if(mpSLineSet) mpSLineSet->clear();
		return false;
	}

	GRFileScanner rScanner;
	if(!rScanner.scan(rFileName, mpRLineSet, spsMemo, progressBar))
	{
		if(mpShotSet) mpShotSet->clear();
		if(mpSLineSet) mpSLineSet->clear();
		if(mpRLineSet) mpRLineSet->clear();
		return false;
	}

    for(int ee  = 0; ee<mpRLineSet->count(); ee++)
	{
		mpRLineSet->get(ee)->sortByPointNum();//첨㰴׮
	}
	mMinSPSX = MAX_FLOAT;	mMinSPSY = MAX_FLOAT;
	mMaxSPSX = MIN_FLOAT;	mMaxSPSY = MIN_FLOAT;
	
	mpRLineSet->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);
	mpSLineSet->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);

	mpRLineSet->getAngleParam(mSinAF, mCosAF);

	// mAF = gGetAngle(mSinAF, mCosAF);
	mTransX0 = MAX_FLOAT;	mTransY0 = MAX_FLOAT;
	mpRLineSet->rotate(mSinAF, mCosAF, mTransX0, mTransY0);
	mpSLineSet->rotate(mSinAF, mCosAF, mTransX0, mTransY0);

	mpRLineSet->translate(-mTransX0, -mTransY0);
	mpSLineSet->translate(-mTransX0, -mTransY0);

	mMinX = MAX_FLOAT;	mMinY = MAX_FLOAT;
	mMaxX = MIN_FLOAT;	mMaxY = MIN_FLOAT;
	mpRLineSet->getRange(mMinX, mMinY, mMaxX, mMaxY);
	mpSLineSet->getRange(mMinX, mMinY, mMaxX, mMaxY);

//modify by psl 070619

	mpShotSet->checkSInfo(spsMemo);
	mpShotSet->checkRInfo(spsMemo,progressBar);

	//mpSLineSet->clearVainShots();

#pragma region "add by liuwei"
	float minx, miny, maxx, maxy;
	minx = MAX_FLOAT;	miny = MAX_FLOAT;
	maxx = MIN_FLOAT;	maxy = MIN_FLOAT;
	mpRLineSet->getRange(minx,miny,maxx,maxy);
	mpSPSParam->mRX0=minx;// źͼ첨еʼ굽 GSPSParam  [Liu Wei 5/31/2023]
	mpSPSParam->mRY0=miny;// źͼ첨еʼ굽 GSPSParam  [Liu Wei 5/31/2023]
	minx = MAX_FLOAT;	miny = MAX_FLOAT;
	maxx = MIN_FLOAT;	maxy = MIN_FLOAT;
	mpSLineSet->getRange(minx,miny,maxx,maxy);
	mpSPSParam->mSX0=minx;// źͼ첨еʼ굽 GSPSParam  [Liu Wei 5/31/2023]
	mpSPSParam->mSY0=miny;// źͼ첨еʼ굽 GSPSParam  [Liu Wei 5/31/2023]
#pragma endregion "add by liuwei"


	return true;
}

bool GProject::createFolders()
{
//	if(mPSVFlag==PPDATA)
//	{
	QDir dir;
	//if(!dir.mkdir(mProjectDir)) return false;
	if(!dir.mkdir(mProjectDir+"\\data")) return false;
	if(!dir.mkdir(mProjectDir+"\\image")) return false;
	if(!dir.mkdir(mProjectDir+"\\table")) return false;
/*	}
	else
	{
     QDir dir;
	//if(!dir.mkdir(mProjectDir)) return false;
	if(!dir.mkdir(mProjectDirSV+"\\data")) return false;
	if(!dir.mkdir(mProjectDirSV+"\\image")) return false;
	if(!dir.mkdir(mProjectDirSV+"\\table")) return false;
	}
	*/
	return true;
}

bool GProject::createFolders(const QString &projectName, const QString &projectDir)
{
	mProjectName = projectName;
	mProjectDir = projectDir;

	QDir dir;
	if(!dir.mkdir(mProjectDir)) return false;
	return true;
}

bool GProject::scanForShotRecorderRange(const QString &fileName, 
												QProgressBar *progressBar, 
			                           QTextEdit *sgyMemo )
{
    GSEGYFileScanner sgyScanner;
	sgyScanner.setProject(mpCurrentPtr);
	return sgyScanner.scanForShotRecorderRange(fileName,progressBar,sgyMemo);
}

bool GProject::appendSEGYInfo(const QString &fileName, QProgressBar *progressBar, 
							  QTextEdit *sgyMemo)
{
	GSEGYFileScanner sgyScanner;
//	if(mPSVFlag==PPDATA)
		sgyScanner.setProject(mpCurrentPtr);
//	else
//		sgyScanner.setProject(mpCurrentPtrSV);
	if(!sgyScanner.scan(fileName, progressBar, sgyMemo)) return false;

//	if(mPSVFlag==PPDATA)
	mpShotSet->checkSEGYInfo(sgyMemo);
//	else
//	mpShotSetSV->checkSEGYInfo(sgyMemo);
	appendSEGYFileName(fileName);
	return true;
}

void GProject::refresh()
{
//	if(mPSVFlag==PPDATA)
//	{
	mpSLineSet->clearVainShots();			//޹ϵڵ
	mpSLineSet->sort();						//ߺŵݼ
	mpShotSet->sort();						//ߺڹϵݼ
	mpShotSet->extractCSP(mpSLineSet);		//ȡڵ
	mpShotSet->extractCRP(mpRLineSet);		//ȡ첨

	mpSLineSetNew->clearVainShots();			//޹ϵڵ
	mpSLineSetNew->sort();						//ߺŵݼ
	mpShotSetNew->sort();						//ߺڹϵݼ
	mpShotSetNew->extractCSP(mpSLineSetNew);		//ȡڵ
	mpShotSetNew->extractCRP(mpRLineSetNew);		//ȡ첨
/*	}
	else
	{
	mpSLineSetSV->clearVainShots();			//޹ϵڵ
	mpSLineSetSV->sort();						//ߺŵݼ
	mpShotSetSV->sort();						//ߺڹϵݼ
	mpShotSetSV->extractCSP(mpSLineSet);		//ȡڵ
	mpShotSetSV->extractCRP(mpRLineSet);		//ȡ첨
	}
	*/
}

bool GProject::saveTime()
{
//    if(mPSVFlag==PPDATA)
//	{
	QString destFileName = mProjectDir+"/"+mProjectName;
	return !mpShotSet->savePriTime( destFileName+".pik" );
/*	}
	else
	{
	QString destFileName = mProjectDirSV+"/"+mProjectNameSV;
	return !mpShotSetSV->savePriTime( destFileName+".pik" );
	}
	*/
}

bool GProject::saveProjectFile()
{
	Q_INT32 i, data;
	QFile file(getProjectFileName());			//ļͳһʹQFileд
	if(!file.open(IO_WriteOnly)) return false;

	QDataStream stream( &file );

	//<1>ļ־Ͱ汾
	stream << PROJECT_TAG;
	stream << PROJECT_VERSION;			
	//<2>
//	if(mPSVFlag)
//	{
	stream << mProjectName;
	stream << mProjectDir;
/*	}
	else
	{
	stream << mProjectName;
	stream << mProjectDir;
	}
	*/
	stream << mMinSPSX << mMinSPSY << mMaxSPSX << mMaxSPSY;
	stream << mAF << mSinAF << mCosAF;				//mAFΪĿ
	stream << mTransX0 << mTransY0;
	stream << mMinX << mMinY << mMaxX << mMaxY;
	//<3>Ĭ
//	stream << mAmpWidth << mTrcWidth << mTrcHeight <<mTrcInv;
	stream << mAmpWidth << mTrcWidth << mTrcHeight;
//	if(mGainMode == gAGC) data = 1;
//	else data = 0;
	stream << data;
	stream << mEquWin << mFixedGain;
	stream << mFromTimeIndex << mToTimeIndex;		//Ŀ
	//<4>SEG-Yļб
	stream << (Q_INT32)mSEGYFileList.count();
	for(i=0; i<getSEGYFileCount(); i++)
	{
		stream << getSEGYFileName(i);
	}

	file.close();
	return true;
}


/*
bool GProject::saveSPS(const QString &spsFileName) // add by shanzhou in 2026/10/15
{
    GShotSet *pShotSet = getShotSet();
    GSLineSet *pSLineSet = getSLineSet();
    GRLineSet *pRLineSet = getRLineSet();

    if ((pSLineSet == NULL || pSLineSet->count() == 0) &&
        (pRLineSet == NULL || pRLineSet->count() == 0))
    {
        return false;
    }

    QFileInfo fi(spsFileName);
    QString dir  = fi.absolutePath();
    QString base = fi.completeBaseName();
    if (base.isEmpty()) base = fi.fileName();
    QDir qdir(dir);
    QString prefix = qdir.filePath(base);

    QString sFileName = prefix + ".s";
    QString rFileName = prefix + ".r";
    QString xFileName = prefix + ".x";

    bool sOk = true;
    if (pSLineSet && pSLineSet->count() > 0)
    {
        sOk = pSLineSet->saveSFile2(sFileName);
    }

    bool rOk = true;
    if (pRLineSet && pRLineSet->count() > 0)
    {
        rOk = pRLineSet->saveRFile2(rFileName);
    }

    if (pShotSet && pShotSet->count() > 0 && pSLineSet && pRLineSet)
    {
        pShotSet->saveXFile2(xFileName, pSLineSet, pRLineSet);
    }

    return sOk && rOk;
}
*/



//2026.3.24 ùļsaveȲñ׼spsļ by Wushanzhou

bool GProject::saveSPS(const QString &spsFileName) // add by shanzhou in 2026/10/15
{
    GShotSet *pShotSet = getShotSet();
    GSLineSet *pSLineSet = getSLineSet();
    GRLineSet *pRLineSet = getRLineSet();

    if (pShotSet == NULL || pSLineSet == NULL || pRLineSet == NULL)
        return false;

    QFileInfo fi(spsFileName);
    QString dir  = fi.absolutePath();
    QString base = fi.completeBaseName();
    QDir qdir(dir);
    QString prefix = qdir.filePath(base);

    QString sFileName = prefix + ".s";
    QString rFileName = prefix + ".r";
    QString xFileName = prefix + ".x";

    // Ϊ SPS2.1 ıӿ
    if (!pSLineSet->saveSFile2(sFileName))
        return false;

    if (!pRLineSet->saveRFile2(rFileName))
        return false;

    if (!pShotSet->saveXFile2(xFileName, pSLineSet, pRLineSet))
        return false;

    return true;
}

bool GProject::save()
{
	if(mpShotSet==NULL)
		return false;
	if(!saveProjectFile()) return false;

//	if(mPSVFlag==PPDATA)
//	{
	if(!mpShotSet->save(getXFileName(), getFBTFileName(),
						getFBTFileNameSV())) return false;
	if(!mpSLineSet->save(getSFileName())) return false;
	if(!mpRLineSet->save(getRFileName())) return false;
 /*   }
	else
	{
	if(!mpShotSetSV->save(getXFileName(), getFBTFileName(),
						getFBTFileNameSV())) return false;
	if(!mpSLineSetSV->save(getSFileName())) return false;
	if(!mpRLineSetSV->save(getRFileName())) return false;
    
	}
	*/
	return true;
}

bool GProject::saveToDB(QProgressDialog *dlg)
{
/*	dlg->setLabelText(tr("Save Receive Info to DataBase"));
	if(!mpRLineSet->saveToDB(mWorkAreaNo, mLineBindNo, dlg)) return false;
	dlg->setLabelText(tr("Save Shot Info to DataBase"));
	if(!mpShotSet->saveToDB(mWorkAreaNo, mLineBindNo, dlg)) return false;
	*/
	return true;
}

bool GProject::loadProjectFile( const QString &fileName )
{
	QString prjTag, prjVersion, str;

	Q_INT32 i, count, data;
	QFile file( fileName );			//ļͳһʹQFileд
	if(!file.open(IO_ReadOnly)) return false;
	
	QDataStream stream(&file);

	//<1>ȡļ־Ͱ汾ŲȽ
	stream >> prjTag;
	if(prjTag != PROJECT_TAG) return false;
	stream >> prjVersion;
	if(prjVersion != PROJECT_VERSION) return false;

	//clear();			//ԭclear()ʯʹѧ
 QString projectName,projectDir,projectNameSV,projectDirSV;
	//<2>ȡ
//	if(mPSVFlag==PPDATA)
//	{
	stream >> projectName;
	stream >> projectDir;
	mProjectName = projectName;
	mProjectDir  = projectDir;
//	}
//	else
//	{
//	stream >> projectNameSV;
//	stream >> projectDirSV;
//	}
	//mProjectDir = waInfo.workareaDir + "/" + mProjectDir;	//Ϊʯʹѧӿ
	stream >> mMinSPSX >> mMinSPSY >> mMaxSPSX >> mMaxSPSY;
	stream >> mAF >> mSinAF >> mCosAF;				//mAFΪĿ
	stream >> mTransX0 >> mTransY0;
	stream >> mMinX >> mMinY >> mMaxX >> mMaxY;
	//<3>ȡĬ
	//stream >> mAmpWidth >> mTrcWidth >> mTrcHeight >> mTrcInv;
	stream >> mAmpWidth >> mTrcWidth >> mTrcHeight ;
	stream >> data;
//	if(data == 1) mGainMode = gAGC;
//	else mGainMode = gFixed;
	stream >> mEquWin >> mFixedGain;
	stream >> mFromTimeIndex >> mToTimeIndex;		//Ŀ
	//<4>ȡSEG-Yļб
	stream >> count;
	mIsNoSEGYFile = false;
	bool isModify = false;
	for(i=0; i<count; i++)
	{
		stream >> str;
		QFileInfo finfo(str);
		if(!finfo.exists())
		{
			if(QMessageBox::warning(NULL, tr("Warning"), "File \""+str+"\" Doesn't Exists! \nPlease Assign again!",
									QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
			{
				str = QFileDialog::getOpenFileName(QString::null, 
											"SEG-Y Files (*.sgy);;All files (*.*)",
											NULL, "name", tr("Open SEG-Y File") );// Qt3 [Liu Wei 4/13/2023]
				if(str != QString::null) 
				{
					appendSEGYFileName(str);
					isModify = true;
				}
				else mIsNoSEGYFile = true;
			}
			else mIsNoSEGYFile = true;

		}
		else appendSEGYFileName(str);
	}

	file.close();

	if(isModify) saveProjectFile();
	return true;
}
	
bool GProject::load(const QString &fileName)
{
	if(!loadProjectFile(fileName)) return false;

//	if(mPSVFlag==PPDATA)
//	{
		if(!mpSLineSet->load(getSFileName())) return false;
		if(!mpRLineSet->load(getRFileName())) return false;
		if(!mpShotSet->load(getXFileName(), getFBTFileName(), 
							getFBTFileNameSV())) return false;
/*	}
   else
	{
		if(!mpSLineSetSV->load(getSFileName())) return false;
		if(!mpRLineSetSV->load(getRFileName())) return false;
		if(!mpShotSetSV->load(getXFileName(), getFBTFileName(), 
							getFBTFileNameSV())) return false;
	}
	*/
	return true;
}

bool GProject::loadFromDB( const QString &lineBindNo )
{
  return true;
}

bool GProject::reLoad()
{
	return load(getProjectFileName());
}

bool GProject::saveFBT()
{
	float type = 0;
	
	QString s1 = QString::fromLocal8Bit("");
	QString s2 = QString::fromLocal8Bit("ѡ񱣴ͣɫΪһɫΪ");
	QString s3 = QString::fromLocal8Bit("һ");
	QString s4 = QString::fromLocal8Bit("һͶ");
	switch( QMessageBox::information( 0,s1 ,s2,s3,s4, "Cancel",
		0,      // Enter == button 0
        2 ) ) { // Escape == button 2
    case 0: type = 0.6;
        break;
    case 1:type = 0.1;
        break;
    case 2: type = -1;
        break;
    }
    if(type<0)
		return false;
	////////////////////////////////////
	QString s = QFileDialog::getSaveFileName(
		QString::null,
		"txt (*.*)",
		0,
		"open file dialog"
		"Choose a file name" );
	QFile file(s);
	file.open(IO_WriteOnly);
	QTextStream stream(&file);
	int si = getShotSet()->get(0)->getCPInv()/1000;
    for(int i=0; i<getShotSet()->count(); i++)
	{
		int trcCount = getShotSet()->get(i)->getTrcCount();
		int sFileRecord = getShotSet()->get(i)->getRecNum();
		QString pointNum = getShotSet()->get(i)->getPointNum();
		for(int j=0;j<trcCount;j++)
		{
		
            int *fbt = getShotSet()->get(i)->getFBTByIndex(j);
			//if(*fbt<2)
			int tempFBT = *fbt;
			if(tempFBT<1)
				//	   continue;
				tempFBT = 1;
			float fbtqc = getShotSet()->get(i)->getQCFBTByIndex(j);
			// 		   if(fbtqc<type)
			// 			   tempFBT = -1;
			QString temp =QString::number(sFileRecord).leftJustify(15,' ')+"       "+QString::number(j).leftJustify(15,' ')
				+"     "+QString::number(tempFBT*si).leftJustify(15,' ');
		
			stream<<temp<<"\r\n";
		}
	}
	
	file.close();
	return true;

// 	float type = 0;
//    
// 	QString s1 = QString::fromLocal8Bit("");
// 	QString s2 = QString::fromLocal8Bit("ѡ񱣴ͣɫΪһɫΪ");
// 	QString s3 = QString::fromLocal8Bit("һ");
// 	QString s4 = QString::fromLocal8Bit("һͶ");
// 	switch( QMessageBox::information( 0,s1 ,s2,s3,s4, "Cancel",
// 			0,      // Enter == button 0
//         2 ) ) { // Escape == button 2
//     case 0: type = 0.6;
//         break;
//     case 1:type = 0.1;
//         break;
//     case 2: type = -1;
//         break;
//     }
//     if(type<0)
// 		return false;
// 	////////////////////////////////////
//    QString s = QFileDialog::getSaveFileName(
//                     QString::null,
//                     "txt (*.*)",
//                     0,
//                     "open file dialog"
//                     "Choose a file name" );
// 	QFile file(s);
// 	file.open(IO_WriteOnly);
// 	QTextStream stream(&file);
// 	int si = getShotSet()->get(0)->getCPInv()/1000;
//     for(int i=0; i<getShotSet()->count(); i++)
// 	{
// 		int trcCount = getShotSet()->get(i)->getTrcCount();
// 		int sFileRecord = getShotSet()->get(i)->getRecNum();
// 		QString pointNum = getShotSet()->get(i)->getPointNum();
// 		for(int j=0;j<trcCount;j++)
// 		{
//            int trcNum;
// 		   QString trcPointNum = getShotSet()->get(i)->getRPointByTrcIndex(j)->getPointNum();
// 		   if(!(getShotSet()->get(i)->getTrcByIndex(j,trcNum)))
// 		   continue;
//             int *fbt = getShotSet()->get(i)->getFBTByIndex(j);
// 		   //if(*fbt<2)
// 		   int tempFBT = *fbt;
// 		   if(tempFBT<1)
// 		//	   continue;
// 		  tempFBT = 1;
// 		   float fbtqc = getShotSet()->get(i)->getQCFBTByIndex(j);
// // 		   if(fbtqc<type)
// // 			   tempFBT = -1;
// 		   QString temp =QString::number(sFileRecord).leftJustify(15,' ')+"       "+QString::number(trcNum).leftJustify(15,' ')
//  			   +"     "+QString::number(tempFBT*si).leftJustify(15,' ');
// 		//   QString temp ="      "+QString::number(sFileRecord).leftJustify(10,' ')+"       "+pointNum
// 		//	   +"       "+trcPointNum
// 		//	   +"     "+QString::number(tempFBT*si).leftJustify(10,' ');
// 		   stream<<temp<<"\r\n";
// 		}
// 	}
// 
//    file.close();
//   return true;

// 	float type = 0;
// 	
// 	QString s1 = QString::fromLocal8Bit("");
// 	QString s2 = QString::fromLocal8Bit("ѡ񱣴ͣɫΪһɫΪ");
// 	QString s3 = QString::fromLocal8Bit("һ");
// 	QString s4 = QString::fromLocal8Bit("һͶ");
// 	switch( QMessageBox::information( 0,s1 ,s2,s3,s4, "Cancel",
// 		0,      // Enter == button 0
//         2 ) ) { // Escape == button 2
//     case 0: type = 0.6;
//         break;
//     case 1:type = 0.1;
//         break;
//     case 2: type = -1;
//         break;
//     }
//     if(type<0)
// 		return false;
// 	////////////////////////////////////
// 	QString s = QFileDialog::getSaveFileName(
// 		QString::null,
// 		"txt (*.*)",
// 		0,
// 		"open file dialog"
// 		"Choose a file name" );
// 	QFile file(s);
// 	file.open(IO_WriteOnly);
// 	QTextStream stream(&file);
// 	int si = getShotSet()->get(0)->getCPInv()/1000;
//     for(int i=0; i<getShotSet()->count(); i++)
// 	{
// 		int trcCount = getShotSet()->get(i)->getTrcCount();
// 		int sFileRecord = getShotSet()->get(i)->getRecNum();
// 		QString sNum = getShotSet()->get(i)->getPointNum();
// 		float stv = getShotSet()->get(i)->getStatic();
// 		float toalStv=0;
// 		for(int j=0;j<trcCount;j++)
// 		{
// 			float rtv = getShotSet()->get(i)->getRPointByTrcIndex(j)->getStatic();
// 			QString rNum = getShotSet()->get(i)->getRPointByTrcIndex(j)->getPointNum();
// 			int trcNum;
// 			if(!(getShotSet()->get(i)->getTrcByIndex(j,trcNum)))
// 				continue;
//             int *fbt = getShotSet()->get(i)->getFBTByIndex(j);
// 			//if(*fbt<2)
// 			int tempFBT = *fbt;
// 			float fbtqc = getShotSet()->get(i)->getQCFBTByIndex(j);
// 			toalStv = stv+rtv;
// 			if(fbtqc<type)
// 			{
// 				tempFBT = -1;
// 				toalStv = 0;
// 			}
//             if(tempFBT<4)
// 				toalStv = 0;
// 			QString temp =QString::number(sFileRecord).leftJustify(15,' ')+"       "+sNum.leftJustify(15,' ')
// 				+"     "+rNum.leftJustify(15,' ')+"    "+QString::number(tempFBT*si).leftJustify(15,' ');//QString::number(tempFBT*si-toalStv).leftJustify(15,' ');
// 			stream<<temp<<"\r\n";
// 		}
// 	}
// 	
// 	file.close();
//   return true;

}
/*
bool GProject::saveFBT2()
{
	float type = 0;
 //   switch( QMessageBox::information( 0, "FBT Save Type",
 //       "Please Choice Type to Save FBT,Red FBT is First Class,Blue FBT is Second Class",
 //       "&Save First Class FBT (Red)", "&Save First And Second Class FBT(Red And Blue)", "Cancel",
 //       0,      // Enter == button 0
  //      2 ) ) { // Escape == button 2
		
	QString s1 = QString::fromLocal8Bit("");
	QString s2 = QString::fromLocal8Bit("ѡ񱣴ͣɫΪһɫΪ");
	QString s3 = QString::fromLocal8Bit("һ");
	QString s4 = QString::fromLocal8Bit("һͶ");
	switch( QMessageBox::information( 0,s1 ,s2,s3,s4, "Cancel",
			0,      // Enter == button 0
        2 ) ) { // Escape == button 2
    case 0: type = 0.6;
        break;
    case 1:type = 0.1;
        break;
    case 2: type = -1;
        break;
    }
    if(type<0)
		return false;
	////////////////////////////////////
   QString s = QFileDialog::getSaveFileName(
                    QString::null,
                    "txt (*.*)",
                    0,
                    "open file dialog"
                    "Choose a file name" );
	QFile file(s);
	file.open(IO_WriteOnly);
	QTextStream stream(&file);
	int si = getShotSet()->get(0)->getCPInv()/1000;
    for(int i=0; i<getShotSet()->count(); i++)
	{
		int trcCount = getShotSet()->get(i)->getTrcCount();
		int sFileRecord = getShotSet()->get(i)->getRecNum();
		float sx = getShotSet()->get(i)->getSPoint()->getSPSX();
		float sy = getShotSet()->get(i)->getSPoint()->getSPSY();
		float sz =-1*getShotSet()->get(i)->getSPoint()->getSPSZ();
		float wellDepth = getShotSet()->get(i)->getSPoint()->getDepth();
		QString sString = QString::number(sFileRecord).leftJustify(10,' ')+QString::number(trcCount).leftJustify(10,' ')
			+QString::number(sx,'f',1).leftJustify(10,' ')+QString::number(sy,'f',1).leftJustify(10,' ')+
			QString::number(sz,'f',1).leftJustify(10,' ')+QString::number(wellDepth,'f',1).leftJustify(10,' ');
      stream<<sString<<"\r\n";
		for(int j=0;j<trcCount;j++)
		{
          GRPoint *rPoint =  getShotSet()->get(i)->getRPointByTrcIndex(j);
		  int rStak = rPoint->getLineName().toInt()*1000+rPoint->getPointNum().toInt();
		  float rx = rPoint->getSPSX();
		  float ry = rPoint->getSPSY();
		  float rz = -1*rPoint->getSPSZ();
		  QString rString = QString::number(rStak).leftJustify(10,' ')+QString::number(rx,'f',1).leftJustify(10,' ')
			  +QString::number(ry,'f',1).leftJustify(10,' ')+QString::number(rz,'f',1).leftJustify(10,' ');
		   int *fbt = getShotSet()->get(i)->getFBTByIndex(j);
		   int tempFBT = *fbt;
		   float fbtqc = getShotSet()->get(i)->getQCFBTByIndex(j);
		   if(fbtqc<type)
			   tempFBT = -1;
		   QString temp = QString::number(tempFBT*si/1000.0,'f',5).leftJustify(10,' ');
		   stream<<rString<<temp<<"\r\n";
		}
	}
// 	for(int i=0; i<getShotSet()->count(); i++)
// 	{
// 		int trcCount = getShotSet()->get(i)->getTrcCount();
// 		int sFileRecord = getShotSet()->get(i)->getRecNum();
// 		float sx = getShotSet()->get(i)->getSPoint()->getSPSX();
// 		float sy = getShotSet()->get(i)->getSPoint()->getSPSY();
// 		float sz = getShotSet()->get(i)->getSPoint()->getSPSZ();
// 		float wellDepth = 1*getShotSet()->get(i)->getSPoint()->getDepth();
// 		QString sString = QString::number(sFileRecord).leftJustify(10,' ')
// 			+QString::number(sx,'f',1).leftJustify(10,' ')+QString::number(sy,'f',1).leftJustify(10,' ')+
// 			QString::number(sz,'f',1).leftJustify(10,' ')+QString::number(wellDepth,'f',1).leftJustify(10,' ');
// 	
// 		for(int j=0;j<trcCount;j++)
// 		{	
// 			stream<<sString;
// 			GRPoint *rPoint =  getShotSet()->get(i)->getRPointByTrcIndex(j);
// 			int rStak = rPoint->getLineName().toInt()*10000+rPoint->getPointNum().toInt();
// 			float rx = rPoint->getSPSX();
// 			float ry = rPoint->getSPSY();
// 			float rz =  rPoint->getSPSZ();
// 			QString rString = QString::number(rStak).leftJustify(10,' ')+QString::number(rx,'f',1).leftJustify(10,' ')
// 				+QString::number(ry,'f',1).leftJustify(10,' ')+QString::number(rz,'f',1).leftJustify(10,' ');
// 			int *fbt = getShotSet()->get(i)->getFBTByIndex(j);
// 			int tempFBT = *fbt;
// 			float fbtqc = getShotSet()->get(i)->getQCFBTByIndex(j);
// 			if(fbtqc<type)
// 				tempFBT = -1;
// 			QString temp = QString::number(tempFBT*si,'f',5).leftJustify(10,' ');
// 			stream<<rString<<temp<<"\r\n";
// 		}
// 	}
   file.close();
  return true;

}
*/

bool GProject::loadFBT()
{
// 		int si = getShotSet()->get(0)->getCPInv()/1000;
// ///////////////////////////////////////////////////////
//   QString s = QFileDialog::getOpenFileName(
//                     QString::null,
//                     "txt (*.*)",
//                     0,
//                     "open file dialog"
//                     "Choose a file name" );
//    if(s=="")
// 	   return false;
// 	QFile file(s);
// 	if(!file.open(IO_ReadOnly))
// 		return false;
// 	QProgressDialog progress( "loading....", "Abort Load", file.size(),
//                           0, "progress", TRUE );
// 	//////////
// 	QTextStream stream(&file);
// 	int oldFFid,newFFid;
// 	oldFFid = newFFid = -1;
// 	int tempTrcCount=0;
// 	while(!file.atEnd())
// 	{
// 	 QString line = stream.readLine();
// 	 progress.setProgress(file.at());
// 	 if(progress.wasCancelled())
// 		 break;
// 	  int data1,data2,data3;
// 	  int dataCount = sscanf(line,"%d %d %d",&data1,&data2,&data3);
// 	  if(dataCount<3)
// 		   continue;
// 	  newFFid = data1;
//         GShot *pShot = getShotSet()->getByRecNum(data1);
// 		if(pShot==NULL)
// 			continue;
// 	    if(newFFid!=oldFFid)
//               tempTrcCount = 0;
// 		else
// 		{	
// 			tempTrcCount++;
// 			for(int i=tempTrcCount; i<pShot->getTrcCount(); i++)
// 			{
// 				int trcNum;
// 				if(pShot->getTrcByIndex(i,trcNum))
//                 if(data2==trcNum)
// 				{
// 				 int *fbt = pShot->getFBTByIndex(i);
// 				 fbt[0] = data3*1.0/si;
// 				 pShot->setQCFBTByIndex(i,1);
// 				 break; 
// 				}
// 			}	
// 		
// 		}	
// 		oldFFid = newFFid;
// 	}
//     file.close();
// 	save();
//    return true;
	int si = getShotSet()->get(0)->getCPInv()/1000;
	///////////////////////////////////////////////////////
	QString s = QFileDialog::getOpenFileName(
		QString::null,
		"txt (*.*)",
		0,
		"open file dialog"
		"Choose a file name" );
	if(s=="")
		return false;
	QFile file(s);
	if(!file.open(IO_ReadOnly))
		return false;
//	QProgressDialog progress( "loading....", "Abort Load", file.size(),
//		0, "progress", TRUE );
	QProgressDialog progress(QString::fromLocal8Bit("ļ..."), 
 						  QString::fromLocal8Bit("ȡ"), 0,file.size(),0);
	//////////
	QTextStream stream(&file);
	int oldFFid,newFFid;
	oldFFid = newFFid = -1;
	int tempTrcCount=0;
	while(!file.atEnd())
	{
		QString line = stream.readLine();
		progress.setValue(file.at());
		if(progress.wasCanceled())
			break;
		int data1,data2,data3;
		int dataCount = sscanf(line,"%d %d %d",&data1,&data2,&data3);
		if(dataCount<3)
			continue;
		newFFid = data1;
        GShot *pShot = getShotSet()->getByRecNum(data1);
		if(pShot==NULL)
			continue;
		int *fbt = pShot->getFBTByIndex(data2);
		fbt[0] = data3*1.0/si;
		pShot->setQCFBTByIndex(data2,1);
		
	}
    file.close();
	save();
   return true;
}
/*
bool GProject::loadFBT2()
{
	int si = getShotSet()->get(0)->getCPInv()/1000;
	///////////////////////////////////////////////////////
	QString s = QFileDialog::getOpenFileName(
		QString::null,
		"txt (*.*)",
		0,
		"open file dialog"
		"Choose a file name" );
	if(s=="")
		return false;
	QFile file(s);
	if(!file.open(IO_ReadOnly))
		return false;
	QProgressDialog progress( "loading....", "Abort Load", file.size(),
		0, "progress", TRUE );
	//////////
	QTextStream stream(&file);
	int oldFFid,newFFid;
	oldFFid = newFFid = -1;
	int tempTrcCount=0;
	while(!file.atEnd())
	{
		QString line = stream.readLine();
		progress.setProgress(file.at());
		if(progress.wasCancelled())
			break;
		int sRecNum;
		int trcCount;
		float sx,sy,sz,sh;
		int dataCount = sscanf(line,"%d %d %f %f %f %f",&sRecNum,&trcCount,&sx,&sy,&sz,&sh);
		if(dataCount<6)
			continue;
		GShot *shot = getShotSet()->getByRecNum(sRecNum);
		for(int i=0; i<trcCount; i++)
		{
			  int rRec;
			  float rx,ry,rz,time;
              int rDataCount  = sscanf(line,"%d  %f %f %f %f",&rRec,&rx,&ry,&rz,&time);
			  if (rDataCount<5)
			  {
				  continue;
			  }
              int rLineNum,rPointNum;
			  rLineNum = rRec/1000;
			  rPointNum = rRec-rLineNum*1000;
			  GRPoint *rPoint = getRLineSet()->getByLineName(QString::number(rLineNum))->getByPointNum(QString::number(rPointNum));
			  int index= shot->getTrcIndexByRPoint(rPoint);
              shot->setFBTDataByIndex(index,time*1000/si);

		}	
		
	}
    file.close();
	save();
	return true;
}
*/
/*
void GProject::addRStatic()
{
	QString fileName = QFileDialog::getOpenFileName(
                          "/home","RPoint Static File (*.rst);;"
						  	"Any files(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	/////////////////////////////////////////
	 GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GRLineSet *rLineSet = pProject->getRLineSet();
    int count = rLineSet->count();
  	QFile file(fileName);
	if(file.open(IO_ReadOnly))
	{
		Q_INT32 fileLong = file.size();
		QProgressDialog *pdialog=new QProgressDialog( 
		"add r static ","Cancel",fileLong, NULL, 0,true,0);
    	pdialog->reset();

		 QTextStream stream(&file);
		 int i=0;
		 while ( !stream.atEnd() )
		 {
			 pdialog->setProgress(file.at());
			QString string1 =  stream.readLine();
			QString string2 = string1.mid(0,5);
		//	string2+="\n";
			QString string3 = string1.mid(5,15);
		//	string2.stripWhiteSpace();
		    QString pointNum = string2;
			float sta = string3.toFloat();
		//	sta = -sta;
			
       		for(int i=0; i<count; i++ )
			{
				GRLine *rLine = rLineSet->get(0);
				GRPoint *pRPoint = rLine->getByPointNum(pointNum);
				if(pRPoint == NULL) continue;
				float tempsta = pRPoint->getStatica();
			//pRPoint->setStatic(-1*sta+tempsta);
					pRPoint->setStatic(sta+tempsta);
				break;
			}
		
			
				GRLine *rLine = rLineSet->get(0);
				GRPoint *pRPoint = rLine->get(i);
				if(pRPoint == NULL) continue;
				float tempsta = pRPoint->getStatica();
			//pRPoint->setStatic(-1*sta+tempsta);
				pRPoint->setStatic(sta+tempsta);
			    i++;
			
		 }	
		 pdialog->setProgress(100,100);
	   delete pdialog;
	 }
	GRLine *rLine = rLineSet->get(0);
	float *data = new float[rLine->count()];
	for(int i=0; i<rLine->count(); i++)
	{
		data[i] = rLine->get(i)->getStatica();
	}
	amplitude(data,rLine->count());
	for( i=0; i<rLine->count(); i++)
	{
		rLine->get(i)->setStatic(data[i]);
	}
	
}

void GProject::addSStatic()
{
	QString fileName = QFileDialog::getOpenFileName(
                          "/home","SPoint Static File (*.sst);;"
						  "Any file(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	////////////////////////
	 GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GSLineSet *shotLineSet = pProject->getSLineSet();
  int count = shotLineSet->count();
  	QFile file(fileName);
	if(file.open(IO_ReadOnly))
	{
		 QTextStream stream(&file);
		 while ( !stream.atEnd() )
		 {
			QString string1 =  stream.readLine();
			QString string2 = string1.mid(0,5);
		//	string2+="\n";
			QString string3 = string1.mid(5,15);
		//	string2.stripWhiteSpace();
		    QString pointNum = string2;
			float sta = string3.toFloat();
			////////
		//	sta = -sta;
			/////////
			for(int i=0; i<count; i++ )
			{
			//	GSLine *sLine = shotLineSet->get(i);
				GSLine *sLine = shotLineSet->get(0);
				GSPoint *psPoint = sLine->getByPointNum(pointNum);
				if(psPoint == NULL) continue;
				float tempsta = psPoint->getStatic();
			//	psPoint->setStatic(-1*sta+tempsta);
				psPoint->setStatic(sta+tempsta);
			}
		 
		 }
	 }

}
*/
/*
void GProject::saveSStatic()
{

	QString fileName = QFileDialog::getSaveFileName(
                          "/home","SPoint Static File (*.sst);;"
						  	"Any files(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	////////////////
	 GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GSLineSet *shotLineSet = pProject->getSLineSet();
   	GSLine *sLine = shotLineSet->get(0);
	////////////////////
  	QFile file(fileName+".sst");
	QTextStream stream(&file);
	if(file.open(IO_WriteOnly))
	{		
		for(int j=0; j<sLine->count(); j++ )
		{ 
		  stream <<sLine->get(j)->getPointNum()<<"            "<<
			  sLine->get(j)->getStatic()<<"\r\n";
		}
	}
	
	QString fileName = QFileDialog::getSaveFileName(
		"/home","SPoint Static File (*.sst);;"
		"Any files(*)",
		0,
		"open file dialog"
		"Enter a file name" );
	////////////////
	GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();
	
	GShotSet *shotSet = getCurrentProject()->getShotSet();
	////////////////////
	QFile file(fileName+".sst");
	QTextStream stream(&file);
	if(file.open(IO_WriteOnly))
	{		
		for(int j=0; j<shotSet->count(); j++ )
		{ 
			stream <<shotSet->get(j)->getPointNum()<<"            "<<
				shotSet->get(j)->getStatic()<<"\r\n";
		}
	}
}
*/
/*
void GProject::saveRStatic()
{

	QString fileName = QFileDialog::getSaveFileName(
                          "/home","RPoint Static File (*.rst);;"
						  	"Any files(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	///////////
		////////////////
    GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GRLineSet *rLineSet = pProject->getRLineSet();
  	GRLine *rLine = rLineSet->get(0);
	////////////

  	QFile file(fileName+".rst");
		QTextStream stream(&file);
	if(file.open(IO_WriteOnly))
	{		
		for(int j=0; j<rLine->count(); j++ )
		{ 
		  stream <<rLine->get(j)->getPointNum()<<"     "<<
			  rLine->get(j)->getStatica()<<"\r\n";
		}
	}
	
	QString fileName = QFileDialog::getSaveFileName(
                          "/home","RPoint Static File (*.rst);;"
						  	"Any files(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	///////////
		////////////////
    GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GRLineSet *rLineSet = pProject->getRLineSet();
  	GRLine *rLine = rLineSet->get(0);
	////////////

  	QFile file(fileName+".rst");
		QTextStream stream(&file);
	if(file.open(IO_WriteOnly))
	{		
		for(int j=0; j<rLine->count(); j++ )
		{ 
		  stream <<rLine->get(j)->getPointNum()<<"     "<<
			  rLine->get(j)->getStatic()<<"\r\n";
		}
	}
}
*/

/*
void GProject::zeroRStatic()
{
   GProject *pProject;
	if(mPSVFlag==PPDATA)
	{
		pProject = getCurrentProject();
	}
	else
		pProject = getCurrentProjectSV();

	GRLineSet *rLineSet = pProject->getRLineSet();
    int count = rLineSet->count();
	for(int j=0; j<count; j++)
	{
		GRLine *rLine = rLineSet->get(j);
  		for(int i=0; i<rLine->count(); i++)
		{
			rLine->get(i)->setStatic(0);
		}
	}
    QMessageBox::information( 0, "Information",
    "Zero R Static Finished!.\n");
}
*/
/////////////////////////////////////////////////////////
void GProject::addRDiff()
{
	QString fileName = QFileDialog::getOpenFileName(
                          "/home","RPoint Time Diff File (*.rst);;"
						  	"Any files(*)",
                            0,
                     "open file dialog"
                      "Enter a file name" );
	/////////////////////////////////////////
	 GProject *pProject;
//	if(mPSVFlag==PPDATA)
//	{
		pProject = getCurrentProject();
//	}
//	else
//		pProject = getCurrentProjectSV();

	GRLineSet *rLineSet = pProject->getRLineSet();
    int count = rLineSet->count();
  	QFile file(fileName);
	if(file.open(IO_ReadOnly))
	{
		Q_INT32 fileLong = file.size();
//		QProgressDialog *pdialog=new QProgressDialog( 
//		"add r static ","Cancel",fileLong, NULL, 0,true,0);
		QProgressDialog *pdialog=new QProgressDialog(
		"add r static ","Cancel", 0,file.size(),0);

    	pdialog->reset();

		 QTextStream stream(&file);
		 int i=0;
		 while ( !stream.atEnd() )
		 {
			 pdialog->setValue(file.at());
			QString string1 =  stream.readLine();
			QString string2 = string1.mid(0,5);
		//	string2+="\n";
			QString string3 = string1.mid(5,15);
		//	string2.stripWhiteSpace();
		    QString pointNum = string2;
			float sta = string3.toFloat();
		//	sta = -sta;
			
       		for(int i=0; i<count; i++ )
			{
				GRLine *rLine = rLineSet->get(0);
				GRPoint *pRPoint = rLine->getByPointNum(pointNum);
				if(pRPoint == NULL) continue;
					pRPoint->setTimeDiff(sta);
				break;
			}
		
		 }	
		 pdialog->setValue(file.size());
	   delete pdialog;
	 }
}

float GProject::getMaxRLineLong()
{
  float max = 0;
  for(int i=0; i<mpRLineSet->count(); i++)
  {
	GRLine *rLine = mpRLineSet->get(i);
	int count = rLine->count();
	float x1 = rLine->get(0)->getX();
	float y1 = rLine->get(0)->getY();
	float x2 = rLine->get(count-1)->getX();
	float y2 = rLine->get(count-1)->getY();
    float offset = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	if(max<offset)
		max = offset;
  }
  return max;
}

int GProject::getShotIndexCount()
{
	return mpSLineSet->count();
}

QString GProject::getSPSFileName()// add by zhaohu
{
	QFileInfo fi(mPrjFileName);
	return fi.dirPath() + "/" + mPrjName + ".sps";
}

double GProject::getAngle( double sinAF, double cosAF )
{
	double af = asin(sinAF);//[-11]Χڣasin()[-/ 2/ 2]Χڵֵ

	if(cosAF < 0) af = PI-af;//۽ǣڶޣ
	else if(sinAF < 0) af = 2*PI+af;//

	return af;//صĽǶΪ[0,2PI]
}
