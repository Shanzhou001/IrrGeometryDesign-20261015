#include "analyzebinform.h"
#include "gutils.h"
#include "qmessagebox.h"
#include "math.h"
#include "trosedividedlg.h"
#include "tqfdividedlg.h"
#include "tchangeshowareadlg.h"
#include "gproject.h"
#include "tgeometryimfordlg.h"

analyzebinform::analyzebinform(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QLabel *coordinate = new QLabel( statusBar() );
	coordinate->setMinimumWidth( 200 );
	statusBar()->addWidget( coordinate, 0, true );

	mpBinDoc = new analyzebindoc();
	
	mpBinView = new analyzebinview(this);
	mpBinView->setlabel(coordinate);
	mpBinView->setDoc(mpBinDoc);

	mpBinView->refresh();	
	setCentralWidget(mpBinView);
	in_loop = false;

	CMPFoldVSBins=NULL;
	CMPOffsetVSTraces=NULL;
	CMPOffsetVSFoldBins=NULL;
	CMPAzimuthVSTraces=NULL;
	CMPmeiguiVStraces=NULL;
	CMPBinfold=NULL;
	m_dataNonuniform=NULL;
	m_dataAngle=NULL;
	m_dataFoot=NULL;
	m_dataCSDFPercent=NULL;
	
}

analyzebinform::~analyzebinform()
{
	if(mpBinDoc!=NULL) delete mpBinDoc;
	if(mpBinView!=NULL) delete mpBinView;
	
	if(CMPBinfold!=NULL) gFree2(CMPBinfold);
	if(CMPFoldVSBins!=NULL) gFree1(CMPFoldVSBins);
	if(CMPOffsetVSTraces!=NULL) gFree1(CMPOffsetVSTraces);
	if(CMPOffsetVSFoldBins!=NULL) gFree2(CMPOffsetVSFoldBins);
	if(CMPmeiguiVStraces!=NULL) gFree2(CMPmeiguiVStraces);
	if(CMPAzimuthVSTraces!=NULL) gFree1(CMPAzimuthVSTraces);
	if(m_dataNonuniform!=NULL) gFree2(m_dataNonuniform);
	if(m_dataAngle!=NULL) gFree2(m_dataAngle);
	if(m_dataFoot!=NULL) gFree2(m_dataFoot);
	if(m_dataCSDFPercent!=NULL) gFree2(m_dataCSDFPercent);
}


void analyzebinform::on_Fold_Bins_triggered()//覆盖次数-面元数量
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		mpBinDoc->foldVSbins();
		mpBinView->drawworkareaFoldVSBIns(mpBinView->xcoordworkfold,mpBinView->ycoordworkfold);

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

void analyzebinform::on_Offset_Traces_triggered()//炮检距-炮检对
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		mpBinDoc->offsetVStraces();
		mpBinView->drawworkareaOffsetVSTraces(mpBinView->xcoordworkoffset,mpBinView->ycoordworkoffset);

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

void analyzebinform::on_Azimuth_Traces_triggered()//方位角-炮检对
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		mpBinDoc->azimuthVStraces();
		mpBinView->drawworkareaAzimuthVSTraces(mpBinView->ycoordworkAzimuth);	

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

void analyzebinform::on_Rose_Traces_triggered()//玫瑰花图
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		mpBinDoc->roseVStraces();
		mpBinView->drawworkareaRoseVSTraces(lxRose,lyRose,CMPdlgmaxoffset);

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

