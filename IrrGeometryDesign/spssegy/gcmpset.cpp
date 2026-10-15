#include "gproject.h"
#include "gcmpset.h"

//----global function----------------------------------------------------------
int gCompareCMPPointByX( GCMPPoint *p1, GCMPPoint *p2 )
{
	if(p1->getX() < p2->getX()) return -1;
	else if(p1->getX() > p2->getX()) return 1;
	else return 0;
}

int gCompareCMPLineByY( GCMPLine *p1, GCMPLine *p2 )
{
	if(p1->getY() < p2->getY()) return -1;
	else if(p1->getY() > p2->getY()) return 1;
	else return 0;
}

int gCompareCMPItemBySR(GCMPItem *p1, GCMPItem *p2)
{
	float r1 = p1->getSTRange();
	float r2 = p2->getSTRange();

// 	GSPoint *pSPoint = p1->getShot()->getSPoint();
// 	GRPoint *pRPoint = p1->getShot()->getRPointByTrcIndex(p1->getTrcIndex());
// 	if(pRPoint->getX() < pSPoint->getX()) r1 = -r1;
// 	pSPoint = p2->getShot()->getSPoint();
// 	pRPoint = p2->getShot()->getRPointByTrcIndex(p1->getTrcIndex());
// 	if(pRPoint->getX() < pSPoint->getX()) r2 = -r2;

	if(r1 < r2) return -1;
	else if(r1 > r2) return 1;
	else return 0;
}
//----class GCMPItem-----------------------------------------------------------
GCMPItem::GCMPItem()
{
	mpShotSet = GProject::getCurrentProject()->getShotSet();
	mFilePos = 0;
	mTrcIndex = -1;
	mShotIndex = -1;
	mSTRange = 0;
}

GCMPItem::GCMPItem(GShot *pShot, GRPoint *pRPoint, int trcIndex)
{
	mpShotSet = GProject::getCurrentProject()->getShotSet();
	mFilePos = pShot->getTrcOffsetByIndex(trcIndex);
	mTrcIndex = trcIndex;
	mShotIndex = pShot->getIndex();
	//mSTRange = pShot->getSPoint()->getX() - pRPoint->getX();
	mSTRange = gDistance2D(pShot->getSPoint()->getSPSX(), pShot->getSPoint()->getSPSY(),
						   pRPoint->getSPSX(), pRPoint->getSPSY());//炮检距
	///////////////////////////
	mAngle = gAngle(pShot->getSPoint()->getSPSX(), pShot->getSPoint()->getSPSY(),
		pRPoint->getSPSX(), pRPoint->getSPSY());//求方位角，add by zhao hu

    float sx = pShot->getSPoint()->getX();
	float rx = pRPoint->getX();
	if(rx<sx)
		mSTRange = -mSTRange;
	
}

void GCMPItem::save(QDataStream &stream)
{
	stream << mShotIndex << mSTRange;
	stream << mTrcIndex; 
	stream << mFilePos;
}

void GCMPItem::load(QDataStream &stream)
{
	stream >> mShotIndex >> mSTRange;
	stream >> mTrcIndex;
	stream >> mFilePos;
}
//----class GCMPPoint-----------------------------------------------------------
GCMPPoint::GCMPPoint()
{
	mpShotSet = GProject::getCurrentProject()->getShotSet();
	mCrossLineNum = -1;
	mNearShotIndex = -1;
	mNearRange = MAX_FLOAT;
	mX = 0;
	mY = 0;  
	mpCMPItemList = new GPtrVector<GCMPItem>(32);
	mpCMPItemList->setAutoDelete(true);
}

GCMPPoint::~GCMPPoint()
{
	delete mpCMPItemList;
}

void GCMPPoint::insert(GShot *pShot, GRPoint *pRPoint, int trcIndex, 
						 float xCMP, float yCMP)
{
	//查找有没有同点放炮,如有取野外记录号大者
	int count = this->count();
/*	for(int i=0; i<count; i++)
	{
		GCMPItem *pItem = get(i);
		if(pItem->getShot()->getSPoint() == pShot->getSPoint()) //同点放炮
		{
			if(pShot->getRecNum() < pItem->getRecNum()) return;
			else 
			{
				mpCMPItemList->remove(i);
				break;
			}
		}
	}
*/	
	mX = (mX*count+xCMP) / (count+1);
	mY = (mY*count+yCMP) / (count+1);
	GCMPItem *pCMPItem = new GCMPItem(pShot, pRPoint, trcIndex);
	mpCMPItemList->append(pCMPItem);
	if(fabs(pCMPItem->getSTRange()) < mNearRange)
	{
		mNearRange =fabs(pCMPItem->getSTRange());
		mNearShotIndex = pCMPItem->getShotIndex();
	}
}

void GCMPPoint::save(QDataStream &stream)
{
	stream << mCrossLineNum;
	stream << mX << mY;
	stream << mNearRange << mNearShotIndex;
	stream << (Q_INT32)count();

	for(int i=0; i<count(); i++)
	{
		GCMPItem *pCMPItem = get(i);
		pCMPItem->save(stream);
	}
}

