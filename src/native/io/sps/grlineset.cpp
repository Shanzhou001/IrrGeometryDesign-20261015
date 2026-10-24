#include "grlineset.h"
#include "gproject.h"

//----全局函数------------------------------------------------------------------
int gCompareRPointByX(GRPoint *p1, GRPoint *p2)
{
	if(p1->getX() > p2->getX()) return 1;
	else if(p1->getX() < p2->getX()) return -1;
	else return 0;
}

int gCompareRPointByY(GRPoint *p1, GRPoint *p2)
{
	if(p1->getY() > p2->getY()) return 1;
	else if(p1->getY() < p2->getY()) return -1;
	else return 0;
}

int gCompareRPointByPointNum(GRPoint *p1, GRPoint *p2)
{
// 	if(p1->getPointNum().toInt() > p2->getPointNum().toInt() ) return 1;
// 	else if(p1->getPointNum().toInt()  < p2->getPointNum().toInt() ) return -1;
// 	else return 0;

	if(p1->getPointNum().toFloat() > p2->getPointNum().toFloat() ) return 1;//modify by zhaohu
	else if(p1->getPointNum().toFloat()  < p2->getPointNum().toFloat() ) return -1;
	else return 0;
}

//----class GRPoint-------------------------------------------------------------
GRPoint::GRPoint()
{
	mState = rpUnSaved;
	mPosState = 1;
	mEvaluateState = rpEvaluate;
	mLineName = QString::null;
	mPointNum = QString::null;
	mSPSX = 0;	mSPSY = 0;	mSPSZ = 0;
	mBase = 0;	
	mSPSStatic = 0;
	mX = 0;		mY = 0;
	mStatic = 0;
	mpShotList = new GValueVector<Q_INT32>;
	mFitFBTStatic = 0;
	mPSVTimeDiff = 10;
}

GRPoint::GRPoint(const QString &lineName, const QString &pointNum, 
				 float spsX, float spsY, float spsZ, float spsStatic, float base)
{
	mState = rpUnSaved;
	mPosState = 1;
	mLineName = lineName;
	mPointNum = pointNum;
	mSPSX = spsX;	mSPSY = spsY;	mSPSZ = spsZ;
	mBase = base;	
	mSPSStatic = spsStatic;
	mX = 0;		mY = 0;
	mStatic = 0;
	mpShotList = new GValueVector<Q_INT32>;
	mFitFBTStatic = 0;
	mPSVTimeDiff = 10;
}

GRPoint::~GRPoint()
{
	delete mpShotList;
}

int GRPoint::get(int index)
{
	if(0 <= index && index < count()) return mpShotList->at(index);
	else return -1;
}

void GRPoint::rotate(float sinAF, float cosAF)
{
	mX = mSPSX*cosAF+mSPSY*sinAF;
	mY = -mSPSX*sinAF+mSPSY*cosAF;
}

void GRPoint::translate(float x, float y)
{
	mX += x;
	mY += y;
}

float GRPoint::getStatic()
{
	GProject *pProject = mpProject;
	return 1000*(mSPSZ - pProject->getBase()) / pProject->getVelocity();
}

bool GRPoint::save(QDataStream &stream)
{
	stream << mPosState;
	stream << mEvaluateState;
	stream << mLineName;
	stream << mPointNum;
	stream << mSPSX << mSPSY << mSPSZ;
	stream << mBase << mSPSStatic;
	stream << mX << mY;
	stream << (Q_INT32)count();
	for(int j=0; j<count(); j++)
	{
		stream << (Q_INT32)get(j);
	}

	mState = rpSaved;
	return true;
}

bool GRPoint::load(QDataStream &stream)
{
	Q_INT32 count, index, data;

	stream >> mPosState;
	stream >> data;
	mEvaluateState = (GREvaluateState)data;
	stream >> mLineName;
	stream >> mPointNum;
	stream >> mSPSX >> mSPSY >> mSPSZ;
	stream >> mBase >> mSPSStatic;
	stream >> mX >> mY;
	stream >> count;
	for(int j=0; j<count; j++)
	{
		stream >> index;
		append(index);
	}

	mState = rpSaved;
	return true;
}
//----class GRLine--------------------------------------------------------------
GRLine::GRLine(bool isAtuoDel)
{
	mLineName = QString::null;
	mpRPointList = new GPtrVector<GRPoint>(128);
	mpRPointList->setAutoDelete(isAtuoDel);
}

