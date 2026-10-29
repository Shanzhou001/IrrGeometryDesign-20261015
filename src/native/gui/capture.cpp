#include "../plotting/figurepaths.h"
// add by wushanzhou in 2026/10/15
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <QTime>
#include <QFile>
#include <QTextStream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "survey/tspsform.h"
#include "../experiments/geometryexperimentengine.h"
#include "survey/tspsdlg.h"
#include "survey/tcellgridform.h"
#include "dialogs/spssampledlg.h"
#include "dialogs/tchangeshowareadlg.h"
#include "dialogs/trealovtdlg.h"
#include "dialogs/ovtvolumeattributeview.h"
#include "dialogs/draw2dfrm.h"
#include "dialogs/analyzebinform.h"
#include "dialogs/geometrytemplateparameter.h"
#include "../app/datareconstruction.h"
#include "../io/sps/gproject.h"
#include "../core/gutils.h"

#include "capture.h"

int captureGui(TSPSForm& w,int argc,char** argv,bool& handled)
{
    handled=true;
    for(int i=1;i<argc;i++)if(strcmp(argv[i],"--gui-controls-test")==0) {
        SpsSampleDlg sample;sample.show();
        QLineEdit* t=sample.findChild<QLineEdit*>("lineEdit_TrSamplingRate");
        QPushButton* ok=sample.findChild<QPushButton*>("pushButton_OK");if(!t||!ok)return 10;
        float before=sample.Tsampling_rate;t->setText("invalid");ok->click();
        if(sample.result()==QDialog::Accepted||sample.Tsampling_rate!=before||t->styleSheet().isEmpty())return 11;
        t->setText("0.3");ok->click();if(sample.result()!=QDialog::Accepted||fabs(sample.Tsampling_rate-.3)>1e-6)return 12;
        GeometryTemplateParameter geometry;
        QLineEdit* count=geometry.findChild<QLineEdit*>("mChannel_num_PerRline");
        QLineEdit* first=geometry.findChild<QLineEdit*>("mFirstSP_X0");if(!count||!first)return 13;
        QString previous=first->text();count->setText(QString::number(count->text().toInt()+2));
        if(previous==first->text())return 14;
        TCellGridForm grid;grid.show();QLineEdit* spacing=grid.findChild<QLineEdit*>("cXGridEdit");if(!spacing)return 15;
        spacing->setText("bad");QMetaObject::invokeMethod(&grid,"accept",Qt::DirectConnection);
        if(grid.result()==QDialog::Accepted||spacing->styleSheet().isEmpty())return 16;
        grid.reject();QFile report(FigurePaths::artifactPath("build/gui_controls.json"));
        if(!report.open(QIODevice::WriteOnly|QIODevice::Text))return 17;
        report.write("{\"status\":\"passed\",\"sampling_rejects_invalid\":true,\"sampling_valid_button\":true,\"grid_rejects_invalid\":true,\"template_dependency\":true}\n");
        printf("GUI controls accepted.\n");return 0;
    }

    for(int i=1;i<argc;i++)if(strcmp(argv[i],"--gui-workflow-test")==0 || strcmp(argv[i],"--gui-gallery-test")==0)
    {
        ResearchFiguresDialog dialog(&w);dialog.show();QApplication::processEvents();QApplication::processEvents();
        if(strcmp(argv[i],"--gui-gallery-test")==0 && dialog.figureCount()>=13 && !dialog.imageFits())return 8;
        if(strcmp(argv[i],"--gui-workflow-test")==0) {
            dialog.onRun();QTime timer;timer.start();
            while(dialog.workflowRunning() && timer.elapsed()<14400000){QApplication::processEvents();Sleep(100);}
            if(dialog.workflowRunning() || dialog.workflowExitCode()!=0)return 1;
        }
        dialog.onRefresh();int count=dialog.figureCount();if(count<13)return 2;
        for(int j=0;j<count;j++){
            dialog.selectFigure(j);QApplication::processEvents();if(!dialog.imageLoaded())return 3;
            dialog.resize(900,650);QApplication::processEvents();QApplication::processEvents();if(!dialog.imageFits())return 4;
            dialog.resize(1080,800);QApplication::processEvents();QApplication::processEvents();if(!dialog.imageFits())return 9;
            QSplitter* panels=dialog.findChild<QSplitter*>();
            if(!panels)return 18;
            QList<int> narrow;narrow<<160<<220<<120;panels->setSizes(narrow);
            QApplication::processEvents();QApplication::processEvents();if(!dialog.imageFits())return 19;
            QList<int> normal;normal<<450<<100<<100;panels->setSizes(normal);
            QApplication::processEvents();QApplication::processEvents();if(!dialog.imageFits())return 20;

            if(!FigurePaths::capture(&dialog,QString("gallery_%1.png").arg(j+1,2,10,QChar('0'))))return 5;
        }
        dialog.selectFigure(0);dialog.onNext();dialog.onPrev();dialog.onRefresh();QApplication::processEvents();
        if(!FigurePaths::capture(&dialog,"08_dlg_figure_gallery.png"))return 6;
        QFile report(FigurePaths::artifactPath("results/current/gui_verification.json"));
        if(!report.open(QIODevice::WriteOnly|QIODevice::Text))return 7;
        QTextStream stream(&report);stream<<"{\"status\":\"passed\",\"images_loaded\":"<<count<<",\"resize_navigation_refresh\":true,\"workflow_executed\":"<<(strcmp(argv[i],"--gui-workflow-test")==0?"true":"false")<<"}\n";
        printf("GUI end-to-end accepted: %d images, navigation, resize and refresh.\n",count);return 0;
    }

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot") == 0)
		{
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&w,"01_main_window.png"))return 1;
			return 0;
		}
		if (strcmp(argv[i], "--screenshot-all") == 0)
		{
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&w,"01_main_window.png"))return 1;
			w.quickLoadDemoSPS(false);
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&w,"02_survey_loaded.png"))return 1;

			// 1. TSPSDlg
			TSPSDlg dlg1;
			dlg1.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg1,"03_dlg_sps_parameters.png"))return 1;
			dlg1.close();

			// 2. TCellGridForm
			TCellGridForm dlg2;
			dlg2.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg2,"04_dlg_cell_grid.png"))return 1;
			dlg2.close();

			// 3. SpsSampleDlg
			SpsSampleDlg dlg3;
			dlg3.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg3,"05_dlg_sps_sampling.png"))return 1;
			dlg3.close();

			// 4. TChangeShowareadlg
			TChangeShowareadlg dlg4;
			dlg4.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg4,"06_dlg_show_area.png"))return 1;
			dlg4.close();

			// 5. TRealOVTdlg
			TRealOVTdlg dlg5;
			if (GProject::getCurrentProject()) dlg5.init(GProject::getCurrentProject()->getAttrAnalyzeParam());
			dlg5.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg5,"07_dlg_ovt_attributes.png"))return 1;
			dlg5.close();

			// 6. ResearchFiguresDialog
			ResearchFiguresDialog dlg6;
			dlg6.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg6,"08_dlg_figure_gallery.png"))return 1;
			dlg6.close();

			// 7. DataReconstruction
			DataReconstruction dlg7(&w);
			dlg7.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg7,"09_dlg_data_reconstruction.png"))return 1;
			dlg7.close();

			// 8. GeometryTemplateParameter
			GeometryTemplateParameter dlg8(&w);
			dlg8.show();
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&dlg8,"10_dlg_template_parameter.png"))return 1;
			dlg8.close();

			// 9. OVTVolumeAttributeView (auto-renders Offset Tolerance on setOVTcubeData)
			{
				const int nx = 16, ny = 12;
				GValueVector<short> **offCube = NULL, **aziCube = NULL;
				gAlloc2(offCube, nx, ny);
				gAlloc2(aziCube, nx, ny);
				for (int ix = 0; ix < nx; ix++) {
					for (int iy = 0; iy < ny; iy++) {
						for (int k = 0; k < 6; k++) {
							offCube[ix][iy].append((short)(200 + ix * 120 + iy * 80 + k * 15));
							aziCube[ix][iy].append((short)((ix * 20 + iy * 15 + k * 5) % 360));
						}
					}
				}
				OVTVolumeAttributeView dlg9(&w);
				dlg9.setOVTcubeData(offCube, aziCube, nx, ny, 200.0f, 250.0f, 1600.0f, 1500.0f);
				dlg9.show();
				QCoreApplication::processEvents();
				if(!FigurePaths::capture(&dlg9,"11_dlg_ovt_volume_attributes.png"))return 1;
				dlg9.close();
				for (int ix = 0; ix < nx; ix++)
					for (int iy = 0; iy < ny; iy++) {
						offCube[ix][iy].clear();
						aziCube[ix][iy].clear();
					}
				gFree2(offCube);
				gFree2(aziCube);
			}

			// 10. Draw2Dfrm (focusing energy & fold view)
			{
				const int lx = 40, ly = 30;
				float **focus = NULL;
				int **fold = NULL;
				gAlloc2(focus, lx, ly);
				gAlloc2(fold, lx, ly);
				for (int ix = 0; ix < lx; ix++) {
					for (int iy = 0; iy < ly; iy++) {
						float dx = (ix - lx * 0.5f) / (lx * 0.5f);
						float dy = (iy - ly * 0.5f) / (ly * 0.5f);
						focus[ix][iy] = 100.0f * expf(-(dx * dx + dy * dy) * 2.0f);
						fold[ix][iy] = 8 + (int)(24.0f * expf(-(dx * dx + dy * dy)));
					}
				}
				Draw2Dfrm dlg10(&w);
				dlg10.setlx(lx);
				dlg10.setly(ly);
				dlg10.setstr("OVT Fold & Focusing Energy");
				dlg10.setFold(fold);
				dlg10.setShotFocus(focus);
				dlg10.show();
				QCoreApplication::processEvents();
				if(!FigurePaths::capture(&dlg10,"12_dlg_draw2d_energy.png"))return 1;
				dlg10.close();
				gFree2(focus);
				gFree2(fold);
			}

			// 11. analyzebinform (CMP bin fold & attribute view)
			{
				const int bx = 32, by = 24;
				GValueVector<short> **offArr = NULL, **aziArr = NULL;
				int **binFold = NULL;
				gAlloc2(offArr, bx, by);
				gAlloc2(aziArr, bx, by);
				gAlloc2(binFold, bx, by);
				for (int ix = 0; ix < bx; ix++) {
					for (int iy = 0; iy < by; iy++) {
						int f = 4 + ((ix + iy) % 12);
						binFold[ix][iy] = f;
						for (int k = 0; k < f; k++) {
							offArr[ix][iy].append((short)(150 + k * 180));
							aziArr[ix][iy].append((short)((k * 30 + ix * 7) % 360));
						}
					}
				}
				analyzebinform dlg11(&w);
				dlg11.setAnalyzedata(offArr, aziArr, bx, by, 20, 25.0f, 25.0f, 0.0f);
				dlg11.setAODividNum(1, 1, 0, 0, 360, 2500, "PP: 0-360", "Offset: 0-2500");
				dlg11.CMPLayoutAttibuteConculate(binFold);
				dlg11.setDataValueworkarea(true);
				dlg11.on_Bin_Fold_triggered();
				dlg11.show();
				QCoreApplication::processEvents();
				if(!FigurePaths::capture(&dlg11,"13_dlg_analyze_bin.png"))return 1;
				dlg11.close();
				for (int ix = 0; ix < bx; ix++)
					for (int iy = 0; iy < by; iy++) {
						offArr[ix][iy].clear();
						aziArr[ix][iy].clear();
					}
				gFree2(offArr);
				gFree2(aziArr);
				gFree2(binFold);
			}

			printf("Captured all audit screenshots successfully!\n");
			return 0;
		}
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot-survey") == 0)
		{
			w.quickLoadDemoSPS(false);
			QCoreApplication::processEvents();
			if(!FigurePaths::capture(&w,"02_survey_loaded.png"))return 1;
			return 0;
		}
	}

    handled=false;return 0;
}
