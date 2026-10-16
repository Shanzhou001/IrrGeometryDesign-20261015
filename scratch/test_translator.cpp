#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QTextCodec>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec) codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    QTranslator trans;
    bool ok = trans.load("IrrGeometryDesign/IrrGeometryDesign_zh.qm");
    printf("Load QM: %s\n", ok ? "TRUE" : "FALSE");

    QString t1 = trans.translate("TSPSForm", "New");
    QString t2 = trans.translate("TSPSForm", "Show New SPS");
    QString t3 = trans.translate("TSPSForm", "Load");
    QString t4 = trans.translate("TSPSForm", "RSample");
    QString t5 = trans.translate("TSPSForm", "SSample");

    printf("New -> %s (bytes: %d)\n", t1.toLocal8Bit().constData(), t1.length());
    printf("Show New SPS -> %s\n", t2.toLocal8Bit().constData());
    printf("Load -> %s\n", t3.toLocal8Bit().constData());
    printf("RSample -> %s\n", t4.toLocal8Bit().constData());
    printf("SSample -> %s\n", t5.toLocal8Bit().constData());

    return 0;
}
