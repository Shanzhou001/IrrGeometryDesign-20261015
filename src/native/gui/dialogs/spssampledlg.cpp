#include "spssampledlg.h"

SpsSampleDlg::SpsSampleDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    Tsampling_rate=ui.lineEdit_TrSamplingRate->text().toFloat();
    Lsampling_rate=ui.lineEdit_LiSamplingRate->text().toFloat();
	mode_value = 0;
	Tsampling_rate = ui.lineEdit_TrSamplingRate->text().toFloat();
	Lsampling_rate = ui.lineEdit_LiSamplingRate->text().toFloat();
}

SpsSampleDlg::~SpsSampleDlg()
{

}


void SpsSampleDlg::on_pushButton_OK_clicked()
{
    bool okT=false,okL=false;
    float t=ui.lineEdit_TrSamplingRate->text().toFloat(&okT);
    float l=ui.lineEdit_LiSamplingRate->text().toFloat(&okL);
    bool validT=okT && t>0 && t<=1, validL=okL && l>0 && l<=1;
    ui.lineEdit_TrSamplingRate->setStyleSheet(validT?"":"border:2px solid red");
    ui.lineEdit_LiSamplingRate->setStyleSheet(validL?"":"border:2px solid red");
    ui.lineEdit_TrSamplingRate->setToolTip("Required: finite sampling ratio in (0, 1].");
    ui.lineEdit_LiSamplingRate->setToolTip("Required: finite sampling ratio in (0, 1].");
    if(!validT || !validL){(validT?ui.lineEdit_LiSamplingRate:ui.lineEdit_TrSamplingRate)->setFocus();return;}
    mode_value=ui.comboBox_SamplingMode->currentIndex();Tsampling_rate=t;Lsampling_rate=l;
	accept();
}

void SpsSampleDlg::on_pushButton_Cancel_clicked()
{
	reject();
}
