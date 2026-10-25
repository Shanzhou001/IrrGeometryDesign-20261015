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

#include "capture.h"

int captureGui(TSPSForm& w,int argc,char** argv,bool& handled)
{
    handled=true;
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
