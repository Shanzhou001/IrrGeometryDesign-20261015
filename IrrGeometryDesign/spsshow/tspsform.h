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
//#include "gmmaindoc3d.h"//ֲ

// ΰ [3/14/2023 Liu Wei]
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

class TSPSForm : public Q3MainWindow
{
    Q_OBJECT

public:
    TSPSForm( QWidget* parent = 0 );
    ~TSPSForm();
    virtual int exec();
	bool is2Dcal;
//	void setGmMainDoc3DDoc(GmMainDoc3D *doc) {mp3DDoc=doc;}//ֲ
public slots:
    virtual void closeEvent( QCloseEvent * e );
    virtual void refresh();

	// [3/14/2023 Liu Wei]--------------⼸ܺʱûʵ

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
    void updateExperimentActionsState();
    //virtual void editSPoint();
    //virtual void fillSPS();
	// [3/14/2023 Liu Wei]--------------⼸ܺʱûʵ

    virtual void onZoomScaleChanged( double zoomScaleX, double zoomScaleY, double preZoomScaleX, double preZoomScaleY );
    virtual void zoom();
    virtual void onZoomChanged( int value );
    virtual void grid();//Ԫ
	//ʵֵܺеvoid on_ShowNewSPS_triggered()ʵּ򵥹۲ϵͳ
	void on_ShowImportSPS_triggered();
	void on_ShowNewSPS_triggered();
	void on_newspsAction_triggered();
	// [3/14/2023 Liu Wei ]
	void on_actionGeometry_Attribute_Plotting_triggered();
	void on_actionOVT_Volume_Attributes_triggered();
	void on_actionOVT_Patch_Attributes_triggered();
	void on_Load_SPS_Action_triggered();
	// 뿥ͬѧļӿ [Liu Wei 8/13/2023]
	void on_actionRSample_triggered();//߼첨Ż
	void on_actionSSample_triggered();//ڵŻ

	void on_actionRAOptimizing_triggered();
	void on_actionSAOptimizing_triggered();
	void on_Cancel_clicked();//lwj-2023.9.13
	void on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered();//۲ϵͳĦֵ
	// 뿥ͬѧļӿ [Liu Wei 8/13/2023]
	//void on_SetShowMaxFold_triggered();
	//void on_CRPFoldAction_triggered();
	//void on_CRPAnalyzeAction_triggered();
	//void on_FoldAction_triggered();
	//void on_CMPAnalyzeAction_triggered();
	//void on_actionPolyfitcurve_triggered();
	//void on_DefineCRPline_triggered();


protected:
    bool mIsFill;
	bool mIsLiner;//ֱƱ־
	QSplitter *mSpl;
    GSPSView *mpView;
	//Gcmpview *mpCMPView;
    bool mIsMouseZoom;
	//void Freeline(vector<double> x,vector <double> y);
	//GmMainDoc3D *mp3DDoc;//ֲ
	bool mIsNewCreate;//add by zhaohu Ƿʾ½Ĺ۲ϵͳ
	bool mOwnProject;//add Wushanzhou


public://ͨΪprivate
	Ui::TSPSForm ui;

private:
    void init();
    void destroy();
	//void BaseCRPfun();
	//void BaseCMPfun();
	//int mAimID;
	//QString mAimName;
	//GGaussTracer *mpTracer;

	//////////////////////////////////////////////////////////////////////////add by LiuWei
	GProject *mpProject;

	GValueVector<short> **offsetarray;	
	GValueVector<short> **Azimutharray;

	OVTVolumeAttributeView *VolumeAttributeview;
	QVector<QVector<int> > R_Sampling_Arrays;	// 첨 [Liu Wei 6/30/2023]
	QVector<int> RL_Sampling;
	QVector<QVector<int> > S_Sampling_Arrays;	// Դ [Liu Wei 6/30/2023]
	QVector<int> SL_Sampling;
	//////////////////////////////////////////////////////////////////////////add by LiuWei

};

#endif // TSPSFORM_H