////显示玫瑰图，随意输入分割次数
//void analyzebinform::on_actionRoseDivide_triggered()
//{
//	if(mpBinDoc->getDataValueworkarea()==true)	
//	{
//		TQFDividedlg *dlg=new TQFDividedlg(this);
//		dlg->setAngleDivNum(lyRose);
//		dlg->setOffsetDivNum(lxRose);
//		dlg->setMaxOffsetQf(CMPdlgmaxoffset);
//		dlg->init();
//		if(dlg->exec())
//		{
//			lxRose=dlg->getOffsetDivNum();//炮检距分成几段
//			lyRose=dlg->getAngleDivNum();//方位角分成几段
//			CMPdlgmaxoffset=dlg->getMaxOffsetQf();//最大炮检距			
//		delete dlg;	
//		int xline=CMPxbinnumber;
//		int yline=CMPybinnumber;
//		float kedu=CMPdlgmaxoffset;
//		int ii=1;
//		float a=0;
//		while(kedu/ii>1)//选择一个合适的刻度
//		{
//			a=kedu/ii;
//			ii=ii*10;	
//		}
//		kedu=ii/100;
//		a=CMPdlgmaxoffset/kedu+1;
//		float offset=kedu*a;
//
//		int scaleoffsetRose=offset/lxRose;//偏移距lxRose等份分组
//		int scaleangleRose=360/lyRose;
//		//玫瑰花图-炮检对
//		if(CMPmeiguiVStraces==NULL) gFree2(CMPmeiguiVStraces)	;
//		gAlloc2(CMPmeiguiVStraces,lxRose,lyRose);
//		for(int i=0;i<lxRose;i++)
//		{
//			for(int j=0;j<lyRose;j++)
//			{
//				CMPmeiguiVStraces[i][j]=0;//付初值
//			}
//		}
//		for(int i=0;i<xline;i++)
//		{
//			for(int j=0;j<yline;j++)
//			{
//				for(int k=0;k<azimutharray[i][j].count();k++)
//				{
//					int kk=0;int hh=0;
//					if(azimutharray[i][j][k]!=-1) 
//					{
//						hh=offsetarray[i][j][k]/scaleoffsetRose;//炮检距
//						if(hh>=lxRose) 
//							hh = lxRose-1;//modify by psl
//						kk=azimutharray[i][j][k]/scaleangleRose;//方位角
//						if(kk<0)
//							kk = 0;
//						if(kk>lyRose-1)
//							kk = lyRose-1;//modify by psl
//						CMPmeiguiVStraces[hh][kk]++;
//					}
//				}	
//			}
//		}
//		//玫瑰花图-炮检对
//		mpBinDoc->setDataMeiguiVSTraces(CMPmeiguiVStraces);
//		mpBinView->refresh();
//		mpBinDoc->roseVStraces();
//		mpBinView->drawworkareaRoseVSTraces(lxRose,lyRose,CMPdlgmaxoffset);
//		}
//	}
//}
////综合质量因子统计重新划分
//void analyzebinform::on_actionQFDivide_triggered()
//{
//	if(mpBinDoc->getDataValueworkarea()==true)	
//	{
//		TQFDividedlg *dlg=new TQFDividedlg(this);
//		dlg->setAngleDivNum(lyCSQF);
//		dlg->setOffsetDivNum(lxCSQF);
//		dlg->setMaxOffsetQf(maxoffsetCSQF);
//		dlg->init();
//		if(dlg->exec())
//		{
//			lxCSQF=dlg->getOffsetDivNum();//炮检距分成几段
//			lyCSQF=dlg->getAngleDivNum();//方位角分成几段
//			maxoffsetCSQF=dlg->getMaxOffsetQf();//统计的最大炮检距
//			delete dlg;	
//			CSDFactorfun(offsetarray,azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold,CMPdlgmaxoffset);
//			mpBinView->drawoffsetNonuniformfactor(m_dataCSDFPercent,CMPxbinnumber,CMPybinnumber,m_maxCSDFPfactor,m_minCSDFPfactor,1,1,"QF Factor",m_CSQFfactor);
//			
//		}
//	}
//		
//}
////改变炮检距、方位角非均匀性系数，采集脚印显示范围
//void analyzebinform::on_actionChangeShowarea_triggered()
//{
//	if(bShowOffset==true||bShowangle==true||bShowFoot==true) 
//	{
//		if(mpBinDoc->getDataValueworkarea()==true)	
//		{
//			float maxv,minv;
//			if(bShowOffset==true)
//			{
//				maxv=m_maxNonuniformfactor;
//				minv=m_minNonuniformfactor;
//			}
//			else if(bShowangle==true)
//			{
//				maxv=m_maxAnglefactor;
//				minv=m_minAnglefactor;
//			}
//			else if(bShowFoot==true)
//			{
//				maxv=m_maxFootfactor;
//				minv=m_minFootfactor;
//			}
//			TChangeShowareadlg *dlg=new TChangeShowareadlg(this);
//			dlg->init(maxv,minv);
//			if(dlg->exec())
//			{
//				maxv=dlg->getMaxShowValue();
//				minv=dlg->getMinShowValue();
//				delete dlg;
//				if(bShowOffset==true)
//				{
//					m_maxNonuniformfactor=maxv;
//					m_minNonuniformfactor=minv;
//					on_actionOffsetFactor_triggered();
//				}
//				else if(bShowangle==true)
//				{
//					m_maxAnglefactor=maxv;
//					m_minAnglefactor=minv;
//					on_actionAzimuthFactor_triggered();
//				}
//				else if(bShowFoot==true)
//				{
//					m_maxFootfactor=maxv;
//					m_minFootfactor=minv;
//					on_actionFootFactor_triggered();
//				}
//			}
//		}
//	}
//}

void analyzebinform::on_Offset_Fold_Bins_triggered()//柱状图
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		mpBinDoc->offsetVSfoldbins();
		mpBinView->drawworkareaOffsetVSFoldBins(mpBinView->ycoordworkOffsetVSFoldBins,lxzhu,maxzhufold);

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

void analyzebinform::on_Bin_Fold_triggered()//覆盖次数
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		//mpBinView->drawworkareaBinfold(mpBinView->Binfold,mpBinView->xbinnumber,mpBinView->ybinnumber);
		
		mpBinView->drawWorkAreaBinFold(mpBinView->Binfold,mpBinView->xbinnumber,mpBinView->ybinnumber);
		mpBinDoc->valuebinfold=true;
		mpBinDoc->valueoffsetVSfoldbins=false;
		mpBinDoc->valuefoldVSbins=false;
		mpBinDoc->valueoffsetVStraces=false;	
		mpBinDoc->valueazimuthVStraces=false;
		mpBinDoc->valueroseVStraces=false;

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}

void analyzebinform::on_Show_next_point_triggered()//显示下一个
{
	if(mpBinDoc->showNextpoint()==true)
	{
		mpBinView->refresh();
	}
}

void analyzebinform::on_Show_previous_point_triggered()//显示上一个
{
	if(mpBinDoc->showProvouspoint()==true)
	{
		mpBinView->refresh();
	}
}


void analyzebinform::setData3(bool i)
{
	mpBinDoc->setData3(i);
}

void analyzebinform::setData(int **data)
{
	mpBinDoc->setData(data);
}

void analyzebinform::setData2(int **data2)
{
	mpBinDoc->setData2(data2);   
}

void analyzebinform::setData4(int *i)
{
	mpBinDoc->setData4(i);

}

void analyzebinform::setDatatracespac(int i)
{
	mpBinDoc->setDatatracespac(i);
}

void analyzebinform::setDatapoint(QPoint point)
{
	mpBinDoc->setDatapoint(point);
	mpBinView->refresh();
}

void analyzebinform::setDataMeiguiVSTraces(int **MeiguiVSTraces)//传玫瑰花图-炮检对
{
	mpBinDoc->setDataMeiguiVSTraces(MeiguiVSTraces);
}

