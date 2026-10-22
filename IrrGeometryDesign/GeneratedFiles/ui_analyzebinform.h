/********************************************************************************
** Form generated from reading UI file 'analyzebinform.ui'
**
** Created: Thu Oct 22 11:47:40 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANALYZEBINFORM_H
#define UI_ANALYZEBINFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_analyzebinformClass
{
public:
    QAction *Fold_Bins;
    QAction *Offset_Traces;
    QAction *Azimuth_Traces;
    QAction *Rose_Traces;
    QAction *Offset_Fold_Bins;
    QAction *Bin_Fold;
    QAction *Show_next_point;
    QAction *Show_previous_point;
    QAction *actionOffsetFactor;
    QAction *actionAzimuthFactor;
    QAction *actionFootFactor;
    QAction *actionCSQFactor;
    QAction *actionGeometryImformation;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuBin_analyze;
    QMenu *menuTool;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *analyzebinformClass)
    {
        if (analyzebinformClass->objectName().isEmpty())
            analyzebinformClass->setObjectName(QString::fromUtf8("analyzebinformClass"));
        analyzebinformClass->setWindowModality(Qt::ApplicationModal);
        analyzebinformClass->resize(1100, 700);
        analyzebinformClass->setMouseTracking(true);
        analyzebinformClass->setAcceptDrops(true);
        analyzebinformClass->setAutoFillBackground(true);
        Fold_Bins = new QAction(analyzebinformClass);
        Fold_Bins->setObjectName(QString::fromUtf8("Fold_Bins"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/DataReconstruction/Resources/foldVSBINS.PNG"), QSize(), QIcon::Normal, QIcon::Off);
        Fold_Bins->setIcon(icon);
        Offset_Traces = new QAction(analyzebinformClass);
        Offset_Traces->setObjectName(QString::fromUtf8("Offset_Traces"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/DataReconstruction/Resources/offsetVStraces.PNG"), QSize(), QIcon::Normal, QIcon::Off);
        Offset_Traces->setIcon(icon1);
        Azimuth_Traces = new QAction(analyzebinformClass);
        Azimuth_Traces->setObjectName(QString::fromUtf8("Azimuth_Traces"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/DataReconstruction/Resources/azimuthVStraces.PNG"), QSize(), QIcon::Normal, QIcon::Off);
        Azimuth_Traces->setIcon(icon2);
        Rose_Traces = new QAction(analyzebinformClass);
        Rose_Traces->setObjectName(QString::fromUtf8("Rose_Traces"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/DataReconstruction/Resources/rose.PNG"), QSize(), QIcon::Normal, QIcon::Off);
        Rose_Traces->setIcon(icon3);
        Offset_Fold_Bins = new QAction(analyzebinformClass);
        Offset_Fold_Bins->setObjectName(QString::fromUtf8("Offset_Fold_Bins"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/DataReconstruction/Resources/offsetVSfold-bins.PNG"), QSize(), QIcon::Normal, QIcon::Off);
        Offset_Fold_Bins->setIcon(icon4);
        Bin_Fold = new QAction(analyzebinformClass);
        Bin_Fold->setObjectName(QString::fromUtf8("Bin_Fold"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/DataReconstruction/Resources/binfold.png"), QSize(), QIcon::Normal, QIcon::Off);
        Bin_Fold->setIcon(icon5);
        Show_next_point = new QAction(analyzebinformClass);
        Show_next_point->setObjectName(QString::fromUtf8("Show_next_point"));
        Show_next_point->setEnabled(false);
        Show_previous_point = new QAction(analyzebinformClass);
        Show_previous_point->setObjectName(QString::fromUtf8("Show_previous_point"));
        Show_previous_point->setEnabled(false);
        actionOffsetFactor = new QAction(analyzebinformClass);
        actionOffsetFactor->setObjectName(QString::fromUtf8("actionOffsetFactor"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/DataReconstruction/Resources/Offset_Nonuniform.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOffsetFactor->setIcon(icon6);
        actionAzimuthFactor = new QAction(analyzebinformClass);
        actionAzimuthFactor->setObjectName(QString::fromUtf8("actionAzimuthFactor"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/DataReconstruction/Resources/AzimuthNonfor.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAzimuthFactor->setIcon(icon7);
        actionFootFactor = new QAction(analyzebinformClass);
        actionFootFactor->setObjectName(QString::fromUtf8("actionFootFactor"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/DataReconstruction/Resources/foot_factor.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFootFactor->setIcon(icon8);
        actionCSQFactor = new QAction(analyzebinformClass);
        actionCSQFactor->setObjectName(QString::fromUtf8("actionCSQFactor"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/DataReconstruction/Resources/CSQF.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCSQFactor->setIcon(icon9);
        actionGeometryImformation = new QAction(analyzebinformClass);
        actionGeometryImformation->setObjectName(QString::fromUtf8("actionGeometryImformation"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/DataReconstruction/Resources/Surveyinfos.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionGeometryImformation->setIcon(icon10);
        centralWidget = new QWidget(analyzebinformClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        analyzebinformClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(analyzebinformClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1100, 22));
        menuBin_analyze = new QMenu(menuBar);
        menuBin_analyze->setObjectName(QString::fromUtf8("menuBin_analyze"));
        menuTool = new QMenu(menuBar);
        menuTool->setObjectName(QString::fromUtf8("menuTool"));
        analyzebinformClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(analyzebinformClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        analyzebinformClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(analyzebinformClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        analyzebinformClass->setStatusBar(statusBar);
        toolBar = new QToolBar(analyzebinformClass);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        analyzebinformClass->addToolBar(Qt::TopToolBarArea, toolBar);

        menuBar->addAction(menuBin_analyze->menuAction());
        menuBar->addAction(menuTool->menuAction());
        menuBin_analyze->addAction(Fold_Bins);
        menuBin_analyze->addAction(Offset_Traces);
        menuBin_analyze->addAction(Azimuth_Traces);
        menuBin_analyze->addAction(Rose_Traces);
        menuBin_analyze->addAction(Offset_Fold_Bins);
        menuBin_analyze->addAction(Bin_Fold);
        menuBin_analyze->addAction(actionOffsetFactor);
        menuBin_analyze->addAction(actionAzimuthFactor);
        menuBin_analyze->addAction(actionFootFactor);
        menuBin_analyze->addAction(actionCSQFactor);
        menuBin_analyze->addAction(actionGeometryImformation);
        menuTool->addAction(Show_next_point);
        menuTool->addAction(Show_previous_point);
        mainToolBar->addAction(Fold_Bins);
        mainToolBar->addAction(Offset_Traces);
        mainToolBar->addAction(Azimuth_Traces);
        mainToolBar->addAction(Rose_Traces);
        mainToolBar->addAction(Offset_Fold_Bins);
        mainToolBar->addAction(Bin_Fold);
        mainToolBar->addAction(actionOffsetFactor);
        mainToolBar->addAction(actionAzimuthFactor);
        mainToolBar->addAction(actionFootFactor);
        mainToolBar->addAction(actionCSQFactor);
        toolBar->addAction(actionGeometryImformation);

        retranslateUi(analyzebinformClass);

        QMetaObject::connectSlotsByName(analyzebinformClass);
    } // setupUi

    void retranslateUi(QMainWindow *analyzebinformClass)
    {
        analyzebinformClass->setWindowTitle(QApplication::translate("analyzebinformClass", "analyzebinform", 0, QApplication::UnicodeUTF8));
        Fold_Bins->setText(QApplication::translate("analyzebinformClass", "Fold-Bins", 0, QApplication::UnicodeUTF8));
        Offset_Traces->setText(QApplication::translate("analyzebinformClass", "Offset-Traces", 0, QApplication::UnicodeUTF8));
        Azimuth_Traces->setText(QApplication::translate("analyzebinformClass", "Azimuth-Traces", 0, QApplication::UnicodeUTF8));
        Rose_Traces->setText(QApplication::translate("analyzebinformClass", "Rose-Traces", 0, QApplication::UnicodeUTF8));
        Offset_Fold_Bins->setText(QApplication::translate("analyzebinformClass", "Offset-Fold-Bins", 0, QApplication::UnicodeUTF8));
        Bin_Fold->setText(QApplication::translate("analyzebinformClass", "Bin-Fold", 0, QApplication::UnicodeUTF8));
        Show_next_point->setText(QApplication::translate("analyzebinformClass", "Show next point", 0, QApplication::UnicodeUTF8));
        Show_previous_point->setText(QApplication::translate("analyzebinformClass", "Show previous point", 0, QApplication::UnicodeUTF8));
        actionOffsetFactor->setText(QApplication::translate("analyzebinformClass", "Offset Factor", 0, QApplication::UnicodeUTF8));
        actionAzimuthFactor->setText(QApplication::translate("analyzebinformClass", "Azimuth Factor", 0, QApplication::UnicodeUTF8));
        actionFootFactor->setText(QApplication::translate("analyzebinformClass", "Foot Factor", 0, QApplication::UnicodeUTF8));
        actionCSQFactor->setText(QApplication::translate("analyzebinformClass", "CSQFactor", 0, QApplication::UnicodeUTF8));
        actionGeometryImformation->setText(QApplication::translate("analyzebinformClass", "Geometry Imformation", 0, QApplication::UnicodeUTF8));
        menuBin_analyze->setTitle(QApplication::translate("analyzebinformClass", "Bin analyze", 0, QApplication::UnicodeUTF8));
        menuTool->setTitle(QApplication::translate("analyzebinformClass", "Tool", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QApplication::translate("analyzebinformClass", "toolBar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class analyzebinformClass: public Ui_analyzebinformClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANALYZEBINFORM_H
