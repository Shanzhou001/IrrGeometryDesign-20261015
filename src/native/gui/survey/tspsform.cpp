#include "../../plotting/figurepaths.h"
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
#include "../../io/sps/gproject.h"
#include "tspsdlg.h"
//#include "tspscrookdlg.h"
#include "tcellgridform.h"
#include "gcellgrid.h"
#include "tspsform.h"
#include "../../experiments/geometryexperimentengine.h" // add by wushanzhou in 2026/10/15
#include <QDesktopWidget>
//#include "tbendlinewidthdlg.h"
#include "../../core/gutils.h"
#include "../../core/ExpRng.h"
//#include "../dialogs/analyzebinform.h"
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
#include <QDesktopServices>
#include <QUrl>
#include <QScrollArea>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>
#include <QFileInfo>
// =========================================================================
// 科研成果图件与科学结论画廊实现 (支持图片与定量结论一一对应)
// =========================================================================
ResearchFiguresDialog::ResearchFiguresDialog(QWidget *parent)
    : QDialog(parent), mCurrentIndex(0)
{
    setWindowTitle(QString::fromLocal8Bit("研究图件与实验说明 - [IrrGeometryDesign]"));
    resize(1060, 750);
    setSizeGripEnabled(true);

    QFont chFont("SimSun", 9);
    setFont(chFont);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QHBoxLayout *topBar = new QHBoxLayout();
    QLabel *lblSelect = new QLabel(QString::fromLocal8Bit("选择科研图件:"), this);

    mCombo = new QComboBox(this);
    mCombo->setMinimumWidth(620);
	// 2026-10-22: 条目文字较长，显式绑定弹出列表宽度，避免下拉项被截断
	mCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	mCombo->view()->setMinimumWidth(620);

    mBtnPrev = new QPushButton(QString::fromLocal8Bit("<- 上一张"), this);
    mBtnNext = new QPushButton(QString::fromLocal8Bit("下一张 ->"), this);
    mBtnBrowser = new QPushButton(QString::fromLocal8Bit("浏览器查看画廊(HTML)"), this);
    mBtnFolder = new QPushButton(QString::fromLocal8Bit("打开图件总文件夹"), this);

    topBar->addWidget(lblSelect);
    topBar->addWidget(mCombo, 1);
    topBar->addWidget(mBtnPrev);
    topBar->addWidget(mBtnNext);
    topBar->addWidget(mBtnBrowser);
    topBar->addWidget(mBtnFolder);
    mainLayout->addLayout(topBar);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);

    mScrollArea = new QScrollArea(splitter);
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setFrameShape(QFrame::NoFrame);

    mImageLabel = new QLabel(mScrollArea);
    mImageLabel->setAlignment(Qt::AlignCenter);
    mScrollArea->setWidget(mImageLabel);

    mConclusionBrowser = new QTextBrowser(splitter);
    mConclusionBrowser->setOpenExternalLinks(true);
    mConclusionBrowser->setFrameShape(QFrame::NoFrame);
    QPalette pal = mConclusionBrowser->palette();
    pal.setColor(QPalette::Base, palette().color(QPalette::Window));
    mConclusionBrowser->setPalette(pal);

    QList<int> splitSizes;
    splitSizes << 500 << 200;
    splitter->setSizes(splitSizes);

    mainLayout->addWidget(splitter, 1);

    mFileNames.clear();
    mTitles.clear();
    mConclusions.clear();

    mFileNames << "paper/search.png";
    mTitles << QString::fromLocal8Bit("01. Fourier/PSF 采样搜索比较");
    mConclusions << QString::fromLocal8Bit(
        "<b>评价口径：</b>Fourier 字典的 PSF 最大旁瓣，数值越小表示该字典下互干度越低。"
        "不同算法应使用相同初始掩码、评价预算和随机种子进行配对比较。"
        "低互干度不能直接推出更高的 Curvelet 恢复 SNR。"
    );

    mFileNames << "paper/budget.png";
    mTitles << QString::fromLocal8Bit("02. 预算回收与共同评价次数比较");
    mConclusions << QString::fromLocal8Bit(
        "<b>比较口径：</b>同时检查最终结果和共同评价次数下的结果。"
        "预算回收可能通过增加有效评价改善最终值；单位评价效率必须由共同评价次数的配对差异判断。"
        "置信区间和显著性以对应原始结果表为准。"
    );

    mFileNames << "paper/recovery.png";
    mTitles << QString::fromLocal8Bit("03. 共同隐藏集上的恢复质量比较");
    mConclusions << QString::fromLocal8Bit(
        "<b>验证口径：</b>所有采样方法在共同隐藏集 H 上计算 SNR，H 不可进入采样或恢复配置选择。"
        "固定 Curvelet 与内层选择恢复分别报告；内层选择候选包含零预测和线性插值，"
        "因此选择后的改善不能全部归因于 Curvelet。"
    );

    mFileNames << "paper/real_windows.png";
    mTitles << QString::fromLocal8Bit("04. 真实资料预设时间窗与失败情况");
    mConclusions << QString::fromLocal8Bit(
        "<b>适用范围：</b>不同时间窗和掩码划分仍来自同一个资料来源，不能视作多个独立工区。"
        "应保留负 SNR 和失败案例。资料编码、坐标单位和道集含义未确认时，"
        "图件只能作为带有这些限制的探索结果。"
    );

    for (int i = 0; i < mTitles.size(); ++i)
    {
        mCombo->addItem(mTitles[i]);
    }

    connect(mCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectionChanged(int)));
    connect(mBtnPrev, SIGNAL(clicked()), this, SLOT(onPrev()));
    connect(mBtnNext, SIGNAL(clicked()), this, SLOT(onNext()));
    connect(mBtnBrowser, SIGNAL(clicked()), this, SLOT(onOpenInBrowser()));
    connect(mBtnFolder, SIGNAL(clicked()), this, SLOT(onOpenFolder()));

    loadFigure(0);
}

void ResearchFiguresDialog::selectFigure(int index)
{
    if (index >= 0 && index < mFileNames.size())
    {
        loadFigure(index);
    }
}

void ResearchFiguresDialog::onSelectionChanged(int index)
{
    loadFigure(index);
}

void ResearchFiguresDialog::onPrev()
{
    if (mCurrentIndex > 0)
    {
        loadFigure(mCurrentIndex - 1);
    }
}

void ResearchFiguresDialog::onNext()
{
    if (mCurrentIndex < mFileNames.size() - 1)
    {
        loadFigure(mCurrentIndex + 1);
    }
}

void ResearchFiguresDialog::onOpenInBrowser()
{
    QString htmlPath = QDir(FigurePaths::root()).filePath("index.html"); // add by wushanzhou in 2026/10/23
    QFileInfo fi(htmlPath);
    if (fi.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
    else
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("未找到画廊网页"),
                             QString::fromLocal8Bit("画廊网页尚未生成：%1\n请先在命令行运行：\npython pipeline.py figures").arg(htmlPath));
    }
}

