/********************************************************************************
** Form generated from reading UI file 'datareconstruction.ui'
**
** Created: Thu Oct 22 11:47:40 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DATARECONSTRUCTION_H
#define UI_DATARECONSTRUCTION_H

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

class Ui_DataReconstructionClass
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *mTotalshot_num;
    QLabel *label_3;
    QLineEdit *mTotaltrace_num;
    QLabel *label_2;
    QLineEdit *mTotalshotline_num;
    QLabel *label_4;
    QLineEdit *mTotalreceivingline_num;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_7;
    QLineEdit *mSline_interval;
    QLabel *label_10;
    QLineEdit *mRline_interval;
    QLineEdit *mShotpoint_interval;
    QLineEdit *mReceivingpoint_interval;
    QLabel *label_11;
    QLabel *label_12;
    QLabel *label_5;
    QLineEdit *mShot_num_PerSline;
    QHBoxLayout *horizontalLayout;
    QPushButton *mLoadSPSFilesBtn;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *mDisplayandAnalyseBtn;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *mRegularOVTVolumeBtn;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelBtn;

    void setupUi(QDialog *DataReconstructionClass)
    {
        if (DataReconstructionClass->objectName().isEmpty())
            DataReconstructionClass->setObjectName(QString::fromUtf8("DataReconstructionClass"));
        DataReconstructionClass->resize(595, 254);
        verticalLayout = new QVBoxLayout(DataReconstructionClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(DataReconstructionClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        mTotalshot_num = new QLineEdit(groupBox);
        mTotalshot_num->setObjectName(QString::fromUtf8("mTotalshot_num"));
        mTotalshot_num->setReadOnly(true);

        gridLayout->addWidget(mTotalshot_num, 0, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 2, 1, 1);

        mTotaltrace_num = new QLineEdit(groupBox);
        mTotaltrace_num->setObjectName(QString::fromUtf8("mTotaltrace_num"));
        mTotaltrace_num->setReadOnly(true);

        gridLayout->addWidget(mTotaltrace_num, 0, 3, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        mTotalshotline_num = new QLineEdit(groupBox);
        mTotalshotline_num->setObjectName(QString::fromUtf8("mTotalshotline_num"));
        mTotalshotline_num->setReadOnly(true);

        gridLayout->addWidget(mTotalshotline_num, 1, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 1, 2, 1, 1);

        mTotalreceivingline_num = new QLineEdit(groupBox);
        mTotalreceivingline_num->setObjectName(QString::fromUtf8("mTotalreceivingline_num"));
        mTotalreceivingline_num->setReadOnly(true);

        gridLayout->addWidget(mTotalreceivingline_num, 1, 3, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(DataReconstructionClass);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_2->addWidget(label_7, 1, 0, 1, 1);

        mSline_interval = new QLineEdit(groupBox_2);
        mSline_interval->setObjectName(QString::fromUtf8("mSline_interval"));

        gridLayout_2->addWidget(mSline_interval, 1, 1, 1, 1);

        label_10 = new QLabel(groupBox_2);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 1, 2, 1, 1);

        mRline_interval = new QLineEdit(groupBox_2);
        mRline_interval->setObjectName(QString::fromUtf8("mRline_interval"));

        gridLayout_2->addWidget(mRline_interval, 1, 3, 1, 1);

        mShotpoint_interval = new QLineEdit(groupBox_2);
        mShotpoint_interval->setObjectName(QString::fromUtf8("mShotpoint_interval"));

        gridLayout_2->addWidget(mShotpoint_interval, 2, 1, 1, 1);

        mReceivingpoint_interval = new QLineEdit(groupBox_2);
        mReceivingpoint_interval->setObjectName(QString::fromUtf8("mReceivingpoint_interval"));

        gridLayout_2->addWidget(mReceivingpoint_interval, 2, 3, 1, 1);

        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_2->addWidget(label_11, 2, 0, 1, 1);

        label_12 = new QLabel(groupBox_2);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_2->addWidget(label_12, 2, 2, 1, 1);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 3, 0, 1, 1);

        mShot_num_PerSline = new QLineEdit(groupBox_2);
        mShot_num_PerSline->setObjectName(QString::fromUtf8("mShot_num_PerSline"));

        gridLayout_2->addWidget(mShot_num_PerSline, 3, 1, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        mLoadSPSFilesBtn = new QPushButton(DataReconstructionClass);
        mLoadSPSFilesBtn->setObjectName(QString::fromUtf8("mLoadSPSFilesBtn"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(mLoadSPSFilesBtn->sizePolicy().hasHeightForWidth());
        mLoadSPSFilesBtn->setSizePolicy(sizePolicy);
        mLoadSPSFilesBtn->setDefault(true);

        horizontalLayout->addWidget(mLoadSPSFilesBtn);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        mDisplayandAnalyseBtn = new QPushButton(DataReconstructionClass);
        mDisplayandAnalyseBtn->setObjectName(QString::fromUtf8("mDisplayandAnalyseBtn"));
        mDisplayandAnalyseBtn->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(mDisplayandAnalyseBtn->sizePolicy().hasHeightForWidth());
        mDisplayandAnalyseBtn->setSizePolicy(sizePolicy1);
        mDisplayandAnalyseBtn->setDefault(true);

        horizontalLayout->addWidget(mDisplayandAnalyseBtn);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        mRegularOVTVolumeBtn = new QPushButton(DataReconstructionClass);
        mRegularOVTVolumeBtn->setObjectName(QString::fromUtf8("mRegularOVTVolumeBtn"));
        sizePolicy1.setHeightForWidth(mRegularOVTVolumeBtn->sizePolicy().hasHeightForWidth());
        mRegularOVTVolumeBtn->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(mRegularOVTVolumeBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelBtn = new QPushButton(DataReconstructionClass);
        cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));
        sizePolicy.setHeightForWidth(cancelBtn->sizePolicy().hasHeightForWidth());
        cancelBtn->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(cancelBtn);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(DataReconstructionClass);

        QMetaObject::connectSlotsByName(DataReconstructionClass);
    } // setupUi

    void retranslateUi(QDialog *DataReconstructionClass)
    {
        DataReconstructionClass->setWindowTitle(QApplication::translate("DataReconstructionClass", "SPS Seismic geometry attribute analysis", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DataReconstructionClass", "SPS Information", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DataReconstructionClass", "Total shot number", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DataReconstructionClass", "Total trace number", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DataReconstructionClass", "Total shotline number", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("DataReconstructionClass", "Total receiving-line number", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("DataReconstructionClass", "Template Parameter", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("DataReconstructionClass", "Shotline interval (m)", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("DataReconstructionClass", "Receiving line interval (m)", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("DataReconstructionClass", "Shot point interval (m)", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("DataReconstructionClass", "Receiving point interval (m)", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("DataReconstructionClass", "Shot num. per shotline", 0, QApplication::UnicodeUTF8));
        mLoadSPSFilesBtn->setText(QApplication::translate("DataReconstructionClass", "&Load SPS Files", 0, QApplication::UnicodeUTF8));
        mLoadSPSFilesBtn->setShortcut(QApplication::translate("DataReconstructionClass", "Alt+L", 0, QApplication::UnicodeUTF8));
        mDisplayandAnalyseBtn->setText(QApplication::translate("DataReconstructionClass", "&Display and Analyse", 0, QApplication::UnicodeUTF8));
        mDisplayandAnalyseBtn->setShortcut(QApplication::translate("DataReconstructionClass", "Alt+R", 0, QApplication::UnicodeUTF8));
        mRegularOVTVolumeBtn->setText(QApplication::translate("DataReconstructionClass", "Regular OVT Volume", 0, QApplication::UnicodeUTF8));
        cancelBtn->setText(QApplication::translate("DataReconstructionClass", "&Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DataReconstructionClass: public Ui_DataReconstructionClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DATARECONSTRUCTION_H
