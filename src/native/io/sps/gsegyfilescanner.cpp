#include "gsegyfilescanner.h"

GSEGYFileScanner::GSEGYFileScanner()
{
}

QString GSEGYFileScanner::getShotInfo(GShot *pShot)
{
	return QString::fromLocal8Bit( "  ("+QString::number(pShot->getRecNum())+")"+
			" --  "+QString::number(pShot->getTrcCount())+
			" --  "+QString::number(pShot->getCPCount())+
			" --  "+QString::number(pShot->getCPInv()) );
}

QString GSEGYFileScanner::getErrorInfo(int recNum, GErrorType type)
{
	switch(type)
	{
    case gErrorTrcCount: return QString::fromLocal8Bit("记录(")+QString::number(recNum)+
        QString::fromLocal8Bit(")的SEG-Y道数与SPS接收道数不一致");
    case gNoSPSInfo: return QString::fromLocal8Bit("记录(")+QString::number(recNum)+
        QString::fromLocal8Bit(")没有对应的SPS信息");
    case gNoXInfo: return QString::fromLocal8Bit("记录(")+QString::number(recNum)+
        QString::fromLocal8Bit(")缺少对应的炮检关系信息");
	}

	return QString::null;
}

bool GSEGYFileScanner::scan(const QString &fileName, QProgressBar *progressBar,
							QTextEdit *sgyMemo)
{
	GProject *pProject = mpProject;
	if(pProject == NULL) return false;
	GShotSet *pShotSet = pProject->getShotSet();
	if(pShotSet == NULL) return false;

	//----(1)ļ,жļǷ----
	QFile srcFile(fileName);
	if(!srcFile.open(IO_ReadOnly)) return false;

	QDataStream srcStream(&srcFile);
	srcStream.setByteOrder(QDataStream::BigEndian);

	//----(2)жԴļС,ƶļָ,ʼȸ----
//	QIODevice::Offset srcLen = srcFile.size();
	long long srcLen = srcFile.size();
	if(srcLen < 3600) return false;	//С3600ֽΪǷǷSEGYļ
//	QIODevice::Offset srcOffset = 3600;
	long long srcOffset = 3600;

//	progressBar->setTotalSteps(100);
	if (progressBar != NULL) progressBar->setValue(100);

	//----(3)ѭɨļ----
	bool isScanDataTrc;					//ǷҪȴɨݵĳ
	int errCount = 0;					//
	Q_INT16 trcType;					//
	Q_INT16 cpCount, cpInv;				//
	Q_INT32 recNum, preRecNum = -1;		//Ұ¼šҰ¼ű
	Q_INT32 trcNum, trcCount = 0;		//šܵ
	long long shotPos;	//ʼλáݵʼλ
	long long dataPos;	//ʼλáݵʼλ
	QIODevice::Offset tbPos = 0, taoPos = 0;	//TBTAOļλ
	GShot *pShot = NULL;				//ڹϵָ
	GShotXLine *pXLine = NULL;			//ϵ
	GShotXArray *pXArray = NULL;		//ϵ	
	int iL, iA;							//ϵߡű
	QString infoLine, errLine;			//Ϣʾַ
	//Cheng Du old
	//sgyMemo->append( QString::fromLocal8Bit("//----ɨSEG-Yļ \""+
	//				 fileName+"----//") );
    sgyMemo->append(QString::fromLocal8Bit("\n//----扫描SEG-Y文件：")+fileName+"----//");
	
	while(srcOffset < srcLen)
	{
//		if(progressBar != NULL) progressBar->setProgress( srcOffset*100/srcLen );
		if(progressBar != NULL) progressBar->setValue( srcOffset*100/srcLen );

		srcFile.at(srcOffset+8);
		srcStream >> recNum;			//ȡҰ¼
		//for debug
//		recNum-=30000;
		//
		srcStream >> trcNum;			//ȡ
		srcFile.at(srcOffset+28);		
		srcStream >> trcType;			//ȡͣݵ/ݵ
        if(trcNum<0)
			trcType = 4;
		//Ұļб仯˵µһڿʼ
		if(preRecNum != recNum)			
		{
			//----(3.1)pShot->FileOffsetֶ----
			if(pShot != NULL)			//ǰһָ벻ΪNULLЧϢ
			{
			/*
			if(pShot->getTrcCount() != trcCount) //ڵ飬쳣
				{
					if(sgyMemo != NULL) sgyMemo->append(getErrorInfo(recNum, gErrorTrcCount));
					errCount++;
				}
				else 
				{
					pShot->setFilePos(shotPos);
					pShot->setDataPos(dataPos);
					pShot->setTBPos(tbPos);
					pShot->setTAOPos(taoPos);
					pShot->setCPCount(cpCount);
					pShot->setCPInv(cpInv);
					pShot->setFileIndex(pProject->getSEGYFileCount());
					sgyMemo->append(getShotInfo(pShot));
				}
				*/
				pShot->setFilePos(shotPos);
				pShot->setDataPos(dataPos);
				pShot->setTBPos(tbPos);
				pShot->setTAOPos(taoPos);
				pShot->setCPCount(cpCount);
				pShot->setCPInv(cpInv);
				pShot->setFileIndex(pProject->getSEGYFileCount());
					sgyMemo->append(getShotInfo(pShot));
			}
			//----(3.2)ȡǰڣɨ----
			pShot = pShotSet->getByRecNum(recNum);
			if(pShot == NULL) //ڹϵļûм
			{
				if(sgyMemo != NULL) sgyMemo->append(getErrorInfo(recNum, gNoSPSInfo));
				errCount++;
				preRecNum = recNum; 
				srcFile.at(srcOffset+114);
				srcStream >> cpCount;
				srcOffset += 240+(long long)cpCount*4;
				continue;
			}
				
			preRecNum = recNum;
			srcFile.at(srcOffset+114);
			srcStream >> cpCount;
			srcStream >> cpInv;

			if(trcType <= 3) //ûи
			{
				iL = 0;		iA = 0;
				pXLine = pShot->get(iL);
				pXArray = pXLine->get(iA);
				dataPos = srcOffset;	//¼ݵλ
				isScanDataTrc = false;	//Ҫɨݵλ
			}
			else isScanDataTrc = true;	
			
			trcCount = 0;				//0
			shotPos = srcOffset;		//¼ʼλ
		}
		
		if(pShot == NULL) //ûм¼
		{
			srcOffset += 240+(long long)cpCount*4;
			continue;
		}

		if(isScanDataTrc) //и,ȴݵĳ
		{
			if(trcType <= 3)
			{
				iL = 0;		iA = 0;
				pXLine = pShot->get(iL);
				pXArray = pXLine->get(iA);
				dataPos = srcOffset;	//¼ݵλ
				isScanDataTrc = false;
			}
			else
			{
				if(trcType == 4) tbPos = srcOffset;		//TB
				if(trcType == 5) taoPos = srcOffset;	//TAO
				srcOffset += 240+(long long)cpCount*4;
				continue;
			}
		}

		if(pXArray == NULL) //ϵļȱϢ
		{
			if(sgyMemo != NULL) sgyMemo->append(getErrorInfo(recNum, gNoXInfo));
			srcOffset += 240+(long long)cpCount*4;
			errCount++;
			continue;
		}
		if(pXArray->getFirstTrc() == trcNum) //¼еһļеλ
		{
			pXArray->setFilePos(srcOffset);
		}
		if( pXArray->getLastTrc() == trcNum ) //һн,pXArray
		{
			if(iA < pShot->get(iL)->count()-1) iA++;
			else 
			{
				iL++;	iA = 0;
				pXLine = pShot->get(iL);
			}
			if(pXLine != NULL) pXArray = pXLine->get(iA);
			else pXArray = NULL;
		}

		srcOffset += 240+(long long)cpCount*4;			//ƶļָ뵽һ
      if(trcNum>0)
	  {
		//  pShot->setTrcNumByIndex(trcCount,trcNum);
		  trcCount++;							//
	  }
	}

	if(pShot != NULL)			//һݲΪNULLЧϢ
	{
		if(pShot->getTrcCount() != trcCount) //ڵ飬쳣
		{
			if(sgyMemo != NULL) sgyMemo->append(getErrorInfo(recNum, gErrorTrcCount));
			errCount++;
		}
		else 
		{
			pShot->setFilePos(shotPos);
			pShot->setDataPos(dataPos);
			pShot->setTBPos(tbPos);
			pShot->setTAOPos(taoPos);
			pShot->setCPCount(cpCount);
			pShot->setCPInv(cpInv);
			pShot->setFileIndex(pProject->getSEGYFileCount());
			sgyMemo->append(getShotInfo(pShot));
		}
	}
    sgyMemo->append(QString::fromLocal8Bit("//----SEG-Y扫描完成，错误数(")+
        QString::number(errCount)+")----//");

	//----(4)رļ----
//	progressBar->reset();
	if (progressBar != NULL) progressBar->setValue(100);
	srcFile.close();
	return true;
}

