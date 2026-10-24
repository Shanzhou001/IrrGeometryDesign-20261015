#ifndef SPSSAMPLEDLG_H
#define SPSSAMPLEDLG_H

#include <QDialog>
#include "ui_spssampledlg.h"
#include "sample.h"

class SpsSampleDlg : public QDialog
{
	Q_OBJECT

public:
	SpsSampleDlg(QWidget *parent = 0);
	~SpsSampleDlg();

	int mode_value;
	float Tsampling_rate;
	float Lsampling_rate;
private:
	Ui::SpsSampleDlgClass ui;

private slots:
	void on_pushButton_Cancel_clicked();
	void on_pushButton_OK_clicked();
};

#endif // SPSSAMPLEDLG_H
