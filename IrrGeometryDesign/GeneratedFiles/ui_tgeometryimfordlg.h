/********************************************************************************
** Form generated from reading UI file 'tgeometryimfordlg.ui'
**
** Created: Thu Oct 22 11:47:39 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TGEOMETRYIMFORDLG_H
#define UI_TGEOMETRYIMFORDLG_H

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

class Ui_TGeometryImforDlgClass
{
public:
    QGroupBox *Geometry;
    QLineEdit *emaxYoffset;
    QLabel *label_21;
    QLineEdit *emaxXoffset;
    QLabel *label_20;
    QLabel *label_22;
    QLineEdit *emaxoffset;
    QLabel *label_23;
    QLineEdit *eXYratio;
    QLabel *label_24;
    QLineEdit *eManCiarea;
    QLineEdit *eAllarea;
    QLineEdit *eXLength;
    QLabel *label_25;
    QLineEdit *eManCiYlength;
    QLabel *label_26;
    QLineEdit *eYLength;
    QLabel *label_27;
    QLabel *label_28;
    QLabel *label_29;
    QLineEdit *eManCiXlength;
    QLabel *label_30;
    QLineEdit *eAllSPNum;
    QLabel *label_31;
    QLineEdit *eJZarea;
    QLabel *label_32;
    QLineEdit *eJZfold;
    QLabel *label_33;
    QLabel *label_34;
    QLabel *label_35;
    QLabel *label_36;
    QLabel *label_37;
    QLabel *label_38;
    QLabel *label_39;
    QLabel *label_40;
    QLabel *label_41;
    QLabel *label_42;
    QLabel *label_43;
    QLineEdit *eMaxFold;
    QPushButton *Cancel;

    void setupUi(QDialog *TGeometryImforDlgClass)
    {
        if (TGeometryImforDlgClass->objectName().isEmpty())
            TGeometryImforDlgClass->setObjectName(QString::fromUtf8("TGeometryImforDlgClass"));
        TGeometryImforDlgClass->resize(532, 350);
        Geometry = new QGroupBox(TGeometryImforDlgClass);
        Geometry->setObjectName(QString::fromUtf8("Geometry"));
        Geometry->setGeometry(QRect(10, 10, 511, 291));
        emaxYoffset = new QLineEdit(Geometry);
        emaxYoffset->setObjectName(QString::fromUtf8("emaxYoffset"));
        emaxYoffset->setEnabled(false);
        emaxYoffset->setGeometry(QRect(120, 60, 91, 20));
        label_21 = new QLabel(Geometry);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(20, 60, 91, 21));
        emaxXoffset = new QLineEdit(Geometry);
        emaxXoffset->setObjectName(QString::fromUtf8("emaxXoffset"));
        emaxXoffset->setEnabled(false);
        emaxXoffset->setGeometry(QRect(120, 20, 91, 20));
        label_20 = new QLabel(Geometry);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setGeometry(QRect(20, 20, 91, 21));
        label_22 = new QLabel(Geometry);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setGeometry(QRect(20, 100, 91, 21));
        emaxoffset = new QLineEdit(Geometry);
        emaxoffset->setObjectName(QString::fromUtf8("emaxoffset"));
        emaxoffset->setEnabled(false);
        emaxoffset->setGeometry(QRect(120, 100, 91, 20));
        label_23 = new QLabel(Geometry);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setGeometry(QRect(20, 140, 91, 21));
        eXYratio = new QLineEdit(Geometry);
        eXYratio->setObjectName(QString::fromUtf8("eXYratio"));
        eXYratio->setEnabled(false);
        eXYratio->setGeometry(QRect(120, 140, 91, 20));
        label_24 = new QLabel(Geometry);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setGeometry(QRect(260, 220, 101, 21));
        eManCiarea = new QLineEdit(Geometry);
        eManCiarea->setObjectName(QString::fromUtf8("eManCiarea"));
        eManCiarea->setEnabled(false);
        eManCiarea->setGeometry(QRect(380, 220, 91, 20));
        eAllarea = new QLineEdit(Geometry);
        eAllarea->setObjectName(QString::fromUtf8("eAllarea"));
        eAllarea->setEnabled(false);
        eAllarea->setGeometry(QRect(380, 100, 91, 20));
        eXLength = new QLineEdit(Geometry);
        eXLength->setObjectName(QString::fromUtf8("eXLength"));
        eXLength->setEnabled(false);
        eXLength->setGeometry(QRect(380, 20, 91, 20));
        label_25 = new QLabel(Geometry);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setGeometry(QRect(260, 180, 131, 21));
        eManCiYlength = new QLineEdit(Geometry);
        eManCiYlength->setObjectName(QString::fromUtf8("eManCiYlength"));
        eManCiYlength->setEnabled(false);
        eManCiYlength->setGeometry(QRect(380, 180, 91, 20));
        label_26 = new QLabel(Geometry);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setGeometry(QRect(260, 60, 131, 21));
        eYLength = new QLineEdit(Geometry);
        eYLength->setObjectName(QString::fromUtf8("eYLength"));
        eYLength->setEnabled(false);
        eYLength->setGeometry(QRect(380, 60, 91, 20));
        label_27 = new QLabel(Geometry);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setGeometry(QRect(260, 140, 131, 21));
        label_28 = new QLabel(Geometry);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        label_28->setGeometry(QRect(260, 20, 131, 21));
        label_29 = new QLabel(Geometry);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setGeometry(QRect(260, 100, 91, 21));
        eManCiXlength = new QLineEdit(Geometry);
        eManCiXlength->setObjectName(QString::fromUtf8("eManCiXlength"));
        eManCiXlength->setEnabled(false);
        eManCiXlength->setGeometry(QRect(380, 140, 91, 20));
        label_30 = new QLabel(Geometry);
        label_30->setObjectName(QString::fromUtf8("label_30"));
        label_30->setGeometry(QRect(20, 180, 101, 21));
        eAllSPNum = new QLineEdit(Geometry);
        eAllSPNum->setObjectName(QString::fromUtf8("eAllSPNum"));
        eAllSPNum->setEnabled(false);
        eAllSPNum->setGeometry(QRect(120, 180, 91, 20));
        label_31 = new QLabel(Geometry);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        label_31->setGeometry(QRect(20, 220, 101, 21));
        eJZarea = new QLineEdit(Geometry);
        eJZarea->setObjectName(QString::fromUtf8("eJZarea"));
        eJZarea->setEnabled(false);
        eJZarea->setGeometry(QRect(120, 220, 91, 20));
        label_32 = new QLabel(Geometry);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setGeometry(QRect(20, 260, 101, 21));
        eJZfold = new QLineEdit(Geometry);
        eJZfold->setObjectName(QString::fromUtf8("eJZfold"));
        eJZfold->setGeometry(QRect(120, 260, 91, 20));
        label_33 = new QLabel(Geometry);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        label_33->setGeometry(QRect(220, 20, 21, 21));
        label_34 = new QLabel(Geometry);
        label_34->setObjectName(QString::fromUtf8("label_34"));
        label_34->setGeometry(QRect(220, 60, 21, 21));
        label_35 = new QLabel(Geometry);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        label_35->setGeometry(QRect(220, 100, 21, 21));
        label_36 = new QLabel(Geometry);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        label_36->setGeometry(QRect(220, 220, 31, 21));
        label_37 = new QLabel(Geometry);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        label_37->setGeometry(QRect(480, 20, 21, 21));
        label_38 = new QLabel(Geometry);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        label_38->setGeometry(QRect(480, 60, 21, 21));
        label_39 = new QLabel(Geometry);
        label_39->setObjectName(QString::fromUtf8("label_39"));
        label_39->setGeometry(QRect(480, 140, 21, 21));
        label_40 = new QLabel(Geometry);
        label_40->setObjectName(QString::fromUtf8("label_40"));
        label_40->setGeometry(QRect(480, 180, 21, 21));
        label_41 = new QLabel(Geometry);
        label_41->setObjectName(QString::fromUtf8("label_41"));
        label_41->setGeometry(QRect(480, 100, 31, 21));
        label_42 = new QLabel(Geometry);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        label_42->setGeometry(QRect(480, 220, 31, 21));
        label_43 = new QLabel(Geometry);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setGeometry(QRect(260, 260, 101, 21));
        eMaxFold = new QLineEdit(Geometry);
        eMaxFold->setObjectName(QString::fromUtf8("eMaxFold"));
        eMaxFold->setEnabled(false);
        eMaxFold->setGeometry(QRect(380, 260, 91, 20));
        Cancel = new QPushButton(TGeometryImforDlgClass);
        Cancel->setObjectName(QString::fromUtf8("Cancel"));
        Cancel->setGeometry(QRect(290, 310, 141, 31));

        retranslateUi(TGeometryImforDlgClass);

        QMetaObject::connectSlotsByName(TGeometryImforDlgClass);
    } // setupUi

    void retranslateUi(QDialog *TGeometryImforDlgClass)
    {
        TGeometryImforDlgClass->setWindowTitle(QApplication::translate("TGeometryImforDlgClass", "TGeometryImforDlg", 0, QApplication::UnicodeUTF8));
        Geometry->setTitle(QApplication::translate("TGeometryImforDlgClass", "Geometry Imformation", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("TGeometryImforDlgClass", "Max Y offset", 0, QApplication::UnicodeUTF8));
        label_20->setText(QApplication::translate("TGeometryImforDlgClass", "Max X offset", 0, QApplication::UnicodeUTF8));
        label_22->setText(QApplication::translate("TGeometryImforDlgClass", "Max Offset", 0, QApplication::UnicodeUTF8));
        label_23->setText(QApplication::translate("TGeometryImforDlgClass", "XYratio", 0, QApplication::UnicodeUTF8));
        label_24->setText(QApplication::translate("TGeometryImforDlgClass", "ManCi Square", 0, QApplication::UnicodeUTF8));
        label_25->setText(QApplication::translate("TGeometryImforDlgClass", "ManCi CLlength", 0, QApplication::UnicodeUTF8));
        label_26->setText(QApplication::translate("TGeometryImforDlgClass", "Cline Length", 0, QApplication::UnicodeUTF8));
        label_27->setText(QApplication::translate("TGeometryImforDlgClass", "ManCi Inlength", 0, QApplication::UnicodeUTF8));
        label_28->setText(QApplication::translate("TGeometryImforDlgClass", "Inline Length", 0, QApplication::UnicodeUTF8));
        label_29->setText(QApplication::translate("TGeometryImforDlgClass", "All Square", 0, QApplication::UnicodeUTF8));
        label_30->setText(QApplication::translate("TGeometryImforDlgClass", "All SPoint Number", 0, QApplication::UnicodeUTF8));
        label_31->setText(QApplication::translate("TGeometryImforDlgClass", "JZ Square", 0, QApplication::UnicodeUTF8));
        label_32->setText(QApplication::translate("TGeometryImforDlgClass", "JZ Fold", 0, QApplication::UnicodeUTF8));
        label_33->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_34->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_35->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_36->setText(QApplication::translate("TGeometryImforDlgClass", "km2", 0, QApplication::UnicodeUTF8));
        label_37->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_38->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_39->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_40->setText(QApplication::translate("TGeometryImforDlgClass", "km", 0, QApplication::UnicodeUTF8));
        label_41->setText(QApplication::translate("TGeometryImforDlgClass", "km2", 0, QApplication::UnicodeUTF8));
        label_42->setText(QApplication::translate("TGeometryImforDlgClass", "km2", 0, QApplication::UnicodeUTF8));
        label_43->setText(QApplication::translate("TGeometryImforDlgClass", "Max Fold", 0, QApplication::UnicodeUTF8));
        Cancel->setText(QApplication::translate("TGeometryImforDlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TGeometryImforDlgClass: public Ui_TGeometryImforDlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TGEOMETRYIMFORDLG_H