void analyzebinform::setDataBinfold(int **Binfold)//
{
	mpBinDoc->setDataBinfold(Binfold);
}

void analyzebinform::setDataOffsetVSFoldBins(int **OffsetVSFoldBins)//传炮检距-覆盖次数-面元数量
{
	mpBinDoc->setDataOffsetVSFoldBins(OffsetVSFoldBins);
}


void analyzebinform::setDatamaxoffset(int maxoffset)//传最
{
	mpBinDoc->setDatamaxoffset(maxoffset);
}

void analyzebinform::setDataXbinnumber(int xbinnumber)//传最
{
	mpBinDoc->setDataXbinnumber(xbinnumber);
}

void analyzebinform::setDataYbinnumber(int ybinnumber)//传最
{
	mpBinDoc->setDataYbinnumber(ybinnumber);
}

void analyzebinform::setDataOffsetVSTraces(int *OffsetVSTraces)// 传炮检距-炮检对数
{
	mpBinDoc->setDataOffsetVSTraces(OffsetVSTraces);
}

void analyzebinform::setDataAzimuthVSTraces(int *AzimuthVSTraces)// 传-炮检对数组
{
	mpBinDoc->setDataAzimuthVSTraces(AzimuthVSTraces);
}

void analyzebinform::setDataFoldVSBins(int *FoldVSBins)
{
	mpBinDoc->setDataFoldVSBins(FoldVSBins);
}


void analyzebinform::setDatamaxfold(int maxfold)//传最大
{
	mpBinDoc->setDatamaxfold(maxfold);
}

void analyzebinform::setDataValueworkarea(int valueworkarea)//设置工作区数据参数（用于显示）
{	//参数valueworkarea应该应该是一个bool类型
	mpBinDoc->setDatamaxfold(CMPmaxfold);
	mpBinDoc->setDatamaxoffset(CMPdlgmaxoffset);
	mpBinDoc->setDataXbinnumber(CMPxbinnumber);
	mpBinDoc->setDataYbinnumber(CMPybinnumber);
	mpBinDoc->setDataOffsetVSTraces(CMPOffsetVSTraces);
	mpBinDoc->setDataBinfold(CMPBinfold);
	mpBinDoc->setDataAzimuthVSTraces(CMPAzimuthVSTraces);
	mpBinDoc->setDataMeiguiVSTraces(CMPmeiguiVStraces);
	mpBinDoc->setDataOffsetVSFoldBins(CMPOffsetVSFoldBins);
	mpBinDoc->setDataFoldVSBins(CMPFoldVSBins);

	mpBinDoc->setDataValueworkarea(valueworkarea);//数据准备完成标识
	
	mpBinDoc->foldVSbins();

	mpBinView->setAODividNum(mAmizthNum,mOffsetNum,mShowAzimuth,mShowOffset,mBaseDivAmizth,mBaseDivOffset,mTextAmizth,mTextOffset);//方位角和炮检距分的段数
	
	mpBinView->refresh();
}

void analyzebinform::setDatalineOffsetVSTraces(int *lineOffsetVSTraces)// 传线面元炮检距-炮检对数组
{
	mpBinDoc->setDatalineOffsetVSTraces(lineOffsetVSTraces);
}

void analyzebinform::setDatalineAzimuthVSTraces(int *lineAzimuthVSTraces)// 传线面元方位角-炮检对数组
{
	mpBinDoc->setDatalineAzimuthVSTraces(lineAzimuthVSTraces);
}

void analyzebinform::setDatalineFoldVSBins(int *lineFoldVSBins)//线面元
{
	mpBinDoc->setDatalineFoldVSBins(lineFoldVSBins);
}
void analyzebinform::setDatalineOffsetVSFoldBins(int **lineOffsetVSFoldBins)//传炮检距-覆盖次数-面元数量
{
	mpBinDoc->setDatalineOffsetVSFoldBins(lineOffsetVSFoldBins);
}

void analyzebinform::setDatalineMeiguiVSTraces(int **lineMeiguiVSTraces)//传玫瑰花图-炮检对
{
	mpBinDoc->setDatalineMeiguiVSTraces(lineMeiguiVSTraces);
}

void analyzebinform::setDatalinecoordinate(QPoint linecoordinate)
{
	mpBinDoc->setDatalinecoordinate(linecoordinate);
}

void analyzebinform::setDataValueline(int valueline)//画
{

	mpBinDoc->setDataValueline(valueline);
	mpBinDoc->foldVSbins();
	mpBinView->refresh();
}


