#include "datareconstruction.h"
#include "../gui/dialogs/tdatawizardform.h"
#include "../gui/survey/tspsform.h"
#include "../sampling/spsmaskbuilder.h"
#include <QProgressDialog>
#include <QDir>
#include <QMessageBox>
#include <omp.h>
#include "qt_windows.h"
#include <algorithm>
#include <numeric>

//根据正弦和余弦值反算角度 // add by wushanzhou in 2026/10/15
double gGetAngle(double sinAF, double cosAF)
{
	double af = asin(sinAF);//假设参数在[-1，1]范围内，则asin()函数返回[-π/ 2，π/ 2]范围内的值。

	if(cosAF < 0) af = PI-af;//钝角（第二、三象限）
	else if(sinAF < 0) af = 2*PI+af;//第四象限

	return af;//返回的角度为[0,2PI]
}

DataReconstruction::DataReconstruction(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	mpProject = GProject::getCurrentProject();
	if (mpProject) setProject(mpProject);
}

DataReconstruction::~DataReconstruction()
{
	// 暂时抛弃 [Liu Wei 5/29/2023]
	//if(mpProject!=NULL)
	//{
	//	mpProject->destroy();
	//	mpProject=NULL;
	//}
}

void DataReconstruction::setProject(GProject *pProject)
{
	mpProject = pProject;
	if (!pProject) return;
	GSLineSet *pSLineSet = (pProject->mIsNewImfo && pProject->getSLineSetNew() && pProject->getSLineSetNew()->count() > 0)
	                       ? pProject->getSLineSetNew()
	                       : ((pProject->getSLineSet() && pProject->getSLineSet()->count() > 0) ? pProject->getSLineSet() : pProject->getSLineSetNew());
	GRLineSet *pRLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
	                       ? pProject->getRLineSetNew()
	                       : ((pProject->getRLineSet() && pProject->getRLineSet()->count() > 0) ? pProject->getRLineSet() : pProject->getRLineSetNew());
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if (pSLineSet && pRLineSet && pSLineSet->count() > 0 && pRLineSet->count() > 0)
	{
		int totalS = 0, totalR = 0;
		for (int i = 0; i < pSLineSet->count(); i++)
			if (pSLineSet->get(i)) totalS += pSLineSet->get(i)->count();
		for (int i = 0; i < pRLineSet->count(); i++)
			if (pRLineSet->get(i)) totalR += pRLineSet->get(i)->count();
		ui.mTotalshot_num->setText(QString::number(totalS));
		ui.mTotalshotline_num->setText(QString::number(pSLineSet->count()));
		ui.mTotaltrace_num->setText(QString::number(totalR));
		ui.mTotalreceivingline_num->setText(QString::number(pRLineSet->count()));
		if (pSPSParam)
		{
			ui.mRline_interval->setText(QString::number(pSPSParam->mRLineInv));
			ui.mSline_interval->setText(QString::number(pSPSParam->mSLineInv));
			ui.mReceivingpoint_interval->setText(QString::number(pSPSParam->mRPtInv));
			ui.mShotpoint_interval->setText(QString::number(pSPSParam->mSPtInv));
			ui.mShot_num_PerSline->setText(QString::number(pSPSParam->mSPtCount));
		}
		ui.mDisplayandAnalyseBtn->setEnabled(true);
	}
}