void ResearchFiguresDialog::onOpenFolder()
{
    QString dirPath = FigurePaths::root();
    QDir dir(dirPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}

void ResearchFiguresDialog::loadFigure(int index)
{
    if (index < 0 || index >= mFileNames.size()) return;
    mCurrentIndex = index;

    if (mCombo->currentIndex() != index)
    {
        mCombo->blockSignals(true);
        mCombo->setCurrentIndex(index);
        mCombo->blockSignals(false);
    }

    QString filePath = QDir(FigurePaths::root()).filePath(mFileNames[index]);
    QFileInfo fi(filePath);

    if (fi.exists())
    {
        QPixmap pix(filePath);
        if (!pix.isNull())
        {
            mImageLabel->setPixmap(pix);
            mImageLabel->adjustSize();
        }
        else
        {
            mImageLabel->setText(QString::fromLocal8Bit("<font color=red>无法解析图像文件：%1</font>").arg(filePath));
        }
    }
    else
    {
        mImageLabel->setText(QString::fromLocal8Bit(
            "<font color=red><b>图件尚未生成：</b>%1</font><br><br>"
            "请恢复对应实验记录后运行正式绘图模块。<br>"
            "python pipeline.py figures 仅刷新已有图片的图库。").arg(mFileNames[index]));
    }

    mConclusionBrowser->setHtml(mConclusions[index] + QString::fromLocal8Bit(
        "<br><br><b>归档状态：</b>保留图片不等于已验证的新实验结果。正式使用前请核对同名数据文件、原始结果表和运行 manifest。"));
    mBtnPrev->setEnabled(index > 0);
    mBtnNext->setEnabled(index < mFileNames.size() - 1);
}

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
    mAllocatedCMPX = 0;
    mAllocatedCMPY = 0;
}

TSPSForm::~TSPSForm()
{
    // 指针数据内存空间销毁
    if(offsetarray != NULL)
    {
        for(int i = 0; i < mAllocatedCMPX; i++)
        {
            for(int j = 0; j < mAllocatedCMPY; j++)
            {
                offsetarray[i][j].clear();
                Azimutharray[i][j].clear();
            }
        }

        gFree2(offsetarray);
        gFree2(Azimutharray);
        offsetarray = NULL;
        Azimutharray = NULL;
    }

#pragma region "add by wushanzhou in 2026/10/23"
    if(mOwnProject && mpProject != NULL)
    {
        mpProject->destroy();
    }
    mpProject = NULL;
    destroy();
#pragma endregion "add by wushanzhou in 2026/10/23"
}

