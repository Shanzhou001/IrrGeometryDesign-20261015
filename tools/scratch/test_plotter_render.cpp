#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <vector>
#include "scientificplotter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set GBK codec
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (!codec) codec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    std::vector<float> curr, best;
    for (int i = 0; i < 50; i++)
    {
        curr.push_back(0.8f - 0.01f * i + (i % 3 == 0 ? 0.02f : -0.01f));
        best.push_back(0.8f - 0.012f * i);
    }

    ScientificPlotter::plotSamplingConvergenceCurve(
        curr, best, 0.96f, 0.52f, 0.50f, 64, 128, "test_curve.png"
    );

    printf("Saved test_curve.png\n");
    return 0;
}
