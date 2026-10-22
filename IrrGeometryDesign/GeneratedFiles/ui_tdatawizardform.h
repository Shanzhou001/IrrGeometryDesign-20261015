/********************************************************************************
** Form generated from reading UI file 'tdatawizardform.ui'
**
** Created: Thu Oct 22 11:47:39 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TDATAWIZARDFORM_H
#define UI_TDATAWIZARDFORM_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <Qt3Support/Q3FileDialog>
#include <Qt3Support/Q3ProgressDialog>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfoListIterator>
#include <QtGui/QMessageBox>
#include "gproject.h"

QT_BEGIN_NAMESPACE

class Ui_TDataWizardForm
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *mStepLabel;
    Q3GroupBox *groupBox1;
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout;
    QLabel *textLabel1;
    QLabel *textLabel2;
    QLabel *textLabel3;
    QVBoxLayout *vboxLayout1;
    QLineEdit *mSFileNameEdit;
    QLineEdit *mRFileNameEdit;
    QLineEdit *mXFileNameEdit;
    QVBoxLayout *vboxLayout2;
    QToolButton *mSFileBtn;
    QToolButton *mRFileBtn;
    QToolButton *mXFileBtn;
    QTextEdit *mSPSMemo;
    QProgressBar *mSEGYProgressBar;
    QHBoxLayout *hboxLayout1;
    QPushButton *mLoadBtn;
    QSpacerItem *spacer1;
    QPushButton *mNextBtn;
    QPushButton *cancelBtn;

    void setupUi(QDialog *TDataWizardForm)
    {
        if (TDataWizardForm->objectName().isEmpty())
            TDataWizardForm->setObjectName(QString::fromUtf8("TDataWizardForm"));
        TDataWizardForm->resize(501, 413);
        verticalLayout = new QVBoxLayout(TDataWizardForm);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        mStepLabel = new QLabel(TDataWizardForm);
        mStepLabel->setObjectName(QString::fromUtf8("mStepLabel"));
        mStepLabel->setMinimumSize(QSize(0, 20));
        mStepLabel->setMaximumSize(QSize(32767, 20));
        mStepLabel->setWordWrap(false);

        verticalLayout->addWidget(mStepLabel);

        groupBox1 = new Q3GroupBox(TDataWizardForm);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        groupBox1->setColumnLayout(0, Qt::Vertical);
        groupBox1->layout()->setSpacing(6);
        groupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout = new QGridLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(groupBox1->layout());
        if (boxlayout)
            boxlayout->addLayout(gridLayout);
        gridLayout->setAlignment(Qt::AlignTop);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        vboxLayout = new QVBoxLayout();
        vboxLayout->setSpacing(6);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        textLabel1 = new QLabel(groupBox1);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
        textLabel1->setWordWrap(false);

        vboxLayout->addWidget(textLabel1);

        textLabel2 = new QLabel(groupBox1);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        textLabel2->setWordWrap(false);

        vboxLayout->addWidget(textLabel2);

        textLabel3 = new QLabel(groupBox1);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        textLabel3->setWordWrap(false);

        vboxLayout->addWidget(textLabel3);


        hboxLayout->addLayout(vboxLayout);

        vboxLayout1 = new QVBoxLayout();
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        mSFileNameEdit = new QLineEdit(groupBox1);
        mSFileNameEdit->setObjectName(QString::fromUtf8("mSFileNameEdit"));

        vboxLayout1->addWidget(mSFileNameEdit);

        mRFileNameEdit = new QLineEdit(groupBox1);
        mRFileNameEdit->setObjectName(QString::fromUtf8("mRFileNameEdit"));

        vboxLayout1->addWidget(mRFileNameEdit);

        mXFileNameEdit = new QLineEdit(groupBox1);
        mXFileNameEdit->setObjectName(QString::fromUtf8("mXFileNameEdit"));

        vboxLayout1->addWidget(mXFileNameEdit);


        hboxLayout->addLayout(vboxLayout1);

        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(6);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        mSFileBtn = new QToolButton(groupBox1);
        mSFileBtn->setObjectName(QString::fromUtf8("mSFileBtn"));

        vboxLayout2->addWidget(mSFileBtn);

        mRFileBtn = new QToolButton(groupBox1);
        mRFileBtn->setObjectName(QString::fromUtf8("mRFileBtn"));

        vboxLayout2->addWidget(mRFileBtn);

        mXFileBtn = new QToolButton(groupBox1);
        mXFileBtn->setObjectName(QString::fromUtf8("mXFileBtn"));

        vboxLayout2->addWidget(mXFileBtn);


        hboxLayout->addLayout(vboxLayout2);


        gridLayout->addLayout(hboxLayout, 0, 0, 1, 1);


        verticalLayout->addWidget(groupBox1);

        mSPSMemo = new QTextEdit(TDataWizardForm);
        mSPSMemo->setObjectName(QString::fromUtf8("mSPSMemo"));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        mSPSMemo->setFont(font);
        mSPSMemo->setReadOnly(true);

        verticalLayout->addWidget(mSPSMemo);

        mSEGYProgressBar = new QProgressBar(TDataWizardForm);
        mSEGYProgressBar->setObjectName(QString::fromUtf8("mSEGYProgressBar"));

        verticalLayout->addWidget(mSEGYProgressBar);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        mLoadBtn = new QPushButton(TDataWizardForm);
        mLoadBtn->setObjectName(QString::fromUtf8("mLoadBtn"));
        mLoadBtn->setDefault(true);

        hboxLayout1->addWidget(mLoadBtn);

        spacer1 = new QSpacerItem(170, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacer1);

        mNextBtn = new QPushButton(TDataWizardForm);
        mNextBtn->setObjectName(QString::fromUtf8("mNextBtn"));
        mNextBtn->setEnabled(false);
        mNextBtn->setDefault(true);

        hboxLayout1->addWidget(mNextBtn);

        cancelBtn = new QPushButton(TDataWizardForm);
        cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));

        hboxLayout1->addWidget(cancelBtn);


        verticalLayout->addLayout(hboxLayout1);


        retranslateUi(TDataWizardForm);
        QObject::connect(mLoadBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(loadBtnClicked()));
        QObject::connect(cancelBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(reject()));
        QObject::connect(mNextBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(nextBtnClicked()));
        QObject::connect(mXFileBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(selXFileBtnClicked()));
        QObject::connect(mRFileBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(selectRFile()));
        QObject::connect(mSFileBtn, SIGNAL(clicked()), TDataWizardForm, SLOT(selSFileBtnClicked()));

        QMetaObject::connectSlotsByName(TDataWizardForm);
    } // setupUi

    void retranslateUi(QDialog *TDataWizardForm)
    {
        TDataWizardForm->setWindowTitle(QApplication::translate("TDataWizardForm", "Data Input Wizard", 0, QApplication::UnicodeUTF8));
        mStepLabel->setText(QApplication::translate("TDataWizardForm", "Step %1", 0, QApplication::UnicodeUTF8));
        groupBox1->setTitle(QApplication::translate("TDataWizardForm", "SPS File Group", 0, QApplication::UnicodeUTF8));
        textLabel1->setText(QApplication::translate("TDataWizardForm", "Shot FileName(*.s)", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("TDataWizardForm", "Receive FileName(*.r)", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("TDataWizardForm", "Relationship FileName(*.x)", 0, QApplication::UnicodeUTF8));
        mSFileBtn->setText(QApplication::translate("TDataWizardForm", "...", 0, QApplication::UnicodeUTF8));
        mRFileBtn->setText(QApplication::translate("TDataWizardForm", "...", 0, QApplication::UnicodeUTF8));
        mXFileBtn->setText(QApplication::translate("TDataWizardForm", "...", 0, QApplication::UnicodeUTF8));
        mLoadBtn->setText(QApplication::translate("TDataWizardForm", "&Load", 0, QApplication::UnicodeUTF8));
        mLoadBtn->setShortcut(QApplication::translate("TDataWizardForm", "Alt+L", 0, QApplication::UnicodeUTF8));
        mNextBtn->setText(QApplication::translate("TDataWizardForm", "&Next", 0, QApplication::UnicodeUTF8));
        cancelBtn->setText(QApplication::translate("TDataWizardForm", "&Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TDataWizardForm: public Ui_TDataWizardForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TDATAWIZARDFORM_H
