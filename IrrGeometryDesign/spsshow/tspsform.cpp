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
    setWindowTitle(QString::fromLocal8Bit("科研成果图件与科学结论画廊 - [IrrGeometryDesign]"));
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
    mCombo->setMinimumWidth(440);

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

    mFileNames << "00_research_workflow_summary.png";
    mTitles << QString::fromLocal8Bit("00. 全流程科研闭环综合大看板 (Workflow Summary Dashboard)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>全流程科研闭环综合大看板（观测系统设计 -> 空间相干度优化 -> CMP属性评估 -> 压缩感知POCS波场重构 -> 多指标定量对比）。<br>"
        "<b>【核心结论与图件支撑】</b>如图件所示，系统打通了从野外地震观测设计到保真恢复的完整链路。Jitter-SA 优化将 50% 采样率下的曲波互相干度由 0.9601（规则减抽）降至 0.2238（10 轮均值），相干度比值衰减 12.71 dB；同时在 50% 欠采样下 2D Curvelet POCS（线性插值热启动）重构信噪比达 13.90 dB，相关系数 0.9794，较线性插值基线（11.83 dB）提升 2.07 dB。注意：此为合成数值模拟条件下的结果，真实野外资料验证见 README 第八节。"
    );

    mFileNames << "01_sa_convergence_curve.png";
    mTitles << QString::fromLocal8Bit("01. 模拟退火相干度下降收敛曲线 (SA Coherence Minimization)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>模拟退火算法对观测感知矩阵互相干度 μ 的优化收敛轨迹（横轴：迭代步数，纵轴：相干度 μ）。<br>"
        "<b>【核心结论与图件支撑】</b>当前解轨迹（μ_trace）在 Metropolis 准则下持续探索，历史最优轨迹（best_mu_trace）严格单调非增；50% 采样率单次代表轮次历史最优 μ 由初始抖动网格值下降至该轮收敛值（图示曲线终点为 SA 历史最优，不含 CG 精修）。10 轮独立试验均值为 0.2238，平均相干度比值衰减 12.71 dB；该图仅反映 μ 轨迹，空间均匀性需另行评估。"
    );

    mFileNames << "02_sampling_benchmark_bars.png";
    mTitles << QString::fromLocal8Bit("02. 空间采样方案多指标对比柱状图 (Uniform vs Random vs Jitter-SA)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>30%、50%、70% 三种采样率下，规则减抽(Regular)、纯随机(Random)与 Jitter-SA 抖动退火优化的互相干度 μ 柱状对比。<br>"
        "<b>【核心结论与图件支撑】</b>柱状图表明：Jitter-SA 在各采样率下均取得最低相干度（50% 下 10 轮均值 0.2238，规则减抽 0.9601、纯随机 0.3332），改善率 76.69% 与 32.33%，相干度比值衰减 12.71 dB。该图仅反映 μ 差异；空间均匀性需用最大间隔/最近邻距离另行评估，不能由本图直接断言。"
    );

    mFileNames << "03_cmp_fold_distribution_map.png";
    mTitles << QString::fromLocal8Bit("03. CMP面元覆盖次数空间分布热力图 (CMP Fold Heatmap)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>真实工区 SPS 测线 CMP 面元网格空间覆盖次数热力分布（颜色越暖代表覆盖次数越高）。<br>"
        "<b>【核心结论与图件支撑】</b>热力图显示真实测线有效映射 29,719 个 CMP 面元，平均覆盖次数 106.4 次，覆盖次数范围 1~198 次，变异系数 Cv = 0.5508。面元网格空间分布平稳连续，有效消除了空间几何退化与假阳性，各反射点照明充分。"
    );

    mFileNames << "04_offset_azimuth_rose_charts.png";
    mTitles << QString::fromLocal8Bit("04. 炮检距分布直方图与方位角玫瑰图 (Offset & Azimuth Rose)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>真实观测系统有效道的炮检距统计直方图与 360° 空间方位角玫瑰分布图。<br>"
        "<b>【核心结论与图件支撑】</b>左侧直方图显示炮检距在 1.7 ~ 9324.0 m 全范围内合理过渡，均值 3875.9 m，近/中/远炮检距占比分别为 45.3% / 34.5% / 20.3%；右侧玫瑰图展示 24 个扇区方位角覆盖完整度达 100%，具备理想的全方位观测特征，为后续各向异性分析提供高品质空间支持。"
    );

    mFileNames << "05_wavefield_reconstruction_quad.png";
    mTitles << QString::fromLocal8Bit("05. 2D波场欠采样与POCS压缩感知重构四联剖面 (CS Reconstruction)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>(a) 原始完整波场、(b) 50% 欠采样波场、(c) 2D Curvelet POCS 压缩感知重构波场、(d) 残差剖面的四联对比。<br>"
        "<b>【核心结论与图件支撑】</b>剖面对比显示：在缺失 50% 道的情况下，2D Curvelet POCS 重构剖面恢复了反射同相轴的连续性。全幅 SNR 为 13.90 dB（缺失道口径 10.90 dB），高于空间线性插值基线（全幅 11.83 dB，+2.07 dB），RMSE 为 0.032070。以上为合成数值模拟条件；真实野外资料上该增益并不成立（-0.62 dB），详见 README 第八节。"
    );

    mFileNames << "06_recon_convergence_trace_comparison.png";
    mTitles << QString::fromLocal8Bit("06. POCS残差收敛曲线与抽稀道单道波形对比细节 (Trace Comparison)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>左图：POCS迭代残差与PSNR提升收敛曲线；右图：抽取单道（原始道 vs 缺失插值道 vs 重构道）的振幅与子波重叠对比。<br>"
        "<b>【核心结论与图件支撑】</b>左图显示 POCS 残差随迭代下降并趋于平稳；右图为抽取单道（原始道 vs 重构道）的振幅与子波重叠对比。单道指标与迭代收敛数值请以 recon_metrics_summary.csv 及 figures/ 中对应图件为准，此处不再硬编码未计算的数字。"
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
    QString htmlPath = QDir::current().filePath("figures/index.html");
    QFileInfo fi(htmlPath);
    if (fi.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
    else
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("未找到画廊网页"),
                             QString::fromLocal8Bit("画廊网页尚未生成：%1\n请先在命令行运行：\nIrrGeometryDesign.exe --run-experiments results\\synthetic --repeat 10 --seed 42").arg(htmlPath));
    }
}

