code = """#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QPixmap>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QString>
#include <stdio.h>

#include "spsshow/tspsdlg.h"
#include "spsshow/tcellgridform.h"
#include "spssampledlg.h"
#include "tchangeshowareadlg.h"
#include "trealovtdlg.h"

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec) codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    QApplication a(argc, argv);

    QFont appFont("SimSun", 9);
    appFont.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(appFont);

    QTranslator qtTranslator;
    if (qtTranslator.load("x64/Release/IrrGeometryDesign_zh.qm"))
    {
        a.installTranslator(&qtTranslator);
    }

    // 1. Test TSPSDlg
    {
        TSPSDlg dlg;
        dlg.show();
        QCoreApplication::processEvents();
        QPixmap pix = QPixmap::grabWidget(&dlg);
        pix.save("dlg_tspsdlg.png");
    }

    // 2. Test TCellGridForm
    {
        TCellGridForm dlg;
        dlg.show();
        QCoreApplication::processEvents();
        QPixmap pix = QPixmap::grabWidget(&dlg);
        pix.save("dlg_tcellgridform.png");
    }

    // 3. Test SPSSampleDlg
    {
        SPSSampleDlg dlg;
        dlg.show();
        QCoreApplication::processEvents();
        QPixmap pix = QPixmap::grabWidget(&dlg);
        pix.save("dlg_spssampledlg.png");
    }

    // 4. Test TChangeShowareadlg
    {
        TChangeShowareadlg dlg;
        dlg.show();
        QCoreApplication::processEvents();
        QPixmap pix = QPixmap::grabWidget(&dlg);
        pix.save("dlg_tchangeshowareadlg.png");
    }

    // 5. Test TRealOVTdlg
    {
        TRealOVTdlg dlg;
        dlg.show();
        QCoreApplication::processEvents();
        QPixmap pix = QPixmap::grabWidget(&dlg);
        pix.save("dlg_trealovtdlg.png");
    }

    printf("All dialog screenshots captured successfully!\n");
    return 0;
}
"""

with open("scratch/test_all_dialogs.cpp", "wb") as f:
    f.write(b"\\xef\\xbb\\xbf" + code.encode("utf-8"))
print("Done writing scratch/test_all_dialogs.cpp")
