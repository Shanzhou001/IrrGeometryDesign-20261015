#include "changecoordinatedlg.h"

Changecoordinatedlg::Changecoordinatedlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.CoordinateMax->setText(QString::number(0));
}

Changecoordinatedlg::~Changecoordinatedlg()
{

}


void Changecoordinatedlg::on_ChangCoordinateOK_clicked()
{
	accept();
}

void Changecoordinatedlg::on_ChangCoordinateCancel_clicked()
{
	reject();
}

void Changecoordinatedlg::on_CoordinateMax_textChanged(const QString &)
{
	coordmaxvalue=ui.CoordinateMax->text().toFloat();
}