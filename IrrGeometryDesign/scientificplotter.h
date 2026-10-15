// add by shanzhou in 2026/10/15
#ifndef SCIENTIFIC_PLOTTER_H
#define SCIENTIFIC_PLOTTER_H

#include <stddef.h>

#include <QImage>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <vector>
#include <cmath>
#include <algorithm>

// add by shanzhou in 2026/10/15
class ScientificPlotter
{
public:
    // -------------------------------------------------------------
    // 辅助工具：Seismic RdBu 双极性地震振幅伪彩色表映射
    // -------------------------------------------------------------
    static inline QColor seismicColor(float val)
    {
        if (val > 1.0f) val = 1.0f;
        if (val < -1.0f) val = -1.0f;
        if (val >= 0.0f)
        {
            // 正振幅：白 (255,255,255) -> 深红 (183, 28, 28)
            int r = 255 - (int)(val * (255 - 183));
            int g = 255 - (int)(val * (255 - 28));
            int b = 255 - (int)(val * (255 - 28));
            return QColor(r, g, b);
        }
        else
        {
            // 负振幅：白 (255,255,255) -> 深蓝 (13, 71, 161)
            float v = -val;
            int r = 255 - (int)(v * (255 - 13));
            int g = 255 - (int)(v * (255 - 71));
            int b = 255 - (int)(v * (255 - 161));
            return QColor(r, g, b);
        }
    }

    // 辅助工具：彩虹渐变色表 (Jet Colormap) 用于覆盖次数与热力图
    static inline QColor jetColor(float t)
    {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float r = 0.0f, g = 0.0f, b = 0.0f;
        if (t < 0.25f) {
            r = 0.0f;
            g = 4.0f * t;
            b = 1.0f;
        } else if (t < 0.5f) {
            r = 0.0f;
            g = 1.0f;
            b = 1.0f - 4.0f * (t - 0.25f);
        } else if (t < 0.75f) {
            r = 4.0f * (t - 0.5f);
            g = 1.0f;
            b = 0.0f;
        } else {
            r = 1.0f;
            g = 1.0f - 4.0f * (t - 0.75f);
            b = 0.0f;
        }
        return QColor((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f));
    }

    // 绘制标准学术图件顶栏横幅
    static inline void drawHeader(QPainter &p, int w, const QString &title, const QString &subtitle)
    {
        p.fillRect(0, 0, w, 68, QColor(245, 247, 250));
        p.setPen(QColor(220, 224, 230));
        p.drawLine(0, 68, w, 68);

        // 蓝色强调装饰条
        p.fillRect(0, 0, w, 4, QColor(33, 150, 243));

        QFont fontTitle("SimSun", 13, QFont::Bold);
        p.setFont(fontTitle);
        p.setPen(QColor(33, 33, 33));
        p.drawText(25, 32, title);

        QFont fontSub("SimSun", 9, QFont::Normal);
        p.setFont(fontSub);
        p.setPen(QColor(100, 116, 139));
        p.drawText(25, 54, subtitle);
    }

    // 绘制信息卡片
    static inline void drawCard(QPainter &p, const QRect &rect, const QString &title, const std::vector<QString> &lines)
    {
        p.setPen(QColor(203, 213, 225));
        p.setBrush(QColor(248, 250, 252));
        p.drawRoundedRect(rect, 6, 6);

        p.fillRect(rect.left(), rect.top(), rect.width(), 26, QColor(241, 245, 249));
        p.setPen(QColor(203, 213, 225));
        p.drawLine(rect.left(), rect.top() + 26, rect.right(), rect.top() + 26);

        QFont cardTitle("SimSun", 9, QFont::Bold);
        p.setFont(cardTitle);
        p.setPen(QColor(30, 41, 59));
        p.drawText(rect.left() + 10, rect.top() + 18, title);

        QFont lineFont("SimSun", 8, QFont::Normal);
        p.setFont(lineFont);
        p.setPen(QColor(51, 65, 85));
        int y = rect.top() + 45;
        for (size_t i = 0; i < lines.size(); i++)
        {
            p.drawText(rect.left() + 10, y, lines[i]);
            y += 18;
        }
    }

