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
	// 设置 Qt 4 全局 C 字符串与本地编码 (add by shanzhou in 2026/10/15)
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	if (!codec) codec = QTextCodec::codecForLocale();
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication a(argc, argv);

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
			int repeatRuns = 1;
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
	if(qtTranslator->load(QCoreApplication::applicationDirPath() + "/IrrGeometryDesign_zh.qm.qm")){
		a.installTranslator(qtTranslator);
	}
	else{
		qDebug(QObject::tr("load failed!!\n"));
		qDebug() << QCoreApplication::applicationDirPath();
	}

	TSPSForm w;
	w.show();
	return a.exec();
}
