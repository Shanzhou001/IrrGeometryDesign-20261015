/********************************************************************************
** Form generated from reading UI file 'trealovtdlg.ui'
**
** Created: Thu Oct 22 11:47:39 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TREALOVTDLG_H
#define UI_TREALOVTDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TRealOVTdlgClass
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *Geometry;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_21;
    QLineEdit *eInLineMaxOffset;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_30;
    QLineEdit *eBeginInOffset;
    QLabel *label_29;
    QLineEdit *eEndInOffset;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_23;
    QLineEdit *eCLineMaxOffset;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_26;
    QLineEdit *eBeginCLOffset;
    QLabel *label_25;
    QLineEdit *eEndCLOffset;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *OVT_PatchAttri;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *Cancel;

    void setupUi(QDialog *TRealOVTdlgClass)
    {
        if (TRealOVTdlgClass->objectName().isEmpty())
            TRealOVTdlgClass->setObjectName(QString::fromUtf8("TRealOVTdlgClass"));
        TRealOVTdlgClass->resize(408, 241);
        verticalLayout_2 = new QVBoxLayout(TRealOVTdlgClass);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        Geometry = new QGroupBox(TRealOVTdlgClass);
        Geometry->setObjectName(QString::fromUtf8("Geometry"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Geometry->sizePolicy().hasHeightForWidth());
        Geometry->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(Geometry);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_21 = new QLabel(Geometry);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        horizontalLayout_5->addWidget(label_21);

        eInLineMaxOffset = new QLineEdit(Geometry);
        eInLineMaxOffset->setObjectName(QString::fromUtf8("eInLineMaxOffset"));
        eInLineMaxOffset->setEnabled(false);

        horizontalLayout_5->addWidget(eInLineMaxOffset);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_30 = new QLabel(Geometry);
        label_30->setObjectName(QString::fromUtf8("label_30"));

        horizontalLayout_2->addWidget(label_30);

        eBeginInOffset = new QLineEdit(Geometry);
        eBeginInOffset->setObjectName(QString::fromUtf8("eBeginInOffset"));

        horizontalLayout_2->addWidget(eBeginInOffset);

        label_29 = new QLabel(Geometry);
        label_29->setObjectName(QString::fromUtf8("label_29"));

        horizontalLayout_2->addWidget(label_29);

        eEndInOffset = new QLineEdit(Geometry);
        eEndInOffset->setObjectName(QString::fromUtf8("eEndInOffset"));

        horizontalLayout_2->addWidget(eEndInOffset);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_23 = new QLabel(Geometry);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        horizontalLayout_4->addWidget(label_23);

        eCLineMaxOffset = new QLineEdit(Geometry);
        eCLineMaxOffset->setObjectName(QString::fromUtf8("eCLineMaxOffset"));
        eCLineMaxOffset->setEnabled(false);

        horizontalLayout_4->addWidget(eCLineMaxOffset);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_26 = new QLabel(Geometry);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        horizontalLayout_3->addWidget(label_26);

        eBeginCLOffset = new QLineEdit(Geometry);
        eBeginCLOffset->setObjectName(QString::fromUtf8("eBeginCLOffset"));

        horizontalLayout_3->addWidget(eBeginCLOffset);

        label_25 = new QLabel(Geometry);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        horizontalLayout_3->addWidget(label_25);

        eEndCLOffset = new QLineEdit(Geometry);
        eEndCLOffset->setObjectName(QString::fromUtf8("eEndCLOffset"));

        horizontalLayout_3->addWidget(eEndCLOffset);


        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_2->addWidget(Geometry);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        OVT_PatchAttri = new QPushButton(TRealOVTdlgClass);
        OVT_PatchAttri->setObjectName(QString::fromUtf8("OVT_PatchAttri"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(OVT_PatchAttri->sizePolicy().hasHeightForWidth());
        OVT_PatchAttri->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(OVT_PatchAttri);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);

        Cancel = new QPushButton(TRealOVTdlgClass);
        Cancel->setObjectName(QString::fromUtf8("Cancel"));
        sizePolicy1.setHeightForWidth(Cancel->sizePolicy().hasHeightForWidth());
        Cancel->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(Cancel);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(TRealOVTdlgClass);

        QMetaObject::connectSlotsByName(TRealOVTdlgClass);
    } // setupUi

    void retranslateUi(QDialog *TRealOVTdlgClass)
    {
        TRealOVTdlgClass->setWindowTitle(QApplication::translate("TRealOVTdlgClass", "TRealOVTdlg", 0, QApplication::UnicodeUTF8));
        Geometry->setTitle(QApplication::translate("TRealOVTdlgClass", "Parameter", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("TRealOVTdlgClass", "Inline Max Offset", 0, QApplication::UnicodeUTF8));
        label_30->setText(QApplication::translate("TRealOVTdlgClass", "Begin offset", 0, QApplication::UnicodeUTF8));
        label_29->setText(QApplication::translate("TRealOVTdlgClass", "--", 0, QApplication::UnicodeUTF8));
        label_23->setText(QApplication::translate("TRealOVTdlgClass", "Cline Max Offset", 0, QApplication::UnicodeUTF8));
        label_26->setText(QApplication::translate("TRealOVTdlgClass", "Begin offset", 0, QApplication::UnicodeUTF8));
        label_25->setText(QApplication::translate("TRealOVTdlgClass", "--", 0, QApplication::UnicodeUTF8));
        OVT_PatchAttri->setText(QApplication::translate("TRealOVTdlgClass", "Cal OVTPA", 0, QApplication::UnicodeUTF8));
        Cancel->setText(QApplication::translate("TRealOVTdlgClass", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TRealOVTdlgClass: public Ui_TRealOVTdlgClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TREALOVTDLG_H
