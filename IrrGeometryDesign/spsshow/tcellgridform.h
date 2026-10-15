/****************************************************************************
** Form interface generated from reading ui file 'src\sps\tcellgridform.ui'
**
** Created: 星期四 二月 17 17:27:37 2011
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TCELLGRIDFORM_H
#define TCELLGRIDFORM_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>
#include "ui_tcellgridform.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;

class TCellGridForm : public QDialog
{
    Q_OBJECT

public:
    TCellGridForm( QWidget* parent = 0);
    ~TCellGridForm();

   

public slots:
    virtual void accept();

protected:
    float mX0;
    float mY0;
	float mXn, mYn;

public:
	Ui::TCellGridForm ui;

private:
    void init();
    void destroy();


private slots://  [Liu Wei 3/28/2023]
	void on_cYGridEdit_textChanged(const QString &txt);
	void on_cXGridEdit_textChanged(const QString &txt);
};

#endif // TCELLGRIDFORM_H