GRLine::GRLine(const QString &lineName, bool isAtuoDel)
{
	mLineName = lineName;
	mpRPointList = new GPtrVector<GRPoint>(128);
	mpRPointList->setAutoDelete(isAtuoDel);
}

GRLine::~GRLine()
{
	delete mpRPointList;
}

void GRLine::clearShotList()
{
	for(int i=0; i<count(); i++)
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		pRPoint->clear();
	}
}

int GRLine::getIndexByPointNum(const QString &pointNum)
{
	for(int i=0; i<count(); i++)
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		if(pRPoint->getPointNum().toFloat() == pointNum.toFloat()) return i;
	}
	return -1;
}

GRPoint * GRLine::getByPointNum(const QString &pointNum)
{
	int index = getIndexByPointNum(pointNum);
	if(index < 0) return NULL;
	else return mpRPointList->at(index);
}

void GRLine::getSPSRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		if(pRPoint->getSPSX() < minX) minX = pRPoint->getSPSX();
		if(pRPoint->getSPSX() > maxX) maxX = pRPoint->getSPSX();
		if(pRPoint->getSPSY() < minY) minY = pRPoint->getSPSY();
		if(pRPoint->getSPSY() > maxY) maxY = pRPoint->getSPSY();
	}
}

void GRLine::getRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		if(pRPoint->getX() < minX) minX = pRPoint->getX();
		if(pRPoint->getX() > maxX) maxX = pRPoint->getX();
		if(pRPoint->getY() < minY) minY = pRPoint->getY();
		if(pRPoint->getY() > maxY) maxY = pRPoint->getY();
	}
}

void GRLine::rotate(float sinAF, float cosAF, float &minX, float &minY)
{
	for(int i=0; i<count(); i++) 
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		pRPoint->rotate(sinAF, cosAF);
		if(pRPoint->getX() < minX) minX = pRPoint->getX();
		if(pRPoint->getY() < minY) minY = pRPoint->getY();
	}
}

void GRLine::translate(float x, float y)
{
	for(int i=0; i<count(); i++)
	{
		GRPoint *pRPoint = mpRPointList->at(i);
		pRPoint->translate(x, y);
	}
}

bool GRLine::getAngleParam(float &sinAF, float &cosAF)
{
	if(count() <= 1) return false;	//如果检波点少于2退出

	int i, count;
	count = mpRPointList->count();
	float *xArray = new float[count];
	float *yArray = new float[count];
	float spsX0, spsY0, spsXN, spsYN, dx, dy;
	float a, b, x0, y0, xN, yN, d;		//拟合直线参数 y = a*x+b
	bool isSwap = false;
	//初步判断曲线方向
	spsX0 = mpRPointList->at(0)->getSPSX();
	spsY0 = mpRPointList->at(0)->getSPSY();
	spsXN = mpRPointList->at(count-1)->getSPSX();
	spsYN = mpRPointList->at(count-1)->getSPSY();
	dx = fabs(spsXN-spsX0);
	dy = fabs(spsYN-spsY0);
	if(dx < dy) isSwap = true;	//如果角度大于45则交换X,Y（为什么要交换，交换后有什么效果？ 刘伟）

	xArray[0] = 0.0f;		yArray[0] = 0.0f;		
	for(i=1; i<count; i++)
	{
		xArray[i] = mpRPointList->at(i)->getSPSX() - spsX0;
		yArray[i] = mpRPointList->at(i)->getSPSY() - spsY0;
	}
	
	// 原始代码 [3/18/2023 Liu Wei]
	if(!isSwap)	//求旋转角度	
	{
		gFittingLine(xArray, yArray, 0, count-1, a, b);// [3/17/2023 Liu Wei] 直接计算角度参数
		x0 = 0.0f;		y0 = b;
		xN = xArray[count-1];	yN = a*xN+b;
	}
	else 
	{
		gFittingLine(yArray, xArray, 0, count-1, a, b); //（为什么要交换，交换后有什么效果？ 刘伟:精度更高?!）
		y0 = 0.0f;		x0 = b;
		yN = yArray[count-1];	xN = a*yN+b;
	}

	dx = xN-x0;		dy = yN-y0;
	d = gDistance2D( x0, y0, xN, yN );
	sinAF = dy/d;	cosAF = dx/d; 

	//解决坐标交换的问题（暂时不需要）
	//double tempAngle=GProject::getAngle(sinAF,cosAF)-PI/2;
	//sinAF=sinf(tempAngle), cosAF=cosf(tempAngle);

	return true;
}

