#ifndef CHANGECOORDINATEDLG_H
#define CHANGECOORDINATEDLG_H

#include <QDialog>
#include "ui_changecoordinatedlg.h"

class Changecoordinatedlg : public QDialog
{
	Q_OBJECT

public:
	Changecoordinatedlg(QWidget *parent = 0);
	~Changecoordinatedlg();

private:
	Ui::ChangecoordinatedlgClass ui;
private:
	float  coordmaxvalue;
public:
	float getcoordmaxvalue() {return coordmaxvalue;}
	void setcoordmaxvalue(float coordmaxvalue1) {coordmaxvalue=coordmaxvalue1;}
private slots:
	void on_CoordinateMax_textChanged(const QString &);
	void on_ChangCoordinateCancel_clicked();
	void on_ChangCoordinateOK_clicked();
};

#endif // CHANGECOORDINATEDLG_H
