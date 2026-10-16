import re

with open("IrrGeometryDesign/spsshow/tspsform.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# 1. Update init()
old_init_pat = re.compile(r'void TSPSForm::init\(\)\s*\{.*?setCentralWidget\(mSpl\);\s*\}', re.DOTALL)

new_init = '''void TSPSForm::init()
{
	//----调整窗口大小和位置----
	QPoint p( 0, 0 );
	QRect desk;
	int top = 0, left = 0, width = 1040, height = 720;
	desk = QApplication::desktop()->availableGeometry( 1 );

	left = ( desk.width()-width )/2;
	top = ( desk.height()-height )/2;
	setGeometry( left, top, width, height );

	QWidget *mainContainer = new QWidget(this);
	QVBoxLayout *mainVBox = new QVBoxLayout(mainContainer);
	mainVBox->setContentsMargins(4, 4, 4, 4);
	mainVBox->setSpacing(4);

	QFrame *workflowBar = new QFrame(mainContainer);
	workflowBar->setStyleSheet(
		"QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #eef2f7); "
		"border: 1px solid #b0c4de; border-radius: 4px; padding: 3px; }"
	);
	QHBoxLayout *flowLayout = new QHBoxLayout(workflowBar);
	flowLayout->setContentsMargins(4, 2, 4, 2);
	flowLayout->setSpacing(5);

	QLabel *lblFlowTitle = new QLabel(QString::fromLocal8Bit("<b>【科研全流程导航】:</b>"), workflowBar);
	lblFlowTitle->setStyleSheet("color: #0b3c6d; font-family: SimSun; font-size: 12px;");
	flowLayout->addWidget(lblFlowTitle);

	QString btnNormalStyle = 
		"QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f0f4f8); "
		"border: 1px solid #94a3b8; border-radius: 3px; padding: 4px 6px; font-family: SimSun; font-size: 12px; color: #1e293b; }"
		"QPushButton:hover { background: #e2e8f0; border-color: #3b82f6; color: #1d4ed8; }"
		"QPushButton:pressed { background: #cbd5e1; }";

	QString btnAccentStyle = 
		"QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2563eb, stop:1 #1d4ed8); "
		"border: 1px solid #1e40af; border-radius: 3px; padding: 4px 8px; font-family: SimSun; font-weight: bold; font-size: 12px; color: #ffffff; }"
		"QPushButton:hover { background: #1d4ed8; border-color: #172554; }"
		"QPushButton:pressed { background: #1e3a8a; }";

	QString btnGalleryStyle = 
		"QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #059669, stop:1 #047857); "
		"border: 1px solid #065f46; border-radius: 3px; padding: 4px 7px; font-family: SimSun; font-weight: bold; font-size: 12px; color: #ffffff; }"
		"QPushButton:hover { background: #047857; }"
		"QPushButton:pressed { background: #064e3b; }";

	QPushButton *btnStep1 = new QPushButton(QString::fromLocal8Bit("1. 示范工区"), workflowBar);
	btnStep1->setStyleSheet(btnNormalStyle);
	btnStep1->setToolTip(QString::fromLocal8Bit("一键载入标准三维地震示范观测系统 (8接收线x128道，12激发线)"));
	connect(btnStep1, SIGNAL(clicked()), this, SLOT(quickLoadDemoSPS()));
	flowLayout->addWidget(btnStep1);

	QPushButton *btnStep2 = new QPushButton(QString::fromLocal8Bit("2. 导入SPS"), workflowBar);
	btnStep2->setStyleSheet(btnNormalStyle);
	btnStep2->setToolTip(QString::fromLocal8Bit("通过数据向导导入外部 SPS 文件 (*.s, *.r, *.x)"));
	connect(btnStep2, SIGNAL(clicked()), this, SLOT(on_Load_SPS_Action_triggered()));
	flowLayout->addWidget(btnStep2);

	QPushButton *btnStep3 = new QPushButton(QString::fromLocal8Bit("3. CMP面元网格"), workflowBar);
	btnStep3->setStyleSheet(btnNormalStyle);
	btnStep3->setToolTip(QString::fromLocal8Bit("划分 CMP 面元网格并自动计算覆盖次数"));
	connect(btnStep3, SIGNAL(clicked()), this, SLOT(grid()));
	flowLayout->addWidget(btnStep3);

	QPushButton *btnStep4 = new QPushButton(QString::fromLocal8Bit("4. 排列退火优化"), workflowBar);
	btnStep4->setStyleSheet(btnNormalStyle);
	btnStep4->setToolTip(QString::fromLocal8Bit("执行空间非规则采样与模拟退火优化互相干度 μ"));
	connect(btnStep4, SIGNAL(clicked()), this, SLOT(on_actionRAOptimizing_triggered()));
	flowLayout->addWidget(btnStep4);

	QPushButton *btnStep5 = new QPushButton(QString::fromLocal8Bit("5. 多采样率试验"), workflowBar);
	btnStep5->setStyleSheet(btnNormalStyle);
	btnStep5->setToolTip(QString::fromLocal8Bit("多采样率基准对比试验 (30%, 50%, 70%)"));
	connect(btnStep5, SIGNAL(clicked()), this, SLOT(on_actionRunSamplingBenchmark_triggered()));
	flowLayout->addWidget(btnStep5);

	QPushButton *btnStep6 = new QPushButton(QString::fromLocal8Bit("6. 属性质量评估"), workflowBar);
	btnStep6->setStyleSheet(btnNormalStyle);
	btnStep6->setToolTip(QString::fromLocal8Bit("导出观测系统空间属性质量评估报告与热力图、玫瑰图"));
	connect(btnStep6, SIGNAL(clicked()), this, SLOT(on_actionExportAttributeReport_triggered()));
	flowLayout->addWidget(btnStep6);

	QPushButton *btnStep7 = new QPushButton(QString::fromLocal8Bit("7. 波场POCS重构"), workflowBar);
	btnStep7->setStyleSheet(btnNormalStyle);
	btnStep7->setToolTip(QString::fromLocal8Bit("地震波场欠采样与压缩感知 POCS 凸集投影重构试验"));
	connect(btnStep7, SIGNAL(clicked()), this, SLOT(on_actionRunWavefieldReconstruction_triggered()));
	flowLayout->addWidget(btnStep7);

	QPushButton *btnSuite = new QPushButton(QString::fromLocal8Bit("★ 一键全流程闭环验证"), workflowBar);
	btnSuite->setStyleSheet(btnAccentStyle);
	btnSuite->setToolTip(QString::fromLocal8Bit("自动化全流程执行全部科研对比试验并集中生成科研图件"));
	connect(btnSuite, SIGNAL(clicked()), this, SLOT(on_actionRunFullExperimentSuite_triggered()));
	flowLayout->addWidget(btnSuite);

	QPushButton *btnGallery = new QPushButton(QString::fromLocal8Bit("成果图件画廊"), workflowBar);
	btnGallery->setStyleSheet(btnGalleryStyle);
	btnGallery->setToolTip(QString::fromLocal8Bit("查看 00~06 全部科研成果图件与科学结论"));
	connect(btnGallery, SIGNAL(clicked()), this, SLOT(on_actionViewGallery_triggered()));
	flowLayout->addWidget(btnGallery);

	QPushButton *btnDir = new QPushButton(QString::fromLocal8Bit("图件目录"), workflowBar);
	btnDir->setStyleSheet(btnNormalStyle);
	btnDir->setToolTip(QString::fromLocal8Bit("打开保存全部数据图与结果图的 all_result_figures 文件夹"));
	connect(btnDir, SIGNAL(clicked()), this, SLOT(on_actionOpenFiguresDir_triggered()));
	flowLayout->addWidget(btnDir);

	mSpl=new QSplitter(mainContainer);
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

	connect(ui.saveAction, SIGNAL(triggered()), this, SLOT(saveSPS()));
	connect(ui.zoomAction, SIGNAL(triggered()), this, SLOT(zoom()));
	connect(ui.gridAction, SIGNAL(triggered()), this, SLOT(grid()));
	connect(ui.actionQuickDemoSPS, SIGNAL(triggered()), this, SLOT(quickLoadDemoSPS()));
	connect(ui.actionRunSamplingBenchmark, SIGNAL(triggered()), this, SLOT(on_actionRunSamplingBenchmark_triggered()));
	connect(ui.actionExportAttributeReport, SIGNAL(triggered()), this, SLOT(on_actionExportAttributeReport_triggered()));
	connect(ui.actionRunWavefieldReconstruction, SIGNAL(triggered()), this, SLOT(on_actionRunWavefieldReconstruction_triggered()));
	connect(ui.actionRunFullExperimentSuite, SIGNAL(triggered()), this, SLOT(on_actionRunFullExperimentSuite_triggered()));
	connect(ui.actionViewGallery, SIGNAL(triggered()), this, SLOT(on_actionViewGallery_triggered()));
	connect(ui.actionOpenFiguresDir, SIGNAL(triggered()), this, SLOT(on_actionOpenFiguresDir_triggered()));

	mainVBox->addWidget(workflowBar);
	mainVBox->addWidget(mSpl, 1);
	setCentralWidget(mainContainer);
}'''

text = old_init_pat.sub(new_init, text)

# 2. Replace lines 1700, 1885, 2094, 2206
old_1700 = '''QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_xj_before);
			QMessageBox::information(NULL,tr("Computation Information"),info_str);'''

new_1700 = '''QString info_str = QString::fromLocal8Bit("互相干度参数 (μ): ") + QString::number(miu_xj_before, 'f', 4);
			QMessageBox::information(this, QString::fromLocal8Bit("互相干度计算结果"), info_str);'''
text = text.replace(old_1700, new_1700)

old_1885 = '''QString miu_str=QString::fromLocal8Bit("miu0:")+QString::number(miu0)+","+QString::fromLocal8Bit("miu1:")+QString::number(miu1)+","+QString::fromLocal8Bit("优化前最大互相关值:")+QString::number(miu_xj_after)+";";
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_after)+","+tr("Optimizing time: ")+QString::number(duration)+" s";
			QMessageBox::information(NULL,tr("Computation Information"),miu_str+info_str);'''

new_1885 = '''QString miu_str = QString::fromLocal8Bit("检波点排列模拟退火优化完成：\\n") +
				QString::fromLocal8Bit("● 初始互相干度: ") + QString::number(miu0, 'f', 4) + QString::fromLocal8Bit("\\n") +
				QString::fromLocal8Bit("● 优化后互相干度 (μ): ") + QString::number(miu_after, 'f', 4) + QString::fromLocal8Bit("\\n") +
				QString::fromLocal8Bit("● 优化耗时: ") + QString::number(duration, 'f', 2) + QString::fromLocal8Bit(" 秒");
			QMessageBox::information(this, QString::fromLocal8Bit("排列退火优化计算完成"), miu_str);'''
text = text.replace(old_1885, new_1885)

old_2094 = '''QString miu_str=QString::fromLocal8Bit("miu0:")+QString::number(miu0)+","+QString::fromLocal8Bit("miu1:")+QString::number(miu1)+","+QString::fromLocal8Bit("优化前最大互相关值:")+QString::number(miu_xj_after)+";";
			QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_after)+","+tr("Optimizing time: ")+QString::number(duration)+" s";
			QMessageBox::information(NULL,tr("Computation Information"),miu_str+info_str);'''

new_2094 = '''QString miu_str = QString::fromLocal8Bit("激发点排列模拟退火优化完成：\\n") +
				QString::fromLocal8Bit("● 初始互相干度: ") + QString::number(miu0, 'f', 4) + QString::fromLocal8Bit("\\n") +
				QString::fromLocal8Bit("● 优化后互相干度 (μ): ") + QString::number(miu_after, 'f', 4) + QString::fromLocal8Bit("\\n") +
				QString::fromLocal8Bit("● 优化耗时: ") + QString::number(duration, 'f', 2) + QString::fromLocal8Bit(" 秒");
			QMessageBox::information(this, QString::fromLocal8Bit("排列退火优化计算完成"), miu_str);'''
text = text.replace(old_2094, new_2094)

old_2206 = '''QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ): ")+QString::number(miu_xj_before);
	QMessageBox::information(NULL,tr("Computation Information"),info_str);'''

new_2206 = '''QString info_str = QString::fromLocal8Bit("当前载入观测系统互相干度 (μ): ") + QString::number(miu_xj_before, 'f', 4);
	QMessageBox::information(this, QString::fromLocal8Bit("互相干度计算结果"), info_str);'''
text = text.replace(old_2206, new_2206)

# 3. Update the 4 experiment slots to call ensureProjectLoaded
old_bench_slot = re.compile(r'void TSPSForm::on_actionRunSamplingBenchmark_triggered\(\)\s*\{.*?if \(!pProj \|\| \(!pProj->getSLineSet\(\).*?return;\s*\}', re.DOTALL)
new_bench_head = '''void TSPSForm::on_actionRunSamplingBenchmark_triggered()
{
    if (!ensureProjectLoaded(false, false)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();'''
text = old_bench_slot.sub(new_bench_head, text)

old_attr_slot = re.compile(r'void TSPSForm::on_actionExportAttributeReport_triggered\(\)\s*\{.*?if \(!pProj->mIsNewImfo \|\| \(!pProj->getRLineSetNew\(\).*?return;\s*\}', re.DOTALL)
new_attr_head = '''void TSPSForm::on_actionExportAttributeReport_triggered()
{
    if (!ensureProjectLoaded(true, true)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();'''
text = old_attr_slot.sub(new_attr_head, text)

old_recon_slot = re.compile(r'void TSPSForm::on_actionRunWavefieldReconstruction_triggered\(\)\s*\{.*?if \(!pProj->mIsNewImfo \|\| \(!pProj->getRLineSetNew\(\).*?return;\s*\}', re.DOTALL)
new_recon_head = '''void TSPSForm::on_actionRunWavefieldReconstruction_triggered()
{
    if (!ensureProjectLoaded(false, true)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();'''
text = old_recon_slot.sub(new_recon_head, text)

old_suite_slot = re.compile(r'void TSPSForm::on_actionRunFullExperimentSuite_triggered\(\)\s*\{.*?if \(!pProj->mIsNewImfo \|\| \(!pProj->getRLineSetNew\(\).*?return;\s*\}', re.DOTALL)
new_suite_head = '''void TSPSForm::on_actionRunFullExperimentSuite_triggered()
{
    if (!ensureProjectLoaded(true, true)) return;
    GProject *pProj = mpProject ? mpProject : GProject::getCurrentProject();'''
text = old_suite_slot.sub(new_suite_head, text)

# Save with UTF-8 BOM
with open("IrrGeometryDesign/spsshow/tspsform.cpp", "wb") as f:
    f.write(b"\\xef\\xbb\\xbf" + text.encode("utf-8"))

print("Patched tspsform.cpp successfully!")
