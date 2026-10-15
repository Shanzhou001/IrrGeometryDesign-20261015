#include "gsegyfilescanner.h"

GSEGYFileScanner::GSEGYFileScanner()
{
}

QString GSEGYFileScanner::getShotInfo(GShot *pShot)
{
	return QString::fromLocal8Bit( "  ("+QString::number(pShot->getRecNum())+")"+
			" -- 道数 "+QString::number(pShot->getTrcCount())+
			" -- 采样点数 "+QString::number(pShot->getCPCount())+
			" -- 采样间隔 "+QString::number(pShot->getCPInv()) );
}

QString GSEGYFileScanner::getErrorInfo(int recNum, GErrorType type)
{
	switch(type)
	{
	case gErrorTrcCount	:	return QString::fromLocal8Bit("  警告 ("+QString::number(recNum)+
							") SEG-Y文件中道数与SPS文件记载不一致" );
	case gNoSPSInfo		:	return QString::fromLocal8Bit("  警告 ("+QString::number(recNum)+
							") 没有对应的SPS信息" );
	case gNoXInfo		:	return QString::fromLocal8Bit("  警告 ("+QString::number(recNum)+
							") 缺少与SEG-Y文件对应的关系信息" );
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

	//----(1)打开文件,判断文件是否存在----
	QFile srcFile(fileName);
	if(!srcFile.open(IO_ReadOnly)) return false;

	QDataStream srcStream(&srcFile);
	srcStream.setByteOrder(QDataStream::BigEndian);

	//----(2)判断源文件大小,移动文件指针,初始化进度杆----
//	QIODevice::Offset srcLen = srcFile.size();
	long long srcLen = srcFile.size();
	if(srcLen < 3600) return false;	//如果小于3600字节认为是非法SEGY文件
//	QIODevice::Offset srcOffset = 3600;
	long long srcOffset = 3600;

//	progressBar->setTotalSteps(100);
	progressBar->setValue(100);

	//----(3)循环扫描数据文件----
	bool isScanDataTrc;					//是否需要等待扫描数据道的出现
	int errCount = 0;					//警告错误计数器
	Q_INT16 trcType;					//道类型
	Q_INT16 cpCount, cpInv;				//采样点数、采样间隔
	Q_INT32 recNum, preRecNum = -1;		//野外记录号、野外记录号备份
	Q_INT32 trcNum, trcCount = 0;		//道号、总道数
	long long shotPos;	//炮起始位置、数据道起始位置
	long long dataPos;	//炮起始位置、数据道起始位置
	QIODevice::Offset tbPos = 0, taoPos = 0;	//TB、TAO文件位置
	GShot *pShot = NULL;				//炮关系指针
	GShotXLine *pXLine = NULL;			//关系线
	GShotXArray *pXArray = NULL;		//关系排列	
	int iL, iA;							//关系线、排列序号变量
	QString infoLine, errLine;			//信息提示字符串
	//Cheng Du old
	//sgyMemo->append( QString::fromLocal8Bit("//----扫描SEG-Y文件 \""+
	//				 fileName+"----//") );
	sgyMemo->append( QString::fromLocal8Bit("\n//----扫描SEG-Y文件 \"") + fileName+"\"----//" );//Lena add 1009
	
	while(srcOffset < srcLen)
	{
//		if(progressBar != NULL) progressBar->setProgress( srcOffset*100/srcLen );
		if(progressBar != NULL) progressBar->setValue( srcOffset*100/srcLen );

		srcFile.at(srcOffset+8);
		srcStream >> recNum;			//读取野外记录号
		//for debug
//		recNum-=30000;
		//
		srcStream >> trcNum;			//读取道号
		srcFile.at(srcOffset+28);		
		srcStream >> trcType;			//读取道类型（数据道/非数据道）
        if(trcNum<0)
			trcType = 4;
		//野外文件号有变化，说明新的一炮开始
		if(preRecNum != recNum)			
		{
			//----(3.1)填充pShot->FileOffset字段----
			if(pShot != NULL)			//如果前一炮指针不为NULL，填充有效信息
			{
			/*
			if(pShot->getTrcCount() != trcCount) //炮道数检查，道数异常放弃该炮
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
			//----(3.2)获取当前炮，重新设置扫描变量----
			pShot = pShotSet->getByRecNum(recNum);
			if(pShot == NULL) //在关系文件中没有记载
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

			if(trcType <= 3) //没有辅助道
			{
				iL = 0;		iA = 0;
				pXLine = pShot->get(iL);
				pXArray = pXLine->get(iA);
				dataPos = srcOffset;	//记录数据道位置
				isScanDataTrc = false;	//不需要再扫描数据道位置
			}
			else isScanDataTrc = true;	
			
			trcCount = 0;				//道计数器清0
			shotPos = srcOffset;		//记录炮起始位置
		}
		
		if(pShot == NULL) //如果该炮没有记录跳过
		{
			srcOffset += 240+(long long)cpCount*4;
			continue;
		}

		if(isScanDataTrc) //含有辅助道,等待数据道的出现
		{
			if(trcType <= 3)
			{
				iL = 0;		iA = 0;
				pXLine = pShot->get(iL);
				pXArray = pXLine->get(iA);
				dataPos = srcOffset;	//记录数据道位置
				isScanDataTrc = false;
			}
			else
			{
				if(trcType == 4) tbPos = srcOffset;		//TB道
				if(trcType == 5) taoPos = srcOffset;	//TAO道
				srcOffset += 240+(long long)cpCount*4;
				continue;
			}
		}

		if(pXArray == NULL) //关系文件中缺少炮排列信息
		{
			if(sgyMemo != NULL) sgyMemo->append(getErrorInfo(recNum, gNoXInfo));
			srcOffset += 240+(long long)cpCount*4;
			errCount++;
			continue;
		}
		if(pXArray->getFirstTrc() == trcNum) //记录排列第一道在文件中的位置
		{
			pXArray->setFilePos(srcOffset);
		}
		if( pXArray->getLastTrc() == trcNum ) //如果一个排列结束,更新pXArray
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

		srcOffset += 240+(long long)cpCount*4;			//移动文件指针到下一道
      if(trcNum>0)
	  {
		//  pShot->setTrcNumByIndex(trcCount,trcNum);
		  trcCount++;							//道计数器增加
	  }
	}

	if(pShot != NULL)			//如果最后一炮数据不为NULL，填充有效信息
	{
		if(pShot->getTrcCount() != trcCount) //炮道数检查，道数异常放弃该炮
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
	sgyMemo->append( QString::fromLocal8Bit("//----SEG-Y文件扫描结束 警告错误("+
					 QString::number(errCount)+")----//") );

	//----(4)关闭文件----
//	progressBar->reset();
	progressBar->setValue(100);
	srcFile.close();
	return true;
}

bool GSEGYFileScanner::scanForShotRecorderRange(const QString &fileName, 
												QProgressBar *progressBar, 
			                           QTextEdit *sgyMemo )
{		
	GProject *pProject = mpProject;
	if(pProject == NULL) return false;
	
	//----(1)打开文件,判断文件是否存在----
	QFile srcFile(fileName);
	if(!srcFile.open(IO_ReadOnly)) return false;

	QDataStream srcStream(&srcFile);
	srcStream.setByteOrder(QDataStream::BigEndian);

	//----(2)判断源文件大小,移动文件指针,初始化进度杆----
	long long srcLen = srcFile.size();
	if(srcLen < 3600) return false;	//如果小于3600字节认为是非法SEGY文件
	long long srcOffset = 3600;
//	progressBar->setTotalSteps(100);
	progressBar->setValue(100);

	Q_INT16 cpCount, cpInv;				//采样点数、采样间隔
	Q_INT32 recNum, preRecNum = -1;		//野外记录号、野外记录号备份
	Q_INT32 trcNum, trcCount = 0;		//道号、总道数
	int shotCount = 0;
	int minRecorderNum = 1000000000;
	int maxRecorderNum = -1000000000;
	sgyMemo->append( QString::fromLocal8Bit("\n//----扫描SEG-Y文件 \"") + fileName+"\"----//" );//Lena add 1009

	while(srcOffset < srcLen)
	{
		srcFile.at(srcOffset+8);
		srcStream >> recNum;			//读取野外记录号
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
		srcStream >> recNum;			//读取野外记录号
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
		sgyMemo->append( QString::fromLocal8Bit("//----SEG-Y文件扫描1结束 //")+"\n"+
			        QString::fromLocal8Bit("共")+shotNumString 
					 +QString::fromLocal8Bit("炮")+"----//") ;

	//----(4)关闭文件----
//	progressBar->reset();
	progressBar->setValue(100);
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
// 	//----(1)打开文件,判断文件是否存在----
// 	QFile srcFile(fileName);
// 	if(!srcFile.open(IO_ReadOnly)) return false;
// 
// 	QDataStream srcStream(&srcFile);
// 	srcStream.setByteOrder(QDataStream::BigEndian);
// 
// 	//----(2)判断源文件大小,移动文件指针,初始化进度杆----
// 	QIODevice::Offset srcLen = srcFile.size();
// 	if(srcLen < 3600) return false;	//如果小于3600字节认为是非法SEGY文件
// 	QIODevice::Offset srcOffset = 3600;
// 
// 	while(srcOffset < srcLen)
// 	{
// 		srcFile.at(srcOffset+8);
// 	}
}