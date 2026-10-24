#include "gslineset.h"
#include "gmath.h"
#include <QTextStream>
#include "gutils.h"
#include "gspsparam.h"//add by zhaohu

//----内部全局函数--------------------------------------------------------------
int gCompareSPoint(GSPoint *p1, GSPoint *p2 )
{
	int ptNum1, ptNum2, bit;
	ptNum1 = gExtractInt(p1->getPointNum(), bit);
	ptNum2 = gExtractInt(p2->getPointNum(), bit);
	if(ptNum1 > ptNum2) return 1;
	else if(ptNum1 < ptNum2) return -1;
	else return 0;
	//// 使用下面这串代码，排序结果不是按照点号进行排序！！ [3/17/2023 Liu Wei]
	//float ptNum1, ptNum2, bit;
	//ptNum1 = sqrt(pow(p1->getX(),2)+pow(p1->getY(),2));
	//ptNum2 = sqrt(pow(p2->getX(),2)+pow(p2->getY(),2));
	//if(ptNum1 > ptNum2) return 1;
	//else if(ptNum1 < ptNum2) return -1;
	//else return 0;
}

int gCompareSLine(GSLine *p1, GSLine *p2)
{
	int lnNum1, lnNum2, bit;
	lnNum1 = gExtractInt(p1->getLineName(), bit);
	lnNum2 = gExtractInt(p2->getLineName(), bit);
	if(lnNum1 > lnNum2) return 1;
	else if(lnNum1 < lnNum2) return -1;
	else return 0;
}

//----class GSPoint-------------------------------------------------------------
GSPoint::GSPoint()
{
	mLineName = QString::null;
	mPointNum = QString::null;
	mSPSX = 0;	mSPSY = 0;	mSPSZ = 0;
	mDepth = 0;		mBase = 0;
	mX = 0;		mY = 0;
	mSPSStatic = 0;
	mTAO = 0;
	mpShotList = new GValueVector<Q_INT32>;
	mStatic = 0;
	mIsWellShot = true;
	//add by LiuWei
	mRandomOffsetX=0;
	mRandomOffsetY=0;
}

GSPoint::GSPoint(const QString &lineName, const QString &pointNum, 
				 float spsX, float spsY, float spsZ, float spsStatic, 
				 float depth, float base, float tao)
{
	mLineName = lineName;
	mPointNum = pointNum;
	mSPSX = spsX;	mSPSY = spsY;	mSPSZ = spsZ;
	mSPSStatic = spsStatic;
	mDepth = depth;		mBase = base;
	mX = 0;		mY = 0;
	mTAO = tao;
	mpShotList = new GValueVector<Q_INT32>;
	mIsWellShot = true;
	//add by LiuWei
	mRandomOffsetX=0;
	mRandomOffsetY=0;
}

GSPoint::~GSPoint()
{
	delete mpShotList;
}

int GSPoint::get(int index)
{
	if(0 <= index && index < count()) return mpShotList->at(index);
	else return -1;
}

void GSPoint::rotate(float sinAF, float cosAF)
{
	mX = mSPSX*cosAF+mSPSY*sinAF;
	mY = -mSPSX*sinAF+mSPSY*cosAF;
}

void GSPoint::translate(float x, float y)
{
	mX += x;
	mY += y;
}

bool GSPoint::save(QDataStream &stream)
{
	stream << mLineName;
	stream << mPointNum;
	stream << mSPSX << mSPSY << mSPSZ << mDepth;
	stream << mSPSStatic << mBase;
	stream << mTAO;
	stream << mX << mY;
	stream << (Q_INT32)count();
	for(int j=0; j<count(); j++)
	{
		stream << (Q_INT32)get(j);
	}
     Q_INT32 flag = 0;
	if(mIsWellShot)
     flag = 1;
    stream << flag;
	return true;
}