void analyzebinform::CMPLayoutAttibuteConculate(int **foldData)//观测系统属性计算
{	//传递参数foldData并没有被调用-刘伟
	int xline=CMPxbinnumber;
	int yline=CMPybinnumber;
	int zline=CMPmaxfold;
	if(CMPBinfold==NULL) gAlloc2(CMPBinfold,xline,yline);
	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			CMPBinfold[i][j]=0;
		}
	}
	//求覆盖次数-面元个数
	//分配内存
	if(CMPFoldVSBins==NULL) gAlloc1(CMPFoldVSBins,zline+1);
	for(int i=0;i<zline;i++)
	{
		CMPFoldVSBins[i]=0;//覆盖次数-面元数量数组初始化
	}	
	//分配内存	
	CMPdlgmaxoffset=0;//最大炮检距
	CMPmaxfold=0;

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			int temp=0;
			for(int k=0;k<offsetarray[i][j].count();k++)
			{
				if(offsetarray[i][j][k]!=-1) 
				{
					temp++;
					if(offsetarray[i][j][k]>CMPdlgmaxoffset) CMPdlgmaxoffset=offsetarray[i][j][k];//求最大炮检距
				}
			}
			if(temp!=0) CMPFoldVSBins[temp-1]++;
			CMPBinfold[i][j]=temp;//面元的覆盖次数数组	
			if(CMPmaxfold<CMPBinfold[i][j]) CMPmaxfold=CMPBinfold[i][j];//寻找最大覆盖次数
		}
	}
	//求覆盖次数-面元个数

	//炮检距-炮检对

	if(CMPOffsetVSTraces==NULL) gAlloc1(CMPOffsetVSTraces,CMPdlgmaxoffset+1);
	for(int i=0;i<CMPdlgmaxoffset;i++)
	{
		CMPOffsetVSTraces[i]=0;//炮检距-炮检对数组初始化
	}

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{		
			for(int k=0;k<offsetarray[i][j].count();k++)
			{
				int temp=0;
				if(offsetarray[i][j][k]!=-1) 
				{
					temp=offsetarray[i][j][k];
					CMPOffsetVSTraces[temp]++;
				}
			}
		}
	}
	//炮检距-炮检对
	//计算炮检距-覆盖次数-面元数量
	//玫瑰图分段
	lxRose=10;//炮检距分成几段
	lyRose=36;//方位角分段
	lxCSQF=6;//炮检距分成几段
	lyCSQF=36;//方位角分段
	maxoffsetCSQF=CMPdlgmaxoffset;//最大炮检距
//玫瑰图分段	
	lxzhu=8;//柱状图炮检距分段
	
	if(CMPOffsetVSFoldBins==NULL) gAlloc2(CMPOffsetVSFoldBins,lxzhu,CMPmaxfold+1);
	for(int i=0;i<lxzhu;i++)
	{
		for(int j=0;j<CMPmaxfold;j++)
		{
			CMPOffsetVSFoldBins[i][j]=0;
		}	
	}
	float kedu=CMPdlgmaxoffset;
	int ii=1;
	float a=0;
	while(kedu/ii>1)//选择一个合适的刻度
	{
		a=kedu/ii;
		ii=ii*10;	
	}
	kedu=ii/100;
	a=CMPdlgmaxoffset/kedu+1;
	float offset=kedu*a;

	int scaleoffset=offset/lxzhu;//偏移距lxRose等份分组
	int scaleoffsetRose=offset/lxRose;//偏移距lxRose等份分组
	int scaleangleRose=360/lyRose;//方位角等分
//玫瑰图分段
	QVector<int> tempfoldbins;
	int tempoffsetnum=0;
	maxzhufold=0;//统计的数组中最大覆盖次数

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{		
			tempfoldbins.clear();
			for(int h=0;h<lxzhu;h++)
			{
				tempfoldbins.push_back(0);
			}
			for(int k=0;k<offsetarray[i][j].count();k++)
			{
				if(offsetarray[i][j][k]!=-1) 
				{
					tempoffsetnum=offsetarray[i][j][k]/scaleoffset;//哪个炮检距段
					if(tempoffsetnum>=lxzhu) tempoffsetnum = lxzhu-1;
					tempfoldbins[tempoffsetnum]++;	//记录覆盖次数			
				}
			}
			for(int h=0;h<tempfoldbins.count();h++)
			{
				if(tempfoldbins[h]>0) CMPOffsetVSFoldBins[h][tempfoldbins[h]]++;
				if(tempfoldbins[h]>maxzhufold) maxzhufold=tempfoldbins[h];
			}
		}
	}
	tempfoldbins.clear();
	//炮检距-覆盖次数-面元数量数组付初值
	//方位角炮检对
	if(CMPAzimuthVSTraces!=NULL) gFree1(CMPAzimuthVSTraces);
	gAlloc1(CMPAzimuthVSTraces,360);
	//	if(CMPAzimuthVSTraces==NULL)	gAlloc1(CMPAzimuthVSTraces,360);
	for(int i=0;i<360;i++)
	{
		CMPAzimuthVSTraces[i]=0;//方位角-炮检对数组初始化
	}

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			for(int k=0;k<azimutharray[i][j].count();k++)
			{
				int temp=0;
				if(azimutharray[i][j][k]!=-1) 
				{
					temp=azimutharray[i][j][k];
					//CMPAzimuthVSTraces[temp]++;//方位角，赵老师的原始代码：存在内存溢出的风险
					if (temp >= 360)//  [Liu Wei 3/20/2023]
					{
						temp = temp-360;
					} 
					else if (temp<0)
					{
						temp=0;
					}
					CMPAzimuthVSTraces[temp]++;//方位角
				}
			}	
		}
	}
	//方位角炮检对
	//玫瑰花图-炮检对
	if(CMPmeiguiVStraces==NULL) gAlloc2(CMPmeiguiVStraces,lxRose,lyRose);
	for(int i=0;i<lxRose;i++)
	{
		for(int j=0;j<lyRose;j++)
		{
			CMPmeiguiVStraces[i][j]=0;//付初值
		}
	}
	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			for(int k=0;k<azimutharray[i][j].count();k++)
			{
				int kk=0;int hh=0;
				if(azimutharray[i][j][k]!=-1) 
				{
					hh=offsetarray[i][j][k]/scaleoffsetRose;//炮检距
					if(hh<0) continue;// 出现负数，导致内存溢出 [Liu Wei 3/20/2023]
					if(hh>=lxRose) 
						hh = lxRose-1;//modify by psl
					kk=azimutharray[i][j][k]/scaleangleRose;//方位角
                    if(kk<0)
						kk = 0;
					if(kk>lyRose-1)
						kk = lyRose-1;//modify by psl
					CMPmeiguiVStraces[hh][kk]++;
				}
			}	
		}
	}
	//玫瑰花图-炮检对
