/****************************************************************************
** Form interface generated from reading ui file 'datainput\tdatawizardform.ui'
**
** Created: 星期四 四月 14 22:45:43 2011
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TDATAWIZARDFORM_H
#define TDATAWIZARDFORM_H

#include <qvariant.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include "gproject.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QWidgetStack;
class QWidget;
class QGroupBox;
class QLineEdit;
class QToolButton;
class QTextEdit;
class QProgressBar;
class QPushButton;

class TDataWizardForm : public QDialog
{
    Q_OBJECT

public:
    TDataWizardForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TDataWizardForm();

    QLabel* mStepLabel;
    QWidgetStack* mWidgetStack;
    QWidget* spsPage;
    QGroupBox* groupBox1;
    QLabel* textLabel1;
    QLabel* textLabel2;
    QLabel* textLabel3;
    QLabel* textLabel4_2;
    QLineEdit* mSFileNameEdit;
    QLineEdit* mRFileNameEdit;
    QLineEdit* mXFileNameEdit;
    QLineEdit* mSEGYFileNameEdit_2;
    QToolButton* mSFileBtn;
    QToolButton* mRFileBtn;
    QToolButton* mXFileBtn;
    QToolButton* mSEGYFileBtn_2;
    QTextEdit* mSPSMemo;
    QProgressBar* mSEGYProgressBar_2;
    QWidget* page;
    QTextEdit* mSEGYMemo;
    QProgressBar* mSEGYProgressBar;
    QLabel* textLabel4_3;
    QLineEdit* mcopySEGYFileNameEdit;
    QPushButton* mLoadBtn;
    QPushButton* mNextBtn;
    QPushButton* cancelBtn;

public slots:
    virtual void closeEvent( QCloseEvent * e );
    virtual void loadBtnClicked();
    virtual void cancelBtnClicked();
    virtual void nextBtnClicked();
    virtual void selSFileBtnClicked();
    virtual void selXFileBtnClicked();
    virtual void selSEGYFileBtnClicked();
    virtual void setProject( GProject * pProject );
    virtual void appendLogFile();
    virtual void selectRFile();

protected:
    QVBoxLayout* TDataWizardFormLayout;
    QVBoxLayout* spsPageLayout;
    QGridLayout* groupBox1Layout;
    QHBoxLayout* layout17;
    QVBoxLayout* layout12;
    QVBoxLayout* layout14;
    QVBoxLayout* layout15;
    QGridLayout* pageLayout;
    QHBoxLayout* layout23;
    QHBoxLayout* layout3;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

private:
    Q_INT32 mCurrentStep;
    GProject *mpProject;

    void init();
    void destroy();

};

#endif // TDATAWIZARDFORM_H