void ResearchFiguresDialog::onOpenFolder()
{
    QString dirPath = QDir::current().filePath("figures");
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

    QString dirPath = QDir::current().filePath("figures");
    QString filePath = QDir(dirPath).filePath(mFileNames[index]);
    QFileInfo fi(filePath);
    if (!fi.exists())
    {
        filePath = QDir::current().filePath("figures/" + mFileNames[index]);
        fi.setFile(filePath);
    }

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
            "提示：本版本 GUI 不再提供一键运行入口。请先在命令行运行：<br>"
            "IrrGeometryDesign.exe --run-experiments results\\synthetic --repeat 10 --seed 42").arg(mFileNames[index]));
    }

    mConclusionBrowser->setHtml(mConclusions[index]);
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
	/************************************************************************/
	/* 采样前的检波器排列也要进行属性计算和分析，用于与优化后排列进行对比 */
	/* 检波点采样及显示 */
	/************************************************************************/
	if (!ensureProjectLoaded(false, false)) return;
	GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
	if (!pProject) return;
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if (!pSPSParam) return;
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
			
			QString info_str = QString::fromLocal8Bit(
				"检波线空间抽稀完成：\n"
				"● 抽稀后互相干度 (μ): %1\n\n"
				"【后续优化建议】：\n"
				"建议继续点击【退火优化】按钮，采用模拟退火算法优化扰动非规则排列，破坏周期性假频。").arg(miu_xj_before);
			QMessageBox::information(this, QString::fromLocal8Bit("检波线抽稀计算完成"), info_str);// 请确认哪些内容需要输出！！！ [Liu Wei 6/29/2023]

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
		if (mpProject->getCellGrid()) mpProject->getCellGrid()->calculateCoverCount();
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
	if (!ensureProjectLoaded(false, false)) return;
	GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
	if (!pProject) return;
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if (!pSPSParam) return;
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
			
			QString info_str = QString::fromLocal8Bit("互相干度参数 (μ): ") + QString::number(miu_xj_before, 'f', 4);
			QMessageBox::information(this, QString::fromLocal8Bit("互相干度计算结果"), info_str);

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
		mIsFill = true;
		mIsLiner = true;
		mpView->mIsImpotSps = false;
		mpProject->mIsNewImfo = true;
		if (mpProject->getCellGrid()) mpProject->getCellGrid()->calculateCoverCount();
		refresh();
		mpView->repaintContents();
	}
	delete sample_dlg;
}

