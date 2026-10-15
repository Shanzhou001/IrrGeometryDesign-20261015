/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <qdialog.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QLabel>
#include <QCloseEvent>
#include "gproject.h"
#include "tspsdlg.h"
//#include "tspscrookdlg.h"
#include "tcellgridform.h"
#include "gcellgrid.h"
#include "tspsform.h"
#include "geometryexperimentengine.h" // add by shanzhou in 2026/10/15
#include <QDesktopWidget>
//#include "tbendlinewidthdlg.h"
#include "gutils.h"
//#include "analyzebinform.h"
#include <QProgressDialog>
//#include "ggausstracer.h"
//#include "gmblock3d.h"
//#include "gaabbox.h"
//#include "gmsurface.h"
//#include "gspline3d.h"
//#include "tsetshowmaxfolddlg.h"
#include <QVector>
#include <qobject.h>
#include <QWidget>
#include <QTimer>
#include <QTextStream>
#include <QMessageBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <QDir>
TSPSForm::TSPSForm( QWidget* parent ):Q3MainWindow(parent)    //add 2026/4/14 by Wushanzhou
{
    ui.setupUi(this);

    // 既兼容程序主入口，也兼容“显示已导入SPS”的复用场景
    mpProject = GProject::getCurrentProject();
    if(mpProject == NULL)
    {
        mpProject = GProject::create();
        mOwnProject = true;
    }
    else
    {
        mOwnProject = false;
    }

    init();

    Azimutharray = NULL;
    offsetarray = NULL;
}

TSPSForm::~TSPSForm()
{
    // 指针数据内存空间销毁
    if(offsetarray != NULL)
    {
        GProject *pProject = GProject::getCurrentProject();
        GCellGrid *pCellGrid = pProject->getCellGrid();

        int CMPxbinnumber = pCellGrid->getNumX();
        int CMPybinnumber = pCellGrid->getNumY();

        for(int i = 0; i < CMPxbinnumber; i++)
        {
            for(int j = 0; j < CMPybinnumber; j++)
            {
                offsetarray[i][j].clear();
                Azimutharray[i][j].clear();
            }
        }

        gFree2(offsetarray);
        gFree2(Azimutharray);
    }

#pragma region "add by Wushanzhou"
    if(mOwnProject && mpProject != NULL)
    {
        mpProject->destroy();
    }
    mpProject = NULL;
    destroy();
#pragma endregion "add by Wushanzhou"
}

void TSPSForm::init()
{
	//界面中的工具栏是怎么来的，自动添加的吗？刘伟
	//----调整窗口大小和位置----
	QPoint p( 0, 0 );
	QRect desk;
	int top = 0, left = 0, width = 800, height = 600;
	desk = QApplication::desktop()->availableGeometry( 1 );

	left = ( desk.width()-width )/2;
	top = ( desk.height()-height )/2;
	setGeometry( left, top, width, height );

	mSpl=new QSplitter(this);
	mSpl->setOrientation(Qt::Vertical);
	mpView = new GSPSView(mSpl);

	mpView->mIsImpotSps=true;//设置显示观测系统类型（内建观测系统还是外部输入观测系统）

	mpView->show();

	QLabel *xStatusLabel = new QLabel( statusBar() );
	xStatusLabel->setMinimumWidth( 80 );
	xStatusLabel->setText( "X: " );
	statusBar()->addWidget( xStatusLabel, 0, true );
	mpView->setXLabel(xStatusLabel);

	QLabel *yStatusLabel = new QLabel( statusBar() );
	yStatusLabel->setMinimumWidth( 80 );
	yStatusLabel->setText( "Y: " );
	statusBar()->addWidget( yStatusLabel, 0, true );
	mpView->setYLabel(yStatusLabel);
	mpView->setStatusBar(statusBar());

	GProject *pProject = GProject::getCurrentProject();
	double scale = pProject->getLength() / desk.width();
	double sh = pProject->getWidth() / desk.height();
	if(scale < sh) scale = sh;
	mpView->setScale(scale);//实际坐标范围与绘图坐标范围的比值

	mIsFill = false;
	mIsLiner = false;
	mIsMouseZoom = false;
	connect(mpView, SIGNAL(zoomScaleChanged(double, double, double, double)), this, 
		SLOT(onZoomScaleChanged(double, double, double, double)));

	QList<int> list;
	list.append(400);
	list.append(100);
	list.append(300);
	mSpl->setSizes(list);
	// 连接保存、缩放、网格显示动作 // add by shanzhou in 2026/10/15
	connect(ui.saveAction, SIGNAL(triggered()), this, SLOT(saveSPS()));
	connect(ui.zoomAction, SIGNAL(triggered()), this, SLOT(zoom()));
	connect(ui.gridAction, SIGNAL(triggered()), this, SLOT(grid()));
    connect(ui.actionRunSamplingBenchmark, SIGNAL(triggered()), this, SLOT(on_actionRunSamplingBenchmark_triggered()));
    connect(ui.actionExportAttributeReport, SIGNAL(triggered()), this, SLOT(on_actionExportAttributeReport_triggered()));
    connect(ui.actionRunWavefieldReconstruction, SIGNAL(triggered()), this, SLOT(on_actionRunWavefieldReconstruction_triggered()));
    connect(ui.actionRunFullExperimentSuite, SIGNAL(triggered()), this, SLOT(on_actionRunFullExperimentSuite_triggered()));
	setCentralWidget(mSpl);
}

void TSPSForm::destroy()
{
	delete mpView;
}

void TSPSForm::closeEvent( QCloseEvent *e )
{
	Q3MainWindow::closeEvent( e );

	qApp->exit_loop();
}

int TSPSForm::exec()
{
	show();
	qApp->enter_loop();	
	if(mIsFill) return QDialog::Accepted;
	else return QDialog::Rejected;
}

void TSPSForm::refresh()
{
	mpView->refresh();
	updateExperimentActionsState(); // add by shanzhou in 2026/10/15
}

//新建sps文件
void TSPSForm::on_newspsAction_triggered()
{
	GProject *pProject = GProject::getCurrentProject();
	mpView->stop();
	TSPSDlg *tSPSDlg = new TSPSDlg(this);	

	GSPSParam *pSPSParam = pProject->getSPSParam();

	tSPSDlg->setSPS(pSPSParam);

	if(tSPSDlg->exec())
	{
		//----------------------------------------------将模型原点坐标传入观测系统类中
		if(pSPSParam->mOrginalByReceiver==true)//以第一个检波点为原点
		{
			// 全部注释掉，因为在对话框中有自动操作 [Liu Wei 5/31/2023]
			//GRLineSet *pRLineSet = pProject->getRLineSet();
			//if(pRLineSet == NULL) return ;
			//int beginX=0;
			//int beginY=1000000000;
			//
			//for(int i=0; i<pRLineSet->count(); i++)
			//{
			//	GRLine *rLine = pRLineSet->get(i);
			//	{
			//		for(int j=0;j<rLine->count();j++)
			//		{
			//			GRPoint *rPoint=rLine->get(j);
			//			if(rPoint->getSPSY()<beginY) //找y坐标最小的点
			//			{
			//				beginX=rPoint->getSPSX();
			//				beginY=rPoint->getSPSY();
			//			}
			//		}
			//	}		
			//}
			//pSPSParam->mRX0=beginX;
			//pSPSParam->mRY0=beginY;	

			//beginX=pRLineSet->get(0)->get(0)->getSPSX();
			//beginY=pRLineSet->get(0)->get(0)->getSPSY();
			//pSPSParam->mRX0=pRLineSet->get(0)->get(0)->getSPSX();
			//pSPSParam->mRY0=pRLineSet->get(0)->get(0)->getSPSY();

			//double sx0,sy0;
			//gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,sx0,sy0);
			//sx0=sx0+pSPSParam->mMoveX0;//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
			//sy0=sy0+pSPSParam->mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
			//double SX0,SY0;
			//gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,sx0,sy0,SX0,SY0);//计算检波点起始坐标
			//pSPSParam->mSX0=SX0;
			//pSPSParam->mSY0=SY0;
			// 全部注释掉，因为在对话框中有自动操作 [Liu Wei 5/31/2023]
		}

		if(pSPSParam->mOrginalByShot==true)//以第一个炮点为原点
		{
			GShotSet *pShotSet = pProject->getShotSet();
			if(pShotSet == NULL) return ;

			int beginX=0;
			int beginY=1000000000;

			/*//modify by zhaohu
			for(int i=0;i<pShotSet->count();i++)
			{
			GShot *pShot = pShotSet->get(i);//
			if(pShot->getSPoint()->getSPSX()>beginX)//找x坐标最大值
			{
			beginX=pShot->getSPoint()->getSPSX();

			}
			if(pShot->getSPoint()->getSPSY()<beginY)//找y坐标最小值
			{
			beginY=pShot->getSPoint()->getSPSY();
			}

			}
			pSPSParam->mSX0=beginX;
			pSPSParam->mSY0=beginY;	
			*///modify by zhaohu

			beginX=pShotSet->get(0)->getSPoint()->getSPSX();//add by zhaohu
			beginY=pShotSet->get(0)->getSPoint()->getSPSY();//add by zhaohu
			pSPSParam->mSX0=pShotSet->get(0)->getSPoint()->getSPSX();//add by zhaohu
			pSPSParam->mSY0=pShotSet->get(0)->getSPoint()->getSPSY();//add by zhaohu	

			/*double rx0,ry0;
			gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
			rx0=rx0+pSPSParam->mMoveX0;//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
			ry0=ry0+pSPSParam->mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）*/

			double sinAf = pSPSParam->mSinAf;//在函数开始处缓存不变参数
			double cosAf = pSPSParam->mCosAf;
			double moveX0 = pSPSParam->mMoveX0;
			double moveY0 = pSPSParam->mMoveY0;
			
			double rx0,ry0;//在需要计算时直接使用缓存的值
			gRotate(-sinAf,cosAf,beginX,beginY,rx0,ry0);
			rx0 += moveX0;
			ry0 += moveY0;


			double RX0,RY0;
			gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
			pSPSParam->mRX0=RX0;
			pSPSParam->mRY0=RY0;
		}
		//----------------------------------------------将模型原点坐标传入观测系统类中


		GRLineSet *pRLineSetNew = pProject->getRLineSetNew();
		pRLineSetNew->create(pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]

		GSLineSet *pSLineSetNew = pProject->getSLineSetNew();
		pSLineSetNew->create(pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]
		//pSLineSetNew->createByold(pProject->getSLineSet());//利用老的炮线建立新炮线 add by zhaohu

		//float TransX0=pProject->getTransX0();
		//float TransY0=pProject->getTransY0();
		//pRLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, TransX0, TransY0);
		//TransX0=pProject->getTransX0();
		//TransY0=pProject->getTransY0();
		//pSLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, TransX0, TransY0);
		////TransX0=pProject->getTransX0();
		////TransY0=pProject->getTransY0();
		//pRLineSetNew->translate(-TransX0, -TransY0);
		//pSLineSetNew->translate(-TransX0, -TransY0);//平移坐标原点，

		pSPSParam->mTransX0 = MAX_FLOAT;	
		pSPSParam->mTransY0 = MAX_FLOAT;

		pRLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
		pSLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
		pRLineSetNew->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);
		pSLineSetNew->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);//平移坐标原点

		mpProject->setTransX0(pSPSParam->mTransX0);
		mpProject->setTransY0(pSPSParam->mTransY0);

		// 获取新建观测系统坐标范围并进行回去区间控制参数赋值，否则观测系统显示有问题（默认显示坐标范围0-8000） [Liu Wei 6/19/2023]
		float mMinSPSX = MAX_FLOAT;	float mMinSPSY = MAX_FLOAT;
		float mMaxSPSX = MIN_FLOAT;	float mMaxSPSY = MIN_FLOAT;
		// 工程对象中的对应参数是否需要传值需进一步测试确认！ [Liu Wei 6/19/2023]
		pRLineSetNew->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);
		pSLineSetNew->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);

		GShotSet *pShotSetNew = pProject->getShotSetNew();
		pShotSetNew->create(pSPSParam,pSLineSetNew,pRLineSetNew);//  [Liu Wei 5/31/2023]
		

		// 进行坐标扩边 [Liu Wei 6/19/2023]
		// 扩边的作用是为了保证显示美观（四周有足够的留白） [Liu Wei 3/29/2023]
		pProject->mLength=mMaxSPSX-mMinSPSX+2000;
		pProject->mWidth=mMaxSPSY-mMinSPSY+2000;
		pProject->mX0=mMinSPSX-1000;// 为了保证程序能够正常运行，新建观测系统坐标X（最小值）必须设置为大于1000 [Liu Wei 6/19/2023]
		pProject->mZ0=mMinSPSY-1000;// 为了保证程序能够正常运行，新建观测系统坐标Y（最小值）必须设置为大于1000 [Liu Wei 6/19/2023]
		// 获取新建观测系统坐标范围并进行回去区间控制参数赋值，否则观测系统显示有问题（默认显示坐标范围0-8000） [Liu Wei 6/19/2023]

		mIsFill = true;
		mIsLiner = true ;

		mpView->mIsImpotSps=false;//add by zhaohu 是否显示输入的观测系统
		//工程对象内部也要同步设置指示标识参数
		mpProject->mIsNewImfo=true; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]

		refresh();//add by zhaohu 显示不同的sps后，重新刷新一下视图

		mpView->repaintContents();

		// 激活观测系统μ值计算和优化功能 [Liu Wei 8/13/2023]
		ui.actionRSample->setEnabled(true);
		ui.actionRAOptimizing->setEnabled(true);
		ui.actionSSample->setEnabled(true);
		ui.actionSAOptimizing->setEnabled(true);
	}
	delete tSPSDlg;
}

