#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec) codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    QImage img(900, 500, QImage::Format_RGB32);
    img.fill(qRgb(255, 255, 255));
    QPainter p(&img);

    QFont f1("SimSun", 12);
    p.setFont(f1);
    p.setPen(QColor(0, 0, 0));

    QString s_local = QString::fromLocal8Bit("1. fromLocal8Bit: 模拟退火(SA)优化相干度μ下降收敛曲线");
    QString s_utf8  = QString::fromUtf8("2. fromUtf8: 模拟退火(SA)优化相干度μ下降收敛曲线");
    QString s_en    = QString("3. English: Simulated Annealing (SA) Coherence Convergence Curve");

    p.drawText(20, 50, s_local);
    p.drawText(20, 100, s_utf8);
    p.drawText(20, 150, s_en);

    QFont f2("Microsoft YaHei", 12);
    p.setFont(f2);
    p.drawText(20, 200, QString::fromLocal8Bit("4. YaHei Local: 模拟退火(SA)优化相干度μ下降收敛曲线"));
    p.drawText(20, 250, QString::fromUtf8("5. YaHei Utf8: 模拟退火(SA)优化相干度μ下降收敛曲线"));

    // Also test pure English labels
    p.setFont(QFont("Arial", 12));
    p.drawText(20, 300, QString("6. Arial English: Mutual Coherence mu Convergence vs Iterations"));
    p.drawText(20, 350, QString::fromLocal8Bit("7. fromLocal8Bit without mu: 模拟退火优化相干度下降收敛曲线"));

    p.end();
    img.save("test_painter.png");
    printf("Saved test_painter.png successfully\n");
    return 0;
}
