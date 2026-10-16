/********************************************************************************
** Form generated from reading UI file 'trosedividedlg.ui'
**
** Created: Fri Oct 16 19:51:39 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TROSEDIVIDEDLG_H
#define UI_TROSEDIVIDEDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_TRoseDividedlgClass
{
public:
    QGroupBox *Basic_parameter_3;
    QGridLayout *gridLayout_3;
    QLabel *label_12;
    QLineEdit *OffsetDivNumber;
    QLabel *label_13;
    QLineEdit *AngleDivNumber;
    QPushButton *OK;
    QPushButton *Cancel;

    void setupUi(QDialog *TRoseDividedlgClass)
    {
        if (TRoseDividedlgClass->objectName().isEmpty())
            TRoseDividedlgClass->setObjectName(QString::fromUtf8("TRoseDividedlgClass"));
        TRoseDividedlgClass->resize(274, 151);
        Basic_parameter_3 = new QGroupBox(TRoseDividedlgClass);
        Basic_parameter_3->setObjectName(QString::fromUtf8("Basic_parameter_3"));
        Basic_parameter_3->setGeometry(QRect(10, 10, 251, 101));
        gridLayout_3 = new QGridLayout(Basic_parameter_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_12 = new QLabel(Basic_parameter_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_3->addWidget(label_12, 0, 0, 1, 1);

        OffsetDivNumber = new QLineEdit(Basic_parameter_3);
        OffsetDivNumber->setObjectName(QString::fromUtf8("OffsetDivNumber"));

        gridLayout_3->addWidget(OffsetDivNumber, 0, 1, 1, 1);

        label_13 = new QLabel(Basic_parameter_3);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_3->addWidget(label_13, 1, 0, 1, 1);

        AngleDivNumber = new QLineEdit(Basic_parameter_3);
        AngleDivNumber->setObjectName(QString::fromUtf8("AngleDivNumber"));
        AngleDivNumber->setReadOnly(true);

        gridLayout_3->addWidget(AngleDivNumber, 1, 1, 1, 1);

        OK = new QPushButton(TRoseDividedlgClass);
        OK->setObjectName(QString::fromUtf8("OK"));
        OK->setGeometry(QRect(40, 120, 75, 23));
        Cancel = new QPushButton(TRoseDividedlgClass);
        Cancel->setObjectName(QString::fromUtf8("Cancel"));
        Cancel->setGeometry(QRect(150, 120, 75, 23));

        retranslateUi(TRoseDividedlgClass);

        QMetaObject::connectSlotsByName(TRoseDividedlgClass);
    } // setupUi

    void retranslateUi(QDialog *TRoseDividedlgClass)
    {
        TRoseDividedlgClass->setWindowTitle(QApplication::translate("TRoseDividedlgClass", "TRoseDividedlg", 0, QApplication::UnicodeUTF8));
        Basic_parameter_3->setTitle(QApplication::translate("TRoseDividedlgClass", "Divide parameter", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("TRoseDividedlgClass", "Offset divide number", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("TRoseDividedlgClass", "Angle divide number", 0, QApplication::UnicodeUTF8));
        OK->setText(QApplication::translate("TRoseDividedlgClass", "OK", 0, QApplication::UnicodeUTF8));
        Cancel->setText(QApplication::translate("TRoseDividedlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TRoseDividedlgClass: public Ui_TRoseDividedlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TROSEDIVIDEDLG_H