//计算炮检距非均匀性系数
	computeNonuniformfactorfun(offsetarray,CMPxbinnumber,CMPybinnumber,CMPmaxfold,CMPmaxfold,CMPmaxfold/5);//计算炮检距非均匀性系数
	computeAnglefactorfun(azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold,CMPmaxfold,CMPmaxfold/50);//计算方位角非均匀性系数
	Footprintfun(offsetarray,CMPxbinnumber,CMPybinnumber,CMPmaxfold,CMPmaxfold,CMPmaxfold/50);//采集脚印
	CSDFactorfun(offsetarray,azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold,CMPdlgmaxoffset);
}

//炮检距非均匀性系数显示
void analyzebinform::on_actionOffsetFactor_triggered()
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		//mpBinView->drawoffsetNonuniformfactor(m_dataNonuniform,CMPxbinnumber,CMPybinnumber,m_maxNonuniformfactor,m_minNonuniformfactor,1,1,"Offset Factor",-9999);
		mpBinView->drawOffsetNonuniformFactor(m_dataNonuniform,CMPxbinnumber,CMPybinnumber,m_maxNonuniformfactor,m_minNonuniformfactor,1,1,"Offset Factor",-9999);
		mpBinDoc->valueOffsetFactor=true;
		mpBinDoc->valuebinfold=false;
		mpBinDoc->valueoffsetVSfoldbins=false;
		mpBinDoc->valuefoldVSbins=false;
		mpBinDoc->valueoffsetVStraces=false;	
		mpBinDoc->valueazimuthVStraces=false;
		mpBinDoc->valueroseVStraces=false;

		bShowangle=false;
		bShowFoot=false;
		bShowOffset=true;
	}
}
//方位角非均匀性系数显示
void analyzebinform::on_actionAzimuthFactor_triggered()
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		//mpBinView->drawoffsetNonuniformfactor(m_dataAngle,CMPxbinnumber,CMPybinnumber,m_maxAnglefactor,m_minAnglefactor,1,1,"Azimuth Factor",-9999);
		mpBinView->drawOffsetNonuniformFactor(m_dataAngle,CMPxbinnumber,CMPybinnumber,m_maxAnglefactor,m_minAnglefactor,1,1,"Azimuth Factor",-9999);
		bShowangle=true;
		bShowFoot=false;
		bShowOffset=false;
	}
}
//采集脚印系数
void analyzebinform::on_actionFootFactor_triggered()
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		//mpBinView->drawoffsetNonuniformfactor(m_dataFoot,CMPxbinnumber,CMPybinnumber,m_maxFootfactor,m_minFootfactor,1,1,"Foot Factor",-9999);
		mpBinView->drawOffsetNonuniformFactor(m_dataFoot,CMPxbinnumber,CMPybinnumber,m_maxFootfactor,m_minFootfactor,1,1,"Foot Factor",-9999);
		bShowangle=false;
		bShowFoot=true;
		bShowOffset=false;
	}
}


//观测系统设计质量系数
void analyzebinform::on_actionCSQFactor_triggered()
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		//mpBinView->drawoffsetNonuniformfactor(m_dataCSDFPercent,CMPxbinnumber,CMPybinnumber,m_maxCSDFPfactor,m_minCSDFPfactor,1,1,"QF Factor",m_CSQFfactor);
		mpBinView->drawOffsetNonuniformFactor(m_dataCSDFPercent,CMPxbinnumber,CMPybinnumber,m_maxCSDFPfactor,m_minCSDFPfactor,1,1,"QF Factor",m_CSQFfactor);	
		bShowangle=false;
		bShowFoot=false;
		bShowOffset=false;
	}
}
int analyzebinform::exec()
{
	if ( in_loop ) {
		qWarning( "QDialog::exec: Recursive call detected" );
		return -1;
	}
	show();
	in_loop = true;
	return qApp->enter_loop();		
}

