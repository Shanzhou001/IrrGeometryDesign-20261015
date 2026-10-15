#include "gshotset.h"
#include "grlineset.h"
#include "gslineset.h"
#include "gproject.h"
#include "gmath.h"
#include "gutils.h"


//----ڲȫֺ--------------------------------------------------------------
int gCompareShotByX(GShot *p1, GShot *p2 )
{
	if( p1->getSPoint()->getX() > p2->getSPoint()->getX() ) return 1;
	else if( p1->getSPoint()->getX() < p2->getSPoint()->getX() ) return -1;
	else return 0;
}

int gCompareShotByY(GShot *p1, GShot *p2 )
{
	if( p1->getSPoint()->getY() > p2->getSPoint()->getY() ) return 1;
	else if( p1->getSPoint()->getY() < p2->getSPoint()->getY() ) return -1;
	else return 0;
}

//----ȽϺϳ׮----
int gCompareShotByStackNum(GShot *p1, GShot *p2)
{
	int stk1, stk2, bit;

	stk1 = gExtractInt(p1->getLineName(), bit);
	stk1 *= pow((double)10, bit+1);
	stk1 += gExtractInt(p1->getPointNum(), bit);

	stk2 = gExtractInt(p2->getLineName(), bit);
	stk2 *= pow((double)10, bit+1);
	stk2 += gExtractInt(p2->getPointNum(), bit);

	if(stk1 > stk2) return 1;
	else if(stk1 < stk2) return -1;
	else return 0;
}


int gCompareShotByLineAndPoint(GShot *p1, GShot *p2)
{
	int stk1, stk2, bit;

	stk1 = gExtractInt(p1->getLineName(), bit);
	stk1 *= pow((double)10, bit+1);
	stk1 += gExtractInt(p1->getPointNum(), bit);

	stk2 = gExtractInt(p2->getLineName(), bit);
	stk2 *= pow((double)10, bit+1);
	stk2 += gExtractInt(p2->getPointNum(), bit);

	if(stk1 > stk2) return 1;
	else if(stk1 < stk2) return -1;
	else return 0;
}

int gCompareShotByPointAndLine(GShot *p1, GShot *p2)
{
	int stk1, stk2, bit;

	stk1 = gExtractInt(p1->getPointNum(), bit);
	stk1 *= pow((double)10, bit+1);
	stk1 += gExtractInt(p1->getLineName(), bit);

	stk2 = gExtractInt(p2->getPointNum(), bit);
	stk2 *= pow((double)10, bit+1);
	stk2 += gExtractInt(p2->getLineName(), bit);

	if(stk1 > stk2) return 1;
	else if(stk1 < stk2) return -1;
	else return 0;
}

int gCompareFieldRecordNumber( GShot *p1, GShot *p2 )
{
	if( p1->getRecNum() > p2->getRecNum() ) return 1;
	else if( p1->getRecNum() < p2->getRecNum() ) return -1;
	else return 0;
}
/*
int gAntiCompareFieldRecordNumber( GShot *p1, GShot *p2 )
{
	if( p1->getRecNumber() < p2->getRecNumber() ) return 1;
	else if( p1->getRecNumber() > p2->getRecNumber() ) return -1;
	else return 0;
}*/

int gCompareCOPArrayByX( GShotXArray *p1, GShotXArray *p2 )
{
	if( p1->getShotX() > p2->getShotX() ) return 1;
	else if( p1->getShotX() < p2->getShotX() ) return -1;
	else return 0;
}

//----class GShotXArray---------------------------------------------------------
GShotXArray::GShotXArray()
{
	mArrayType = gLeftXArray;
	mFilePos = 0;
	mFirstPt = QString::null;	mFirstPtIndex = 0;	mFirstTrc = 0;
	mLastPt = QString::null;	mLastPtIndex = 0;	mLastTrc = 0;
	mPriTimeArray = NULL;
	mPriTimeArraySV = NULL;
	mTrcCount = 0;
	mTrcInc = 0;
	mFitDifference = NULL;
}

GShotXArray::GShotXArray(QString firstPt, QString lastPt, Q_INT32 firstTrc, Q_INT32 lastTrc,
						 Q_INT32 trcInc, Q_INT32 trcCount)
{
	mArrayType = gLeftXArray;
	mFilePos = 0;
	mFirstPt = firstPt;	mFirstPtIndex = 0;	mFirstTrc = firstTrc;
	mLastPt = lastPt;	mLastPtIndex = 0;	mLastTrc = lastTrc;
	mPriTimeArray = new Q_INT32[trcCount];
	mPriTimeArraySV = new Q_INT32[trcCount];
	mFitDifference = new float[trcCount];
	for(int i=0; i<trcCount; i++)
	{
		mPriTimeArray[i] = UN_PICK_UP_FBT;
		mPriTimeArraySV[i] = UN_PICK_UP_FBT;
		mFitDifference = 0;
	}
	mTrcCount = trcCount;
	mTrcInc = trcInc;
}

GShotXArray::~GShotXArray()
{
	if(mPriTimeArray != NULL) 
	{
		delete []mPriTimeArray;
	}
	if(mPriTimeArraySV != NULL) 
	{
		delete []mPriTimeArraySV;
	}
	if(mFitDifference!=NULL)
		delete []mFitDifference;
}

bool GShotXArray::isSingle(float sx, GRLine *pRLine)
{
	GRPoint *pFRPoint, *pLRPoint;
	pFRPoint = pRLine->getByPointNum(mFirstPt);
	pLRPoint = pRLine->getByPointNum(mLastPt);
	if((pFRPoint->getX()-sx)*(pLRPoint->getX()-sx) >= 0) //
		return true;
	else return false;
}

void GShotXArray::setArrayType(float sx, GRLine *pRLine)
{
	GRPoint *pFRPoint, *pLRPoint;
	pFRPoint = pRLine->getByPointNum(mFirstPt);
	pLRPoint = pRLine->getByPointNum(mLastPt);
	if(pFRPoint->getX() >= sx) mArrayType = gRightXArray;
	else if(pLRPoint->getX() <= sx) mArrayType = gLeftXArray;
}