void TSPSForm::init()
{
    ui.actionRAOptimizing->setText(QString::fromLocal8Bit("检波点自适应 PSF 优化"));
    ui.actionSAOptimizing->setText(QString::fromLocal8Bit("炮点自适应 PSF 优化"));
    ui.actionCalculate_the_Miu_for_Loaded_Geometry->setText(QString::fromLocal8Bit("历史曲波 Gram 差值诊断"));
	//----调整窗口大小和位置----
	QPoint p( 0, 0 );
	QRect desk;
	int top = 0, left = 0, width = 1040, height = 720;
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

	// 2026.10.19: mSpl 仅含 mpView 一个子控件，原代码却给出 3 个尺寸值
	// (400/100/300)，多出的两项被 Qt 忽略并在左/上侧留下空白分栏。
	// 现改为单值，主视图占满窗口。
	QList<int> list;
	list.append(1);
	mSpl->setSizes(list);

	connect(ui.saveAction, SIGNAL(triggered()), this, SLOT(saveSPS()));
	connect(ui.zoomAction, SIGNAL(triggered()), this, SLOT(zoom()));
	connect(ui.gridAction, SIGNAL(triggered()), this, SLOT(grid()));
	connect(ui.actionQuickDemoSPS, SIGNAL(triggered()), this, SLOT(quickLoadDemoSPS()));
	connect(ui.actionViewGallery, SIGNAL(triggered()), this, SLOT(on_actionViewGallery_triggered()));
	connect(ui.actionOpenFiguresDir, SIGNAL(triggered()), this, SLOT(on_actionOpenFiguresDir_triggered()));

	ui.Toolbar->addSeparator();
	ui.Toolbar->addAction(ui.actionQuickDemoSPS);
	ui.Toolbar->addAction(ui.actionViewGallery);

	// 2026.10.19: mScaleBox / textLabel3 在 .ui 中声明却从未加入工具栏布局，
	// 使窗口左上角出现一个无标签、无作用的游离输入框。Q3ToolBar 仅支持
	// addAction/addSeparator，不支持 addWidget；且该缩放输入框的作用可由
	// 菜单"视图 -> 鼠标缩放"完全覆盖，故直接隐藏，保持界面简洁。
	ui.textLabel3->hide();
	ui.mScaleBox->hide();

	setCentralWidget(mSpl);

	updateAllActionsState();
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
	updateAllActionsState();
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
void TSPSForm::saveSPS() // add by wushanzhou in 2026/10/15
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
	if (!ensureProjectLoaded(false, false)) return;
	TCellGridForm *gridForm = new TCellGridForm(this);
	if(gridForm->exec() == QDialog::Accepted)
	{
		mpView->repaintContents();
	}

	delete gridForm;
	updateAllActionsState();
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
	if (!ensureProjectLoaded(true, false)) return;
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
	GCellGrid *pCellGrid = pProject->getCellGrid();
	GSPSParam *pSPSParam = pProject->getSPSParam();

	if (!pSLineSet || !pRLineSet || !pShotSet || pSLineSet->count() == 0 || pRLineSet->count() == 0 || pShotSet->count() == 0)
	{
		quickLoadDemoSPS(false);
		pSLineSet = (pProject->mIsNewImfo && pProject->getSLineSetNew() && pProject->getSLineSetNew()->count() > 0)
		            ? pProject->getSLineSetNew() : pProject->getSLineSet();
		pRLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
		            ? pProject->getRLineSetNew() : pProject->getRLineSet();
		pShotSet = (pProject->mIsNewImfo && pProject->getShotSetNew() && pProject->getShotSetNew()->count() > 0)
		           ? pProject->getShotSetNew() : pProject->getShotSet();
		pCellGrid = pProject->getCellGrid();
		pSPSParam = pProject->getSPSParam();
	}

	if (!pCellGrid || !pCellGrid->getCoverCount() || pCellGrid->getNumX() <= 0 || pCellGrid->getNumY() <= 0)
	{
		if (pCellGrid && pSPSParam && pSLineSet && pRLineSet)
		{
			float minSX = HUGE_VAL, minSY = HUGE_VAL, maxSX = -HUGE_VAL, maxSY = -HUGE_VAL;
			float minRX = HUGE_VAL, minRY = HUGE_VAL, maxRX = -HUGE_VAL, maxRY = -HUGE_VAL;
			pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
			pRLineSet->getRange(minRX, minRY, maxRX, maxRY);

			if (minSX > maxSX) { minSX = minRX; maxSX = maxRX; minSY = minRY; maxSY = maxRY; }
			if (minRX > maxRX) { minRX = minSX; maxRX = maxSX; minRY = minSY; maxRY = maxSY; }
			if (minSX > maxSX) { minSX = 0.0f; maxSX = 8000.0f; minSY = 0.0f; maxSY = 6000.0f; minRX = 0.0f; maxRX = 8000.0f; minRY = 0.0f; maxRY = 6000.0f; }

			float x0 = (minRX + minSX) / 2.0f;
			float y0 = (minRY + minSY) / 2.0f;
			float xn = (maxRX + maxSX) / 2.0f;
			float yn = (maxRY + maxSY) / 2.0f;

			float gx = (pSPSParam->mRPtInv > 0.0f) ? (pSPSParam->mRPtInv / 2.0f) : 25.0f;
			float gy = (pSPSParam->mSPtInv > 0.0f) ? (pSPSParam->mSPtInv / 2.0f) : 25.0f;
			if (gx <= 0.1f) gx = 25.0f;
			if (gy <= 0.1f) gy = 25.0f;

			int numX = (int)((xn - x0) / gx) + 10;
			int numY = (int)((yn - y0) / gy) + 10;
			if (numX < 20) numX = 80;
			if (numY < 20) numY = 60;
			if (numX > 600) numX = 600;
			if (numY > 600) numY = 600;

			pCellGrid->mDiffX = -gx / 2.0f;
			pCellGrid->mDiffY = -gy / 2.0f;
			pCellGrid->create(pSPSParam, x0 + pCellGrid->mDiffX, y0 + pCellGrid->mDiffY, numX, numY, gx, gy);
			pCellGrid->calculateCoverCount();
		}
	}

	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;

	minSX = minSY = LONG_MAX;
	maxSX = maxSY = LONG_MIN;
	minRX = minRY = LONG_MAX;
	maxRX = maxRY = LONG_MIN;

	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);

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

	int CMPxbinnumber=pCellGrid->getNumX();
	int CMPybinnumber=pCellGrid->getNumY();

	int CMPmaxfold=0;
	if (pCellGrid->getCoverCount())
	{
		for(int i=0;i<CMPxbinnumber;i++)
		{
			for(int j=0;j<CMPybinnumber;j++)
			{
				int f = pCellGrid->getCoverCount()[j][i];
				if(CMPmaxfold < f) CMPmaxfold = f;
			}
		}
	}

	// 安全释放前次分析可能遗留的 offsetarray 与 Azimutharray
	if(offsetarray!=NULL) 
	{
		for(int i=0;i<mAllocatedCMPX;i++)
		{
			for(int j=0;j<mAllocatedCMPY;j++)
			{
				offsetarray[i][j].clear();
				Azimutharray[i][j].clear();
			}
		}	
		gFree2(offsetarray);
		gFree2(Azimutharray);	
		offsetarray = NULL;
		Azimutharray = NULL;
	}

	mAllocatedCMPX = CMPxbinnumber;
	mAllocatedCMPY = CMPybinnumber;

	gAlloc2(offsetarray,CMPxbinnumber,CMPybinnumber);
	gAlloc2(Azimutharray,CMPxbinnumber,CMPybinnumber);
	for(int i=0;i<CMPxbinnumber;i++)
	{
		for(int j=0;j<CMPybinnumber;j++)
		{
			offsetarray[i][j].append(-1);
			Azimutharray[i][j].append(-1);
		}
	}

	for(int ii=0;ii<pShotSet->count();ii++)
	{
		GShot *pShot = pShotSet->get(ii);
		if(pShot != 0 && pShot->getSPoint() != 0)
		{
			float offsetx = pShot->getSPoint()->getRandomOffsetX();
			float offsety = pShot->getSPoint()->getRandomOffsetY();
			double sx=pShot->getSPoint()->getX()+offsetx;
			double sy=pShot->getSPoint()->getY()+offsety;
			double ssx=pShot->getSPoint()->getSPSX()+offsetx;
			double ssy=pShot->getSPoint()->getSPSY()+offsety;
			for(int si=0; si<pShot->getTrcCount(); si++)
			{
				GRPoint *rpoint = pShot->getRPointByTrcIndex(si,pRLineSet);
				if(rpoint==NULL)
					continue;
				float rx = rpoint->getX()+offsetx;
				float ry = rpoint->getY()+offsety;
				float rrx = rpoint->getSPSX()+offsetx;
				float rry  = rpoint->getSPSY()+offsety;

				float tempcmpx = (rx+sx)/2;
				float tempcmpy = (ry+sy)/2;
				int i = (tempcmpx-x0)/pCellGrid->mGridX;
				int j = (tempcmpy-y0)/pCellGrid->mGridY;
				if(i<0||j<0)
					continue; 	
				if(i>=CMPxbinnumber||j>=CMPybinnumber)
					continue; 
				short offset=sqrt((float)((sx-rx)*(sx-rx)+(sy-ry)*(sy-ry)));//炮检距

				if (offsetarray[i][j].count() == 1 && offsetarray[i][j][0] == -1)
				{
					offsetarray[i][j][0] = offset;
				}
				else 
				{
					offsetarray[i][j].append(offset);
				}

				float temp1 = (float)(rry - ssy);
				float temp2 = (float)(rrx - ssx);
				float temp3;
				if (temp2 == 0.0f)
					temp3 = 100000.0f;
				else
					temp3 = temp1 / temp2;

				float temp = atanf(temp3) * 180.0f / 3.14159265f;
				short tarray = 0;
				if (rrx < ssx)
				{
					if (rry < ssy)	
						tarray = (short)(270 - fabs(temp));
					else 
						tarray = (short)(270 + fabs(temp));
				}
				else 
				{
					if (rry < ssy)	
						tarray = (short)(90 + fabs(temp));
					else 
						tarray = (short)(90 - fabs(temp));
				}
				if (Azimutharray[i][j].count() == 1 && Azimutharray[i][j][0] == -1)
				{
					Azimutharray[i][j][0] = tarray;
				}
				else 
				{
					Azimutharray[i][j].append(tarray);
				}
			}									
		}
	}

	int realMaxFold = 0;
	for (int i = 0; i < CMPxbinnumber; i++)
	{
		for (int j = 0; j < CMPybinnumber; j++)
		{
			int traceCnt = 0;
			for (int k = 0; k < offsetarray[i][j].count(); k++)
			{
				if (offsetarray[i][j][k] != -1) traceCnt++;
			}
			if (traceCnt > realMaxFold) realMaxFold = traceCnt;
		}
	}
	if (realMaxFold > CMPmaxfold) CMPmaxfold = realMaxFold;
	if (CMPmaxfold <= 0) CMPmaxfold = 1;

	analyzebinform *dlg = new analyzebinform(this);
	bool valueanalyzebinWorkArea = (CMPmaxfold > 0);
	dlg->setAnalyzedata(offsetarray, Azimutharray, CMPxbinnumber, CMPybinnumber, CMPmaxfold + 1, pCellGrid->mGridX, pCellGrid->mGridY, 0);

	QString strAzimuth = "PP: " + QString::fromLocal8Bit("方位角:0-360");
	QString strOffset = QString::fromLocal8Bit("炮检距:0-") + QString::number(pCellGrid->mMaxoffset > 0 ? pCellGrid->mMaxoffset : 3200);
	dlg->setAODividNum(1, 1, 0, 0, 360, pCellGrid->mMaxoffset > 0 ? pCellGrid->mMaxoffset : 3200, strAzimuth, strOffset);
	dlg->CMPLayoutAttibuteConculate(pProject->getCellGrid()->getCoverCount());	
	dlg->setDataValueworkarea(valueanalyzebinWorkArea);	
	dlg->on_Bin_Fold_triggered(); // 打开窗口默认立即呈现真实 CMP 面元覆盖次数二维热力图
	dlg->exec();
	delete dlg;
}

