#include "spssampledlg.h"

SpsSampleDlg::SpsSampleDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mode_value = 0;
	Tsampling_rate = 0.75f;
	Lsampling_rate = 0.75f;
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
	if (Tsampling_rate <= 0.0f || Tsampling_rate > 1.0f) Tsampling_rate = 0.75f;
	if (Lsampling_rate <= 0.0f || Lsampling_rate > 1.0f) Lsampling_rate = 0.75f;
	accept();
}

void SpsSampleDlg::on_pushButton_Cancel_clicked()
{
	reject();
}