GShotXArray * GShotXArray::splitXArray(float sx, GRLine *pRLine)
{

/*   int i, j, findex, lindex, count;
	GRPoint *pRPoint, *pFRPoint, *pLRPoint;

	findex = pRLine->getIndexByPointNum(mFirstPt);
	lindex = pRLine->getIndexByPointNum(mLastPt);
   	pFRPoint = pRLine->get(findex);
	pLRPoint = pRLine->get(lindex);
	//////////////////////////
	GShotXArray *pXArray = new GShotXArray;
	pXArray->mTrcInc = mTrcInc;
	if(pFRPoint->getX() < sx)	//һ֧,β֧
	{
		count = 0;
		for(i=findex; i<=lindex; i++)
		{
			pRPoint = pRLine->get(i);
			if(pRPoint->getX() > sx) break;
			count++;
		}
		pXArray->mFirstSDistance = fabs(pRPoint->getX()-sx);
		//֧
		pXArray->mArrayType = gRightXArray;
		pXArray->mFirstPt = pRPoint->getPointNum();
		pXArray->mFirstPtIndex = i;
		pXArray->mLastPt = mLastPt;
		pXArray->mLastPtIndex = mLastPtIndex;
		pXArray->mFirstTrc = mFirstTrc+count*mTrcInc;
		pXArray->mLastTrc = mLastTrc;
		pXArray->mTrcCount = mTrcCount-count;
		pXArray->mPriTimeArray = new Q_INT32[pXArray->mTrcCount];
		pXArray->mPriTimeArraySV = new Q_INT32[pXArray->mTrcCount];
		pXArray->mFitDifference = new float[pXArray->mTrcCount];
		for(j=0; j<pXArray->mTrcCount; j++)
		{
			pXArray->mPriTimeArray[j] = UN_PICK_UP_FBT;
			pXArray->mPriTimeArraySV[j] = UN_PICK_UP_FBT;
			pXArray->mFitDifference[j] = 0;
		}
		//֧
		mArrayType = gLeftXArray;
		mLastTrc = pXArray->mFirstTrc-mTrcInc;
		mLastPtIndex = i-1;
		pRPoint = pRLine->get(mLastPtIndex);
		mLastPt = pRPoint->getPointNum();
		mTrcCount = count;
		delete []mPriTimeArray;
		delete []mPriTimeArraySV;
		delete []mFitDifference;
		mPriTimeArray = new Q_INT32[mTrcCount];
		mPriTimeArraySV = new Q_INT32[mTrcCount];
		mFitDifference = new float[mTrcCount];
		for(j=0; j<mTrcCount; j++) 
		{
			mPriTimeArray[j] = UN_PICK_UP_FBT;
			mPriTimeArraySV[j] = UN_PICK_UP_FBT;
			mFitDifference[j] = 0;
		}
	}
	else	//һ֧,β֧
	{
		count = 0;
		for(i=findex; i<=lindex; i++)
		{
			pRPoint = pRLine->get(i);
			if( pRPoint->getX() < sx ) break;
			count++;
		}
		pXArray->mFirstSDistance = fabs(pRPoint->getX()-sx);
		//֧
		pXArray->mArrayType = gLeftXArray;
		pXArray->mFirstPt = pRPoint->getPointNum();
		pXArray->mFirstPtIndex = i;
		pXArray->mLastPt = mLastPt;
		pXArray->mLastPtIndex = mLastPtIndex;
		pXArray->mFirstTrc = mFirstTrc+count*mTrcInc;
		pXArray->mLastTrc = mLastTrc;
		pXArray->mTrcCount = mTrcCount-count;
		pXArray->mPriTimeArray = new Q_INT32[pXArray->mTrcCount];
		pXArray->mPriTimeArraySV = new Q_INT32[pXArray->mTrcCount];
        pXArray->mFitDifference = new float[pXArray->mTrcCount];
		for(j=0; j<pXArray->mTrcCount; j++) 
		{
			pXArray->mPriTimeArray[j] = UN_PICK_UP_FBT;
			pXArray->mPriTimeArraySV[j] = UN_PICK_UP_FBT;
			pXArray->mFitDifference[j] = 0;
		}
		//֧
		mArrayType = gRightXArray;
		mLastTrc = pXArray->mFirstTrc-mTrcInc;
		mLastPtIndex = i-1;		
		pRPoint = pRLine->get(mLastPtIndex);
		mLastPt = pRPoint->getPointNum();
		mTrcCount = count;
		delete []mPriTimeArray;
		delete []mPriTimeArraySV;
		delete []mFitDifference;
		mPriTimeArray = new Q_INT32[mTrcCount];
		mPriTimeArraySV = new Q_INT32[mTrcCount];
		mFitDifference = new float[mTrcCount];
		for( j=0; j<mTrcCount; j++ ) 
		{
			mPriTimeArray[j] = UN_PICK_UP_FBT;
			mPriTimeArraySV[j] = UN_PICK_UP_FBT;
            mFitDifference[j] = 0;
		}
	}

	return pXArray;*/
	int i, j, findex, lindex, count;
	GRPoint *pRPoint, *pFRPoint, *pLRPoint;

	findex = pRLine->getIndexByPointNum(mFirstPt);
	pFRPoint = pRLine->get(findex);
	lindex = pRLine->getIndexByPointNum(mLastPt);
	pLRPoint = pRLine->get(lindex);

	GShotXArray *pXArray = new GShotXArray;
	pXArray->mTrcInc = mTrcInc;
	if(pFRPoint->getX() < sx)	//һ֧,β֧
	{
		count = 0;
		for(i=findex; i<=lindex; i++)
		{
			pRPoint = pRLine->get(i);
			if(pRPoint->getX() > sx) break;
			count++;
		}
		pXArray->mFirstSDistance = fabs(pRPoint->getX()-sx);
		//֧
		pXArray->mArrayType = gRightXArray;
		pXArray->mFirstPt = pRPoint->getPointNum();
		pXArray->mFirstPtIndex = i;
		pXArray->mLastPt = mLastPt;
		pXArray->mLastPtIndex = mLastPtIndex;
		pXArray->mFirstTrc = mFirstTrc+count*mTrcInc;
		pXArray->mLastTrc = mLastTrc;
		pXArray->mTrcCount = mTrcCount-count;
		pXArray->mPriTimeArray = new Q_INT32[pXArray->mTrcCount];
		for(j=0; j<pXArray->mTrcCount; j++)
		{
			pXArray->mPriTimeArray[j] = UN_PICK_UP_FBT;
		}
		//֧
		mArrayType = gLeftXArray;
		mLastTrc = pXArray->mFirstTrc-mTrcInc;
		mLastPtIndex = i-1;
		pRPoint = pRLine->get(mLastPtIndex);
		mLastPt = pRPoint->getPointNum();
		mTrcCount = count;
		delete []mPriTimeArray;
		mPriTimeArray = new Q_INT32[mTrcCount];
		for(j=0; j<mTrcCount; j++) 
		{
			mPriTimeArray[j] = UN_PICK_UP_FBT;
		}
	}
	else	//һ֧,β֧
	{
		count = 0;
		for(i=findex; i<=lindex; i++)
		{
			pRPoint = pRLine->get(i);
			if( pRPoint->getX() < sx ) break;
			count++;
		}
		pXArray->mFirstSDistance = fabs(pRPoint->getX()-sx);
		//֧
		pXArray->mArrayType = gLeftXArray;
		pXArray->mFirstPt = pRPoint->getPointNum();
		pXArray->mFirstPtIndex = i;
		pXArray->mLastPt = mLastPt;
		pXArray->mLastPtIndex = mLastPtIndex;
		pXArray->mFirstTrc = mFirstTrc+count*mTrcInc;
		pXArray->mLastTrc = mLastTrc;
		pXArray->mTrcCount = mTrcCount-count;
		pXArray->mPriTimeArray = new Q_INT32[pXArray->mTrcCount];
		for(j=0; j<pXArray->mTrcCount; j++) 
		{
			pXArray->mPriTimeArray[j] = UN_PICK_UP_FBT;
		}
		//֧
		mArrayType = gRightXArray;
		mLastTrc = pXArray->mFirstTrc-mTrcInc;
		mLastPtIndex = i-1;		
		pRPoint = pRLine->get(mLastPtIndex);
		mLastPt = pRPoint->getPointNum();
		mTrcCount = count;
		delete []mPriTimeArray;
		mPriTimeArray = new Q_INT32[mTrcCount];
		for( j=0; j<mTrcCount; j++ ) 
		{
			mPriTimeArray[j] = UN_PICK_UP_FBT;
		}
	}

	return pXArray;
}


void GShotXArray::save(QDataStream &stream)
{
	stream << (Q_INT32)mArrayType;
	stream << mFilePos;
	stream << mMinTrcIndex << mMaxTrcIndex;
	stream << mFirstTrc << mLastTrc << mTrcInc;
	stream << mFirstPt << mLastPt;
	stream << mFirstPtIndex << mLastPtIndex;
	stream << mTrcCount;
}

void GShotXArray::savePriTime(QDataStream &tstream)
{
	for(int i=0; i<mTrcCount; i++)
	{
		tstream << mPriTimeArray[i];
	}
}

void GShotXArray::savePriTimeSV(QDataStream &tstream)
{
	for(int i=0; i<mTrcCount; i++)
	{
		tstream << mPriTimeArraySV[i];
	}
}

void GShotXArray::save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV)
{
	save(stream);
	savePriTime(tstream);
	savePriTimeSV(tstreamSV);

}

void GShotXArray::load( QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV )
{
	Q_INT32 arType;
	stream >> arType;
	mArrayType = (GShotXArrayType)arType;
	stream >> mFilePos;
	stream >> mMinTrcIndex >> mMaxTrcIndex;
	stream >> mFirstTrc >> mLastTrc >> mTrcInc;
	stream >> mFirstPt >> mLastPt;
	stream >> mFirstPtIndex >> mLastPtIndex;
	stream >> mTrcCount;
	//ļ
	mPriTimeArray = new Q_INT32[mTrcCount];
	mPriTimeArraySV = new Q_INT32[mTrcCount];
	mFitDifference = new float[mTrcCount];
	for( int i=0; i<mTrcCount; i++ )
	{
		tstream >> mPriTimeArray[i];
		tstreamSV >> mPriTimeArraySV[i];
        mFitDifference[i] = 0;
	}
}


bool GShotXArray::load(QDataStream &stream)//add by zhaohu
{
	Q_INT32 arType;
	stream >> arType;
	mArrayType = (GShotXArrayType)arType;
	stream >> mMinTrcIndex >> mMaxTrcIndex;
	stream >> mFirstTrc >> mLastTrc >> mTrcInc;
	stream >> mFirstPt >> mLastPt;
	stream >> mFirstPtIndex >> mLastPtIndex;
	stream >> mTrcCount;
	gSkipPadBytes(stream, GXARRAY_PAD_SIZE);

	if(stream.device()->status() != IO_Ok) return false;
	else return true;
}

//----class GShotXLine----------------------------------------------------------
GShotXLine::GShotXLine()
{
	mLineName = QString::null;
	mLineIndex = -1;
	mMinIndex = 0;		mMaxIndex = 0;
	mTrcCount = 0;

	mpXArrayList = new GPtrVector<GShotXArray>(2);
	mpXArrayList->setAutoDelete(true);

	mMinTrc = MAX_INT32;
	mMaxTrc = MIN_INT32;
}

GShotXLine::GShotXLine(const QString &lineName)
{
	mLineName = lineName;
	mLineIndex = -1;
	mMinIndex = 0;		mMaxIndex = 0;
	mTrcCount = 0;

	mpXArrayList = new GPtrVector<GShotXArray>(2);
	mpXArrayList->setAutoDelete(true);

	mMinTrc = MAX_INT32;
	mMaxTrc = MIN_INT32;
}

GShotXLine::~GShotXLine()
{
	delete mpXArrayList;
}

GShotXArray * GShotXLine::get(int index)
{
	if(0 <= index && index < count()) return mpXArrayList->at(index);
	else return NULL;
}