// 保存与导出 SPS 观测系统（含 .r/.s/.x 及 CSV 坐标表）
void TSPSForm::saveSPS() // add by shanzhou in 2026/10/15
{
    GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProject) {
        QMessageBox::warning(this, QString::fromLocal8Bit("错误"),
                             QString::fromLocal8Bit("当前工程为空，无法保存。"));
        return;
    }

    GSLineSet *pSLineSet = pProject->getSLineSet();
    GRLineSet *pRLineSet = pProject->getRLineSet();

    int sCount = (pSLineSet != NULL) ? pSLineSet->count() : 0;
    int rCount = (pRLineSet != NULL) ? pRLineSet->count() : 0;

    if (sCount == 0 && rCount == 0) {
        QMessageBox::warning(this, QString::fromLocal8Bit("错误"),
                             QString::fromLocal8Bit("当前工程中没有观测系统数据（炮线或检波线），无法保存。"));
        return;
    }

    // 默认保存路径为当前工作目录
    QString defaultPath = QDir::current().filePath("opt_geometry.sps");

    QString fileName = QFileDialog::getSaveFileName(
        this,
        QString::fromLocal8Bit("导出观测系统（SPS三文件及坐标表）"),
        defaultPath,
        QString::fromLocal8Bit("SPS 观测系统基名 (*.sps);;SPS 接收点文件 (*.r);;所有文件 (*.*)")
    );

    if (fileName.isEmpty())
        return;

    // 执行保存
    if (!pProject->saveSPS(fileName)) {
        QMessageBox::warning(this, QString::fromLocal8Bit("错误"),
                             QString::fromLocal8Bit("导出 SPS 文件失败，请检查文件写入权限或磁盘路径！"));
        return;
    }

    // 获取导出文件具体路径
    QFileInfo fi(fileName);
    QString dir  = fi.absolutePath();
    QString base = fi.completeBaseName();
    if (base.isEmpty()) base = fi.fileName();
    QDir qdir(dir);
    QString prefix = qdir.filePath(base);

    QString sFileName = prefix + ".s";
    QString rFileName = prefix + ".r";
    QString xFileName = prefix + ".x";
    QString csvFileName = prefix + "_receiver_coords.csv";

    // 额外导出一份通用的 CSV 坐标清单，方便论文作图、数据审计或在 Excel/Python 中直接处理
    int totalRPoints = 0;
    if (pRLineSet != NULL)
    {
        QFile csvFile(csvFileName);
        if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream csvStream(&csvFile);
            csvStream << "LineName,PointNum,X,Y,Z\n";
            for (int i = 0; i < pRLineSet->count(); i++)
            {
                GRLine *pLine = pRLineSet->get(i);
                if (!pLine) continue;
                for (int j = 0; j < pLine->count(); j++)
                {
                    GRPoint *pt = pLine->get(j);
                    if (!pt) continue;
                    totalRPoints++;
                    csvStream << pt->getLineName() << ","
                              << pt->getPointNum() << ","
                              << QString::number(pt->getSPSX(), 'f', 2) << ","
                              << QString::number(pt->getSPSY(), 'f', 2) << ","
                              << QString::number(pt->getSPSZ(), 'f', 2) << "\n";
                }
            }
            csvFile.close();
        }
    }

    int totalSPoints = 0;
    if (pSLineSet != NULL)
    {
        for (int i = 0; i < pSLineSet->count(); i++)
        {
            GSLine *pLine = pSLineSet->get(i);
            if (pLine) totalSPoints += pLine->count();
        }
    }

    QString msg = QString::fromLocal8Bit("SPS 观测系统文件导出成功！\n\n");
    msg += QString::fromLocal8Bit("1. 检波点文件 (R):\n   ") + rFileName + QString::fromLocal8Bit(" (共 %1 点)\n\n").arg(totalRPoints);
    msg += QString::fromLocal8Bit("2. 炮点文件 (S):\n   ") + sFileName + QString::fromLocal8Bit(" (共 %1 点)\n\n").arg(totalSPoints);
    msg += QString::fromLocal8Bit("3. 炮检关系 (X):\n   ") + xFileName + "\n\n";
    msg += QString::fromLocal8Bit("4. 坐标清单 (CSV):\n   ") + csvFileName;

    QMessageBox::information(this, QString::fromLocal8Bit("导出成功"), msg);
}

void TSPSForm::on_saveAction_triggered()
{
    saveSPS();
}

void TSPSForm::on_zoomAction_triggered()
{
    zoom();
}

void TSPSForm::on_gridAction_triggered()
{
    grid();
}

void TSPSForm::onZoomScaleChanged(double zoomScaleX, double zoomScaleY,
								  double preZoomScaleX, double preZoomScaleY)
{
	mIsMouseZoom = true;
	ui.mScaleBox->setValue(zoomScaleX*100);
	mIsMouseZoom = false;
}

void TSPSForm::zoom()
{
	mpView->start(GSPSView::gZoom);
}


void TSPSForm::onZoomChanged( int value )
{
	if(mIsMouseZoom) return ;
	mpView->setZoomScaleX((double)value / 100.0);
	mpView->setZoomScaleY((double)value / 100.0);
	mpView->refresh();
}


void TSPSForm::grid()
{
	TCellGridForm *gridForm = new TCellGridForm(this);
	if(gridForm->exec() == QDialog::Accepted)
	{
		mpView->repaintContents();
	}

	delete gridForm;
	//  [Liu Wei 3/28/2023]
	ui.actionGeometry_Attribute_Plotting->setEnabled(true);
	updateExperimentActionsState(); // add by shanzhou in 2026/10/15
	ui.actionOVT_Volume_Attributes->setEnabled(true);
	ui.actionOVT_Patch_Attributes->setEnabled(true);
}

//显示新建的sps
void TSPSForm::on_ShowNewSPS_triggered()
{
	mpView->mIsImpotSps=false;//add by zhaohu 是否显示输入的观测系统
	//工程对象内部也要同步设置指示标识参数
	mpProject->mIsNewImfo=true; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]
	refresh();//add by zhaohu 显示不同的sps后，重新刷新一下视图
}
//显示外部输入的sps
void TSPSForm::on_ShowImportSPS_triggered()
{
	mpView->mIsImpotSps=true;//add by zhaohu 是否显示输入的观测系统
	//工程对象内部也要同步设置指示标识参数
	mpProject->mIsNewImfo=false; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]
	refresh();//add by zhaohu 显示不同的sps后，重新刷新一下视图
}
//计算并显示常规观测系统属性
void TSPSForm::on_actionGeometry_Attribute_Plotting_triggered()
{
	GProject *pProject = GProject::getCurrentProject();
	GSLineSet *pSLineSet = pProject->getSLineSet();
	GRLineSet *pRLineSet = pProject->getRLineSet();
	GShotSet *pShotSet = pProject->getShotSet();
	GCellGrid *pCellGrid = pProject->getCellGrid();
	GSPSParam *pSPSParam = pProject->getSPSParam();

	//赵虎添加
	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;

	minSX = minSY = LONG_MAX;
	maxSX = maxSY = LONG_MIN;
	minRX = minRY = LONG_MAX;
	maxRX = maxRY = LONG_MIN;

	//pSLineSet->getSPSRange(minSX, minSY, maxSX, maxSY);//赵老师的原始代码
	//pRLineSet->getSPSRange(minRX, minRY, maxRX, maxRY);//赵老师的原始代码
	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);//  [Liu Wei 3/20/2023]
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);//  [Liu Wei 3/20/2023]

	float  x0, y0, xn, yn, gx, gy;
	x0 = (minRX + minSX)/2;
	y0 = (minRY + minSY)/2;	
	xn = (maxRX + maxSX)/2;
	yn = (maxRY + maxSY)/2;

	x0 +=pCellGrid->mDiffX;
	y0 +=pCellGrid->mDiffY;


	float minbxcoord=x0;
	float minbycoord=y0;
	float maxbxcoord=xn;
	float maxbycoord=yn;

	int CMPxbinnumber=pCellGrid->getNumX();
	int CMPybinnumber=pCellGrid->getNumY();

	int **tempfold;
	gAlloc2(tempfold,CMPxbinnumber,CMPybinnumber);//临时存放CRP面元覆盖次数数组
	for(int i=0;i<CMPxbinnumber;i++)
		for(int j=0;j<CMPybinnumber;j++)
			tempfold[i][j]=0;

	int CMPmaxfold=0;

	for(int i=0;i<CMPxbinnumber;i++)
	{
		for(int j=0;j<CMPybinnumber;j++)
		{
			tempfold[i][j] = pCellGrid->getCoverCount()[j][i];
			if(CMPmaxfold<tempfold[i][j]) CMPmaxfold=tempfold[i][j];//寻找最大覆盖次数
		}
	}
	gFree2(tempfold);//仅仅是为了寻找最大覆盖次数
	//防止内存泄露
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
	//申请内存空间



	gAlloc2(offsetarray,CMPxbinnumber,CMPybinnumber);
	gAlloc2(Azimutharray,CMPxbinnumber,CMPybinnumber);
	//初始化数据
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
	}

