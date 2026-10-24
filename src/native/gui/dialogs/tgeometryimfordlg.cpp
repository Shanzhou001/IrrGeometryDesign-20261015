#include "tgeometryimfordlg.h"
#include <qmessagebox.h>

TGeometryImforDlg::TGeometryImforDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

TGeometryImforDlg::~TGeometryImforDlg()
{

}

void TGeometryImforDlg::init(int xl,int yl,float bx,float by,int **foldarray,int Maxfold)
{
	mxl=xl;myl=yl;mfoldarray=foldarray;
	mbx=bx;mby=by;
	mMaxfold=Maxfold;
}

void TGeometryImforDlg::on_Cancel_clicked()
{
	reject();
}

void TGeometryImforDlg::on_eJZfold_textChanged(const QString &)
{
	int JZfoldNum=0;
	int JZfold=ui.eJZfold->text().toInt();
	if(JZfold>0&&JZfold<=mMaxfold)
	{
		for(int i=0;i<mxl;i++)
		{
			for(int j=0;j<myl;j++)
			{
				if(mfoldarray[i][j]>=JZfold) JZfoldNum++;//统计大于截止覆盖次数的面元个数
			}
		}
		float JZarea=JZfoldNum*mbx*0.001*mby*0.001;//某个覆盖次数（截止覆盖次数）以上面积
		ui.eJZarea->setText(QString::number(JZarea));
	}
	else
	{
		ui.eJZarea->setText(QString::number(0));//错误的
	}

}