GShotXArray * GShotXLine::getXArrayByTrc(Q_INT32 firstTrc)
{
	uint i;
	GShotXArray *pXArray;
	for(i=0; i<mpXArrayList->count(); i++)
	{
		pXArray = mpXArrayList->at(i);
		if(firstTrc == pXArray->getFirstTrc())
		{
			return pXArray;
		}
	}
	return NULL;
}

bool GShotXLine::append(GShotXArray *pXArray)
{
	if(!mpXArrayList->append(pXArray)) return false;
	if(pXArray->getFirstTrc() < mMinTrc) mMinTrc = pXArray->getFirstTrc();
	if(pXArray->getLastTrc() > mMaxTrc) mMaxTrc = pXArray->getLastTrc();
	return true;
}

void GShotXLine::save(QDataStream &stream)
{
	stream << mLineName;
	stream << mLineIndex << mTrcCount;
	stream << mMinTrc << mMaxTrc;
	stream << mMinIndex << mMaxIndex;
	stream << (Q_INT32)mpXArrayList->count();

	for(int i=0; i<count(); i++)
	{
		GShotXArray *pXArray = mpXArrayList->at(i);
		pXArray->save(stream);
	}
}

void GShotXLine::save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV)
{
	stream << mLineName;
	stream << mLineIndex << mTrcCount;
	stream << mMinTrc << mMaxTrc;
	stream << mMinIndex << mMaxIndex;
	stream << (Q_INT32)mpXArrayList->count();

	for(int i=0; i<count(); i++)
	{
		GShotXArray *pXArray = mpXArrayList->at(i);
		pXArray->save(stream, tstream,tstreamSV);
	}
}

void GShotXLine::savePriTime(QDataStream &tstream)
{
	for(int i=0; i<count(); i++)
	{
		GShotXArray *pXArray = mpXArrayList->at(i);
		pXArray->savePriTime(tstream);
	}
}

void GShotXLine::savePriTimeSV(QDataStream &tstream)
{
	for(int i=0; i<count(); i++)
	{
		GShotXArray *pXArray = mpXArrayList->at(i);
		pXArray->savePriTimeSV(tstream);
	}
}

void GShotXLine::load( QDataStream &stream, QDataStream &tstream ,QDataStream &tstreamSV)
{
	stream >> mLineName;
	stream >> mLineIndex >> mTrcCount;
	stream >> mMinTrc >> mMaxTrc;
	stream >> mMinIndex >> mMaxIndex;

	Q_INT32 count;
	stream >> count;

	for(int i=0; i<count; i++)
	{
		GShotXArray *pXArray = new GShotXArray;
		pXArray->load(stream, tstream,tstreamSV);
		mpXArrayList->append(pXArray);
	}
}


bool GShotXLine::load(QDataStream &stream)//add by zhaohu
{
	Q_INT32 count;
	stream >> mLineName;
	stream >> mLineIndex >> mTrcCount;
	stream >> mMinTrc >> mMaxTrc;
	stream >> mMinIndex >> mMaxIndex;
	stream >> count;
	gSkipPadBytes(stream, GXLINE_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) return false;

	for(int i=0; i<count; i++)
	{
		GShotXArray *pXArray = new GShotXArray();
		if(!pXArray->load(stream))
		{
			mpXArrayList->clear();
			return false;
		}
		mpXArrayList->append(pXArray);
	}

	return true;
}

//----class GShot---------------------------------------------------------------
GShot::GShot()
{
	mpXLineList = new GPtrVector<GShotXLine>(8);
	mpXLineList->setAutoDelete(true);
	init();
}

GShot::GShot(Q_INT32 recNum, const QString &lineName, const QString &pointNum)
{
	mpXLineList = new GPtrVector<GShotXLine>(8);
	mpXLineList->setAutoDelete(true);
	init();

	mLineName = lineName;
	mPointNum = pointNum;
	mRecNum = recNum;
}

GShot::~GShot()
{
	if(mQCFBT!=NULL)
	{
		delete []mQCFBT;
	}
	if (mTrcNum!=NULL)
	{
		delete []mTrcNum;
	}
	//////////////////add for bug 110119
	if (mRLIndex!=NULL)
	{
		delete []mRLIndex;
	}
	if (mRPIndex!=NULL)
	{
		delete []mRPIndex;
	}

	//if (mpSPoint!=NULL) // Ϊⲿݽָ룬ԲҪɾ [Liu Wei 6/19/2023]
	//{
	//	delete mpSPoint;
	//}

	////////////////
	delete mpXLineList;
}

void GShot::init()
{
	//<1>ʼ
	mState = siUnSaved;
	mPosState = 1;
	mLineName = QString::null;
	mPointNum = QString::null;
	mRecNum = 0;
	mFileIndex = -1;
	mFilePos = 0;
	mDataPos = 0;
	mTAOPos	= 0;
	mTBPos = 0;
	mCPCount = 0;
	mCPInv = 0;
	mQCFBT = NULL;
	mTrcNum = NULL;
	//////////////////add for bug 110119
	mRLIndex = NULL;
	mRPIndex = NULL;

	mpSPoint = NULL;//  [Liu Wei 6/19/2023]
}

float GShot::getStatic()
{
	GProject *pProject= mpProject;	
	return 1000*(getSPoint()->getSPSZ() - pProject->getBase()) / pProject->getVelocity();
}

void GShot::setFileIndex(int index)
{
	mFileIndex = index;
    if (mTrcNum!=NULL)
   delete []mTrcNum;
	mTrcNum = new int[mTrcCount*2];

 }

///////////add by psl 110119
int GShot::setRLineAndPointIndex(QDataStream *dataStream)
{
	//////////////////add for bug 110119
	if (mRLIndex!=NULL)
	{
		delete []mRLIndex;
	}
	if (mRPIndex!=NULL)
	{
		delete []mRPIndex;
	}
	////////////////
	//////////////////add for bug 110119
    if (mRLIndex!=NULL)
		delete []mRLIndex;
	mRLIndex = new int[getTrcCount()];
	if (mRPIndex!=NULL)
		delete []mRPIndex;
	mRPIndex = new int[getTrcCount()];
	//޸Ϊݵͷ¼첨׮Żü첨
		Q_INT32 rStack;
		Q_INT32 tempRec = getRecNum()+30000;
		Q_INT32 recNum;
		int tempTrcCount = 0;
		QFile *pFile = (QFile *)dataStream->device();
		for(int i=0; i<getTrcCount(); i++)
		{
			long long pos= getTrcOffsetByIndex(i);
			pFile->at(pos+8);
		    *dataStream >> recNum;	
			if(recNum==tempRec)
			{
				pFile->at(pos+56);
				*dataStream >> rStack;
				QString lineName;
				QString pointName;
				int temp = rStack/1000;
				lineName = QString::number(temp);
				pointName = QString::number(rStack-1000*temp);
				mRLIndex[tempTrcCount] = GProject::getCurrentProject()->getRLineSet()->getIndexByLineName(lineName);
				mRPIndex[tempTrcCount] = GProject::getCurrentProject()->getRLineSet()
					->get(mRLIndex[i])->getIndexByPointNum(pointName);
                tempTrcCount++;
			}
		
		}
	return tempTrcCount;
}

bool GShot::getTrcByIndex(int index, int &trcNum)
{
	GShotXLine *pXLine = NULL;
	GShotXArray *pXArray = NULL;
	return getTrcInfoByIndex(index, trcNum, pXLine, pXArray);
}

bool GShot::getIndexByTrc(int &index, int trcNum)
{
	for( int i=0; i<count(); i++ )
	{
		GShotXLine *pXLine = get( i );
		for( int j=0; j<pXLine->count(); j++ )
		{
			GShotXArray *pXArray = pXLine->get( j );
			if(pXArray->getFirstTrc() <= trcNum &&
			   trcNum <= pXArray->getLastTrc()) 
			{
				index = pXArray->getMinTrcIndex()+
						(trcNum-pXArray->getFirstTrc()) / pXArray->getTrcInc();
				return true;
			}
		}
	}
	return false;
}

long long GShot::getTrcOffset(int trc)
{
	for( int i=0; i<count(); i++ )
	{
		GShotXLine *pXLine = mpXLineList->at(i);
		if( pXLine->getMinTrc() <= trc && trc <= pXLine->getMaxTrc() )
		{
			for( int j=0; j<pXLine->count(); j++ )
			{
				GShotXArray *pXArray = pXLine->get(j);
				if( pXArray->getFirstTrc() <= trc && trc <= pXArray->getLastTrc() )
				{
					long long temp1= 240+mCPCount*4;
					long long temp2=temp1*(long long)trc-pXArray->getFirstTrc();
					long long temp3=temp2/(long long )pXArray->getTrcInc();
					return pXArray->getFilePos()+temp3;//modify by zhaohu

// 					return pXArray->getFilePos()+(240+mCPCount*4)*( trc-pXArray->getFirstTrc() )/pXArray->getTrcInc();
				}
			}
		}
	}
	return 0;	//ûҵõ
}

long long GShot::getTrcOffsetByIndex(int index)
{
	if(index < 0 || index >= getTrcCount()) return 0; //ûиõ

	long long temp1=240+mCPCount*4;
	long long temp2=temp1*(long long)index;
	return getDataPos()+temp2;//modify by zhaohu

//	return getDataPos()+(240+(long long)mCPCount*4)*index;
}