#pragma omp parallel for
	for(int ii=0;ii<pShotSet->count();ii++)
	{
		GShot *pShot = pShotSet->get(ii);
		if(pShot != 0)
		{
			//getRandomOffsetX()和getRandomOffsetY()有什么作用？请教赵老师20230117;
			float offsetx = pShot->getSPoint()->getRandomOffsetX();//使用该函数便于加速运算
			float offsety = pShot->getSPoint()->getRandomOffsetY();//使用该函数便于加速运算
			//double sx=pShot->getSPoint()->getSPSX()+offsetx;//赵老师的原始代码
			//double sy=pShot->getSPoint()->getSPSY()+offsety;//赵老师的原始代码
			double sx=pShot->getSPoint()->getX()+offsetx;//  [Liu Wei 3/20/2023]
			double sy=pShot->getSPoint()->getY()+offsety;//  [Liu Wei 3/20/2023]
			double ssx=pShot->getSPoint()->getSPSX()+offsetx;
			double ssy=pShot->getSPoint()->getSPSY()+offsety;
			for(int si=0; si<pShot->getTrcCount(); si++)
			{
				GRPoint *rpoint = pShot->getRPointByTrcIndex(si,pRLineSet);
				if(rpoint==NULL)
					continue;
				//float rx = rpoint->getSPSX()+offsetx;//赵老师的原始代码
				//float ry = rpoint->getSPSY()+offsety;//赵老师的原始代码
				float rx = rpoint->getX()+offsetx;//  [Liu Wei 3/20/2023]
				float ry = rpoint->getY()+offsety;//  [Liu Wei 3/20/2023]
				float rrx = rpoint->getSPSX()+offsetx;
				float rry  = rpoint->getSPSY()+offsety;

				float tempcmpx = (rx+sx)/2;
				float tempcmpy = (ry+sy)/2;
				int i = (tempcmpx-x0)/pCellGrid->mGridX;
				int j = (tempcmpy-y0)/pCellGrid->mGridY;
				if(i<0||j<0)
					continue; 	
				if(i>pCellGrid->getNumX()-2||j>pCellGrid->getNumY()-2)
					continue; 
				short offset=sqrt((float)((sx-rx)*(sx-rx)+(sy-ry)*(sy-ry)));//炮检距
				int h=0;

				if(offsetarray[i][j][0]==-1)
				{
#pragma omp critical  
					offsetarray[i][j][0]=offset;

				}
				else 
				{
#pragma omp critical  
					//omp_set_lock(&mylock);
					offsetarray[i][j].append(offset);
					//omp_unset_lock(&mylock);
				}

				// 采用SPS原始坐标 [Liu Wei 3/20/2023]求方位角
				float temp1=(float)(rry-ssy);
				float temp2=(float)(rrx-ssx);
				float temp3;
				if(temp2==0)
					temp3 = tan(3.14159265/2);
				else
					temp3=temp1/temp2;


				float temp=atanf(temp3)*180/3.14159265;//方位角角度	


				short tarray=0;
				if(rrx<ssx)
				{
					if(rry<ssy)	
					{tarray=270-abs(temp);}
					else 
					{tarray=270+abs(temp);}
				}
				else
				{
					if(rry<ssy)	
					{tarray=90+abs(temp);}
					else 
					{tarray=90-abs(temp);}//与cmp线夹角，顺时针方向
				}
				//求方位角
				if(Azimutharray[i][j][0]==-1)
				{
#pragma omp critical
					//omp_set_lock(&mylock);
					Azimutharray[i][j][0]=tarray;
					//omp_unset_lock(&mylock);
				}
				else 
				{
#pragma omp critical
					//omp_set_lock(&mylock);
					Azimutharray[i][j].append(tarray);
					//omp_unset_lock(&mylock);
				}

			}									
		}
	}

	analyzebinform *dlg=new analyzebinform(this);
	bool valueanalyzebinWorkArea;
	if(CMPmaxfold>0) 	valueanalyzebinWorkArea=true;
	//dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,pSPSParam->mCircleR);// error C2039: “mCircleR”: 不是“GSPSParam”的成员
	dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,0);//mCircleR的初值为0

	//赵虎添加	
	QString strAzimuth="PP: "+QString::fromLocal8Bit("方位角:")+QString::number(0)+
		QString::fromLocal8Bit("-")+QString::number(360);
	QString strOffset=QString::fromLocal8Bit("炮检距:")+QString::number(0)+
		QString::fromLocal8Bit("-")+QString::number(pCellGrid->mMaxoffset);
	dlg->setAODividNum(1,1,0,0,360,pCellGrid->mMaxoffset,strAzimuth,strOffset);//方位角和炮检距分的段数
	dlg->CMPLayoutAttibuteConculate(pProject->getCellGrid()->getCoverCount());	
	dlg->setDataValueworkarea(valueanalyzebinWorkArea);	
	dlg->setAttribute(Qt::WA_DeleteOnClose,true);//关闭窗体时，销毁窗体，清空内存
	dlg->exec();
}

