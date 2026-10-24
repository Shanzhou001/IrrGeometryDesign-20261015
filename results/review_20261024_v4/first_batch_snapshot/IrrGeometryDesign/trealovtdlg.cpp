#include "trealovtdlg.h"

using namespace std;

TRealOVTdlg::TRealOVTdlg(QWidget *parent)
: QDialog(parent)
{
	ui.setupUi(this);
	pAttrAnalyzeParam=new GAttrAnalyzeParam();
}

TRealOVTdlg::~TRealOVTdlg()
{

}

void TRealOVTdlg::init(GAttrAnalyzeParam *param)
{
	pAttrAnalyzeParam=param;
	ui.eBeginInOffset->setText(QString::number(pAttrAnalyzeParam->mRealBeginAreaInOffset));
	ui.eEndInOffset->setText(QString::number(pAttrAnalyzeParam->mRealEndAreaInOffset));
	ui.eInLineMaxOffset->setText(QString::number(pAttrAnalyzeParam->mInMaxOffset));
	ui.eCLineMaxOffset->setText(QString::number(pAttrAnalyzeParam->mCLMaxOffset));

	ui.eBeginCLOffset->setText(QString::number(pAttrAnalyzeParam->mRealBeginAreaCLOffset));
	ui.eEndCLOffset->setText(QString::number(pAttrAnalyzeParam->mRealEndAreaCLOffset));
}
//计算OVT属性的对象是滚动后的采集观测系统排列（炮线和检波线，SPS）。刘伟
//函数原始功能是快速计算一个OVT片中的覆盖次数。刘伟
//功能：通过界面指定某一个OVT片，这个参数取值有待商榷。刘伟
//功能基本实现 [11/3/2022 Liu Wei]
void TRealOVTdlg::on_OVT_PatchAttri_clicked()
{
	GProject *pProject = GProject::getCurrentProject();
	if (!pProject) return;
	GSLineSet *pSLineSet = (pProject->mIsNewImfo && pProject->getSLineSetNew() && pProject->getSLineSetNew()->count() > 0)
	                       ? pProject->getSLineSetNew()
	                       : ((pProject->getSLineSet() && pProject->getSLineSet()->count() > 0) ? pProject->getSLineSet() : pProject->getSLineSetNew());
	GRLineSet *pRLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
	                       ? pProject->getRLineSetNew()
	                       : ((pProject->getRLineSet() && pProject->getRLineSet()->count() > 0) ? pProject->getRLineSet() : pProject->getRLineSetNew());
	GShotSet *pShotSet = (pProject->mIsNewImfo && pProject->getShotSetNew() && pProject->getShotSetNew()->count() > 0)
	                     ? pProject->getShotSetNew()
	                     : ((pProject->getShotSet() && pProject->getShotSet()->count() > 0) ? pProject->getShotSet() : pProject->getShotSetNew());
	GSPSParam *pSPSParam = pProject->getSPSParam();	//SPS基本参数。合并前两边调用函数名不同
	GCellGrid *pCellGrid = pProject->getCellGrid();	//共中心点（面元）数据集

	if (!pSLineSet || !pRLineSet || !pShotSet || !pSPSParam || !pCellGrid ||
	    pSLineSet->count() == 0 || pRLineSet->count() == 0 || pShotSet->count() == 0 ||
	    pCellGrid->getNumX() <= 0 || pCellGrid->getNumY() <= 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("观测系统数据或面元网格未就绪，无法计算 OVT 片属性。"));
		return;
	}

	if(fabsf(ui.eBeginInOffset->text().toInt()) >ui.eInLineMaxOffset->text().toInt() || fabsf(ui.eEndInOffset->text().toInt()) >ui.eInLineMaxOffset->text().toInt() 
		|| fabsf(ui.eBeginCLOffset->text().toInt()) >ui.eCLineMaxOffset->text().toInt() || fabsf(ui.eEndCLOffset->text().toInt())>ui.eCLineMaxOffset->text().toInt())
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("理论情况下，超过最大炮检距范围；横向间隔为2倍接收线距，纵向间隔为2倍炮线距"));//消息对话框显示提示用户违法操作
		return;
	}// 参数要求：同一个方向的参数必须同时为正或负，不能跨坐标轴。
	// 获取界面输入偏移距向量片范围，OVT分析范围，为什么是这个范围！？
	// 答：人为指定的范围，测试用

	pAttrAnalyzeParam->mRealBeginAreaInOffset=ui.eBeginInOffset->text().toInt();
	pAttrAnalyzeParam->mRealEndAreaInOffset=ui.eEndInOffset->text().toInt();
	pAttrAnalyzeParam->mRealBeginAreaCLOffset=ui.eBeginCLOffset->text().toInt();
	pAttrAnalyzeParam->mRealEndAreaCLOffset=ui.eEndCLOffset->text().toInt();

	float minSX, minSY, maxSX, maxSY;//最大最小震源坐标-滚动后
	float minRX, minRY, maxRX, maxRY;//最大最小检波点坐标-滚动后
	minSX = minSY = LONG_MAX;
	maxSX = maxSY = LONG_MIN;
	minRX = minRY = LONG_MAX;
	maxRX = maxRY = LONG_MIN;
	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);	//得到炮线变换后坐标范围
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);	//得到测线变换后坐标范围
	float  x0, y0, xn, yn;
	if (pCellGrid->mGridXY != NULL)
	{
		x0 = pCellGrid->mGridXY[0][0].x();
		y0 = pCellGrid->mGridXY[0][0].z();
		xn = pCellGrid->mGridXY[pCellGrid->getNumY()][pCellGrid->getNumX()].x();
		yn = pCellGrid->mGridXY[pCellGrid->getNumY()][pCellGrid->getNumX()].z();
	}
	else
	{
		x0 = (minRX + minSX)/2 + pCellGrid->mDiffX;
		y0 = (minRY + minSY)/2 + pCellGrid->mDiffY;	
		xn = (maxRX + maxSX)/2;
		yn = (maxRY + maxSY)/2;
	}

	//固定坐标原点为观测系统中第一条检波线的第一个检波点
	int	Num_ShotPerSLine, Num_RecPerRecLine, Num_RecLine;
	float int_shotpoint, int_receiver, int_RecLine, int_SLine;
	float Hxi_receiver, Vyi_receiver, Hxi_shot, Vyi_shot;
	float maxDis_nonlongitudinal; // Maximum non-longitudinal distance （最大非纵炮检距）
	float maxDis_longitudinal; // Maximum longitudinal distance （最大纵向炮检距）
	//自动获取模板参数
	Num_ShotPerSLine=pSPSParam->mSPtCount;
	Num_RecPerRecLine=pSPSParam->mRPtCount;
	Num_RecLine=pSPSParam->mRLineCount;
	int_shotpoint=pSPSParam->mSPtInv;
	int_receiver=pSPSParam->mRPtInv;
	int_RecLine=pSPSParam->mRLineInv;
	int_SLine=pSPSParam->mSLineInv;//炮线距离

	//////////////////////////////////////////////////////////////////////////手动输入模板参数-测试用202230117
	//Num_ShotPerSLine=5;
	//Num_RecPerRecLine=240;
	//Num_RecLine=40;
	//int_shotpoint=50;
	//int_receiver=25;
	//int_RecLine=125;
	//int_SLine=150;//炮点检波线方向的滚动距离
	//////////////////////////////////////////////////////////////////////////手动输入模板参数-测试用202230117

	//最大非纵距和最大纵向距离其实是最大炮检距纵、横向分量的大小（在做实际SPS数据的时候，我们要通过扫描整个观测系统提取这个数据）
	//maxDis_nonlongitudinal=(Num_RecLine-1)*int_RecLine/2 + (Num_ShotPerSLine-1)*int_shotpoint/2;
	//maxDis_longitudinal=(Num_RecPerRecLine-1)*int_receiver/2;

	maxDis_nonlongitudinal=pCellGrid->mCLMaxoffset;
	maxDis_longitudinal=pCellGrid->mInMaxoffset;

	//横、纵向OVT片间距-可用于构建绘图坐标-根据模板参数自动计算
	float gy, gx; //横向间距gy，纵向间距gx
	int maxgys_y=gcd( int(int_RecLine),int(int_shotpoint) ); //求两个数的最大公约数
	int maxgys_x=gcd( int(int_receiver),int(int_SLine) ); //求两个数的最大公约数
	if (maxgys_y <= 0) maxgys_y = 1;
	if (maxgys_x <= 0) maxgys_x = 1;
	gy=int(int_shotpoint*int_RecLine)/maxgys_y; //求两个数的最小公倍数
	gx=int(int_receiver*int_SLine)/maxgys_x; //求两个数的最小公倍数
	if (gy <= 0) gy = 50.0f;
	if (gx <= 0) gx = 50.0f;

	float realOVTLengthX=fabsf(pAttrAnalyzeParam->mRealEndAreaInOffset - pAttrAnalyzeParam->mRealBeginAreaInOffset);
	float realOVTLengthY=fabsf(pAttrAnalyzeParam->mRealEndAreaCLOffset - pAttrAnalyzeParam->mRealBeginAreaCLOffset);
	if (realOVTLengthX > 2*gx || realOVTLengthY > 2*gy)
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("指定OVT片间距太大，将采用理论间距：横向间隔为2*gy，纵向间隔为2*gx！"));//消息对话框显示提示用户违法操作
	}
	//计算目标OVT片的索引
	int target_OVT_ix, target_OVT_iy;
	int tgtix1,tgtix2,tgtiy1,tgtiy2;
	if (pAttrAnalyzeParam->mRealBeginAreaInOffset>=0)
	{
		tgtix1=int(pAttrAnalyzeParam->mRealBeginAreaInOffset/(2*gx))+1;
	} 
	else
	{
		tgtix1=int(pAttrAnalyzeParam->mRealBeginAreaInOffset/(2*gx))-1;
	}

	if (pAttrAnalyzeParam->mRealEndAreaInOffset>=0)
	{
		tgtix2=int(pAttrAnalyzeParam->mRealEndAreaInOffset/(2*gx))+1;
	} 
	else
	{
		tgtix2=int(pAttrAnalyzeParam->mRealEndAreaInOffset/(2*gx))-1;
	}

	if (pAttrAnalyzeParam->mRealBeginAreaCLOffset>=0)
	{
		tgtiy1=int(pAttrAnalyzeParam->mRealBeginAreaCLOffset/(2*gy))+1;
	} 
	else
	{
		tgtiy1=int(pAttrAnalyzeParam->mRealBeginAreaCLOffset/(2*gy))-1;
	}

	if (pAttrAnalyzeParam->mRealEndAreaCLOffset>=0)
	{
		tgtiy2=int(pAttrAnalyzeParam->mRealEndAreaCLOffset/(2*gy))+1;
	} 
	else
	{
		tgtiy2=int(pAttrAnalyzeParam->mRealEndAreaCLOffset/(2*gy))-1;
	}

	if (tgtix1!=tgtix2)
	{
		target_OVT_ix=min(tgtix1,tgtix2);
	}
	else 
	{
		target_OVT_ix=tgtix1;
	}

	if (tgtiy1!=tgtiy2)
	{
		target_OVT_iy=min(tgtiy1,tgtiy2);
	}
	else 
	{
		target_OVT_iy=tgtiy1;
	}

	//计算横向分组号-炮线方向
	//分组号的值域是[min_gyi, max_gyi]，但是不包含0的整数
	int min_gyi, max_gyi;
	if ((Num_ShotPerSLine-1)*int_shotpoint <= int_RecLine)
	{
		max_gyi = int(maxDis_nonlongitudinal/(2*gy)+1);
		min_gyi = -int(maxDis_nonlongitudinal/(2*gy)+1);
	}
	else
	{
		max_gyi = int(maxDis_nonlongitudinal/(2*gy));
		min_gyi = -int(maxDis_nonlongitudinal/(2*gy));
	}
	//计算纵向分组号-检波线方向
	//分组号的值域是[min_gxi, max_gxi]，但是不包含0的整数
	int max_gxi = int(maxDis_longitudinal/(2*gx) + 1);
	int min_gxi = -int(maxDis_longitudinal/(2*gx) + 1);

	if (target_OVT_ix>max_gxi || target_OVT_ix<min_gxi || target_OVT_iy>max_gyi || target_OVT_iy<min_gyi)
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("目标排列片不超出理论计算范围！"));//消息对话框显示提示用户违法操作
		return;
	}
	//覆盖次数数组
	int CMPxbinnumber=pCellGrid->getNumX();
	int CMPybinnumber=pCellGrid->getNumY();
	int **tempfold;//临时存放CRP面元覆盖次数数组
	gAlloc2(tempfold,CMPxbinnumber,CMPybinnumber);	//申请数组空间
	// CRP面元覆盖次数存储数组初始化
	for(int i=0;i<CMPxbinnumber;i++)
		for(int j=0;j<CMPybinnumber;j++)
			tempfold[i][j]=0;
	//偏移距和方位角属性数组
	GValueVector<short> **offsetarray;	
	GValueVector<short> **Azimutharray;
	gAlloc2(offsetarray,CMPxbinnumber,CMPybinnumber);
	gAlloc2(Azimutharray,CMPxbinnumber,CMPybinnumber);
	//赵虎添加
	for(int i=0;i<CMPxbinnumber;i++)
	{
		for(int j=0;j<CMPybinnumber;j++)
		{
			for(int k=0;k<1;k++)
			{
				offsetarray[i][j].append(-1);
				Azimutharray[i][j].append(-1);
			}
		}
	}//-1表示空
	//遍历所有炮点和检波线-检波点
	for(int ii=0;ii<pShotSet->count();ii++)	//炮集数循环
	{
		GShot *pShot = pShotSet->get(ii);	//得到指定下标炮集
		if(pShot != 0)
		{
			double sx=pShot->getSPoint()->getX();
			double sy=pShot->getSPoint()->getY();
			double ssx=pShot->getSPoint()->getSPSX();
			double ssy=pShot->getSPoint()->getSPSY();
			for(int si=0; si<pShot->getTrcCount(); si++)	//该炮对应道总数循环
			{
				GRPoint *rpoint = pShot->getRPointByTrcIndex(si,pRLineSet);
				if(rpoint==NULL)
					continue;
				float rrx = rpoint->getSPSX();
				float rry  = rpoint->getSPSY();
				float rx = rpoint->getX();
				float ry = rpoint->getY();
				int xInoffset=rx-sx;//炮检距向量纵向分量
				int yCLoffset=ry-sy;//炮间距向量横向分量	
				int tempx=(rx+sx)/2;	//炮检对中心坐标x
				int tempy=(ry+sy)/2;	//炮检对中心坐标y

				int iox=(tempx-x0)/pCellGrid->mGridX;
				int ioy=(tempy-y0)/pCellGrid->mGridY;

				if(iox>=0&&ioy>=0&&iox<=CMPxbinnumber||ioy<=CMPybinnumber)
				{
					//OVT片分组编号索引（映射）
					int tempxi,tempyi;
					if (xInoffset>0)//计算纵向OVT片分组编号
					{
						tempxi=int(xInoffset/(2*gx))+1;
					}
					else
					{
						tempxi=int(xInoffset/(2*gx))-1;
					}
					if (yCLoffset>0)//计算横向OVT片分组编号
					{
						tempyi=int(yCLoffset/(2*gy))+1;
					} 
					else
					{
						tempyi=int(yCLoffset/(2*gy))-1;
					}
					//超出理论最大炮检距的强制归入最大炮检距组
					if (tempxi>max_gxi)
					{
						tempxi=max_gxi;
					}
					else if (tempxi<min_gxi)
					{
						tempxi=min_gxi;
					}
					else
					{
						tempxi=tempxi;
					}
					//超出理论最大炮检距的强制归入最大炮检距组
					if (tempyi>max_gyi)
					{
						tempyi=max_gyi;
					}
					else if (tempyi<min_gyi)
					{
						tempyi=min_gyi;
					}
					else
					{
						tempyi=tempyi;
					}
					//判断是否是目标OVT片（利用分组编号）
					if (target_OVT_ix==tempxi && target_OVT_iy==tempyi)
					{
						tempfold[iox][ioy]++;
						//累积记录炮检距
						float tOffset=sqrtf(xInoffset*xInoffset+yCLoffset*yCLoffset);//计算炮检距大小（与坐标系旋转没有关系）
						if(offsetarray[iox][ioy][0]==-1)
						{
							offsetarray[iox][ioy][0]=short(tOffset);
						}
						else 
						{
							offsetarray[iox][ioy].append(short(tOffset));
						}

						//求方位角:炮点指向检波点的向量方位
						float temp1=(float)(rry-ssy);	//用转换后的坐标计算不行吗？
						float temp2=(float)(rrx-ssx);	//答：不行，因为真实的观测系统可能不是正南正北（炮线方向）					
						//利用反正切函数求解方位角弧度，并将弧度值转换为角度，因为不论是atanf，atan2f输出的角度都是弧度值，需要进行转换。
						//https://blog.csdn.net/weixin_46098577/article/details/116026828
						float atgt_deg, azimuth_deg; //反正切角度, 换算方位角度
						if(temp2==0) //temp2==0表示炮点和检波点都在y轴上。此外，还应该判断dx的正负，判定具体角度正半轴0°(正北为零度），负半轴180°
						{
							if (temp1 >= 0)//正半轴
							{
								azimuth_deg=0;
							}
							else //负半轴
							{
								azimuth_deg=180;
							}
						}
						else 
						{
							atgt_deg=atanf(temp1/temp2)*180/PI;//弧度值转换为角度值 
							//atan2(y,x)、atan2f(y,x)的值域输出为(-pi,pi);而atan(x)、atanf(x)只能输出(-pi/2,pi/2)
							if(rrx<ssx)//直接用rrx，rry；ssx，ssy，效果如何？
							{
								if(rry<ssy)	
								{azimuth_deg=270-abs(atgt_deg);}
								else 
								{azimuth_deg=270+abs(atgt_deg);}
							}
							else
							{
								if(rry<ssy)	
								{azimuth_deg=90+abs(atgt_deg);}
								else 
								{azimuth_deg=90-abs(atgt_deg);}
							}
							//if(rx<sx)//为什么不直接用rrx，rry；ssx，ssy呢？
							//{
							//	if(ry<sy)	
							//	{azimuth_deg=270-abs(atgt_deg);}
							//	else 
							//	{azimuth_deg=270+abs(atgt_deg);}
							//}
							//else
							//{
							//	if(ry<sy)	
							//	{azimuth_deg=90+abs(atgt_deg);}
							//	else 
							//	{azimuth_deg=90-abs(atgt_deg);}
							//}
						}

						if(Azimutharray[iox][ioy][0]==-1)
						{
							Azimutharray[iox][ioy][0]=short(azimuth_deg);
						}
						else 
						{
							Azimutharray[iox][ioy].append(short(azimuth_deg));
						}
					}
				}
			}									
		}
	}
	//测试是否存在多炮检距、方位角面元
	for(int i=0;i<CMPxbinnumber;i++)
	{
		for(int j=0;j<CMPybinnumber;j++)
		{
			int sum_offset=0;
			if (offsetarray[i][j].count()>1)//有多个炮检对的面元
			{
				sum_offset++;
			}
		}
	}	
	QString str="Inline offset:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaInOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaInOffset)+" , Crline offset:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaCLOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaCLOffset);

	int CMPmaxfold=0;
	for(int i=0;i<CMPxbinnumber;i++)
	{
		for(int j=0;j<CMPybinnumber;j++)
		{
			if(tempfold[i][j]>CMPmaxfold) CMPmaxfold=tempfold[i][j];
		}
	}
	if(CMPmaxfold==0)
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("最大覆盖次数为零"));
		return;
	}

	analyzebinform *dlg=new analyzebinform(this);
	bool valueanalyzebinWorkArea;
	if(CMPmaxfold>0) 	valueanalyzebinWorkArea=true;
	//dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,pSPSParam->mCircleR);//error C2039: “mCircleR”: 不是“GSPSParam”的成员
	dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,0);	

	//赵虎添加
	QString strAzimuth="";
	QString strOffset="OVT Patch:"+str;
	dlg->setAODividNum(1,1,0,0,360,pCellGrid->mMaxoffset,strAzimuth,strOffset);//方位角和炮检距分的段数
	dlg->CMPLayoutAttibuteConculate(tempfold);	
	dlg->setDataValueworkarea(valueanalyzebinWorkArea);	
	dlg->setAttribute(Qt::WA_DeleteOnClose,true);//关闭窗体时，销毁窗体，清空内存
	dlg->exec();

	//释放内存空间
	if(offsetarray!=NULL) 
	{
		for(int i=0;i<CMPxbinnumber;i++)
		{
			for(int j=0;j<CMPybinnumber;j++)
			{
				offsetarray[i][j].clear();
				Azimutharray[i][j].clear();
			}
		}	
		gFree2(offsetarray);
		gFree2(Azimutharray);	
	}

	gFree2(tempfold);
}
//基于赵老师的原始代码修改
//功能：计算任意网格片中的OVT片属性
//缺陷：会出现属性空白异常，暂时舍弃
//void TRealOVTdlg::on_OVT_ArbitraryPatchAttri_clicked()
//{
//	GProject *pProject = GProject::getCurrentProject();
//	GSLineSet *pSLineSet = pProject->getSLineSet();
//	GRLineSet *pRLineSet = pProject->getRLineSet();
//	GShotSet *pShotSet = pProject->getShotSet();
//	GSPSParam *pSPSParam = pProject->getSPSParam();
//	GCellGrid *pCellGrid = pProject->getCellGrid();
//
//	if(ui.eBeginInOffset->text().toInt() > ui.eEndInOffset->text().toInt() || ui.eBeginCLOffset->text().toInt() > ui.eEndCLOffset->text().toInt())
//	{
//		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("起始炮检距分量要小于终止炮检距分量，推荐：横向间隔为炮线距的2倍，纵向间隔为接收线距的2倍"));//消息对话框显示提示用户违法操作
//		return;
//	}
//	pAttrAnalyzeParam->mRealBeginAreaInOffset=ui.eBeginInOffset->text().toInt();
//	pAttrAnalyzeParam->mRealEndAreaInOffset=ui.eEndInOffset->text().toInt();
//	pAttrAnalyzeParam->mRealBeginAreaCLOffset=ui.eBeginCLOffset->text().toInt();
//	pAttrAnalyzeParam->mRealEndAreaCLOffset=ui.eEndCLOffset->text().toInt();
//
//	float minSX, minSY, maxSX, maxSY;
//	float minRX, minRY, maxRX, maxRY;
//	minSX = minSY = LONG_MAX;
//	maxSX = maxSY = LONG_MIN;
//	minRX = minRY = LONG_MAX;
//	maxRX = maxRY = LONG_MIN;
//	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
//	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);
//	float  x0, y0, xn, yn, gx, gy;
//	x0 = (minRX + minSX)/2;
//	y0 = (minRY + minSY)/2;	
//	xn = (maxRX + maxSX)/2;
//	yn = (maxRY + maxSY)/2;
//	x0 +=pCellGrid->mDiffX;
//	y0 +=pCellGrid->mDiffY;
//
//	float OVTLengthX=pAttrAnalyzeParam->mRealEndAreaInOffset-pAttrAnalyzeParam->mRealBeginAreaInOffset;//无用
//	float OVTLengthY=pAttrAnalyzeParam->mRealEndAreaCLOffset-pAttrAnalyzeParam->mRealBeginAreaCLOffset;//无用
//
//	int CMPxbinnumber=pCellGrid->getNumX();
//	int CMPybinnumber=pCellGrid->getNumY();
//	int **tempfold;
//	gAlloc2(tempfold,CMPxbinnumber,CMPybinnumber);//临时存放CRP面元覆盖次数数组
//	for(int i=0;i<CMPxbinnumber;i++)
//		for(int j=0;j<CMPybinnumber;j++)
//			tempfold[i][j]=0;
//	//偏移距和方位角属性
//	GValueVector<short> **offsetarray;	
//	GValueVector<short> **Azimutharray;
//	gAlloc2(offsetarray,CMPxbinnumber,CMPybinnumber);
//	gAlloc2(Azimutharray,CMPxbinnumber,CMPybinnumber);
//	//赵虎添加
//	for(int i=0;i<CMPxbinnumber;i++)
//	{
//		for(int j=0;j<CMPybinnumber;j++)
//		{
//			for(int k=0;k<1;k++)
//			{
//				offsetarray[i][j].append(-1);
//				Azimutharray[i][j].append(-1);
//			}
//		}
//	}	//列表中第一个元素赋值为-1，为什么要这样做？leo
//	for(int ii=0;ii<pShotSet->count();ii++)
//	{
//		GShot *pShot = pShotSet->get(ii);
//		if(pShot != 0)
//		{
//			double sx=pShot->getSPoint()->getX();
//			double sy=pShot->getSPoint()->getY();
//			double ssx=pShot->getSPoint()->getSPSX();
//			double ssy=pShot->getSPoint()->getSPSY();
//			for(int si=0; si<pShot->getTrcCount(); si++)
//			{
//				GRPoint *rpoint = pShot->getRPointByTrcIndex(si,pRLineSet);
//				if(rpoint==NULL)
//					continue;
//				float rrx = rpoint->getSPSX();
//				float rry  = rpoint->getSPSY();
//				float rx = rpoint->getX();
//				float ry = rpoint->getY();
//
//				//int tInoffset=abs(rx-sx)/2;//临时存储的炮检距矢量（十字观测系统坐标系内的矢量）
//				//int tCLoffset=abs(ry-sy)/2;//为什么要除以2呢？--刘伟20230110
//
//				int tInoffset=rx-sx;// 临时存储的炮检距矢量（十字观测系统坐标系内的矢量）
//				int tCLoffset=ry-sy;// 炮检距矢量分量的绝对值 [Liu Wei 3/22/2023]
//
//				int tempx=(rx+sx)/2;
//				int tempy=(ry+sy)/2;
//				if(pAttrAnalyzeParam->mRealBeginAreaInOffset<=tInoffset && tInoffset<pAttrAnalyzeParam->mRealEndAreaInOffset)
//				{
//					if(pAttrAnalyzeParam->mRealBeginAreaCLOffset<=tCLoffset && tCLoffset<pAttrAnalyzeParam->mRealEndAreaCLOffset)
//					{
//						int i=(tempx-x0)/pCellGrid->mGridX;
//						int j=(tempy-y0)/pCellGrid->mGridY;
//						if(i>=0&&j>=0&&i<=CMPxbinnumber||j<=CMPybinnumber)
//						{
//							tempfold[i][j]++;
//
//							short offset=sqrt((float)((sx-rx)*(sx-rx)+(sy-ry)*(sy-ry)));//炮检距
//							if(offsetarray[i][j][0]==-1)
//							{
//								offsetarray[i][j][0]=offset;
//							}
//							else 
//							{
//								offsetarray[i][j].append(offset);
//							}
//							//求方位角
//							float temp1=(float)(rry-ssy);	
//							float temp2=(float)(rrx-ssx);	//炮点指向检波点的向量方位
//							float temp3=0;
//							if(temp2==0)	//temp==0表示在y轴上。此外，还应该判断dx的正负，确定是0°还是180°
//								temp3 = tanf(3.14159265/2);	//+无穷大，我建议直接赋值角度
//							else
//								temp3 = temp1/temp2;	//atan(x)/atanf(x)的值域(-pi/2,pi/2)
//							float temp=atanf(temp3)*180/3.14159265;//方位角角度，因为不论是atanf，atan2f输出的角度都是弧度值，需要进行转换。	
//							short tarray=0;//  [Liu Wei 3/22/2023]
//							if(rrx<ssx)	//极坐标系的三、四象限（上北下南左西右东，正北为0°，正东为90°，顺时针增加）
//							{
//								if(rry<ssy)	
//								{tarray=270-abs(temp);}	
//								else 
//								{tarray=270+abs(temp);}
//							}
//							else	   //极坐标系的一、二象限
//							{
//								if(rry<ssy)	
//								{tarray=90+abs(temp);}
//								else 
//								{tarray=90-abs(temp);}//与cmp线夹角，顺时针方向
//							}
//
//							if(Azimutharray[i][j][0]==-1)
//							{
//								Azimutharray[i][j][0]=tarray;
//							}
//							else 
//							{
//								Azimutharray[i][j].append(tarray);
//							}
//							//求方位角
//						}
//					}
//				}
//			}									
//		}
//	}
//	QString str="OVT Patch(Inline:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaInOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaInOffset)+",Crline:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaCLOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaCLOffset)+")";
//	int CMPmaxfold=0;
//	for(int i=0;i<CMPxbinnumber;i++)
//	{
//		for(int j=0;j<CMPybinnumber;j++)
//		{
//			if(tempfold[i][j]>CMPmaxfold) CMPmaxfold=tempfold[i][j];
//		}
//	}
//	if(CMPmaxfold==0)
//	{
//		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("最大覆盖次数为零"));//消息对话框显示提示用户违法操作
//		return;
//	}
//	analyzebinform *dlg=new analyzebinform(this);
//	bool valueanalyzebinWorkArea;
//	if(CMPmaxfold>0) 	valueanalyzebinWorkArea=true;
//	//dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,pSPSParam->mCircleR);
//	dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,0); //pSPSParam->mCircleR参数默认为0
//
//	//赵虎添加	
//	QString strAzimuth="";
//	QString strOffset=str;
//	dlg->setAODividNum(1,1,0,0,360,pCellGrid->mMaxoffset,strAzimuth,strOffset);//方位角和炮检距分的段数
//	dlg->CMPLayoutAttibuteConculate(tempfold);	
//	dlg->setDataValueworkarea(valueanalyzebinWorkArea);	
//	dlg->setAttribute(Qt::WA_DeleteOnClose,true);//关闭窗体时，销毁窗体，清空内存
//	dlg->exec();
//
//	if(offsetarray!=NULL) 
//	{
//		for(int i=0;i<CMPxbinnumber;i++)
//		{
//			for(int j=0;j<CMPybinnumber;j++)
//			{
//				offsetarray[i][j].clear();
//				Azimutharray[i][j].clear();
//			}
//		}	
//		gFree2(offsetarray);
//		gFree2(Azimutharray);	
//	}
//	gFree2(tempfold);
//	delete tempfold;
//	accept();
//}

void TRealOVTdlg::on_Cancel_clicked()
{
	reject();
}