bool GShot::getTrcInfoByIndex( int index, Q_INT32 &trcNum, GShotXLine *pXLine, GShotXArray *pXArray )
{
	int count, indexCount = 0;
	for( uint i=0; i<mpXLineList->count(); i++ )
	{
		pXLine = get( i );
		for( int j=0; j<pXLine->count(); j++ )
		{
			pXArray = pXLine->get( j );
			count = indexCount+pXArray->getTrcCount();
			if( index < count ) 
			{
				trcNum = (index-indexCount)*pXArray->getTrcInc()+pXArray->getFirstTrc();
				return true;
			}
			indexCount += pXArray->getTrcCount();
		}
	}
	return false;
}

bool GShot::getTrcIndexByRPoint( GRPoint *pRPoint, Q_INT32 &trcIndex )
{
	trcIndex = 0;
	for( int i=0; i<(int)(mpXLineList->count()); i++ )
	{
		GShotXLine *pXLine = get( i );		
		for( int j=0; j<pXLine->count(); j++ )
		{
			GShotXArray *pXArray = pXLine->get( j );
			if( pXLine->getLineName() == pRPoint->getLineName() )
			{
				if( pRPoint->getPointNum().toFloat() >= pXArray->getFirstPt().toFloat() &&
					pRPoint->getPointNum().toFloat() <= pXArray->getLastPt().toFloat() )
				{
					int incPt = (pXArray->getLastPt().toFloat()-pXArray->getFirstPt().toFloat()) / 
								(pXArray->getTrcCount()-1);
					trcIndex = trcIndex+
							   (pRPoint->getPointNum().toInt()-pXArray->getFirstPt().toInt())/incPt;
					return true;
				}
			}
			trcIndex += pXArray->getTrcCount();
		}
	}
	return false;
} 

/*bool GShot::getTrcIndexByRPoint( GRPoint *pRPoint, Q_INT32 &trcIndex )
{
	trcIndex = 0;
	for( int i=0; i<(int)(mpXLineList->count()); i++ )
	{
		GShotXLine *pXLine = get( i );		
		for( int j=0; j<pXLine->count(); j++ )
		{
			GShotXArray *pXArray = pXLine->get( j );
			if( pXLine->getLineName() == pRPoint->getLineName() )
			{
				if( pRPoint->getPointNum() >= pXArray->getFirstPt() &&
					pRPoint->getPointNum() <= pXArray->getLastPt() )
				{
					int incPt = (pXArray->getLastPt()-pXArray->getFirstPt()) / 
								(pXArray->getTrcCount()-1);
					trcIndex = trcIndex+
							   (pRPoint->getPointNum()-pXArray->getFirstPt())/incPt;
					return true;
				}
			}
			trcIndex += pXArray->getTrcCount();
		}
	}
	return false;
}
*/
float GShot::getTrcDistance(Q_INT32 index)
{
  GRPoint *rPoint = getRPointByTrcIndex(index);
  if(rPoint==NULL)
	  return -1;
  return gDistance2D(getSPoint()->getSPSX(),getSPoint()->getSPSY(),
		        rPoint->getSPSX(),rPoint->getSPSY());
}

GSPoint * GShot::getSPoint()
{ 
//	GProject *pProject=mpProject;
	 GProject *pProject=GProject::getCurrentProject();// add by zhao hu
	if(pProject != NULL)
		if(mpSPoint!=NULL) 
			return mpSPoint;
		else 
			return pProject->getSLineSet()->get(mLineIndex)->get(mPointIndex); // ½۲ϵͳʱڴʳִ(ΪgetSLineSet()صĶΪա [Liu Wei 6/5/2023]
		
	else return NULL;
}

GShotXLine * GShot::get(int index)
{
	if(0 <= index && index < count()) return mpXLineList->at(index);
	else return NULL;
}

GShotXLine * GShot::getXLineByName(const QString &lineName)
{
	int i;
	GShotXLine *pXLine;
	for(i=0; i<count(); i++)
	{
		pXLine = mpXLineList->at(i);
		if(lineName == pXLine->getLineName())
		{
			return pXLine;
		}
	}
	return NULL;
}

GShotXLine * GShot::getXLineByTrcIndex(int index)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = get(i);
		if(pXLine->getMinIndex() <= index && index <= pXLine->getMaxIndex())
		{
			return pXLine;
		}
	}

	return NULL;
}

GShotXArray * GShot::getXArrayByTrcIndex(int index)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = get(i);
		if(pXLine->getMinIndex() <= index && index <= pXLine->getMaxIndex())
		{
			for(int j=0; j<pXLine->count(); j++)
			{
				GShotXArray *pXArray = pXLine->get(j);
				if(pXArray->getMinTrcIndex() <= index && index <= pXArray->getMaxTrcIndex())
				{
					return pXArray;
				}
			}
		}
	}
	return NULL;
}

GRPoint * GShot::getRPointByTrcIndex(int index)
{
	GRLine *rLine = GProject::getCurrentProject()->getRLineSet()->get(mRLIndex[index]);
	if(rLine==NULL)
		return NULL;
	GRPoint *rPoint = rLine->get(mRPIndex[index]);
	if(rPoint==NULL)
		return NULL;
   // return GProject::getCurrentProject()->getRLineSet()->get(mRLIndex[index])->get(mRPIndex[index]);
}
//add by LiuWei 20230117
GRPoint * GShot::getRPointByTrcIndex(int index, GRLineSet *pRLineSet)
{

	GShotXLine *pXLine = getXLineByTrcIndex(index);
	if(pXLine == NULL) return NULL;

	GShotXArray *pXArray = NULL;
	for(int i=0; i<pXLine->count(); i++)
	{
		if(pXLine->get(i)->getMinTrcIndex() <= index && index <= pXLine->get(i)->getMaxTrcIndex())
		{
			pXArray = pXLine->get(i);
			break;
		}
	}
	if(pXArray == NULL) return NULL;

	if(pRLineSet == NULL) return NULL;
	GRLine *pRLine = pRLineSet->get(pXLine->getLineIndex());
	if(pRLine == NULL) return NULL;
	return pRLine->get(pXArray->getFirstPtIndex()+index-pXArray->getMinTrcIndex());
}

// GRPoint * GShot::getRPointByTrcIndex(int index)
// {
// 	 	GProject *pProject = mpProject;
// 	 	GShotXLine *pXLine = getXLineByTrcIndex(index);
// 	 	if(pXLine == NULL) return NULL;
// 	 
// 	 	GShotXArray *pXArray = NULL;
// 	 	for(int i=0; i<pXLine->count(); i++)
// 	 	{
// 	 		if(pXLine->get(i)->getMinTrcIndex() <= index && index <= pXLine->get(i)->getMaxTrcIndex())
// 	 		{
// 	 			pXArray = pXLine->get(i);
// 	 			break;
// 	 		}
// 	 	}
// 	 	if(pXArray == NULL) return NULL;
// 	 
// 	 	GRLineSet *pRLineSet = pProject->getRLineSet();
// 	 	if(pRLineSet == NULL) return NULL;
// 	 	GRLine *pRLine = pRLineSet->get(pXLine->getLineIndex());
// 	 	if(pRLine == NULL) return NULL;
// 	 	return pRLine->get(pXArray->getFirstPtIndex()+index-pXArray->getMinTrcIndex());
// 
// }

int GShot::getTrcIndexByRPoint(GRPoint * rPoint)
{
  for(int i=0; i<getTrcCount(); i++)
  {
	  GRPoint *tempRPoint = getRPointByTrcIndex(i);
	  if(tempRPoint->getLineName()==rPoint->getLineName())
		  if(tempRPoint->getPointNum()==rPoint->getPointNum())
			  return i;
  }
  return  -1;
}


Q_INT32 * GShot::getPriTimePtr(int index)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = get(i);
		if(pXLine->getMinIndex() <= index && index <= pXLine->getMaxIndex())
		{
			int curIndex = pXLine->getMinIndex();
			for(int j=0; j<pXLine->count(); j++)
			{
				GShotXArray *pXArray = pXLine->get(j);
				if(curIndex <= index && index < curIndex+pXArray->getTrcCount())
				{
					return pXArray->getPriTimeArray()+index-curIndex;
				}
				curIndex += pXArray->getTrcCount();
			}
		}
	}

	return NULL;
}

Q_INT32 * GShot::getPriTimePtrSV(int index)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = get(i);
		if(pXLine->getMinIndex() <= index && index <= pXLine->getMaxIndex())
		{
			int curIndex = pXLine->getMinIndex();
			for(int j=0; j<pXLine->count(); j++)
			{
				GShotXArray *pXArray = pXLine->get(j);
				if(curIndex <= index && index < curIndex+pXArray->getTrcCount())
				{
					return pXArray->getPriTimeArraySV()+index-curIndex;
				}
				curIndex += pXArray->getTrcCount();
			}
		}
	}

	return NULL;
}

