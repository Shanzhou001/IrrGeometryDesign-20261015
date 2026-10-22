/********************************************************************************
** Form generated from reading UI file 'ovtvolumeattributeview.ui'
**
** Created: Thu Oct 22 11:47:40 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OVTVOLUMEATTRIBUTEVIEW_H
#define UI_OVTVOLUMEATTRIBUTEVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <qcustomplot.h>

QT_BEGIN_NAMESPACE

class Ui_OVTVolumeAttributeViewClass
{
public:
    QAction *actionOffset_Tolerance;
    QAction *actionOffset_Dispersion;
    QAction *actionMin_Offset;
    QAction *actionMax_Offset;
    QAction *actionAverage_Offset;
    QAction *actionAzimuth_Tolerance;
    QAction *actionTest_Colormap;
    QAction *actionOVT_Trace_Count;
    QAction *actionAzimuth_Dispersion;
    QAction *actionMin_Azimuth;
    QAction *actionMax_Azimuth;
    QAction *actionAverage_Azimuth;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QCustomPlot *mCustomPlot;
    QMenuBar *menuBar;
    QMenu *menuOVT_Volume_Attributes;
    QMenu *menuMy_Plots;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *OVTVolumeAttributeViewClass)
    {
        if (OVTVolumeAttributeViewClass->objectName().isEmpty())
            OVTVolumeAttributeViewClass->setObjectName(QString::fromUtf8("OVTVolumeAttributeViewClass"));
        OVTVolumeAttributeViewClass->resize(790, 570);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OVTVolumeAttributeViewClass->sizePolicy().hasHeightForWidth());
        OVTVolumeAttributeViewClass->setSizePolicy(sizePolicy);
        actionOffset_Tolerance = new QAction(OVTVolumeAttributeViewClass);
        actionOffset_Tolerance->setObjectName(QString::fromUtf8("actionOffset_Tolerance"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OTOL.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOffset_Tolerance->setIcon(icon);
        actionOffset_Dispersion = new QAction(OVTVolumeAttributeViewClass);
        actionOffset_Dispersion->setObjectName(QString::fromUtf8("actionOffset_Dispersion"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/DataReconstruction/Resources/ODIS.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOffset_Dispersion->setIcon(icon1);
        actionMin_Offset = new QAction(OVTVolumeAttributeViewClass);
        actionMin_Offset->setObjectName(QString::fromUtf8("actionMin_Offset"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OMIN.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMin_Offset->setIcon(icon2);
        actionMax_Offset = new QAction(OVTVolumeAttributeViewClass);
        actionMax_Offset->setObjectName(QString::fromUtf8("actionMax_Offset"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OMAX.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMax_Offset->setIcon(icon3);
        actionAverage_Offset = new QAction(OVTVolumeAttributeViewClass);
        actionAverage_Offset->setObjectName(QString::fromUtf8("actionAverage_Offset"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OAVR.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAverage_Offset->setIcon(icon4);
        actionAzimuth_Tolerance = new QAction(OVTVolumeAttributeViewClass);
        actionAzimuth_Tolerance->setObjectName(QString::fromUtf8("actionAzimuth_Tolerance"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/DataReconstruction/Resources/ATOL.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAzimuth_Tolerance->setIcon(icon5);
        actionTest_Colormap = new QAction(OVTVolumeAttributeViewClass);
        actionTest_Colormap->setObjectName(QString::fromUtf8("actionTest_Colormap"));
        actionOVT_Trace_Count = new QAction(OVTVolumeAttributeViewClass);
        actionOVT_Trace_Count->setObjectName(QString::fromUtf8("actionOVT_Trace_Count"));
        actionAzimuth_Dispersion = new QAction(OVTVolumeAttributeViewClass);
        actionAzimuth_Dispersion->setObjectName(QString::fromUtf8("actionAzimuth_Dispersion"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/DataReconstruction/Resources/ADIS.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAzimuth_Dispersion->setIcon(icon6);
        actionMin_Azimuth = new QAction(OVTVolumeAttributeViewClass);
        actionMin_Azimuth->setObjectName(QString::fromUtf8("actionMin_Azimuth"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/DataReconstruction/Resources/AMIN.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMin_Azimuth->setIcon(icon7);
        actionMax_Azimuth = new QAction(OVTVolumeAttributeViewClass);
        actionMax_Azimuth->setObjectName(QString::fromUtf8("actionMax_Azimuth"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/DataReconstruction/Resources/AMAX.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMax_Azimuth->setIcon(icon8);
        actionAverage_Azimuth = new QAction(OVTVolumeAttributeViewClass);
        actionAverage_Azimuth->setObjectName(QString::fromUtf8("actionAverage_Azimuth"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/DataReconstruction/Resources/AAVR.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAverage_Azimuth->setIcon(icon9);
        centralWidget = new QWidget(OVTVolumeAttributeViewClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        mCustomPlot = new QCustomPlot(centralWidget);
        mCustomPlot->setObjectName(QString::fromUtf8("mCustomPlot"));

        gridLayout->addWidget(mCustomPlot, 0, 0, 1, 1);

        OVTVolumeAttributeViewClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(OVTVolumeAttributeViewClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 790, 22));
        menuOVT_Volume_Attributes = new QMenu(menuBar);
        menuOVT_Volume_Attributes->setObjectName(QString::fromUtf8("menuOVT_Volume_Attributes"));
        menuMy_Plots = new QMenu(menuBar);
        menuMy_Plots->setObjectName(QString::fromUtf8("menuMy_Plots"));
        OVTVolumeAttributeViewClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(OVTVolumeAttributeViewClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        OVTVolumeAttributeViewClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(OVTVolumeAttributeViewClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        OVTVolumeAttributeViewClass->setStatusBar(statusBar);

        menuBar->addAction(menuOVT_Volume_Attributes->menuAction());
        menuBar->addAction(menuMy_Plots->menuAction());
        menuOVT_Volume_Attributes->addAction(actionOffset_Tolerance);
        menuOVT_Volume_Attributes->addAction(actionOffset_Dispersion);
        menuOVT_Volume_Attributes->addAction(actionMin_Offset);
        menuOVT_Volume_Attributes->addAction(actionMax_Offset);
        menuOVT_Volume_Attributes->addAction(actionAverage_Offset);
        menuMy_Plots->addAction(actionAzimuth_Tolerance);
        menuMy_Plots->addAction(actionAzimuth_Dispersion);
        menuMy_Plots->addAction(actionMin_Azimuth);
        menuMy_Plots->addAction(actionMax_Azimuth);
        menuMy_Plots->addAction(actionAverage_Azimuth);
        mainToolBar->addAction(actionOffset_Tolerance);
        mainToolBar->addAction(actionOffset_Dispersion);
        mainToolBar->addAction(actionAverage_Offset);
        mainToolBar->addAction(actionAzimuth_Tolerance);
        mainToolBar->addAction(actionAzimuth_Dispersion);
        mainToolBar->addAction(actionAverage_Azimuth);

        retranslateUi(OVTVolumeAttributeViewClass);

        QMetaObject::connectSlotsByName(OVTVolumeAttributeViewClass);
    } // setupUi

    void retranslateUi(QMainWindow *OVTVolumeAttributeViewClass)
    {
        OVTVolumeAttributeViewClass->setWindowTitle(QApplication::translate("OVTVolumeAttributeViewClass", "OVTVolumeAttributeView", 0, QApplication::UnicodeUTF8));
        actionOffset_Tolerance->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Offset Tolerance", 0, QApplication::UnicodeUTF8));
        actionOffset_Dispersion->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Offset Dispersion", 0, QApplication::UnicodeUTF8));
        actionMin_Offset->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Minimum Offset", 0, QApplication::UnicodeUTF8));
        actionMax_Offset->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Maximum Offset", 0, QApplication::UnicodeUTF8));
        actionAverage_Offset->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Average Offset", 0, QApplication::UnicodeUTF8));
        actionAzimuth_Tolerance->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Azimuth Tolerance", 0, QApplication::UnicodeUTF8));
        actionTest_Colormap->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Test Colormap", 0, QApplication::UnicodeUTF8));
        actionOVT_Trace_Count->setText(QApplication::translate("OVTVolumeAttributeViewClass", "OVT Trace_Count", 0, QApplication::UnicodeUTF8));
        actionAzimuth_Dispersion->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Azimuth Dispersion", 0, QApplication::UnicodeUTF8));
        actionMin_Azimuth->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Minimum Azimuth", 0, QApplication::UnicodeUTF8));
        actionMax_Azimuth->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Maximum Azimuth", 0, QApplication::UnicodeUTF8));
        actionAverage_Azimuth->setText(QApplication::translate("OVTVolumeAttributeViewClass", "Average Azimuth", 0, QApplication::UnicodeUTF8));
        menuOVT_Volume_Attributes->setTitle(QApplication::translate("OVTVolumeAttributeViewClass", "Offset OVT Volume Attributes", 0, QApplication::UnicodeUTF8));
        menuMy_Plots->setTitle(QApplication::translate("OVTVolumeAttributeViewClass", "Azimuth OVT Volume Attributes", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OVTVolumeAttributeViewClass: public Ui_OVTVolumeAttributeViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OVTVOLUMEATTRIBUTEVIEW_H
