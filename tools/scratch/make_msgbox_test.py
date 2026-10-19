import os

code = """#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QPixmap>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtCore/QString>
#include <stdio.h>

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

    QMessageBox msgBox;
    msgBox.setWindowTitle(QString::fromLocal8Bit("多采样率对比试验完成"));
    msgBox.setIcon(QMessageBox::Information);

    QString summary = QString::fromLocal8Bit("【多采样率基准对比试验完成】(重复试验: 1 轮, 主随机种子: 42)\\n"
        "1. 30% 采样率 (m=38/128): 规则相干度=0.9125, SA均值=0.4821 (标准差: 0.0000), 相比规则抽样改善率: 47.17%, 相干度比值衰减: 5.54 dB\\n"
        "2. 50% 采样率 (m=64/128): 规则相干度=0.8845, SA均值=0.4182 (标准差: 0.0000), 相比规则抽样改善率: 52.72%, 相干度比值衰减: 6.51 dB\\n"
        "3. 70% 采样率 (m=90/128): 规则相干度=0.8120, SA均值=0.3540 (标准差: 0.0000), 相比规则抽样改善率: 56.40%, 相干度比值衰减: 7.21 dB\\n\\n"
        "【科研成果图件支撑】(全部数据图与结果图集中保存至 experiment_results/all_result_figures 文件夹)：\\n"
        "  ● 图件 01_sa_convergence_curve.png: 模拟退火优化相干度下降收敛曲线\\n"
        "  ● 图件 02_sampling_benchmark_bars.png: 三种采样方案相干度对比柱状图");

    msgBox.setText(summary);
    QPushButton *btnViewFigure = msgBox.addButton(QString::fromLocal8Bit("查看科研成果图件"), QMessageBox::ActionRole);
    QPushButton *btnOk = msgBox.addButton(QString::fromLocal8Bit("确定"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(btnViewFigure);
    msgBox.show();

    QCoreApplication::processEvents();
    QPixmap pix = QPixmap::grabWidget(&msgBox);
    pix.save("test_msgbox_render.png");
    return 0;
}
"""

with open("scratch/test_msgbox_render.cpp", "wb") as f:
    f.write(b"\xef\xbb\xbf" + code.encode("utf-8"))
print("Done writing scratch/test_msgbox_render.cpp")