//计算并显示OVT体属性
//所有OVT体属性：（ａ）最小炮检距；（ｂ）最大炮检距；（ｃ）平均炮检距；（ｄ）炮检距容差（方差或标准差）；（e）方位角容差（方差或标准差） [10/26/2022 Liu Wei]
void TSPSForm::on_actionOVT_Volume_Attributes_triggered()
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
	GSPSParam *pSPSParam = pProject->getSPSParam();
	GCellGrid *pCellGrid = pProject->getCellGrid();

	if (!pSLineSet || !pRLineSet || !pShotSet || !pSPSParam || !pCellGrid ||
	    pSLineSet->count() == 0 || pRLineSet->count() == 0 || pShotSet->count() == 0 ||
	    pCellGrid->getNumX() <= 0 || pCellGrid->getNumY() <= 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("观测系统数据或面元网格未就绪，无法计算 OVT 体属性。"));
		return;
	}

	// 获取炮点和检波点坐标的范围
	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;
	minSX = minSY = LONG_MAX;
	maxSX = maxSY = LONG_MIN;
	minRX = minRY = LONG_MAX;
	maxRX = maxRY = LONG_MIN;
	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);

	float x0, y0, xn, yn;
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

	QList<float> Hdy, Vdx;
	int	Num_ShotPerSLine, Num_RecPerRecLine, Num_RecLine;
	float int_shotpoint, int_receiver, int_RecLine, int_SLine;
	float Hxi_receiver, Vyi_receiver, Hxi_shot, Vyi_shot;
	float maxDis_nonlongitudinal;
	float maxDis_longitudinal;

	Num_ShotPerSLine=pSPSParam->mSPtCount;
	Num_RecPerRecLine=pSPSParam->mRPtCount;
	Num_RecLine=pSPSParam->mRLineCount;
	int_shotpoint=pSPSParam->mSPtInv;
	int_receiver=pSPSParam->mRPtInv;
	int_RecLine=pSPSParam->mRLineInv;
	int_SLine=pSPSParam->mSLineInv;

	maxDis_nonlongitudinal=pCellGrid->mCLMaxoffset;
	maxDis_longitudinal=pCellGrid->mInMaxoffset;

	float gy, gx; //横、纵向OVT片间距-可用于构建绘图坐标-根据模板参数自动计算
	int maxgys_y=gcd( int(int_RecLine),int(int_shotpoint) ); //求两个数的最大公约数
	int maxgys_x=gcd( int(int_receiver),int(int_SLine) ); //求两个数的最大公约数
	if (maxgys_y <= 0) maxgys_y = 1;
	if (maxgys_x <= 0) maxgys_x = 1;
	gy=int(int_shotpoint*int_RecLine)/maxgys_y; //求两个数的最小公倍数
	gx=int(int_receiver*int_SLine)/maxgys_x; //求两个数的最小公倍数
	if (gy <= 0) gy = 50.0f;
	if (gx <= 0) gx = 50.0f;

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

				if(cell_i>=0 && cell_j>=0 && cell_i < CMPxbinnumber && cell_j < CMPybinnumber)
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
    updateSampling(true,false);
}

void TSPSForm::on_actionSSample_triggered()
{
    updateSampling(false,false);
}

// GUI and research CLI share the Fourier/PSF adaptive search. Each line is
// optimized independently in index coordinates; this is not a joint 2D search.
static std::vector<int> guiInitialMask(int count, int keep, int mode,
                                     const QVector<int>& saved, unsigned long long seed)
{
    std::vector<int> mask(count, 0);
    if (saved.size() == count)
    {
        for (int i=0; i<count; ++i) mask[i]=saved[i];
        if (ValidateSamplingMask(count, keep, &mask[0])) return mask;
    }
    exprng::seed(seed);
    int* generated = mode==0 ? Sampling1S_Count(count,keep)
                   : mode==1 ? Sampling1J_Count(count,keep) : Sampling1F_Count(count,keep);
    if (!generated) return std::vector<int>();
    mask.assign(generated,generated+count);
    delete [] generated;
    return mask;
}

static bool guiOptimizeMask(const std::vector<int>& initial, unsigned long long seed,
                            std::vector<int>& optimized, QString& error)
{
    int keep=0;
    for (size_t i=0; i<initial.size(); ++i) keep+=initial[i];
    if (keep==0 || initial.empty()) { error="empty initial mask"; return false; }
    // Full selection and one retained point have no useful coherence search.
    if (keep==(int)initial.size() || keep==1) { optimized=initial; return true; }
    ConstraintWeights weights;
    AdaptiveSearchOptions options;
    ConstrainedResult result;
    if (!RunAdaptiveSamplingSearch((int)initial.size(),keep,initial,weights,seed,600,options,result))
    {
        error=QString::fromLatin1(result.status.c_str());
        return false;
    }
    optimized=result.mask;
    return true;
}

void TSPSForm::on_actionRAOptimizing_triggered()
{
    updateSampling(true,true);
}

void TSPSForm::on_actionSAOptimizing_triggered()
{
    updateSampling(false,true);
}

void TSPSForm::on_Cancel_clicked()
{
    close();
}

void TSPSForm::updateSampling(bool receiver, bool optimize)
{
    if (!ensureProjectLoaded(false,false)) return;
    GProject* project=mpProject ? mpProject : GProject::getCurrentProject();
    GSPSParam* parameters=project ? project->getSPSParam() : NULL;
    if (!parameters) return;
    SpsSampleDlg dialog(this);
    if (!dialog.exec()) return;
    const double traceRate=dialog.Tsampling_rate, lineRate=dialog.Lsampling_rate;
    const int lineKeep=receiver ? parameters->mRLineCount : parameters->mSLineCount;
    const int pointKeep=receiver ? parameters->mRPtCount : parameters->mSPtCount;
    if (!(traceRate>0 && traceRate<=1 && lineRate>0 && lineRate<=1) || lineKeep<1 || pointKeep<1)
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("采样参数无效"),
            QString::fromLocal8Bit("线内与线间采样率必须在 (0,1] 内，且模板点数必须大于零。"));
        return;
    }
    const double rowValue=ceil(lineKeep/lineRate), columnValue=ceil(pointKeep/traceRate);
    if (rowValue>2048 || columnValue>2048)
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("采样规模超限"),
            QString::fromLocal8Bit("当前自适应研究实现每个索引方向最多支持 2048 个候选点。请提高采样率或缩小模板。"));
        return;
    }
    const int rows=(int)rowValue, columns=(int)columnValue;
    GRLineSet* receivers=project->getRLineSetNew();
    GSLineSet* sources=project->getSLineSetNew();
    bool ready=receivers && sources && project->getShotSetNew()
        && receivers->count()>0 && sources->count()>0;
    ready=ready && (receiver ? receivers->count()==lineKeep : sources->count()==lineKeep);
    for (int i=0; ready && i<lineKeep; ++i)
        ready=receiver ? receivers->get(i) && receivers->get(i)->count()==pointKeep
                       : sources->get(i) && sources->get(i)->count()==pointKeep;
    if (!ready)
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("需要规则观测模板"),
            QString::fromLocal8Bit("此功能优化规则模板的线间与线内索引。请先创建与参数一致的观测系统；导入的任意二维坐标不能直接按此模板改写。"));
        return;
    }
    QVector<QVector<int> >& savedPoints=receiver ? R_Sampling_Arrays : S_Sampling_Arrays;
    QVector<int>& savedLines=receiver ? RL_Sampling : SL_Sampling;
    const unsigned long long seed=20261026ULL;
    std::vector<int> initialLines=guiInitialMask(rows,lineKeep,dialog.mode_value,
        optimize ? savedLines : QVector<int>(),seed);
    if (initialLines.empty()) return;
    std::vector<std::vector<int> > initialPoints(rows), finalPoints(rows);
    for (int i=0; i<rows; ++i)
    {
        QVector<int> saved=optimize && i<savedPoints.size() ? savedPoints[i] : QVector<int>();
        initialPoints[i]=guiInitialMask(columns,pointKeep,dialog.mode_value,saved,seed+179ULL*(i+1));
        if (initialPoints[i].empty()) return;
    }
    QProgressDialog progress(QString::fromLocal8Bit("正在计算线间及线内 Fourier/PSF 指标..."),
        QString::fromLocal8Bit("取消"),0,lineKeep+1,this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setAutoClose(false);
    progress.setAutoReset(false);
    progress.show();
    QApplication::processEvents();
    if (progress.wasCanceled()) return;
    const clock_t started=clock();
    QString error;
    std::vector<int> finalLines;
    finalLines=initialLines;
    if (optimize && !guiOptimizeMask(initialLines,seed,finalLines,error))
    {
        progress.close();
        QMessageBox::warning(this,QString::fromLocal8Bit("优化未完成"),error);
        return;
    }
    progress.setValue(1);
    QApplication::processEvents();
    if (progress.wasCanceled()) return;
    float beforeWithin=0, afterWithin=0;
    int completed=1;
    for (int i=0; i<rows; ++i)
    {
        finalPoints[i]=initialPoints[i];
        if (!finalLines[i]) continue;
        if (progress.wasCanceled()) return;
        if (optimize && !guiOptimizeMask(initialPoints[i],seed+179ULL*(i+1),finalPoints[i],error))
        {
            progress.close();
            QMessageBox::warning(this,QString::fromLocal8Bit("优化未完成"),error);
            return;
        }
        // Compare the same selected lines before and after within-line search.
        beforeWithin+=SamplingMaskCoherence(initialPoints[i]);
        afterWithin+=SamplingMaskCoherence(finalPoints[i]);
        progress.setValue(++completed);
        QApplication::processEvents();
        if (progress.wasCanceled()) return;
    }
    QVector<QVector<float> > coordinatesX,coordinatesY;
    QVector<QVector<int> > nextPoints;
    QVector<int> nextLines;
    for (int i=0; i<rows; ++i)
    {
        QVector<int> rowMask;
        for (int j=0; j<columns; ++j) rowMask.append(finalPoints[i][j]);
        nextPoints.append(rowMask);
        nextLines.append(finalLines[i]);
        if (!finalLines[i]) continue;
        QVector<float> x,y;
        for (int j=0; j<columns; ++j)
        {
            if (!finalPoints[i][j]) continue;
            if (receiver)
            {
                x.append(parameters->mRX0+j*parameters->mRPtInv*traceRate);
                y.append(parameters->mRY0+i*parameters->mRLineInv*lineRate);
            }
            else
            {
                x.append(parameters->mSX0+i*parameters->mSLineInv*lineRate);
                y.append(parameters->mSY0+j*parameters->mSPtInv*traceRate);
            }
        }
        coordinatesX.append(x); coordinatesY.append(y);
    }
    // All computation remains local until cancellation has been checked.
    if (progress.wasCanceled()) return;
    progress.close();
    if (receiver) receivers->create(coordinatesX,coordinatesY,parameters);
    else sources->create(coordinatesX,coordinatesY,parameters);
    savedPoints=nextPoints; savedLines=nextLines;
    project->getShotSetNew()->create(parameters,sources,receivers);
    mIsFill=true; mIsLiner=true;
    mpView->mIsImpotSps=false; project->mIsNewImfo=true;
    if (project->getCellGrid()) project->getCellGrid()->calculateCoverCount();
    refresh(); mpView->repaintContents();
    const QString status=optimize ? QString::fromLocal8Bit("自适应混合优化完成") : QString::fromLocal8Bit("精确点数采样完成");
    const QString message=status+QString::fromLocal8Bit(
        "（Fourier/PSF，索引坐标）：\n"
        "线间互干度：%1 -> %2\n"
        "选中线的线内平均互干度：%3 -> %4\n"
        "优化模式每个非退化搜索最多 600 次目标评价，固定种子 20261026。\n"
        "CPU 耗时：%5 秒。\n\n"
        "线间和每条线独立优化；该结果不等同于真实二维物理坐标联合优化，也不直接证明恢复 SNR 提升。")
        .arg(SamplingMaskCoherence(initialLines),0,'f',4)
        .arg(SamplingMaskCoherence(finalLines),0,'f',4)
        .arg(beforeWithin/lineKeep,0,'f',4).arg(afterWithin/lineKeep,0,'f',4)
        .arg((double)(clock()-started)/CLOCKS_PER_SEC,0,'f',2);
    QMessageBox::information(this,status,message);
}

