/********************************************************************************
** Form generated from reading UI file 'tqfdividedlg.ui'
**
** Created: Wed Oct 14 11:55:38 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TQFDIVIDEDLG_H
#define UI_TQFDIVIDEDLG_H

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

class Ui_TQFDividedlgClass
{
public:
    QPushButton *OK;
    QPushButton *Cancel;
    QGroupBox *Basic_parameter_3;
    QLabel *label_12;
    QLineEdit *OffsetDivNumber;
    QLabel *label_13;
    QLineEdit *AngleDivNumber;
    QLabel *label_14;
    QLineEdit *MaxOffset;

    void setupUi(QDialog *TQFDividedlgClass)
    {
        if (TQFDividedlgClass->objectName().isEmpty())
            TQFDividedlgClass->setObjectName(QString::fromUtf8("TQFDividedlgClass"));
        TQFDividedlgClass->resize(282, 190);
        OK = new QPushButton(TQFDividedlgClass);
        OK->setObjectName(QString::fromUtf8("OK"));
        OK->setGeometry(QRect(40, 160, 75, 23));
        Cancel = new QPushButton(TQFDividedlgClass);
        Cancel->setObjectName(QString::fromUtf8("Cancel"));
        Cancel->setGeometry(QRect(150, 160, 75, 23));
        Basic_parameter_3 = new QGroupBox(TQFDividedlgClass);
        Basic_parameter_3->setObjectName(QString::fromUtf8("Basic_parameter_3"));
        Basic_parameter_3->setGeometry(QRect(10, 10, 251, 141));
        label_12 = new QLabel(Basic_parameter_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(10, 29, 120, 21));
        OffsetDivNumber = new QLineEdit(Basic_parameter_3);
        OffsetDivNumber->setObjectName(QString::fromUtf8("OffsetDivNumber"));
        OffsetDivNumber->setGeometry(QRect(136, 29, 105, 21));
        label_13 = new QLabel(Basic_parameter_3);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(10, 62, 120, 21));
        AngleDivNumber = new QLineEdit(Basic_parameter_3);
        AngleDivNumber->setObjectName(QString::fromUtf8("AngleDivNumber"));
        AngleDivNumber->setGeometry(QRect(136, 62, 105, 21));
        label_14 = new QLabel(Basic_parameter_3);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(10, 100, 120, 21));
        MaxOffset = new QLineEdit(Basic_parameter_3);
        MaxOffset->setObjectName(QString::fromUtf8("MaxOffset"));
        MaxOffset->setGeometry(QRect(136, 100, 105, 21));

        retranslateUi(TQFDividedlgClass);

        QMetaObject::connectSlotsByName(TQFDividedlgClass);
    } // setupUi

    void retranslateUi(QDialog *TQFDividedlgClass)
    {
        TQFDividedlgClass->setWindowTitle(QApplication::translate("TQFDividedlgClass", "TQFDividedlg", 0, QApplication::UnicodeUTF8));
        OK->setText(QApplication::translate("TQFDividedlgClass", "OK", 0, QApplication::UnicodeUTF8));
        Cancel->setText(QApplication::translate("TQFDividedlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
        Basic_parameter_3->setTitle(QApplication::translate("TQFDividedlgClass", "Divide parameter", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("TQFDividedlgClass", "Offset divide number", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("TQFDividedlgClass", "Angle divide number", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("TQFDividedlgClass", "Max Offset", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TQFDividedlgClass: public Ui_TQFDividedlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TQFDIVIDEDLG_H
