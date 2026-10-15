#ifndef TCHANGESHOWAREADLG_H
#define TCHANGESHOWAREADLG_H

#include <QDialog>
#include "ui_tchangeshowareadlg.h"

class TChangeShowareadlg : public QDialog
{
	Q_OBJECT

public:
	TChangeShowareadlg(QWidget *parent = 0);
	~TChangeShowareadlg();
	void init(float maxv,float minv);
	float getMaxShowValue() {return MaxShowValue;}
	float getMinShowValue() {return MinShowValue;}

private:
	Ui::TChangeShowareadlgClass ui;

	float MaxShowValue;
	float MinShowValue;

private slots:
	void on_Cancel_clicked();
	void on_OK_clicked();
};

#endif // TCHANGESHOWAREADLG_H
