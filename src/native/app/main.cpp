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
#include "../gui/survey/tspsform.h"

#include "../cli/commandline.h"
#include "../gui/capture.h"
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
	// 设置 Qt 4 全局 C 字符串与本地编码为系统本地编码 (GBK/GB18030) (add by wushanzhou in 2026/10/15)
	QTextCodec *codec = QTextCodec::codecForLocale();
	if (!codec) codec = QTextCodec::codecForName("GBK");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication a(argc, argv);

	// 设置应用程序默认简体中文字体与反走样 (add by wushanzhou in 2026/10/15)
	QFont appFont("SimSun", 9);
	appFont.setStyleStrategy(QFont::PreferAntialias);
	a.setFont(appFont);

	bool commandHandled=false;
	int commandResult=runCommandLine(argc,argv,commandHandled);
	if(commandHandled)return commandResult;

	QTranslator *qtTranslator = new QTranslator;
	QString appDir = QCoreApplication::applicationDirPath();
	if (qtTranslator->load(appDir + "/IrrGeometryDesign_zh.qm") ||
	    qtTranslator->load("IrrGeometryDesign_zh.qm") ||
	    qtTranslator->load("resources/qt/IrrGeometryDesign_zh.qm") ||
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

	bool captureHandled=false;
	int captureResult=captureGui(w,argc,argv,captureHandled);
	if(captureHandled)return captureResult;

	return a.exec();
}