void GCMPPoint::load(QDataStream &stream)
{
	stream >> mCrossLineNum;
	stream >> mX >> mY;
	stream >> mNearRange >> mNearShotIndex;

	Q_INT32 count;
	stream >> count;
	mpCMPItemList->clear();
	for(int i=0; i<count; i++)
	{
		GCMPItem *pCMPItem = new GCMPItem;
		mpCMPItemList->append(pCMPItem);
		pCMPItem->load(stream);
	}
}

void GCMPPoint::sort()
{
	mpCMPItemList->heapsort(gCompareCMPItemBySR);
}
//----class GCMPLine------------------------------------------------------------
GCMPLine::GCMPLine()
{
	GProject *pProject = GProject::getCurrentProject();
	mpShotSet = pProject->getShotSet();
	mpRLineSet = pProject->getRLineSet();
	mHalfHCMPInv = pProject->getHCMPInv()/2;
	mHalfVCMPInv = pProject->getVCMPInv()/2;
	mpCMPPointList = new GPtrVector<GCMPPoint>(1024);
	mY = 0;
}

GCMPLine::~GCMPLine()
{
	delete mpCMPPointList;
}

void GCMPLine::insert(GShot *pShot, GRPoint *pRPoint, int trcIndex)
{
	int i, first, last, index;
	GCMPPoint *pCMPPoint;
	GSPoint *pSPoint = pShot->getSPoint();
	float x = (pRPoint->getX() + pSPoint->getX())/2; //x为中心点位置
	float y = (pRPoint->getY() + pSPoint->getY())/2; //y为中心点位置
	//二分查找共中心点下标位置
	first = 0;	last = count()-1;
	index = -1;
	while(first <= last)
	{
		i = (first+last)/2;
		pCMPPoint = get(i);
		if(x > pCMPPoint->mX+mHalfHCMPInv) first = i+1;
		else if(x < pCMPPoint->mX-mHalfHCMPInv) last = i-1;
		else { index = i; break; }
	}

	//如果还没有相应的共中心点插入(排序)
	mY = (mY*count()+y) / (count()+1);
	if(index == -1)
	{
		pCMPPoint = new GCMPPoint;
		pCMPPoint->mX = x;
		pCMPPoint->mY = y;
		pCMPPoint->insert( pShot, pRPoint, trcIndex, x, y );
		mpCMPPointList->insertByOrder(pCMPPoint, gCompareCMPPointByX);
	}
	else pCMPPoint->insert(pShot, pRPoint, trcIndex, x, y);
}

int GCMPLine::getMaxCoverCount()
{
	int max = 0;
	for(int i=0; i<count(); i++)
	{
		GCMPPoint *pCMPPoint = get(i);
		if(pCMPPoint->count() > max)
			max = pCMPPoint->count();
	}
	return max;
}

void GCMPLine::sort()
{
	for(int i=0; i<count(); i++)
	{
		GCMPPoint *pCMPPoint = get(i);
		pCMPPoint->sort();//delete by psl in 060703
	}
}

void GCMPLine::save(QDataStream &stream)
{
	stream << mInLineNum;
	stream << mY;
	stream << (Q_INT32)count();

	for(int i=0; i<count(); i++)
	{
		GCMPPoint *pCMPPoint = get(i);
		pCMPPoint->save(stream);
	}
}

void GCMPLine::load(QDataStream &stream)
{
	stream >> mInLineNum;
	stream >> mY;

	Q_INT32 count;
	stream >> count;
	for(int i=0; i<count; i++)
	{
		GCMPPoint *pCMPPoint = new GCMPPoint;
		mpCMPPointList->append(pCMPPoint);
		pCMPPoint->load(stream);
	}
}
//----class GCMPLineSet--------------------------------------------------------
GCMPLineSet::GCMPLineSet()
{
	/*GProject *pProject = GProject::getCurrentProject();
	mpShotSet = pProject->getShotSet();
	mpRLineSet = pProject->getRLineSet();
	mHalfHCMPInv = pProject->getHCMPInv()/2;
	mHalfVCMPInv = pProject->getVCMPInv()/2;*/
	mpCMPLineList = new GPtrVector<GCMPLine>(8);
	mpCMPLineList->setAutoDelete(true);
}

GCMPLineSet::~GCMPLineSet()
{
	delete mpCMPLineList;
}

void GCMPLineSet::clear()
{
	mpCMPLineList->clear();
}