float *GShot::getFitDiference(int index)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = get(i);
		if(pXLine->getMinIndex() <= index && index <= pXLine->getMaxIndex())
		{
			int curIndex = pXLine->getMinIndex();
			for(int j=0; j<pXLine->count(); j++)
			{
				GShotXArray *pXArray = pXLine->get(j);
				if(curIndex <= index && index < curIndex+pXArray->getTrcCount())
				{
					return pXArray->getFitDifference()+index-curIndex;
				}
				curIndex += pXArray->getTrcCount();
			}
		}
	}

	return NULL;
}

void GShot::save(QDataStream &stream)
{
	stream << mPosState;
	stream << mIndex << mRecNum;
	stream << mLineName << mPointNum;
	stream << mLineIndex << mPointIndex;
	stream << mFileIndex << mFilePos << mDataPos << mTBPos << mTAOPos;
	stream << mCPCount << mCPInv;
	stream << mTrcCount;
	stream << (Q_INT32)mEvaluateState;

	stream << (Q_INT32)count();
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = mpXLineList->at(i);
		pXLine->save(stream);
	}

	mState = siSaved;
	///////////////
		///////////////
	for(int i=0; i<mTrcCount; i++)
		stream << mQCFBT[i];
	//////////////////add by psl 110119
	for(int i=0; i<mTrcCount; i++)
	{
		stream<<mRLIndex[i]<<mRPIndex[i];
	}
	///////////end
/*	for (i=0; i<mTrcCount;i++)
	{
		stream << mTrcNum[i];
	}
	*/
}

void GShot::save(QDataStream &stream, QDataStream &tstream,QDataStream &tstreamSV)
{
	stream << mPosState;
	stream << mIndex << mRecNum;
	stream << mLineName << mPointNum;
	stream << mLineIndex << mPointIndex;
	stream << mFileIndex << mFilePos << mDataPos << mTBPos << mTAOPos;
	stream << mCPCount << mCPInv;
	stream << mTrcCount;
	stream << (Q_INT32)mEvaluateState;

	stream << (Q_INT32)mpXLineList->count();
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = mpXLineList->at(i);
		pXLine->save(stream, tstream,tstreamSV);
	}

	mState = siSaved;
	///////////////
	for(int i=0; i<mTrcCount; i++)
		stream << mQCFBT[i];
	//////////////////add by psl 110119
	for(int i=0; i<mTrcCount; i++)
	{
		stream<<mRLIndex[i]<<mRPIndex[i];
	}
	///////////end
/*	for (i=0; i<mTrcCount; i++)
	{
		stream<<mTrcNum[i];
	}
*/
}

void GShot::savePriTime(QDataStream &tstream)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = mpXLineList->at(i);
		pXLine->savePriTime(tstream);
	}
}

void GShot::savePriTimeSV(QDataStream &tstream)
{
	for(int i=0; i<count(); i++)
	{
		GShotXLine *pXLine = mpXLineList->at(i);
		pXLine->savePriTimeSV(tstream);
	}
}


void GShot::load(QDataStream &stream, QDataStream &tstream, QDataStream &tstreamSV)
{

	Q_INT32 count, data;

	stream >> mPosState;
	stream >> mIndex >> mRecNum;
	stream >> mLineName >> mPointNum;
	stream >> mLineIndex >> mPointIndex;
	//mTAO = 0;
	stream >> mFileIndex >> mFilePos >> mDataPos >> mTBPos >> mTAOPos;
	stream >> mCPCount >> mCPInv;
	stream >> mTrcCount;
	stream >> data;
	mEvaluateState = (GSEvaluateState)data;

	//mTBPos = mDataPos;		//for dubug;
	stream >> count;
	for(int i=0; i<count; i++)
	{
		GShotXLine *pXLine = new GShotXLine;
		pXLine->load(stream, tstream,tstreamSV);
		mpXLineList->append(pXLine);
	}

	mState = siSaved;
	////////
	if(mQCFBT!=NULL)
	delete []mQCFBT;
	mQCFBT = new float[getTrcCount()];
	if(mTrcNum!=NULL)
		delete []mTrcNum;
	mTrcNum = new int[getTrcCount()];
		///////////////
	for(int i=0; i<mTrcCount; i++)
		stream >> mQCFBT[i];
	//////////////////add by psl 110119
	//////////////////add for bug 110119
    if (mRLIndex!=NULL)
 		delete []mRLIndex;
 	mRLIndex = new int[mTrcCount];
 	if (mRPIndex!=NULL)
 		delete []mRPIndex;
 	mRPIndex = new int[mTrcCount];
 	for(int i=0; i<mTrcCount; i++)
 	{
 		stream>>mRLIndex[i]>>mRPIndex[i];
 	}
	///////////end
   /*  for (i=0; i<mTrcCount; i++)
     {
		 stream>>mTrcNum[i];
     }
	*/
}


bool GShot::load(QDataStream &stream, GSLineSet *pSLineSet)//add by zhaohu
{
	Q_INT32 i, count;

	stream >> mIndex >> mRecNum;
	stream >> mLineName >> mPointNum;
	stream >> mLineIndex >> mPointIndex;
	stream >> mFileIndex >> mFilePos;
	stream >> mTrcCount;
	stream >> count;
	gSkipPadBytes(stream, GSHOT_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) return false;

	for(i=0; i<count; i++)
	{
		GShotXLine *pXLine = new GShotXLine();
		if(!pXLine->load(stream))
		{
			mpXLineList->clear();
			return false;
		}
		mpXLineList->append(pXLine);
	}

	GSLine *pSLine = pSLineSet->get(mLineIndex);
	mpSPoint = pSLine->get(mPointIndex);

	return true;
}

float GShot::getQCFBTByIndex(int index)
{ 
	if(mQCFBT==NULL)
		return NULL;
	if(index>-1&&index<getTrcCount())
	return mQCFBT[index];
	else
		return -2;
}

int GShot::getTrcNumByIndex(int index)
{
	if(mTrcNum==NULL)
		return NULL;
	if(index>-1&&index<getTrcCount())
		return mTrcNum[index];
	else
		return -1;
}

void GShot::setTrcNumByIndex(int index,int trcNum)
{
	 if(index>-1&&index<getTrcCount())
	mTrcNum[index] = trcNum;
}


void GShot::setQCFBTByIndex(int index,float data)
{

   if(index>-1&&index<getTrcCount())
	mQCFBT[index] = data;

}

void GShot::setCPCount(int cpCount)
{ 
	mCPCount = cpCount;
	if(mQCFBT!=NULL)
		delete []mQCFBT;
	mQCFBT = new float[getTrcCount()];
	for(int i=0; i<getTrcCount(); i++)
		mQCFBT[i] = -1;
}
//----class GShotLine-----------------------------------------------------------
GShotLine::GShotLine()
{
	mpShotList = new GPtrVector<GShot>(128);
}

GShotLine::~GShotLine()
{
	delete mpShotList;
}

int GShotLine::getIndexByRecNum(int recNum)
{
	for(int i=0; i<count(); i++)
	{
		GShot *pShot = mpShotList->at(i);
		if(pShot->getRecNum() == recNum) return i;
	}
	return -1;
}

GShot * GShotLine::getByRecNum(int recNum)
{
	return get(getIndexByRecNum(recNum));
}

void GShotLine::sortByX()
{
	mpShotList->heapsort(gCompareShotByX);
}

void GShotLine::sortByY()
{
	mpShotList->heapsort(gCompareShotByY);
}
//----class GShotSet------------------------------------------------------------
GShotSet::GShotSet()
{
	mpShotList = new GPtrVector<GShot>(512);
	mpShotList->setAutoDelete(true);
}

GShotSet::~GShotSet()
{
	delete mpShotList;
}

void GShotSet::clear()
{
	mpShotList->clear();
}

GShot * GShotSet::get(int index)
{
	return mpShotList->at(index);
}

int GShotSet::getIndexByRecNum(int recNum)
{
	for(uint i=0; i<mpShotList->count(); i++)
	{
		if(recNum == mpShotList->at(i)->getRecNum())
		{
			return i;
		}
	}
	return -1;
}

GShot * GShotSet::getByRecNum(int recNum)
{
	int index = getIndexByRecNum(recNum);
	if(index >= 0) return mpShotList->at(index);
	else return NULL;
}

GShot * GShotSet::getByLinePoint(const QString &lineName, const QString &pointNum)  //2026.3.31 by Wushanzhou
{
	for(int i = 0; i < count(); i++)
	{
		GShot *pShot = mpShotList->at(i);
		if(pShot != NULL &&
		   pShot->getLineName() == lineName &&
		   pShot->getPointNum() == pointNum)
		{
			return pShot;
		}
	}
	return NULL;
}

