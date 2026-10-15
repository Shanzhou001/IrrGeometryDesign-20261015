#include "gspsscanner.h"
#include <QDebug>

//------------------------------------------------------------------------------
// ͳһSPSֵʽ
//  "1.00" -> "1", "1.0010" -> "1.001", "5001.000" -> "5001"
// Ǵַȥβոԭ
//------------------------------------------------------------------------------
static QString gNormalizeSPSKey(const QString &text) // add by shanzhou in 2026/10/15
{
	QString s = text.stripWhiteSpace();
	if(s.isEmpty()) return s;

	bool ok = false;
	double v = s.toDouble(&ok);
	if(!ok) return s;

	QString t = QString::number(v, 'f', 12);
	while(t.contains('.') && (t.endsWith('0') || t.endsWith('.')))
	{
		t.truncate(t.length() - 1);
	}
	if(t == "-0") t = "0";

	return t;
}

//------------------------------------------------------------------------------
// Xļṹ
//------------------------------------------------------------------------------
struct XParsedRecord
{
	Q_INT32 recNum;
	QString shotLineName;
	QString shotPointNum;

	Q_INT32 firstTrc;
	Q_INT32 lastTrc;
	Q_INT32 trcInc;

	QString rLineName;
	QString firstPt;
	QString lastPt;

	XParsedRecord()
	{
		recNum = 0;
		firstTrc = 0;
		lastTrc = 0;
		trcInc = 0;
	}
};

//------------------------------------------------------------------------------
// жX¼ǷϷ
//------------------------------------------------------------------------------
static bool gIsValidXRecord(const XParsedRecord &rec)
{
	if(rec.recNum <= 0) return false;
	if(rec.shotLineName.isEmpty()) return false;
	if(rec.shotPointNum.isEmpty()) return false;

	if(rec.firstTrc <= 0) return false;
	if(rec.lastTrc < rec.firstTrc) return false;
	if(rec.trcInc <= 0) return false;
	if((rec.lastTrc - rec.firstTrc) % rec.trcInc != 0) return false;

	if(rec.rLineName.isEmpty()) return false;
	if(rec.firstPt.isEmpty()) return false;
	if(rec.lastPt.isEmpty()) return false;

	Q_INT32 trcCount = (rec.lastTrc - rec.firstTrc) / rec.trcInc + 1;
	if(trcCount <= 0) return false;
	if(trcCount > 200000) return false;   // ֹݵ³ڴ

	return true;
}

//------------------------------------------------------------------------------
// ֻϹ̶пģ壨ݵǰԼ 3.31.x һļ
//------------------------------------------------------------------------------
static bool gParseXLegacyFixed(const QString &line, XParsedRecord &rec)
{
	char dest[32];
	const char *src = line;
	bool ok = false;

	rec.recNum = QString(qstrncpy(dest, src+7, 9)).stripWhiteSpace().toInt(&ok);
	if(!ok) return false;

	rec.shotLineName = gNormalizeSPSKey(QString(qstrncpy(dest, src+17, 11)).stripWhiteSpace());
	rec.shotPointNum = gNormalizeSPSKey(QString(qstrncpy(dest, src+27, 11)).stripWhiteSpace());

	rec.firstTrc = QString(qstrncpy(dest, src+38, 6)).stripWhiteSpace().toInt(&ok);
	if(!ok) return false;

	rec.lastTrc = QString(qstrncpy(dest, src+43, 6)).stripWhiteSpace().toInt(&ok);
	if(!ok) return false;

	rec.trcInc = QString(qstrncpy(dest, src+48, 2)).stripWhiteSpace().toInt(&ok);
	if(!ok) return false;

	rec.rLineName = gNormalizeSPSKey(QString(qstrncpy(dest, src+49, 11)).stripWhiteSpace());
	rec.firstPt   = gNormalizeSPSKey(QString(qstrncpy(dest, src+59, 11)).stripWhiteSpace());
	rec.lastPt    = gNormalizeSPSKey(QString(qstrncpy(dest, src+69, 11)).stripWhiteSpace());

	return gIsValidXRecord(rec);
}