//计算并显示OVT体属性
//所有OVT体属性：（ａ）最小炮检距；（ｂ）最大炮检距；（ｃ）平均炮检距；（ｄ）炮检距容差（方差或标准差）；（e）方位角容差（方差或标准差） [10/26/2022 Liu Wei]
void TSPSForm::on_actionOVT_Volume_Attributes_triggered()
{
	/************************************************************************/
	/* 该功能模块是基于采集模板进行OVT网格划分和分组编号，所以需要知道并获取模板参数    */
	/************************************************************************/
	if(FALSE)// 预留OVT网格尺寸定义接口
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("默认情况下，横向间隔为炮线距，纵向间隔为接收线距"));//消息对话框提示用户网格设置信息
		return;
	}
	// 获取项目中设计观测系统或导入观测系统参数
	GProject *pProject = GProject::getCurrentProject();
	GSLineSet *pSLineSet = pProject->getSLineSet();
	GRLineSet *pRLineSet = pProject->getRLineSet();
	GShotSet *pShotSet = pProject->getShotSet();	//处理关系文件的类（重点）
	GSPSParam *pSPSParam = pProject->getSPSParam();//从底层管理类中去获取数据【模板参数无法获取的问题，基本解决】
	GCellGrid *pCellGrid = pProject->getCellGrid();

	//测试pSLineSet、pRLineSet、pShotSet的所有可获取参数（功能函数）
	int test_count=pShotSet->count();//349
	GShot *test_shot=pShotSet->get(138);
	int tt_count=test_shot->count();//18
	int tt_TrcCount=test_shot->getTrcCount();//9072
	int tt_LtIndex=test_shot->getLtIndex();//0
	int tt_CPCount=test_shot->getCPCount();//0
	int tt_CPint=test_shot->getCPInv();//0

	// 获取炮点和检波点坐标的范围
	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;
	minSX = minSY = LONG_MAX;
	maxSX = maxSY = LONG_MIN;
	minRX = minRY = LONG_MAX;
	maxRX = maxRY = LONG_MIN;
	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);	//震源线集合中存储了所有的炮点坐标信息
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);	//检波线集合中存储了所有的检波点坐标信息
	// 寻找最大最小网格中心坐标范围
	float x0, y0, xn, yn;
	x0 = (minRX + minSX)/2;	//第一个网格左上角坐标x
	y0 = (minRY + minSY)/2;	//第一个网格左上角坐标y
	xn = (maxRX + maxSX)/2;	//最后一个网格左上角坐标x
	yn = (maxRY + maxSY)/2;	//最后一个网格左上角坐标y

	//设定坐标原点为观测系统中第一条检波线的第一个检波点
	QList<float> Hdy, Vdx; //横、纵向坐标分量列表
	int	Num_ShotPerSLine, Num_RecPerRecLine, Num_RecLine;
	float int_shotpoint, int_receiver, int_RecLine, int_SLine;
	float Hxi_receiver, Vyi_receiver, Hxi_shot, Vyi_shot;
	float maxDis_nonlongitudinal; // Maximum non-longitudinal distance （最大非纵炮检距）
	float maxDis_longitudinal; // Maximum longitudinal distance （最大纵向炮检距）

	//获取模板参数--目前得到的是默认参数并没有从输入观测系统参数中去得到这些参数20230117
	Num_ShotPerSLine=pSPSParam->mSPtCount;	//***
	Num_RecPerRecLine=pSPSParam->mRPtCount;	// 不太重要 [Liu Wei 3/22/2023]
	Num_RecLine=pSPSParam->mRLineCount;		// 不太重要 [Liu Wei 3/22/2023]
	int_shotpoint=pSPSParam->mSPtInv;		//*****
	int_receiver=pSPSParam->mRPtInv;		//*****
	int_RecLine=pSPSParam->mRLineInv;		//*****
	int_SLine=pSPSParam->mSLineInv;			//*****

	////手动输入模板参数-测试用
	//Num_ShotPerSLine=5;
	//Num_RecPerRecLine=240;
	//Num_RecLine=40;
	//int_shotpoint=50;
	//int_receiver=25;
	//int_RecLine=125;
	//int_SLine=150;//炮点检波线方向的滚动距离

	//最大非纵距和最大纵向距离其实是最大炮检距纵、横向分量的大小（仅仅适用于规则炮线的情况）
	//maxDis_nonlongitudinal=(Num_RecLine-1)*int_RecLine/2 + (Num_ShotPerSLine-1)*int_shotpoint/2;
	//maxDis_longitudinal=(Num_RecPerRecLine-1)*int_receiver/2;

	maxDis_nonlongitudinal=pCellGrid->mCLMaxoffset;//***** 需要在转换坐标系下面计算得到
	maxDis_longitudinal=pCellGrid->mInMaxoffset;//***** 需要在转换坐标系下面计算得到

	//在做实际SPS数据的时候，我们要通过扫描整个观测系统提取这个数据？


	float gy, gx; //横、纵向OVT片间距-可用于构建绘图坐标-根据模板参数自动计算
	int maxgys_y=gcd( int(int_RecLine),int(int_shotpoint) ); //求两个数的最大公约数
	int maxgys_x=gcd( int(int_receiver),int(int_SLine) ); //求两个数的最大公约数
	gy=int(int_shotpoint*int_RecLine)/maxgys_y; //求两个数的最小公倍数
	gx=int(int_receiver*int_SLine)/maxgys_x; //求两个数的最小公倍数

	////计算理论观测系统采集模板中所有炮点到每一条检波线的炮检距横向分量（炮线方向）dy和纵向分量（检波线方向）dx
	//float hdy, vdx; //中间变量
	//if ((Num_ShotPerSLine-1)*int_shotpoint <= int_RecLine)
	//{
	//	for (int k=1; k<=Num_ShotPerSLine; k++)
	//	{
	//		for (int i=1; i<=Num_RecLine; i++)
	//		{
	//			//第k炮到第i条检波线上第j个检波点的炮检距向量横向分量dy_ijk
	//			hdy=int_RecLine*(i-1) - (maxDis_nonlongitudinal - (Num_ShotPerSLine-1)*int_shotpoint + int_shotpoint*(k-1));
	//			Hdy.append(hdy);

	//			for (int j=1; j<=Num_RecPerRecLine; j++)
	//			{
	//				////第k炮到第i条检波线上第j个检波点的炮检距向量纵向分量dx_ijk
	//				vdx=int_receiver*(j-1) - maxDis_longitudinal;
	//				Vdx.append(vdx);
	//			}
	//		}
	//	}
	//}
	//else //模板中炮点覆盖距离大于检波线距，最大的两个dx被强制分组（第一炮，最后一线；最后一炮，第一线）
	//{
	//	for (int k=1; k<=Num_ShotPerSLine; k++)
	//	{
	//		for (int i=1; i<=Num_RecLine; i++)
	//		{
	//			if (k==1 && i==Num_RecLine)
	//			{
	//				hdy=int_RecLine*(i-2) - (maxDis_nonlongitudinal - (Num_ShotPerSLine-1)*int_shotpoint + int_shotpoint*(k-1));
	//				Hdy.append(hdy);
	//			}
	//			else if (k==Num_ShotPerSLine && i==1)
	//			{
	//				hdy=int_RecLine*(i-1) - (maxDis_nonlongitudinal - (Num_ShotPerSLine-1)*int_shotpoint + int_shotpoint*(k-2));
	//				Hdy.append(hdy);
	//			}
	//			else
	//			{
	//				//第k炮到第i条检波线上第j个检波点的炮检距向量横向分量dx_ijk
	//				hdy=int_RecLine*(i-1) - (maxDis_nonlongitudinal - (Num_ShotPerSLine-1)*int_shotpoint + int_shotpoint*(k-1));
	//				Hdy.append(hdy);
	//			}

	//			for (int j=1; j<=Num_RecPerRecLine; j++)
	//			{
	//				////第k炮到第i条检波线上第j个检波点的炮检距向量纵向分量dy_ijk
	//				vdx=int_receiver*(j-1) - maxDis_longitudinal;
	//				Vdx.append(vdx);
	//			}
	//		}
	//	}
	//}

	////计算观测系统模板中所有炮检距向量的横、纵向分组号
	//QList<int> gyi, gxi; //横、纵向炮检距分量坐标分组号
	////计算横向分组号-炮线方向
	//for (int ix=0; ix<Hdy.size(); ix++)
	//{
	//	if (Hdy[ix]>0)
	//	{
	//		gyi.append(int(Hdy[ix]/(2*gy))+1);
	//	} 
	//	else
	//	{
	//		gyi.append(int(Hdy[ix]/(2*gy))-1);
	//	}
	//}
	//
	//// gyi的值域是[min_gyi, max_gyi]，但是不包含0的整数
	//int max_gyi = *std::max_element(gyi.begin(), gyi.end());
	//int min_gyi = *std::min_element(gyi.begin(), gyi.end());

	////计算纵向分组号-检波线方向
	//for (int ix=0; ix<Vdx.size(); ix++)
	//{
	//	if (Vdx[ix]>0)
	//	{
	//		gxi.append(int(Vdx[ix]/(2*gx))+1);
	//	} 
	//	else
	//	{
	//		gxi.append(int(Vdx[ix]/(2*gx))-1);
	//	}
	//}
	//// gxi的值域是[min_gxi, max_gxi]，但是不包含0的整数
	//int max_gxi = *std::max_element(gxi.begin(), gxi.end());
	//int min_gxi = *std::min_element(gxi.begin(), gxi.end());
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

	// 计算OVT划片组数和编号数组
	int ny_OVT=max_gyi-min_gyi; //横向划片组数
	int	nx_OVT=max_gxi-min_gxi; //纵向划片组数
	QList<int> gpnumber_y, gpnumber_x; //OVT划片分组编号
	for (int iy=min_gyi; iy<=max_gyi; iy++)
	{
		if (iy!=0)
		{
			gpnumber_y.append(iy);
		}		
	}//crossline
	for (int ix=min_gxi; ix<=max_gxi; ix++)
	{
		if (ix!=0)
		{
			gpnumber_x.append(ix);
		}
	}//inline
	// 申请OVT划片数组空间
	GValueVector<short> **temp_offset; //存放每一个OVT片中的偏移距；
	GValueVector<short> **temp_azimuth;//存放每一个OVT片中的方位角；
	GValueVector<float> **temp_x_coordinate;//存放每一个OVT片中来自工区不同位置、具有相同或相近炮检距和方位角的炮检对中点坐标X。
	GValueVector<float> **temp_y_coordinate;//存放每一个OVT片中来自工区不同位置、具有相同或相近炮检距和方位角的炮检对中点坐标Y。
	gAlloc2(temp_offset,nx_OVT,ny_OVT);
	gAlloc2(temp_azimuth,nx_OVT,ny_OVT);
	gAlloc2(temp_x_coordinate,nx_OVT,ny_OVT);
	gAlloc2(temp_y_coordinate,nx_OVT,ny_OVT);

	int CMPxbinnumber=pCellGrid->getNumX();
	int CMPybinnumber=pCellGrid->getNumY();

	int **mtempfold; //临时存放OVT片中炮检对数目
	gAlloc2(mtempfold,nx_OVT,ny_OVT);
	for(int i=0;i<nx_OVT;i++)
		for(int j=0;j<ny_OVT;j++)
			mtempfold[i][j]=0;

	//for(int i=0;i<nx_OVT;i++)
	//{
	//	for(int j=0;j<ny_OVT;j++)
	//	{
	//		for(int k=0;k<1;k++)
	//		{
	//			temp_offset[i][j].append(-1);
	//			temp_azimuth[i][j].append(-1);
	//		}
	//	}
	//}

	for(int ii=0;ii<pShotSet->count();ii++)
	{//遍历每一炮
		GShot *pShot = pShotSet->get(ii);
		if(pShot != 0)
		{
			double sx=pShot->getSPoint()->getX();//实际没有旋转的炮点坐标x
			double sy=pShot->getSPoint()->getY();//实际没有旋转的炮点坐标y
			double ssx=pShot->getSPoint()->getSPSX();//旋转后的炮点坐标x
			double ssy=pShot->getSPoint()->getSPSY();//旋转后的炮点坐标y
			for(int si=0; si<pShot->getTrcCount(); si++)
			{//遍历每一炮对应的接收道
				GRPoint *rpoint = pShot->getRPointByTrcIndex(si,pRLineSet);
				if(rpoint==NULL)
					continue;
				float rrx = rpoint->getSPSX();//实际没有旋转的检波点坐标x
				float rry  = rpoint->getSPSY();//实际没有旋转的检波点坐标y
				float rx = rpoint->getX();//旋转后的检波点坐标x
				float ry = rpoint->getY();//旋转后的检波点坐标y
				// 应不应该用转换后的坐标进行区间计算？ [Liu Wei 3/23/2023]
				int tInoffset=rx-sx;//炮检距矢量（炮点指向检波点的矢量）的纵向分量
				int tCLoffset=ry-sy;//炮检距矢量（炮点指向检波点的矢量）的横向分量

				float temprsx=(rrx+ssx)/2;//炮检距矢量中心点坐标的纵向分量（原始坐标系）
				float temprsy=(rry+ssy)/2;//炮检距矢量中心点坐标的横向分量（原始坐标系）
				float tempx=(rx+sx)/2;//炮检距矢量中心点坐标的纵向分量（旋转坐标系）
				float tempy=(ry+sy)/2;//炮检距矢量中心点坐标的横向分量（旋转坐标系）

				int cell_i=(tempx-x0)/pCellGrid->mGridX;
				int cell_j=(tempy-y0)/pCellGrid->mGridY;

				if(cell_i>=0&&cell_j>=0&&cell_i<=CMPxbinnumber||cell_j<=CMPybinnumber)//判断待计算炮检距中点是否在规定网格范围内（暂时保留）
				{
					//OVT片分组编号索引（映射）
					int tempxi,tempyi;
					if (tInoffset>0)//计算纵向OVT片分组编号
					{
						tempxi=int(tInoffset/(2*gx))+1;
					}
					else
					{
						tempxi=int(tInoffset/(2*gx))-1;
					}
					if (tCLoffset>0)//计算横向OVT片分组编号
					{
						tempyi=int(tCLoffset/(2*gy))+1;
					} 
					else
					{
						tempyi=int(tCLoffset/(2*gy))-1;
					}

					//超出理论最大炮检距的强制归入最大炮检距组-暂时保留
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
					//超出理论最大炮检距的强制归入最大炮检距组-暂时保留
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

					//计算对应OVT片分组的数组索引号
					int xi, yi;
					xi=gpnumber_x.indexOf(tempxi); //纵向分组编号索引（检波线方向）
					yi=gpnumber_y.indexOf(tempyi); //横向分组编号索引（炮点线方向）
					//累计记录炮检距向量中心点坐标
					temp_x_coordinate[xi][yi].append(temprsx);
					temp_y_coordinate[xi][yi].append(temprsy);
					//累积记录炮检距
					float tOffset=sqrtf(tInoffset*tInoffset+tCLoffset*tCLoffset);//计算炮检距大小（与坐标系旋转没有关系）
					//temp_offset[xi][yi].append(tOffset);
					temp_offset[xi][yi].append(short(tOffset));

					//求方位角:炮点指向检波点的向量方位
					float temp1=(float)(rry-ssy);	//用转换后的坐标做不行吗？
					float temp2=(float)(rrx-ssx);	//答：不行，因为真实的观测系统可能不是正南正北（炮线方向）					
					//利用反正切函数求解方位角弧度，并将弧度值转换为角度，因为不论是atanf，atan2f输出的角度都是弧度值，需要进行转换。
					//https://blog.csdn.net/weixin_46098577/article/details/116026828
					float atgt_deg; //反正切角度
					if(temp2==0)	//temp==0表示在y轴上。此外，还应该判断dx的正负，确定是0°还是180°
					{
						if (temp1 > 0)//正半轴
						{
							atgt_deg=0;
						}
						else if (temp1 < 0)//负半轴
						{
							atgt_deg=180;
						}
					}
					else
					{
						float temp=atanf(temp1/temp2)*180/PI; //atan(x)、atanf(x)的输出值范围(-pi/2,pi/2)

						if(rrx<ssx)	//极坐标系的三、四象限（上北下南左西右东，正北为0°，正东为90°，顺时针增加）
						{
							if(rry<ssy)	
							{atgt_deg=270-abs(temp);}	
							else 
							{atgt_deg=270+abs(temp);}
						}
						else	   //极坐标系的一、二象限
						{
							if(rry<ssy)	
							{atgt_deg=90+abs(temp);}
							else 
							{atgt_deg=90-abs(temp);}//与cmp线夹角，顺时针方向
						}
					}
					//temp_azimuth[xi][yi].append(atgt_deg);//累积记录方位角
					temp_azimuth[xi][yi].append(short(atgt_deg));//累积记录方位角
				}
			}									
		}
	}

	////寻找OVT片中的最大的炮检距个数
	//int OffsetNum_Max=0;
	//for(int i=0; i<nx_OVT; i++)
	//{
	//	for(int j=0; j<ny_OVT; j++)
	//	{
	//		if(temp_offset[i][j].size()>OffsetNum_Max) OffsetNum_Max=temp_offset[i][j].size();
	//		mtempfold[i][j]=temp_offset[i][j].size();
	//	}
	//}
	//if(OffsetNum_Max==0)
	//{
	//	QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("OVT片中的没有炮检对！"));//消息对话框显示提示用户违法操作
	//	return;
	//}

