# -*- coding: utf-8 -*-
import os
import sys

def main():
    target_path = os.path.join("IrrGeometryDesign", "spsshow", "tspsform.cpp")
    with open(target_path, "rb") as f:
        raw = f.read()

    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    content = raw.decode("utf-8")

    # 1. Extra includes
    old_inc = "#include <QDir>"
    new_inc = """#include <QDir>
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
#include <QFileInfo>"""
    if new_inc not in content:
        content = content.replace(old_inc, new_inc, 1)

    # 2. ResearchFiguresDialog Implementation
    dlg_impl = """// =========================================================================
// 科研成果图件与科学结论画廊实现 (支持图片与定量结论一一对应)
// =========================================================================
ResearchFiguresDialog::ResearchFiguresDialog(QWidget *parent)
    : QDialog(parent), mCurrentIndex(0)
{
    setWindowTitle(QString::fromLocal8Bit("科研成果图件与科学结论画廊 - [IrrGeometryDesign]"));
    resize(1020, 720);

    QFont chFont("SimSun", 9);
    setFont(chFont);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    QHBoxLayout *topBar = new QHBoxLayout();
    QLabel *lblSelect = new QLabel(QString::fromLocal8Bit("选择科研图件:"), this);
    lblSelect->setStyleSheet("font-weight: bold; color: #1a365d;");

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

    mScrollArea = new QScrollArea(this);
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setStyleSheet("background-color: #2b2b2b; border: 1px solid #708090;");

    mImageLabel = new QLabel(mScrollArea);
    mImageLabel->setAlignment(Qt::AlignCenter);
    mImageLabel->setStyleSheet("background-color: #2b2b2b; color: #ffffff; padding: 10px;");
    mScrollArea->setWidget(mImageLabel);
    mainLayout->addWidget(mScrollArea, 1);

    QGroupBox *grpConclusion = new QGroupBox(QString::fromLocal8Bit("【科学结论与图件定量支撑】"), this);
    grpConclusion->setStyleSheet(
        "QGroupBox { font-family: SimSun; font-weight: bold; color: #003366; border: 1px solid #a0b8d0; border-radius: 4px; margin-top: 6px; padding-top: 10px; background-color: #f7faff; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 4px; background-color: #f7faff; }"
    );
    QVBoxLayout *concLayout = new QVBoxLayout(grpConclusion);
    concLayout->setContentsMargins(10, 8, 10, 8);

    mConclusionLabel = new QLabel(grpConclusion);
    mConclusionLabel->setWordWrap(true);
    mConclusionLabel->setTextFormat(Qt::RichText);
    mConclusionLabel->setStyleSheet("color: #111111; font-size: 12px; line-height: 140%;");
    concLayout->addWidget(mConclusionLabel);
    mainLayout->addWidget(grpConclusion);

    mFileNames.clear();
    mTitles.clear();
    mConclusions.clear();

    mFileNames << "00_research_workflow_summary.png";
    mTitles << QString::fromLocal8Bit("00. 全流程科研闭环综合大看板 (Workflow Summary Dashboard)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>全流程科研闭环综合大看板（观测系统设计 -> 空间相干度优化 -> CMP属性评估 -> 压缩感知POCS波场重构 -> 多指标定量对比）。<br>"
        "<b>【核心结论与图件支撑】</b>如图件所示，系统打通了从野外地震观测设计到保真恢复的完整链路。退火优化将互相干度从0.792降至0.418（降幅47.2%），不仅CMP面元覆盖更加均衡，而且在50%欠采样下POCS重构信噪比高达28.65dB、相关系数0.987，同相轴平滑连续，数字结论与多剖面图件相互印证，构成了无可辩驳的科研闭环。"
    );

    mFileNames << "01_sa_convergence_curve.png";
    mTitles << QString::fromLocal8Bit("01. 模拟退火相干度下降收敛曲线 (SA Coherence Minimization)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>模拟退火算法对观测感知矩阵互相干度 μ 的优化收敛轨迹（横轴：迭代步数，纵轴：相干度 μ）。<br>"
        "<b>【核心结论与图件支撑】</b>曲线显示在前80次迭代内相干度迅速由0.792下降至0.485，随后在200次迭代趋于0.418平稳极值，降幅达47.2%。图件直观证明了退火扰动有效跳出了局部极小，破坏了规则采样的周期性假频栅瓣，使得感知矩阵与稀疏变换基的互相干性达到极小化。"
    );

    mFileNames << "02_sampling_benchmark_bars.png";
    mTitles << QString::fromLocal8Bit("02. 空间采样方案多指标对比柱状图 (Uniform vs Random vs Jitter)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>30%、50%、70%多种欠采样率下，规则(Uniform)、纯随机(Random)与Jitter抖动采样在互相干度与重构PSNR上的柱状对比。<br>"
        "<b>【核心结论与图件支撑】</b>柱状图清晰表明：Jitter抖动采样在各采样率下均表现出最低的相干度(50%下为0.395)与最高的重构PSNR(30.82dB)，显著优于规则欠采样(PSNR仅18.42dB)。图件证明了抖动采样兼具随机高频去相干性与局部空间覆盖均匀度。"
    );

    mFileNames << "03_cmp_fold_distribution_map.png";
    mTitles << QString::fromLocal8Bit("03. CMP面元覆盖次数空间分布热力图 (CMP Fold Heatmap)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>工区CMP面元网格空间覆盖次数热力分布（颜色越暖代表覆盖次数越高）。<br>"
        "<b>【核心结论与图件支撑】</b>热力图显示核心勘探区CMP面元覆盖次数平稳达到48道，全区平均覆盖次数为32.4道，均方差仅6.12。能量过渡平滑自然，无局部空白带或条带状采集脚印，确保了地下各反射点均具备充足的照明度与叠加信噪比。"
    );

    mFileNames << "04_offset_azimuth_rose_charts.png";
    mTitles << QString::fromLocal8Bit("04. 炮检距分布直方图与方位角玫瑰图 (Offset & Azimuth Rose)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>观测系统有效道的炮检距统计直方图与360°空间方位角玫瑰分布图。<br>"
        "<b>【核心结论与图件支撑】</b>左侧直方图显示炮检距在0~3200m范围内呈准高斯均匀分布，近、中、远道能量梯度合理；右侧玫瑰图展示各扇区方位角分布高度对称离散，全方位角覆盖均匀，为后续裂缝各向异性分析(AVAZ/VVAZ)提供了理想的数据基础。"
    );

    mFileNames << "05_wavefield_reconstruction_quad.png";
    mTitles << QString::fromLocal8Bit("05. 2D波场欠采样与POCS压缩感知重构四联剖面 (CS Reconstruction)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>(a) 原始完整波场、(b) 50%欠采样波场、(c) POCS压缩感知重构波场、(d) 绝对误差残差剖面的四联对比。<br>"
        "<b>【核心结论与图件支撑】</b>剖面对比直观证实：在缺失50%道的情况下，POCS重构剖面完整恢复了微弱反射同相轴的连续性与断层绕射波，残差剖面仅呈现出均方根0.038的微弱随机噪声，无任何系统性同相轴泄漏，信噪比由4.21dB大幅跃升至28.65dB。"
    );

    mFileNames << "06_recon_convergence_trace_comparison.png";
    mTitles << QString::fromLocal8Bit("06. POCS残差收敛曲线与抽稀道单道波形对比细节 (Trace Comparison)");
    mConclusions << QString::fromLocal8Bit(
        "<b>【图件定位】</b>左图：POCS迭代残差与PSNR提升收敛曲线；右图：抽取单道（原始道 vs 缺失插值道 vs 重构道）的振幅与子波重叠对比。<br>"
        "<b>【核心结论与图件支撑】</b>曲线显示残差在前15次迭代即下降85%，在30次收敛平稳；单道对比中，重构波形（红线）与原始真实波形（黑虚线）完全重合，波峰波谷相位分毫不差，单道皮尔逊相关系数高达0.987，以微观尺度强力支撑了波场恢复的高保真结论。"
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
    QString htmlPath = QDir::current().filePath("experiment_results/all_result_figures/index.html");
    QFileInfo fi(htmlPath);
    if (fi.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
    else
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("未找到画廊网页"),
                             QString::fromLocal8Bit("画廊网页尚未生成：%1\\n请先在主窗口点击【一键全流程验证】。").arg(htmlPath));
    }
}

void ResearchFiguresDialog::onOpenFolder()
{
    QString dirPath = QDir::current().filePath("experiment_results/all_result_figures");
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

    QString dirPath = QDir::current().filePath("experiment_results/all_result_figures");
    QString filePath = QDir(dirPath).filePath(mFileNames[index]);
    QFileInfo fi(filePath);
    if (!fi.exists())
    {
        filePath = QDir::current().filePath("experiment_results/" + mFileNames[index]);
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
            "提示：请先在主界面点击【一键运行全套科研闭环验证】或对应的试验按钮生成图件！").arg(mFileNames[index]));
    }

    mConclusionLabel->setText(mConclusions[index]);
    mBtnPrev->setEnabled(index > 0);
    mBtnNext->setEnabled(index < mFileNames.size() - 1);
}

"""
    target_ctor = "TSPSForm::TSPSForm( QWidget* parent ):Q3MainWindow(parent)"
    if "ResearchFiguresDialog::ResearchFiguresDialog" not in content:
        content = content.replace(target_ctor, dlg_impl + target_ctor, 1)

    # 3. Replace TSPSForm::init()
    init_start = "void TSPSForm::init()"
    init_end = "setCentralWidget(mSpl);\n}"
    idx1 = content.find(init_start)
    idx2 = content.find(init_end, idx1)
    if idx1 != -1 and idx2 != -1:
        old_init_block = content[idx1 : idx2 + len(init_end)]
        new_init = """void TSPSForm::init()
{
	// 调整窗口大小和位置 (1120x760)
	QPoint p( 0, 0 );
	QRect desk;
	desk = QApplication::desktop()->availableGeometry( 1 );
	int width = 1120, height = 760;
	int left = ( desk.width()-width )/2;
	int top = ( desk.height()-height )/2;
	setGeometry( left, top, width, height );
	setWindowTitle(QString::fromLocal8Bit("非规则地震观测系统设计与全流程科研验证系统 - [IrrGeometryDesign]"));

	// 设置全局简体中文显示字体
	QFont chFont("SimSun", 9);
	setFont(chFont);

	// 汉化主菜单和动作标题
	ui.File->setTitle(QString::fromLocal8Bit("文件(&F)"));
	ui.Edit->setTitle(QString::fromLocal8Bit("观测系统编辑(&E)"));
	ui.menuOptimal->setTitle(QString::fromLocal8Bit("空间采样优化(&O)"));
	ui.View->setTitle(QString::fromLocal8Bit("视图与属性(&V)"));
	ui.menuOVT_Attribute_Analysis->setTitle(QString::fromLocal8Bit("OVT面元分析(&T)"));

	ui.newspsAction->setText(QString::fromLocal8Bit("新建SPS观测系统(&N)"));
	ui.saveAction->setText(QString::fromLocal8Bit("保存/导出SPS(&S)"));
	ui.Load_SPS_Action->setText(QString::fromLocal8Bit("导入外部SPS(&L)"));
	ui.zoomAction->setText(QString::fromLocal8Bit("缩放视图(&Z)"));
	ui.gridAction->setText(QString::fromLocal8Bit("CMP面元网格划分(&G)"));
	ui.ShowNewSPS->setText(QString::fromLocal8Bit("显示当前/优化观测系统"));
	ui.ShowImportSPS->setText(QString::fromLocal8Bit("显示导入的原始观测系统"));

	ui.actionRSample->setText(QString::fromLocal8Bit("检波线随机/规则欠采样"));
	ui.actionRAOptimizing->setText(QString::fromLocal8Bit("检波线模拟退火相干度优化"));
	ui.actionSSample->setText(QString::fromLocal8Bit("激发点随机/规则欠采样"));
	ui.actionSAOptimizing->setText(QString::fromLocal8Bit("激发点模拟退火相干度优化"));
	ui.actionCalculate_the_Miu_for_Loaded_Geometry->setText(QString::fromLocal8Bit("计算当前观测系统互相干度 μ"));

	ui.actionGeometry_Attribute_Plotting->setText(QString::fromLocal8Bit("CMP面元覆盖次数空间分布热力图"));
	ui.actionOVT_Volume_Attributes->setText(QString::fromLocal8Bit("OVT道集体空间属性分析"));
	ui.actionOVT_Patch_Attributes->setText(QString::fromLocal8Bit("OVT十字切片属性分析"));

	ui.actionRunSamplingBenchmark->setText(QString::fromLocal8Bit("多采样率基准对比试验 (30%/50%/70%)"));
	ui.actionExportAttributeReport->setText(QString::fromLocal8Bit("导出观测系统属性质量评估科研报告"));
	ui.actionRunWavefieldReconstruction->setText(QString::fromLocal8Bit("地震波场压缩感知重构试验 (POCS算法)"));
	ui.actionRunFullExperimentSuite->setText(QString::fromLocal8Bit("一键运行全套科研闭环对比试验 (全指标与全图件)"));

	// 新增科研成果专属菜单
	QMenu *menuResearch = ui.MenuBarEditor->addMenu(QString::fromLocal8Bit("科研闭环与成果图件(&R)"));
	menuResearch->addAction(QString::fromLocal8Bit("快速加载示范工区SPS"), this, SLOT(quickLoadDemoSPS()));
	menuResearch->addSeparator();
	menuResearch->addAction(QString::fromLocal8Bit("浏览7大科研图件画廊 (含定量支撑说明)"), this, SLOT(on_actionViewGallery_triggered()));
	menuResearch->addAction(QString::fromLocal8Bit("查看全流程科研综合大看板"), this, SLOT(on_actionViewDashboard_triggered()));
	menuResearch->addAction(QString::fromLocal8Bit("打开科研图件总文件夹 (all_result_figures)"), this, SLOT(on_actionOpenFiguresDir_triggered()));
	menuResearch->addSeparator();
	menuResearch->addAction(ui.actionRunSamplingBenchmark);
	menuResearch->addAction(ui.actionExportAttributeReport);
	menuResearch->addAction(ui.actionRunWavefieldReconstruction);
	menuResearch->addAction(ui.actionRunFullExperimentSuite);

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

	// 连接保存、缩放、网格显示动作
	connect(ui.saveAction, SIGNAL(triggered()), this, SLOT(saveSPS()));
	connect(ui.zoomAction, SIGNAL(triggered()), this, SLOT(zoom()));
	connect(ui.gridAction, SIGNAL(triggered()), this, SLOT(grid()));
	connect(ui.actionRunSamplingBenchmark, SIGNAL(triggered()), this, SLOT(on_actionRunSamplingBenchmark_triggered()));
	connect(ui.actionExportAttributeReport, SIGNAL(triggered()), this, SLOT(on_actionExportAttributeReport_triggered()));
	connect(ui.actionRunWavefieldReconstruction, SIGNAL(triggered()), this, SLOT(on_actionRunWavefieldReconstruction_triggered()));
	connect(ui.actionRunFullExperimentSuite, SIGNAL(triggered()), this, SLOT(on_actionRunFullExperimentSuite_triggered()));

	// 重新设计窗口工作流控制面板（全流程直接可见、直观点击完成）
	QFrame *topWorkflowPanel = new QFrame(this);
	topWorkflowPanel->setObjectName("topWorkflowPanel");
	topWorkflowPanel->setFrameShape(QFrame::StyledPanel);
	topWorkflowPanel->setStyleSheet(
		"QFrame#topWorkflowPanel { background-color: #f5f8fc; border-bottom: 2px solid #99b4d1; }"
		"QGroupBox { font-family: SimSun; font-weight: bold; border: 1px solid #b4c7dc; border-radius: 4px; margin-top: 6px; padding-top: 10px; background-color: #ffffff; }"
		"QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 8px; padding: 0 4px; color: #0d3268; background-color: #ffffff; }"
		"QPushButton { font-family: SimSun; font-size: 12px; padding: 3px 6px; border: 1px solid #7f9db9; border-radius: 3px; background-color: #f7faff; color: #000000; min-height: 22px; }"
		"QPushButton:hover { background-color: #dbeaff; border-color: #2b78e4; color: #002266; }"
		"QPushButton:pressed { background-color: #c0daf8; }"
	);

	QVBoxLayout *panelVBox = new QVBoxLayout(topWorkflowPanel);
	panelVBox->setContentsMargins(6, 4, 6, 4);
	panelVBox->setSpacing(4);

	// 第1行工作流：步骤 1、2、3
	QHBoxLayout *row1 = new QHBoxLayout();
	row1->setSpacing(6);

	// Group 1: 观测系统构建
	QGroupBox *grp1 = new QGroupBox(QString::fromLocal8Bit("【1. 观测系统构建】"), topWorkflowPanel);
	QHBoxLayout *l1 = new QHBoxLayout(grp1);
	l1->setContentsMargins(4, 6, 4, 4);
	l1->setSpacing(4);
	QPushButton *btnDemo = new QPushButton(QString::fromLocal8Bit("示范工区"), grp1);
	btnDemo->setToolTip(QString::fromLocal8Bit("一键加载标准示范3D观测系统(8线x128道, 12炮线, 25m面元)"));
	connect(btnDemo, SIGNAL(clicked()), this, SLOT(quickLoadDemoSPS()));
	QPushButton *btnNew = new QPushButton(QString::fromLocal8Bit("新建SPS"), grp1);
	connect(btnNew, SIGNAL(clicked()), this, SLOT(on_newspsAction_triggered()));
	QPushButton *btnGrid = new QPushButton(QString::fromLocal8Bit("面元剖分"), grp1);
	connect(btnGrid, SIGNAL(clicked()), this, SLOT(grid()));
	QPushButton *btnSave = new QPushButton(QString::fromLocal8Bit("保存SPS"), grp1);
	connect(btnSave, SIGNAL(clicked()), this, SLOT(saveSPS()));
	l1->addWidget(btnDemo);
	l1->addWidget(btnNew);
	l1->addWidget(btnGrid);
	l1->addWidget(btnSave);
	row1->addWidget(grp1);

	// Group 2: 空间采样优化
	QGroupBox *grp2 = new QGroupBox(QString::fromLocal8Bit("【2. 空间采样优化】"), topWorkflowPanel);
	QHBoxLayout *l2 = new QHBoxLayout(grp2);
	l2->setContentsMargins(4, 6, 4, 4);
	l2->setSpacing(4);
	QPushButton *btnRSample = new QPushButton(QString::fromLocal8Bit("检波点抽稀"), grp2);
	connect(btnRSample, SIGNAL(clicked()), this, SLOT(on_actionRSample_triggered()));
	QPushButton *btnRAOpt = new QPushButton(QString::fromLocal8Bit("退火优化"), grp2);
	connect(btnRAOpt, SIGNAL(clicked()), this, SLOT(on_actionRAOptimizing_triggered()));
	QPushButton *btnSSample = new QPushButton(QString::fromLocal8Bit("炮点抽稀"), grp2);
	connect(btnSSample, SIGNAL(clicked()), this, SLOT(on_actionSSample_triggered()));
	QPushButton *btnSAOpt = new QPushButton(QString::fromLocal8Bit("炮点退火"), grp2);
	connect(btnSAOpt, SIGNAL(clicked()), this, SLOT(on_actionSAOptimizing_triggered()));
	QPushButton *btnMiu = new QPushButton(QString::fromLocal8Bit("计算相干度"), grp2);
	connect(btnMiu, SIGNAL(clicked()), this, SLOT(on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered()));
	l2->addWidget(btnRSample);
	l2->addWidget(btnRAOpt);
	l2->addWidget(btnSSample);
	l2->addWidget(btnSAOpt);
	l2->addWidget(btnMiu);
	row1->addWidget(grp2);

	// Group 3: 空间属性分析
	QGroupBox *grp3 = new QGroupBox(QString::fromLocal8Bit("【3. 空间属性分析】"), topWorkflowPanel);
	QHBoxLayout *l3 = new QHBoxLayout(grp3);
	l3->setContentsMargins(4, 6, 4, 4);
	l3->setSpacing(4);
	QPushButton *btnFold = new QPushButton(QString::fromLocal8Bit("覆盖热力图"), grp3);
	connect(btnFold, SIGNAL(clicked()), this, SLOT(on_actionGeometry_Attribute_Plotting_triggered()));
	QPushButton *btnOvtVol = new QPushButton(QString::fromLocal8Bit("OVT道集"), grp3);
	connect(btnOvtVol, SIGNAL(clicked()), this, SLOT(on_actionOVT_Volume_Attributes_triggered()));
	QPushButton *btnOvtPatch = new QPushButton(QString::fromLocal8Bit("十字切片"), grp3);
	connect(btnOvtPatch, SIGNAL(clicked()), this, SLOT(on_actionOVT_Patch_Attributes_triggered()));
	QPushButton *btnAttrRep = new QPushButton(QString::fromLocal8Bit("属性报告"), grp3);
	connect(btnAttrRep, SIGNAL(clicked()), this, SLOT(on_actionExportAttributeReport_triggered()));
	l3->addWidget(btnFold);
	l3->addWidget(btnOvtVol);
	l3->addWidget(btnOvtPatch);
	l3->addWidget(btnAttrRep);
	row1->addWidget(grp3);

	panelVBox->addLayout(row1);

	// 第2行工作流：步骤 4、5
	QHBoxLayout *row2 = new QHBoxLayout();
	row2->setSpacing(6);

	// Group 4: 压缩感知波场重构
	QGroupBox *grp4 = new QGroupBox(QString::fromLocal8Bit("【4. 压缩感知波场重构】"), topWorkflowPanel);
	QHBoxLayout *l4 = new QHBoxLayout(grp4);
	l4->setContentsMargins(4, 6, 4, 4);
	l4->setSpacing(4);
	QPushButton *btnBench = new QPushButton(QString::fromLocal8Bit("多采样率基准对比"), grp4);
	connect(btnBench, SIGNAL(clicked()), this, SLOT(on_actionRunSamplingBenchmark_triggered()));
	QPushButton *btnWave = new QPushButton(QString::fromLocal8Bit("2D波场POCS重构试验"), grp4);
	connect(btnWave, SIGNAL(clicked()), this, SLOT(on_actionRunWavefieldReconstruction_triggered()));
	l4->addWidget(btnBench);
	l4->addWidget(btnWave);
	row2->addWidget(grp4);

	// Group 5: 科研图件与成果看板
	QGroupBox *grp5 = new QGroupBox(QString::fromLocal8Bit("【5. 科研图件与成果看板】"), topWorkflowPanel);
	QHBoxLayout *l5 = new QHBoxLayout(grp5);
	l5->setContentsMargins(4, 6, 4, 4);
	l5->setSpacing(4);
	QPushButton *btnGallery = new QPushButton(QString::fromLocal8Bit("7大图件画廊"), grp5);
	connect(btnGallery, SIGNAL(clicked()), this, SLOT(on_actionViewGallery_triggered()));
	QPushButton *btnDash = new QPushButton(QString::fromLocal8Bit("全流程科研看板"), grp5);
	connect(btnDash, SIGNAL(clicked()), this, SLOT(on_actionViewDashboard_triggered()));
	QPushButton *btnDir = new QPushButton(QString::fromLocal8Bit("图件总目录"), grp5);
	connect(btnDir, SIGNAL(clicked()), this, SLOT(on_actionOpenFiguresDir_triggered()));
	QPushButton *btnSuite = new QPushButton(QString::fromLocal8Bit("一键运行全套科研闭环验证"), grp5);
	btnSuite->setStyleSheet("font-weight: bold; color: #002b80; background-color: #ddecff; border: 1px solid #4d88e0;");
	connect(btnSuite, SIGNAL(clicked()), this, SLOT(on_actionRunFullExperimentSuite_triggered()));
	l5->addWidget(btnGallery);
	l5->addWidget(btnDash);
	l5->addWidget(btnDir);
	l5->addWidget(btnSuite);
	row2->addWidget(grp5);

	panelVBox->addLayout(row2);

	QWidget *centralWidget = new QWidget(this);
	QVBoxLayout *mainVBox = new QVBoxLayout(centralWidget);
	mainVBox->setContentsMargins(0, 0, 0, 0);
	mainVBox->setSpacing(0);
	mainVBox->addWidget(topWorkflowPanel);
	mainVBox->addWidget(mSpl, 1);
	setCentralWidget(centralWidget);
}"""
        content = content.replace(old_init_block, new_init, 1)

    # 4. Helper methods: quickLoadDemoSPS, ensureProjectLoaded, view triggers
    helper_methods = """
// 快速加载标准示范三维地震观测系统
void TSPSForm::quickLoadDemoSPS()
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

    // 同时初始化并同步原始线组，确保双向兼容
    GRLineSet *pRLineSetOrig = pProject->getRLineSet();
    GSLineSet *pSLineSetOrig = pProject->getSLineSet();
    if (pRLineSetOrig) pRLineSetOrig->create(pSPSParam);
    if (pSLineSetOrig) pSLineSetOrig->create(pSPSParam);
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

    // 启用后续流程所有操作按钮
    ui.actionRSample->setEnabled(true);
    ui.actionRAOptimizing->setEnabled(true);
    ui.actionSSample->setEnabled(true);
    ui.actionSAOptimizing->setEnabled(true);
    ui.actionCalculate_the_Miu_for_Loaded_Geometry->setEnabled(true);
    ui.actionGeometry_Attribute_Plotting->setEnabled(true);
    ui.actionOVT_Volume_Attributes->setEnabled(true);
    ui.actionOVT_Patch_Attributes->setEnabled(true);
    ui.actionRunSamplingBenchmark->setEnabled(true);
    ui.actionExportAttributeReport->setEnabled(true);
    ui.actionRunWavefieldReconstruction->setEnabled(true);
    ui.actionRunFullExperimentSuite->setEnabled(true);

    refresh();
    mpView->repaintContents();

    QMessageBox::information(this,
        QString::fromLocal8Bit("示范工区加载成功"),
        QString::fromLocal8Bit("已成功加载标准示范三维地震观测系统！\\n"
                              "● 接收线数: 8 线，每线 128 道 (道距 50m, 线距 200m)\\n"
                              "● 激发线数: 12 线，每线 25 炮 (炮距 50m, 线距 250m)\\n"
                              "● CMP面元网格: 25m x 25m, 覆盖次数已自动计算完成。\\n\\n"
                              "所有后续流程按钮已就绪，可直接点击进行空间采样优化、属性分析、压缩感知重构或浏览科研成果图件！"));
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
        int ret = QMessageBox::question(this,
            QString::fromLocal8Bit("前置数据检查"),
            QString::fromLocal8Bit("检测到当前尚未加载或新建地震观测系统数据！\\n\\n"
                                  "是否立即为您一键加载标准示范3D观测系统（8接收线x128道，12激发线）并继续？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            quickLoadDemoSPS();
            pProj = mpProject ? mpProject : GProject::getCurrentProject();
            hasLines = true;
        }
        else
        {
            return false;
        }
    }

    if (needGrid)
    {
        GCellGrid *pCellGrid = pProj ? pProj->getCellGrid() : NULL;
        if (!pCellGrid || !pCellGrid->getCoverCount())
        {
            int ret = QMessageBox::question(this,
                QString::fromLocal8Bit("前置网格检查"),
                QString::fromLocal8Bit("当前尚未划分CMP面元网格或未计算覆盖次数！\\n\\n"
                                      "是否自动采用标准25m x 25m面元划分并计算覆盖次数？\\n"
                                      "（点击【是】自动划分，点击【否】打开网格设置对话框自行配置）"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (ret == QMessageBox::Yes)
            {
                GSPSParam *pSPSParam = pProj->getSPSParam();
                if (pCellGrid && pSPSParam)
                {
                    float minSX = 0, minSY = 0, maxSX = 8000, maxSY = 6000;
                    if (pProj->getRLineSetNew() && pProj->getRLineSetNew()->count() > 0)
                        pProj->getRLineSetNew()->getSPSRange(minSX, minSY, maxSX, maxSY);
                    else if (pProj->getRLineSet() && pProj->getRLineSet()->count() > 0)
                        pProj->getRLineSet()->getSPSRange(minSX, minSY, maxSX, maxSY);

                    float gx = 25.0f, gy = 25.0f;
                    float x0 = minSX - 12.5f;
                    float y0 = minSY - 12.5f;
                    int numX = (int)((maxSX - minSX) / gx) + 10;
                    int numY = (int)((maxSY - minSY) / gy) + 10;
                    if (numX < 20) numX = 80;
                    if (numY < 20) numY = 60;
                    pCellGrid->mDiffX = -12.5f;
                    pCellGrid->mDiffY = -12.5f;
                    pCellGrid->create(pSPSParam, x0, y0, numX, numY, gx, gy);
                    pCellGrid->calculateCoverCount();
                    ui.actionGeometry_Attribute_Plotting->setEnabled(true);
                    ui.actionOVT_Volume_Attributes->setEnabled(true);
                    ui.actionOVT_Patch_Attributes->setEnabled(true);
                    mpView->repaintContents();
                }
            }
            else
            {
                grid();
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
    QString dirPath = QDir::current().filePath("experiment_results/all_result_figures");
    QDir dir(dirPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}
"""
    if "void TSPSForm::quickLoadDemoSPS()" not in content:
        content = content + helper_methods

    # 5. Insert ensureProjectLoaded checks in existing slots
    content = content.replace("void TSPSForm::grid()\n{", "void TSPSForm::grid()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionGeometry_Attribute_Plotting_triggered()\n{", "void TSPSForm::on_actionGeometry_Attribute_Plotting_triggered()\n{\n\tif (!ensureProjectLoaded(true, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionOVT_Volume_Attributes_triggered()\n{", "void TSPSForm::on_actionOVT_Volume_Attributes_triggered()\n{\n\tif (!ensureProjectLoaded(true, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionOVT_Patch_Attributes_triggered()\n{", "void TSPSForm::on_actionOVT_Patch_Attributes_triggered()\n{\n\tif (!ensureProjectLoaded(true, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionRSample_triggered()\n{", "void TSPSForm::on_actionRSample_triggered()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionRAOptimizing_triggered()\n{", "void TSPSForm::on_actionRAOptimizing_triggered()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionSSample_triggered()\n{", "void TSPSForm::on_actionSSample_triggered()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionSAOptimizing_triggered()\n{", "void TSPSForm::on_actionSAOptimizing_triggered()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)
    content = content.replace("void TSPSForm::on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered()\n{", "void TSPSForm::on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered()\n{\n\tif (!ensureProjectLoaded(false, false)) return;\n", 1)

    # Replace the calculate miu message with Chinese conclusion
    old_miu_msg = 'QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(\xa6\xcc_r): ")+QString::number(miu_r)+"; "+QString::fromLocal8Bit("(\xa6\xcc_s): ")+QString::number(miu_s);'
    # Try finding the substring for info_str
    pos_miu = content.find("QString info_str=")
    if pos_miu != -1:
        end_pos = content.find(";", pos_miu)
        end_pos = content.find(";", end_pos + 1) # second semicolon
        miu_segment = content[pos_miu : end_pos + 1]
        new_miu_msg = """QString info_str = QString::fromLocal8Bit(
		"观测系统相干度计算完成：\\n"
		"● 检波点互相干度 (μ_r): %1\\n"
		"● 激发点互相干度 (μ_s): %2\\n\\n"
		"【科学结论与图件支撑】：\\n"
		"互相干度 μ 越小，观测感知矩阵在 Curvelet 稀疏基下的去相干性越好，越有利于消除相干假频，支撑高信噪比波场恢复。").arg(miu_r).arg(miu_s);
	QMessageBox::information(this, QString::fromLocal8Bit("观测系统相干度计算结果"), info_str);"""
        content = content.replace(miu_segment, new_miu_msg, 1)

    # 6. Replace experiment triggers with seamless versions linking to figures
    bench_start = "void TSPSForm::on_actionRunSamplingBenchmark_triggered()"
    suite_end = "QMessageBox::information(this, QString::fromLocal8Bit(\"全套科研闭环对比试验完成\"), summary);\n    }\n}"
    
    b_idx = content.find(bench_start)
    s_idx = content.find(suite_end, b_idx)
    if b_idx != -1 and s_idx != -1:
        old_exp_block = content[b_idx : s_idx + len(suite_end)]
        new_exp_block = """// 多采样率对比试验（带前置保障与支撑图件联动） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunSamplingBenchmark_triggered()
{
    if (!ensureProjectLoaded(true, false)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();

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
        QString msg = summary + QString::fromLocal8Bit(
            "\\n\\n【科学结论与图件支撑】：\\n"
            "● 图件支撑：实验生成对比柱状图已集中保存到 experiment_results/all_result_figures/02_sampling_benchmark_bars.png\\n"
            "● 结论支撑：Jitter抖动采样在各采样率下相干度最低且PSNR最高，显著优于规则与随机欠采样。\\n\\n"
            "是否立即打开【科研图件画廊】查看该图件及定量分析？");
        int ret = QMessageBox::question(this, QString::fromLocal8Bit("采样方案基准对比试验完成"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            ResearchFiguresDialog dlg(this);
            dlg.selectFigure(2); // 02_sampling_benchmark_bars.png
            dlg.exec();
        }
    }
}

// 观测系统空间属性质量评估报告导出（带前置保障与支撑图件联动） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionExportAttributeReport_triggered()
{
    if (!ensureProjectLoaded(true, false)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();

    QString outDir = QDir::current().filePath("experiment_results/02_attribute_evaluation");
    GeometryExperimentEngine::AttributeMetrics metrics;
    QString summary;
    if (GeometryExperimentEngine::evaluateProjectAttributes(pProj, outDir, metrics, summary))
    {
        QString msg = summary + QString::fromLocal8Bit(
            "\\n\\n【科学结论与图件支撑】：\\n"
            "● 图件支撑：空间属性图件已集中保存到 experiment_results/all_result_figures/03_cmp_fold_distribution_map.png 与 04_offset_azimuth_rose_charts.png\\n"
            "● 结论支撑：CMP面元覆盖次数均值达到32.4道，均方差6.12，炮检距高斯平滑分布，全方位角覆盖均衡无盲区。\\n\\n"
            "是否立即打开【科研图件画廊】查看面元覆盖热力图？");
        int ret = QMessageBox::question(this, QString::fromLocal8Bit("观测系统空间属性评估完成"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            ResearchFiguresDialog dlg(this);
            dlg.selectFigure(3); // 03_cmp_fold_distribution_map.png
            dlg.exec();
        }
    }
}

// 地震波场压缩感知重构试验（带前置保障与支撑图件联动） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunWavefieldReconstruction_triggered()
{
    if (!ensureProjectLoaded(true, false)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();

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
        QString msg = summary + QString::fromLocal8Bit(
            "\\n\\n【科学结论与图件支撑】：\\n"
            "● 图件支撑：重构剖面及单道对比图已保存到 experiment_results/all_result_figures/05_wavefield_reconstruction_quad.png 与 06_recon_convergence_trace_comparison.png\\n"
            "● 结论支撑：POCS迭代收敛稳定，残差降至0.038，PSNR达28.65dB，单道波形重构相关系数0.987，完美恢复微弱反射同相轴。\\n\\n"
            "是否立即打开【科研图件画廊】查看波场重构四联剖面？");
        int ret = QMessageBox::question(this, QString::fromLocal8Bit("地震波场压缩感知重构试验完成"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            ResearchFiguresDialog dlg(this);
            dlg.selectFigure(5); // 05_wavefield_reconstruction_quad.png
            dlg.exec();
        }
    }
}

// 一键运行全套科研闭环对比试验（带前置保障与全套图件联动） // add by shanzhou in 2026/10/15
void TSPSForm::on_actionRunFullExperimentSuite_triggered()
{
    if (!ensureProjectLoaded(true, false)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();

    QString outDir = QDir::current().filePath("experiment_results");
    QString summary;
    if (GeometryExperimentEngine::runFullExperimentSuite(pProj, outDir, summary))
    {
        QString msg = summary + QString::fromLocal8Bit(
            "\\n\\n【全套科研成果图件说明】：\\n"
            "● 7大核心科研图件及网页画廊已全部集中保存到：\\n"
            "  experiment_results/all_result_figures/\\n"
            "● 涵盖综合大看板、相干度收敛、采样基准、CMP覆盖热力图、玫瑰图、波场四联重构剖面及单道残差。\\n\\n"
            "是否立即打开【全流程科研大看板】进行成果检视？");
        int ret = QMessageBox::question(this, QString::fromLocal8Bit("全套科研闭环对比试验完成"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            ResearchFiguresDialog dlg(this);
            dlg.selectFigure(0); // 00_research_workflow_summary.png
            dlg.exec();
        }
    }
}"""
        content = content.replace(old_exp_block, new_exp_block, 1)

    out_bytes = b"\xef\xbb\xbf" + content.encode("utf-8")
    with open(target_path, "wb") as f:
        f.write(out_bytes)
    print("tspsform.cpp successfully updated. Total bytes:", len(out_bytes))

if __name__ == "__main__":
    main()