bool GSPoint::load(QDataStream &stream)
{
	Q_INT32 count, index;

	stream >> mLineName;
	stream >> mPointNum;
	stream >> mSPSX >> mSPSY >> mSPSZ >> mDepth;
	stream >> mSPSStatic >> mBase;
	stream >> mTAO;
	stream >> mX >> mY;
	stream >> count;
	for(int j=0; j<count; j++)
	{
		stream >> index;
		append(index);
	}
	Q_INT32 flag;
    stream >> flag;
	if(flag)mIsWellShot = true;
	else mIsWellShot = false;
	return true;
}
//----class GSLine--------------------------------------------------------------
GSLine::GSLine()
{
	mLineName = QString::null;
	mpSPointList = new GPtrVector<GSPoint>(128);
	mpSPointList->setAutoDelete(true);
}

GSLine::GSLine(const QString &lineName)
{
	mLineName = lineName;
	mpSPointList = new GPtrVector<GSPoint>(128);
	mpSPointList->setAutoDelete(true);
}

GSLine::~GSLine()
{
	delete mpSPointList;
}

GSPoint * GSLine::get(int index)
{
	return mpSPointList->at(index);
}

int GSLine::getIndexByPointNum(const QString &pointNum)
{
	for(int i=0; i<count(); i++)
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		if(pSPoint->getPointNum() == pointNum) return i;
	}
	return -1;
}

GSPoint * GSLine::getByPointNum(const QString &pointNum)
{
	int index = getIndexByPointNum(pointNum);
	if(index < 0) return NULL;
	else return mpSPointList->at(index);
}

void GSLine::getSPSRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		if(pSPoint->getSPSX() < minX) minX = pSPoint->getSPSX();
		if(pSPoint->getSPSX() > maxX) maxX = pSPoint->getSPSX();
		if(pSPoint->getSPSY() < minY) minY = pSPoint->getSPSY();
		if(pSPoint->getSPSY() > maxY) maxY = pSPoint->getSPSY();
	}
}

void GSLine::getRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		if(pSPoint->getX() < minX) minX = pSPoint->getX();
		if(pSPoint->getX() > maxX) maxX = pSPoint->getX();
		if(pSPoint->getY() < minY) minY = pSPoint->getY();
		if(pSPoint->getY() > maxY) maxY = pSPoint->getY();
	}
}

void GSLine::rotate(float sinAF, float cosAF, float &minX, float &minY)
{
	for(int i=0; i<count(); i++) 
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		pSPoint->rotate(sinAF, cosAF);
		if(pSPoint->getX() < minX) minX = pSPoint->getX();
		if(pSPoint->getY() < minY) minY = pSPoint->getY();
	}
}

void GSLine::translate(float x, float y)
{
	for(int i=0; i<count(); i++)
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		pSPoint->translate(x, y);
	}
}

void GSLine::sort()
{
	mpSPointList->heapsort(gCompareSPoint);		//按点号排序
}

void GSLine::clearShotList()
{
	for(int i=0; i<count(); i++)
	{
		GSPoint *pSPoint = mpSPointList->at(i);
		pSPoint->clear();
	}
}

void GSLine::save(QDataStream &stream)
{
	GSPoint *pSPoint;
	stream << mLineName;
	stream << (Q_INT32)mpSPointList->count();
	for(int i=0; i<count(); i++)
	{
		pSPoint = mpSPointList->at(i);
		pSPoint->save(stream);
	}
}

void GSLine::load(QDataStream &stream)
{
	Q_INT32 count;
	GSPoint *pSPoint;
	stream >> mLineName ;
	stream >> count;
	for(int i=0; i<count; i++)
	{
		pSPoint = new GSPoint;
		pSPoint->load(stream);
		mpSPointList->append(pSPoint);
	}
}
//----class GSLineSet-----------------------------------------------------------
GSLineSet::GSLineSet()
{
	mpSLineList = new GPtrVector<GSLine>(8);
	mpSLineList->setAutoDelete(true);
}

GSLineSet::~GSLineSet()
{
	delete mpSLineList;
}

void GSLineSet::clearVainShots()
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = get(i);
		for(int j=0; j<pSLine->count(); )
		{
			GSPoint *pSPoint = pSLine->get(j);
			if(pSPoint->count() == 0) pSLine->remove(j);
			else j++;
		}
	}
}