void GShotSet::checkSInfo(QTextEdit *spsMemo)
{
//	GProject *pProject = mpProject;
	GProject *pProject = GProject::getCurrentProject();
	
	if(pProject == NULL) return;

	int i, errCount = 0;
	GShot *pShot;
	GSLineSet *pSLineSet = pProject->getSLineSet();
	GSLine *pSLine;
	GSPoint *pSPoint;

	if(spsMemo != NULL)
		spsMemo->append( QString::fromLocal8Bit("\n//----ڵϢ----//") );

	pSLineSet->clearShotList();	//ڵڹϵ
	for(i=0; i<count(); )
	{
		pShot = mpShotList->at(i);

		//¼ĵϢտɿϢΪԺڵҪ͸
		pShot->mLineIndex = pSLineSet->getIndexByLineName(pShot->getLineName());
		if(pShot->mLineIndex < 0) goto ERROR_SINFO;
		pSLine = pSLineSet->get(pShot->mLineIndex);
		pShot->mPointIndex = pSLine->getIndexByPointNum(pShot->getPointNum());
		if(pShot->mPointIndex < 0) goto ERROR_SINFO;
		pSPoint = pSLine->get(pShot->mPointIndex);

		//ﱾӦдڹϵڹϵԺܱ仯Ϊ˼ڵʱ¼
		pSPoint->append(i);		

		i++;
		continue;	//ûдѭ
		
		ERROR_SINFO:
		{
			if(spsMemo != NULL)
			{
				spsMemo->append( QString::fromLocal8Bit("  : ûڵҰ¼("+
								 QString::number(pShot->mRecNum)+")Ӧ") );
				spsMemo->append( QString::fromLocal8Bit("  ɾҰ¼("+
								 QString::number(pShot->mRecNum)+")ӦϢ") );
			}
			mpShotList->remove( i );
			errCount++;
		}
	}

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("//----ڵϢ ("+
						  QString::number(errCount)+")----//") );
}

void GShotSet::checkRInfo(QTextEdit *spsMemo,QProgressBar *progressBar)
{
	/////////
		//	if(progressBar != NULL) progressBar->setProgress( srcOffset*100/srcLen );
//	progressBar->setTotalSteps(100);
	progressBar->setValue(100);
	/////////////
	GProject *pProject = GProject::getCurrentProject();
	if(pProject == NULL) return;

	int i, j, k, errCount = 0;
	GRLineSet *pRLineSet = pProject->getRLineSet();
	GRLine *pRLine;

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("\n//----첨Ϣ----//") );

	pRLineSet->clearShotList();	//м첨ڹϵ

	for(i=0; i<count(); )
	{
//		if(progressBar != NULL) progressBar->setProgress( i*100/count() );
		if(progressBar != NULL) progressBar->setValue( i*100/count() );
		GShot *pShot = mpShotList->at(i);

		GSPoint *pSPoint = pShot->getSPoint();// ҽ޸ģ [Liu Wei 6/19/2023]

		pShot->mTrcCount = 0;
		for(j=0; j<pShot->count(); j++)
		{
			GShotXLine *pXLine = pShot->get(j);
			pXLine->mLineIndex = pRLineSet->getIndexByLineName(pXLine->getLineName());
			if(pXLine->mLineIndex < 0) goto ERROR_RINFO; //ȱټ첨
			pRLine = pRLineSet->get(pXLine->mLineIndex);
			
			pXLine->mTrcCount = 0;
			pXLine->mMinIndex = pShot->mTrcCount;
			for(k=0; k<pXLine->count(); k++)
			{
				GShotXArray *pXArray = pXLine->get(k);
				pXArray->mMinTrcIndex = pShot->mTrcCount;
				//첨ڼ첨бǷ
				pXArray->mFirstPtIndex = pRLine->getIndexByPointNum(pXArray->getFirstPt());
				pXArray->mLastPtIndex = pRLine->getIndexByPointNum(pXArray->getLastPt());
				if(pXArray->mFirstPtIndex < 0 || pXArray->mLastPtIndex < 0) goto ERROR_RINFO;

				/*if(!pXArray->isSingle(pSPoint->getX(), pRLine))
				{
					pXLine->append(pXArray->splitXArray(pSPoint->getX(), pRLine));
				}
				else 
				*/
				pXArray->setArrayType(pSPoint->getX(), pRLine);
				//첨ڼ첨бǷ
				//pXArray->mFirstPtIndex = pRLine->getIndexByPointNum(pXArray->getFirstPt());
				//pXArray->mLastPtIndex = pRLine->getIndexByPointNum(pXArray->getLastPt());
// 				if(abs(pXArray->mLastPtIndex - pXArray->mFirstPtIndex)+1 
// 				   != pXArray->getTrcCount()) goto ERROR_RINFO;
				pShot->mTrcCount += pXArray->mTrcCount;
				pXLine->mTrcCount += pXArray->mTrcCount;
				pXArray->mMaxTrcIndex = pShot->mTrcCount-1;
				if(pXLine->mMaxTrc < pXArray->mLastTrc) pXLine->mMaxTrc = pXArray->mLastTrc;
				if(pXLine->mMinTrc > pXArray->mFirstTrc) pXLine->mMinTrc = pXArray->mLastTrc;
			}
			pXLine->mMaxIndex = pShot->mTrcCount-1;
		}
		i++;
		continue;	//ûдѭ

		ERROR_RINFO:
		{
			if( spsMemo != NULL )
			{
				spsMemo->append( QString::fromLocal8Bit("  : ûм첨ϢҰ¼("+
								 QString::number(pShot->mRecNum)+")Ӧ") );
				spsMemo->append( QString::fromLocal8Bit("  ɾҰ¼("+
								 QString::number(pShot->mRecNum)+")ӦϢ") );
			}
			mpShotList->remove( i );
			errCount++;
		}
	}
	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("//----첨Ϣ ("+
						 QString::number(errCount)+")----//") );
//	progressBar->reset();
	progressBar->setValue(100);
}

void GShotSet::checkSEGYInfo(QTextEdit *sgyMemo)
{
	int i, errCount = 0;
	GShot *pShot;

	if(sgyMemo != NULL)
		sgyMemo->append( QString::fromLocal8Bit("\n//----SEG-YļϢ----//") );

	for(i=0; i<count(); )
	{
		pShot = mpShotList->at(i);
		if(pShot->mFilePos == 0)
		{
			if(sgyMemo != NULL)
			{
				sgyMemo->append( QString::fromLocal8Bit("  : ûSEG-YҰ¼("+
								 QString::number(pShot->mRecNum)+")Ӧ") );
				sgyMemo->append( QString::fromLocal8Bit("  ɾҰ¼("+
								 QString::number(pShot->mRecNum)+")ӦϢ") );
			}
			mpShotList->remove(i);//modify by psl in 070316
			errCount++;
		}
		else i++;
	}
	if( sgyMemo != NULL )
		sgyMemo->append( QString::fromLocal8Bit("//----SEG-YļϢ ("+
						 QString::number(errCount)+")----//") );
}

void GShotSet::sort()
{
	mpShotList->heapsort(gCompareShotByStackNum); 
	//mpShotList->sort(gCompareShotByStackNum); 
	for(int i=0; i<count(); i++)
	{
		get(i)->mIndex = i;		//д
	}
}

void GShotSet::sortByLineAndPoint()
{
    mpShotList->heapsort(gCompareShotByLineAndPoint); 
	for(int i=0; i<count(); i++)
	{
		get(i)->mIndex = i;		//д
	}
}

void GShotSet::sortByPointAndLine()
{
    mpShotList->heapsort(gCompareShotByPointAndLine); 
	for(int i=0; i<count(); i++)
	{
		get(i)->mIndex = i;		//д
	}
}

void GShotSet::sortByRec()
{
    mpShotList->heapsort(gCompareFieldRecordNumber); 
	for(int i=0; i<count(); i++)
	{
		get(i)->mIndex = i;		//д
	}
}

void GShotSet::extractInline(float y, float diffy, GShotLine *pSLine)
{
	pSLine->clear();
	for(int i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		if(fabs(y - pShot->getSPoint()->getY()) <= diffy)
		{
			pSLine->append(pShot);
		}
	}
	pSLine->sortByX();
}

void GShotSet::extractInline2D(GShotLine *pSLine)
{
	pSLine->clear();
	for(int i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		pSLine->append(pShot);
	}
	pSLine->sortByX();
}

void GShotSet::extractCrossline(float x, float diffx, GShotLine *pSLine)
{
	pSLine->clear();
	for(int i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		if(fabs(x - pShot->getSPoint()->getX()) <= diffx)
		{
			pSLine->append(pShot);
		}
	}
	pSLine->sortByY();
}

void GShotSet::extractCRP(GRLineSet *pRLineSet)
{
	int i, j, k;
	GRLine *pRLine;
	GRPoint *pRPoint;
	//CRP
	pRLineSet->clearShotList();

	for(i=0; i<count(); i++)
	{
		GShot *pShot = mpShotList->at(i);
		for(j=0; j<pShot->count(); j++)
		{
			GShotXLine *pXLine = pShot->get(j);
			pRLine = pRLineSet->getByLineName( pXLine->getLineName() );
			for(k=0; k<pXLine->count(); k++)
			{
				GShotXArray *pXArray = pXLine->get(k);
				Q_INT32 first = pXArray->mFirstPtIndex;
				Q_INT32 last = pXArray->mLastPtIndex;
				if(first > last) gSwap( first, last );
				for(int p=first; p<=last; p++)
				{
					pRPoint = pRLine->get(p);
					pRPoint->mpShotList->append(i);
				}
			}
		}
	}
}

