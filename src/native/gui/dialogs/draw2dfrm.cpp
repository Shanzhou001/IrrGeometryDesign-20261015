#include "draw2dfrm.h"


Draw2Dfrm::Draw2Dfrm(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QLabel *coordinate = new QLabel( statusBar() );
	coordinate->setMinimumWidth( 200 );
	statusBar()->addWidget( coordinate, 0, true );

	mSpl=new QSplitter(this);
	mpView=new Draw2DView(mSpl);
	mpColorview=new tcolorview(mSpl);	
	mpView->setlabel(coordinate);
	mpView->refresh();
	mpView->show();
	mpColorview->hide();

	QList<int> list;
	list.append(800);

	list.append(100);
	list.append(700);
	mSpl->setSizes(list);
	setCentralWidget(mSpl);

	mshotfocus=NULL;
	mrecvfocus=NULL;
	mSumfocus=NULL;
	mfold=NULL;
	in_loop = false;
	mlx = 0; mly = 0;
	minSfocus = maxSfocus = minRfocus = maxRfocus = minSumfocus = maxSumfocus = 0.0f;
	maxfold = minfold = 0;

}

Draw2Dfrm::~Draw2Dfrm()
{
	if (in_loop)
	{
		in_loop = false;
		qApp->exit_loop();
	}
}

void Draw2Dfrm::closeEvent(QCloseEvent *e)
{
	e->accept();
	if (in_loop)
	{
		in_loop = false;
		qApp->exit_loop();
	}
}

void Draw2Dfrm::setminbx(float minbx)
{
	mpView->setminbx(minbx);
}

void Draw2Dfrm::setminby(float minby)
{
	mpView->setminby(minby);
}

void Draw2Dfrm::set2Darray(float *array1)
{

}

void Draw2Dfrm::setxnumber(int lx)
{

	mpView->refresh();
}

//震源聚焦

void Draw2Dfrm::on_actionShotFocus_triggered()
{
	if(mshotfocus==NULL) return;
	mpView->setcolorbelt(mpColorview->getcolorbeltvary());
	mpView->drawEnergy(mshotfocus,mlx,mly,maxSfocus,minSfocus,QString::fromLocal8Bit("炮点聚焦"));
	mpColorview->show();//画色标视图
	mpColorview->drawcolorcodevary(maxSfocus,minSfocus);//画色标
	mpColorview->setCRPview(mpView);
}


//检波器聚焦
void Draw2Dfrm::on_actionReceiverFocus_triggered()
{
	if(mrecvfocus==NULL) return;
	mpView->setcolorbelt(mpColorview->getcolorbeltvary());
	mpView->drawEnergy(mrecvfocus,mlx,mly,maxRfocus,minRfocus,QString::fromLocal8Bit("检波点聚焦"));
	mpColorview->show();//画色标视图
	mpColorview->drawcolorcodevary(maxRfocus,minRfocus);//画色标
	mpColorview->setCRPview(mpView);
}
//双聚焦
void Draw2Dfrm::on_actionDoubleFocus_triggered()
{
	if(mSumfocus==NULL) return;
	mpView->setcolorbelt(mpColorview->getcolorbeltvary());
	mpView->drawEnergy(mSumfocus,mlx,mly,maxSumfocus,minSumfocus,QString::fromLocal8Bit("双聚焦"));
	mpColorview->show();//画色标视图
	mpColorview->drawcolorcodevary(maxSumfocus,minSumfocus);//画色标
	mpColorview->setCRPview(mpView);
}

int Draw2Dfrm::exec()
{
	if ( in_loop ) {
		qWarning( "QDialog::exec: Recursive call detected" );
		return -1;
	}
	show();
	in_loop = true;
	qApp->enter_loop();		
	return 0;   // 2026.10.20 修复 C4715：补全返回路径
}

//ovt覆盖次数
void Draw2Dfrm::on_actionOVTfold_triggered()
{
	if (mfold == NULL || mlx <= 0 || mly <= 0) return;
	mpColorview->hide();
	mpView->drawworkareaBinfold(mfold,mlx,mly,ovtstr);
}