#pragma region "显示模块数据准备，赵老师"	
	//QString str="OVT"+QString::fromLocal8Bit("分析,")+"Inline:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaInOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaInOffset)+",Cline:"+QString::number(pAttrAnalyzeParam->mRealBeginAreaCLOffset)+"-"+QString::number(pAttrAnalyzeParam->mRealEndAreaCLOffset);
	//int CMPmaxfold=0;
	//for(int i=0;i<CMPxbinnumber;i++)
	//{
	//	for(int j=0;j<CMPybinnumber;j++)
	//	{
	//		if(tempfold[i][j]>CMPmaxfold) CMPmaxfold=tempfold[i][j];
	//	}
	//}
	//if(CMPmaxfold==0)
	//{
	//	QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("最大覆盖次数为零"));//消息对话框显示提示用户违法操作
	//	return;
	//}

	//analyzebinform *dlg=new analyzebinform(this);
	//bool valueanalyzebinWorkArea;
	//if(CMPmaxfold>0) 	valueanalyzebinWorkArea=true;
	//dlg->setAnalyzedata(offsetarray,Azimutharray,CMPxbinnumber,CMPybinnumber,CMPmaxfold+1,pCellGrid->mGridX,pCellGrid->mGridY,pSPSParam->mCircleR);

	////赵虎添加	
	//QString strAzimuth="";
	//QString strOffset=str;
	//dlg->setAODividNum(1,1,0,0,360,pCellGrid->mMaxoffset,strAzimuth,strOffset);//方位角和炮检距分的段数
	//dlg->CMPLayoutAttibuteConculate(tempfold);	
	//dlg->setDataValueworkarea(valueanalyzebinWorkArea);	
	//dlg->setAttribute(Qt::WA_DeleteOnClose,true);//关闭窗体时，销毁窗体，清空内存
	//dlg->exec();
#pragma endregion "显示模块数据准备，赵老师"

	VolumeAttributeview=new OVTVolumeAttributeView(this);
	VolumeAttributeview->setOVTcubeData(temp_offset, temp_azimuth, nx_OVT, ny_OVT, 2*gx, 2*gy, maxDis_longitudinal, maxDis_nonlongitudinal);
	VolumeAttributeview->exec();//阻塞显示窗口

	delete VolumeAttributeview;
	VolumeAttributeview=NULL;

	//刘伟
	if(temp_offset!=NULL) 
	{
		for(int i=0;i<nx_OVT;i++)
		{
			for(int j=0;j<ny_OVT;j++)
			{
				temp_offset[i][j].clear();
				temp_azimuth[i][j].clear();
				temp_x_coordinate[i][j].clear();
				temp_y_coordinate[i][j].clear();
			}
		}	
		gFree2(temp_offset);
		gFree2(temp_azimuth);	
		gFree2(temp_x_coordinate);
		gFree2(temp_y_coordinate);
	}

	gFree2(mtempfold);
}
//计算并显示OVT片属性
void TSPSForm::on_actionOVT_Patch_Attributes_triggered()
{
	GProject *pProject = GProject::getCurrentProject();
	GAttrAnalyzeParam *pAttrAnalyzeParam = pProject->getAttrAnalyzeParam();
	TRealOVTdlg *dlg=new TRealOVTdlg();
	dlg->init(pAttrAnalyzeParam);
	if(dlg->exec())
	{
		delete dlg;
	}
	else
	{
		delete dlg;
		return;
	}
}

// 导入SPS观测系统并显示 [Liu Wei 5/29/2023]
void TSPSForm::on_Load_SPS_Action_triggered()
{
	GProject *pProject = GProject::getCurrentProject();
	DataReconstruction *LoadSPS_dlg = new DataReconstruction(this);
	LoadSPS_dlg->setProject(pProject);
	if(LoadSPS_dlg->exec())
	{
		mpView->mIsImpotSps=true; //add by zhaohu 是否显示输入的观测系统
		//工程对象内部也要同步设置指示标识参数
		mpProject->mIsNewImfo=false; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]
		refresh();

		// 激活计算μ值的功能 [Liu Wei 8/13/2023]
		ui.actionCalculate_the_Miu_for_Loaded_Geometry->setEnabled(true);
	}
	delete LoadSPS_dlg;
}