//------------------------------------------------------------------------------
// ͳһX¼ڣǰֻϹ̶ģ
//------------------------------------------------------------------------------
static bool gParseXRecord(const QString &line, XParsedRecord &rec, QString &errMsg)
{
	XParsedRecord tmp;

	if(gParseXLegacyFixed(line, tmp))
	{
		rec = tmp;
		return true;
	}

	errMsg = QString::fromLocal8Bit("δƥ䵽ǰֵ֧X¼ģ");
	return false;
}

//--class GXFileScanner---------------------------------------------------------
GXFileScanner::GXFileScanner()
{
	mpProject = NULL;
}

//------------------------------------------------------------------------------
//  scanLineɨXļһм¼ݲЧжϣ
//  pShotĹϵӹϵ¼
//  lineΪXļһм¼pShotΪڹϵ
//
//  ˵úϹ̶ģʹá
//------------------------------------------------------------------------------
bool GXFileScanner::scanLine(const QString &line, GShot *pShot)
{
	if(pShot == NULL) return false;

	XParsedRecord rec;
	if(!gParseXLegacyFixed(line, rec))
	{
		qDebug() << "[GXFileScanner::scanLine] legacy parse failed, line =" << line;
		return false;
	}

	bool isNewLine = false;
	GShotXLine *pXLine = pShot->getXLineByName(rec.rLineName);
	if(pXLine == NULL)
	{
		pXLine = new GShotXLine(rec.rLineName);
		isNewLine = true;
	}

	GShotXArray *pXArray = pXLine->getXArrayByTrc(rec.firstTrc);
	if(pXArray == NULL)
	{
		Q_INT32 trcCount = (rec.lastTrc - rec.firstTrc) / rec.trcInc + 1;
		pXArray = new GShotXArray(rec.firstPt, rec.lastPt,
								  rec.firstTrc, rec.lastTrc,
								  rec.trcInc, trcCount);
		pXLine->append(pXArray);
	}

	if(isNewLine) pShot->append(pXLine);
	return true;
}

//------------------------------------------------------------------------------
//  scanɨXļݲЧжϣɨϢ
//  ShotSetڹϵ¼
//  fileNameΪXļspsMemoΪɨϢʾؼòΪʾϢ
//------------------------------------------------------------------------------
bool GXFileScanner::scan(const QString &fileName, GShotSet *pShotSet, QTextEdit *spsMemo)
{
	if(mpProject == NULL)
		mpProject = GProject::getCurrentProject();

	if(mpProject == NULL || pShotSet == NULL)
	{
		if(spsMemo != NULL)
			spsMemo->append(QString::fromLocal8Bit("//----Xļɨʧܣ̶ڼΪ----//"));
		return false;
	}

	QFile file(fileName);
	if(!file.open(IO_ReadOnly))
	{
		if(spsMemo != NULL)
			spsMemo->append(QString::fromLocal8Bit("//----޷򿪹ϵļ \"") +
							fileName + "\"----//");
		return false;
	}

	QTextStream stream(&file);
	QString line;
	Q_INT32 lineCount = 0;

	if(spsMemo != NULL)
		spsMemo->append(QString::fromLocal8Bit("//----ɨϵļ \"") +
						fileName + "\"----//");

	while(!stream.atEnd())
	{
		lineCount++;
		line = stream.readLine();

		if(line.isEmpty()) continue;
		if(line.stripWhiteSpace().isEmpty()) continue;
		if(QChar(line[0]).upper() == 'H') continue;

		XParsedRecord rec;
		QString errMsg;
		if(!gParseXRecord(line, rec, errMsg))
		{
			if(spsMemo != NULL)
			{
				spsMemo->append(QString::fromLocal8Bit("  : (") +
								QString::number(lineCount) +
								QString::fromLocal8Bit(")"));
				spsMemo->append("  " + line);
				spsMemo->append(QString::fromLocal8Bit("  ԭ: ") + errMsg);
				spsMemo->append(QString::fromLocal8Bit("//----ϵļɨ ʧ----//"));
			}

			file.close();
			return false;
		}

		GShot *pShot = pShotSet->getByRecNum(rec.recNum);
		bool isNewShot = false;

		if(pShot == NULL)
		{
			pShot = new GShot(rec.recNum, rec.shotLineName, rec.shotPointNum);
			pShot->setProject(mpProject);
			isNewShot = true;
		}

		GShotXLine *pXLine = pShot->getXLineByName(rec.rLineName);
		if(pXLine == NULL)
		{
			pXLine = new GShotXLine(rec.rLineName);
			pShot->append(pXLine);
		}

		GShotXArray *pXArray = pXLine->getXArrayByTrc(rec.firstTrc);
		if(pXArray == NULL)
		{
			Q_INT32 trcCount = (rec.lastTrc - rec.firstTrc) / rec.trcInc + 1;

			pXArray = new GShotXArray(rec.firstPt, rec.lastPt,
									  rec.firstTrc, rec.lastTrc,
									  rec.trcInc, trcCount);
			pXLine->append(pXArray);
		}

		if(isNewShot)
			pShotSet->append(pShot);
	}

	if(spsMemo != NULL)
		spsMemo->append(QString::fromLocal8Bit("//----ϵļɨ ɹ----//"));

	file.close();
	return true;
}

