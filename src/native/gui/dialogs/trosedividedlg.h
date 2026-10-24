#ifndef TROSEDIVIDEDLG_H
#define TROSEDIVIDEDLG_H

#include <QDialog>
#include "ui_trosedividedlg.h"

class TRoseDividedlg : public QDialog
{
	Q_OBJECT

public:
	TRoseDividedlg(QWidget *parent = 0);
	~TRoseDividedlg();
	void setOffsetDivNum(int offsetnum) {mOffsetDivNum=offsetnum;}
	void setAngleDivNum(int anglenum) {mAngleDivNum=anglenum;}
	int getOffsetDivNum() {return mOffsetDivNum;}
	int getAngleDivNum() {return mAngleDivNum;}
	void init();

private:
	Ui::TRoseDividedlgClass ui;
	int mOffsetDivNum;
	int mAngleDivNum;

private slots:
	void on_Cancel_clicked();
	void on_OK_clicked();
};

#endif // TROSEDIVIDEDLG_H