void GRLine::sortByX()
{
	mpRPointList->heapsort(gCompareRPointByX);
}

void GRLine::sortByY()
{
	mpRPointList->heapsort(gCompareRPointByY);
}

void GRLine::sortByPointNum()
{
	mpRPointList->heapsort(gCompareRPointByPointNum);
}

void GRLine::save(QDataStream &stream)
{
	GRPoint *pRPoint;
	stream << mLineName << (Q_INT32)count();
	for(int i=0; i<count(); i++)
	{
		pRPoint = mpRPointList->at(i);
		pRPoint->save(stream);
	}
}

void GRLine::load(QDataStream &stream)
{
	Q_INT32 count;
	GRPoint *pRPoint;
	stream >> mLineName >> count;
	for(int i=0; i<count; i++)
	{
		pRPoint = new GRPoint;
		pRPoint->load(stream);
		mpRPointList->append(pRPoint);
	}
}
//----class GRLineSet-----------------------------------------------------------
GRLineSet::GRLineSet()
{
	mpRLineList = new GPtrVector<GRLine>(8);
	mpRLineList->setAutoDelete(true);
}

GRLineSet::~GRLineSet()
{
	delete mpRLineList;
}

void GRLineSet::clearShotList()
{
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		pRLine->clearShotList();
	}
}

int GRLineSet::getIndexByLineName(const QString &lineName)
{
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		if(pRLine->getLineName() == lineName) return i;
	}
	return -1;
}

GRLine * GRLineSet::getByLineName(const QString &lineName)
{
	int index = getIndexByLineName(lineName);
	if(index < 0) return NULL;
	else return mpRLineList->at(index);
}

void GRLineSet::getSPSRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		pRLine->getSPSRange(minX, minY, maxX, maxY);
	}
}

void GRLineSet::getRange(float &minX, float &minY, float &maxX, float &maxY)
{
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		pRLine->getRange(minX, minY, maxX, maxY);
	}
}

void GRLineSet::rotate(float sinAF, float cosAF, float &minX, float &minY)
{
	for(int i=0; i<count(); i++) 
	{
		GRLine *pRLine = mpRLineList->at(i);
		pRLine->rotate(sinAF, cosAF, minX, minY);
	}
}

void GRLineSet::translate(float x, float y)
{
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		pRLine->translate(x, y);
	}
}

bool GRLineSet::getAngleParam(float &sinAF, float &cosAF)
{
	bool isSuccess = false;
	float sumSinAF = 0, sumCosAF = 0;
	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = mpRLineList->at(i);
		if(pRLine->getAngleParam(sinAF, cosAF))
		{
			sumSinAF += sinAF;
			sumCosAF += cosAF;
			isSuccess = true;
		}
	}

	if(isSuccess) 
	{
		sinAF = sumSinAF/count();
		cosAF = sumCosAF/count();
		return true;
	}
	else return false;
}

bool GRLineSet::save(QDataStream &stream)// add by zhaohu
{
	stream << (Q_INT32)count();
	gSkipPadBytes(stream, GRLINESET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) 
	{
		return false;
	}

	GRLine *pRLine;
	for(int i=0; i<count(); i++)
	{
		pRLine = mpRLineList->at(i);
		pRLine->save(stream);
	}

	return true;
}


bool GRLineSet::save(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(IO_WriteOnly)) return false;

	QDataStream stream( &file );
	stream << (Q_INT32)count();

	GRLine *pRLine;
	for(int i=0; i<count(); i++)
	{
		pRLine = mpRLineList->at(i);
		pRLine->save(stream);
	}

	file.close();
	return true;
}

