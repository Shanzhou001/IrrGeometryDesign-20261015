/****************************************************************************
** Form interface generated from reading ui file 'src\sps\tspsform.ui'
**
** Created:   17 17:27:31 2011
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TSPSFORM_H
#define TSPSFORM_H

#include <qvariant.h>
#include <q3mainwindow.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3PopupMenu>
#include <QCloseEvent>
#include "gspsview.h"
#include "ui_tspsform.h"
//#include "ggausstracer.h"
//#include "gcmpview.h"
#include <QSplitter>
//#include "gmmaindoc3d.h"//  

//    [3/14/2023 Liu Wei]
#include "gproject.h"
#include "trealovtdlg.h"
#include "ovtvolumeattributeview.h"
#include "analyzebinform.h"
#include "datareconstruction.h"
#include "spssampledlg.h"
#include "sample.h"
#include "OptimizeMiu.h"
#include "DiagMulmatrix.h"
#include "spsloadcal.h"
#include <QWidget>
#include <QtGui/QDialog>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QAction;
class Q3ActionGroup;
class Q3ToolBar;
class Q3PopupMenu;
class QLabel;
class QSpinBox;

#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QStringList>
#include <QTextBrowser>
#include <QTextEdit>
#include <QSplitter>

class ResearchFiguresDialog : public QDialog
{
    Q_OBJECT
public:
    ResearchFiguresDialog(QWidget *parent = 0);
    void selectFigure(int index);

public slots:
    void onSelectionChanged(int index);
    void onPrev();
    void onNext();
    void onOpenInBrowser();
    void onOpenFolder();

private:
    void loadFigure(int index);

    QComboBox *mCombo;
    QLabel *mImageLabel;
    QScrollArea *mScrollArea;
    QTextBrowser *mConclusionBrowser;
    QPushButton *mBtnPrev;
    QPushButton *mBtnNext;
    QPushButton *mBtnBrowser;
    QPushButton *mBtnFolder;

    QStringList mFileNames;
    QStringList mTitles;
    QStringList mConclusions;
    int mCurrentIndex;
};

class ExperimentReportDialog : public QDialog
{
    Q_OBJECT
public:
    ExperimentReportDialog(const QString &title, const QString &reportText, int figureIndex = -1, QWidget *parent = 0);
    bool shouldViewFigure() const { return mViewFigure; }

private slots:
    void onViewFigure();

private:
    bool mViewFigure;
    int mFigureIndex;
    QTextEdit *mTextEdit;
    QPushButton *mBtnView;
    QPushButton *mBtnOk;
};

class TSPSForm : public Q3MainWindow
{
    Q_OBJECT

public:
    TSPSForm( QWidget* parent = 0 );
    ~TSPSForm();
    virtual int exec();
	bool is2Dcal;
//	void setGmMainDoc3DDoc(GmMainDoc3D *doc) {mp3DDoc=doc;}//  
public slots:
    virtual void closeEvent( QCloseEvent * e );
    virtual void refresh();

	// [3/14/2023 Liu Wei]--------------           

	//12/3 2025 Wu ShanZhou
    // add by shanzhou in 2026/10/15
    virtual void saveSPS();
    void on_saveAction_triggered();
    void on_zoomAction_triggered();
    void on_gridAction_triggered();
    void on_actionRunSamplingBenchmark_triggered();
    void on_actionExportAttributeReport_triggered();
    void on_actionRunWavefieldReconstruction_triggered();
    void on_actionRunFullExperimentSuite_triggered();
    void updateAllActionsState();
    void updateExperimentActionsState();
    void quickLoadDemoSPS(bool showMsgBox = true);
    void on_actionViewDashboard_triggered();
    void on_actionViewGallery_triggered();
    void on_actionOpenFiguresDir_triggered();
    bool ensureProjectLoaded(bool needGrid = false, bool needSampled = false);
    //virtual void editSPoint();
    //virtual void fillSPS();
	// [3/14/2023 Liu Wei]--------------           

    virtual void onZoomScaleChanged( double zoomScaleX, double zoomScaleY, double preZoomScaleX, double preZoomScaleY );
    virtual void zoom();
    virtual void onZoomChanged( int value );
    virtual void grid();//  
	//        void on_ShowNewSPS_triggered()              
	void on_ShowImportSPS_triggered();
	void on_ShowNewSPS_triggered();
	void on_newspsAction_triggered();
	// [3/14/2023 Liu Wei ]
	void on_actionGeometry_Attribute_Plotting_triggered();
	void on_actionOVT_Volume_Attributes_triggered();
	void on_actionOVT_Patch_Attributes_triggered();
	void on_Load_SPS_Action_triggered();
	void on_actionRSample_triggered();
	void on_actionSSample_triggered();

	void on_actionRAOptimizing_triggered();
	void on_actionSAOptimizing_triggered();
	void on_Cancel_clicked();
	void on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered();
	//void on_SetShowMaxFold_triggered();
	//void on_CRPFoldAction_triggered();
	//void on_CRPAnalyzeAction_triggered();
	//void on_FoldAction_triggered();
	//void on_CMPAnalyzeAction_triggered();
	//void on_actionPolyfitcurve_triggered();
	//void on_DefineCRPline_triggered();


protected:
    bool mIsFill;
	bool mIsLiner;
	QSplitter *mSpl;
    GSPSView *mpView;
    bool mIsMouseZoom;
	bool mIsNewCreate;
	bool mOwnProject;

public:
	Ui::TSPSForm ui;

private:
    void init();
    void destroy();

	GProject *mpProject;

	GValueVector<short> **offsetarray;	
	GValueVector<short> **Azimutharray;
	int mAllocatedCMPX;
	int mAllocatedCMPY;

	OVTVolumeAttributeView *VolumeAttributeview;
	QVector<QVector<int> > R_Sampling_Arrays;
	QVector<int> RL_Sampling;
	QVector<QVector<int> > S_Sampling_Arrays;
	QVector<int> SL_Sampling;
	//////////////////////////////////////////////////////////////////////////add by LiuWei

};

#endif // TSPSFORM_H