//加载sps文件
void DataReconstruction::on_mLoadSPSFilesBtn_clicked()
{
	GProject *pProject = GProject::getCurrentProject();
	if(pProject == NULL) return;

	// 若已有SPS则清空
	if(pProject->getMaxSPSX() != 0)
	{
		pProject->clear();
		ui.mDisplayandAnalyseBtn->setEnabled(false);
	}

	TDataWizardForm *dlg = new TDataWizardForm(this);
	dlg->setProject(pProject); 

	int totalnum_spoint = 0;
	int totalnum_rpoint = 0;

	if(dlg->exec())
	{
		// 重新从工程中获取最新SPS对象，不能继续使用导入前指针
		GSLineSet *pSLineSet = pProject->getSLineSet();
		GRLineSet *pRLineSet = pProject->getRLineSet();
		GShotSet  *pShotSet  = pProject->getShotSet();

		if(pSLineSet == NULL || pRLineSet == NULL || pShotSet == NULL)
		{
			delete dlg;
			QMessageBox::warning(this,
								 QString::fromLocal8Bit("提示"),
								 QString::fromLocal8Bit("SPS导入后未能正确获取炮线/检波线/关系数据集。"));
			return;
		}

		if(pSLineSet->count() <= 0 || pRLineSet->count() <= 0 || pShotSet->count() <= 0)
		{
			delete dlg;
			QMessageBox::warning(this,
								 QString::fromLocal8Bit("提示"),
								 QString::fromLocal8Bit("SPS导入成功，但数据集为空。"));
			return;
		}

		if (pProject->getCellGrid() != NULL)
		{
			pProject->getCellGrid()->clear();
		}

		float sina = 0.0f, cosa = 1.0f;
		pRLineSet->getAngleParam(sina, cosa);//求接收线的倾角参数

		GSPSParam *pSPSParam = pProject->getSPSParam();
		pSPSParam->mSinAf = sina;
		pSPSParam->mCosAf = cosa;
		pSPSParam->mAf = gGetAngle(sina, cosa);//计算倾角-赵虎

		// 平移量初始化为最大值
		pSPSParam->mTransX0 = MAX_FLOAT;
		pSPSParam->mTransY0 = MAX_FLOAT;

		// 旋转到规则分析坐标系
		pRLineSet->rotate(sina, cosa, pSPSParam->mTransX0, pSPSParam->mTransY0);
		pSLineSet->rotate(sina, cosa, pSPSParam->mTransX0, pSPSParam->mTransY0);

		// 平移到局部坐标原点
		pRLineSet->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);
		pSLineSet->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);

		// -----------------------------
		// 从SPS扫描观测系统参数
		// -----------------------------
		int slnum = pSLineSet->count();//总炮线数
		int rlnum = pRLineSet->count();//总检波线数

		if (slnum <= 0 || rlnum <= 0)
		{
			delete dlg;
			QMessageBox::warning(this,
								 QString::fromLocal8Bit("提示"),
								 QString::fromLocal8Bit("导入的SPS中炮线数或检波线数为0。"));
			return;
		}

		float sline_int = 0.0f;
		float rline_int = 0.0f;
		float sp_int    = 0.0f;
		float rp_int    = 0.0f;

		totalnum_spoint = 0;
		totalnum_rpoint = 0;

		// 统计总接收点数
		for (int i = 0; i < rlnum; i++)
		{
			if (pRLineSet->get(i) != NULL)
				totalnum_rpoint += pRLineSet->get(i)->count();
		}

		// 统计总炮点数与每线炮点数
		QVector<int> scountperline;
		for (int i = 0; i < slnum; i++)
		{
			if (pSLineSet->get(i) != NULL)
			{
				int cnt = pSLineSet->get(i)->count();
				totalnum_spoint += cnt;
				scountperline.append(cnt);
			}
		}

		// 提取炮点数相同且最多的炮线的炮数
		int scountperline_v_nmax = 0;
		if (!scountperline.isEmpty())
		{
			std::sort(scountperline.begin(), scountperline.end());//升序
			QVector<int> scountperline_v, scountperline_n;

			int prior_v = scountperline.at(0);
			int ct = 1;
			for (int i = 1; i < scountperline.size(); i++)
			{
				if (scountperline.at(i) == prior_v)
				{
					ct++;
				}
				else
				{
					scountperline_v.append(prior_v);
					scountperline_n.append(ct);
					ct = 1;
					prior_v = scountperline.at(i);
				}
			}
			scountperline_v.append(prior_v);
			scountperline_n.append(ct);

			int maxi = scountperline_n.size() - 1;
			scountperline_v_nmax = gRound(scountperline_v.at(maxi) * 0.8);
		}

		// -------------------计算接收线距
		QList<float> rlint_list;
		for (int i = 1; i < rlnum; i++)
		{
			float sum_rlint = 0.0f;
			GRLine *line0 = pRLineSet->get(0);
			GRLine *irline = pRLineSet->get(i);
			if (line0 == NULL || irline == NULL) continue;

			int tcount = (line0->count() > irline->count()) ? irline->count() : line0->count();
			if (tcount <= 0) continue;

			for (int j = 0; j < tcount; j++)
			{
				sum_rlint += fabs(irline->get(j)->getY() - line0->get(j)->getY()) / tcount;
			}
			rlint_list.append(sum_rlint / i);
		}

		if (!rlint_list.isEmpty())
		{
			double rlint_summation = std::accumulate(rlint_list.begin(), rlint_list.end(), 0.0);
			rline_int = gRound(rlint_summation / rlint_list.size() / 5) * 5;
		}

		// -------------------计算检波点距（仅扫描一条接收线）
		GRLine *rline0 = pRLineSet->get(0);
		if (rline0 != NULL)
		{
			int ticount = rline0->count();
			float sum_rpint = 0.0f;
			if (ticount > 1)
			{
				for (int i = 1; i < ticount; i++)
				{
					sum_rpint += fabs(rline0->get(i)->getX() - rline0->get(0)->getX()) / i;
				}
				rp_int = gRound(sum_rpint / (ticount - 1) / 5) * 5;
			}
		}

		// -------------------计算炮线距
		QList<float> slint_list;
		for (int i = 0; i < slnum - 1; i++)
		{
			if (pSLineSet->get(i) == NULL || pSLineSet->get(i + 1) == NULL) continue;

			float sum_slint = 0.0f;
			if (pSLineSet->get(i)->count() >= scountperline_v_nmax &&
				pSLineSet->get(i + 1)->count() >= scountperline_v_nmax)
			{
				GSLine *isline  = pSLineSet->get(i);
				GSLine *isline1 = pSLineSet->get(i + 1);
				int tcount = (isline1->count() > isline->count()) ? isline->count() : isline1->count();
				if (tcount <= 0) continue;

				for (int j = 0; j < tcount; j++)
				{
					sum_slint += fabs(isline->get(j)->getX() - isline1->get(j)->getX()) / tcount;
				}
				slint_list.append(sum_slint);
			}
		}

		if (!slint_list.isEmpty())
		{
			double slint_summation = std::accumulate(slint_list.begin(), slint_list.end(), 0.0);
			sline_int = gRound(slint_summation / slint_list.size() / 5) * 5;
		}

		// -------------------计算炮点距
		QList<float> sp_int_list;
		for (int i = 0; i < slnum; i++)
		{
			GSLine *sline = pSLineSet->get(i);
			if (sline == NULL) continue;

			int count_si = sline->count();
			if (count_si >= scountperline_v_nmax)
			{
				float sum_sp_int = 0.0f;
				int count_j = 0;

				for (int j = 1; j < count_si; j++)
				{
					float temp_int = fabs(sline->get(j)->getY() - sline->get(j - 1)->getY());
					if (temp_int <= 100.0f)//假设炮点距不大于100m
					{
						sum_sp_int += temp_int;
						count_j++;
					}
				}

				if (count_j > 0)
				{
					sp_int_list.append(sum_sp_int / count_j);
				}
			}
		}

		if (!sp_int_list.isEmpty())
		{
			double sp_int_summation = std::accumulate(sp_int_list.begin(), sp_int_list.end(), 0.0);
			sp_int = gRound(sp_int_summation / sp_int_list.size() / 5) * 5;
		}

		// 显示导入参数
		ui.mTotalshot_num->setText(QString::number(totalnum_spoint));
		ui.mTotalshotline_num->setText(QString::number(slnum));
		ui.mTotaltrace_num->setText(QString::number(totalnum_rpoint));
		ui.mTotalreceivingline_num->setText(QString::number(rlnum));
		ui.mRline_interval->setText(QString::number(rline_int));
		ui.mSline_interval->setText(QString::number(sline_int));
		ui.mReceivingpoint_interval->setText(QString::number(rp_int));
		ui.mShotpoint_interval->setText(QString::number(sp_int));

		// -----------------------------
		// 只要已经成功导入SPS，就允许继续分析
		// -----------------------------
		ui.mDisplayandAnalyseBtn->setEnabled(true);

		// 缩小实际坐标扩边尺寸，改善显示效果
		pProject->mLength = pProject->getMaxSPSX() - pProject->getMinSPSX() + 2000;
		pProject->mWidth  = pProject->getMaxSPSY() - pProject->getMinSPSY() + 2000;
		pProject->mX0     = pProject->getMinSPSX() - 1000;
		pProject->mZ0     = pProject->getMinSPSY() - 1000;