void TSPSForm::on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered()
{
	if (!ensureProjectLoaded(false, false)) return;
	GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
	if (!pProject) return;

	GSLineSet *pSLineSet = (pProject->mIsNewImfo && pProject->getSLineSetNew() && pProject->getSLineSetNew()->count() > 0)
	                       ? pProject->getSLineSetNew()
	                       : ((pProject->getSLineSet() && pProject->getSLineSet()->count() > 0) ? pProject->getSLineSet() : pProject->getSLineSetNew());
	GRLineSet *pRLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
	                       ? pProject->getRLineSetNew()
	                       : ((pProject->getRLineSet() && pProject->getRLineSet()->count() > 0) ? pProject->getRLineSet() : pProject->getRLineSetNew());

	if (!pRLineSet || pRLineSet->count() == 0 || !pSLineSet || pSLineSet->count() == 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("当前工程中没有可用的炮线或检波线数据，无法执行历史 Gram 差值诊断。"));
		return;
	}

	QVector<QVector<float> > mRLineSet_x, mSLineSet_x, mRLineSet_y, mSLineSet_y;
	int RLine_num = pRLineSet->count();
	for (int n = 0; n < RLine_num; n++)
	{
		GRLine *pLine = pRLineSet->get(n);
		if (!pLine || pLine->count() == 0) continue;
		int RLine_npl = pLine->count();
		QVector<float> mRLine_x, mRLine_y;
		for (int m = 0; m < RLine_npl; m++)
		{
			GRPoint *pt = pLine->get(m);
			if (!pt) continue;
			float rx = pt->getX();
			if (rx == 0.0f) rx = pt->getSPSX();
			float ry = pt->getY();
			if (ry == 0.0f) ry = pt->getSPSY();
			mRLine_x.append(rx);
			mRLine_y.append(ry);
		}
		if (!mRLine_x.isEmpty())
		{
			mRLineSet_x.append(mRLine_x);
			mRLineSet_y.append(mRLine_y);
		}
	}

	int SLine_num = pSLineSet->count();
	for (int n = 0; n < SLine_num; n++)
	{
		GSLine *pLine = pSLineSet->get(n);
		if (!pLine || pLine->count() == 0) continue;
		int SLine_npl = pLine->count();
		QVector<float> mSLine_x, mSLine_y;
		for (int m = 0; m < SLine_npl; m++)
		{
			GSPoint *pt = pLine->get(m);
			if (!pt) continue;
			float sx = pt->getX();
			if (sx == 0.0f) sx = pt->getSPSX();
			float sy = pt->getY();
			if (sy == 0.0f) sy = pt->getSPSY();
			mSLine_x.append(sx);
			mSLine_y.append(sy);
		}
		if (!mSLine_x.isEmpty())
		{
			mSLineSet_x.append(mSLine_x);
			mSLineSet_y.append(mSLine_y);
		}
	}

	if (mRLineSet_x.isEmpty() || mSLineSet_y.isEmpty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("观测系统坐标数据有效道数不足，无法计算相干度。"));
		return;
	}

	// 检波点
	int Rlm = ArrayLineNum(mRLineSet_x, "R");
	int **arrayRPhi = SpsArrayTrans(mRLineSet_x, "R");

	// 炮点
	int Slm = ArrayLineNum(mSLineSet_y, "S");
	int **arraySPhi = SpsArrayTrans(mSLineSet_y, "S");

    if(Rlm<=0 || Slm<=0 || Rlm>1024 || Slm>1024 || !arrayRPhi || !arraySPhi) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        if(arrayRPhi){for(int row=0;row<mRLineSet_x.size();row++)free(arrayRPhi[row]);free(arrayRPhi);} // add by wushanzhou in 2026/10/24
        if(arraySPhi){for(int row=0;row<mSLineSet_y.size();row++)free(arraySPhi[row]);free(arraySPhi);} // add by wushanzhou in 2026/10/24
        QMessageBox::warning(this,QString::fromLocal8Bit("坐标诊断无法执行"),QString::fromLocal8Bit("请检查空坐标、非有限值和栅格跨度。历史稠密 Gram 诊断仅支持最多 1024 列；大规模观测系统请使用正式 Fourier/PSF 实验。")); // add by wushanzhou in 2026/10/24
        return; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24

	float miu_r = 1.0f, miu_s = 1.0f;
	if (Rlm > 0 && arrayRPhi)
	{
		miu_r = CurveletAveMiu2D(mRLineSet_x.size(), Rlm, arrayRPhi, "EigenReal");
	}
	if (Slm > 0 && arraySPhi)
	{
		miu_s = CurveletAveMiu2D(mSLineSet_y.size(), Slm, arraySPhi, "EigenReal");
	}

	// 释放内存
	if (arrayRPhi)
	{
		for (int i = 0; i < mRLineSet_x.size(); i++)
		{
			if (arrayRPhi[i]) free(arrayRPhi[i]);
		}
		free(arrayRPhi);
	}
	if (arraySPhi)
	{
		for (int i = 0; i < mSLineSet_y.size(); i++)
		{
			if (arraySPhi[i]) free(arraySPhi[i]);
		}
		free(arraySPhi);
	}

	float miu_avg = 0.5f * (miu_r + miu_s);
	QString info_str = QString::fromLocal8Bit("当前观测系统历史曲波 Gram 诊断结果：\n\n") + // add by wushanzhou in 2026/10/24
		QString::fromLocal8Bit("● 检波排列 Gram 差值: ") + QString::number(miu_r, 'f', 4) + QString::fromLocal8Bit("\n") +
		QString::fromLocal8Bit("● 震源排列 Gram 差值: ") + QString::number(miu_s, 'f', 4) + QString::fromLocal8Bit("\n") +
		QString::fromLocal8Bit("● 综合平均 Gram 差值: ") + QString::number(miu_avg, 'f', 4) + QString::fromLocal8Bit("\n\n") +
		QString::fromLocal8Bit("统计基础：检波线 ") + QString::number(mRLineSet_x.size()) + QString::fromLocal8Bit(" 条，10 m 栅格列数 ") + QString::number(Rlm) + // add by wushanzhou in 2026/10/24
		QString::fromLocal8Bit("；激发线 ") + QString::number(mSLineSet_y.size()) + QString::fromLocal8Bit(" 条，栅格列数 ") + QString::number(Slm) + QString::fromLocal8Bit("。\n") + // add by wushanzhou in 2026/10/24
		QString::fromLocal8Bit("注：此处为历史曲波 Gram 差值诊断。正式论文实验使用独立的 Fourier/PSF 指标；该诊断值不能直接证明重构或抗混叠效果改善。"); // add by wushanzhou in 2026/10/24
	QMessageBox::information(this, QString::fromLocal8Bit("相干度计算完成"), info_str);
}