void GShotSet::extractCSP(GSLineSet *pSLineSet)
{
	//ԭĹڼ¼
	pSLineSet->clearShotList();
	//ɨȫڹϵ빲ϢΪڹϵ¼λ
	for(int i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		pShot->mLineIndex = pSLineSet->getIndexByLineName(pShot->getLineName());
		GSLine *pSLine = pSLineSet->get(pShot->mLineIndex);
		pShot->mPointIndex = pSLine->getIndexByPointNum(pShot->getPointNum());
		GSPoint *pSPoint = pSLine->get(pShot->mPointIndex);
		pSPoint->append(i);
	}
}


bool GShotSet::save(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(IO_WriteOnly)) return false;

	QDataStream stream(&file);
	stream << (Q_INT32)mpShotList->count();

	GShot *pShot;
	for(int i=0; i<count(); i++)
	{
		pShot = mpShotList->at(i);
		pShot->save(stream);
	}

	file.close();
	return true;
}

bool GShotSet::save(const QString &fileName, const QString &tfileName, 
					const QString &tfileNameSV)
{
	QFile file(fileName);
	if(!file.open(IO_WriteOnly)) return false;

	QFile tfile(tfileName);
	if(!tfile.open(IO_WriteOnly)) return false;

	QFile tfileSV(tfileNameSV);
	if(!tfileSV.open(IO_WriteOnly)) return false;



	QDataStream stream(&file);
	QDataStream tstream(&tfile);
	QDataStream tstreamSV(&tfileSV);
	stream << (Q_INT32)mpShotList->count();

	GShot *pShot;
	for(int i=0; i<count(); i++)
	{
		pShot = mpShotList->at(i);
		pShot->save(stream, tstream,tstreamSV);
	}

	file.close();
	tfile.close();
	tfileSV.close();
	return true;
}

bool GShotSet::savePriTime(const QString &tfileName)
{
	QFile tfile(tfileName);
	if(!tfile.open(IO_WriteOnly)) return false;
	QDataStream tstream(&tfile);

	GShot *pShot;
	for(int i=0; i<count(); i++ )
	{
		pShot = mpShotList->at(i);
		pShot->savePriTime(tstream);
	}

	tfile.close();
	return true;
}

bool GShotSet::savePriTimeSV(const QString &tfileName)
{
	QFile tfile(tfileName);
	if(!tfile.open(IO_WriteOnly)) return false;
	QDataStream tstream(&tfile);

	GShot *pShot;
	for(int i=0; i<count(); i++ )
	{
		pShot = mpShotList->at(i);
		pShot->savePriTimeSV(tstream);
	}

	tfile.close();
	return true;
}

bool GShotSet::load(const QString &fileName, const QString &tfileName,
					const QString &tfileNameSV)
{
	QFile file(fileName);
	if(!file.open(IO_ReadOnly)) return false;
	QFile tfile(tfileName);
	if(!tfile.open(IO_ReadOnly)) return false;

	QFile tfileSV(tfileNameSV);
	if(!tfileSV.open(IO_ReadOnly)) return false;
	QDataStream stream(&file);
	QDataStream tstream(&tfile);
	QDataStream tstreamSV(&tfileSV);
	Q_INT32 count;
	stream >> count;

	clear();
	for(int i=0; i<count; i++)
	{
		GShot *pShot = new GShot;
		pShot->setProject(mpProject);
		pShot->load(stream, tstream, tstreamSV);
		mpShotList->append(pShot);
	}

	file.close();
	tfile.close();
	tfileSV.close();
	return true;
}


////add by psl 110119
void GShotSet::calculateRIndex()
{
	QFile *file = new QFile;
	QDataStream *stream = new QDataStream;
	file  = new QFile( GProject::getCurrentProject()->getSEGYFileName(0) );
	if( !file->open( IO_ReadOnly ) ) return ;
	stream = new QDataStream ( file );
	stream->setByteOrder( QDataStream::BigEndian );


	QProgressDialog progress( QString::fromLocal8Bit("..."), "ȡ",  count(),0);
	progress.setValue(0);

	for(int i=0; i<count(); i++)
	{
	  GShot *pShot = get(i);
//       //for bug úɾ
// 	  if(pShot->getRecNum()!=28)
// 		  continue;
// 	  //
	  pShot->mTrcCount = pShot->setRLineAndPointIndex(stream);
	  progress.setValue(i);
	  if(progress.wasCanceled())
		break;

	}
	file->close();
	delete file;
	delete stream;
}



void GShotSet::create(GSPSParam *pSPSParam, GSLineSet *pSLineSet, GRLineSet *pRLineSet, GShotSet *pShotsetOld)
{
	int i, j, ii, jj, recNum, trc, first, last, tn;
	float offset, minLOffset, maxLOffset, minROffset, maxROffset;
	GShot *pShot;
	GSPoint *pSPoint;
	GRPoint *pRPoint;
	GSLine *pSLine;
	GRLine *pRLine;

	clear();

	minLOffset = pSPSParam->mMinLOffset;
	maxLOffset = pSPSParam->mMaxLOffset;
	minROffset = pSPSParam->mMinROffset;
	maxROffset = pSPSParam->mMaxROffset;

	recNum = 0;
	for(i=0; i<pSLineSet->count(); i++)
	{
		pSLine = pSLineSet->get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			recNum++;
			pSPoint = pSLine->get(j);
			pShot = new GShot(recNum, pSLine->getLineName(), pSPoint->getPointNum());

			int recNumZH = pShotsetOld->getRecNumByLineNamePointName(pSLine->getLineName(),pSPoint->getPointNum());//add by zhaohu ,ߺڵŵõڵҰ¼
			pShot->mRecNumZH=recNumZH;//add by zhaohu

			pShot->mIndex = recNum-1;//ffid
			pShot->setSPoint(pSPoint);
			pShot->mLineIndex = i;
			pShot->mPointIndex = j;
			pShot->mTrcCount = 0;
			append(pShot);
			pSPoint->append(j);

			trc = 0;
			for(ii=0; ii<pRLineSet->count(); ii++)
			{
				pRLine = pRLineSet->get(ii);
				GShotXLine *pXLine = new GShotXLine(pRLine->getLineName());
				pXLine->mLineIndex = ii;
				pXLine->mLineName = pRLine->getLineName();
				pXLine->mMinTrc = pXLine->mMaxTrc = trc+1;
				pXLine->mMinIndex = pXLine->mMaxIndex = trc;
				pXLine->mTrcCount = 0;
				pShot->append(pXLine);

				GShotXArray *pXArray;
				// Ѱ֧
				for(jj=0; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pSPoint->getX() - pRPoint->getX();
					if(offset <= maxLOffset ) break;
					//					if(fabs(offset-maxLOffset)<0.5*pSPSParam->mRPtInv) break;
				}
				first = jj;
				//  Ѱ֧յ
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pSPoint->getX() - pRPoint->getX();
					if(offset < minLOffset) 
						//					if(fabs(offset-minLOffset)<0.5*pSPSParam->mRPtInv)
					{
						break ;
					}
				}
				last = jj-1;
				//				last = jj;
				if(first <= last)
				{
					tn = last-first+1;
					pXArray = new GShotXArray(pRLine->get(first)->getPointNum(),
						pRLine->get(last)->getPointNum(), trc+1, trc+tn, 1, tn);
					pXArray->mArrayType = GShotXArray::gLeftXArray;
					pXArray->mFirstPtIndex = first;
					pXArray->mLastPtIndex = last;
					pXArray->mMinTrcIndex = trc;
					pXArray->mMaxTrcIndex = trc+tn-1;
					pXLine->append(pXArray);
					pXLine->mMaxIndex += pXArray->mMaxTrcIndex;
					pXLine->mTrcCount += tn;
					trc += tn;
				}
				// Ѱ֧
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pRPoint->getX()-pSPoint->getX();
					if(offset >= minROffset) break;
					//					if(fabs(offset-minROffset)<0.5*pSPSParam->mRPtInv) break;
				}
				first = jj;
				// Ѱ֧յ
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pRPoint->getX()-pSPoint->getX();
					if(offset > maxROffset ) 
						//					if(fabs(offset-maxROffset)<0.5*pSPSParam->mRPtInv)
					{
						break;
					}
				}
				last = jj-1;
				//				last = jj;
				if(first <= last)
				{
					tn = last-first+1;
					pXArray = new GShotXArray(pRLine->get(first)->getPointNum(),
						pRLine->get(last)->getPointNum(), trc+1, trc+tn, 1, tn);
					pXArray->mArrayType = GShotXArray::gRightXArray;
					pXArray->mFirstPtIndex = first;
					pXArray->mLastPtIndex = last;
					pXArray->mMinTrcIndex = trc;
					pXArray->mMaxTrcIndex = trc+tn-1;
					pXLine->append(pXArray);
					pXLine->mMaxIndex = pXArray->mMaxTrcIndex;
					pXLine->mTrcCount += tn;
					trc += tn;
				}
				pShot->mTrcCount += pXLine->mTrcCount;
			}
		}
	}
}

