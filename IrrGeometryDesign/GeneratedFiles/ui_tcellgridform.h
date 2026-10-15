/********************************************************************************
** Form generated from reading UI file 'tcellgridform.ui'
**
** Created: Wed Oct 14 11:55:38 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCELLGRIDFORM_H
#define UI_TCELLGRIDFORM_H

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
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TCellGridForm
{
public:
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QLabel *textLabel1;
    QLabel *textLabel6;
    QLineEdit *cYDiffEdit;
    QLabel *cFirstCMPLabel;
    QLabel *textLabel5;
    QSpinBox *cXGridNumBox;
    QLineEdit *cXGridEdit;
    QLabel *textLabel3;
    QLabel *textLabel4;
    QLineEdit *cXDiffEdit;
    QLabel *textLabel2;
    QLineEdit *cYGridEdit;
    QSpinBox *cYGridNumBox;
    QSpacerItem *spacer5;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacer9;
    QPushButton *cOkBtn;
    QPushButton *cCancelBtn;

    void setupUi(QDialog *TCellGridForm)
    {
        if (TCellGridForm->objectName().isEmpty())
            TCellGridForm->setObjectName(QString::fromUtf8("TCellGridForm"));
        TCellGridForm->resize(335, 156);
        vboxLayout = new QVBoxLayout(TCellGridForm);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textLabel1 = new QLabel(TCellGridForm);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
        textLabel1->setWordWrap(false);

        gridLayout->addWidget(textLabel1, 1, 0, 1, 1);

        textLabel6 = new QLabel(TCellGridForm);
        textLabel6->setObjectName(QString::fromUtf8("textLabel6"));
        textLabel6->setWordWrap(false);

        gridLayout->addWidget(textLabel6, 3, 2, 1, 1);

        cYDiffEdit = new QLineEdit(TCellGridForm);
        cYDiffEdit->setObjectName(QString::fromUtf8("cYDiffEdit"));

        gridLayout->addWidget(cYDiffEdit, 1, 3, 1, 1);

        cFirstCMPLabel = new QLabel(TCellGridForm);
        cFirstCMPLabel->setObjectName(QString::fromUtf8("cFirstCMPLabel"));
        cFirstCMPLabel->setWordWrap(false);

        gridLayout->addWidget(cFirstCMPLabel, 0, 0, 1, 4);

        textLabel5 = new QLabel(TCellGridForm);
        textLabel5->setObjectName(QString::fromUtf8("textLabel5"));
        textLabel5->setWordWrap(false);

        gridLayout->addWidget(textLabel5, 3, 0, 1, 1);

        cXGridNumBox = new QSpinBox(TCellGridForm);
        cXGridNumBox->setObjectName(QString::fromUtf8("cXGridNumBox"));
        cXGridNumBox->setMinimum(1);
        cXGridNumBox->setMaximum(10000);

        gridLayout->addWidget(cXGridNumBox, 2, 1, 1, 1);

        cXGridEdit = new QLineEdit(TCellGridForm);
        cXGridEdit->setObjectName(QString::fromUtf8("cXGridEdit"));

        gridLayout->addWidget(cXGridEdit, 3, 1, 1, 1);

        textLabel3 = new QLabel(TCellGridForm);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        textLabel3->setWordWrap(false);

        gridLayout->addWidget(textLabel3, 2, 0, 1, 1);

        textLabel4 = new QLabel(TCellGridForm);
        textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
        textLabel4->setWordWrap(false);

        gridLayout->addWidget(textLabel4, 2, 2, 1, 1);

        cXDiffEdit = new QLineEdit(TCellGridForm);
        cXDiffEdit->setObjectName(QString::fromUtf8("cXDiffEdit"));

        gridLayout->addWidget(cXDiffEdit, 1, 1, 1, 1);

        textLabel2 = new QLabel(TCellGridForm);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        textLabel2->setWordWrap(false);

        gridLayout->addWidget(textLabel2, 1, 2, 1, 1);

        cYGridEdit = new QLineEdit(TCellGridForm);
        cYGridEdit->setObjectName(QString::fromUtf8("cYGridEdit"));

        gridLayout->addWidget(cYGridEdit, 3, 3, 1, 1);

        cYGridNumBox = new QSpinBox(TCellGridForm);
        cYGridNumBox->setObjectName(QString::fromUtf8("cYGridNumBox"));
        cYGridNumBox->setMinimum(1);
        cYGridNumBox->setMaximum(10000);

        gridLayout->addWidget(cYGridNumBox, 2, 3, 1, 1);


        vboxLayout->addLayout(gridLayout);

        spacer5 = new QSpacerItem(20, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacer5);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacer9 = new QSpacerItem(41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacer9);

        cOkBtn = new QPushButton(TCellGridForm);
        cOkBtn->setObjectName(QString::fromUtf8("cOkBtn"));

        hboxLayout->addWidget(cOkBtn);

        cCancelBtn = new QPushButton(TCellGridForm);
        cCancelBtn->setObjectName(QString::fromUtf8("cCancelBtn"));

        hboxLayout->addWidget(cCancelBtn);


        vboxLayout->addLayout(hboxLayout);

        QWidget::setTabOrder(cXDiffEdit, cYDiffEdit);
        QWidget::setTabOrder(cYDiffEdit, cXGridNumBox);
        QWidget::setTabOrder(cXGridNumBox, cYGridNumBox);
        QWidget::setTabOrder(cYGridNumBox, cXGridEdit);
        QWidget::setTabOrder(cXGridEdit, cYGridEdit);
        QWidget::setTabOrder(cYGridEdit, cOkBtn);
        QWidget::setTabOrder(cOkBtn, cCancelBtn);

        retranslateUi(TCellGridForm);
        QObject::connect(cOkBtn, SIGNAL(clicked()), TCellGridForm, SLOT(accept()));
        QObject::connect(cCancelBtn, SIGNAL(clicked()), TCellGridForm, SLOT(reject()));

        QMetaObject::connectSlotsByName(TCellGridForm);
    } // setupUi

    void retranslateUi(QDialog *TCellGridForm)
    {
        TCellGridForm->setWindowTitle(QApplication::translate("TCellGridForm", "Cell Grid", 0, QApplication::UnicodeUTF8));
        textLabel1->setText(QApplication::translate("TCellGridForm", "X Diff", 0, QApplication::UnicodeUTF8));
        textLabel6->setText(QApplication::translate("TCellGridForm", "Y Grid", 0, QApplication::UnicodeUTF8));
        cYDiffEdit->setText(QApplication::translate("TCellGridForm", "0", 0, QApplication::UnicodeUTF8));
        cFirstCMPLabel->setText(QApplication::translate("TCellGridForm", "First CMP Coordinate:", 0, QApplication::UnicodeUTF8));
        textLabel5->setText(QApplication::translate("TCellGridForm", "X Grid", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("TCellGridForm", "X Grid Num", 0, QApplication::UnicodeUTF8));
        textLabel4->setText(QApplication::translate("TCellGridForm", "Y Grid Num", 0, QApplication::UnicodeUTF8));
        cXDiffEdit->setText(QApplication::translate("TCellGridForm", "0", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("TCellGridForm", "Y Diff", 0, QApplication::UnicodeUTF8));
        cOkBtn->setText(QApplication::translate("TCellGridForm", "&Ok", 0, QApplication::UnicodeUTF8));
        cOkBtn->setShortcut(QApplication::translate("TCellGridForm", "Alt+O", 0, QApplication::UnicodeUTF8));
        cCancelBtn->setText(QApplication::translate("TCellGridForm", "&Cancel", 0, QApplication::UnicodeUTF8));
        cCancelBtn->setShortcut(QApplication::translate("TCellGridForm", "Alt+C", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TCellGridForm: public Ui_TCellGridForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCELLGRIDFORM_H
