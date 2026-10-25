#ifndef TGEOMETRYIMFORDLG_H
#define TGEOMETRYIMFORDLG_H

#include <QDialog>
#include "ui_tgeometryimfordlg.h"

class TGeometryImforDlg : public QDialog
{
	Q_OBJECT

public:
	TGeometryImforDlg(QWidget *parent = 0);
	~TGeometryImforDlg();
	
	void init(int xl,int yl,float bx,float by,int **foldarray,int Maxfold);
	Ui::TGeometryImforDlgClass ui;
private:
	int mxl;
	int myl;
	float mbx;
	float mby;
	int **mfoldarray;
	int mMaxfold;
	

private slots:
	void on_eJZfold_textChanged(const QString &);
	void on_Cancel_clicked();
};

#endif // TGEOMETRYIMFORDLG_H