#pragma region "提取观测系统模板参数（尽量识别，失败不再中断导入）"

		if (pShotSet->count() > 0)
		{
			GShot *tgt_shot0 = pShotSet->get(0);
			if (tgt_shot0 != NULL)
			{
				int tgt_count0    = tgt_shot0->count();
				int tgt_TrcCount0 = tgt_shot0->getTrcCount();

				int nMyRLineCountPerTemplate = 1;
				int nMyRPointCountPerLine    = 1;
				int nMySLineCountPerTemplate = 1;
				int nMySPointCountPerLine    = 1;

				if (tgt_count0 > 0)
				{
					nMyRLineCountPerTemplate = tgt_count0;
					nMyRPointCountPerLine = gRound(static_cast<double>(tgt_TrcCount0) / tgt_count0);
					if (nMyRPointCountPerLine <= 0) nMyRPointCountPerLine = 1;
				}

				// 给保守默认值，非规则系统也至少能继续用
				if (slnum > 0)
				{
					nMySPointCountPerLine = gRound(static_cast<double>(totalnum_spoint) / slnum);
					if (nMySPointCountPerLine <= 0) nMySPointCountPerLine = 1;
				}

				if (tgt_count0 == rlnum)//全线接收-横向不滚动
				{
					if (tgt_TrcCount0 == totalnum_rpoint)//纵向不滚动
					{
						nMySLineCountPerTemplate = slnum;
					}
					else if (tgt_TrcCount0 < totalnum_rpoint)//纵向滚动
					{
						int slnum_PerTemplate = 1;
						GSLine *sline0 = pSLineSet->get(0);
						if (sline0 != NULL && sline0->count() > 0)
						{
							//GShot *pShot0 = pShotSet->getByRecNum(sline0->get(0)->getRecNum());
							GShot *pShot0 = pShotSet->getByLinePoint(      //2026.3.31 by Wushanzhou
								sline0->get(0)->getLineName(),
								sline0->get(0)->getPointNum()
								);
							if (pShot0 != NULL && pShot0->count() > 0)
							{
								int maxTrc0 = pShot0->get(0)->getMaxTrc();
								int minTrc0 = pShot0->get(0)->getMinTrc();

								for (int ii = 1; ii < pSLineSet->count(); ii++)
								{
									GSLine *sline = pSLineSet->get(ii);
									if (sline == NULL || sline->count() <= 0) break;

									//GShot *pShot = pShotSet->getByRecNum(sline->get(0)->getRecNum());
									GShot *pShot = pShotSet->getByLinePoint(     //2026.3.31 by Wushanzhou
										sline->get(0)->getLineName(),
										sline->get(0)->getPointNum()
										);
									if (pShot == NULL || pShot->count() <= 0) break;

									int maxTrc = pShot->get(0)->getMaxTrc();
									int minTrc = pShot->get(0)->getMinTrc();
									if (maxTrc == maxTrc0 && minTrc == minTrc0)
										slnum_PerTemplate++;
									else
										break;
								}
							}
						}
						nMySLineCountPerTemplate = slnum_PerTemplate;
					}
					else
					{
						QMessageBox::warning(this,
							QString::fromLocal8Bit("提示"),
							QString::fromLocal8Bit("SPS已成功导入，但该观测系统不满足当前规则模板参数自动提取条件（关系总道数异常）。将采用保守默认值。"));

						nMySLineCountPerTemplate = 1;
					}
				}
				else if (tgt_count0 < rlnum)//部分线接收-横向滚动
				{
					if (tgt_count0 > 0 &&
						gRound(static_cast<double>(tgt_TrcCount0) / tgt_count0) ==
						gRound(static_cast<double>(totalnum_rpoint) / rlnum))//不纵向滚动
					{
						nMySLineCountPerTemplate = slnum;

						GSLine *sln_0 = pSLineSet->get(0);
						if (sln_0 != NULL)
						{
							int sn = sln_0->count();
							int spnum_PerShotline = 1;

							if (sn > 0)
							{
								//GShot *pShot0 = pShotSet->getByRecNum(sln_0->get(0)->getRecNum());
								GShot *pShot0 = pShotSet->getByLinePoint(     //2026.3.31 by Wushanzhou
									sln_0->get(0)->getLineName(),
									sln_0->get(0)->getPointNum()
									);

								if (pShot0 != NULL && pShot0->count() > 0)
								{
									for (int i = 1; i < sn; i++)
									{
										//GShot *pShot = pShotSet->getByRecNum(sln_0->get(i)->getRecNum());
										GShot *pShot = pShotSet->getByLinePoint(    //2026.3.31 by Wushanzhou
											sln_0->get(i)->getLineName(),
											sln_0->get(i)->getPointNum()
											);
										if (pShot != NULL && pShot->count() > 0 &&
											pShot->get(0)->getLineName() == pShot0->get(0)->getLineName() &&
											pShot->get(0)->getLineIndex() == pShot0->get(0)->getLineIndex())
										{
											spnum_PerShotline++;
										}
										else
										{
											break;
										}
									}
								}
							}
							nMySPointCountPerLine = spnum_PerShotline;
						}
					}
					else //横向+纵向滚动
					{
						int slnum_PerTemplate = 1;
						int spnum_PerShotline = 1;

						GSLine *sln_0 = pSLineSet->get(0);
						if (sln_0 != NULL && sln_0->count() > 0)
						{
							int sn = sln_0->count();
							GShot *pShot0 = pShotSet->getByRecNum(sln_0->get(0)->getRecNum());

							if (pShot0 != NULL && pShot0->count() > 0)
							{
								for (int i = 1; i < sn; i++)
								{
									//GShot *pShot = pShotSet->getByRecNum(sln_0->get(i)->getRecNum());
									GShot *pShot = pShotSet->getByLinePoint(     //2026.3.31 by Wushanzhou
										sln_0->get(i)->getLineName(),
										sln_0->get(i)->getPointNum()
										);
									if (pShot != NULL && pShot->count() > 0 &&
										pShot->get(0)->getLineName() == pShot0->get(0)->getLineName() &&
										pShot->get(0)->getLineIndex() == pShot0->get(0)->getLineIndex())
									{
										spnum_PerShotline++;
									}
									else
									{
										break;
									}
								}

								int maxTrc0 = pShot0->get(0)->getMaxTrc();
								int minTrc0 = pShot0->get(0)->getMinTrc();

								for (int ii = 1; ii < pSLineSet->count(); ii++)
								{
									GSLine *sln = pSLineSet->get(ii);
									if (sln == NULL || sln->count() <= 0) break;

									//GShot *pShot = pShotSet->getByRecNum(sln->get(0)->getRecNum());
									GShot *pShot = pShotSet->getByLinePoint(   //2026.3.31 by Wushanzhou
										sln->get(0)->getLineName(),
										sln->get(0)->getPointNum()
										);
									if (pShot == NULL || pShot->count() <= 0) break;

									int maxTrc = pShot->get(0)->getMaxTrc();
									int minTrc = pShot->get(0)->getMinTrc();
									if (maxTrc == maxTrc0 && minTrc == minTrc0)
										slnum_PerTemplate++;
									else
										break;
								}
							}
						}

						nMySPointCountPerLine = spnum_PerShotline;
						nMySLineCountPerTemplate = slnum_PerTemplate;
					}
				}
				else
				{
					QMessageBox::warning(this,
						QString::fromLocal8Bit("提示"),
						QString::fromLocal8Bit("SPS已成功导入，但该观测系统属于非规则几何，无法自动提取规则模板参数。将采用保守默认值。"));

					nMySLineCountPerTemplate = 1;
				}

				pProject->getSPSParam()->mRPtCount    = nMyRPointCountPerLine;
				pProject->getSPSParam()->mRLineCount  = nMyRLineCountPerTemplate;
				pProject->getSPSParam()->mSLineCount  = nMySLineCountPerTemplate;

				ui.mShot_num_PerSline->setText(QString::number(nMySPointCountPerLine));
			}
		}