int GSLineSet::getIndexByLineName(const QString &lineName)
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = mpSLineList->at(i);
		if(pSLine->getLineName() == lineName) return i;
	}
	return -1;
}

GSLine * GSLineSet::getByLineName(const QString &lineName)
{
	int index = getIndexByLineName(lineName);
	if(index < 0) return NULL;
	else return mpSLineList->at(index);
}

void GSLineSet::getSPSRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = mpSLineList->at(i);
		pSLine->getSPSRange(minX, minY, maxX, maxY);
	}
}

void GSLineSet::getRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = mpSLineList->at(i);
		pSLine->getRange(minX, minY, maxX, maxY);
	}
}

void GSLineSet::rotate(float sinAF, float cosAF, float &minX, float &minY)
{
	for(int i=0; i<count(); i++) 
	{
		GSLine *pSLine = mpSLineList->at(i);
		pSLine->rotate(sinAF, cosAF, minX, minY);
	}
}

void GSLineSet::translate(float x, float y)
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = mpSLineList->at(i);
		pSLine->translate(x, y);
	}
}

void GSLineSet::sort()
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine= get(i);
		pSLine->sort();
	}
	mpSLineList->heapsort(gCompareSLine);		//按线号排序
}

void GSLineSet::clearShotList()
{
	for(int i=0; i<count(); i++)
	{
		GSLine *pSLine = mpSLineList->at(i);
		pSLine->clearShotList();
	}
}

bool GSLineSet::save(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(IO_WriteOnly)) return false;

	QDataStream stream(&file);
	stream << (Q_INT32)count();

	GSLine *pSLine;
	for( uint i=0; i<mpSLineList->count(); i++ )
	{
		pSLine = mpSLineList->at(i);
		pSLine->save(stream);
	}

	file.close();
	return true;
}

bool GSLineSet::load(const QString &fileName)
{
	QFile file( fileName );
	if(!file.open(IO_ReadOnly)) return false;

	QDataStream stream( &file );
//	stream.setProject(mpProject);
	clear();	//清空原数据列表

	Q_INT32 count;
	stream >> count;

	GSLine *pSLine;
	for(int i=0; i<count; i++)
	{
		pSLine = new GSLine;
		pSLine->load( stream );
		mpSLineList->append( pSLine );
	}
	
	file.close();
	return true;
}

bool GSLineSet::loadStatic(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(IO_ReadOnly)) return false;

	QTextStream stream(&file);
	
	QString line;
	QString lineName, ptNum;

	while(!stream.atEnd())
	{
		line = stream.readLine();
		QTextIStream istream(&line);
		istream >> lineName;
		GSLine *pSLine = getByLineName(lineName);
		if(pSLine == NULL) continue;
		istream >> ptNum;
		GSPoint *pSPoint = pSLine->getByPointNum(ptNum);
		if(pSPoint == NULL) continue;
		istream >> pSPoint->mSPSStatic;

	}
	file.close();

	return true;
}


