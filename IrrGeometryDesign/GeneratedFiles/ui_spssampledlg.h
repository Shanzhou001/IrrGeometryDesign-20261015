/********************************************************************************
** Form generated from reading UI file 'spssampledlg.ui'
**
** Created: Fri Oct 16 19:51:29 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPSSAMPLEDLG_H
#define UI_SPSSAMPLEDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SpsSampleDlgClass
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QComboBox *comboBox_SamplingMode;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLineEdit *lineEdit_TrSamplingRate;
    QLabel *label_3;
    QLineEdit *lineEdit_LiSamplingRate;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_OK;
    QPushButton *pushButton_Cancel;

    void setupUi(QDialog *SpsSampleDlgClass)
    {
        if (SpsSampleDlgClass->objectName().isEmpty())
            SpsSampleDlgClass->setObjectName(QString::fromUtf8("SpsSampleDlgClass"));
        SpsSampleDlgClass->resize(384, 178);
        verticalLayout = new QVBoxLayout(SpsSampleDlgClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(SpsSampleDlgClass);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        comboBox_SamplingMode = new QComboBox(SpsSampleDlgClass);
        comboBox_SamplingMode->setObjectName(QString::fromUtf8("comboBox_SamplingMode"));

        horizontalLayout_3->addWidget(comboBox_SamplingMode);


        verticalLayout->addLayout(horizontalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(SpsSampleDlgClass);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        lineEdit_TrSamplingRate = new QLineEdit(SpsSampleDlgClass);
        lineEdit_TrSamplingRate->setObjectName(QString::fromUtf8("lineEdit_TrSamplingRate"));

        gridLayout->addWidget(lineEdit_TrSamplingRate, 0, 1, 1, 1);

        label_3 = new QLabel(SpsSampleDlgClass);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        lineEdit_LiSamplingRate = new QLineEdit(SpsSampleDlgClass);
        lineEdit_LiSamplingRate->setObjectName(QString::fromUtf8("lineEdit_LiSamplingRate"));

        gridLayout->addWidget(lineEdit_LiSamplingRate, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton_OK = new QPushButton(SpsSampleDlgClass);
        pushButton_OK->setObjectName(QString::fromUtf8("pushButton_OK"));

        horizontalLayout->addWidget(pushButton_OK);

        pushButton_Cancel = new QPushButton(SpsSampleDlgClass);
        pushButton_Cancel->setObjectName(QString::fromUtf8("pushButton_Cancel"));

        horizontalLayout->addWidget(pushButton_Cancel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(SpsSampleDlgClass);

        QMetaObject::connectSlotsByName(SpsSampleDlgClass);
    } // setupUi

    void retranslateUi(QDialog *SpsSampleDlgClass)
    {
        SpsSampleDlgClass->setWindowTitle(QApplication::translate("SpsSampleDlgClass", "SpsSampleDlg", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SpsSampleDlgClass", "Sampling Mode", 0, QApplication::UnicodeUTF8));
        comboBox_SamplingMode->clear();
        comboBox_SamplingMode->insertItems(0, QStringList()
         << QApplication::translate("SpsSampleDlgClass", "Random", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SpsSampleDlgClass", "Jitter random", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SpsSampleDlgClass", "Segmented random", 0, QApplication::UnicodeUTF8)
        );
        label_2->setText(QApplication::translate("SpsSampleDlgClass", "Trace Sampling Rate(0-1)", 0, QApplication::UnicodeUTF8));
        lineEdit_TrSamplingRate->setText(QApplication::translate("SpsSampleDlgClass", "0.5", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("SpsSampleDlgClass", "Line Sampling Rate(0-1)", 0, QApplication::UnicodeUTF8));
        lineEdit_LiSamplingRate->setText(QApplication::translate("SpsSampleDlgClass", "1", 0, QApplication::UnicodeUTF8));
        pushButton_OK->setText(QApplication::translate("SpsSampleDlgClass", "OK", 0, QApplication::UnicodeUTF8));
        pushButton_Cancel->setText(QApplication::translate("SpsSampleDlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SpsSampleDlgClass: public Ui_SpsSampleDlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPSSAMPLEDLG_H
