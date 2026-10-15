#include <QtGui/QApplication>
#include "tspsform.h"
#include <QTranslator>
#include <QDebug>
#include "geometryexperimentengine.h" // add by shanzhou in 2026/10/15
#include <string.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	// add by shanzhou in 2026/10/15
	for (int i = 1; i < argc; i++)
	{
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
	//获取翻译文件路径并加载
	if(qtTranslator->load(QCoreApplication::applicationDirPath() + "/IrrGeometryDesign_zh.qm.qm")){
		a.installTranslator(qtTranslator);
	}
	else{
		qDebug(QObject::tr("load failed!!\n"));
		qDebug() << QCoreApplication::applicationDirPath();
	}

	//DataReconstruction w;
	//w.show();

	TSPSForm w;
	w.show();
	return a.exec();
}
