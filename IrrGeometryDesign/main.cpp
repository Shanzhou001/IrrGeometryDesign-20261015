// add by shanzhou in 2026/10/15
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QDebug>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "tspsform.h"
#include "geometryexperimentengine.h"
#include "spsshow/tspsdlg.h"
#include "spsshow/tcellgridform.h"
#include "spssampledlg.h"
#include "tchangeshowareadlg.h"
#include "trealovtdlg.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == NULL || hStdOut == INVALID_HANDLE_VALUE)
	{
		if (AttachConsole(ATTACH_PARENT_PROCESS))
		{
			FILE *dummy = NULL;
			freopen_s(&dummy, "CONOUT$", "w", stdout);
			freopen_s(&dummy, "CONOUT$", "w", stderr);
		}
	}
#endif
	// 设置 Qt 4 全局 C 字符串与本地编码为系统本地编码 (GBK/GB18030) (add by shanzhou in 2026/10/15)
	QTextCodec *codec = QTextCodec::codecForLocale();
	if (!codec) codec = QTextCodec::codecForName("GBK");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication a(argc, argv);

	// 设置应用程序默认简体中文字体与反走样 (add by shanzhou in 2026/10/15)
	QFont appFont("SimSun", 9);
	appFont.setStyleStrategy(QFont::PreferAntialias);
	a.setFont(appFont);

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "--run-tests") == 0 || strcmp(argv[i], "-t") == 0)
		{
			bool ok = GeometryExperimentEngine::runUnitTests();
			return ok ? 0 : 1;
		}
		if (strcmp(argv[i], "--run-experiments") == 0 || strcmp(argv[i], "-e") == 0)
		{
			QString outDir = "experiment_results";
			int repeatRuns = 3;
			unsigned long long masterSeed = 42ULL;

			for (int j = 1; j < argc; j++)
			{
				if ((strcmp(argv[j], "--run-experiments") == 0 || strcmp(argv[j], "-e") == 0) && j + 1 < argc && argv[j + 1][0] != '-')
				{
					outDir = QString::fromLocal8Bit(argv[j + 1]);
				}
				else if ((strcmp(argv[j], "--repeat") == 0 || strcmp(argv[j], "-r") == 0) && j + 1 < argc)
				{
					repeatRuns = atoi(argv[j + 1]);
					if (repeatRuns < 1) repeatRuns = 1;
				}
				else if ((strcmp(argv[j], "--seed") == 0 || strcmp(argv[j], "-s") == 0) && j + 1 < argc)
				{
					masterSeed = (unsigned long long)_atoi64(argv[j + 1]);
				}
			}

			QString summary;
			printf("Starting Full Experiment Suite (Benchmark + Attributes + Wavefield CS Reconstruction)...\n");
			printf("Parameters: OutDir='%s', RepeatRuns=%d, MasterSeed=%llu\n",
				outDir.toLocal8Bit().constData(), repeatRuns, masterSeed);

			bool success = GeometryExperimentEngine::runFullExperimentSuite(NULL, outDir, summary, masterSeed, repeatRuns);
			printf("%s\n", summary.toLocal8Bit().constData());

			if (!success)
			{
				fprintf(stderr, "\n[ERROR]: Full experiment suite encountered failure in one or more required sub-experiments! Exiting with code 1.\n");
				return 1;
			}
			return 0;
		}
	}

	QTranslator *qtTranslator = new QTranslator;
	QString appDir = QCoreApplication::applicationDirPath();
	if (qtTranslator->load(appDir + "/IrrGeometryDesign_zh.qm") ||
	    qtTranslator->load("IrrGeometryDesign_zh.qm") ||
	    qtTranslator->load("IrrGeometryDesign/IrrGeometryDesign_zh.qm") ||
	    qtTranslator->load(appDir + "/IrrGeometryDesign_zh.qm.qm"))
	{
		a.installTranslator(qtTranslator);
	}
	else
	{
		qDebug("load translator IrrGeometryDesign_zh.qm failed!\n");
	}

	TSPSForm w;
	w.show();

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot") == 0)
		{
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("current_gui_window.png");
			return 0;
		}
		if (strcmp(argv[i], "--screenshot-all") == 0)
		{
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("audit_main_window.png");

			// 1. TSPSDlg
			TSPSDlg dlg1;
			dlg1.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg1).save("audit_dlg_tspsdlg.png");
			dlg1.close();

			// 2. TCellGridForm
			TCellGridForm dlg2;
			dlg2.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg2).save("audit_dlg_tcellgridform.png");
			dlg2.close();

			// 3. SpsSampleDlg
			SpsSampleDlg dlg3;
			dlg3.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg3).save("audit_dlg_spssampledlg.png");
			dlg3.close();

			// 4. TChangeShowareadlg
			TChangeShowareadlg dlg4;
			dlg4.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg4).save("audit_dlg_tchangeshowareadlg.png");
			dlg4.close();

			// 5. TRealOVTdlg
			TRealOVTdlg dlg5;
			dlg5.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg5).save("audit_dlg_trealovtdlg.png");
			dlg5.close();

			// 6. ResearchFiguresDialog
			ResearchFiguresDialog dlg6;
			dlg6.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg6).save("audit_dlg_researchfigures.png");
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
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("audit_populated_survey.png");
			return 0;
		}
	}

	return a.exec();
}