void GSLineSet::create(GSPSParam *pSPSParam)
{
	int i, j;
	double x, y, cosAf, sinAf;
	GSLine *pSLine;

	clear();

	float af = pSPSParam->mSAf;
	sinAf = (float)sin(af);
	cosAf = (float)cos(af);
	for(i=0; i<pSPSParam->mSLineCount; i++)
	{
		pSLine = new GSLine();
//		pSLine->mLineName = QString::number(i+1);
		if(pSPSParam->mSLineType == GSPSParam::gCrossLine)
		{
			x = pSPSParam->mSX0 + i*pSPSParam->mSLineInv*sinAf;
			y = pSPSParam->mSY0 - i*pSPSParam->mSLineInv*cosAf;
		}
		else if(pSPSParam->mSLineType == GSPSParam::gInLine)
		{
			x = pSPSParam->mSX0 - i*pSPSParam->mSLineInv*sinAf;
			y = pSPSParam->mSY0 + i*pSPSParam->mSLineInv*cosAf;
		}
		else
		{
			x = pSPSParam->mSX0 + i*pSPSParam->mSLineInv*sin(pSPSParam->mAf+PI/2);
			y = pSPSParam->mSY0 - i*pSPSParam->mSLineInv*cos(pSPSParam->mAf+PI/2);
		}


		for(j=0; j<pSPSParam->mSPtCount; j++)
		{
//			GSPoint *pSPoint = new GSPoint(pSLine);
			GSPoint *pSPoint = new GSPoint();//add by zhaohu
			pSPoint->mLineName = QString::number(i+1);
			pSPoint->mPointNum = QString::number(j+1);
			pSPoint->setSPSX(x);
			pSPoint->setSPSY(y);
			pSPoint->setSPSZ(pSPSParam->mSZ0);
			pSLine->append(pSPoint);

			x += pSPSParam->mSPtInv*cosAf;
			y += pSPSParam->mSPtInv*sinAf;
		}

		mpSLineList->append(pSLine);
	}
}
//已有规则观测系统排列基础上替换坐标(炮线排列）
void GSLineSet::create( QVector<QVector<float> > &sxcrd, QVector<QVector<float> > &sycrd, GSPSParam *pSPSParam )
{
	int i, j;
	double x, y, cosAf, sinAf;

	//clear();

	float af = pSPSParam->mSAf;
	sinAf = (float)sin(af);
	cosAf = (float)cos(af);

	for(i=0; i<pSPSParam->mSLineCount; i++)
	{
		// 正确性还有待验证 [Liu Wei 6/16/2023]
		if(pSPSParam->mSLineType == GSPSParam::gCrossLine)
		{
			x = pSPSParam->mSX0 + (sxcrd[i][0]-pSPSParam->mSX0)*sinAf;
			y = pSPSParam->mSY0 - (sxcrd[i][0]-pSPSParam->mSX0)*cosAf;
			//x = pSPSParam->mSX0 + i*pSPSParam->mSLineInv*sinAf;
			//y = pSPSParam->mSY0 - i*pSPSParam->mSLineInv*cosAf;
		}
		else if(pSPSParam->mSLineType == GSPSParam::gInLine)
		{
			x = pSPSParam->mSX0 - (sycrd[i][0]-pSPSParam->mSY0)*sinAf;
			y = pSPSParam->mSY0 + (sycrd[i][0]-pSPSParam->mSY0)*cosAf;
			//x = pSPSParam->mSX0 - i*pSPSParam->mSLineInv*sinAf;
			//y = pSPSParam->mSY0 + i*pSPSParam->mSLineInv*cosAf;
		}
		else
		{
			x = pSPSParam->mSX0 + (sxcrd[i][0]-pSPSParam->mSX0)*sin(pSPSParam->mAf+PI/2);
			y = pSPSParam->mSY0 - (sxcrd[i][0]-pSPSParam->mSX0)*cos(pSPSParam->mAf+PI/2);
			//x = pSPSParam->mSX0 + i*pSPSParam->mSLineInv*sin(pSPSParam->mAf+PI/2);
			//y = pSPSParam->mSY0 - i*pSPSParam->mSLineInv*cos(pSPSParam->mAf+PI/2);
		}

		int ij=0;
		for(j=0; j<pSPSParam->mSPtCount; j++)
		{
			//GSPoint *pSPoint = new GSPoint();//add by zhaohu
			//pSPoint->mLineName = QString::number(i+1);
			//pSPoint->mPointNum = QString::number(j+1);
			mpSLineList->at(i)->mpSPointList->at(j)->mSPSX=x;
			mpSLineList->at(i)->mpSPointList->at(j)->mSPSY=y;
			//pSPoint->setSPSX(x);
			//pSPoint->setSPSY(y);
			//pSPoint->setSPSZ(pSPSParam->mSZ0);
			//pSLine->append(pSPoint);

			if (++ij < pSPSParam->mSPtCount)//防止越界
			{
				x += (sycrd[i][ij]-sycrd[i][j])*cosAf;
				y += (sycrd[i][ij]-sycrd[i][j])*sinAf;
			}

			//x += pSPSParam->mSPtInv*cosAf;
			//y += pSPSParam->mSPtInv*sinAf;
		}
	}
}
void  GSLineSet::createByold(GSLineSet *SLSet)//add by zhaohu
{
	int i,j;
	double x, y, cosAf, sinAf;
	GSLine *pSLine;

	clear();

	for(i=0; i<SLSet->count(); i++)
	{
		GSLine *oldSLine =SLSet->get(i);
		pSLine = new GSLine();
//		pSLine->mLineName = QString::number(i+1);
		pSLine->mLineName = oldSLine->getLineName();
		
		for(j=0; j<oldSLine->count(); j++)
		{
			GSPoint *pSPoint = new GSPoint();//add by zhaohu
//			pSPoint->mLineName = QString::number(i+1);
//			pSPoint->mPointNum = QString::number(j+1);
			pSPoint->mLineName = oldSLine->getLineName();
			pSPoint->mPointNum = oldSLine->get(j)->getPointNum();
			pSPoint->setSPSX(oldSLine->get(j)->getSPSX());
			pSPoint->setSPSY(oldSLine->get(j)->getSPSY());
			pSPoint->setSPSZ(oldSLine->get(j)->getSPSZ());
			pSLine->append(pSPoint);
		}
		mpSLineList->append(pSLine);
	}
}


