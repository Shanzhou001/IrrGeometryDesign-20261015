#ifndef DRAW2DFRM_H
#define DRAW2DFRM_H

#include <QMainWindow>
#include "ui_draw2dfrm.h"
#include "draw2Dview.h"
#include "colorview.h"
#include <QSplitter>

class Draw2Dfrm : public QMainWindow
{
	Q_OBJECT

public:
	Draw2Dfrm(QWidget *parent = 0);
	~Draw2Dfrm();

private:
	Ui::Draw2DfrmClass ui;
private:
	Draw2DView *mpView;//画图指针
	QSplitter *mSpl;
	tcolorview *mpColorview;//颜色显示类

	bool in_loop ;
	float mlx,mly;
	float **mshotfocus;
	float **mrecvfocus;
	float **mSumfocus;
	float minSfocus;
	float maxSfocus;
	float minRfocus;
	float maxRfocus;
	float minSumfocus;
	float maxSumfocus;
	int **mfold;
	int maxfold;
	int minfold;
	QString ovtstr;
public:
	int exec();
	void setminbx(float minbx);
	void setminby(float minby);
	void set2Darray(float *array1);
	void setxnumber(int lx);

	void setlx(int lx) {mlx=lx;}
	void setly(int ly) {mly=ly;}
	void setstr(QString str) {ovtstr=str;}

	void setFold(int **fold) 
	{
		mfold=fold;
		maxfold=0;minfold=100000000000;
		for(int i=0;i<mlx;i++)
		{
			for(int j=0;j<mly;j++)
			{
				if(maxfold<fold[i][j]) maxfold=fold[i][j];
				if(minfold>fold[i][j]) minfold=fold[i][j];
			}
		}
		on_actionOVTfold_triggered();
	}

	void setShotFocus(float **sfocus) 
	{
		mshotfocus=sfocus;
		maxSfocus=0;minSfocus=100000000000;
		for(int i=0;i<mlx;i++)
		{
			for(int j=0;j<mly;j++)
			{
				if(maxSfocus<mshotfocus[i][j]) maxSfocus=mshotfocus[i][j];
				if(minSfocus>mshotfocus[i][j]) minSfocus=mshotfocus[i][j];
			}
		}
		on_actionShotFocus_triggered();
	}
	void setRecvFocus(float **rfocus) 
	{
		mrecvfocus=rfocus;
		maxRfocus=0;minRfocus=100000000000;
		for(int i=0;i<mlx;i++)
		{
			for(int j=0;j<mly;j++)
			{
				if(maxRfocus<mrecvfocus[i][j]) maxRfocus=mrecvfocus[i][j];
				if(minRfocus>mrecvfocus[i][j]) minRfocus=mrecvfocus[i][j];
			}
		}
	}
	void setSumFocus(float **sumfovus) 
	{
		mSumfocus=sumfovus;
		maxSumfocus=0;minSumfocus=100000000000;
		for(int i=0;i<mlx;i++)
		{
			for(int j=0;j<mly;j++)
			{
				if(maxSumfocus<mSumfocus[i][j]) maxSumfocus=mSumfocus[i][j];
				if(minSumfocus>mSumfocus[i][j]) minSumfocus=mSumfocus[i][j];
			}
		}
	}


private slots:
	void on_actionOVTfold_triggered();
	void on_actionDoubleFocus_triggered();
	void on_actionReceiverFocus_triggered();
	void on_actionShotFocus_triggered();
};

#endif // DRAW2DFRM_H
