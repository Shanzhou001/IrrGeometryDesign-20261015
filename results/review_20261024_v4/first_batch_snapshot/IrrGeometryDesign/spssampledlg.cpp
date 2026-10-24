#include "spssampledlg.h"

SpsSampleDlg::SpsSampleDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

SpsSampleDlg::~SpsSampleDlg()
{

}


void SpsSampleDlg::on_pushButton_OK_clicked()
{
	int crt_index=ui.comboBox_SamplingMode->currentIndex();
	switch(crt_index){
	case 0:
		mode_value=0;
		break;
	case 1:
		mode_value=1;
		break;
	default : 
		mode_value=2;
	}
	Tsampling_rate=ui.lineEdit_TrSamplingRate->text().toFloat();
	Lsampling_rate=ui.lineEdit_LiSamplingRate->text().toFloat();
	accept();
}

void SpsSampleDlg::on_pushButton_Cancel_clicked()
{
	reject();
}