bool GSEGYFileScanner::scanForShotRecorderRange(const QString &fileName, 
												QProgressBar *progressBar, 
			                           QTextEdit *sgyMemo )
{		
	GProject *pProject = mpProject;
	if(pProject == NULL) return false;
	
	//----(1)ļ,жļǷ----
	QFile srcFile(fileName);
	if(!srcFile.open(IO_ReadOnly)) return false;

	QDataStream srcStream(&srcFile);
	srcStream.setByteOrder(QDataStream::BigEndian);

	//----(2)жԴļС,ƶļָ,ʼȸ----
	long long srcLen = srcFile.size();
	if(srcLen < 3600) return false;	//С3600ֽΪǷǷSEGYļ
	long long srcOffset = 3600;
//	progressBar->setTotalSteps(100);
	if (progressBar != NULL) progressBar->setValue(100);

	Q_INT16 cpCount, cpInv;				//
	Q_INT32 recNum, preRecNum = -1;		//Ұ¼šҰ¼ű
	Q_INT32 trcNum, trcCount = 0;		//šܵ
	int shotCount = 0;
	int minRecorderNum = 1000000000;
	int maxRecorderNum = -1000000000;
    sgyMemo->append(QString::fromLocal8Bit("\n//----扫描SEG-Y文件：")+fileName+"----//");

	while(srcOffset < srcLen)
	{
		srcFile.at(srcOffset+8);
		srcStream >> recNum;			//ȡҰ¼
		srcFile.at(srcOffset+114);
			srcStream >> cpCount;
		srcOffset += (240+(long long)cpCount*4);
		if(preRecNum==recNum)
		 trcCount++;
		if(preRecNum!=recNum)
		{
			shotCount++;
			if(shotCount>1)
				break;
		}
		preRecNum = recNum;
	}
	shotCount = 0;
    srcOffset = 3600;
	int moveStep = trcCount*2.0/3;
	while(srcOffset < srcLen)
	{
//		if(progressBar != NULL) progressBar->setProgress( srcOffset*100/srcLen );
		if(progressBar != NULL) progressBar->setValue( srcOffset*100/srcLen );

		srcFile.at(srcOffset+8);
		srcStream >> recNum;			//ȡҰ¼
		srcFile.at(srcOffset+114);
			srcStream >> cpCount;
		srcOffset += ((long long)moveStep*(240+(long long)cpCount*4));
		if(recNum>maxRecorderNum)
			maxRecorderNum = recNum;
		if(recNum<minRecorderNum)
			minRecorderNum = recNum;
		if(preRecNum!=recNum)
			shotCount++;
		preRecNum = recNum;
	}
	QString shotNumString = QString::number(shotCount);
	pProject->setRecorderNum(minRecorderNum,maxRecorderNum);
    sgyMemo->append(QString::fromLocal8Bit("//----SEG-Y扫描结果----//\n")+
        shotNumString+
        "----//");

	//----(4)رļ----
//	progressBar->reset();
	if (progressBar != NULL) progressBar->setValue(100);
	srcFile.close();
	return true;
}

void GSEGYFileScanner::ExportShotSgy(const QString &fileName)//zhaohu
{
// 	GProject *pProject = mpProject;
// 	if(pProject == NULL) return false;
// 	GShotSet *pShotSet = pProject->getShotSet();
// 	if(pShotSet == NULL) return false;
// 
// 	//----(1)ļ,жļǷ----
// 	QFile srcFile(fileName);
// 	if(!srcFile.open(IO_ReadOnly)) return false;
// 
// 	QDataStream srcStream(&srcFile);
// 	srcStream.setByteOrder(QDataStream::BigEndian);
// 
// 	//----(2)жԴļС,ƶļָ,ʼȸ----
// 	QIODevice::Offset srcLen = srcFile.size();
// 	if(srcLen < 3600) return false;	//С3600ֽΪǷǷSEGYļ
// 	QIODevice::Offset srcOffset = 3600;
// 
// 	while(srcOffset < srcLen)
// 	{
// 		srcFile.at(srcOffset+8);
// 	}
}