#pragma endregion
	}

	delete dlg;
}

//显示新建SPS观测系统
void DataReconstruction::on_mDisplayandAnalyseBtn_clicked()
{
    GProject *pProject = GProject::getCurrentProject();
    if (pProject == NULL) return;

    // 传递参数给SPS参数管理类
    GSPSParam *pSPSParam = pProject->getSPSParam();
    pSPSParam->mRLineInv = ui.mRline_interval->text().toFloat();
    pSPSParam->mSLineInv = ui.mSline_interval->text().toFloat();
    pSPSParam->mRPtInv   = ui.mReceivingpoint_interval->text().toFloat();
    pSPSParam->mSPtInv   = ui.mShotpoint_interval->text().toFloat();
    pSPSParam->mSPtCount = ui.mShot_num_PerSline->text().toInt();

    // 关键：明确当前不是“新建观测系统”，而是“导入的SPS”
    pProject->mIsNewImfo = false;

    if (qobject_cast<TSPSForm*>(parentWidget()) != NULL)
    {
        accept();
        return;
    }
    // 启动观测系统分析模块
    TSPSForm *tSPSForm = new TSPSForm(this);
    tSPSForm->is2Dcal = false;
    tSPSForm->refresh();
    tSPSForm->exec();

    delete tSPSForm;
}