void analyzebinform::CSDFactorfun(GValueVector<short> **offset,GValueVector<short> **azimuth,int xline,int yline,int zline,float maxoffst)//观测系统设计质量系数函数
{
	int xBeginbinNum=2*mCircleR/mBinx;
	int xEndbinNum=xline-xBeginbinNum;
	int yBeginbinNum=2*mCircleR/mBiny;
	int yEndbinNum=yline-yBeginbinNum;
	bool canCSQFfactor=false;

	int begini;int endi;
	int beginj;int endj;
	GProject::getCurrentProject()->getFoldArea(begini,endi,beginj,endj);//满覆盖的范围

//计算观测系统质量
	int minAreaFold=100000;//满足区域内的最小覆盖次数
	int maxAreaFold=0;
	float maxAreaPercent=0;//满足区域内的最大的百分比
	float minAreaPercent=101;//满足区域内的最小的百分比

	float maxFoldPercent=0;//满覆盖次数范围的最大的百分比
	float minFoldPercent=101;//满覆盖次数范围的最小的百分比

	if(m_dataCSDFPercent!=NULL) gFree2(m_dataCSDFPercent);
	if(gAlloc2(m_dataCSDFPercent,xline,yline))//存放面元比例的数组
	{
		for(int i=0;i<xline;i++)//数组初始化
		{
			for(int j=0;j<yline;j++)
				m_dataCSDFPercent[i][j]=-1;//存放面元比例
		}
	}
	int offsetNum=lxCSQF;int azimuthNum=lyCSQF;
//	float kedu=maxoffst;
	float kedu=maxoffsetCSQF;
	int ii=1;
	float a=0;
	while(kedu/ii>1)//选择一个合适的刻度
	{
		a=kedu/ii;
		ii=ii*10;	
	}
	kedu=ii/100;
	a=maxoffst/kedu+1;
	float tempoffset=kedu*a;
	int scaleoffset=tempoffset/offsetNum;//偏移距n等份分组
	bool **tempdata;
	int scaleazimuth=360/azimuthNum;

	gAlloc2(tempdata,offsetNum,azimuthNum);
	
	//统计比例
	m_maxCSDFPfactor=0;//存放最大的那个系数
	m_minCSDFPfactor=10000000;

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			for(int m=0;m<offsetNum;m++)
			{
				for(int n=0;n<azimuthNum;n++)
				{
					tempdata[m][n]=false;//付初值
				}
			}//初始化
			int tempfold=0;//临时记录覆盖次数
			for(int k=0;k<azimuth[i][j].count();k++)
			{
				int kk=0;int hh=0;
				if(azimuth[i][j][k]!=-1) 
				{
					if(offset[i][j][k]>=maxoffsetCSQF) continue;
					tempfold++;//临时记录覆盖次数
					hh=offset[i][j][k]/scaleoffset;//炮检距
					if (hh<0) continue;// 出现负数，导致内存溢出 [Liu Wei 3/20/2023]
					if(hh>=offsetNum) 
						hh = offsetNum-1;//modify by psl
					kk=azimuth[i][j][k]/scaleazimuth;//方位角
					if(kk<0)
						kk = 0;
					if(kk>azimuthNum-1)
						kk = azimuthNum-1;//modify by psl
					tempdata[hh][kk]=true;
				}
			}//记录那些玫瑰图面元被占用
			int num=0;
			for(int m=0;m<offsetNum;m++)
			{
				for(int n=0;n<azimuthNum;n++)
				{
					if(tempdata[m][n]==true) num++;
				}
			}//统计那些面元
			float e=num/(float)(offsetNum*azimuthNum);
			if(m_maxCSDFPfactor<e) m_maxCSDFPfactor=e;//寻找最大值
			if(m_minCSDFPfactor>e) m_minCSDFPfactor=e;//寻找最小值
			m_dataCSDFPercent[i][j]=e;//面元比例系数

			if(i>=xBeginbinNum&&i<=xEndbinNum&&j>=yBeginbinNum&&j<=yEndbinNum)//整个工区
			{
				if(maxAreaPercent<e) maxAreaPercent=e;//寻找最大值
				if(minAreaPercent>e) minAreaPercent=e;//寻找最小值	
				canCSQFfactor=true;
			}
			if(i>begini&&i<endi&&j>beginj&&j<endj)//满覆盖次数区域，qf值只计算满覆盖次数区域
			{
				if(maxFoldPercent<e) maxFoldPercent=e;//寻找最大值
				if(minFoldPercent>e) minFoldPercent=e;//寻找最小值
				if(maxAreaFold<tempfold) maxAreaFold=tempfold;//寻找最大值
				if(minAreaFold>tempfold) minAreaFold=tempfold;//寻找最小值
			}

		}
	}
	gFree2(tempdata);

	//计算观测系统质量函数
	m_CSQFfactor=0;
	if(canCSQFfactor==true)//计算观测系统质量函数
	{
		int reFold=330;
//		float tempfz=sqrt((float)maxAreaFold*maxAreaFold/(float)(reFold*reFold)+maxAreaPercent*maxAreaPercent);
		float tempfz=sqrt((float)maxAreaFold*maxAreaFold/(float)(reFold*reFold)+maxFoldPercent*maxFoldPercent);
//		float tempfm=sqrt((float)(maxAreaFold-minAreaFold)*(maxAreaFold-minAreaFold)/(float)(reFold*reFold)+(maxAreaPercent-minAreaPercent)*(maxAreaPercent-minAreaPercent)+0.02);
		float tempfm=sqrt((float)(maxAreaFold-minAreaFold)*(maxAreaFold-minAreaFold)/(float)(reFold*reFold)+(maxFoldPercent-minFoldPercent)*(maxFoldPercent-minFoldPercent)+0.02);
		m_CSQFfactor=tempfz/tempfm;
	}
	//计算观测系统质量函数

}
//功能：输入炮检距数据计算非均匀性系数（计算函数）
//array：面元的炮检距数组，xl：x方面面元个数，yl：y方向面元个数，zl：最大的覆盖次数，
//foldup：统计的覆盖次数上限，folddown：统计的覆盖次数下线
void analyzebinform::computeNonuniformfactorfun(GValueVector<short> **offset,int xl,int yl,int zl,int foldup,int folddown)//
{
		if(m_dataNonuniform!=NULL) gFree2(m_dataNonuniform);
		if(gAlloc2(m_dataNonuniform,xl,yl))//存放非均匀性系数的数组
		{
			for(int i=0;i<xl;i++)//数组初始化
			{
				for(int j=0;j<yl;j++)
					m_dataNonuniform[i][j]=-1;//存放非均匀性系数数组
			}
		}
		int *a;//临时存放数组
		float maxoffset=0;//每个面元的最大炮检距
		m_maxNonuniformfactor=0;//存放最大的那个系数
		m_minNonuniformfactor=10000000;
		a=new int[zl];

		for(int i=0;i<xl;i++)
		{
			for(int j=0;j<yl;j++)
			{
				for(int k=0;k<zl;k++)
					a[k]=0;//数组清空
				int ii=0;//有几个值
				maxoffset=a[ii]=offset[i][j][0];
				for(int k=0;k<offset[i][j].count();k++)
				{	
					if(offset[i][j][k]!=-1) //将数据放在准备排序的数组里
					{
						a[ii]=offset[i][j][k];
						ii++;//覆盖次数	
						if(maxoffset<offset[i][j][k]) maxoffset=offset[i][j][k];//最大炮检距
					}
				}				
				if(ii>=folddown&&ii<=foldup)//在输入的覆盖次数范围内进行计算
					//				if(ii>1)//在输入的覆盖次数范围内进行计算
				{
					float diter=maxoffset/ii;//炮检距标准间隔
					int temp=0;
					for(int m=0;m<ii;m++) 
						for(int n=0;n<ii-m-1;n++) 
							if(a[n]>a[n+1]) 
							{ 
								temp=a[n];a[n]=a[n+1];a[n+1]=temp; 
							} 
							//冒泡排序	
							float e=0;//炮检距非均匀性系数
							for(int l=0;l<ii-1;l++)
							{
								float temp2=(float)(a[l+1]-a[l]);//想邻道的炮检距变化量
								temp2=temp2/diter;//两个差值
								if(temp2<1) temp2=1+1-temp2;
								e=e+temp2;
							}
							float temp3=(float)(zl)/ii;//补成满覆盖次数
							e=e*temp3;//补成满覆盖次数
							e=e/(ii-1);//平均到每道										
							if(m_maxNonuniformfactor<e) m_maxNonuniformfactor=e;//寻找最大值
							if(m_minNonuniformfactor>e) m_minNonuniformfactor=e;//寻找最小值
							m_dataNonuniform[i][j]=e;//炮检距非均匀性系数数组
				}			
			}
		}
		delete []a;
		for(int i=0;i<xl;i++)//面元炮检距数组初始化
		{
			for(int j=0;j<yl;j++)
			{
				if(m_dataNonuniform[i][j]==-1) m_dataNonuniform[i][j]=0;
			}
		}
		if(m_minNonuniformfactor==10000000) m_minNonuniformfactor=0;
}
//功能：输入方位角数据计算非均匀性系数（计算函数
void analyzebinform::computeAnglefactorfun(GValueVector<short> **azimuth,int xl,int yl,int zl,int foldup,int folddown)//输入方位角数据计算非均匀性系数（计算函数）
{
		if(gAlloc2(m_dataAngle,xl,yl))//存放非均匀性系数的数组
		{
			for(int i=0;i<xl;i++)//数组初始化
			{
				for(int j=0;j<yl;j++)
					m_dataAngle[i][j]=-1;//存放非均匀性系数数组
			}
		}
		int *a;//临时存放数组
		float maxangle=0;//每个面元的最大炮检距
		m_maxAnglefactor=0;//存放最大的那个系数
		m_minAnglefactor=10000000;
		a=new int[zl];

		for(int i=0;i<xl;i++)
		{
			for(int j=0;j<yl;j++)
			{
				for(int k=0;k<zl;k++)
					a[k]=0;//数组清空
				int ii=0;//有几个值
				maxangle=a[ii]=azimuth[i][j][0];
				for(int k=0;k<azimuth[i][j].count();k++)
				{	
					if(azimuth[i][j][k]!=-1&&azimuth[i][j][k]>=0&&azimuth[i][j][k]<=360) //将数据放在准备排序的数组里
					{
						a[ii]=azimuth[i][j][k];
						ii++;//覆盖次数	
						if(maxangle<azimuth[i][j][k]) maxangle=azimuth[i][j][k];//最大炮检距
					}
				}
				if(ii<=0) continue;
				if(ii>=folddown&&ii<=foldup)//在输入的覆盖次数范围内进行计算
				{
					float diter=(float)360/(float)ii;//炮检距标准间隔
					int temp=0;
					for(int m=0;m<ii;m++) 
						for(int n=0;n<ii-m-1;n++) 
							if(a[n]>a[n+1]) 
							{ 
								temp=a[n];a[n]=a[n+1];a[n+1]=temp; 
							} 
							//冒泡排序	
							float e=0;//方位角非均匀性系数
							for(int l=0;l<ii-1;l++)
							{
								float temp2=(float)(a[l+1]-a[l]);//想邻道的炮检距变化量
								temp2=temp2/diter;//两个差值
								if(temp2<1) temp2=1+1-temp2;
								e=e+temp2;
							}
							float temp2=(float)(360-a[ii-1]+a[0]);//求最大的方位角差值
							temp2=temp2/diter;
							if(temp2<1) temp2=1+1-temp2;
							e=e+temp2;
							e=e/ii;//平均到每道										
							if(m_maxAnglefactor<e) m_maxAnglefactor=e;//寻找最大值
							if(m_minAnglefactor>e) m_minAnglefactor=e;//寻找最小值
							m_dataAngle[i][j]=e;//炮检距非均匀性系数数组
				}			
			}
		}
		delete []a;
		for(int i=0;i<xl;i++)//面元炮检距数组初始化
		{
			for(int j=0;j<yl;j++)
			{
				if(m_dataAngle[i][j]==-1) m_dataAngle[i][j]=0;
			}
		}
		if(m_minAnglefactor==10000000) m_minAnglefactor=0;
}