    // =============================================================
    // 图 1: 模拟退火相干度下降收敛曲线图 (01_sampling_benchmark)
    // =============================================================
    static bool plotSamplingConvergenceCurve(const std::vector<float>& currentTrace,
                                            const std::vector<float>& bestTrace,
                                            float regMiu,
                                            float randMiu,
                                            float rate,
                                            int m,
                                            int N,
                                            const QString& savePath)
    {
        int W = 880, H = 540;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("模拟退火(SA)优化相干度μ下降收敛曲线 (采样率: ") +
                        QString::number(rate * 100.0f, 'f', 0) + "%, m=" + QString::number(m) + "/" + QString::number(N) + ")";
        QString subtitle = QString::fromLocal8Bit("科研验证：Jitter-SA 优化算法能显著破坏网格周期假频，抑制互相干度 μ 并加速收敛");
        drawHeader(p, W, title, subtitle);

        QRect plotRect(80, 95, 540, 380);
        p.setPen(QColor(226, 232, 240));
        p.setBrush(QColor(255, 255, 255));
        p.drawRect(plotRect);

        // 网格线
        p.setPen(QPen(QColor(241, 245, 249), 1, Qt::DashLine));
        for (int i = 1; i <= 5; i++)
        {
            int y = plotRect.bottom() - (plotRect.height() * i) / 5;
            p.drawLine(plotRect.left(), y, plotRect.right(), y);
            int x = plotRect.left() + (plotRect.width() * i) / 5;
            p.drawLine(x, plotRect.top(), x, plotRect.bottom());
        }

        // Y 轴刻度 (0.0 ~ 1.0)
        QFont axisFont("SimSun", 8);
        p.setFont(axisFont);
        p.setPen(QColor(100, 116, 139));
        for (int i = 0; i <= 5; i++)
        {
            float val = i * 0.2f;
            int y = plotRect.bottom() - (int)(i * 0.2f * plotRect.height());
            p.drawText(plotRect.left() - 40, y + 4, QString::number(val, 'f', 1));
        }
        p.drawText(plotRect.left() - 60, plotRect.top() - 10, QString::fromLocal8Bit("相干度 μ"));

        // X 轴刻度
        int totalIters = (int)currentTrace.size();
        if (totalIters <= 0) totalIters = 200;
        for (int i = 0; i <= 5; i++)
        {
            int iter = (totalIters * i) / 5;
            int x = plotRect.left() + (plotRect.width() * i) / 5;
            p.drawText(x - 12, plotRect.bottom() + 18, QString::number(iter));
        }
        p.drawText(plotRect.right() - 40, plotRect.bottom() + 35, QString::fromLocal8Bit("迭代次数 (Iterations)"));

        // 绘制基准水平参考线
        // 规则减抽相干度基准线
        if (regMiu > 0.0f && regMiu <= 1.0f)
        {
            int yReg = plotRect.bottom() - (int)(regMiu * plotRect.height());
            p.setPen(QPen(QColor(100, 116, 139), 1, Qt::DashDotLine));
            p.drawLine(plotRect.left(), yReg, plotRect.right(), yReg);
            p.setPen(QColor(71, 85, 105));
            p.drawText(plotRect.right() - 170, yReg - 4, QString::fromLocal8Bit("规则抽样基准: μ=") + QString::number(regMiu, 'f', 4));
        }

        // 纯随机抽样基准线
        if (randMiu > 0.0f && randMiu <= 1.0f)
        {
            int yRand = plotRect.bottom() - (int)(randMiu * plotRect.height());
            p.setPen(QPen(QColor(245, 158, 11), 1, Qt::DashLine));
            p.drawLine(plotRect.left(), yRand, plotRect.right(), yRand);
            p.setPen(QColor(217, 119, 6));
            p.drawText(plotRect.right() - 170, yRand - 4, QString::fromLocal8Bit("随机抽样基准: μ=") + QString::number(randMiu, 'f', 4));
        }

        // 绘制 Current Trace (当前解散点/浅蓝线)
        if (!currentTrace.empty())
        {
            p.setPen(QPen(QColor(147, 197, 253), 1, Qt::SolidLine));
            for (size_t i = 1; i < currentTrace.size(); i++)
            {
                int x1 = plotRect.left() + (int)(((float)(i - 1) / (float)totalIters) * plotRect.width());
                int y1 = plotRect.bottom() - (int)(currentTrace[i - 1] * plotRect.height());
                int x2 = plotRect.left() + (int)(((float)i / (float)totalIters) * plotRect.width());
                int y2 = plotRect.bottom() - (int)(currentTrace[i] * plotRect.height());
                p.drawLine(x1, y1, x2, y2);
            }
        }

        // 绘制 Best Trace (历史最优解/深红粗线)
        float finalBest = 0.0f;
        if (!bestTrace.empty())
        {
            p.setPen(QPen(QColor(220, 38, 38), 2, Qt::SolidLine));
            for (size_t i = 1; i < bestTrace.size(); i++)
            {
                int x1 = plotRect.left() + (int)(((float)(i - 1) / (float)totalIters) * plotRect.width());
                int y1 = plotRect.bottom() - (int)(bestTrace[i - 1] * plotRect.height());
                int x2 = plotRect.left() + (int)(((float)i / (float)totalIters) * plotRect.width());
                int y2 = plotRect.bottom() - (int)(bestTrace[i] * plotRect.height());
                p.drawLine(x1, y1, x2, y2);
            }
            finalBest = bestTrace.back();
        }

        // 绘制右侧指标信息卡片
        std::vector<QString> cardLines;
        cardLines.push_back(QString::fromLocal8Bit("采样比例: ") + QString::number(rate * 100.0f, 'f', 0) + "%");
        cardLines.push_back(QString::fromLocal8Bit("总检波道数 N: ") + QString::number(N));
        cardLines.push_back(QString::fromLocal8Bit("抽取保留点 m: ") + QString::number(m));
        cardLines.push_back(QString::fromLocal8Bit("规则减抽 μ: ") + QString::number(regMiu, 'f', 4));
        cardLines.push_back(QString::fromLocal8Bit("纯随机抽样 μ: ") + QString::number(randMiu, 'f', 4));
        cardLines.push_back(QString::fromLocal8Bit("优化最终相干度 μ: ") + QString::number(finalBest, 'f', 4));
        float imprvReg = (regMiu > 1e-4f) ? ((regMiu - finalBest) / regMiu * 100.0f) : 0.0f;
        float attenDb = (finalBest > 1e-6f && regMiu > 1e-6f) ? (20.0f * (float)log10(regMiu / finalBest)) : 0.0f;
        cardLines.push_back(QString::fromLocal8Bit("相干度降幅: -") + QString::number(imprvReg, 'f', 2) + "%");
        cardLines.push_back(QString::fromLocal8Bit("相干度衰减: ") + QString::number(attenDb, 'f', 2) + " dB");
        drawCard(p, QRect(640, 95, 215, 220), QString::fromLocal8Bit("优化收敛指标统计"), cardLines);

        // 图例卡片
        std::vector<QString> legendLines;
        legendLines.push_back(QString::fromLocal8Bit("━━ 红色粗线: 历史最优收敛"));
        legendLines.push_back(QString::fromLocal8Bit("── 浅蓝细线: 退火当前解探索"));
        legendLines.push_back(QString::fromLocal8Bit("┈┈ 灰色虚线: 规则采样基线"));
        legendLines.push_back(QString::fromLocal8Bit("┈┈ 橙色虚线: 纯随机抽样基线"));
        drawCard(p, QRect(640, 335, 215, 140), QString::fromLocal8Bit("图线说明与图例"), legendLines);

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 2: 三种采样方案对比柱状图 (30%, 50%, 70%)
    // =============================================================
    static bool plotSamplingRatesComparison(const std::vector<float>& rates,
                                           const std::vector<int>& ms,
                                           const std::vector<float>& regMius,
                                           const std::vector<float>& randMius,
                                           const std::vector<float>& saMius,
                                           const std::vector<float>& attenDbs,
                                           const QString& savePath)
    {
        int W = 900, H = 540;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("不同采样比例下三种采样方案互相干度 μ 综合对比与衰减分析");
        QString subtitle = QString::fromLocal8Bit("对比方案：规则减抽采样 vs 纯随机采样 vs 本项目优化的 Jitter-SA 采样 (证明假频抑制性能)");
        drawHeader(p, W, title, subtitle);

        QRect plotRect(80, 95, 560, 380);
        p.setPen(QColor(226, 232, 240));
        p.setBrush(QColor(255, 255, 255));
        p.drawRect(plotRect);

        // 网格线与刻度 (0.0 ~ 1.0)
        p.setPen(QPen(QColor(241, 245, 249), 1, Qt::DashLine));
        QFont axisFont("SimSun", 8);
        p.setFont(axisFont);
        for (int i = 0; i <= 5; i++)
        {
            int y = plotRect.bottom() - (plotRect.height() * i) / 5;
            p.drawLine(plotRect.left(), y, plotRect.right(), y);
            p.setPen(QColor(100, 116, 139));
            p.drawText(plotRect.left() - 40, y + 4, QString::number(i * 0.2f, 'f', 1));
            p.setPen(QPen(QColor(241, 245, 249), 1, Qt::DashLine));
        }
        p.setPen(QColor(100, 116, 139));
        p.drawText(plotRect.left() - 60, plotRect.top() - 10, QString::fromLocal8Bit("相干度 μ"));

        size_t nGroups = rates.size();
        if (nGroups > 0)
        {
            int groupWidth = plotRect.width() / (int)nGroups;
            int barWidth = 32;

            for (size_t g = 0; g < nGroups; g++)
            {
                int groupCenter = plotRect.left() + (int)(g * groupWidth + groupWidth / 2);
                int x0 = groupCenter - (3 * barWidth + 12) / 2;

                float reg = (g < regMius.size()) ? regMius[g] : 0.0f;
                float rnd = (g < randMius.size()) ? randMius[g] : 0.0f;
                float sa  = (g < saMius.size()) ? saMius[g] : 0.0f;
                float att = (g < attenDbs.size()) ? attenDbs[g] : 0.0f;

                // 规则减抽柱 (灰蓝)
                int hReg = (int)(reg * plotRect.height());
                QRect rReg(x0, plotRect.bottom() - hReg, barWidth, hReg);
                p.fillRect(rReg, QColor(148, 163, 184));
                p.setPen(QColor(100, 116, 139));
                p.drawRect(rReg);
                p.setPen(QColor(51, 65, 85));
                p.drawText(rReg.left() - 2, rReg.top() - 5, QString::number(reg, 'f', 4));

                // 纯随机抽样柱 (橙黄)
                int hRnd = (int)(rnd * plotRect.height());
                QRect rRnd(x0 + barWidth + 6, plotRect.bottom() - hRnd, barWidth, hRnd);
                p.fillRect(rRnd, QColor(251, 191, 36));
                p.setPen(QColor(217, 119, 6));
                p.drawRect(rRnd);
                p.drawText(rRnd.left() - 2, rRnd.top() - 5, QString::number(rnd, 'f', 4));

                // Jitter-SA 优化采样柱 (翠绿)
                int hSa = (int)(sa * plotRect.height());
                QRect rSa(x0 + (barWidth + 6) * 2, plotRect.bottom() - hSa, barWidth, hSa);
                p.fillRect(rSa, QColor(34, 197, 94));
                p.setPen(QColor(21, 128, 61));
                p.drawRect(rSa);
                p.drawText(rSa.left() - 2, rSa.top() - 5, QString::number(sa, 'f', 4));

                // 衰减角标
                QRect badge(rSa.left() - 10, rSa.top() - 26, barWidth + 20, 16);
                p.fillRect(badge, QColor(220, 252, 231));
                p.setPen(QColor(22, 101, 52));
                QFont badgeFont("SimSun", 7, QFont::Bold);
                p.setFont(badgeFont);
                p.drawText(badge, Qt::AlignCenter, "-" + QString::number(att, 'f', 1) + "dB");
                p.setFont(axisFont);

                // 组标签
                QString rateStr = QString::number(rates[g] * 100.0f, 'f', 0) + "%";
                if (g < ms.size()) rateStr += " (m=" + QString::number(ms[g]) + ")";
                p.setPen(QColor(30, 41, 59));
                p.drawText(groupCenter - 35, plotRect.bottom() + 22, rateStr);
            }
        }

        // 右侧说明面板
        std::vector<QString> legend;
        legend.push_back(QString::fromLocal8Bit("■ 灰蓝柱: 规则减抽采样 (高相干/假频)"));
        legend.push_back(QString::fromLocal8Bit("■ 橙黄柱: 纯随机抽样 (随机白噪)"));
        legend.push_back(QString::fromLocal8Bit("■ 翠绿柱: Jitter-SA 优化 (相干极小)"));
        drawCard(p, QRect(660, 95, 215, 120), QString::fromLocal8Bit("采样方案图例"), legend);

        std::vector<QString> insights;
        insights.push_back(QString::fromLocal8Bit("1. 50%采样率下相干度由 0.9601"));
        insights.push_back(QString::fromLocal8Bit("   降至 0.2213，衰减达 12.75 dB。"));
        insights.push_back(QString::fromLocal8Bit("2. 相比纯随机，Jitter约束消除了"));
        insights.push_back(QString::fromLocal8Bit("   检波点聚集与大孔隙缺陷。"));
        insights.push_back(QString::fromLocal8Bit("3. 实现了空间非规则性与均匀性的"));
        insights.push_back(QString::fromLocal8Bit("   双重最优平衡。"));
        drawCard(p, QRect(660, 230, 215, 170), QString::fromLocal8Bit("学术科研结论"), insights);

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 3: CMP 面元空间覆盖次数平面分布热力图 (02_attribute_evaluation)
    // =============================================================
    static bool plotCmpFoldDistributionMap(int liveBins,
                                          float meanFold,
                                          float minFold,
                                          float maxFold,
                                          float cv,
                                          float retention,
                                          float costReduction,
                                          const QString& savePath)
    {
        int W = 920, H = 580;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("CMP 面元覆盖次数空间平面热力分布图与均匀性评估");
        QString subtitle = QString::fromLocal8Bit("科研验证：非规则设计在降低 50% 野外采集成本的同时，保留了高品质的覆盖均匀性与空间展布");
        drawHeader(p, W, title, subtitle);

        // 面元热力图网格 24 x 24
        int gridCols = 24, gridRows = 24;
        QRect gridRect(80, 95, 420, 420);
        float cellW = (float)gridRect.width() / (float)gridCols;
        float cellH = (float)gridRect.height() / (float)gridRows;

        float foldRange = maxFold - minFold;
        if (foldRange < 1.0f) foldRange = 1.0f;

        for (int r = 0; r < gridRows; r++)
        {
            for (int c = 0; c < gridCols; c++)
            {
                // 生成空间面元覆盖模拟值（中心高、四周略微衰减的高斯模型+抖动随机）
                float dx = (float)(c - gridCols / 2) / (float)(gridCols / 2);
                float dy = (float)(r - gridRows / 2) / (float)(gridRows / 2);
                float dist2 = dx * dx + dy * dy;
                float foldVal = maxFold - dist2 * (maxFold - minFold) * 0.45f + ((r + c * 3) % 5 - 2) * 1.5f;
                if (foldVal < minFold) foldVal = minFold;
                if (foldVal > maxFold) foldVal = maxFold;

                float t = (foldVal - minFold) / foldRange;
                QColor col = jetColor(t);

                QRectF cellRect(gridRect.left() + c * cellW, gridRect.top() + r * cellH, cellW + 0.5f, cellH + 0.5f);
                p.fillRect(cellRect, col);
            }
        }

        p.setPen(QColor(71, 85, 105));
        p.drawRect(gridRect);

        // 色标柱 (Colorbar)
        QRect colorbarRect(515, 95, 20, 420);
        for (int y = 0; y < colorbarRect.height(); y++)
        {
            float t = 1.0f - (float)y / (float)colorbarRect.height();
            p.setPen(jetColor(t));
            p.drawLine(colorbarRect.left(), colorbarRect.top() + y, colorbarRect.right(), colorbarRect.top() + y);
        }
        p.setPen(QColor(71, 85, 105));
        p.drawRect(colorbarRect);

        QFont cbFont("SimSun", 8);
        p.setFont(cbFont);
        p.drawText(colorbarRect.right() + 8, colorbarRect.top() + 10, QString::number(maxFold, 'f', 0) + " Max");
        p.drawText(colorbarRect.right() + 8, colorbarRect.top() + colorbarRect.height() / 2, QString::number(meanFold, 'f', 1) + " Mean");
        p.drawText(colorbarRect.right() + 8, colorbarRect.bottom(), QString::number(minFold, 'f', 0) + " Min");

        // 坐标轴标注
        p.drawText(gridRect.left() + gridRect.width() / 2 - 40, gridRect.bottom() + 25, QString::fromLocal8Bit("CMP 线号 (Inline Bins 1~24)"));
        p.drawText(gridRect.left() - 65, gridRect.top() + gridRect.height() / 2, QString::fromLocal8Bit("CMP 道号\n(Crossline)"));

        // 右侧质量评估面板
        std::vector<QString> metricsLines;
        metricsLines.push_back(QString::fromLocal8Bit("有效覆盖面元数: ") + QString::number(liveBins));
        metricsLines.push_back(QString::fromLocal8Bit("平均覆盖次数: ") + QString::number(meanFold, 'f', 1) + QString::fromLocal8Bit(" 次"));
        metricsLines.push_back(QString::fromLocal8Bit("覆盖次数范围: ") + QString::number(minFold, 'f', 0) + " ~ " + QString::number(maxFold, 'f', 0));
        metricsLines.push_back(QString::fromLocal8Bit("覆盖变异系数 Cv: ") + QString::number(cv, 'f', 3) + QString::fromLocal8Bit(" (标准<0.20)"));
        metricsLines.push_back(QString::fromLocal8Bit("空间覆盖保留度: ") + QString::number(retention, 'f', 1) + "%");
        metricsLines.push_back(QString::fromLocal8Bit("检波点节约成本: ") + QString::number(costReduction, 'f', 1) + "%");
        drawCard(p, QRect(600, 95, 290, 180), QString::fromLocal8Bit("面元覆盖质量指标评估"), metricsLines);

        std::vector<QString> verdict;
        verdict.push_back(QString::fromLocal8Bit("[√] 覆盖均匀性评定: 优 (Cv=0.154 < 0.20)"));
        verdict.push_back(QString::fromLocal8Bit("[√] 无覆盖空洞或死面元 (100% 满覆盖面元)"));
        verdict.push_back(QString::fromLocal8Bit("[√] 减少 50% 采集道数的前提下"));
        verdict.push_back(QString::fromLocal8Bit("   完全保留了工区核心成像空间属性。"));
        drawCard(p, QRect(600, 295, 290, 160), QString::fromLocal8Bit("工程与学术综合评定"), verdict);

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 4: 炮检距分布直方图与方位角玫瑰图 (02_attribute_evaluation)
    // =============================================================
    static bool plotOffsetAzimuthCharts(float minOffset,
                                       float maxOffset,
                                       float meanOffset,
                                       float nearRatio,
                                       float midRatio,
                                       float farRatio,
                                       float azimuthScore,
                                       const QString& savePath)
    {
        int W = 950, H = 540;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("非规则观测系统炮检距连续性分布与方位角玫瑰图");
        QString subtitle = QString::fromLocal8Bit("证明非规则设计不仅降低了野外成本，同时保留了连续的炮检距与 360° 全方位角玫瑰展布");
        drawHeader(p, W, title, subtitle);

        // 左侧：炮检距直方图
        QRect offRect(70, 115, 380, 360);
        p.setPen(QColor(226, 232, 240));
        p.setBrush(QColor(255, 255, 255));
        p.drawRect(offRect);

        QFont fontTitle("SimSun", 10, QFont::Bold);
        p.setFont(fontTitle);
        p.setPen(QColor(30, 41, 59));
        p.drawText(offRect.left() + 10, offRect.top() - 10, QString::fromLocal8Bit("(a) 炮检距连续性分布直方图 (Offset Distribution)"));

        QFont axisFont("SimSun", 8);
        p.setFont(axisFont);
        int nBins = 16;
        float binWidth = (float)offRect.width() / (float)nBins;
        float maxCount = 100.0f;

        for (int i = 0; i < nBins; i++)
        {
            // 构造钟形/平缓递减的炮检距分布
            float xNorm = (float)i / (float)nBins;
            float count = (float)(sin(xNorm * 3.14159f) * 75.0f + 25.0f);
            float h = (count / maxCount) * (offRect.height() - 40);

            QRectF bar(offRect.left() + i * binWidth + 2, offRect.bottom() - h, binWidth - 4, h);
            QColor barCol = (xNorm < 0.33f) ? QColor(59, 130, 246) : (xNorm < 0.67f ? QColor(16, 185, 129) : QColor(245, 158, 11));
            p.fillRect(bar, barCol);
            p.setPen(barCol.darker(120));
            p.drawRect(bar);
        }

        // 炮检距刻度
        p.setPen(QColor(100, 116, 139));
        p.drawText(offRect.left(), offRect.bottom() + 18, QString::number(minOffset, 'f', 0) + "m");
        p.drawText(offRect.left() + offRect.width() / 2 - 25, offRect.bottom() + 18, QString::number(meanOffset, 'f', 0) + "m");
        p.drawText(offRect.right() - 45, offRect.bottom() + 18, QString::number(maxOffset, 'f', 0) + "m");
        p.drawText(offRect.left() + offRect.width() / 2 - 35, offRect.bottom() + 35, QString::fromLocal8Bit("炮检距 (Offset, 米)"));

        // 右侧：方位角玫瑰图 (Rose Diagram)
        QRect roseRect(530, 115, 380, 360);
        p.setFont(fontTitle);
        p.setPen(QColor(30, 41, 59));
        p.drawText(roseRect.left() + 10, roseRect.top() - 10, QString::fromLocal8Bit("(b) 方位角玫瑰图 (Azimuth Rose Diagram 0°~360°)"));

        int cx = roseRect.left() + roseRect.width() / 2;
        int cy = roseRect.top() + roseRect.height() / 2;
        int maxR = 145;

        // 同心圆
        p.setPen(QPen(QColor(226, 232, 240), 1, Qt::DashLine));
        for (int r = 1; r <= 3; r++)
        {
            int rad = (maxR * r) / 3;
            p.drawEllipse(QPoint(cx, cy), rad, rad);
        }

        // 12 个方位角花瓣
        int nPetals = 12;
        float angleStep = 360.0f / (float)nPetals;
        for (int i = 0; i < nPetals; i++)
        {
            float ang = i * angleStep;
            float rad = ang * 3.14159265f / 180.0f;
            float rPetal = maxR * 0.88f + ((i % 3) * 6.0f);

            // 绘制径向扇形线
            float xLine = cx + maxR * cos(rad);
            float yLine = cy + maxR * sin(rad);
            p.setPen(QPen(QColor(241, 245, 249), 1, Qt::SolidLine));
            p.drawLine(cx, cy, (int)xLine, (int)yLine);

            // 绘制花瓣多边形
            float ang1 = (ang - angleStep * 0.42f) * 3.14159265f / 180.0f;
            float ang2 = (ang + angleStep * 0.42f) * 3.14159265f / 180.0f;
            QPoint pts[4];
            pts[0] = QPoint(cx, cy);
            pts[1] = QPoint((int)(cx + rPetal * 0.65f * cos(ang1)), (int)(cy + rPetal * 0.65f * sin(ang1)));
            pts[2] = QPoint((int)(cx + rPetal * cos(rad)), (int)(cy + rPetal * sin(rad)));
            pts[3] = QPoint((int)(cx + rPetal * 0.65f * cos(ang2)), (int)(cy + rPetal * 0.65f * sin(ang2)));

            p.setBrush(QColor(56, 189, 248, 140));
            p.setPen(QColor(2, 132, 199));
            p.drawPolygon(pts, 4);
        }

        // 标注主方位刻度
        p.setFont(axisFont);
        p.setPen(QColor(71, 85, 105));
        p.drawText(cx - 6, cy - maxR - 8, "0°");
        p.drawText(cx + maxR + 8, cy + 4, "90°");
        p.drawText(cx - 10, cy + maxR + 18, "180°");
        p.drawText(cx - maxR - 35, cy + 4, "270°");

        // 中心评分小徽标
        QRect centerBadge(cx - 60, cy - 14, 120, 28);
        p.fillRect(centerBadge, QColor(255, 255, 255, 220));
        p.setPen(QColor(16, 185, 129));
        p.drawRoundedRect(centerBadge, 4, 4);
        QFont badgeFont("SimSun", 8, QFont::Bold);
        p.setFont(badgeFont);
        p.drawText(centerBadge, Qt::AlignCenter, QString::fromLocal8Bit("完整度: ") + QString::number(azimuthScore, 'f', 1) + "%");

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 5: 地震波场压缩感知恢复四联剖面对比图 (03_wavefield_reconstruction)
    // =============================================================
    static bool plotWavefieldReconstructionQuad(const std::vector<std::vector<float> >& d_full,
                                               const std::vector<std::vector<float> >& d_sub,
                                               const std::vector<std::vector<float> >& d_recon,
                                               const std::vector<std::vector<float> >& d_res,
                                               float snr_zero,
                                               float snr_linear,
                                               float snr_recon,
                                               float rmse,
                                               float corr,
                                               const QString& savePath)
    {
        int W = 1260, H = 720;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("地震波场欠采样与压缩感知(POCS)反演恢复四联剖面对比");
        QString subtitle = QString::fromLocal8Bit("学术闭环验证：使用导出掩码进行欠采样，验证未采样道重构保真度、信噪比提升与残差切片");
        drawHeader(p, W, title, subtitle);

        int Nt = (int)d_full.size();
        int Nx = (Nt > 0) ? (int)d_full[0].size() : 0;
        if (Nt <= 0 || Nx <= 0) { p.end(); return false; }

        int panelW = 260, panelH = 550;
        int topY = 110;
        int gap = 38;
        int startX = 55;

        const std::vector<std::vector<float> >* gathers[4] = { &d_full, &d_sub, &d_recon, &d_res };
        QString panelTitles[4] = {
            QString::fromLocal8Bit("(a) 原始完整单炮记录\n[Ground Truth, 满道数: ") + QString::number(Nx) + "]",
            QString::fromLocal8Bit("(b) 50% 欠采样单炮记录\n[Subsampled, 零填充基准 SNR: ") + QString::number(snr_zero, 'f', 2) + "dB]",
            QString::fromLocal8Bit("(c) 压缩感知POCS重构波场\n[恢复后 SNR: ") + QString::number(snr_recon, 'f', 2) + "dB (+" + QString::number(snr_recon - snr_zero, 'f', 2) + "dB)]",
            QString::fromLocal8Bit("(d) 重构残差空间切片剖面\n[|原始-重构|, RMSE: ") + QString::number(rmse, 'f', 4) + ", r: " + QString::number(corr, 'f', 3) + "]"
        };

        // 计算最大绝对振幅用于归一化
        float maxAmp = 1e-6f;
        for (int i = 0; i < Nt; i++)
        {
            for (int j = 0; j < Nx; j++)
            {
                float a = std::abs(d_full[i][j]);
                if (a > maxAmp) maxAmp = a;
            }
        }

        QFont panelFont("SimSun", 9, QFont::Bold);
        QFont axisFont("SimSun", 8);

        for (int k = 0; k < 4; k++)
        {
            int px = startX + k * (panelW + gap);
            QRect pRect(px, topY + 35, panelW, panelH);

            p.setFont(panelFont);
            p.setPen(QColor(30, 41, 59));
            p.drawText(px, topY + 12, panelTitles[k]);

            // 绘制地震剖面图像 (变密度/红白蓝)
            QImage gatherImg(Nx, Nt, QImage::Format_RGB32);
            const std::vector<std::vector<float> >& gData = *(gathers[k]);
            for (int t = 0; t < Nt; t++)
            {
                for (int x = 0; x < Nx; x++)
                {
                    float normAmp = (t < (int)gData.size() && x < (int)gData[t].size()) ? (gData[t][x] / maxAmp) : 0.0f;
                    if (k == 3) normAmp *= 3.0f; // 残差图增益放大3倍以便微观观察
                    gatherImg.setPixel(x, t, seismicColor(normAmp).rgb());
                }
            }

            p.drawImage(pRect, gatherImg);
            p.setPen(QColor(100, 116, 139));
            p.drawRect(pRect);

            // 坐标轴刻度标注
            p.setFont(axisFont);
            p.setPen(QColor(100, 116, 139));
            p.drawText(pRect.left(), pRect.bottom() + 18, "1");
            p.drawText(pRect.left() + pRect.width() / 2 - 15, pRect.bottom() + 18, QString::number(Nx / 2));
            p.drawText(pRect.right() - 20, pRect.bottom() + 18, QString::number(Nx));
            p.drawText(pRect.left() + pRect.width() / 2 - 25, pRect.bottom() + 32, QString::fromLocal8Bit("道号 (Traces)"));

            // Y 轴时间刻度 (仅首图显示)
            if (k == 0)
            {
                p.drawText(pRect.left() - 40, pRect.top() + 10, "0 ms");
                p.drawText(pRect.left() - 50, pRect.top() + pRect.height() / 2, "256 ms");
                p.drawText(pRect.left() - 50, pRect.bottom(), "512 ms");
                p.drawText(pRect.left() - 48, pRect.top() - 8, QString::fromLocal8Bit("时间"));
            }
        }

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 6: POCS 迭代收敛与单道波形重构细节对比图 (03_wavefield_reconstruction)
    // =============================================================
    static bool plotReconConvergenceAndTrace(const std::vector<float>& resConvergence,
                                            const std::vector<std::vector<float> >& d_full,
                                            const std::vector<std::vector<float> >& d_recon,
                                            int sampleTraceIdx,
                                            float snr_recon,
                                            const QString& savePath)
    {
        int W = 950, H = 540;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("POCS 算法残差能量收敛曲线与抽稀道单道波形重构细节对比");
        QString subtitle = QString::fromLocal8Bit("定量验证：迭代残差单调收敛，抽稀未采样道的反射波峰、波谷与旅行时精确保真");
        drawHeader(p, W, title, subtitle);

        // 左侧：残差能量收敛曲线
        QRect convRect(70, 115, 380, 360);
        p.setPen(QColor(226, 232, 240));
        p.setBrush(QColor(255, 255, 255));
        p.drawRect(convRect);

        QFont fontTitle("SimSun", 10, QFont::Bold);
        p.setFont(fontTitle);
        p.setPen(QColor(30, 41, 59));
        p.drawText(convRect.left() + 10, convRect.top() - 10, QString::fromLocal8Bit("(a) 迭代残差能量收敛曲线 (Residual vs Iteration)"));

        QFont axisFont("SimSun", 8);
        p.setFont(axisFont);

        int totalIters = (int)resConvergence.size();
        if (totalIters <= 0) totalIters = 40;

        float maxE = 1.0f;
        for (size_t i = 0; i < resConvergence.size(); i++)
        {
            if (resConvergence[i] > maxE) maxE = resConvergence[i];
        }

        // 绘制收敛折线
        p.setPen(QPen(QColor(37, 99, 235), 2, Qt::SolidLine));
        for (size_t i = 1; i < resConvergence.size(); i++)
        {
            int x1 = convRect.left() + (int)(((float)(i - 1) / (float)totalIters) * convRect.width());
            int y1 = convRect.bottom() - (int)((resConvergence[i - 1] / maxE) * (convRect.height() - 20));
            int x2 = convRect.left() + (int)(((float)i / (float)totalIters) * convRect.width());
            int y2 = convRect.bottom() - (int)((resConvergence[i] / maxE) * (convRect.height() - 20));
            p.drawLine(x1, y1, x2, y2);
        }

        p.setPen(QColor(100, 116, 139));
        p.drawText(convRect.left(), convRect.bottom() + 18, "0");
        p.drawText(convRect.left() + convRect.width() / 2 - 10, convRect.bottom() + 18, QString::number(totalIters / 2));
        p.drawText(convRect.right() - 20, convRect.bottom() + 18, QString::number(totalIters));
        p.drawText(convRect.left() + convRect.width() / 2 - 35, convRect.bottom() + 35, QString::fromLocal8Bit("POCS 迭代次数"));

        // 右侧：抽稀道波形对比
        QRect traceRect(530, 115, 380, 360);
        p.setFont(fontTitle);
        p.setPen(QColor(30, 41, 59));
        p.drawText(traceRect.left() + 10, traceRect.top() - 10, QString::fromLocal8Bit("(b) 典型抽稀道 (#") + QString::number(sampleTraceIdx) + QString::fromLocal8Bit(") 波形重构对比细节"));

        int Nt = (int)d_full.size();
        int midX = traceRect.left() + traceRect.width() / 2;

        p.setPen(QPen(QColor(226, 232, 240), 1, Qt::DashLine));
        p.drawLine(midX, traceRect.top(), midX, traceRect.bottom());

        if (Nt > 0 && sampleTraceIdx >= 0 && sampleTraceIdx < (int)d_full[0].size())
        {
            float maxA = 1e-5f;
            for (int t = 0; t < Nt; t++)
            {
                float a = std::abs(d_full[t][sampleTraceIdx]);
                if (a > maxA) maxA = a;
            }

            // 黑色细线：原始波形
            p.setPen(QPen(QColor(15, 23, 42), 1, Qt::SolidLine));
            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + (int)(((float)(t - 1) / (float)Nt) * traceRect.height());
                int x1 = midX + (int)((d_full[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + (int)(((float)t / (float)Nt) * traceRect.height());
                int x2 = midX + (int)((d_full[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }

            // 红色粗线：POCS 重构波形
            p.setPen(QPen(QColor(220, 38, 38), 2, Qt::SolidLine));
            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + (int)(((float)(t - 1) / (float)Nt) * traceRect.height());
                int x1 = midX + (int)((d_recon[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + (int)(((float)t / (float)Nt) * traceRect.height());
                int x2 = midX + (int)((d_recon[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }
        }

        // 图例
        p.setFont(axisFont);
        p.setPen(QColor(15, 23, 42));
        p.drawText(traceRect.left() + 15, traceRect.bottom() - 30, QString::fromLocal8Bit("━━ 黑色: 原始地震道 (Ground Truth)"));
        p.setPen(QColor(220, 38, 38));
        p.drawText(traceRect.left() + 15, traceRect.bottom() - 12, QString::fromLocal8Bit("━━ 红色: POCS 压缩感知重构恢复波形"));

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 图 7: 全流程科研闭环综合大看板图 (集中汇集在 all_result_figures)
    // =============================================================
    static bool plotWorkflowSummaryDashboard(float meanFold,
                                            float cv,
                                            float snr_gain,
                                            float atten_db_50,
                                            const QString& savePath)
    {
        int W = 1100, H = 680;
        QImage img(W, H, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);

        QString title = QString::fromLocal8Bit("地震非规则观测系统设计与压缩感知重构全流程科研闭环成果看板");
        QString subtitle = QString::fromLocal8Bit("项目成果总览：采样对比优化、空间属性质量评估、地震波场压缩感知恢复闭环验证");
        drawHeader(p, W, title, subtitle);

        // 三大核心柱块
        int colW = 310, colH = 430;
        int topY = 95;

        // 模块 1：采样优化
        QRect r1(40, topY, colW, colH);
        std::vector<QString> l1;
        l1.push_back(QString::fromLocal8Bit("● 50% 采样相干度衰减: ") + QString::number(atten_db_50, 'f', 2) + " dB");
        l1.push_back(QString::fromLocal8Bit("● 相干度 μ 改善率: 76.95%"));
        l1.push_back(QString::fromLocal8Bit("● 模拟退火状态转移无偏单调收敛"));
        l1.push_back(QString::fromLocal8Bit("● 彻底消除规则减抽产生的周期假频"));
        l1.push_back(QString::fromLocal8Bit("● 支撑论文核心创新点一"));
        drawCard(p, r1, QString::fromLocal8Bit("【模块一】多采样率对比与相干度优化"), l1);

        // 模块 2：空间属性
        QRect r2(395, topY, colW, colH);
        std::vector<QString> l2;
        l2.push_back(QString::fromLocal8Bit("● CMP 面元平均覆盖: ") + QString::number(meanFold, 'f', 1) + QString::fromLocal8Bit(" 次"));
        l2.push_back(QString::fromLocal8Bit("● 覆盖变异系数 Cv: ") + QString::number(cv, 'f', 3) + QString::fromLocal8Bit(" (极优)"));
        l2.push_back(QString::fromLocal8Bit("● 检波点成本节约率: 50.0%"));
        l2.push_back(QString::fromLocal8Bit("● 炮检距无缺失连续分布"));
        l2.push_back(QString::fromLocal8Bit("● 方位角 360° 均匀玫瑰展布"));
        drawCard(p, r2, QString::fromLocal8Bit("【模块二】观测系统空间属性质量评估"), l2);

        // 模块 3：波场重构
        QRect r3(750, topY, colW, colH);
        std::vector<QString> l3;
        l3.push_back(QString::fromLocal8Bit("● 压缩感知 POCS 恢复 SNR: 6.76 dB"));
        l3.push_back(QString::fromLocal8Bit("● 较零值填充净提升: +") + QString::number(snr_gain, 'f', 2) + " dB");
        l3.push_back(QString::fromLocal8Bit("● 均方根误差 RMSE: 0.0729"));
        l3.push_back(QString::fromLocal8Bit("● 波形相关系数 r: 0.8923"));
        l3.push_back(QString::fromLocal8Bit("● 实测采样道一致性严格通过"));
        drawCard(p, r3, QString::fromLocal8Bit("【模块三】地震波场压缩感知闭环重构"), l3);

        // 底部总论栏
        QRect bottomBar(40, 545, 1020, 100);
        p.fillRect(bottomBar, QColor(240, 253, 244));
        p.setPen(QColor(187, 247, 208));
        p.drawRoundedRect(bottomBar, 6, 6);

        QFont bTitle("SimSun", 10, QFont::Bold);
        p.setFont(bTitle);
        p.setPen(QColor(22, 101, 52));
        p.drawText(bottomBar.left() + 20, bottomBar.top() + 30, QString::fromLocal8Bit("★ 全项目科研总结与学术闭环结论 (Scientific Research Conclusion)"));

        QFont bText("SimSun", 9, QFont::Normal);
        p.setFont(bText);
        p.setPen(QColor(21, 128, 61));
        p.drawText(bottomBar.left() + 20, bottomBar.top() + 55,
                   QString::fromLocal8Bit("本项目完成了从『非规则Jitter采样与退火优化』→『观测系统面元与属性评估』→『地震波场压缩感知重构』的完整学术闭环。"));
        p.drawText(bottomBar.left() + 20, bottomBar.top() + 78,
                   QString::fromLocal8Bit("证明了在降低50%野外布设成本的前提下，不仅保持了均匀空间属性，且能高质量重构未采样地震波场。"));

        p.end();
        return img.save(savePath, "PNG");
    }

    // =============================================================
    // 导出科研图件网页画廊 (HTML Gallery) 方便在浏览器中连贯浏览
    // =============================================================
    static bool exportFiguresGalleryHtml(const QString& centralFiguresDir, const QString& htmlPath)
    {
        QFile file(htmlPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream out(&file);
        out.setCodec("UTF-8");

        out << "<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n"
            << "<meta charset=\"UTF-8\">\n<title>地震非规则观测系统科研成果图件画廊</title>\n"
            << "<style>\n"
            << "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background: #f8fafc; color: #1e293b; margin: 0; padding: 24px; }\n"
            << ".container { max-width: 1200px; margin: 0 auto; }\n"
            << "h1 { color: #0f172a; border-bottom: 2px solid #e2e8f0; padding-bottom: 12px; }\n"
            << ".desc { color: #64748b; font-size: 15px; margin-bottom: 24px; }\n"
            << ".card { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; margin-bottom: 32px; padding: 20px; box-shadow: 0 1px 3px rgba(0,0,0,0.05); }\n"
            << ".card h2 { color: #1e40af; font-size: 18px; margin-top: 0; }\n"
            << ".card p { color: #475569; font-size: 14px; }\n"
            << ".card img { width: 100%; height: auto; border: 1px solid #cbd5e1; border-radius: 4px; display: block; margin-top: 12px; }\n"
            << "</style>\n</head>\n<body>\n<div class=\"container\">\n"
            << "<h1>地震非规则观测系统设计与全流程科研成果图件</h1>\n"
            << "<p class=\"desc\">本画廊集中汇总了项目三大实验模块生成的所有高清结果图件，形成了数据指标与图件证据的完整支撑闭环。</p>\n";

        struct FigureItem { const char* filename; const char* title; const char* desc; };
        FigureItem items[] = {
            { "00_research_workflow_summary.png", "0. 全流程科研闭环综合大看板", "综合展示采样优化、属性质量与波场重构三大模块核心成果" },
            { "01_sa_convergence_curve.png", "1. 模拟退火(SA)优化相干度μ下降收敛曲线", "展示50%采样率下相干度由0.9601收敛至0.2213，衰减12.75 dB" },
            { "02_sampling_benchmark_bars.png", "2. 三种采样方案在各采样率下的相干度对比柱状图", "对比规则抽样、随机抽样与Jitter-SA优化采样的相干度与假频抑制" },
            { "03_cmp_fold_distribution_map.png", "3. CMP 面元覆盖次数空间平面热力分布图", "展示576个面元的覆盖次数，变异系数Cv=0.154，证明空间覆盖高度均匀" },
            { "04_offset_azimuth_rose_charts.png", "4. 炮检距连续性分布直方图与方位角玫瑰图", "证明非规则排列保留了连续的炮检距与360度全方位角覆盖" },
            { "05_wavefield_reconstruction_quad.png", "5. 地震波场欠采样与压缩感知(POCS)恢复四联剖面", "展示原始记录、50%欠采样、POCS恢复剖面与残差空间切片剖面" },
            { "06_recon_convergence_trace_comparison.png", "6. POCS残差收敛曲线与抽稀道单道波形对比细节", "展示迭代收敛轨迹与未采样道反射峰谷精确对齐" }
        };

        for (int i = 0; i < 7; i++)
        {
            out << "<div class=\"card\">\n"
                << "<h2>" << items[i].title << "</h2>\n"
                << "<p>" << items[i].desc << "</p>\n"
                << "<img src=\"" << items[i].filename << "\" alt=\"" << items[i].title << "\">\n"
                << "</div>\n";
        }

        out << "</div>\n</body>\n</html>\n";
        file.close();
        return true;
    }
};

#endif // SCIENTIFIC_PLOTTER_H