void TSPSForm::on_actionRSample_triggered()
{
	/************************************************************************/
	/* 采样前的检波器排列也要进行属性计算和分析，用于与优化后排列进行对比 */
	/* 检波点采样及显示 */
	/************************************************************************/
	GSPSParam *pSPSParam = mpProject->getSPSParam();
	SpsSampleDlg *sample_dlg=new SpsSampleDlg();
	if(sample_dlg->exec()){// pSPSParam中传入的参数是目标规则观测系统的参数 [Liu Wei 6/14/2023]
		int Mode_value=sample_dlg->mode_value;
		float cr=sample_dlg->Tsampling_rate;//线内采样率，注意某些小数在计算机中精度不高，比如0.6在计算机中的值为0.6000000002。
		float cl=sample_dlg->Lsampling_rate;//线间采样率
		int N=ceil(pSPSParam->mRPtCount/cr);// 在出现小数时，无法得到正确的结果 [Liu Wei 7/31/2023]
		int row=ceil(pSPSParam->mRLineCount/cl);// 在出现小数时，无法得到正确的结果 [Liu Wei 7/31/2023]
		float TraceInv = pSPSParam->mRPtInv*cr;//采样前的规则道间距，mRPtInv被当做是一个平均道间距
		float RLineInv = pSPSParam->mRLineInv*cl;//采样前的规则接收线距，mRLineInv被当做是一个平均线间距
		float rx0 = pSPSParam->mRX0;
		float ry0 = pSPSParam->mRY0;

		if (row>1)//二维采样
		{
			int **Phi0;	//初始线间未采样的二维矩阵
			int *phi0;	//线间优化前的一维矩阵
			
			if (Mode_value==0)
			{
				Phi0=Sampling2S(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1S(row, cl); //优化前的一维矩阵
			}
			else if (Mode_value==1)
			{
				Phi0=Sampling2J(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1J(row, cl); //优化前的一维矩阵
			} 
			else
			{
				Phi0=Sampling2F(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1F(row, cl); //优化前的一维矩阵
			}
		
			//****** 对线进行采样，得到线间采样后的二维矩阵 ******//
			int **Phi01 = MulMatrix(row, phi0, N, Phi0);

			//****** 计算二维矩阵的μ值 ******//
			// 当线数较少时，计算线间μ值无意义，则只讨论线内的μ值 //
			float miu_xj_before=CalculateAveMiu(row,N,Phi01);//计算线内的μ值
			
			//****** 采样后检波点坐标计算及显示 ******//
			int *y=Convert1D(row,N,Phi01);//将二维转化为一维
			float *x=RegularMR(row,N, TraceInv,rx0);//规则检波点横坐标
			float *x1=RegularNR(row,N, RLineInv,ry0);//规则检波点纵坐标
			float *Xcoor=MulMatrix_1D_float(row*N,y,x);//采样后横坐标
			float *Ycoor=MulMatrix_1D_float(row*N,y,x1);//采样后纵坐标

			// 将一维坐标转化为二维 [Liu Wei 6/14/2023]
			QVector<QVector<float> > XCoordinate, YCoordinate;
			for (int i=0; i<pSPSParam->mRLineCount; i++)
			{
				QVector<float> rline_xcoordinate;
				QVector<float> rline_ycoordinate;
				for (int j=0; j<pSPSParam->mRPtCount; j++)
				{
					rline_xcoordinate.append(Xcoor[i*pSPSParam->mRPtCount+j]);
					rline_ycoordinate.append(Ycoor[i*pSPSParam->mRPtCount+j]);
				}
				XCoordinate.append(rline_xcoordinate);
				YCoordinate.append(rline_ycoordinate);
			}
			// 保存采样结果 [Liu Wei 6/30/2023]
			if ( (!R_Sampling_Arrays.isEmpty())|| (!RL_Sampling.isEmpty()))
			{
				R_Sampling_Arrays.clear();
				RL_Sampling.clear();
			}// 保证每次保存最新采样结果
			for (int i=0; i<row; i++)
			{
				QVector<int> rv;
				for (int j=0; j<N; j++)
				{
					rv.append(Phi0[i][j]);
				}
				R_Sampling_Arrays.append(rv);
				RL_Sampling.append(phi0[i]);
			}
			// 替换坐标 [Liu Wei 6/14/2023]
			GRLineSet *pRLineSetNew = mpProject->getRLineSetNew();
			pRLineSetNew->create(XCoordinate,YCoordinate,pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]
			
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_xj_before);
			QMessageBox::information(NULL,tr("Computation Information"),info_str);// 请确认哪些内容需要输出！！！ [Liu Wei 6/29/2023]

			// 个人认为：需要重新创建炮检关系 [Liu Wei 6/29/2023]
			GSLineSet *pSLineSetNew = mpProject->getSLineSetNew();
			GShotSet *pShotSetNew = mpProject->getShotSetNew();
			pShotSetNew->create(pSPSParam,pSLineSetNew,pRLineSetNew);

			// ***释放内存*** //
			for (int i=0;i<row;i++)
			{
				free(Phi0[i]);
				free(Phi01[i]);
			}

			free(Phi0);
			free(Phi01);// 有一部分矩阵数据有必要保存（用QVector），然后点击优化按钮进行优化处理 [Liu Wei 6/29/2023]

			delete [] phi0;
			delete [] y;
			delete [] x;
			delete [] x1;
			delete [] Xcoor;
			delete [] Ycoor;

		}
		else//一维采样
		{
			// 暂时不考虑 [Liu Wei 6/30/2023]
		}
		// 完成后显示观测系统 [Liu Wei 6/16/2023]
		mIsFill = true;
		mIsLiner = true ;

		mpView->mIsImpotSps=false;//是否显示输入的观测系统
		//工程对象内部也要同步设置指示标识参数
		mpProject->mIsNewImfo=true; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]
		refresh();//显示不同的sps后，重新刷新一下视图

		mpView->repaintContents();
	}
	delete sample_dlg;
	qDebug()<<"hello world!";// 测试 [Liu Wei 6/5/2023]
}

void TSPSForm::on_actionSSample_triggered()
{
	/************************************************************************/
	/* 震源排列的采样及显示                                                                     */
	/************************************************************************/
	GSPSParam *pSPSParam = mpProject->getSPSParam();
	SpsSampleDlg *sample_dlg=new SpsSampleDlg();
	if(sample_dlg->exec()){// pSPSParam中传入的参数是目标规则观测系统的参数 [Liu Wei 6/14/2023]
		int Mode_value=sample_dlg->mode_value;
		float cr=sample_dlg->Tsampling_rate;//线内采样率
		float cl=sample_dlg->Lsampling_rate;//线间采样率
		int N=ceil(pSPSParam->mSPtCount/cr);
		int row=ceil(pSPSParam->mSLineCount/cl);
		float SPointInv = pSPSParam->mSPtInv*cr;//采样前的规则道间距，mSPtInv被当做是一个平均炮点间距
		float SLineInv = pSPSParam->mSLineInv*cl;//采样前的规则接收线距，mSLineInv被当做是一个平均线间距
		float sx0 = pSPSParam->mSX0;
		float sy0 = pSPSParam->mSY0;

		if (row>1)//二维采样
		{
			int **Phi0;	//初始线间未采样的二维矩阵
			int *phi0;	//线间优化前的一维矩阵

			if (Mode_value==0)
			{
				Phi0=Sampling2S(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1S(row, cl); //优化前的一维矩阵
			}
			else if (Mode_value==1)
			{
				Phi0=Sampling2J(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1J(row, cl); //优化前的一维矩阵
			} 
			else
			{
				Phi0=Sampling2F(row, N, cr);//初始线间未采样的二维矩阵
				phi0 = Sampling1F(row, cl); //优化前的一维矩阵
			}

			//****** 对线进行采样,得到线间采样后的二维矩阵 ******//
			int **Phi01 = MulMatrix(row, phi0, N, Phi0);//线间优化前二维矩阵

			//****** 计算优化前的μ值 ******//
			float miu_xj_before=CalculateAveMiu(row,N,Phi01);//计算线间优化前的μ值

            //****** 采样后炮点坐标计算及显示 ******//
			int *y=Convert1D(row,N,Phi01);//将二维转化为一维
			float *x=RegularMS(row,N,SLineInv,sx0);//规则炮点横坐标
			float *x1=RegularNS(row,N,SPointInv,sy0);//规则炮点纵坐标
			float *Xcoor=MulMatrix_1D_float(row*N,y,x);//采样后横坐标
			float *Ycoor=MulMatrix_1D_float(row*N,y,x1);//采样后纵坐标

			// 保存采样结果 [Liu Wei 6/30/2023]
			if ( (!S_Sampling_Arrays.isEmpty())|| (!SL_Sampling.isEmpty()))
			{
				S_Sampling_Arrays.clear();
				SL_Sampling.clear();
			}// 保证每次保存最新采样结果
			for (int i=0; i<row; i++)
			{
				QVector<int> sv;
				for (int j=0; j<N; j++)
				{
					sv.append(Phi0[i][j]);
				}
				S_Sampling_Arrays.append(sv);
				SL_Sampling.append(phi0[i]);
			}

			// 将一维坐标转化为二维 [Liu Wei 6/14/2023]
			QVector<QVector<float> > XCoordinate, YCoordinate;
			for (int i=0; i<pSPSParam->mSLineCount; i++)
			{
				QVector<float> sline_xcoordinate;
				QVector<float> sline_ycoordinate;
				for (int j=0; j<pSPSParam->mSPtCount; j++)
				{
					sline_xcoordinate.append(Xcoor[i*pSPSParam->mSPtCount+j]);
					sline_ycoordinate.append(Ycoor[i*pSPSParam->mSPtCount+j]);
				}
				XCoordinate.append(sline_xcoordinate);
				YCoordinate.append(sline_ycoordinate);
			}
			// 替换坐标 [Liu Wei 6/14/2023]
			GSLineSet *pSLineSetNew = mpProject->getSLineSetNew();
			pSLineSetNew->create(XCoordinate,YCoordinate,pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]
			
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_xj_before);
			QMessageBox::information(NULL,tr("Computation Information"),info_str);

			// 个人认为：需要重新创建炮检关系 [Liu Wei 6/29/2023]
			GRLineSet *pRLineSetNew = mpProject->getRLineSetNew();
			GShotSet *pShotSetNew = mpProject->getShotSetNew();
			pShotSetNew->create(pSPSParam,pSLineSetNew,pRLineSetNew);

			// ***释放内存*** //
			for (int i=0;i<row;i++)
			{
				free(Phi0[i]);
				free(Phi01[i]);
			}

			free(Phi0);
			free(Phi01);

			delete [] phi0;
			delete [] y;
			delete [] x;
			delete [] x1;
			delete [] Xcoor;
			delete [] Ycoor;

		}
		else//一维采样
		{
			// 暂时不考虑 [Liu Wei 6/30/2023]
		}
	}
	delete sample_dlg;
	qDebug()<<"hello world!";// 测试 [Liu Wei 6/5/2023]
}

void TSPSForm::on_actionRAOptimizing_triggered()
{
    //************** 检波点优化 *************//
	GSPSParam *pSPSParam = mpProject->getSPSParam();
	SpsSampleDlg *sample_dlg=new SpsSampleDlg();
	if(sample_dlg->exec()){// pSPSParam中传入的参数是目标规则观测系统的参数 [Liu Wei 6/14/2023]
		int Mode_value=sample_dlg->mode_value;
		float cr=sample_dlg->Tsampling_rate;//线内采样率
		float cl=sample_dlg->Lsampling_rate;//线间采样率
		int N=ceil(pSPSParam->mRPtCount/cr);
		int row=ceil(pSPSParam->mRLineCount/cl);
		float TraceInv = pSPSParam->mRPtInv*cr;//采样前的规则道间距，mRPtInv被当做是一个平均道间距
		float RLineInv = pSPSParam->mRLineInv*cl;//采样前的规则接收线距，mRLineInv被当做是一个平均线间距
		float rx0 = pSPSParam->mRX0;
		float ry0 = pSPSParam->mRY0;

		if (row>1)//二维采样
		{
			int **Phi0;	//初始线间未采样的二维矩阵
			int *phi0;	//线间优化前的一维矩阵
			int *phi;	//线间优化后的矩阵
			// 申请数组空间 [Liu Wei 6/30/2023]
			phi0 = new int[row];
			Phi0 = new int *[row];
			for (int i=0;i<row;i++)
			{
				Phi0[i] = new int[N];
			}
			// 赋值 [Liu Wei 6/30/2023]
			for (int i=0; i<row; i++)
			{
				for (int j=0; j<N; j++)
				{
					Phi0[i][j]=R_Sampling_Arrays[i][j];
				}
				phi0[i]=RL_Sampling[i];
			}

			if (Mode_value==0)
			{
				phi = UnOptiMiuSA(row,cl,phi0,"R");//优化后的矩阵
			}
			else if (Mode_value==1)
			{
				phi = UnOptiMiuSA(row,cl,phi0,"J");//优化后的矩阵
			} 
			else
			{
				phi = UnOptiMiuSA(row,cl,phi0,"F");//优化后的矩阵
			}
		
			//****** 计算优化前后线之间的μ值 ******//
			float miu0 = TestCurveletMiu1D(row, phi0, "EigenReal"); //优化前线间的μ值
			float miu1 = TestCurveletMiu1D(row,phi,"EigenReal"); //优化后线间的μ值
	
			//****** 对线进行采样,得到线间采样后的二维矩阵 ******//
			int **Phi01 = MulMatrix(row, phi0, N, Phi0);//线间优化前二维矩阵
			int **Phi1 = MulMatrix(row, phi, N, Phi0);//线间优化后二维矩阵，作为线内优化前的初始矩阵

			//****** 计算优化前后线内的μ值 ******//
			float miu_xj_before=CalculateAveMiu(row,N,Phi01);//计算线间优化前的μ值
			float miu_xj_after=CalculateAveMiu(row,N,Phi1);//计算线间优化后的μ值

			// ***************线内优化*************** //
			// 初始化数组 //
			int **PHI = (int**)malloc(sizeof(int*) * row);
			for (int i = 0; i < row; i++) 
			{
				PHI[i] = (int*)malloc(sizeof(int) * N);
			}
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < N; j++)
				{
					PHI[i][j] = 0;
				}
			}

			//****** 进行每条线内优化 ******//
			QProgressDialog *mProgressDlg;
			mProgressDlg = new QProgressDialog("进度","cancel",0,row);//设置进度并显示进度条
			mProgressDlg->setLabelText(QString::fromLocal8Bit("正在执行..."));
			mProgressDlg->setWindowTitle(QString::fromLocal8Bit("请等待..."));
			mProgressDlg->setCancelButtonText(NULL);//取消cancel

			int row_r=0;
			clock_t start, finish;//计算时间
			start = clock();
			if (Mode_value==0)
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"R");//混合优化算法优化μ值
					PHI[j] = phi1;   //得到线间和线内优化后的二维矩阵
					row_r++;
					mProgressDlg->setValue(row_r);
				}
			}
			else if(Mode_value==1)
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"J");//混合优化算法优化μ值
					PHI[j] = phi1;   //得到线间和线内优化后的二维矩阵
					row_r++;
					mProgressDlg->setValue(row_r);
				}
			}
			else
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"F");//混合优化算法优化μ值
					PHI[j] = phi1;   //得到线间和线内优化后的二维矩阵
					row_r++;
					mProgressDlg->setValue(row_r);
				}
			}
			finish = clock();
			float duration = (float)(finish - start) / CLOCKS_PER_SEC;  

			//****** 优化采样后检波点坐标计算及显示 ******//
			int *y=Convert1D(row,N,PHI);//将二维转化为一维
			float *x=RegularMR(row,N, TraceInv,rx0);//规则检波点横坐标
			float *x1=RegularNR(row,N, RLineInv,ry0);//规则检波点纵坐标
			float *Xcoor=MulMatrix_1D_float(row*N,y,x);//采样后横坐标
			float *Ycoor=MulMatrix_1D_float(row*N,y,x1);//采样后纵坐标

			//****** 计算优化后的μ值 ******//
			float miu_after=CalculateAveMiu(row,N,PHI);//计算优化后的μ值

			// 将一维坐标转化为二维 [Liu Wei 6/14/2023]
			QVector<QVector<float> > XCoordinate, YCoordinate;
			for (int i=0; i<pSPSParam->mRLineCount; i++)
			{
				QVector<float> rline_xcoordinate;
				QVector<float> rline_ycoordinate;
				for (int j=0; j<pSPSParam->mRPtCount; j++)
				{
					rline_xcoordinate.append(Xcoor[i*pSPSParam->mRPtCount+j]);
					rline_ycoordinate.append(Ycoor[i*pSPSParam->mRPtCount+j]);
				}
				XCoordinate.append(rline_xcoordinate);
				YCoordinate.append(rline_ycoordinate);
			}

			// 替换坐标 [Liu Wei 6/14/2023]
			GRLineSet *pRLineSetNew = mpProject->getRLineSetNew();
			pRLineSetNew->create(XCoordinate,YCoordinate,pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]

			QString miu_str=QString::fromLocal8Bit("miu0:")+QString::number(miu0)+","+QString::fromLocal8Bit("miu1:")+QString::number(miu1)+","+QString::fromLocal8Bit("优化前最大互相关值:")+QString::number(miu_xj_after)+";";
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_after)+","+tr("Optimizing time: ")+QString::number(duration)+" s";
			QMessageBox::information(NULL,tr("Computation Information"),miu_str+info_str);

			// 个人认为：需要重新创建炮检关系 [Liu Wei 6/29/2023]
			GSLineSet *pSLineSetNew = mpProject->getSLineSetNew();
			GShotSet *pShotSetNew = mpProject->getShotSetNew();
			pShotSetNew->create(pSPSParam,pSLineSetNew,pRLineSetNew);
			
			// ***释放内存*** //
			for (int i=0;i<row;i++)
			{
				free(PHI[i]);
				free(Phi0[i]);
				free(Phi01[i]);
				free(Phi1[i]);
			}

			free(PHI);
			free(Phi0);
			free(Phi01);
			free(Phi1);

			delete [] phi0;
			delete [] phi;
			delete [] y;
			delete [] x;
			delete [] x1;
			delete [] Xcoor;
			delete [] Ycoor;

		}
		
		else//一维采样
		{
			// 暂时不考虑 [Liu Wei 6/30/2023]
		}
		// 完成后显示观测系统 [Liu Wei 6/16/2023]
		mIsFill = true;
		mIsLiner = true ;

		mpView->mIsImpotSps=false;//是否显示输入的观测系统
		//工程对象内部也要同步设置指示标识参数
		mpProject->mIsNewImfo=true; // 返回新建观测系统信息（标识） [Liu Wei 6/20/2023]
		refresh();//显示不同的sps后，重新刷新一下视图

		mpView->repaintContents();
	}
	delete sample_dlg;
	qDebug()<<"hello world!";// 测试 [Liu Wei 6/5/2023]
}
void TSPSForm::on_Cancel_clicked()
{
	close();
}
void TSPSForm::on_actionSAOptimizing_triggered()
{
	/************************************************************************/
	/* 震源排列的采样和优化                                                                     */
	/************************************************************************/
	GSPSParam *pSPSParam = mpProject->getSPSParam();
	SpsSampleDlg *sample_dlg=new SpsSampleDlg();
	if(sample_dlg->exec()){// pSPSParam中传入的参数是目标规则观测系统的参数 [Liu Wei 6/14/2023]
		int Mode_value=sample_dlg->mode_value;
		float cr=sample_dlg->Tsampling_rate;//线内采样率
		float cl=sample_dlg->Lsampling_rate;//线间采样率
		int N=ceil(pSPSParam->mSPtCount/cr);
		int row=ceil(pSPSParam->mSLineCount/cl);
		float SPointInv = pSPSParam->mSPtInv*cr;//采样前的规则道间距，mSPtInv被当做是一个平均炮点间距
		float SLineInv = pSPSParam->mSLineInv*cl;//采样前的规则接收线距，mSLineInv被当做是一个平均线间距
		float sx0 = pSPSParam->mSX0;
		float sy0 = pSPSParam->mSY0;

		if (row>1)//二维采样
		{
			int **Phi0;	//初始线间未采样的二维矩阵
			int *phi0;	//线间优化前的一维矩阵
			int *phi;	//线间优化后的矩阵

			phi0 = new int[row];
			Phi0 = new int *[row];

			for (int i=0;i<row;i++)
			{
				Phi0[i] = new int[N];
			}
			// 赋值 [Liu Wei 6/30/2023]
			for (int i=0; i<row; i++)
			{
				for (int j=0; j<N; j++)
				{
					Phi0[i][j]=S_Sampling_Arrays[i][j];
				}
				phi0[i]=SL_Sampling[i];
			}
			
			if (Mode_value==0)
			{
				phi = UnOptiMiuSA(row,cl,phi0,"R");//优化后的矩阵
			}
			else if (Mode_value==1)
			{
				phi = UnOptiMiuSA(row,cl,phi0,"J");//优化后的矩阵
			} 
			else
			{
				phi = UnOptiMiuSA(row,cl,phi0,"F");//优化后的矩阵
			}

			float miu0 = TestCurveletMiu1D(row, phi0,"EigenReal"); //优化前线间的μ值
			float miu1 = TestCurveletMiu1D(row,phi,"EigenReal"); //优化后线间的μ值

			//****** 对线进行采样,得到线间采样后的二维矩阵 ******//
			int **Phi01 = MulMatrix(row, phi0, N, Phi0);//线间优化前二维矩阵
			int **Phi1 = MulMatrix(row, phi, N, Phi0);//线间优化后二维矩阵

			//****** 计算优化前线内的μ值 ******//
			float miu_xj_before=CalculateAveMiu(row,N,Phi01);//计算线间优化前的μ值
			float miu_xj_after=CalculateAveMiu(row,N,Phi1);//计算线间优化后的μ值

			// ***************线内优化*************** //
			// 初始化数组 //
			int **PHI = (int**)malloc(sizeof(int*) * row);
			for (int i = 0; i < row; i++) 
			{
				PHI[i] = (int*)malloc(sizeof(int) * N);
			}
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < N; j++)
				{
					PHI[i][j] = 0;
				}
			}

			// ***进行每条线内优化*** //
			QProgressDialog *mProgressDlg;
			mProgressDlg = new QProgressDialog("Calculate progress","Cancel",0,row);//设置进度并显示进度条
			//mProgressDlg->setLabelText(tr("Please Wait..."));
			mProgressDlg->setLabelText(QString::fromLocal8Bit("正在执行..."));
			mProgressDlg->setWindowTitle(QString::fromLocal8Bit("请等待..."));
			mProgressDlg->setCancelButtonText(NULL);//取消cancel

			clock_t start, finish;//计算时间
			start = clock();
			int row_s=0;
			if (Mode_value==0)
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"R");//混合优化算法优化μ值
					PHI[j] = phi1;
					row_s++;
					mProgressDlg->setValue(row_s);//设置进度并显示进度条//得到线间和线内优化后的二维矩阵
				}
			}
		
			else if(Mode_value==1)
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"J");//混合优化算法优化μ值
					PHI[j] = phi1; 
					row_s++;
					mProgressDlg->setValue(row_s);
				}
			}
			else
			{
				for (int j=0;j<row;j++)
				{
					int *phi1 = UnOptiMiuSA(N,cr,Phi1[j],"F");//混合优化算法优化μ值
					PHI[j] = phi1;
					row_s++;
					mProgressDlg->setValue(row_s);
				}
			}
			finish = clock();
			float duration = (float)(finish - start) / CLOCKS_PER_SEC;  

			//****** 优化采样后炮点坐标计算及显示 ******//
			int *y=Convert1D(row,N,PHI);//将二维转化为一维
			float *x=RegularMS(row,N,SLineInv,sx0);//规则炮点横坐标
			float *x1=RegularNS(row,N,SPointInv,sy0);//规则炮点纵坐标
			float *Xcoor=MulMatrix_1D_float(row*N,y,x);//采样后横坐标
			float *Ycoor=MulMatrix_1D_float(row*N,y,x1);//采样后纵坐标

			//****** 计算优化后的μ值 ******//
			float miu_after=CalculateAveMiu(row,N,PHI);//计算优化后的μ值

			// 将一维坐标转化为二维 [Liu Wei 6/14/2023]
			QVector<QVector<float> > XCoordinate, YCoordinate;
			for (int i=0; i<pSPSParam->mSLineCount; i++)
			{
				QVector<float> sline_xcoordinate;
				QVector<float> sline_ycoordinate;
				for (int j=0; j<pSPSParam->mSPtCount; j++)
				{
					sline_xcoordinate.append(Xcoor[i*pSPSParam->mSPtCount+j]);
					sline_ycoordinate.append(Ycoor[i*pSPSParam->mSPtCount+j]);
				}
				XCoordinate.append(sline_xcoordinate);
				YCoordinate.append(sline_ycoordinate);
			}

			// 替换坐标 [Liu Wei 6/14/2023]
			GSLineSet *pSLineSetNew = mpProject->getSLineSetNew();
			pSLineSetNew->create(XCoordinate,YCoordinate,pSPSParam);// 重写create函数并将李汶骏的坐标（最好是二维的）导入进去，替换其XY坐标。 [Liu Wei 6/14/2023]

			QString miu_str=QString::fromLocal8Bit("miu0:")+QString::number(miu0)+","+QString::fromLocal8Bit("miu1:")+QString::number(miu1)+","+QString::fromLocal8Bit("优化前最大互相关值:")+QString::number(miu_xj_after)+";";
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_after)+","+tr("Optimizing time: ")+QString::number(duration)+" s";
			QMessageBox::information(NULL,tr("Computation Information"),miu_str+info_str);

			// 个人认为：需要重新创建炮检关系 [Liu Wei 6/29/2023]
			GRLineSet *pRLineSetNew = mpProject->getRLineSetNew();
			GShotSet *pShotSetNew = mpProject->getShotSetNew();
			pShotSetNew->create(pSPSParam,pSLineSetNew,pRLineSetNew);

			// ***释放内存*** //
			delete mProgressDlg;
			for (int i=0;i<row;i++)
			{
				free(PHI[i]);
				free(Phi0[i]);
				free(Phi01[i]);
				free(Phi1[i]);
			}
			free(PHI);
			free(Phi0);
			free(Phi01);
			free(Phi1);

			delete [] phi0;
			delete [] phi;
			delete [] y;
			delete [] x;
			delete [] x1;
			delete [] Xcoor;
			delete [] Ycoor;

		}
		else//一维采样
		{
			// 暂时不考虑 [Liu Wei 6/30/2023]
		}

	}
	delete sample_dlg;
	qDebug()<<"hello world!";// 测试 [Liu Wei 6/30/2023]	
}