//采集脚印计算
void analyzebinform::Footprintfun(GValueVector<short> **offset,int xline,int yline,int zline,int foldup,int folddown)//采集脚印计算
{
	if(gAlloc2(m_dataFoot,xline,yline))//存放非均匀性系数的数组
	{
		for(int i=0;i<xline;i++)//数组初始化
		{
			for(int j=0;j<yline;j++)
				m_dataFoot[i][j]=-1;//存放非均匀性系数数组
		}
	}

	m_maxFootfactor=0;//存放最大的那个系数
	m_minFootfactor=10000000;

	float foot;
	float depth=1000;//目的层深度
	int vp1=2800;int vp2=3200;
	int vs1=1400;int vs2=1660;
	float d1=2.26;float d2=2.38;//密度
	int f=30;//频率

	float dievp=vp2-vp1;float avp=(vp2+vp1)*0.5;//速度
	float died=d2-d1;float avd=(d2+d1)*0.5;//密度
	float dievs=vs2-vs1;float avs=(vs2+vs1)*0.5;//横波
	float A=0.5*(dievp/avp+died/avd);
	float B=0.5*dievp/avp-4*avs*dievs/(avp*avp)-2*avs*avs*died/(avp*avp*avd);
	float C=0.5*dievp/avp;
	float pi=3.14159265358979323846264338327950288419717;//
	float e= 2.718281828459;
	float temp=-1000000*pi*f/(2.807*pow(vp1,3.2));

	for(int i=0;i<xline;i++)
	{
		for(int j=0;j<yline;j++)
		{
			foot=0;
			int num=0;
			for(int k=0;k<offset[i][j].count();k++)
			{
				if(offset[i][j][k]!=-1)
				{
					float tempoffset=(float)offset[i][j][k]/2;
					float r=sqrt(tempoffset*tempoffset+depth*depth);//传播距离
					float sink=tempoffset/r;//正旋
					float tank=tempoffset/depth;//正切
					float Rpp=A+B*sink*sink+C*sink*sink*tank*tank;//纵波反射系数
					float amp=1000000*pow(e,temp)/r;//振幅衰减

					foot=Rpp*amp+foot;//实际振幅等于反射系数剩以衰减因子
					num++;
				}

			}
//			if(num==zline)//只计算满覆盖次数的面元
			if(num>=folddown&&num<=foldup)//在输入的覆盖次数范围内进行计算
			{
				if(num==0) continue;
				m_dataFoot[i][j]=foot/num;
				if(m_maxFootfactor<m_dataFoot[i][j]) m_maxFootfactor=m_dataFoot[i][j];//寻找最大值
				if(m_minFootfactor>m_dataFoot[i][j]) m_minFootfactor=m_dataFoot[i][j];//寻找最小值
			}
		}
	}

	for(int i=0;i<xline;i++)//面元炮检距数组初始化
	{
		for(int j=0;j<yline;j++)
		{
			if(m_dataFoot[i][j]==-1) m_dataFoot[i][j]=0;
		}
	}
	if(m_minFootfactor==10000000) m_minFootfactor=0;
}