void DataReconstruction::on_mRegularOVTVolumeBtn_clicked()
{
	//启动观测系统OVT体属性分析模块
	GeometryTemplateParameter *TemplateParameter_dlg = new GeometryTemplateParameter(this);
	if (TemplateParameter_dlg->exec())
	{
		//定义自变量参数
		int	Num_ShotPerSLine, Num_RecPerRecLine, Num_RecLine, Num_Sline;
		float int_shotpoint, int_receiver, int_RecLine, int_SLine;
		float Hxi_receiver, Vyi_receiver, Hxi_shot, Vyi_shot;
		float maxDis_nonlongitudinal; // Maximum non-longitudinal distance （最大非纵炮检距）
		float maxDis_longitudinal; // Maximum longitudinal distance （最大纵向炮检距）
		float SXmin, SYmin; //第一个炮点的坐标

		//复制模板参数
		Num_ShotPerSLine=TemplateParameter_dlg->getSpointNumPerSline();		//***
		Num_RecPerRecLine=TemplateParameter_dlg->getChannelNumPerRline();	//**
		Num_RecLine=TemplateParameter_dlg->getRlineNumPerTemplate();		//**
		Num_Sline=TemplateParameter_dlg->getSlineNumPerTemplate();			//**
		int_shotpoint=TemplateParameter_dlg->getSpointInterval();			//*****
		int_receiver=TemplateParameter_dlg->getRpointInterval();			//*****
		int_RecLine=TemplateParameter_dlg->getRlineInterval();				//*****
		int_SLine=TemplateParameter_dlg->getSlineInterval();				//*****

		//默认设定了坐标原点为观测系统中第一条检波线的第一个检波点
		SXmin=((Num_RecPerRecLine-1)*int_receiver-(Num_Sline-1)*int_SLine)/2;
		SYmin=((Num_RecLine-1)*int_RecLine-(Num_ShotPerSLine-1)*int_shotpoint)/2;

		//采用用户给定的炮点最小坐标
		float SX0=TemplateParameter_dlg->getFirstSpointX0();
		float SY0=TemplateParameter_dlg->getFirstSpointY0();
		if (int(SX0)!=int(SXmin) || int(SY0)!=int(SYmin))
		{
			SXmin=SX0; SYmin=SY0;
		}

		//最大非纵距和最大纵向距离其实是最大炮检距纵、横向分量的大小
		maxDis_nonlongitudinal=(Num_RecLine-1)*int_RecLine - SYmin;
		maxDis_longitudinal=(Num_RecPerRecLine-1)*int_receiver - SXmin;

		float gy, gx;
		int maxgys_y=gcd( int(int_RecLine),int(int_shotpoint) );
		int maxgys_x=gcd( int(int_receiver),int(int_SLine) );
		if (maxgys_y <= 0) maxgys_y = 1;
		if (maxgys_x <= 0) maxgys_x = 1;
		gy=int(int_shotpoint*int_RecLine)/maxgys_y;
		gx=int(int_receiver*int_SLine)/maxgys_x;
		if (gy <= 0) gy = 50.0f;
		if (gx <= 0) gx = 50.0f;

		int max_gyi, min_gyi;
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

		int max_gxi = int(maxDis_longitudinal/(2*gx)+1);
		int min_gxi = -int(maxDis_longitudinal/(2*gx)+1);
		if (max_gyi < 1) { max_gyi = 1; min_gyi = -1; }
		if (max_gxi < 1) { max_gxi = 1; min_gxi = -1; }

		int ny_OVT=max_gyi-min_gyi;
		int	nx_OVT=max_gxi-min_gxi;
		QList<int> gpnumber_y, gpnumber_x;
		for (int iy=min_gyi; iy<=max_gyi; iy++)
		{
			if (iy!=0) gpnumber_y.append(iy);
		}
		for (int ix=min_gxi; ix<=max_gxi; ix++)
		{
			if (ix!=0) gpnumber_x.append(ix);
		}

		GValueVector<short> **temp_offset;
		GValueVector<short> **temp_azimuth;
		gAlloc2(temp_offset,nx_OVT,ny_OVT);
		gAlloc2(temp_azimuth,nx_OVT,ny_OVT);

		for (int i=0; i<Num_Sline; i++)
		{
			float sij_x=SXmin+i*int_SLine;
			for (int j=0; j<Num_ShotPerSLine; j++)
			{
				float sij_y=SYmin+j*int_shotpoint;
				for (int m=0; m<Num_RecLine; m++)
				{
					float rmn_y=m*int_RecLine;
					for (int n=0; n<Num_RecPerRecLine; n++)
					{
						float rmn_x=n*int_receiver;
						int tInoffset=rmn_x-sij_x;
						int tCLoffset=rmn_y-sij_y;

						int tempxi,tempyi;
						if (tInoffset>0) tempxi=int(tInoffset/(2*gx))+1;
						else             tempxi=int(tInoffset/(2*gx))-1;

						if (tCLoffset>0) tempyi=int(tCLoffset/(2*gy))+1;
						else             tempyi=int(tCLoffset/(2*gy))-1;

						if (tempxi>max_gxi) tempxi=max_gxi;
						else if (tempxi<min_gxi) tempxi=min_gxi;

						if (tempyi>max_gyi) tempyi=max_gyi;
						else if (tempyi<min_gyi) tempyi=min_gyi;

						int xi=gpnumber_x.indexOf(tempxi);
						int yi=gpnumber_y.indexOf(tempyi);
						if (xi < 0 || xi >= nx_OVT || yi < 0 || yi >= ny_OVT) continue;

						float tOffset=sqrtf(tInoffset*tInoffset+tCLoffset*tCLoffset);
						temp_offset[xi][yi].append(short(tOffset));

						float temp1=(float)(rmn_y-sij_y);
						float temp2=(float)(rmn_x-sij_x);
						float atgt_deg=0.0f;

						if(temp2==0)
						{
							if (temp1 > 0) atgt_deg=0;
							else if (temp1 < 0) atgt_deg=180;
						}
						else
						{
							float temp=atanf(temp1/temp2)*180/PI;
							if(rmn_x<sij_x)
							{
								if(rmn_y<sij_y) atgt_deg=270-abs(temp);
								else            atgt_deg=270+abs(temp);
							}
							else
							{
								if(rmn_y<sij_y) atgt_deg=90+abs(temp);
								else            atgt_deg=90-abs(temp);
							}
						}
						temp_azimuth[xi][yi].append(short(atgt_deg));
					}
				}
			}
		}

		OVTVolumeAttributeView *VolumeAttributeview=new OVTVolumeAttributeView(this);
		VolumeAttributeview->setOVTcubeData(temp_offset, temp_azimuth, nx_OVT, ny_OVT, 2*gx, 2*gy, maxDis_longitudinal, maxDis_nonlongitudinal);
		VolumeAttributeview->exec();

		delete VolumeAttributeview;
		VolumeAttributeview=NULL;

		if(temp_offset!=NULL)
		{
			for(int i=0;i<nx_OVT;i++)
			{
				for(int j=0;j<ny_OVT;j++)
				{
					temp_offset[i][j].clear();
					temp_azimuth[i][j].clear();
				}
			}
			gFree2(temp_offset);
			gFree2(temp_azimuth);
		}

		// 新增：生成并导出 CMP mask（导出到当前工作目录） // add by wushanzhou in 2026/10/15
		GProject *pProject = GProject::getCurrentProject();
		if (pProject != NULL)
		{
			SPSMaskBuilder builder;

			float binX = pProject->getHCMPInv();
			float binY = pProject->getVCMPInv();

			if (binX <= 0) binX = 12.5f;
			if (binY <= 0) binY = 12.5f;

			if (!builder.buildCMPMaskFromProject(pProject, binX, binY))
			{
				QMessageBox::warning(this,
									 QString::fromLocal8Bit("提示"),
									 QString::fromLocal8Bit("生成 CMP mask 失败"));
			}
			else
			{
				QString outDir = QDir::currentPath();
				builder.exportCMPMaskTxt(outDir + "/cmp_mask.txt");
				builder.exportCMPFoldTxt(outDir + "/cmp_fold.txt");
				builder.exportCMPMetaTxt(outDir + "/cmp_meta.txt");

				QMessageBox::information(this,
										 QString::fromLocal8Bit("提示"),
										 QString::fromLocal8Bit("CMP mask 和 fold 已导出到:\n") + outDir);
			}
		}
	}

	delete TemplateParameter_dlg;
}

void DataReconstruction::on_cancelBtn_clicked()
{
	reject();
}
