/********************************************************************************
** Form generated from reading UI file 'tspsdlg.ui'
**
** Created: Wed Oct 14 11:55:38 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TSPSDLG_H
#define UI_TSPSDLG_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>
#include "gspsparam.h"

QT_BEGIN_NAMESPACE

class Ui_TSPSDlg
{
public:
    QLabel *textLabel6_3;
    QLineEdit *cSPtInvEdit;
    QLineEdit *cSY0Edit;
    QLabel *textLabel2_3;
    QLabel *textLabel1_2;
    QLineEdit *cLMinOffEdit;
    QLabel *textLabel1_3_2;
    QLabel *textLabel5_2;
    QSpinBox *cRLineCountBox;
    QLineEdit *cRX0Edit;
    QLabel *textLabel3_2;
    QComboBox *cATypeBox;
    QLineEdit *cSLineInvEdit;
    QSpinBox *cRPtCountBox;
    QLabel *textLabel3_3;
    QLineEdit *cSX0Edit;
    QLineEdit *cRMinOffEdit;
    QLabel *textLabel6_2;
    QLabel *textLabel6_2_2;
    QLineEdit *cRMaxOffEdit;
    QLabel *textLabel6;
    QLabel *textLabel1;
    QSpinBox *cSPtCountBox;
    QLineEdit *cSAngleEdit;
    QLineEdit *cLMaxOffEdit;
    QSpinBox *cSLineCountBox;
    QLabel *textLabel4;
    QLabel *textLabel2;
    QLabel *textLabel4_2;
    QLabel *textLabel2_2;
    QFrame *line1;
    QFrame *line2;
    QLabel *textLabel1_4;
    QLineEdit *cAngleEdit;
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacer9;
    QPushButton *cOkBtn;
    QPushButton *cCancelBtn;
    QFrame *line1_2;
    QLabel *textLabel1_3_3;
    QLabel *textLabel1_3;
    QLabel *textLabel3;
    QLabel *textLabel5;
    QLineEdit *cRLineInvEdit;
    QLineEdit *cRY0Edit;
    QLineEdit *cRPtInvEdit;
    QLabel *label_2;
    QLabel *label;
    QLineEdit *cMoveX0Edit;
    QLineEdit *cMoveY0Edit;
    QRadioButton *OrginalByReceiver;
    QRadioButton *OrginalByShot;

    void setupUi(QDialog *TSPSDlg)
    {
        if (TSPSDlg->objectName().isEmpty())
            TSPSDlg->setObjectName(QString::fromUtf8("TSPSDlg"));
        TSPSDlg->resize(527, 400);
        textLabel6_3 = new QLabel(TSPSDlg);
        textLabel6_3->setObjectName(QString::fromUtf8("textLabel6_3"));
        textLabel6_3->setGeometry(QRect(9, 233, 84, 21));
        textLabel6_3->setWordWrap(false);
        cSPtInvEdit = new QLineEdit(TSPSDlg);
        cSPtInvEdit->setObjectName(QString::fromUtf8("cSPtInvEdit"));
        cSPtInvEdit->setGeometry(QRect(351, 179, 167, 21));
        cSY0Edit = new QLineEdit(TSPSDlg);
        cSY0Edit->setObjectName(QString::fromUtf8("cSY0Edit"));
        cSY0Edit->setGeometry(QRect(351, 152, 167, 21));
        textLabel2_3 = new QLabel(TSPSDlg);
        textLabel2_3->setObjectName(QString::fromUtf8("textLabel2_3"));
        textLabel2_3->setGeometry(QRect(9, 260, 84, 21));
        textLabel2_3->setWordWrap(false);
        textLabel1_2 = new QLabel(TSPSDlg);
        textLabel1_2->setObjectName(QString::fromUtf8("textLabel1_2"));
        textLabel1_2->setGeometry(QRect(9, 125, 84, 21));
        textLabel1_2->setWordWrap(false);
        cLMinOffEdit = new QLineEdit(TSPSDlg);
        cLMinOffEdit->setObjectName(QString::fromUtf8("cLMinOffEdit"));
        cLMinOffEdit->setGeometry(QRect(99, 206, 168, 21));
        textLabel1_3_2 = new QLabel(TSPSDlg);
        textLabel1_3_2->setObjectName(QString::fromUtf8("textLabel1_3_2"));
        textLabel1_3_2->setGeometry(QRect(273, 125, 72, 21));
        textLabel1_3_2->setWordWrap(false);
        textLabel5_2 = new QLabel(TSPSDlg);
        textLabel5_2->setObjectName(QString::fromUtf8("textLabel5_2"));
        textLabel5_2->setGeometry(QRect(273, 179, 72, 21));
        textLabel5_2->setWordWrap(false);
        cRLineCountBox = new QSpinBox(TSPSDlg);
        cRLineCountBox->setObjectName(QString::fromUtf8("cRLineCountBox"));
        cRLineCountBox->setGeometry(QRect(99, 9, 168, 21));
        cRLineCountBox->setMaximum(10000);
        cRX0Edit = new QLineEdit(TSPSDlg);
        cRX0Edit->setObjectName(QString::fromUtf8("cRX0Edit"));
        cRX0Edit->setEnabled(true);
        cRX0Edit->setGeometry(QRect(99, 36, 168, 21));
        textLabel3_2 = new QLabel(TSPSDlg);
        textLabel3_2->setObjectName(QString::fromUtf8("textLabel3_2"));
        textLabel3_2->setGeometry(QRect(273, 152, 72, 21));
        textLabel3_2->setWordWrap(false);
        cATypeBox = new QComboBox(TSPSDlg);
        cATypeBox->setObjectName(QString::fromUtf8("cATypeBox"));
        cATypeBox->setGeometry(QRect(99, 260, 168, 21));
        cSLineInvEdit = new QLineEdit(TSPSDlg);
        cSLineInvEdit->setObjectName(QString::fromUtf8("cSLineInvEdit"));
        cSLineInvEdit->setGeometry(QRect(351, 125, 167, 21));
        cRPtCountBox = new QSpinBox(TSPSDlg);
        cRPtCountBox->setObjectName(QString::fromUtf8("cRPtCountBox"));
        cRPtCountBox->setGeometry(QRect(99, 63, 168, 21));
        cRPtCountBox->setMaximum(10000000);
        textLabel3_3 = new QLabel(TSPSDlg);
        textLabel3_3->setObjectName(QString::fromUtf8("textLabel3_3"));
        textLabel3_3->setGeometry(QRect(273, 260, 72, 21));
        textLabel3_3->setWordWrap(false);
        cSX0Edit = new QLineEdit(TSPSDlg);
        cSX0Edit->setObjectName(QString::fromUtf8("cSX0Edit"));
        cSX0Edit->setGeometry(QRect(99, 152, 168, 21));
        cRMinOffEdit = new QLineEdit(TSPSDlg);
        cRMinOffEdit->setObjectName(QString::fromUtf8("cRMinOffEdit"));
        cRMinOffEdit->setGeometry(QRect(99, 233, 168, 21));
        textLabel6_2 = new QLabel(TSPSDlg);
        textLabel6_2->setObjectName(QString::fromUtf8("textLabel6_2"));
        textLabel6_2->setGeometry(QRect(273, 206, 72, 21));
        textLabel6_2->setWordWrap(false);
        textLabel6_2_2 = new QLabel(TSPSDlg);
        textLabel6_2_2->setObjectName(QString::fromUtf8("textLabel6_2_2"));
        textLabel6_2_2->setGeometry(QRect(273, 233, 72, 21));
        textLabel6_2_2->setWordWrap(false);
        cRMaxOffEdit = new QLineEdit(TSPSDlg);
        cRMaxOffEdit->setObjectName(QString::fromUtf8("cRMaxOffEdit"));
        cRMaxOffEdit->setGeometry(QRect(351, 233, 167, 21));
        textLabel6 = new QLabel(TSPSDlg);
        textLabel6->setObjectName(QString::fromUtf8("textLabel6"));
        textLabel6->setGeometry(QRect(9, 206, 84, 21));
        textLabel6->setWordWrap(false);
        textLabel1 = new QLabel(TSPSDlg);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
        textLabel1->setGeometry(QRect(9, 9, 84, 21));
        textLabel1->setWordWrap(false);
        cSPtCountBox = new QSpinBox(TSPSDlg);
        cSPtCountBox->setObjectName(QString::fromUtf8("cSPtCountBox"));
        cSPtCountBox->setGeometry(QRect(99, 179, 168, 21));
        cSPtCountBox->setMaximum(10000000);
        cSAngleEdit = new QLineEdit(TSPSDlg);
        cSAngleEdit->setObjectName(QString::fromUtf8("cSAngleEdit"));
        cSAngleEdit->setEnabled(false);
        cSAngleEdit->setGeometry(QRect(351, 260, 167, 21));
        cLMaxOffEdit = new QLineEdit(TSPSDlg);
        cLMaxOffEdit->setObjectName(QString::fromUtf8("cLMaxOffEdit"));
        cLMaxOffEdit->setGeometry(QRect(351, 206, 167, 21));
        cSLineCountBox = new QSpinBox(TSPSDlg);
        cSLineCountBox->setObjectName(QString::fromUtf8("cSLineCountBox"));
        cSLineCountBox->setGeometry(QRect(99, 125, 168, 21));
        cSLineCountBox->setMaximum(10000);
        textLabel4 = new QLabel(TSPSDlg);
        textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
        textLabel4->setGeometry(QRect(9, 63, 84, 21));
        textLabel4->setWordWrap(false);
        textLabel2 = new QLabel(TSPSDlg);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        textLabel2->setGeometry(QRect(9, 36, 84, 21));
        textLabel2->setWordWrap(false);
        textLabel4_2 = new QLabel(TSPSDlg);
        textLabel4_2->setObjectName(QString::fromUtf8("textLabel4_2"));
        textLabel4_2->setGeometry(QRect(9, 179, 84, 21));
        textLabel4_2->setWordWrap(false);
        textLabel2_2 = new QLabel(TSPSDlg);
        textLabel2_2->setObjectName(QString::fromUtf8("textLabel2_2"));
        textLabel2_2->setGeometry(QRect(9, 152, 84, 21));
        textLabel2_2->setWordWrap(false);
        line1 = new QFrame(TSPSDlg);
        line1->setObjectName(QString::fromUtf8("line1"));
        line1->setGeometry(QRect(9, 350, 509, 16));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);
        line2 = new QFrame(TSPSDlg);
        line2->setObjectName(QString::fromUtf8("line2"));
        line2->setGeometry(QRect(9, 280, 509, 16));
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        textLabel1_4 = new QLabel(TSPSDlg);
        textLabel1_4->setObjectName(QString::fromUtf8("textLabel1_4"));
        textLabel1_4->setGeometry(QRect(9, 90, 84, 21));
        textLabel1_4->setWordWrap(false);
        cAngleEdit = new QLineEdit(TSPSDlg);
        cAngleEdit->setObjectName(QString::fromUtf8("cAngleEdit"));
        cAngleEdit->setEnabled(false);
        cAngleEdit->setGeometry(QRect(99, 90, 168, 21));
        layoutWidget = new QWidget(TSPSDlg);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(9, 366, 509, 27));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setContentsMargins(11, 11, 11, 11);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacer9 = new QSpacerItem(41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacer9);

        cOkBtn = new QPushButton(layoutWidget);
        cOkBtn->setObjectName(QString::fromUtf8("cOkBtn"));

        hboxLayout->addWidget(cOkBtn);

        cCancelBtn = new QPushButton(layoutWidget);
        cCancelBtn->setObjectName(QString::fromUtf8("cCancelBtn"));

        hboxLayout->addWidget(cCancelBtn);

        line1_2 = new QFrame(TSPSDlg);
        line1_2->setObjectName(QString::fromUtf8("line1_2"));
        line1_2->setGeometry(QRect(10, 110, 509, 16));
        line1_2->setFrameShape(QFrame::HLine);
        line1_2->setFrameShadow(QFrame::Sunken);
        textLabel1_3_3 = new QLabel(TSPSDlg);
        textLabel1_3_3->setObjectName(QString::fromUtf8("textLabel1_3_3"));
        textLabel1_3_3->setGeometry(QRect(20, 310, 111, 21));
        textLabel1_3_3->setWordWrap(false);
        textLabel1_3 = new QLabel(TSPSDlg);
        textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
        textLabel1_3->setGeometry(QRect(271, 11, 69, 21));
        textLabel1_3->setWordWrap(false);
        textLabel3 = new QLabel(TSPSDlg);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        textLabel3->setGeometry(QRect(271, 38, 69, 21));
        textLabel3->setWordWrap(false);
        textLabel5 = new QLabel(TSPSDlg);
        textLabel5->setObjectName(QString::fromUtf8("textLabel5"));
        textLabel5->setGeometry(QRect(271, 65, 69, 21));
        textLabel5->setWordWrap(false);
        cRLineInvEdit = new QLineEdit(TSPSDlg);
        cRLineInvEdit->setObjectName(QString::fromUtf8("cRLineInvEdit"));
        cRLineInvEdit->setGeometry(QRect(350, 10, 169, 21));
        cRY0Edit = new QLineEdit(TSPSDlg);
        cRY0Edit->setObjectName(QString::fromUtf8("cRY0Edit"));
        cRY0Edit->setEnabled(true);
        cRY0Edit->setGeometry(QRect(350, 37, 169, 21));
        cRPtInvEdit = new QLineEdit(TSPSDlg);
        cRPtInvEdit->setObjectName(QString::fromUtf8("cRPtInvEdit"));
        cRPtInvEdit->setGeometry(QRect(350, 64, 169, 21));
        label_2 = new QLabel(TSPSDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(280, 300, 61, 21));
        label = new QLabel(TSPSDlg);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(280, 330, 61, 21));
        cMoveX0Edit = new QLineEdit(TSPSDlg);
        cMoveX0Edit->setObjectName(QString::fromUtf8("cMoveX0Edit"));
        cMoveX0Edit->setEnabled(false);
        cMoveX0Edit->setGeometry(QRect(349, 300, 171, 21));
        cMoveY0Edit = new QLineEdit(TSPSDlg);
        cMoveY0Edit->setObjectName(QString::fromUtf8("cMoveY0Edit"));
        cMoveY0Edit->setEnabled(false);
        cMoveY0Edit->setGeometry(QRect(350, 330, 171, 21));
        OrginalByReceiver = new QRadioButton(TSPSDlg);
        OrginalByReceiver->setObjectName(QString::fromUtf8("OrginalByReceiver"));
        OrginalByReceiver->setEnabled(false);
        OrginalByReceiver->setGeometry(QRect(140, 300, 101, 17));
        OrginalByReceiver->setCheckable(true);
        OrginalByReceiver->setChecked(true);
        OrginalByShot = new QRadioButton(TSPSDlg);
        OrginalByShot->setObjectName(QString::fromUtf8("OrginalByShot"));
        OrginalByShot->setEnabled(false);
        OrginalByShot->setGeometry(QRect(140, 330, 111, 17));
        OrginalByShot->setChecked(false);
        QWidget::setTabOrder(cRLineCountBox, cRLineInvEdit);
        QWidget::setTabOrder(cRLineInvEdit, cRX0Edit);
        QWidget::setTabOrder(cRX0Edit, cRY0Edit);
        QWidget::setTabOrder(cRY0Edit, cRPtCountBox);
        QWidget::setTabOrder(cRPtCountBox, cRPtInvEdit);
        QWidget::setTabOrder(cRPtInvEdit, cAngleEdit);
        QWidget::setTabOrder(cAngleEdit, cSLineCountBox);
        QWidget::setTabOrder(cSLineCountBox, cSLineInvEdit);
        QWidget::setTabOrder(cSLineInvEdit, cSX0Edit);
        QWidget::setTabOrder(cSX0Edit, cSY0Edit);
        QWidget::setTabOrder(cSY0Edit, cSPtCountBox);
        QWidget::setTabOrder(cSPtCountBox, cSPtInvEdit);
        QWidget::setTabOrder(cSPtInvEdit, cLMinOffEdit);
        QWidget::setTabOrder(cLMinOffEdit, cLMaxOffEdit);
        QWidget::setTabOrder(cLMaxOffEdit, cRMinOffEdit);
        QWidget::setTabOrder(cRMinOffEdit, cRMaxOffEdit);
        QWidget::setTabOrder(cRMaxOffEdit, cATypeBox);
        QWidget::setTabOrder(cATypeBox, cSAngleEdit);
        QWidget::setTabOrder(cSAngleEdit, cOkBtn);
        QWidget::setTabOrder(cOkBtn, cCancelBtn);

        retranslateUi(TSPSDlg);
        QObject::connect(cOkBtn, SIGNAL(clicked()), TSPSDlg, SLOT(accept()));
        QObject::connect(cCancelBtn, SIGNAL(clicked()), TSPSDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(TSPSDlg);
    } // setupUi

    void retranslateUi(QDialog *TSPSDlg)
    {
        TSPSDlg->setWindowTitle(QApplication::translate("TSPSDlg", "New SPS", 0, QApplication::UnicodeUTF8));
        textLabel6_3->setText(QApplication::translate("TSPSDlg", "R Min Offset", 0, QApplication::UnicodeUTF8));
        textLabel2_3->setText(QApplication::translate("TSPSDlg", "Array Type", 0, QApplication::UnicodeUTF8));
        textLabel1_2->setText(QApplication::translate("TSPSDlg", "SLine Count", 0, QApplication::UnicodeUTF8));
        textLabel1_3_2->setText(QApplication::translate("TSPSDlg", "Line Inv", 0, QApplication::UnicodeUTF8));
        textLabel5_2->setText(QApplication::translate("TSPSDlg", "SPt Inv", 0, QApplication::UnicodeUTF8));
        textLabel3_2->setText(QApplication::translate("TSPSDlg", "Y0", 0, QApplication::UnicodeUTF8));
        cATypeBox->clear();
        cATypeBox->insertItems(0, QStringList()
         << QApplication::translate("TSPSDlg", "In Line", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TSPSDlg", "Cross Line", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TSPSDlg", "Ob-Cross Line", 0, QApplication::UnicodeUTF8)
        );
        textLabel3_3->setText(QApplication::translate("TSPSDlg", "S Angle", 0, QApplication::UnicodeUTF8));
        textLabel6_2->setText(QApplication::translate("TSPSDlg", "L Max Offset", 0, QApplication::UnicodeUTF8));
        textLabel6_2_2->setText(QApplication::translate("TSPSDlg", "R Max Offset", 0, QApplication::UnicodeUTF8));
        textLabel6->setText(QApplication::translate("TSPSDlg", "L Min Offset", 0, QApplication::UnicodeUTF8));
        textLabel1->setText(QApplication::translate("TSPSDlg", "RLine Count", 0, QApplication::UnicodeUTF8));
        cSAngleEdit->setText(QApplication::translate("TSPSDlg", "0", 0, QApplication::UnicodeUTF8));
        textLabel4->setText(QApplication::translate("TSPSDlg", "RPt Count", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("TSPSDlg", "First RLine X0", 0, QApplication::UnicodeUTF8));
        textLabel4_2->setText(QApplication::translate("TSPSDlg", "SPt Count", 0, QApplication::UnicodeUTF8));
        textLabel2_2->setText(QApplication::translate("TSPSDlg", "First SLine X0", 0, QApplication::UnicodeUTF8));
        textLabel1_4->setText(QApplication::translate("TSPSDlg", "R Angle", 0, QApplication::UnicodeUTF8));
        cAngleEdit->setText(QApplication::translate("TSPSDlg", "0", 0, QApplication::UnicodeUTF8));
        cOkBtn->setText(QApplication::translate("TSPSDlg", "&Ok", 0, QApplication::UnicodeUTF8));
        cOkBtn->setShortcut(QApplication::translate("TSPSDlg", "Alt+O", 0, QApplication::UnicodeUTF8));
        cCancelBtn->setText(QApplication::translate("TSPSDlg", "&Cancel", 0, QApplication::UnicodeUTF8));
        cCancelBtn->setShortcut(QApplication::translate("TSPSDlg", "Alt+C", 0, QApplication::UnicodeUTF8));
        textLabel1_3_3->setText(QApplication::translate("TSPSDlg", "Design Original", 0, QApplication::UnicodeUTF8));
        textLabel1_3->setText(QApplication::translate("TSPSDlg", "Line Inv", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("TSPSDlg", "Y0", 0, QApplication::UnicodeUTF8));
        textLabel5->setText(QApplication::translate("TSPSDlg", "RPt Inv", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TSPSDlg", "Move X0", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TSPSDlg", "Move Y0", 0, QApplication::UnicodeUTF8));
        OrginalByReceiver->setText(QApplication::translate("TSPSDlg", "By Receiver", 0, QApplication::UnicodeUTF8));
        OrginalByShot->setText(QApplication::translate("TSPSDlg", "By Shot", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TSPSDlg: public Ui_TSPSDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TSPSDLG_H