bool GRLineSet::saveToDB(const QString &workAreaNo, const QString &lineBindNo,
						 QProgressDialog *dlg)
{
	/*int i, j, step = 0, count = 0;
	GReceiveTable table(lineBindNo);
	dlg->setProgress( 0 );

	for(i=0; i<this->count(); i++)
	{
		count += get(i)->count();
	}

	for(i=0; i<this->count(); i++)
	{
		GRLine *pRLine = get(i);
		for(j=0; j<pRLine->count(); j++)
		{
			GRPoint *pRPoint = pRLine->get(j);
			if(pRPoint->mState == rpUnSaved)
			{
				if(!table.insert(pRLine->mLineName, pRPoint)) return false;
			}
			else if(pRPoint->mState == rpUpdated)
			{
				if(!table.update(pRLine->mLineName, pRPoint)) return false;
			}
			step++;
			dlg->setProgress(100.0*step/count);
		}
	}*/
	return true;
}

bool GRLineSet::load(QDataStream &stream)
{
	clear();	//清空原数据列表

	Q_INT32 count;
	stream >> count;
	gSkipPadBytes(stream, GRLINESET_PAD_SIZE);
	if(stream.device()->status() != IO_Ok) return false;

	GRLine *pRLine;
	for(int i=0; i<count; i++)
	{
		pRLine = new GRLine;
//		pRLine->mpRLineSet = this;
		pRLine->load(stream);
//		{
//			clear();
//			return false;
//		}
		mpRLineList->append( pRLine );
	}

	return true;
}

bool GRLineSet::load(const QString &fileName)
{
	QFile file( fileName );
	if(!file.open(IO_ReadOnly)) return false;

	QDataStream stream( &file );
	clear();	//清空原数据列表

	Q_INT32 count;
	stream >> count;

	GRLine *pRLine;
	for(int i=0; i<count; i++)
	{
		pRLine = new GRLine;
		pRLine->load( stream );
		mpRLineList->append( pRLine );
	}
	
	file.close();
	return true;
}

void GRLineSet::extractInline(GRLine *pSrcRLine, GRLine *pDestRLine)
{
	pDestRLine->clear();
	pDestRLine->mLineName = pSrcRLine->mLineName;
	for(int i=0; i<pSrcRLine->count(); i++)
	{
		pDestRLine->append(pSrcRLine->get(i));
	}
}

void GRLineSet::extractCrossline(float x, float diffx, GRLine *pRLine)
{
	pRLine->clear();
	pRLine->mLineName = QString::null;
	for(int i=0; i<count(); i++ )
	{
		GRLine *p = get(i);
		for(int j=0; j<p->count(); j++)
		{
			GRPoint *pRPoint = p->get(j);
			if(fabs(pRPoint->getX()-x) <= diffx)
			{
				pRLine->append( pRPoint );
			}
		}
	}
}

bool GRLineSet::loadStatic(const QString &fileName)
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
		GRLine *pRLine = getByLineName(lineName);
		if(pRLine == NULL) continue;
		istream >> ptNum;
		GRPoint *pRPoint = pRLine->getByPointNum(ptNum);
		if(pRPoint == NULL) continue;
		istream >> pRPoint->mSPSStatic;

	}
	file.close();
	return true;
}