// 试验与评估模块前置状态校验与联动更新 // add by wushanzhou in 2026/10/15
void TSPSForm::updateAllActionsState()
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    bool hasSurvey = false;
    bool hasGrid = false;

    if (pProj)
    {
        if ((pProj->getSLineSet() && pProj->getSLineSet()->count() > 0) ||
            (pProj->getRLineSet() && pProj->getRLineSet()->count() > 0) ||
            (pProj->getSLineSetNew() && pProj->getSLineSetNew()->count() > 0) ||
            (pProj->getRLineSetNew() && pProj->getRLineSetNew()->count() > 0))
        {
            hasSurvey = true;
        }

        GCellGrid *pCellGrid = pProj->getCellGrid();
        if (pCellGrid && pCellGrid->getCoverCount() != NULL && pCellGrid->getNumX() > 0 && pCellGrid->getNumY() > 0)
        {
            hasGrid = true;
        }
    }

    // 基础与保存
    ui.saveAction->setEnabled(hasSurvey);
    ui.gridAction->setEnabled(hasSurvey);

    // 属性分析与OVT（具备自动预加载与自适应网格就绪保障，允许随时点击）
    ui.actionGeometry_Attribute_Plotting->setEnabled(true);
    ui.actionOVT_Volume_Attributes->setEnabled(true);
    ui.actionOVT_Patch_Attributes->setEnabled(true);

    // 优化采样功能
    ui.actionRSample->setEnabled(true);
    ui.actionRAOptimizing->setEnabled(true);
    ui.actionSSample->setEnabled(true);
    ui.actionSAOptimizing->setEnabled(true);
    ui.actionCalculate_the_Miu_for_Loaded_Geometry->setEnabled(true);

    // 科研试验相关
    ui.actionViewGallery->setEnabled(true);
    ui.actionOpenFiguresDir->setEnabled(true);
}

void TSPSForm::updateExperimentActionsState()
{
    updateAllActionsState();
}





// 快速加载标准示范三维地震观测系统
void TSPSForm::quickLoadDemoSPS(bool showMsgBox)
{
    GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
    if (!pProject) return;

    mpView->stop();
    GSPSParam *pSPSParam = pProject->getSPSParam();
    pSPSParam->reset();

    // 设定标准示范三维观测系统参数
    pSPSParam->mMaxLOffset = 1200;
    pSPSParam->mMinLOffset = 0;
    pSPSParam->mMaxROffset = 1200;
    pSPSParam->mMinROffset = 0;

    pSPSParam->mAf = 0;
    pSPSParam->mRLineCount = 8;
    pSPSParam->mRLineInv = 200.0f;
    pSPSParam->mRPtCount = 128;
    pSPSParam->mRPtInv = 50.0f;
    pSPSParam->mRX0 = 10000.0f;
    pSPSParam->mRY0 = 10000.0f;
    pSPSParam->mRZ0 = 0.0f;

    pSPSParam->mMoveX0 = 0.0f;
    pSPSParam->mMoveY0 = 0.0f;
    pSPSParam->mOrginalByReceiver = true;
    pSPSParam->mOrginalByShot = false;

    pSPSParam->mSAf = (float)(3.14159265358979323846 / 2.0);
    pSPSParam->mSLineType = GSPSParam::gCrossLine;
    pSPSParam->mSLineCount = 12;
    pSPSParam->mSLineInv = 250.0f;
    pSPSParam->mSPtCount = 25;
    pSPSParam->mSPtInv = 50.0f;
    pSPSParam->mSX0 = pSPSParam->mRX0 + pSPSParam->mMaxLOffset;
    pSPSParam->mSY0 = pSPSParam->mRY0 + ((pSPSParam->mRLineCount - 1) * pSPSParam->mRLineInv - (pSPSParam->mSPtCount - 1) * pSPSParam->mSPtInv) / 2.0f;
    pSPSParam->mSZ0 = 0.0f;
    pSPSParam->mSinAf = 0.0f;
    pSPSParam->mCosAf = 1.0f;

    GRLineSet *pRLineSetNew = pProject->getRLineSetNew();
    pRLineSetNew->create(pSPSParam);

    GSLineSet *pSLineSetNew = pProject->getSLineSetNew();
    pSLineSetNew->create(pSPSParam);

    pSPSParam->mTransX0 = MAX_FLOAT;
    pSPSParam->mTransY0 = MAX_FLOAT;

    pRLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
    pSLineSetNew->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
    pRLineSetNew->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);
    pSLineSetNew->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);

    pProject->setTransX0(pSPSParam->mTransX0);
    pProject->setTransY0(pSPSParam->mTransY0);

    float mMinSPSX = MAX_FLOAT, mMinSPSY = MAX_FLOAT;
    float mMaxSPSX = MIN_FLOAT, mMaxSPSY = MIN_FLOAT;
    pRLineSetNew->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);
    pSLineSetNew->getSPSRange(mMinSPSX, mMinSPSY, mMaxSPSX, mMaxSPSY);

    GShotSet *pShotSetNew = pProject->getShotSetNew();
    pShotSetNew->create(pSPSParam, pSLineSetNew, pRLineSetNew);

    pProject->mLength = mMaxSPSX - mMinSPSX + 2000.0f;
    pProject->mWidth = mMaxSPSY - mMinSPSY + 2000.0f;
    pProject->mX0 = mMinSPSX - 1000.0f;
    pProject->mZ0 = mMinSPSY - 1000.0f;

    // 同时初始化并同步原始线组，确保双向坐标与拓扑严格一致
    GRLineSet *pRLineSetOrig = pProject->getRLineSet();
    GSLineSet *pSLineSetOrig = pProject->getSLineSet();
    if (pRLineSetOrig)
    {
        pRLineSetOrig->create(pSPSParam);
        pRLineSetOrig->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
        pRLineSetOrig->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);
    }
    if (pSLineSetOrig)
    {
        pSLineSetOrig->create(pSPSParam);
        pSLineSetOrig->rotate(pSPSParam->mSinAf, pSPSParam->mCosAf, pSPSParam->mTransX0, pSPSParam->mTransY0);
        pSLineSetOrig->translate(-pSPSParam->mTransX0, -pSPSParam->mTransY0);
    }
    GShotSet *pShotSetOrig = pProject->getShotSet();
    if (pShotSetOrig && pSLineSetOrig && pRLineSetOrig)
        pShotSetOrig->create(pSPSParam, pSLineSetOrig, pRLineSetOrig);

    // 自动构建 CMP 面元网格 (25m x 25m)
    GCellGrid *pCellGrid = pProject->getCellGrid();
    if (pCellGrid)
    {
        float gx = 25.0f, gy = 25.0f;
        float x0 = mMinSPSX - 12.5f;
        float y0 = mMinSPSY - 12.5f;
        int numX = (int)((mMaxSPSX - mMinSPSX) / gx) + 10;
        int numY = (int)((mMaxSPSY - mMinSPSY) / gy) + 10;
        if (numX < 20) numX = 80;
        if (numY < 20) numY = 60;
        pCellGrid->mDiffX = -12.5f;
        pCellGrid->mDiffY = -12.5f;
        pCellGrid->create(pSPSParam, x0, y0, numX, numY, gx, gy);
        pCellGrid->calculateCoverCount();
    }

    mIsFill = true;
    mIsLiner = true;
    mpView->mIsImpotSps = false;
    pProject->mIsNewImfo = true;

    // 初始化采样优化数组，杜绝未执行采样时直接运行排列优化的空指针/越界
    R_Sampling_Arrays.clear();
    RL_Sampling.clear();
    for (int i = 0; i < pSPSParam->mRLineCount; i++)
    {
        QVector<int> sv;
        for (int j = 0; j < pSPSParam->mRPtCount; j++)
        {
            sv.append(1);
        }
        R_Sampling_Arrays.append(sv);
        RL_Sampling.append(1);
    }

    S_Sampling_Arrays.clear();
    SL_Sampling.clear();
    for (int i = 0; i < pSPSParam->mSLineCount; i++)
    {
        QVector<int> sv;
        for (int j = 0; j < pSPSParam->mSPtCount; j++)
        {
            sv.append(1);
        }
        S_Sampling_Arrays.append(sv);
        SL_Sampling.append(1);
    }

    refresh();
    mpView->repaintContents();
    updateAllActionsState();

    if (showMsgBox) {
    QMessageBox::information(this,
        QString::fromLocal8Bit("示范工区加载成功"),
        QString::fromLocal8Bit("已成功加载标准示范三维地震观测系统！\n"
                              "● 接收线数: 8 线，每线 128 道 (道距 50m, 线距 200m)\n"
                              "● 激发线数: 12 线，每线 25 炮 (炮距 50m, 线距 250m)\n"
                              "● CMP面元网格: 25m x 25m, 覆盖次数已自动计算完成。\n\n"
                              "所有后续流程按钮已就绪，可直接点击进行空间采样优化、属性分析、压缩感知重构或浏览科研成果图件！"));
    }
}