void GShotSet::create( GSPSParam *pSPSParam, GSLineSet *pSLineSet, GRLineSet *pRLineSet )
{
	int i, j, ii, jj, recNum, trc, first, last, tn;
	float offset, minLOffset, maxLOffset, minROffset, maxROffset;
	GShot *pShot;
	GSPoint *pSPoint;
	GRPoint *pRPoint;
	GSLine *pSLine;
	GRLine *pRLine;

	clear();

	minLOffset = pSPSParam->mMinLOffset;
	maxLOffset = pSPSParam->mMaxLOffset;
	minROffset = pSPSParam->mMinROffset;
	maxROffset = pSPSParam->mMaxROffset;

	recNum = 0;
	for(i=0; i<pSLineSet->count(); i++)
	{
		pSLine = pSLineSet->get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			recNum++;
			pSPoint = pSLine->get(j);
			pShot = new GShot(recNum, pSLine->getLineName(), pSPoint->getPointNum());

			pShot->mIndex = recNum-1;//ffid
			pShot->setSPoint(pSPoint);
			pShot->mLineIndex = i;
			pShot->mPointIndex = j;
			pShot->mTrcCount = 0;
			append(pShot);
			pSPoint->append(j);

			trc = 0;
			for(ii=0; ii<pRLineSet->count(); ii++)
			{
				pRLine = pRLineSet->get(ii);
				GShotXLine *pXLine = new GShotXLine(pRLine->getLineName());
				pXLine->mLineIndex = ii;
				pXLine->mLineName = pRLine->getLineName();
				pXLine->mMinTrc = pXLine->mMaxTrc = trc+1;
				pXLine->mMinIndex = pXLine->mMaxIndex = trc;
				pXLine->mTrcCount = 0;
				pShot->append(pXLine);

				GShotXArray *pXArray;
				// Ѱ֧
				for(jj=0; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pSPoint->getX() - pRPoint->getX();
					if(offset <= maxLOffset ) break;
					//if(fabs(offset-maxLOffset)<0.5*pSPSParam->mRPtInv) break;
				}
				first = jj;
				//  Ѱ֧յ
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pSPoint->getX() - pRPoint->getX();
					//if(offset < minLOffset)// ԭʼж [Liu Wei 6/21/2023] 
					if(offset<0)// Ѱҵ˵һ֧첨 [Liu Wei 6/21/2023]
					{
						break ;
					}
				}
				last = jj-1;
				//last = jj;
				if(first <= last)
				{
					tn = last-first+1;//ÿһڵĽյ
					pXArray = new GShotXArray(pRLine->get(first)->getPointNum(),
						pRLine->get(last)->getPointNum(), trc+1, trc+tn, 1, tn);
					pXArray->mArrayType = GShotXArray::gLeftXArray;
					pXArray->mFirstPtIndex = first;
					pXArray->mLastPtIndex = last;
					pXArray->mMinTrcIndex = trc;
					pXArray->mMaxTrcIndex = trc+tn-1;
					pXLine->append(pXArray);
					pXLine->mMaxIndex += pXArray->mMaxTrcIndex;
					pXLine->mTrcCount += tn;
					trc += tn;
				}
				// Ѱ֧
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pRPoint->getX()-pSPoint->getX();
					//if(offset >= minROffset) break;// ԭʼж [Liu Wei 6/21/2023]
					if(offset>0) break;// ©һ첨 [Liu Wei 6/21/2023]
				}
				first = jj;
				// Ѱ֧յ
				for(; jj<pRLine->count(); jj++)
				{
					pRPoint = pRLine->get(jj);
					offset = pRPoint->getX()-pSPoint->getX();
					if(offset > maxROffset ) 
					//if(fabs(offset-maxROffset)<0.5*pSPSParam->mRPtInv)
					{
						break;
					}
				}
				last = jj-1;
				//last = jj;
				if(first <= last)
				{
					tn = last-first+1;
					pXArray = new GShotXArray(pRLine->get(first)->getPointNum(),
						pRLine->get(last)->getPointNum(), trc+1, trc+tn, 1, tn);
					pXArray->mArrayType = GShotXArray::gRightXArray;
					pXArray->mFirstPtIndex = first;
					pXArray->mLastPtIndex = last;
					pXArray->mMinTrcIndex = trc;
					pXArray->mMaxTrcIndex = trc+tn-1;
					pXLine->append(pXArray);
					pXLine->mMaxIndex = pXArray->mMaxTrcIndex;
					pXLine->mTrcCount += tn;
					trc += tn;
				}
				pShot->mTrcCount += pXLine->mTrcCount;
			}
		}
	}
}

bool GShotSet::save(QDataStream &stream)
{
	stream << (Q_INT32)mpShotList->count();
	gSkipPadBytes(stream, GSHOT_SET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) 
	{
		return false;
	}

	GShot *pShot;
	for(int i=0; i<count(); i++)
	{
		pShot = mpShotList->at(i);
		pShot->save(stream);
	}
	return true;
}


bool GShotSet::load(QDataStream &stream, GSLineSet *pSLineSet)
{
	Q_INT32 count;
	stream >> count;
	gSkipPadBytes(stream, GSHOT_SET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) 
	{
		return false;
	}

	clear();
	for(int i=0; i<count; i++)
	{
		GShot *pShot = new GShot();
		pShot->load(stream, pSLineSet);
		mpShotList->append(pShot);
	}

	return true;
}

int GShotSet::getRecNumByLineNamePointName(QString LineName,QString PointName)//ߺźڵ÷ffidadd by zhaohu
{
	for(int i=0;i<mpShotList->count();i++)
	{
		GShot *shot=mpShotList->at(i);
		if(shot->getLineName()==LineName && shot->getPointNum()==PointName)
		{
			return shot->getRecNum();
		}
	}
}

//sps2.1汾
bool GShotSet::saveXFile2(const QString &fileName, GSLineSet *pSLineSet, GRLineSet *pRLineSet) // add by shanzhou in 2026/10/15
{
	//Ϊ1.0ʽ
/*	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QTextStream stream(&file);
	stream.setEncoding(QTextStream::Locale);
	QString line;

	int i, j, k;
	for(i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		if (!pShot || !pSLineSet) continue;
		if (pShot->mLineIndex < 0 || pShot->mLineIndex >= pSLineSet->count()) continue;
		GSLine *pSLine = pSLineSet->get(pShot->mLineIndex);
		if (!pSLine || pShot->mPointIndex < 0 || pShot->mPointIndex >= pSLine->count()) continue;
		GSPoint *pSPoint = pSLine->get(pShot->mPointIndex);
		if (!pSPoint) continue;

		for(j=0; j<pShot->count(); j++)
		{
			GShotXLine *pXLine = pShot->get(j);
			if (!pXLine || !pRLineSet) continue;
			if (pXLine->getLineIndex() < 0 || pXLine->getLineIndex() >= pRLineSet->count()) continue;
			GRLine *pRLine = pRLineSet->get(pXLine->getLineIndex());
			if (!pRLine) continue;

			for(k=0; k<pXLine->count(); k++)
			{
				GShotXArray *pXArray = pXLine->get(k);
				if (!pXArray) continue;
				if (pXArray->getFirstPtIndex() < 0 || pXArray->getFirstPtIndex() >= pRLine->count()) continue;
				if (pXArray->getLastPtIndex() < 0 || pXArray->getLastPtIndex() >= pRLine->count()) continue;
				GRPoint *pFirstPt = pRLine->get(pXArray->getFirstPtIndex());
				GRPoint *pLastPt = pRLine->get(pXArray->getLastPtIndex());
				if (!pFirstPt || !pLastPt) continue;

				line.sprintf("X%6s%8d1%1s%10.2f%10.2f1%5d%5d1%10.2f%10.2f%10.2f1",
					"      ", pShot->getRecNum(), " ", 
					pSPoint->getLineName().toFloat(), pSPoint->getPointNum().toFloat(),
					pXArray->getFirstTrc(), pXArray->getLastTrc(), 
					pRLine->getLineName().toFloat(),
					pFirstPt->getPointNum().toFloat(),
					pLastPt->getPointNum().toFloat());
				stream << line << "\r\n";
			}
		}
	}
	file.close();
	return true;
*/

	//Ϊ2.1ʽ
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QTextStream stream(&file);
	stream.setEncoding(QTextStream::Locale);
	QString line;

	int i, j, k;
	for(i=0; i<count(); i++)
	{
		GShot *pShot = get(i);
		GSPoint *pSPoint = pSLineSet->get(pShot->mLineIndex)->get(pShot->mPointIndex);
//		GSPoint *pSPoint = pShot->getSPoint();
		for(j=0; j<pShot->count(); j++)
		{
			GShotXLine *pXLine = pShot->get(j);
			GRLine *pRLine = pRLineSet->get(pXLine->getLineIndex());
			for(k=0; k<pXLine->count(); k++)
			{
				GShotXArray *pXArray = pXLine->get(k);
				line.sprintf("X%6s%8d1%1s%10.2f%10.2f1%5d%5d1%10.2f%10.2f%10.2f1",
					"      ", pShot->getRecNum(), " ", 
					pSPoint->getLineName().toFloat(), pSPoint->getPointNum().toFloat(),
					pXArray->getFirstTrc(), pXArray->getLastTrc(), 
					pRLine->getLineName().toFloat(),
					pRLine->get(pXArray->getFirstPtIndex())->getPointNum().toFloat(),
					pRLine->get(pXArray->getLastPtIndex())->getPointNum().toFloat());
				stream << line << "\r\n";
			}
		}
	}

	file.close();
	return true;
}