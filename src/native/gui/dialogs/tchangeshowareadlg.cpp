#include "tchangeshowareadlg.h"
#include <qmessagebox.h>

TChangeShowareadlg::TChangeShowareadlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	MaxShowValue = 1.0f;
	MinShowValue = 0.0f;
	init(1.0f, 0.0f);
}

TChangeShowareadlg::~TChangeShowareadlg()
{

}

void TChangeShowareadlg::init(float maxv,float minv)
{
	ui.MaxShowValue->setText(QString::number(maxv));
	ui.MinShowValue->setText(QString::number(minv));
}

void TChangeShowareadlg::on_OK_clicked()
{
	MaxShowValue=ui.MaxShowValue->text().toFloat();
	MinShowValue=ui.MinShowValue->text().toFloat();
	if(MaxShowValue<=MinShowValue)
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("分段参数错误"));//消息对话框显示提示用户违法操作
	}
	else accept();
}

void TChangeShowareadlg::on_Cancel_clicked()
{
	reject();
}