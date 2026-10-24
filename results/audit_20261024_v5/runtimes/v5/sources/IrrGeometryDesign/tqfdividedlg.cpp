#include "tqfdividedlg.h"
#include <qmessagebox.h>

TQFDividedlg::TQFDividedlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

TQFDividedlg::~TQFDividedlg()
{

}

void TQFDividedlg::init()
{
	ui.OffsetDivNumber->setText(QString::number(mOffsetDivNum));
	ui.AngleDivNumber->setText(QString::number(mAngleDivNum));
	ui.MaxOffset->setText(QString::number(mMaxOffsetQf));
}

void TQFDividedlg::on_OK_clicked()
{
	mOffsetDivNum=ui.OffsetDivNumber->text().toInt();
	mAngleDivNum=ui.AngleDivNumber->text().toInt();
	mMaxOffsetQf=ui.MaxOffset->text().toInt();
	if(mOffsetDivNum<=0||mAngleDivNum<=0||mMaxOffsetQf<2)
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("分段参数错误"));//消息对话框显示提示用户违法操作
	}
	else accept();
}

void TQFDividedlg::on_Cancel_clicked()
{
	reject();
}