/****************************************************************************
** Form interface generated from reading ui file 'src\sps\tspsdlg.ui'
**
** Created: 星期四 二月 17 17:27:32 2011
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TSPSDLG_H
#define TSPSDLG_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include "gspsparam.h"
#include "ui_tspsdlg.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;
class Q3Frame;
class QPushButton;

class TSPSDlg : public QDialog
{
    Q_OBJECT

public:
    TSPSDlg( QWidget* parent = 0);
    ~TSPSDlg();

 

public slots:
    virtual void setSPS( GSPSParam * pSPSParam );
    virtual void accept();

protected:
    GSPSParam *mpSPSParam;

public:
	Ui::TSPSDlg ui;
	void is2Dinit()	;

private:
    void init();
    void destroy();

;


private slots:
	void AutosetSY0();// 根据接收线参数和炮线参数自动计算第一个炮点起始y坐标值 [Liu Wei 6/21/2023]
	void AutosetSX0();// 根据接收线参数和偏移距参数自动计算第一个炮点起始x坐标值 [Liu Wei 7/31/2023]

	void on_OrginalByShot_clicked();
	void on_OrginalByReceiver_clicked();

	void on_cAngleEdit_textChanged(const QString &);
	void on_cMoveX0Edit_textChanged(const QString &);
	void on_cMoveY0Edit_textChanged(const QString &);
	//void on_cLMaxOffEdit_textChanged(QString);	// 暂时取消 [Liu Wei 5/29/2023]
};

#endif // TSPSDLG_H