void GCMPLineSet::insert(GShot *pShot, GShotXLine *pXLine, GRLine *pRLine)
{
	if(pXLine->count() == 0) return;

	int i, j, count = 0;
	float y = 0;
	GShotXArray *pXArray;
	GRPoint *pRPoint;
	GProject *pProject = GProject::getCurrentProject();
	float halfVCMPInv = pProject->getVCMPInv() / 2 ;/// 2;
	bool isExisted = false;				//CMP线是否存在标志
	GCMPLine *pCMPLine;

	if(pProject->getProjectType() == gProject3D)
	{
		//----下面的程序每隔5点采样，计算CMP线的大致水平坐标----
		for(i=0; i<pXLine->count(); i++)
		{

			pXArray = pXLine->get(0);
			for(int j=pXArray->getFirstPtIndex(); j<=pXArray->getLastPtIndex(); j+=5)
			{
				pRPoint = pRLine->get(j);
				y += pRPoint->getY();
				count++;
			}
		}
		y /= count;
		y = (pShot->getSPoint()->getY() + y)/2;

		//----下面的程序将该关系线的共中心点并入----
		for(i=0; i<this->count(); i++)			//查找CMP线是否存在
		{
			pCMPLine = mpCMPLineList->at(i);
			if(pCMPLine->mY - halfVCMPInv <= y && y <= pCMPLine->mY + halfVCMPInv)
			{
				isExisted = true;
				break;
			}
		}
	}
	else
	{
		pCMPLine = mpCMPLineList->at(0);
		if(pCMPLine != NULL) isExisted = true;
	}

	if(!isExisted) pCMPLine = new GCMPLine;	//如果不存在则构造新线

	for(i=0; i<pXLine->count(); i++)
	{
		pXArray = pXLine->get( i );
		int trcIndex = pXArray->getMinTrcIndex();
		for(j=pXArray->getFirstPtIndex(); j<=pXArray->getLastPtIndex(); j++)
		{
			GRPoint *pRPoint = pRLine->get(j);
			//调用下级插入函数，为共中心线插入新的共中心点
			pCMPLine->insert(pShot, pRPoint, trcIndex);	
			trcIndex++;
		}
	}

	if( !isExisted ) mpCMPLineList->insertByOrder(pCMPLine, gCompareCMPLineByY);
}

void GCMPLineSet::extract(GShotSet *pShotSet,GRLineSet *pRLineSet)
{
	clear();
	GProject *pProject = GProject::getCurrentProject();
//	GShotSet *pShotSet = pProject->getShotSet();
//	GRLineSet *pRLineSet = pProject->getRLineSet();
	for(int i=0; i<pShotSet->count(); i++)
	{
		GShot *pShot = pShotSet->get( i );
		for(int j=0; j<pShot->count(); j++)
		{
			GShotXLine *pXLine = pShot->get(j);
			GRLine *pRLine = pRLineSet->getByLineName(pXLine->getLineName());
			insert(pShot, pXLine, pRLine);
		}
	}
	enCode();
}

int GCMPLineSet::getMaxCoverCount()
{
	int max = 0;
	for(int i=0; i<count(); i++)
	{
		GCMPLine *pCMPLine = mpCMPLineList->at(i);
		if( pCMPLine->getMaxCoverCount() > max )
			max = pCMPLine->getMaxCoverCount();
	}
	return max;
}

void GCMPLineSet::enCode()
{
	int i, j;
	float x, minCMPX = MAX_FLOAT;
	Q_INT32 index;
	GCMPLine *pCMPLine, *pMinLine;
	GCMPPoint *pCMPPoint;
	GProject *pProject = GProject::getCurrentProject();
	float halfVCMPInv = pProject->getVCMPInv() / 2;

	//给CMP线编号
	for(i=0; i<count(); i++ )
	{
		pCMPLine = mpCMPLineList->at(i);
		pCMPLine->mInLineNum = i+1;
		x = pCMPLine->get(0)->getX();
		if(minCMPX > x) 
		{
			minCMPX = x;
			index = i;
		}
	}
	pMinLine = mpCMPLineList->at(index);
	//为每条CMP线的CMP点编号
	for(i=0; i<count(); i++)
	{
		pCMPLine = mpCMPLineList->at(i);
		pCMPPoint = pCMPLine->get(0);
		for(j=0; j<pMinLine->count(); j++)
		{
			if(pMinLine->get(j)->getX()-halfVCMPInv <= pCMPPoint->getX() && 
			   pCMPPoint->getX() <= pMinLine->get(j)->getX()+halfVCMPInv )
				break;
		}
		index = j+1;
		for(j=0; j<pCMPLine->count(); j++)
		{
			pCMPPoint = pCMPLine->get(j);
			pCMPPoint->mCrossLineNum = index++;
		}
	}
}

void GCMPLineSet::sort()
{
	for(int i=0; i<count(); i++)
	{
		GCMPLine *pCMPLine = get(i);
		pCMPLine->sort();
	}
}

bool GCMPLineSet::save(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QDataStream stream(&file);
	stream << (Q_INT32)count();

	for(int i=0; i<count(); i++)
	{
		GCMPLine *pCMPLine = get(i);
		pCMPLine->save(stream);
	}

	file.close();
	return true;
}

bool GCMPLineSet::load(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly)) return false;

	QDataStream stream(&file);
	Q_INT32 count;
	stream >> count;

	for(int i=0; i<count; i++)
	{
		GCMPLine *pCMPLine = new GCMPLine;
		mpCMPLineList->append(pCMPLine);
		pCMPLine->load(stream);
	}

	file.close();
	return true;
}
