/********************************************************************************
** Form generated from reading UI file 'geometrytemplateparameter.ui'
**
** Created: Thu Oct 22 11:47:40 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GEOMETRYTEMPLATEPARAMETER_H
#define UI_GEOMETRYTEMPLATEPARAMETER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_GeometryTemplateParameterClass
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_9;
    QLineEdit *mRline_num_PerTemplate;
    QLabel *label_8;
    QLineEdit *mChannel_num_PerRline;
    QLabel *label_10;
    QLineEdit *mRline_interval;
    QLabel *label_12;
    QLineEdit *mReceivingpoint_interval;
    QLabel *label_13;
    QLineEdit *mFirstRP_X0;
    QLabel *label_2;
    QLineEdit *mFirstRP_Y0;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLineEdit *mSline_num_PerTemplate;
    QLabel *label_5;
    QLineEdit *mShot_num_PerSline;
    QLabel *label_7;
    QLineEdit *mSline_interval;
    QLabel *label_11;
    QLineEdit *mShotpoint_interval;
    QLabel *label_3;
    QLineEdit *mFirstSP_X0;
    QLabel *label_4;
    QLineEdit *mFirstSP_Y0;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *Ok_Btn;
    QSpacerItem *horizontalSpacer;
    QPushButton *Cancel_Btn;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QDialog *GeometryTemplateParameterClass)
    {
        if (GeometryTemplateParameterClass->objectName().isEmpty())
            GeometryTemplateParameterClass->setObjectName(QString::fromUtf8("GeometryTemplateParameterClass"));
        GeometryTemplateParameterClass->resize(674, 269);
        verticalLayout = new QVBoxLayout(GeometryTemplateParameterClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(GeometryTemplateParameterClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_2->addWidget(label_9, 0, 0, 1, 1);

        mRline_num_PerTemplate = new QLineEdit(groupBox);
        mRline_num_PerTemplate->setObjectName(QString::fromUtf8("mRline_num_PerTemplate"));

        gridLayout_2->addWidget(mRline_num_PerTemplate, 0, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_2->addWidget(label_8, 0, 2, 1, 1);

        mChannel_num_PerRline = new QLineEdit(groupBox);
        mChannel_num_PerRline->setObjectName(QString::fromUtf8("mChannel_num_PerRline"));

        gridLayout_2->addWidget(mChannel_num_PerRline, 0, 3, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 1, 0, 1, 1);

        mRline_interval = new QLineEdit(groupBox);
        mRline_interval->setObjectName(QString::fromUtf8("mRline_interval"));

        gridLayout_2->addWidget(mRline_interval, 1, 1, 1, 1);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_2->addWidget(label_12, 1, 2, 1, 1);

        mReceivingpoint_interval = new QLineEdit(groupBox);
        mReceivingpoint_interval->setObjectName(QString::fromUtf8("mReceivingpoint_interval"));

        gridLayout_2->addWidget(mReceivingpoint_interval, 1, 3, 1, 1);

        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_2->addWidget(label_13, 2, 0, 1, 1);

        mFirstRP_X0 = new QLineEdit(groupBox);
        mFirstRP_X0->setObjectName(QString::fromUtf8("mFirstRP_X0"));
        mFirstRP_X0->setReadOnly(true);

        gridLayout_2->addWidget(mFirstRP_X0, 2, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 2, 2, 1, 1);

        mFirstRP_Y0 = new QLineEdit(groupBox);
        mFirstRP_Y0->setObjectName(QString::fromUtf8("mFirstRP_Y0"));
        mFirstRP_Y0->setReadOnly(true);

        gridLayout_2->addWidget(mFirstRP_Y0, 2, 3, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(GeometryTemplateParameterClass);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 0, 0, 1, 1);

        mSline_num_PerTemplate = new QLineEdit(groupBox_2);
        mSline_num_PerTemplate->setObjectName(QString::fromUtf8("mSline_num_PerTemplate"));

        gridLayout->addWidget(mSline_num_PerTemplate, 0, 1, 1, 1);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 0, 2, 1, 1);

        mShot_num_PerSline = new QLineEdit(groupBox_2);
        mShot_num_PerSline->setObjectName(QString::fromUtf8("mShot_num_PerSline"));

        gridLayout->addWidget(mShot_num_PerSline, 0, 3, 1, 1);

        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        mSline_interval = new QLineEdit(groupBox_2);
        mSline_interval->setObjectName(QString::fromUtf8("mSline_interval"));

        gridLayout->addWidget(mSline_interval, 1, 1, 1, 1);

        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout->addWidget(label_11, 1, 2, 1, 1);

        mShotpoint_interval = new QLineEdit(groupBox_2);
        mShotpoint_interval->setObjectName(QString::fromUtf8("mShotpoint_interval"));

        gridLayout->addWidget(mShotpoint_interval, 1, 3, 1, 1);

        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        mFirstSP_X0 = new QLineEdit(groupBox_2);
        mFirstSP_X0->setObjectName(QString::fromUtf8("mFirstSP_X0"));

        gridLayout->addWidget(mFirstSP_X0, 2, 1, 1, 1);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 2, 2, 1, 1);

        mFirstSP_Y0 = new QLineEdit(groupBox_2);
        mFirstSP_Y0->setObjectName(QString::fromUtf8("mFirstSP_Y0"));

        gridLayout->addWidget(mFirstSP_Y0, 2, 3, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        Ok_Btn = new QPushButton(GeometryTemplateParameterClass);
        Ok_Btn->setObjectName(QString::fromUtf8("Ok_Btn"));

        horizontalLayout->addWidget(Ok_Btn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        Cancel_Btn = new QPushButton(GeometryTemplateParameterClass);
        Cancel_Btn->setObjectName(QString::fromUtf8("Cancel_Btn"));

        horizontalLayout->addWidget(Cancel_Btn);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(GeometryTemplateParameterClass);

        QMetaObject::connectSlotsByName(GeometryTemplateParameterClass);
    } // setupUi

    void retranslateUi(QDialog *GeometryTemplateParameterClass)
    {
        GeometryTemplateParameterClass->setWindowTitle(QApplication::translate("GeometryTemplateParameterClass", "GeometryTemplateParameter", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("GeometryTemplateParameterClass", "Geophone line parameter", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("GeometryTemplateParameterClass", "Receiving-line num. per template", 0, QApplication::UnicodeUTF8));
        mRline_num_PerTemplate->setText(QApplication::translate("GeometryTemplateParameterClass", "40", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("GeometryTemplateParameterClass", "Channel num. per receiving-line", 0, QApplication::UnicodeUTF8));
        mChannel_num_PerRline->setText(QApplication::translate("GeometryTemplateParameterClass", "336", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("GeometryTemplateParameterClass", "Receiving line interval (m)", 0, QApplication::UnicodeUTF8));
        mRline_interval->setText(QApplication::translate("GeometryTemplateParameterClass", "125", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("GeometryTemplateParameterClass", "Receiving point interval (m)", 0, QApplication::UnicodeUTF8));
        mReceivingpoint_interval->setText(QApplication::translate("GeometryTemplateParameterClass", "25", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("GeometryTemplateParameterClass", "First receiver Coordinate X0 (m)", 0, QApplication::UnicodeUTF8));
        mFirstRP_X0->setText(QApplication::translate("GeometryTemplateParameterClass", "0", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("GeometryTemplateParameterClass", "First receiver Coordinate Y0 (m)", 0, QApplication::UnicodeUTF8));
        mFirstRP_Y0->setText(QApplication::translate("GeometryTemplateParameterClass", "0", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("GeometryTemplateParameterClass", "Shot line Parameter", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("GeometryTemplateParameterClass", "Shotline num. per template", 0, QApplication::UnicodeUTF8));
        mSline_num_PerTemplate->setText(QApplication::translate("GeometryTemplateParameterClass", "1", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("GeometryTemplateParameterClass", "Shot num. per shotline", 0, QApplication::UnicodeUTF8));
        mShot_num_PerSline->setText(QApplication::translate("GeometryTemplateParameterClass", "5", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("GeometryTemplateParameterClass", "Shotline/V_Roll interval (m)", 0, QApplication::UnicodeUTF8));
        mSline_interval->setText(QApplication::translate("GeometryTemplateParameterClass", "175", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("GeometryTemplateParameterClass", "Shot point interval (m)", 0, QApplication::UnicodeUTF8));
        mShotpoint_interval->setText(QApplication::translate("GeometryTemplateParameterClass", "50", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("GeometryTemplateParameterClass", "First shotpoint Coordinate X0 (m)", 0, QApplication::UnicodeUTF8));
        mFirstSP_X0->setText(QApplication::translate("GeometryTemplateParameterClass", "4187.5", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("GeometryTemplateParameterClass", "First shotpoint Coordinate Y0 (m)", 0, QApplication::UnicodeUTF8));
        mFirstSP_Y0->setText(QApplication::translate("GeometryTemplateParameterClass", "2337.5", 0, QApplication::UnicodeUTF8));
        Ok_Btn->setText(QApplication::translate("GeometryTemplateParameterClass", "Ok", 0, QApplication::UnicodeUTF8));
        Cancel_Btn->setText(QApplication::translate("GeometryTemplateParameterClass", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class GeometryTemplateParameterClass: public Ui_GeometryTemplateParameterClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GEOMETRYTEMPLATEPARAMETER_H