bool GSLineSet::save(QDataStream &stream)//add by zhaohu
{
	stream << (Q_INT32)count();

	gSkipPadBytes(stream, GSLINESET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) return false;

	GSLine *pSLine;
	for(int i=0; i<mpSLineList->count(); i++)
	{
		pSLine = mpSLineList->at(i);
		pSLine->save(stream);
	}

	return true;
}

bool GSLineSet::load(QDataStream &stream)
{
	clear();	//清空原数据列表

	Q_INT32 count;
	stream >> count;

	gSkipPadBytes(stream, GSLINESET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) return false;

	GSLine *pSLine;
	for(int i=0; i<count; i++)
	{
		pSLine = new GSLine();
		pSLine->load( stream );
		
		mpSLineList->append( pSLine );
	}

	return true;
}

//sps2.1格式
bool GSLineSet::saveSFile2(const QString &fileName)//add by zhaohu
{
	// 以下为1.0格式
/*	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QTextStream stream(&file);
	stream.setEncoding(QTextStream::Locale);
	QString line;

	int i, j;
	for(i=0; i<count(); i++)
	{
		GSLine *pSLine = get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			GSPoint *pSPoint = pSLine->get(j);
			line.sprintf("S%-16s%8s1%2s%4s%4.1f%4d%2s%4s%9.1f%10.1f%6.1f%3s%6s", 
				(const char *)pSLine->getLineName(), 
				(const char *)pSPoint->getPointNum(), "  ", "    ",
				pSPoint->getDepth(), 0, "  ", "    ",
				pSPoint->getSPSX(), pSPoint->getSPSY(), pSPoint->getSPSZ(),
				"   ", "      ");
			stream << line << "\r\n";

		}
	}

	file.close();
	return true;
*/
	//以下为2.1格式
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QTextStream stream(&file);
	stream.setEncoding(QTextStream::Locale);
	QString line;

	int i, j;
	for(i=0; i<count(); i++)
	{
		GSLine *pSLine = get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			GSPoint *pSPoint = pSLine->get(j);
			line.sprintf("S%10.2f%10.2f%2s1%2s%4s%4.1f%4d%2s%6s%9.1f%10.1f%6.1f%3s%6s", 
				pSPoint->getLineName().toFloat(), 
				pSPoint->getPointNum().toFloat(), "  ","  ", "    ",
				pSPoint->getDepth(), 0, "  ", "    ",
				pSPoint->getSPSX(), pSPoint->getSPSY(), pSPoint->getSPSZ(),
				"   ", "      ");
			stream << line << "\r\n";
		}
	}

	file.close();
	return true;
}