//--class GSFileScanner---------------------------------------------------------
GSFileScanner::GSFileScanner()
{
}

bool GSFileScanner::scan(const QString &fileName, GSLineSet *pSLineSet,
						 GShotSet *pShotSet, QTextEdit *spsMemo)
{
	QFile file( fileName );
	if( !file.open( IO_ReadOnly ) ) 
	{
		if( spsMemo != NULL )
			spsMemo->append( QString::fromLocal8Bit("\n//----޷ڵļ( ")+
							 fileName+" )----//" );
		return false;
	}
	QTextStream stream( &file );

	bool tsOk;
	QString line, lineName, pointNum;
	Q_INT32 lineCount = 0, errCount = 0;
	float spsX, spsY, spsZ, spsStatic, depth, base, tao;
	char dest[32];
	const char *src;
	GSLine *pSLine;
	GSPoint *pSPoint;

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("\n//----ɨڵļ \"")+
						 fileName+"\"----//" );
	bool isWellShot = true;
	while( !stream.atEnd() )
	{
		lineCount++;
		line = stream.readLine();

		if(line.isEmpty()) continue;
		if(line.stripWhiteSpace().isEmpty()) continue;

		if(QChar( line[0] ).upper() == 'H') continue;		//ע

		if(QChar( line[0] ).upper() != 'S') //淶
			goto ERROR_SLINE;
	
		if(QChar( line[26] ).upper() != 'E')
			isWellShot = false;
		else
			isWellShot = true;

		src = line;
		//----(1)ߺ----
		lineName = gNormalizeSPSKey(QString(qstrncpy(dest, src+1, 11)).stripWhiteSpace());
		//----(2)ڵ----
		pointNum = gNormalizeSPSKey(QString(qstrncpy(dest, src+11, 11)).stripWhiteSpace());
		//---(3)У----
		spsStatic = QString(qstrncpy(dest, src+26, 5)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) spsStatic = 0.0f;
		//----(4)ûм,˲ӲԼ,ûмĬΪ-10.0m----
		depth = QString(qstrncpy(dest, src+30, 5)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) depth = -10.0f;
		//----(5)TAOʱ, ӲԼ,ûмΪ0
		tao = QString(qstrncpy(dest, src+39, 3)).stripWhiteSpace().toFloat(&tsOk);
		//----(5)SPSX----
		spsX = QString(qstrncpy(dest, src+46, 10)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(6)SPSY----
		spsY = QString(qstrncpy(dest, src+55, 11)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(7)SPSZ----
		spsZ = QString(qstrncpy(dest, src+65, 7)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(8)SPS׼----
		base = QString(qstrncpy(dest, src+34, 5)).stripWhiteSpace().toFloat(&tsOk);
		if(!tsOk) base = depth > 0 ? spsZ-depth : spsZ;

		pSLine = pSLineSet->getByLineName(lineName);
		if(pSLine == NULL)
		{
			pSLine = new GSLine(lineName);
			pSLineSet->append(pSLine);
		}

		pSPoint = pSLine->getByPointNum(pointNum);
		if(pSPoint == NULL)
		{
			pSPoint = new GSPoint(lineName, pointNum, spsX, spsY, spsZ, spsStatic, depth, base, tao);
			pSPoint->setWellShotFlag(isWellShot);
			int recNum = pShotSet->getRecNumByLineNamePointName(lineName, pointNum);// ΪÿһڵӼ¼Ϣ [3/16/2023 Liu Wei]
			pSPoint->setRecNum(recNum);// ΪÿһڵӼ¼Ϣ [3/16/2023 Liu Wei]
			pSLine->append(pSPoint);
		}

		continue;	//ȷɨ

		ERROR_SLINE:
		{
			if( spsMemo != NULL )
			{
				spsMemo->append( QString::fromLocal8Bit("  : ("+QString::number(lineCount)+")") );
				spsMemo->append( line );
			}
			errCount++;
		}
	}

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("//----ڵļɨ ("+
						 QString::number(errCount)+")----//") );

	return true;
}
//--class GRFileScanner---------------------------------------------------------
GRFileScanner::GRFileScanner()
{
}

bool GRFileScanner::scan(const QString &fileName, GRLineSet *pRLineSet, QTextEdit *spsMemo,
						 QProgressBar *progressBar )
{
	//	progressBar->setTotalSteps(100);
	progressBar->setValue(100);
	QFile file( fileName );
	if( !file.open( IO_ReadOnly ) ) 
	{
		if( spsMemo != NULL )
			spsMemo->append( QString::fromLocal8Bit("\n//----޷򿪼첨ļ( ")+
							 fileName+" )----//" );
		return false;
	}
	QTextStream stream( &file );
	long long srcLen = file.size();
	long long srcOffset = 0;
	bool tsOk;
	QString line, lineName, pointNum;
	Q_INT32 lineCount = 0, errCount = 0;
	float spsX, spsY, spsZ, spsStatic, base;
	char dest[32];
	const char *src;
	GRLine *pRLine;
	GRPoint *pRPoint;

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("\n//----ɨ첨ļ \"")+
						 fileName+"\"----//" );

	while( !stream.atEnd() )
	{
		srcOffset = file.at();
//		if(progressBar != NULL) progressBar->setProgress( srcOffset*100/srcLen );
		if(progressBar != NULL) progressBar->setValue( srcOffset*100/srcLen );
		lineCount++;
		line = stream.readLine();

		if(line.isEmpty()) continue;
		if(line.stripWhiteSpace().isEmpty()) continue;

		if( QChar( line[0] ).upper() == 'H' ) continue;		//ע

		if( QChar( line[0] ).upper() != 'R' ) //淶
			goto ERROR_SLINE;

		src = line;
		//----(1)첨ߺ----
		lineName = gNormalizeSPSKey(QString(qstrncpy(dest, src+1, 11)).stripWhiteSpace());
		//----(2)----
		pointNum = gNormalizeSPSKey(QString(qstrncpy(dest, src+11, 11)).stripWhiteSpace());
		//----(3)Ұ⾲У----
		spsStatic = QString(qstrncpy(dest, src+26, 5)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) spsStatic = 0.0f;
		//----(4)SPSX----
		spsX = QString(qstrncpy(dest, src+46, 10)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(5)SPSY----
		spsY = QString(qstrncpy(dest, src+55, 11)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(6)SPSZ----
		spsZ = QString(qstrncpy(dest, src+65, 7)).stripWhiteSpace().toFloat(&tsOk);
		if( !tsOk ) goto ERROR_SLINE;
		//----(7)SPS׼----
		base = QString(qstrncpy(dest, src+34, 5)).stripWhiteSpace().toFloat(&tsOk);
		if(!tsOk) base = spsZ;

		pRLine = pRLineSet->getByLineName(lineName);
		if(pRLine == NULL)
		{
			pRLine = new GRLine(lineName);
			pRLineSet->append(pRLine);
		}

		pRPoint = pRLine->getByPointNum(pointNum);
		if(pRPoint == NULL)
		{
			pRPoint = new GRPoint(lineName, pointNum, spsX, spsY, spsZ, spsStatic, base);
			pRLine->append(pRPoint);
		}

		continue;	//ȷɨ

		ERROR_SLINE:
		{
			if( spsMemo != NULL )
			{
				spsMemo->append( QString::fromLocal8Bit("  : ("+QString::number(lineCount)+")") );
				spsMemo->append( line );
			}
			errCount++;
		}
	}

	if( spsMemo != NULL )
		spsMemo->append( QString::fromLocal8Bit("//----첨ļɨ ("+
						 QString::number(errCount)+")----//") );
//    progressBar->reset();
	progressBar->setValue(100);
	return true;
}