void GRLineSet::create(GSPSParam *pSPSParam)//add by zhaohu 
{
	int i, j, r0;
	double x, y, cosAf, sinAf;
	GRLine *pRLine;

	clear();

	sinAf = (float)sin(pSPSParam->mAf);
	cosAf = (float)cos(pSPSParam->mAf);
	if(pSPSParam->mRX0 > pSPSParam->mSX0)
	{
		r0 = (int)(pSPSParam->mRX0 - pSPSParam->mSX0);
	}
	else r0 = 0;
	for(i=0; i<pSPSParam->mRLineCount; i++)
	{
		pRLine = new GRLine(this);
		pRLine->setLineName(QString::number(i+1));
		x = pSPSParam->mRX0 - i*pSPSParam->mRLineInv*sinAf;
		y = pSPSParam->mRY0 + i*pSPSParam->mRLineInv*cosAf;

		for(j=0; j<pSPSParam->mRPtCount; j++)
		{
			GRPoint *pRPoint = new GRPoint();
			pRPoint->mLineName=QString::number(i+1);
			pRPoint->mPointNum = QString::number(j+1);
			pRPoint->mSPSX = x;
			pRPoint->mSPSY = y;
			pRPoint->mSPSZ = pSPSParam->mRZ0;
			pRLine->append(pRPoint);

			x += pSPSParam->mRPtInv*cosAf;
			y += pSPSParam->mRPtInv*sinAf;
		}

		mpRLineList->append(pRLine);
	}
}
//已有规则观测系统排列基础上替换坐标(测线排列）
void GRLineSet::create( QVector<QVector<float> > &xcrd, QVector<QVector<float> > &ycrd, GSPSParam *pSPSParam )
{
	int i, j;
	double x, y, cosAf, sinAf;

	//clear(); // 清除已有链表数据 [Liu Wei 6/16/2023]

	sinAf = (float)sin(pSPSParam->mAf);
	cosAf = (float)cos(pSPSParam->mAf);

	for(i=0; i<pSPSParam->mRLineCount; i++)
	{
		//pRLine = new GRLine(this);
		//pRLine->setLineName(QString::number(i+1));
		x = pSPSParam->mRX0 - (ycrd[i][0]-pSPSParam->mRY0)*sinAf;
		y = pSPSParam->mRY0 + (ycrd[i][0]-pSPSParam->mRY0)*cosAf;
		//x = pSPSParam->mRX0 - i*pSPSParam->mRLineInv*sinAf;
		//y = pSPSParam->mRY0 + i*pSPSParam->mRLineInv*cosAf;
		
		int ij=0;
		for(j=0; j<pSPSParam->mRPtCount; j++)
		{
			//GRPoint *pRPoint = new GRPoint();
			//pRPoint->mLineName=QString::number(i+1);
			//pRPoint->mPointNum = QString::number(j+1);
			mpRLineList->at(i)->mpRPointList->at(j)->mSPSX=x;
			mpRLineList->at(i)->mpRPointList->at(j)->mSPSY=y;
			//pRPoint->mSPSX = x;
			//pRPoint->mSPSY = y;
			//pRPoint->mSPSZ = pSPSParam->mRZ0;
			
			if (++ij < pSPSParam->mRPtCount)//防止越界
			{
				x += (xcrd[i][ij]-xcrd[i][j])*cosAf;
				y += (xcrd[i][ij]-xcrd[i][j])*sinAf;
			}

			//x += pSPSParam->mRPtInv*cosAf;
			//y += pSPSParam->mRPtInv*sinAf;
		}
	}
}
//sps2.1格式
bool GRLineSet::saveRFile2(const QString &fileName)//add by zhaohu 
{

	//以下为1.0格式
/*	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)) return false;

	QTextStream stream(&file);
	stream.setEncoding(QTextStream::Locale);
	QString line;

	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = get(i);
		for(int j=0; j<pRLine->count(); j++)
		{
			GRPoint *pRPoint = pRLine->get(j);
			line.sprintf("R%-16s%8s1%2s%4s%4.1f%4d%2s%4s%9.1f%10.1f%6.1f%3s%6s", 
				(const char *)pRPoint->getLineName(), 
				(const char *)pRPoint->getPointNum(), "  ", "    ",
				0.0f, 0, "  ", "    ",
				pRPoint->getSPSX(), pRPoint->getSPSY(), pRPoint->getSPSZ(),
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

	for(int i=0; i<count(); i++)
	{
		GRLine *pRLine = get(i);
		for(int j=0; j<pRLine->count(); j++)
		{
			GRPoint *pRPoint = pRLine->get(j);
			line.sprintf("R%10.2f%10.2f%2s1%2s%4s%4.1f%4d%2s%6s%9.1f%10.1f%6.1f%3s%6s", 
				pRPoint->getLineName().toFloat(), 
				pRPoint->getPointNum().toFloat(), "  ","  ", "    ",
				0.0f, 0, "  ", "    ",
				pRPoint->getSPSX(), pRPoint->getSPSY(), pRPoint->getSPSZ(),
				"   ", "      ");
			stream << line << "\r\n";
		}
	}

	file.close();
	return true;
}