#ifndef TQFDIVIDEDLG_H
#define TQFDIVIDEDLG_H

#include <QDialog>
#include "ui_tqfdividedlg.h"

class TQFDividedlg : public QDialog
{
	Q_OBJECT

public:
	TQFDividedlg(QWidget *parent = 0);
	~TQFDividedlg();
	void setOffsetDivNum(int offsetnum) {mOffsetDivNum=offsetnum;}
	void setAngleDivNum(int anglenum) {mAngleDivNum=anglenum;}
	void setMaxOffsetQf(int offset) {mMaxOffsetQf=offset;}
	int getOffsetDivNum() {return mOffsetDivNum;}
	int getAngleDivNum() {return mAngleDivNum;}
	int getMaxOffsetQf() {return mMaxOffsetQf;}
	void init();

private:
	Ui::TQFDividedlgClass ui;
	int mOffsetDivNum;
	int mAngleDivNum;
	int mMaxOffsetQf;

private slots:
	void on_Cancel_clicked();
	void on_OK_clicked();
};

#endif // TQFDIVIDEDLG_H
