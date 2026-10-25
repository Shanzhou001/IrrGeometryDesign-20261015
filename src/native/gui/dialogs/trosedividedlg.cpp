#include "trosedividedlg.h"
#include <qmessagebox.h>

TRoseDividedlg::TRoseDividedlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

TRoseDividedlg::~TRoseDividedlg()
{

}

void TRoseDividedlg::init()
{
	ui.OffsetDivNumber->setText(QString::number(mOffsetDivNum));
	ui.AngleDivNumber->setText(QString::number(mAngleDivNum));
}
//确定
void TRoseDividedlg::on_OK_clicked()
{
	mOffsetDivNum=ui.OffsetDivNumber->text().toInt();
	mAngleDivNum=ui.AngleDivNumber->text().toInt();
	if(mOffsetDivNum<=0||mAngleDivNum<=0)
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("分段参数错误"));//消息对话框显示提示用户违法操作
	}
	else accept();
}

void TRoseDividedlg::on_Cancel_clicked()
{
	reject();
}