void TSPSForm::on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered()
{
	// 获取项目中设计观测系统或导入观测系统参数
	GProject *pProject = GProject::getCurrentProject();
	GSLineSet *pSLineSet = pProject->getSLineSet();
	GRLineSet *pRLineSet = pProject->getRLineSet();
	//GShotSet *pShotSet = pProject->getShotSet();	//处理关系文件的类
	GSPSParam *pSPSParam = pProject->getSPSParam(); //从底层管理类中去获取数据【模板参数无法获取的问题，基本解决】
	//GCellGrid *pCellGrid = pProject->getCellGrid();

	// 获取输入观测系统的模板参数——因为不一定正确，所以仅作为参考 [Liu Wei 8/13/2023]
	float int_rpt, int_rln, int_spt, int_sln;
	int_rpt=pSPSParam->mRPtInv;		//道间距
	int_rln=pSPSParam->mRLineInv;	//接收线距
	int_spt=pSPSParam->mSPtInv;		//炮点距
	int_sln=pSPSParam->mSLineInv;	//炮线距
	// 获取输入观测系统接收点和炮点坐标 [Liu Wei 8/13/2023]
	QVector<QVector<float> > mRLineSet_x, mSLineSet_x, mRLineSet_y, mSLineSet_y;	//提取导入观测系统接收点坐标、炮点坐标
	int RLine_num, RLine_npl, SLine_num, SLine_npl;	//线数和道数
	RLine_num=pRLineSet->count();
	for (int n=0; n<RLine_num; n++)
	{
		RLine_npl=pRLineSet->get(n)->count();
		QVector<float> mRLine_x,mRLine_y;
		for (int m=0; m<RLine_npl; m++)
		{
			mRLine_x.append(pRLineSet->get(n)->get(m)->getX());
			mRLine_y.append(pRLineSet->get(n)->get(m)->getY());
		}
		mRLineSet_x.append(mRLine_x);
		mRLineSet_y.append(mRLine_y);
	}
	SLine_num=pSLineSet->count();
	for (int n=0; n<SLine_num; n++)
	{
		SLine_npl=pSLineSet->get(n)->count();
		QVector<float> mSLine_x, mSLine_y;
		for (int m=0; m<SLine_npl; m++)
		{
			mSLine_x.append(pSLineSet->get(n)->get(m)->getX());
			mSLine_y.append(pSLineSet->get(n)->get(m)->getY());
		}
		mSLineSet_x.append(mSLine_x);//二维数组
		mSLineSet_y.append(mSLine_y);//二维数组
	}

	// 检波点
	int Rlm=ArrayLineNum(mRLineSet_x,"R");//mRLineSet_y是转换过后的坐标，显示的是实际坐标。Rlm为最长测线数值。
	int **arrayRPhi=SpsArrayTrans(mRLineSet_x,"R");//检波点0-1二维数组

	// 炮点
	int Slm=ArrayLineNum(mSLineSet_y,"S");//Slm为最长炮线数值
	int **arraySPhi= SpsArrayTrans(mSLineSet_y,"S");//炮点0-1二维数组

	float miu_r=1, miu_s=2;
	miu_r=CurveletAveMiu2D(RLine_num, Rlm, arrayRPhi, "EigenReal");	//计算检波点μ值
	miu_s=CurveletAveMiu2D(SLine_num, Slm, arraySPhi, "EigenReal"); //计算炮点μ值

	// 释放内存
	for (int i=0;i<RLine_num;i++)
	{
		free(arrayRPhi[i]);
	}
	free(arrayRPhi);
	for (int i=0;i<SLine_num;i++)
	{
		free(arraySPhi[i]);
	}free(arraySPhi);

	// 将计算得到的μ值输出 [Liu Wei 6/14/2023]
	QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ_r): ")+QString::number(miu_r)+"; "+QString::fromLocal8Bit("(μ_s): ")+QString::number(miu_s);
	QMessageBox::information(NULL,tr("Computation Information"),info_str);//输出信息可以根据需要进行调整
}