//观测系统信息显示板
void analyzebinform::on_actionGeometryImformation_triggered()
{
	if(mpBinDoc->getDataValueworkarea()==true)	
	{
		
		GProject *pProject = GProject::getCurrentProject();
		GSPSParam *pSPSParam = pProject->getSPSParam();
		int begini;int endi;
		int beginj;int endj;
		pProject->getFoldArea(begini,endi,beginj,endj);//满覆盖的范围
		int JZfold=CMPmaxfold;//截止覆盖次数
		int JZfoldNum=0;//大于截止覆盖次数的面元个数

		for(int i=0;i<CMPxbinnumber;i++)
		{
			for(int j=0;j<CMPybinnumber;j++)
			{
				if(CMPBinfold[i][j]>=JZfold) JZfoldNum++;//统计大于截止覆盖次数的面元个数
			}
		}
		float Xlength=CMPxbinnumber*mBinx*0.001;//有数据X长度
		float Ylength=CMPybinnumber*mBiny*0.001;//有数据Y长度
		float Allarea=Xlength*Ylength;//有数据面积
		float ManCiXlength=(endi-begini+1)*mBinx*0.001;//满次X长度
		float ManCiYlength=(endj-beginj+1)*mBiny*0.001;//满次Y长度
		float ManCiarea=ManCiXlength*ManCiYlength;//满次面积
		//int AllSPNum=pSPSParam->mSLineCount*pSPSParam->mSPtCount*pSPSParam->mArrayCount*pSPSParam->mXArrayCount;//总炮数	
		int AllSPNum=pSPSParam->mSLineCount*pSPSParam->mSPtCount;//总炮数，注意这个数值的获取20230117	
		
		float JZarea=JZfoldNum*mBinx*0.001*mBiny*0.001;//某个覆盖次数（截止覆盖次数）以上面积
		float maxXoffset=fabs((pSPSParam->mSPtInv*(pSPSParam->mSPtCount-1))+pSPSParam->mSX0-pSPSParam->mRX0);//最大纵向距
		float maxYoffset=fabs(pSPSParam->mSY0-pSPSParam->mRY0-(pSPSParam->mRLineInv*(pSPSParam->mRLineCount-1)));//最大非纵距
		float maxoffset=sqrt(maxXoffset*maxXoffset+maxYoffset*maxYoffset);//最大炮检距
		float XYratio=maxYoffset/maxXoffset;//横纵比	

		TGeometryImforDlg *dlg=new TGeometryImforDlg();
		dlg->init(CMPxbinnumber,CMPybinnumber,mBinx,mBiny,CMPBinfold,CMPmaxfold);
		dlg->ui.emaxXoffset->setText(QString::number(maxXoffset));
		dlg->ui.emaxYoffset->setText(QString::number(maxYoffset));
		dlg->ui.emaxoffset->setText(QString::number(maxoffset));
		dlg->ui.eXYratio->setText(QString::number(XYratio));
		dlg->ui.eXLength->setText(QString::number(Xlength));
		dlg->ui.eYLength->setText(QString::number(Ylength));
		dlg->ui.eAllarea->setText(QString::number(Allarea));
		dlg->ui.eManCiXlength->setText(QString::number(ManCiXlength));
		dlg->ui.eManCiYlength->setText(QString::number(ManCiYlength));
		dlg->ui.eManCiarea->setText(QString::number(ManCiarea));
		dlg->ui.eAllSPNum->setText(QString::number(AllSPNum));
		dlg->ui.eJZarea->setText(QString::number(JZarea));
		dlg->ui.eJZfold->setText(QString::number(JZfold));
		dlg->ui.eMaxFold->setText(QString::number(CMPmaxfold));		
		dlg->exec();
	}
}