// 保证工程数据就绪辅助校验函数
bool TSPSForm::ensureProjectLoaded(bool needGrid, bool needSampled)
{
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();
    bool hasLines = false;
    if (pProj)
    {
        if ((pProj->getSLineSet() && pProj->getSLineSet()->count() > 0) ||
            (pProj->getRLineSet() && pProj->getRLineSet()->count() > 0) ||
            (pProj->getSLineSetNew() && pProj->getSLineSetNew()->count() > 0) ||
            (pProj->getRLineSetNew() && pProj->getRLineSetNew()->count() > 0))
        {
            hasLines = true;
        }
    }

    if (!hasLines)
    {
        quickLoadDemoSPS(false);
        pProj = mpProject ? mpProject : GProject::getCurrentProject();
        hasLines = true;
    }

    if (needGrid)
    {
        GCellGrid *pCellGrid = pProj ? pProj->getCellGrid() : NULL;
        if (!pCellGrid || !pCellGrid->getCoverCount() || pCellGrid->getNumX() <= 0 || pCellGrid->getNumY() <= 0)
        {
            GSPSParam *pSPSParam = pProj ? pProj->getSPSParam() : NULL;
            if (pCellGrid && pSPSParam)
            {
                float minSX = HUGE_VAL, minSY = HUGE_VAL, maxSX = -HUGE_VAL, maxSY = -HUGE_VAL;
                float minRX = HUGE_VAL, minRY = HUGE_VAL, maxRX = -HUGE_VAL, maxRY = -HUGE_VAL;

                if (pProj->getSLineSetNew() && pProj->getSLineSetNew()->count() > 0)
                    pProj->getSLineSetNew()->getRange(minSX, minSY, maxSX, maxSY);
                else if (pProj->getSLineSet() && pProj->getSLineSet()->count() > 0)
                    pProj->getSLineSet()->getRange(minSX, minSY, maxSX, maxSY);

                if (pProj->getRLineSetNew() && pProj->getRLineSetNew()->count() > 0)
                    pProj->getRLineSetNew()->getRange(minRX, minRY, maxRX, maxRY);
                else if (pProj->getRLineSet() && pProj->getRLineSet()->count() > 0)
                    pProj->getRLineSet()->getRange(minRX, minRY, maxRX, maxRY);

                if (minSX > maxSX) { minSX = minRX; maxSX = maxRX; minSY = minRY; maxSY = maxRY; }
                if (minRX > maxRX) { minRX = minSX; maxRX = maxSX; minRY = minSY; maxRY = maxSY; }
                if (minSX > maxSX) { minSX = 0.0f; maxSX = 8000.0f; minSY = 0.0f; maxSY = 6000.0f; minRX = 0.0f; maxRX = 8000.0f; minRY = 0.0f; maxRY = 6000.0f; }

                float x0 = (minRX + minSX) / 2.0f;
                float y0 = (minRY + minSY) / 2.0f;
                float xn = (maxRX + maxSX) / 2.0f;
                float yn = (maxRY + maxSY) / 2.0f;

                float gx = (pSPSParam->mRPtInv > 0.0f) ? (pSPSParam->mRPtInv / 2.0f) : 25.0f;
                float gy = (pSPSParam->mSPtInv > 0.0f) ? (pSPSParam->mSPtInv / 2.0f) : 25.0f;
                if (gx <= 0.1f) gx = 25.0f;
                if (gy <= 0.1f) gy = 25.0f;

                int numX = (int)((xn - x0) / gx) + 10;
                int numY = (int)((yn - y0) / gy) + 10;
                if (numX < 20) numX = 80;
                if (numY < 20) numY = 60;
                if (numX > 600) numX = 600;
                if (numY > 600) numY = 600;

                pCellGrid->mDiffX = -gx / 2.0f;
                pCellGrid->mDiffY = -gy / 2.0f;
                pCellGrid->create(pSPSParam, x0 + pCellGrid->mDiffX, y0 + pCellGrid->mDiffY, numX, numY, gx, gy);
                pCellGrid->calculateCoverCount();
                ui.actionGeometry_Attribute_Plotting->setEnabled(true);
                ui.actionOVT_Volume_Attributes->setEnabled(true);
                ui.actionOVT_Patch_Attributes->setEnabled(true);
                mpView->repaintContents();
            }
        }
    }

    if (needSampled)
    {
        if (!pProj->mIsNewImfo || (!pProj->getRLineSetNew() && !pProj->getSLineSetNew()))
        {
            pProj->mIsNewImfo = true;
        }
    }

    return true;
}

void TSPSForm::on_actionViewGallery_triggered()
{
    ResearchFiguresDialog dlg(this);
    dlg.exec();
}

void TSPSForm::on_actionViewDashboard_triggered()
{
    ResearchFiguresDialog dlg(this);
    dlg.selectFigure(0);
    dlg.exec();
}

void TSPSForm::on_actionOpenFiguresDir_triggered()
{
    QString dirPath = FigurePaths::root();
    QDir dir(dirPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}
