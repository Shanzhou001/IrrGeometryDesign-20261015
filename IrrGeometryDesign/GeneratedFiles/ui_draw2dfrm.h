/********************************************************************************
** Form generated from reading UI file 'draw2dfrm.ui'
**
** Created: Fri Oct 16 19:51:29 2026
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DRAW2DFRM_H
#define UI_DRAW2DFRM_H

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

class Ui_Draw2DfrmClass
{
public:
    QAction *actionShotFocus;
    QAction *actionReceiverFocus;
    QAction *actionDoubleFocus;
    QAction *actionOVTfold;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFocus;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Draw2DfrmClass)
    {
        if (Draw2DfrmClass->objectName().isEmpty())
            Draw2DfrmClass->setObjectName(QString::fromUtf8("Draw2DfrmClass"));
        Draw2DfrmClass->setWindowModality(Qt::ApplicationModal);
        Draw2DfrmClass->resize(1325, 850);
        actionShotFocus = new QAction(Draw2DfrmClass);
        actionShotFocus->setObjectName(QString::fromUtf8("actionShotFocus"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/test/Resources/shotfocus.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionShotFocus->setIcon(icon);
        actionReceiverFocus = new QAction(Draw2DfrmClass);
        actionReceiverFocus->setObjectName(QString::fromUtf8("actionReceiverFocus"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/test/Resources/revfocus.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionReceiverFocus->setIcon(icon1);
        actionDoubleFocus = new QAction(Draw2DfrmClass);
        actionDoubleFocus->setObjectName(QString::fromUtf8("actionDoubleFocus"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/test/Resources/doublefocus.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDoubleFocus->setIcon(icon2);
        actionOVTfold = new QAction(Draw2DfrmClass);
        actionOVTfold->setObjectName(QString::fromUtf8("actionOVTfold"));
        centralWidget = new QWidget(Draw2DfrmClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setGeometry(QRect(0, 47, 1325, 780));
        Draw2DfrmClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Draw2DfrmClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1325, 24));
        menuFocus = new QMenu(menuBar);
        menuFocus->setObjectName(QString::fromUtf8("menuFocus"));
        Draw2DfrmClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Draw2DfrmClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setGeometry(QRect(0, 24, 1325, 23));
        Draw2DfrmClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Draw2DfrmClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        statusBar->setGeometry(QRect(0, 827, 1325, 23));
        Draw2DfrmClass->setStatusBar(statusBar);

        menuBar->addAction(menuFocus->menuAction());
        menuFocus->addAction(actionShotFocus);
        menuFocus->addAction(actionReceiverFocus);
        menuFocus->addAction(actionDoubleFocus);
        menuFocus->addAction(actionOVTfold);
        mainToolBar->addAction(actionShotFocus);
        mainToolBar->addAction(actionReceiverFocus);
        mainToolBar->addAction(actionDoubleFocus);

        retranslateUi(Draw2DfrmClass);

        QMetaObject::connectSlotsByName(Draw2DfrmClass);
    } // setupUi

    void retranslateUi(QMainWindow *Draw2DfrmClass)
    {
        Draw2DfrmClass->setWindowTitle(QApplication::translate("Draw2DfrmClass", "Draw2Dfrm", 0, QApplication::UnicodeUTF8));
        actionShotFocus->setText(QApplication::translate("Draw2DfrmClass", "Shot Focus", 0, QApplication::UnicodeUTF8));
        actionReceiverFocus->setText(QApplication::translate("Draw2DfrmClass", "Receiver Focus", 0, QApplication::UnicodeUTF8));
        actionDoubleFocus->setText(QApplication::translate("Draw2DfrmClass", "Double Focus", 0, QApplication::UnicodeUTF8));
        actionOVTfold->setText(QApplication::translate("Draw2DfrmClass", "OVT fold", 0, QApplication::UnicodeUTF8));
        menuFocus->setTitle(QApplication::translate("Draw2DfrmClass", "Focus", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Draw2DfrmClass: public Ui_Draw2DfrmClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DRAW2DFRM_H
