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
			// 2026.10.20: 3 轮不足以支撑统计结论，默认提高到 10 轮独立种子
			int repeatRuns = 10;
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
		// 2026.10.20: 真实野外资料重构验证专用入口
		// 用法: --run-real-reconstruction <input.csv> [--out <dir>] [--ratio <0..1>] [--mask <mask.txt>]
		if (strcmp(argv[i], "--run-real-reconstruction") == 0)
		{
			QString inCsv;
			QString outDir = "results/real_field";
			float ratio = 0.50f;
			QString maskPath = "";   // 留空则自动生成抖动掩码（固定种子，可复现）
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc)
				{
					outDir = QString::fromLocal8Bit(argv[j + 1]);
				}
				else if (strcmp(argv[j], "--ratio") == 0 && j + 1 < argc)
				{
					ratio = (float)atof(argv[j + 1]);
					if (ratio <= 0.0f || ratio >= 1.0f) ratio = 0.50f;
				}
				else if (strcmp(argv[j], "--mask") == 0 && j + 1 < argc)
				{
					maskPath = QString::fromLocal8Bit(argv[j + 1]);
				}
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --run-real-reconstruction <input.csv> [--out <dir>] [--ratio <0..1>] [--mask <mask.txt>]\n");
				return 1;
			}
			printf("Real-field wavefield reconstruction\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  outDir: %s\n", outDir.toLocal8Bit().constData());
			printf("  ratio : %.2f\n", ratio);
			printf("  mask  : %s\n", maskPath.isEmpty() ? "(auto-generated jitter)" : maskPath.toLocal8Bit().constData());

			GeometryExperimentEngine::ReconstructionMetrics metrics;
			QString summary;
			bool ok = GeometryExperimentEngine::runWavefieldReconstruction(
				maskPath, outDir, ratio, metrics, summary, inCsv, true, false, true, true);
			printf("%s\n", summary.toLocal8Bit().constData());
			if (!ok)
			{
				fprintf(stderr, "\n[ERROR]: real-field reconstruction failed.\n");
				return 1;
			}
			printf("\nReal-field reconstruction succeeded.\n");
			return 0;
		}
		// 2026.10.20: 真实实测道留出验证（不依赖插值参考场）
		// 用法: --holdout-validate <gather.csv> [--traces m] [--keep k] [--iter n] [--seed s] [--out dir]
		if (strcmp(argv[i], "--holdout-validate") == 0)
		{
			QString inCsv;
			QString outDir = "results/real_field_holdout";
			int m = 59, k = 30, iters = 40;
			unsigned long long hs = 20261020ULL;
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc) outDir = QString::fromLocal8Bit(argv[j + 1]);
				else if (strcmp(argv[j], "--traces") == 0 && j + 1 < argc) m = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--keep") == 0 && j + 1 < argc) k = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--iter") == 0 && j + 1 < argc) iters = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--seed") == 0 && j + 1 < argc) hs = (unsigned long long)_atoi64(argv[j + 1]);
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --holdout-validate <gather.csv> [--traces m] [--keep k] [--iter n] [--seed s] [--out dir]\n");
				return 1;
			}
			printf("Hold-out validation on measured traces\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  traces=%d keep=%d iter=%d seed=%llu\n", m, k, iters, hs);
			GeometryExperimentEngine::HoldoutMetrics hm;
			QString rep;
			bool hok = GeometryExperimentEngine::runHoldoutValidation(inCsv, m, k, iters, hs, outDir, hm, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			if (!hok)
			{
				fprintf(stderr, "\n[ERROR]: hold-out validation failed.\n");
				return 1;
			}
			return 0;
		}
		// 2026.10.20: 批量留出验证并归档（生成 holdout_runs.csv，供第三方复算）
		// 用法: --holdout-batch <gather.csv> [--traces m] [--iter n] [--out dir]
		if (strcmp(argv[i], "--holdout-batch") == 0)
		{
			QString inCsv;
			QString outDir = "results/real_field_holdout";
			int m = 59, iters = 40;
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc) outDir = QString::fromLocal8Bit(argv[j + 1]);
				else if (strcmp(argv[j], "--traces") == 0 && j + 1 < argc) m = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--iter") == 0 && j + 1 < argc) iters = atoi(argv[j + 1]);
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --holdout-batch <gather.csv> [--traces m] [--iter n] [--out dir]\n");
				return 1;
			}
			QList<int> keeps;
			keeps << 20 << 30 << 40;
			QList<unsigned long long> seeds;
			seeds << 20261020ULL << 12345ULL;

			printf("Hold-out batch validation (6 configurations)\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  traces=%d iter=%d out=%s\n", m, iters, outDir.toLocal8Bit().constData());

			QString rep;
			bool bok = GeometryExperimentEngine::runHoldoutBatch(inCsv, m, keeps, seeds, iters, outDir, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			if (!bok)
			{
				fprintf(stderr, "\n[ERROR]: hold-out batch validation failed.\n");
				return 1;
			}
			return 0;
		}
		// 2026.10.20 新增：曲波变换参数鲁棒性实验（结果写入指定目录，可归档）
		// 用法: --miu-robustness [outDir, 默认 results/synthetic/01_sampling_benchmark]
		if (strcmp(argv[i], "--miu-robustness") == 0)
		{
			QString outDir = "results/synthetic/01_sampling_benchmark";
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				outDir = QString::fromLocal8Bit(argv[i + 1]);
			}
			printf("Curvelet parameter robustness experiment\n");
			printf("  outDir: %s\n", outDir.toLocal8Bit().constData());
			QString summary;
			bool ok = GeometryExperimentEngine::runMiuParamRobustnessExperiment(128, 0.50f, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
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