void TSPSForm::on_actionRAOptimizing_triggered()
{
    //************** 检波点优化 *************//
	if (!ensureProjectLoaded(false, false)) return;
	GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
	if (!pProject) return;
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if (!pSPSParam) return;
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
			if (R_Sampling_Arrays.size() < row || RL_Sampling.size() < row)
			{
				R_Sampling_Arrays.clear();
				RL_Sampling.clear();
				for (int i = 0; i < row; i++)
				{
					QVector<int> sv;
					for (int j = 0; j < N; j++)
					{
						sv.append(1);
					}
					R_Sampling_Arrays.append(sv);
					RL_Sampling.append(1);
				}
			}
			for (int i=0; i<row; i++)
			{
				for (int j=0; j<N; j++)
				{
					Phi0[i][j]=(i < R_Sampling_Arrays.size() && j < R_Sampling_Arrays[i].size()) ? R_Sampling_Arrays[i][j] : 1;
				}
				phi0[i]=(i < RL_Sampling.size()) ? RL_Sampling[i] : 1;
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

			QString miu_str = QString::fromLocal8Bit("检波点排列模拟退火优化完成：\n") +
				QString::fromLocal8Bit("● 初始互相干度: ") + QString::number(miu0, 'f', 4) + QString::fromLocal8Bit("\n") +
				QString::fromLocal8Bit("● 优化后互相干度 (μ): ") + QString::number(miu_after, 'f', 4) + QString::fromLocal8Bit("\n") +
				QString::fromLocal8Bit("● 优化耗时: ") + QString::number(duration, 'f', 2) + QString::fromLocal8Bit(" 秒");
			QMessageBox::information(this, QString::fromLocal8Bit("排列退火优化计算完成"), miu_str);

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
		if (mpProject->getCellGrid()) mpProject->getCellGrid()->calculateCoverCount();
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
	if (!ensureProjectLoaded(false, false)) return;
	GProject *pProject = mpProject ? mpProject : GProject::getCurrentProject();
	if (!pProject) return;
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if (!pSPSParam) return;
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
			if (S_Sampling_Arrays.size() < row || SL_Sampling.size() < row)
			{
				S_Sampling_Arrays.clear();
				SL_Sampling.clear();
				for (int i = 0; i < row; i++)
				{
					QVector<int> sv;
					for (int j = 0; j < N; j++)
					{
						sv.append(1);
					}
					S_Sampling_Arrays.append(sv);
					SL_Sampling.append(1);
				}
			}
			for (int i=0; i<row; i++)
			{
				for (int j=0; j<N; j++)
				{
					Phi0[i][j]=(i < S_Sampling_Arrays.size() && j < S_Sampling_Arrays[i].size()) ? S_Sampling_Arrays[i][j] : 1;
				}
				phi0[i]=(i < SL_Sampling.size()) ? SL_Sampling[i] : 1;
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

			QString miu_str = QString::fromLocal8Bit("检波点排列模拟退火优化完成：\n") +
				QString::fromLocal8Bit("● 初始互相干度: ") + QString::number(miu0, 'f', 4) + QString::fromLocal8Bit("\n") +
				QString::fromLocal8Bit("● 优化后互相干度 (μ): ") + QString::number(miu_after, 'f', 4) + QString::fromLocal8Bit("\n") +
				QString::fromLocal8Bit("● 优化耗时: ") + QString::number(duration, 'f', 2) + QString::fromLocal8Bit(" 秒");
			QMessageBox::information(this, QString::fromLocal8Bit("排列退火优化计算完成"), miu_str);

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

		// 完成后显示观测系统
		mIsFill = true;
		mIsLiner = true;
		mpView->mIsImpotSps = false;
		mpProject->mIsNewImfo = true;
		if (mpProject->getCellGrid()) mpProject->getCellGrid()->calculateCoverCount();
		refresh();
		mpView->repaintContents();
	}
	delete sample_dlg;
	qDebug()<<"hello world!";// 测试 [Liu Wei 6/30/2023]	
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
		QMessageBox::warning(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("当前工程中没有可用的炮线或检波线数据，无法计算曲波相干度。"));
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
	QString info_str = QString::fromLocal8Bit("当前观测系统曲波域互相关度 (Incoherence Metric μ) 评测结果：\n\n") +
		QString::fromLocal8Bit("● 检波排列相干度 (μ_r): ") + QString::number(miu_r, 'f', 4) + QString::fromLocal8Bit("\n") +
		QString::fromLocal8Bit("● 震源排列相干度 (μ_s): ") + QString::number(miu_s, 'f', 4) + QString::fromLocal8Bit("\n") +
		QString::fromLocal8Bit("● 综合平均相干度 (μ_avg): ") + QString::number(miu_avg, 'f', 4) + QString::fromLocal8Bit("\n\n") +
		QString::fromLocal8Bit("统计基础：检波线 ") + QString::number(mRLineSet_x.size()) + QString::fromLocal8Bit(" 条，最大道数 ") + QString::number(Rlm) +
		QString::fromLocal8Bit("；激发线 ") + QString::number(mSLineSet_y.size()) + QString::fromLocal8Bit(" 条，最大炮数 ") + QString::number(Slm) + QString::fromLocal8Bit("。\n") +
		QString::fromLocal8Bit("注：相干度越低，说明观测系统在Curvelet变换域的能量聚集与非规则抗混叠恢复能力越强。");
	QMessageBox::information(this, QString::fromLocal8Bit("相干度计算完成"), info_str);
}

// 试验与评估模块前置状态校验与联动更新 // add by shanzhou in 2026/10/15
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
    QString dirPath = QDir::current().filePath("figures");
    QDir dir(dirPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}