// 试验与评估模块前置状态校验与联动更新 // add by shanzhou in 2026/10/15
void TSPSForm::updateExperimentActionsState()
{
    GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
    bool hasSps = false;
    bool hasGrid = false;
    bool hasSampled = false;

    if (pProject)
    {
        GSLineSet *pSLineSet = pProject->getSLineSet();
        GRLineSet *pRLineSet = pProject->getRLineSet();
        int sCount = (pSLineSet != NULL) ? pSLineSet->count() : 0;
        int rCount = (pRLineSet != NULL) ? pRLineSet->count() : 0;
        if (sCount > 0 || rCount > 0)
        {
            hasSps = true;
        }

        GCellGrid *pCellGrid = pProject->getCellGrid();
        if (pCellGrid != NULL && pCellGrid->getCoverCount() != NULL)
        {
            hasGrid = true;
        }

        if (pProject->mIsNewImfo && (pProject->getRLineSetNew() != NULL || pProject->getSLineSetNew() != NULL))
        {
            hasSampled = true;
        }
    }

    // 状态关联：未加载SPS、未建网格、未采样时做状态控制
    ui.actionRunSamplingBenchmark->setEnabled(hasSps);
    ui.actionExportAttributeReport->setEnabled(hasSps && hasGrid && hasSampled);
    ui.actionRunWavefieldReconstruction->setEnabled(hasSps && hasSampled);
    ui.actionRunFullExperimentSuite->setEnabled(hasSps && hasGrid && hasSampled);
}

// 多采样率对比试验（带前置校验） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunSamplingBenchmark_triggered()
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProj || (!pProj->getSLineSet() && !pProj->getRLineSet()) ||
        (pProj->getSLineSet()->count() == 0 && pProj->getRLineSet()->count() == 0))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【未满足前置条件】：尚未加载观测系统！\n"
                                                  "请先点击【文件 -> 加载SPS】导入工区数据。"));
        return;
    }

    int N = 128;
    if (pProj->getRLineSet() && pProj->getRLineSet()->count() > 0)
    {
        GRLine *pLine = pProj->getRLineSet()->get(0);
        if (pLine && pLine->count() > 10)
        {
            N = pLine->count();
        }
    }

    QString outDir = QDir::current().filePath("experiment_results/01_sampling_benchmark");
    std::vector<float> rates;
    rates.push_back(0.30f);
    rates.push_back(0.50f);
    rates.push_back(0.70f);

    std::vector<GeometryExperimentEngine::BenchmarkRecord> results;
    QString summary;
    if (GeometryExperimentEngine::runSamplingBenchmark(N, rates, outDir, results, summary))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("多采样率对比试验完成"), summary);
    }
}

// 观测系统空间属性质量评估报告导出（带网格与采样前置校验） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionExportAttributeReport_triggered()
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProj || (!pProj->getSLineSet() && !pProj->getRLineSet()) ||
        (pProj->getSLineSet()->count() == 0 && pProj->getRLineSet()->count() == 0))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【第1步未完成】：尚未加载观测系统！\n"
                                                  "请先点击【文件 -> 加载SPS】导入工区数据。"));
        return;
    }

    GCellGrid *pCellGrid = pProj->getCellGrid();
    if (!pCellGrid || !pCellGrid->getCoverCount())
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【第2步未完成】：尚未新建网格或计算面元覆盖次数！\n"
                                                  "请点击工具栏【网格】按钮划分CMP面元并计算覆盖次数。"));
        return;
    }

    if (!pProj->mIsNewImfo || (!pProj->getRLineSetNew() && !pProj->getSLineSetNew()))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【第3步未完成】：尚未进行非规则采样或排列优化！\n"
                                                  "请先在【排列优化】菜单中点击【检波点采样】或【排列优化】，生成优化后的非规则排列后，系统才能对比原始与优化后的空间属性。"));
        return;
    }

    QString outDir = QDir::current().filePath("experiment_results/02_attribute_evaluation");
    GeometryExperimentEngine::AttributeMetrics metrics;
    QString summary;
    if (GeometryExperimentEngine::evaluateProjectAttributes(pProj, outDir, metrics, summary))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("观测系统空间属性评估完成"), summary);
    }
}

// 地震波场压缩感知重构试验（带采样掩码前置校验） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunWavefieldReconstruction_triggered()
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProj || (!pProj->getSLineSet() && !pProj->getRLineSet()) ||
        (pProj->getSLineSet()->count() == 0 && pProj->getRLineSet()->count() == 0))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【第1步未完成】：尚未加载观测系统！\n"
                                                  "请先点击【文件 -> 加载SPS】导入工区数据。"));
        return;
    }

    if (!pProj->mIsNewImfo || (!pProj->getRLineSetNew() && !pProj->getSLineSetNew()))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【第2步未完成】：尚未进行非规则采样或排列优化！\n"
                                                  "缺少实际工区的欠采样掩码分布。请先在【排列优化】菜单中执行【检波点采样】或【排列优化】后，再开展波场重构试验。"));
        return;
    }

    QString outDir = QDir::current().filePath("experiment_results/03_wavefield_reconstruction");
    QString maskPath = QDir::current().filePath("cmp_mask.txt");

    float actualRatio = 0.50f;
    GRLineSet *pOrigRLineSet = pProj->getOriginalRLineSet();
    GRLineSet *pNewRLineSet = pProj->getRLineSetNew();
    if (pOrigRLineSet && pNewRLineSet && pOrigRLineSet->count() > 0)
    {
        int origPts = 0, newPts = 0;
        for (int i = 0; i < pOrigRLineSet->count(); i++)
        {
            if (pOrigRLineSet->get(i)) origPts += pOrigRLineSet->get(i)->count();
        }
        for (int i = 0; i < pNewRLineSet->count(); i++)
        {
            if (pNewRLineSet->get(i)) newPts += pNewRLineSet->get(i)->count();
        }
        if (origPts > 0) actualRatio = (float)newPts / (float)origPts;
    }

    GeometryExperimentEngine::ReconstructionMetrics metrics;
    QString summary;
    if (GeometryExperimentEngine::runWavefieldReconstruction(maskPath, outDir, actualRatio, metrics, summary))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("地震波场压缩感知重构试验完成"), summary);
    }
}

// 一键运行全套科研闭环对比试验（带全流程前置校验） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunFullExperimentSuite_triggered()
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProj || (!pProj->getSLineSet() && !pProj->getRLineSet()) ||
        (pProj->getSLineSet()->count() == 0 && pProj->getRLineSet()->count() == 0))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【步骤1未完成】：尚未加载SPS观测系统数据！\n"
                                                  "请先通过【文件 -> 加载SPS】导入工区数据。"));
        return;
    }

    GCellGrid *pCellGrid = pProj->getCellGrid();
    if (!pCellGrid || !pCellGrid->getCoverCount())
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【步骤2未完成】：尚未新建网格与计算覆盖次数！\n"
                                                  "请先点击工具栏【网格】按钮划分CMP面元网格。"));
        return;
    }

    if (!pProj->mIsNewImfo || (!pProj->getRLineSetNew() && !pProj->getSLineSetNew()))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("前置条件未满足"),
                             QString::fromLocal8Bit("【步骤3未完成】：尚未执行非规则采样或排列优化！\n"
                                                  "请先在【排列优化】菜单中执行【检波点采样】或【排列优化】。"));
        return;
    }

    QString outDir = QDir::current().filePath("experiment_results");
    QString summary;
    if (GeometryExperimentEngine::runFullExperimentSuite(pProj, outDir, summary))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("全套科研闭环对比试验完成"), summary);
    }
}
