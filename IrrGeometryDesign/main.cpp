// add by shanzhou in 2026/10/15
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QDebug>
#include <string.h>
#include "tspsform.h"
#include "geometryexperimentengine.h"

int main(int argc, char *argv[])
{
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
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				outDir = QString::fromLocal8Bit(argv[i + 1]);
			}
			QString summary;
			printf("Starting Full Experiment Suite (Benchmark + Attributes + Wavefield CS Reconstruction)...\n");
			GeometryExperimentEngine::runFullExperimentSuite(NULL, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
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
