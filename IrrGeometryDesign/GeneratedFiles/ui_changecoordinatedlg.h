/********************************************************************************
** Form generated from reading UI file 'changecoordinatedlg.ui'
**
** Created: Wed Oct 14 11:55:39 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHANGECOORDINATEDLG_H
#define UI_CHANGECOORDINATEDLG_H

#include <Qt3Support/Q3GroupBox>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_ChangecoordinatedlgClass
{
public:
    QPushButton *ChangCoordinateOK;
    QPushButton *ChangCoordinateCancel;
    Q3GroupBox *groupBox1;
    QLabel *textLabel1;
    QLineEdit *CoordinateMax;

    void setupUi(QDialog *ChangecoordinatedlgClass)
    {
        if (ChangecoordinatedlgClass->objectName().isEmpty())
            ChangecoordinatedlgClass->setObjectName(QString::fromUtf8("ChangecoordinatedlgClass"));
        ChangecoordinatedlgClass->resize(199, 100);
        ChangCoordinateOK = new QPushButton(ChangecoordinatedlgClass);
        ChangCoordinateOK->setObjectName(QString::fromUtf8("ChangCoordinateOK"));
        ChangCoordinateOK->setGeometry(QRect(10, 70, 82, 25));
        ChangCoordinateCancel = new QPushButton(ChangecoordinatedlgClass);
        ChangCoordinateCancel->setObjectName(QString::fromUtf8("ChangCoordinateCancel"));
        ChangCoordinateCancel->setGeometry(QRect(100, 70, 82, 25));
        groupBox1 = new Q3GroupBox(ChangecoordinatedlgClass);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        groupBox1->setGeometry(QRect(10, 10, 181, 51));
        textLabel1 = new QLabel(groupBox1);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
        textLabel1->setGeometry(QRect(10, 20, 70, 20));
        textLabel1->setWordWrap(false);
        CoordinateMax = new QLineEdit(groupBox1);
        CoordinateMax->setObjectName(QString::fromUtf8("CoordinateMax"));
        CoordinateMax->setGeometry(QRect(90, 20, 81, 21));

        retranslateUi(ChangecoordinatedlgClass);

        QMetaObject::connectSlotsByName(ChangecoordinatedlgClass);
    } // setupUi

    void retranslateUi(QDialog *ChangecoordinatedlgClass)
    {
        ChangecoordinatedlgClass->setWindowTitle(QApplication::translate("ChangecoordinatedlgClass", "Changecoordinatedlg", 0, QApplication::UnicodeUTF8));
        ChangCoordinateOK->setText(QApplication::translate("ChangecoordinatedlgClass", "OK", 0, QApplication::UnicodeUTF8));
        ChangCoordinateCancel->setText(QApplication::translate("ChangecoordinatedlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
        groupBox1->setTitle(QApplication::translate("ChangecoordinatedlgClass", "Change coordinate", 0, QApplication::UnicodeUTF8));
        textLabel1->setText(QApplication::translate("ChangecoordinatedlgClass", "Max value", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ChangecoordinatedlgClass: public Ui_ChangecoordinatedlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHANGECOORDINATEDLG_H
