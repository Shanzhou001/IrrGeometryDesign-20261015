/********************************************************************************
** Form generated from reading UI file 'tspsform.ui'
**
** Created: Fri Oct 16 09:30:54 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TSPSFORM_H
#define UI_TSPSFORM_H

#include <Qt3Support/Q3MainWindow>
#include <Qt3Support/Q3MimeSourceFactory>
#include <Qt3Support/Q3ToolBar>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSpinBox>
#include "gspsview.h"

QT_BEGIN_NAMESPACE

class Ui_TSPSForm
{
public:
    QAction *saveAction;
    QAction *zoomAction;
    QAction *editSPtAction;
    QAction *fillSPSAction;
    QAction *gridAction;
    QAction *newspsAction;
    QAction *ShowNewSPS;
    QAction *ShowImportSPS;
    QAction *actionGeometry_Attribute_Plotting;
    QAction *actionOVT_Volume_Attributes;
    QAction *actionOVT_Patch_Attributes;
    QAction *Load_SPS_Action;
    QAction *actionRSample;
    QAction *actionRAOptimizing;
    QAction *actionSSample;
    QAction *actionSAOptimizing;
    QAction *actionCalculate_the_Miu_for_Loaded_Geometry;
    QAction *actionRunSamplingBenchmark;
    QAction *actionExportAttributeReport;
    QAction *actionRunWavefieldReconstruction;
    QAction *actionRunFullExperimentSuite;
    QAction *actionViewGallery;
    QAction *actionOpenFiguresDir;
    QAction *actionQuickDemoSPS;
    Q3ToolBar *Toolbar;
    QLabel *textLabel3;
    QSpinBox *mScaleBox;
    QMenuBar *MenuBarEditor;
    QMenu *File;
    QMenu *View;
    QMenu *Edit;
    QMenu *menuOVT_Attribute_Analysis;
    QMenu *menuOptimal;

    void setupUi(Q3MainWindow *TSPSForm)
    {
        if (TSPSForm->objectName().isEmpty())
            TSPSForm->setObjectName(QString::fromUtf8("TSPSForm"));
        TSPSForm->resize(666, 490);
        saveAction = new QAction(TSPSForm);
        saveAction->setObjectName(QString::fromUtf8("saveAction"));
        saveAction->setEnabled(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/DataReconstruction/Resources/filesave.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveAction->setIcon(icon);
        zoomAction = new QAction(TSPSForm);
        zoomAction->setObjectName(QString::fromUtf8("zoomAction"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/DataReconstruction/Resources/mouse_zoom.png"), QSize(), QIcon::Normal, QIcon::Off);
        zoomAction->setIcon(icon1);
        editSPtAction = new QAction(TSPSForm);
        editSPtAction->setObjectName(QString::fromUtf8("editSPtAction"));
        editSPtAction->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/DataReconstruction/Resources/editshot.png"), QSize(), QIcon::Normal, QIcon::Off);
        editSPtAction->setIcon(icon2);
        fillSPSAction = new QAction(TSPSForm);
        fillSPSAction->setObjectName(QString::fromUtf8("fillSPSAction"));
        fillSPSAction->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/DataReconstruction/Resources/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        fillSPSAction->setIcon(icon3);
        gridAction = new QAction(TSPSForm);
        gridAction->setObjectName(QString::fromUtf8("gridAction"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/DataReconstruction/Resources/griding.png"), QSize(), QIcon::Normal, QIcon::Off);
        gridAction->setIcon(icon4);
        newspsAction = new QAction(TSPSForm);
        newspsAction->setObjectName(QString::fromUtf8("newspsAction"));
        newspsAction->setEnabled(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/DataReconstruction/Resources/newsps.png"), QSize(), QIcon::Normal, QIcon::Off);
        newspsAction->setIcon(icon5);
        ShowNewSPS = new QAction(TSPSForm);
        ShowNewSPS->setObjectName(QString::fromUtf8("ShowNewSPS"));
        ShowNewSPS->setEnabled(true);
        ShowImportSPS = new QAction(TSPSForm);
        ShowImportSPS->setObjectName(QString::fromUtf8("ShowImportSPS"));
        ShowImportSPS->setEnabled(true);
        actionGeometry_Attribute_Plotting = new QAction(TSPSForm);
        actionGeometry_Attribute_Plotting->setObjectName(QString::fromUtf8("actionGeometry_Attribute_Plotting"));
        actionGeometry_Attribute_Plotting->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/DataReconstruction/Resources/GeometryAT.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionGeometry_Attribute_Plotting->setIcon(icon6);
        actionOVT_Volume_Attributes = new QAction(TSPSForm);
        actionOVT_Volume_Attributes->setObjectName(QString::fromUtf8("actionOVT_Volume_Attributes"));
        actionOVT_Volume_Attributes->setEnabled(false);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OVTVL.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOVT_Volume_Attributes->setIcon(icon7);
        actionOVT_Patch_Attributes = new QAction(TSPSForm);
        actionOVT_Patch_Attributes->setObjectName(QString::fromUtf8("actionOVT_Patch_Attributes"));
        actionOVT_Patch_Attributes->setEnabled(false);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/DataReconstruction/Resources/OVTPT.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOVT_Patch_Attributes->setIcon(icon8);
        Load_SPS_Action = new QAction(TSPSForm);
        Load_SPS_Action->setObjectName(QString::fromUtf8("Load_SPS_Action"));
        actionRSample = new QAction(TSPSForm);
        actionRSample->setObjectName(QString::fromUtf8("actionRSample"));
        actionRSample->setEnabled(false);
        actionRAOptimizing = new QAction(TSPSForm);
        actionRAOptimizing->setObjectName(QString::fromUtf8("actionRAOptimizing"));
        actionRAOptimizing->setEnabled(false);
        actionSSample = new QAction(TSPSForm);
        actionSSample->setObjectName(QString::fromUtf8("actionSSample"));
        actionSSample->setEnabled(false);
        actionSAOptimizing = new QAction(TSPSForm);
        actionSAOptimizing->setObjectName(QString::fromUtf8("actionSAOptimizing"));
        actionSAOptimizing->setEnabled(false);
        actionCalculate_the_Miu_for_Loaded_Geometry = new QAction(TSPSForm);
        actionCalculate_the_Miu_for_Loaded_Geometry->setObjectName(QString::fromUtf8("actionCalculate_the_Miu_for_Loaded_Geometry"));
        actionCalculate_the_Miu_for_Loaded_Geometry->setEnabled(false);
        actionRunSamplingBenchmark = new QAction(TSPSForm);
        actionRunSamplingBenchmark->setObjectName(QString::fromUtf8("actionRunSamplingBenchmark"));
        actionExportAttributeReport = new QAction(TSPSForm);
        actionExportAttributeReport->setObjectName(QString::fromUtf8("actionExportAttributeReport"));
        actionRunWavefieldReconstruction = new QAction(TSPSForm);
        actionRunWavefieldReconstruction->setObjectName(QString::fromUtf8("actionRunWavefieldReconstruction"));
        actionRunFullExperimentSuite = new QAction(TSPSForm);
        actionRunFullExperimentSuite->setObjectName(QString::fromUtf8("actionRunFullExperimentSuite"));
        actionViewGallery = new QAction(TSPSForm);
        actionViewGallery->setObjectName(QString::fromUtf8("actionViewGallery"));
        actionOpenFiguresDir = new QAction(TSPSForm);
        actionOpenFiguresDir->setObjectName(QString::fromUtf8("actionOpenFiguresDir"));
        actionQuickDemoSPS = new QAction(TSPSForm);
        actionQuickDemoSPS->setObjectName(QString::fromUtf8("actionQuickDemoSPS"));
        Toolbar = new Q3ToolBar(TSPSForm);
        Toolbar->setObjectName(QString::fromUtf8("Toolbar"));
        Toolbar->setGeometry(QRect(0, 0, 231, 30));
        textLabel3 = new QLabel();
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        textLabel3->setGeometry(QRect(0, 0, 49, 14));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        textLabel3->setFont(font);
        textLabel3->setWordWrap(false);
        mScaleBox = new QSpinBox();
        mScaleBox->setObjectName(QString::fromUtf8("mScaleBox"));
        mScaleBox->setGeometry(QRect(0, 0, 60, 21));
        mScaleBox->setMinimum(1);
        mScaleBox->setMaximum(10000);
        mScaleBox->setValue(100);
        MenuBarEditor = new QMenuBar(TSPSForm);
        MenuBarEditor->setObjectName(QString::fromUtf8("MenuBarEditor"));
        MenuBarEditor->setGeometry(QRect(0, 0, 361, 21));
        File = new QMenu(MenuBarEditor);
        File->setObjectName(QString::fromUtf8("File"));
        View = new QMenu(MenuBarEditor);
        View->setObjectName(QString::fromUtf8("View"));
        Edit = new QMenu(MenuBarEditor);
        Edit->setObjectName(QString::fromUtf8("Edit"));
        menuOVT_Attribute_Analysis = new QMenu(MenuBarEditor);
        menuOVT_Attribute_Analysis->setObjectName(QString::fromUtf8("menuOVT_Attribute_Analysis"));
        menuOptimal = new QMenu(MenuBarEditor);
        menuOptimal->setObjectName(QString::fromUtf8("menuOptimal"));

        Toolbar->addAction(saveAction);
        Toolbar->addSeparator();
        Toolbar->addAction(editSPtAction);
        Toolbar->addAction(fillSPSAction);
        Toolbar->addAction(gridAction);
        Toolbar->addSeparator();
        Toolbar->addAction(zoomAction);
        MenuBarEditor->addAction(File->menuAction());
        MenuBarEditor->addAction(Edit->menuAction());
        MenuBarEditor->addAction(menuOptimal->menuAction());
        MenuBarEditor->addAction(View->menuAction());
        MenuBarEditor->addAction(menuOVT_Attribute_Analysis->menuAction());
        File->addAction(actionQuickDemoSPS);
        File->addAction(newspsAction);
        File->addAction(saveAction);
        File->addAction(Load_SPS_Action);
        View->addAction(zoomAction);
        View->addAction(actionGeometry_Attribute_Plotting);
        Edit->addAction(gridAction);
        Edit->addAction(ShowNewSPS);
        Edit->addAction(ShowImportSPS);
        menuOVT_Attribute_Analysis->addAction(actionOVT_Volume_Attributes);
        menuOVT_Attribute_Analysis->addAction(actionOVT_Patch_Attributes);
        menuOptimal->addAction(actionRSample);
        menuOptimal->addAction(actionRAOptimizing);
        menuOptimal->addAction(actionSSample);
        menuOptimal->addAction(actionSAOptimizing);
        menuOptimal->addSeparator();
        menuOptimal->addAction(actionCalculate_the_Miu_for_Loaded_Geometry);
        menuOptimal->addSeparator();
        menuOptimal->addAction(actionRunSamplingBenchmark);
        menuOptimal->addAction(actionExportAttributeReport);
        menuOptimal->addAction(actionRunWavefieldReconstruction);
        menuOptimal->addAction(actionRunFullExperimentSuite);
        menuOptimal->addSeparator();
        menuOptimal->addAction(actionViewGallery);
        menuOptimal->addAction(actionOpenFiguresDir);

        retranslateUi(TSPSForm);
        QObject::connect(saveAction, SIGNAL(triggered()), TSPSForm, SLOT(saveSPS()));
        QObject::connect(zoomAction, SIGNAL(triggered()), TSPSForm, SLOT(zoom()));
        QObject::connect(mScaleBox, SIGNAL(valueChanged(int)), TSPSForm, SLOT(onZoomChanged(int)));
        QObject::connect(gridAction, SIGNAL(triggered()), TSPSForm, SLOT(grid()));

        QMetaObject::connectSlotsByName(TSPSForm);
    } // setupUi

    void retranslateUi(Q3MainWindow *TSPSForm)
    {
        TSPSForm->setWindowTitle(QApplication::translate("TSPSForm", "SPS \345\234\260\351\234\207\350\247\202\346\265\213\347\263\273\347\273\237\350\256\276\350\256\241\344\270\216\345\210\206\346\236\220", 0, QApplication::UnicodeUTF8));
        saveAction->setText(QApplication::translate("TSPSForm", "\344\277\235\345\255\230\350\247\202\346\265\213\347\263\273\347\273\237(&S)", 0, QApplication::UnicodeUTF8));
        saveAction->setIconText(QApplication::translate("TSPSForm", "\344\277\235\345\255\230", 0, QApplication::UnicodeUTF8));
        saveAction->setProperty("name", QVariant(QApplication::translate("TSPSForm", "saveAction", 0, QApplication::UnicodeUTF8)));
        zoomAction->setIconText(QApplication::translate("TSPSForm", "\351\274\240\346\240\207\347\274\251\346\224\276(&Z)", 0, QApplication::UnicodeUTF8));
        zoomAction->setProperty("name", QVariant(QApplication::translate("TSPSForm", "zoomAction", 0, QApplication::UnicodeUTF8)));
        editSPtAction->setText(QApplication::translate("TSPSForm", "\347\274\226\350\276\221\347\202\256\347\202\271", 0, QApplication::UnicodeUTF8));
        editSPtAction->setIconText(QApplication::translate("TSPSForm", "\347\274\226\350\276\221\347\202\256\347\202\271", 0, QApplication::UnicodeUTF8));
        editSPtAction->setProperty("name", QVariant(QApplication::translate("TSPSForm", "editSPtAction", 0, QApplication::UnicodeUTF8)));
        fillSPSAction->setText(QApplication::translate("TSPSForm", "\345\241\253\345\205\205 SPS(&F)", 0, QApplication::UnicodeUTF8));
        fillSPSAction->setIconText(QApplication::translate("TSPSForm", "\345\241\253\345\205\205 SPS", 0, QApplication::UnicodeUTF8));
        fillSPSAction->setProperty("name", QVariant(QApplication::translate("TSPSForm", "fillSPSAction", 0, QApplication::UnicodeUTF8)));
        gridAction->setIconText(QApplication::translate("TSPSForm", "CMP \351\235\242\345\205\203\347\275\221\346\240\274(&G)", 0, QApplication::UnicodeUTF8));
        gridAction->setProperty("name", QVariant(QApplication::translate("TSPSForm", "gridAction", 0, QApplication::UnicodeUTF8)));
        newspsAction->setText(QApplication::translate("TSPSForm", "\346\226\260\345\273\272\350\247\202\346\265\213\347\263\273\347\273\237(&N)", 0, QApplication::UnicodeUTF8));
        ShowNewSPS->setText(QApplication::translate("TSPSForm", "\346\230\276\347\244\272\344\274\230\345\214\226\345\220\216\350\247\202\346\265\213\347\263\273\347\273\237", 0, QApplication::UnicodeUTF8));
        ShowImportSPS->setText(QApplication::translate("TSPSForm", "\346\230\276\347\244\272\345\257\274\345\205\245\350\247\202\346\265\213\347\263\273\347\273\237", 0, QApplication::UnicodeUTF8));
        actionGeometry_Attribute_Plotting->setText(QApplication::translate("TSPSForm", "\350\247\202\346\265\213\347\263\273\347\273\237\345\261\236\346\200\247\347\273\274\345\220\210\345\210\206\346\236\220", 0, QApplication::UnicodeUTF8));
        actionOVT_Volume_Attributes->setText(QApplication::translate("TSPSForm", "OVT \344\275\223\345\261\236\346\200\247\345\210\206\346\236\220", 0, QApplication::UnicodeUTF8));
        actionOVT_Patch_Attributes->setText(QApplication::translate("TSPSForm", "OVT \347\211\207\345\261\236\346\200\247\345\210\206\346\236\220", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionOVT_Patch_Attributes->setToolTip(QApplication::translate("TSPSForm", "OVT \347\211\207\345\261\236\346\200\247\345\210\206\346\236\220", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        Load_SPS_Action->setText(QApplication::translate("TSPSForm", "\345\212\240\350\275\275 SPS \346\226\207\344\273\266(&L)", 0, QApplication::UnicodeUTF8));
        actionRSample->setText(QApplication::translate("TSPSForm", "\346\243\200\346\263\242\347\202\271\351\235\236\350\247\204\345\210\231\351\207\207\346\240\267(&R)", 0, QApplication::UnicodeUTF8));
        actionRAOptimizing->setText(QApplication::translate("TSPSForm", "\346\243\200\346\263\242\347\202\271\346\216\222\345\210\227\351\200\200\347\201\253\344\274\230\345\214\226(&A)", 0, QApplication::UnicodeUTF8));
        actionSSample->setText(QApplication::translate("TSPSForm", "\347\202\256\347\202\271\351\235\236\350\247\204\345\210\231\351\207\207\346\240\267(&S)", 0, QApplication::UnicodeUTF8));
        actionSAOptimizing->setText(QApplication::translate("TSPSForm", "\347\202\256\347\202\271\346\216\222\345\210\227\351\200\200\347\201\253\344\274\230\345\214\226(&M)", 0, QApplication::UnicodeUTF8));
        actionCalculate_the_Miu_for_Loaded_Geometry->setText(QApplication::translate("TSPSForm", "\350\256\241\347\256\227\345\275\223\345\211\215\350\247\202\346\265\213\347\263\273\347\273\237\344\272\222\347\233\270\345\271\262\345\272\246 \316\274", 0, QApplication::UnicodeUTF8));
        actionRunSamplingBenchmark->setText(QApplication::translate("TSPSForm", "\345\244\232\351\207\207\346\240\267\347\216\207\345\237\272\345\207\206\345\257\271\346\257\224\350\257\225\351\252\214 (30%/50%/70%)", 0, QApplication::UnicodeUTF8));
        actionExportAttributeReport->setText(QApplication::translate("TSPSForm", "\345\257\274\345\207\272\350\247\202\346\265\213\347\263\273\347\273\237\347\251\272\351\227\264\345\261\236\346\200\247\350\264\250\351\207\217\350\257\204\344\274\260\346\212\245\345\221\212", 0, QApplication::UnicodeUTF8));
        actionRunWavefieldReconstruction->setText(QApplication::translate("TSPSForm", "\345\234\260\351\234\207\346\263\242\345\234\272\345\216\213\347\274\251\346\204\237\347\237\245 POCS \351\207\215\346\236\204\350\257\225\351\252\214", 0, QApplication::UnicodeUTF8));
        actionRunFullExperimentSuite->setText(QApplication::translate("TSPSForm", "\344\270\200\351\224\256\345\205\250\346\265\201\347\250\213\347\247\221\347\240\224\351\227\255\347\216\257\345\257\271\346\257\224\350\257\225\351\252\214", 0, QApplication::UnicodeUTF8));
        actionViewGallery->setText(QApplication::translate("TSPSForm", "\347\247\221\347\240\224\346\210\220\346\236\234\345\233\276\344\273\266\347\224\273\345\273\212\344\270\216\347\247\221\345\255\246\347\273\223\350\256\272", 0, QApplication::UnicodeUTF8));
        actionOpenFiguresDir->setText(QApplication::translate("TSPSForm", "\346\211\223\345\274\200\347\247\221\347\240\224\347\273\223\346\236\234\345\233\276\344\273\266\346\226\207\344\273\266\345\244\271", 0, QApplication::UnicodeUTF8));
        actionQuickDemoSPS->setText(QApplication::translate("TSPSForm", "\345\277\253\351\200\237\350\275\275\345\205\245\346\240\207\345\207\206\347\244\272\350\214\203\350\247\202\346\265\213\347\263\273\347\273\237(&D)", 0, QApplication::UnicodeUTF8));
        Toolbar->setProperty("label", QVariant(QApplication::translate("TSPSForm", "Toolbar", 0, QApplication::UnicodeUTF8)));
        textLabel3->setText(QApplication::translate("TSPSForm", " Scale ", 0, QApplication::UnicodeUTF8));
        mScaleBox->setSuffix(QApplication::translate("TSPSForm", "%", 0, QApplication::UnicodeUTF8));
        File->setTitle(QApplication::translate("TSPSForm", "\346\226\207\344\273\266(&F)", 0, QApplication::UnicodeUTF8));
        View->setTitle(QApplication::translate("TSPSForm", "\350\247\206\345\233\276(&V)", 0, QApplication::UnicodeUTF8));
        Edit->setTitle(QApplication::translate("TSPSForm", "\347\274\226\350\276\221(&E)", 0, QApplication::UnicodeUTF8));
        menuOVT_Attribute_Analysis->setTitle(QApplication::translate("TSPSForm", "OVT \345\261\236\346\200\247\345\210\206\346\236\220(&T)", 0, QApplication::UnicodeUTF8));
        menuOptimal->setTitle(QApplication::translate("TSPSForm", "\347\251\272\351\227\264\344\274\230\345\214\226\344\270\216\347\247\221\347\240\224\350\257\225\351\252\214(&O)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TSPSForm: public Ui_TSPSForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TSPSFORM_H
