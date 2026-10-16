/********************************************************************************
** Form generated from reading UI file 'tchangeshowareadlg.ui'
**
** Created: Fri Oct 16 19:51:42 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCHANGESHOWAREADLG_H
#define UI_TCHANGESHOWAREADLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_TChangeShowareadlgClass
{
public:
    QGroupBox *Basic_parameter_3;
    QLabel *label_12;
    QLineEdit *MaxShowValue;
    QLabel *label_13;
    QLineEdit *MinShowValue;
    QPushButton *Cancel;
    QPushButton *OK;

    void setupUi(QDialog *TChangeShowareadlgClass)
    {
        if (TChangeShowareadlgClass->objectName().isEmpty())
            TChangeShowareadlgClass->setObjectName(QString::fromUtf8("TChangeShowareadlgClass"));
        TChangeShowareadlgClass->resize(272, 152);
        Basic_parameter_3 = new QGroupBox(TChangeShowareadlgClass);
        Basic_parameter_3->setObjectName(QString::fromUtf8("Basic_parameter_3"));
        Basic_parameter_3->setGeometry(QRect(10, 10, 251, 101));
        label_12 = new QLabel(Basic_parameter_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(10, 29, 120, 21));
        MaxShowValue = new QLineEdit(Basic_parameter_3);
        MaxShowValue->setObjectName(QString::fromUtf8("MaxShowValue"));
        MaxShowValue->setGeometry(QRect(136, 29, 105, 21));
        label_13 = new QLabel(Basic_parameter_3);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(10, 62, 120, 21));
        MinShowValue = new QLineEdit(Basic_parameter_3);
        MinShowValue->setObjectName(QString::fromUtf8("MinShowValue"));
        MinShowValue->setGeometry(QRect(136, 62, 105, 21));
        Cancel = new QPushButton(TChangeShowareadlgClass);
        Cancel->setObjectName(QString::fromUtf8("Cancel"));
        Cancel->setGeometry(QRect(160, 120, 75, 23));
        OK = new QPushButton(TChangeShowareadlgClass);
        OK->setObjectName(QString::fromUtf8("OK"));
        OK->setGeometry(QRect(50, 120, 75, 23));

        retranslateUi(TChangeShowareadlgClass);

        QMetaObject::connectSlotsByName(TChangeShowareadlgClass);
    } // setupUi

    void retranslateUi(QDialog *TChangeShowareadlgClass)
    {
        TChangeShowareadlgClass->setWindowTitle(QApplication::translate("TChangeShowareadlgClass", "TChangeShowareadlg", 0, QApplication::UnicodeUTF8));
        Basic_parameter_3->setTitle(QApplication::translate("TChangeShowareadlgClass", "Show area parameter", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("TChangeShowareadlgClass", "MaxShow Value", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("TChangeShowareadlgClass", "MinShow Value", 0, QApplication::UnicodeUTF8));
        Cancel->setText(QApplication::translate("TChangeShowareadlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
        OK->setText(QApplication::translate("TChangeShowareadlgClass", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TChangeShowareadlgClass: public Ui_TChangeShowareadlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCHANGESHOWAREADLG_H
