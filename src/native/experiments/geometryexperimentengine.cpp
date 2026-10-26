#include "experiment_internal.h"

bool reconstructionFinite(double v){return v==v && fabs(v)<=std::numeric_limits<double>::max();} // add by wushanzhou in 2026/10/24

const float PI_CONST = 3.14159265358979323846f;

QString samplingMaskHash(const std::vector<int>& mask) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    QByteArray bits; // add by wushanzhou in 2026/10/23
    for (size_t i = 0; i < mask.size(); i++) bits.append(mask[i] ? '1' : '0'); // add by wushanzhou in 2026/10/23
    return QString(QCryptographicHash::hash(bits, QCryptographicHash::Sha1).toHex()); // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23
bool saveSamplingMask(const QString& path, const ConstrainedResult& result, // add by wushanzhou in 2026/10/23
                              unsigned long long seed, int budget, const QString& objective, const QString& protocol) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/23
    QDir().mkpath(QFileInfo(path).absolutePath()); // add by wushanzhou in 2026/10/23
    QFile f(path); // add by wushanzhou in 2026/10/23
    if (result.mask.empty() || !f.open(QIODevice::WriteOnly | QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream os(&f); os.setCodec("UTF-8"); // add by wushanzhou in 2026/10/23
    os << "# protocol=" << (protocol.isEmpty()?(objective.startsWith("Physical")?"physical_psf_v3":"classic_psf_v2"):protocol) << " objective=" << objective << "\n"; // add by wushanzhou in 2026/10/24
    os << "# seed=" << QString::number(seed) << " budget_cap=" << budget << " evals=" << result.evals << "\n"; // add by wushanzhou in 2026/10/23
    os << "# mu=" << QString::number(result.mu, 'g', 9) << " objective_value=" << QString::number(result.objective, 'g', 9) // add by wushanzhou in 2026/10/23
       << " max_gap=" << result.maxGap << " nn_cv=" << result.nnCv << "\n"; // add by wushanzhou in 2026/10/23
    if(objective.startsWith("Physical"))os << "# max_gap_m=" << QString::number(result.maxGapM,'g',12) << " nn_cv_units=physical\n"; // add by wushanzhou in 2026/10/23
    os << "# mask_sha1=" << samplingMaskHash(result.mask) << " status=" << QString::fromStdString(result.status) << "\n"; // add by wushanzhou in 2026/10/23
    for (size_t i = 0; i < result.mask.size(); i++) os << result.mask[i] << "\n"; // add by wushanzhou in 2026/10/23
    os.flush(); // add by wushanzhou in 2026/10/23
    if (f.error() != QFile::NoError) return false; // add by wushanzhou in 2026/10/23
    f.close(); // add by wushanzhou in 2026/10/23
    QFile trace(path + ".trace.csv"); // add by wushanzhou in 2026/10/23
    if (!trace.open(QIODevice::WriteOnly | QIODevice::Text)) return false; // add by wushanzhou in 2026/10/23
    QTextStream ts(&trace); ts << "evaluation,best_objective,current_or_candidate_objective" << (protocol.isEmpty()?"\n":",phase\n"); // add by wushanzhou in 2026/10/24
    for(size_t i=0;i<result.bestTrace.size();i++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        ts << (int)i+1 << "," << QString::number(result.bestTrace[i],'g',9) << "," << QString::number(i<result.currentTrace.size()?result.currentTrace[i]:result.bestTrace[i],'g',9); // add by wushanzhou in 2026/10/24
        if(!protocol.isEmpty())ts << "," << (i<result.phaseTrace.size()?result.phaseTrace[i]:-1); // add by wushanzhou in 2026/10/24
        ts << "\n"; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    ts.flush(); return trace.error() == QFile::NoError; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23


// 2026.10.20: 带符号增益格式化 —— 修复负增益被写成 "+-0.62 dB" 的问题。
// 正数显示 "+x"，负数显示 "-x"，零显示 "0"。
QString fmtSigned(float v, int prec)
{
    if (v > 1e-9f)  return QString("+") + QString::number(v, 'f', prec);
    if (v < -1e-9f) return QString::number(v, 'f', prec);
    return QString::number(0.0, 'f', prec);
}

// 2026.10.20: 比较类结论必须由实测差值决定，禁止无条件输出"超过基线"。
// 返回: 1 = 优于基线; -1 = 劣于基线; 0 = 差异可忽略(|d| < tol)
int compareGain(float delta, float tol)
{
    if (delta > tol)  return 1;
    if (delta < -tol) return -1;
    return 0;
}


// 统一获取科研成果图件集中保存目录 (figures/)
QString getCentralFiguresDir(const QString& currentOutputDir)
{
    return FigurePaths::directory(currentOutputDir);
}

// 定位仓库自带真实 SPS 示范工区 (sps/5151-5171.S, .R, .X) 所在目录
// 修复(2026.10.19): 原实现以硬编码绝对路径 "d:/Desktop/work/..." 作为回退，
// 导致在非项目根目录运行 --test 时 Test 9 失败（可移植性缺陷）。
// 现改为相对可执行文件与当前工作目录的多候选搜索，不依赖任何固定盘符路径。
QString findRepositorySpsDir()
{
    QList<QString> candidates;

    // 1) 可执行文件所在目录向上回溯（覆盖 x64/Release、bin 等常见输出目录布局）
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir d(exeDir);
    for (int i = 0; i < 5 && !d.isRoot(); i++)
    {
        candidates.append(d.absoluteFilePath("data/sps"));   // 2026.10.20: 目录重组后 SPS 样本位于 data/sps
        if (!d.cdUp()) break;
    }

    // 2) 当前工作目录及其上级（覆盖从项目根或子目录运行的情形）
    QDir cwd = QDir::current();
    candidates.append(cwd.absoluteFilePath("data/sps"));
    if (cwd.cdUp()) candidates.append(cwd.absoluteFilePath("data/sps"));

    // 3) 环境变量覆盖（便于测试/CI 指定外部数据目录）
    candidates.append(QString::fromLocal8Bit(qgetenv("IRR_SPS_DIR")));

    for (int i = 0; i < candidates.size(); i++)
    {
        const QString &dir = candidates.at(i);
        if (dir.isEmpty()) continue;
        if (QFile::exists(dir + "/5151-5171.S") &&
            QFile::exists(dir + "/5151-5171.R") &&
            QFile::exists(dir + "/5151-5171.X"))
        {
            return dir;
        }
    }
    return QString();
}

// 尝试从工程仓库常见位置自动加载真实 SPS 示范工区 (sps/5151-5171.S, .R, .X)
GProject* loadRepositorySpsProject()
{
    const QString spsDir = findRepositorySpsDir();
    if (spsDir.isEmpty())
    {
        return NULL;
    }
    const QString sFile = spsDir + "/5151-5171.S";
    const QString rFile = spsDir + "/5151-5171.R";
    const QString xFile = spsDir + "/5151-5171.X";

    GProject *pProj = GProject::create();
    pProj->setFlag();
    bool ok = pProj->appendSPSInfo(sFile, rFile, xFile, NULL, NULL);
    if (!ok)
    {
        GProject::destroy();
        return NULL;
    }
    pProj->refresh();

    // 自动构建 CMP 面元网格 (25m x 25m)，使用旋转对齐后的测线坐标系 (Transformed Coordinates)
    GCellGrid *pCellGrid = pProj->getCellGrid();
    GSPSParam *pSPSParam = pProj->getSPSParam();
    if (pCellGrid && pSPSParam)
    {
        float gx = 25.0f, gy = 25.0f;
        float x0 = pProj->getMinX() - 12.5f;
        float y0 = pProj->getMinY() - 12.5f;
        int numX = (int)((pProj->getMaxX() - pProj->getMinX()) / gx) + 2;
        int numY = (int)((pProj->getMaxY() - pProj->getMinY()) / gy) + 2;
        if (numX < 20) numX = 80;
        if (numY < 20) numY = 60;
        pCellGrid->mDiffX = -12.5f;
        pCellGrid->mDiffY = -12.5f;
        pCellGrid->create(pSPSParam, x0, y0, numX, numY, gx, gy);
        pCellGrid->calculateCoverCount();
    }
    return pProj;
}


// -----------------------------------------------------------------------------
// 快速 2D FFT / IFFT 自包含实现（用于地震波场重构 POCS 反演）
// -----------------------------------------------------------------------------
void internal_fft1d(std::vector<std::complex<float> >& a, bool invert)
{
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++)
    {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1)
    {
        float ang = 2.0f * PI_CONST / (float)len * (invert ? -1.0f : 1.0f);
        std::complex<float> wlen((float)cos(ang), (float)sin(ang));
        for (int i = 0; i < n; i += len)
        {
            std::complex<float> w(1.0f, 0.0f);
            for (int j = 0; j < len / 2; j++)
            {
                std::complex<float> u = a[i + j];
                std::complex<float> v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert)
    {
        for (int i = 0; i < n; i++)
        {
            a[i] /= (float)n;
        }
    }
}

void internal_fft2d(std::vector<std::vector<std::complex<float> > >& mat, bool invert)
{
    int rows = (int)mat.size();
    if (rows == 0) return;
    int cols = (int)mat[0].size();

    // 沿列 (时间方向) 变换
    std::vector<std::complex<float> > col(rows);
    for (int j = 0; j < cols; j++)
    {
        for (int i = 0; i < rows; i++) col[i] = mat[i][j];
        internal_fft1d(col, invert);
        for (int i = 0; i < rows; i++) mat[i][j] = col[i];
    }

    // 沿行 (空间道方向) 变换
    for (int i = 0; i < rows; i++)
    {
        internal_fft1d(mat[i], invert);
    }
}

bool checkFileExistsAndNonEmpty(const QString& filePath)
{
    QFileInfo fi(filePath);
    return fi.exists() && fi.size() > 0;
}

// =============================================================================
// 严格掩码校验函数 (Task 7 / Section VII) // add by wushanzhou in 2026/10/15
// =============================================================================
bool GeometryExperimentEngine::validateAndLoadMaskFile(const QString& maskFilePath,
                                                      int expectedN,
                                                      int expectedSampledPoints,
                                                      std::vector<int>& outMask,
                                                      QString& outErrorMsg)
{
    // add by wushanzhou in 2026/10/15
    outMask.clear();
    QString expPointsStr = (expectedSampledPoints > 0) ? QString::number(expectedSampledPoints) : QString::fromLocal8Bit("未指定");

    if (maskFilePath.isEmpty())
    {
        outErrorMsg = QString::fromLocal8Bit("掩码文件路径为空 (预期长度: %1, 实际长度: 0, 预期采样点数: %2, 实际采样点数: 0)")
            .arg(expectedN).arg(expPointsStr);
        return false;
    }
    QFile f(maskFilePath);
    if (!f.exists())
    {
        outErrorMsg = QString::fromLocal8Bit("掩码文件不存在: %1 (预期长度: %2, 实际长度: 0, 预期采样点数: %3, 实际采样点数: 0)")
            .arg(maskFilePath).arg(expectedN).arg(expPointsStr);
        return false;
    }
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        outErrorMsg = QString::fromLocal8Bit("无法打开指定的掩码文件: %1 (预期长度: %2, 实际长度: 0, 预期采样点数: %3, 实际采样点数: 0)")
            .arg(maskFilePath).arg(expectedN).arg(expPointsStr);
        return false;
    }

    QTextStream in(&f);
    in.setCodec("UTF-8");
    std::vector<int> tokens;
    int lineNum = 0;
    int onesCount = 0;

    while (!in.atEnd())
    {
        lineNum++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList parts = line.split(QRegExp("[,;\\s]+"), QString::SkipEmptyParts);
        for (int i = 0; i < parts.size(); i++)
        {
            QString tok = parts[i].trimmed();
            if (tok != "0" && tok != "1")
            {
                outErrorMsg = QString::fromLocal8Bit("掩码文件包含非法非二值内容: '%1' (位于第 %2 行; 预期长度: %3, 实际长度: %4, 预期采样点数: %5, 实际采样点数: %6)")
                    .arg(tok).arg(lineNum).arg(expectedN).arg((int)tokens.size()).arg(expPointsStr).arg(onesCount);
                f.close();
                return false;
            }
            int val = (tok == "1") ? 1 : 0;
            if (val == 1) onesCount++;
            tokens.push_back(val);
        }
    }
    f.close();

    int actualLen = (int)tokens.size();
    if (actualLen != expectedN)
    {
        if (tokens.empty())
        {
            outErrorMsg = QString::fromLocal8Bit("掩码文件为空 (预期长度: %1, 实际长度: 0, 预期采样点数: %2, 实际采样点数: 0)")
                .arg(expectedN).arg(expPointsStr);
        }
        else if (actualLen < expectedN)
        {
            outErrorMsg = QString::fromLocal8Bit("掩码文件截断/条目不足 (预期长度: %1, 实际长度: %2, 预期采样点数: %3, 实际采样点数: %4)")
                .arg(expectedN).arg(actualLen).arg(expPointsStr).arg(onesCount);
        }
        else
        {
            outErrorMsg = QString::fromLocal8Bit("掩码文件包含多余条目 (预期长度: %1, 实际长度: %2, 预期采样点数: %3, 实际采样点数: %4)")
                .arg(expectedN).arg(actualLen).arg(expPointsStr).arg(onesCount);
        }
        return false;
    }

    if (expectedSampledPoints > 0)
    {
        if (onesCount != expectedSampledPoints)
        {
            outErrorMsg = QString::fromLocal8Bit("掩码有效采样点数不符合约定 (预期长度: %1, 实际长度: %2, 预期采样点数: %3, 实际采样点数: %4)")
                .arg(expectedN).arg(actualLen).arg(expectedSampledPoints).arg(onesCount);
            return false;
        }
    }
    else
    {
        if (onesCount <= 0 || onesCount >= expectedN)
        {
            outErrorMsg = QString::fromLocal8Bit("掩码有效采样点数非法 (预期长度: %1, 实际长度: %2, 预期采样点数: %3, 实际采样点数: %4)")
                .arg(expectedN).arg(actualLen).arg(expPointsStr).arg(onesCount);
            return false;
        }
    }

    outMask = tokens;
    return true;
}

bool GeometryExperimentEngine::validateAndLoadMaskFile(const QString& maskFilePath,
                                                      int expectedN,
                                                      std::vector<int>& outMask,
                                                      QString& outErrorMsg)
{
    return validateAndLoadMaskFile(maskFilePath, expectedN, -1, outMask, outErrorMsg);
}

// =============================================================================
// 外部地震数据 CSV 严格加载校验 (Task 3: 防止指定真实 CSV 静默回退) // add by wushanzhou in 2026/10/15
// =============================================================================
bool GeometryExperimentEngine::loadAndValidateSeismicCSV(const QString& filePath,
                                                         int expectedNt,
                                                         int expectedNx,
                                                         std::vector<std::vector<float> >& outGather,
                                                         QString& outErrorMsg)
{
    outGather.clear();
    if (filePath.isEmpty())
    {
        outErrorMsg = QString::fromLocal8Bit("外部地震数据文件路径为空");
        return false;
    }
    QFile f(filePath);
    if (!f.exists())
    {
        outErrorMsg = QString::fromLocal8Bit("指定的外部地震数据文件不存在: %1").arg(filePath);
        return false;
    }
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        outErrorMsg = QString::fromLocal8Bit("无法打开外部地震数据文件: %1").arg(filePath);
        return false;
    }

    QTextStream in(&f);
    in.setCodec("UTF-8");
    std::vector<std::vector<float> > tempGather;
    int lineNum = 0;
    int validRows = 0;

    while (!in.atEnd())
    {
        lineNum++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList parts = line.split(QRegExp("[,;\\s]+"), QString::SkipEmptyParts);
        if (parts.empty()) continue;

        // 首个非注释非空行，如果包含表头字段（例如 Time_s 或 Trace_1 等文本），则跳过表头
        if (validRows == 0)
        {
            bool firstColIsNum = false;
            parts[0].toDouble(&firstColIsNum);
            if (!firstColIsNum && (parts[0].toLower().startsWith("time") || parts[0].toLower().startsWith("trace")))
            {
                continue;
            }
        }

        int colCount = parts.size();
        int colOffset = 0;
        if (colCount == expectedNx)
        {
            colOffset = 0;
        }
        else if (colCount == expectedNx + 1)
        {
            colOffset = 1; // 跳过第 0 列时间列
        }
        else
        {
            outErrorMsg = QString::fromLocal8Bit("外部地震数据列维度不符合约定 (第 %1 行包含 %2 列, 严格期望为 %3 列纯道数据或 %4 列含时间道数据; 文件: %5)")
                .arg(lineNum).arg(colCount).arg(expectedNx).arg(expectedNx + 1).arg(filePath);
            f.close();
            return false;
        }

        std::vector<float> rowData(expectedNx, 0.0f);
        for (int c = 0; c < expectedNx; c++)
        {
            QString valStr = parts[colOffset + c].trimmed();
            bool ok = false;
            double dval = valStr.toDouble(&ok);
            if (!ok || dval != dval || dval > 1e30 || dval < -1e30 || valStr.toLower() == "nan" || valStr.toLower() == "inf" || valStr.toLower() == "-inf")
            {
                outErrorMsg = QString::fromLocal8Bit("外部地震数据包含非法数值/缺失值/NaN/Inf: '%1' (第 %2 行第 %3 列; 文件: %4)")
                    .arg(valStr).arg(lineNum).arg(colOffset + c + 1).arg(filePath);
                f.close();
                return false;
            }
            rowData[c] = (float)dval;
        }

        tempGather.push_back(rowData);
        validRows++;
    }
    f.close();

    if (validRows == 0)
    {
        outErrorMsg = QString::fromLocal8Bit("外部地震数据文件为空 (有效数据行数: 0, 期望: %1 行; 文件: %2)")
            .arg(expectedNt).arg(filePath);
        return false;
    }

    if (validRows != expectedNt)
    {
        outErrorMsg = QString::fromLocal8Bit("外部地震数据采样时间点数(行数)不符合约定 (实际包含 %1 行有效数据, 严格期望为 %2 行; 文件: %3)")
            .arg(validRows).arg(expectedNt).arg(filePath);
        return false;
    }

    outGather = tempGather;
    return true;
}

// 辅助函数: 导出二维道集矩阵 CSV // add by wushanzhou in 2026/10/15
bool exportGatherCSV(const QString& filePath,
                           const std::vector<std::vector<float> >& matData,
                           const QString& headerNote,
                           const QString& modeName,
                           int Nt, int Nx, float dt,
                           int expNt, int expNx, float exportDt,
                           int stepT, int stepX,
                           QString& outErrorMsg)
{
    QFile gFile(filePath);
    if (!gFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outErrorMsg = QString::fromLocal8Bit("【波场重构失败】：无法创建道集矩阵输出文件: %1").arg(filePath);
        return false;
    }
    QTextStream out(&gFile);
    out.setCodec("UTF-8");
    out << "# Matrix_Type: " << headerNote << "\n";
    out << "# Mode: " << modeName << "\n";
    out << "# Grid_Dimensions: " << expNt << " rows (samples) x " << expNx << " cols (traces)\n";
    out << "# Sampling_Interval_dt: " << QString::number(exportDt, 'f', 4) << " s\n";
    out << "Time_s";
    for (int col = 0; col < expNx; col++) out << ",Trace_" << (col * stepX + 1);
    out << "\n";
    for (int r = 0; r < expNt; r++)
    {
        int rowIdx = r * stepT;
        out << QString::number(rowIdx * dt, 'f', 4);
        for (int c = 0; c < expNx; c++)
        {
            int colIdx = c * stepX;
            out << "," << QString::number(matData[rowIdx][colIdx], 'f', 6);
        }
        out << "\n";
    }
    gFile.close();
    return true;
}


// =============================================================================
// 2026.10.20 新增：空间均匀性度量（衡量"非规则但均匀"这一核心命题）
// 此前代码只优化 μ，SA 的任意交换可能制造大空洞，却没有任何指标能发现。
// 三个指标：
//   ① max_gap        —— 相邻采样点最大间隔（"大孔隙"，端点留白也计入）
//   ② nn_cv          —— 最近邻距离变异系数（"聚集"）
//   ③ unit_occupancy —— 抖动单元"恰好一点"的比例（量化 SA 对抖动性的破坏）
// =============================================================================
int uniformity_max_gap(const int* mask, int N)
{
    int prev = -1, gap = 0, first = -1, last = -1;
    for (int i = 0; i < N; i++)
    {
        if (mask[i] != 1) continue;
        if (first < 0) first = i;
        if (prev >= 0 && i - prev > gap) gap = i - prev;
        prev = i;
        last = i;
    }
    if (first >= 0)
    {
        if (first > gap) gap = first;
        if (N - 1 - last > gap) gap = N - 1 - last;
    }
    return gap;
}

float uniformity_nn_cv(const int* mask, int N)
{
    std::vector<int> idx;
    for (int i = 0; i < N; i++) if (mask[i] == 1) idx.push_back(i);
    const int n = (int)idx.size();
    if (n < 2) return 0.0f;
    std::vector<double> d;
    d.reserve(n);
    for (int k = 0; k < n; k++)
    {
        int left = (k > 0) ? idx[k] - idx[k - 1] : (k + 1 < n ? idx[k + 1] - idx[k] : 0);
        int right = (k + 1 < n) ? idx[k + 1] - idx[k] : (k > 0 ? idx[k] - idx[k - 1] : 0);
        d.push_back((double)((left < right) ? left : right));
    }
    double mean = 0.0;
    for (size_t i = 0; i < d.size(); i++) mean += d[i];
    mean /= (double)d.size();
    if (mean <= 1e-9) return 0.0f;
    double var = 0.0;
    for (size_t i = 0; i < d.size(); i++) { double t = d[i] - mean; var += t * t; }
    var /= (double)d.size();
    return (float)(sqrt(var) / mean);
}

float uniformity_unit_occupancy(const int* mask, int N, int m)
{
    if (N <= 0 || m <= 0 || m >= N) return 1.0f;
    const double alpha = (double)N / (double)m;
    std::vector<int> cnt(m, 0);
    for (int i = 0; i < N; i++)
    {
        if (mask[i] != 1) continue;
        int u = (int)((double)i / alpha);
        if (u < 0) u = 0;
        if (u >= m) u = m - 1;
        cnt[u]++;
    }
    int okUnits = 0;
    for (int u = 0; u < m; u++) if (cnt[u] == 1) okUnits++;
    return (float)okUnits / (float)m;
}

// =============================================================================
// Task 1: 多采样率对比试验 (30%, 50%, 70%) // add by wushanzhou in 2026/10/15
// =============================================================================
bool GeometryExperimentEngine::runFullExperimentSuite(GProject *pProject,
                                                     const QString& outputDir,
                                                     QString& fullReportSummary,
                                                     unsigned long long rngSeed)
{
    return runFullExperimentSuite(pProject, outputDir, fullReportSummary, rngSeed, 1);
}

// 兼容原有签名的转发实现 (add by wushanzhou in 2026/10/15)
bool GeometryExperimentEngine::runSamplingBenchmark(int N,
                                                    const std::vector<float>& rates,
                                                    const QString& outputDir,
                                                    std::vector<BenchmarkRecord>& outResults,
                                                    QString& outReportSummary)
{
    return runSamplingBenchmark(N, rates, outputDir, outResults, outReportSummary, 42ULL, 1);
}

bool GeometryExperimentEngine::evaluateProjectAttributes(GProject *pProject,
                                                        const QString& outputDir,
                                                        AttributeMetrics& outMetrics,
                                                        QString& outReportSummary)
{
    return evaluateProjectAttributes(pProject, outputDir, outMetrics, outReportSummary, true);
}

bool GeometryExperimentEngine::runWavefieldReconstruction(const QString& maskFilePath,
                                                         const QString& outputDir,
                                                         float customRatio,
                                                         ReconstructionMetrics& outMetrics,
                                                         QString& outReportSummary)
{
    return runWavefieldReconstruction(maskFilePath, outputDir, customRatio, outMetrics, outReportSummary, "", true);
}

bool GeometryExperimentEngine::runFullExperimentSuite(GProject *pProject,
                                                     const QString& outputDir,
                                                     QString& fullReportSummary)
{
    return runFullExperimentSuite(pProject, outputDir, fullReportSummary, 42ULL, 1);
}

// 内部测试日志记录辅助函数
// 内部测试日志记录辅助函数 // add by wushanzhou in 2026/10/15

bool GeometryExperimentEngine::runHoldoutValidation(const QString& inputCsvPath,
                                                    int useTraces,
                                                    int keepTraces,
                                                    int iterations,
                                                    unsigned long long seed,
                                                    const QString& outputDir,
                                                    HoldoutMetrics& outMetrics,
                                                    QString& outReport,
                                                    const QString& externalMaskPath)
{
    const clock_t holdoutStart = clock();   // 2026.10.20: 记录本组实际耗时
    QDir dir(outputDir);
    if (!dir.exists()) QDir().mkpath(outputDir);

    // ---- 1. 读入真实道集（不做 256x128 尺寸假设，按文件实际列数）----
    QFile in(inputCsvPath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        outReport = QString::fromLocal8Bit("【留出验证失败】无法打开输入文件: %1").arg(inputCsvPath);
        return false;
    }
    in.close(); // add by wushanzhou in 2026/10/23
    std::vector<std::vector<double> > all;
    std::vector<double> coordinates; // add by wushanzhou in 2026/10/23
    if (!loadHoldoutGather(inputCsvPath,all,coordinates,outReport)) return false; // add by wushanzhou in 2026/10/23

    const int availNt = (int)all.size();
    const int availNx = (int)all[0].size();
    if (useTraces<0 || useTraces>availNx || keepTraces<0 || iterations<1) // add by wushanzhou in 2026/10/23
    { outReport="invalid holdout dimensions/iterations"; return false; } // add by wushanzhou in 2026/10/23
    int m=useTraces>0?useTraces:availNx; // add by wushanzhou in 2026/10/23
    int k=keepTraces>0?keepTraces:m/2; // add by wushanzhou in 2026/10/23
    if (m<4 || k<2 || k>=m) {outReport="holdout requires 2 <= keep < traces";return false;} // add by wushanzhou in 2026/10/23
    coordinates.resize(m); // add by wushanzhou in 2026/10/23

    // 截取前 m 道
    std::vector<std::vector<float> > truth(availNt, std::vector<float>(m, 0.0f));
    for (int i = 0; i < availNt; i++)
        for (int j = 0; j < m; j++)
            truth[i][j] = (float)all[i][j];

    // ---- 2. 用 seed 随机选 k 道作为"已知" ----
    exprng::seed(seed);
    std::vector<int> idx(m);
    for (int i = 0; i < m; i++) idx[i] = i;
    for (int i = m - 1; i > 0; i--)
    {
        int j = (int)(exprng::integer() % (unsigned long)(i + 1));
        int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
    }
    std::vector<int> mask(m, 0);
    if (!externalMaskPath.isEmpty())
    {
        // 2026.10.20 新增：用外部掩码（抖动 / SA 优化）代替随机抽稀，
        // 使留出验证真正检验"本文采样设计"而不是"任意随机缺失"。
        std::vector<int> extMask;
        QString extErr;
        bool okExt = validateAndLoadMaskFile(externalMaskPath, m, extMask, extErr);
        if (!okExt)
        {
            outReport = QString::fromLocal8Bit("【留出验证失败】外部掩码校验失败 - %1").arg(extErr);
            return false;
        }
        int ones = 0;
        for (int j = 0; j < m && j < (int)extMask.size(); j++)
        {
            mask[j] = extMask[j] ? 1 : 0;
            ones += mask[j];
        }
        if (ones < 2 || ones >= m)
        {
            outReport = QString::fromLocal8Bit("【留出验证失败】外部掩码已知道数不合法 (%1/%2)")
                .arg(ones).arg(m);
            return false;
        }
        k = ones;
    }
    else
    {
        for (int i = 0; i < k; i++) mask[idx[i]] = 1;
    }

    // 观测数据: 未知道置 0
    std::vector<std::vector<float> > dObs(availNt, std::vector<float>(m, 0.0f));
    std::vector<int> known;
    for (int j = 0; j < m; j++)
    {
        if (!mask[j]) continue;
        known.push_back(j);
        for (int i = 0; i < availNt; i++) dObs[i][j] = truth[i][j];
    }

    // ---- 3a. 零填充 ----
    // 未知道保持 0（dObs 本身）

    // ---- 3b. 线性插值 ----
    std::vector<std::vector<float> > dLin,dRec; // add by wushanzhou in 2026/10/23
    double consistencyError=0,knownScale=1; // add by wushanzhou in 2026/10/23
    if (!coordinateHoldoutReconstruct(truth,mask,coordinates,iterations,dLin,dRec,consistencyError,&knownScale)) // add by wushanzhou in 2026/10/23
    {outReport="coordinate reconstruction/data consistency failed";return false;} // add by wushanzhou in 2026/10/23

    // ---- 4. 只在留出道集合上统计 SNR ----
    double eZero = 0.0, eLin = 0.0, ePocs = 0.0, eTruth = 0.0;
    long cnt = 0;
    for (int j = 0; j < m; j++)
    {
        if (mask[j]) continue;                       // 只用留出道
        for (int i = 0; i < availNt; i++)
        {
            double g = truth[i][j];
            eTruth += g * g;
            double a = 0.0 - g;                      // 零填充
            double b = dLin[i][j] - g;
            double c = dRec[i][j] - g;
            eZero += a * a;
            eLin  += b * b;
            ePocs += c * c;
            cnt++;
        }
    }
    if (cnt == 0 || eTruth <= 1e-20)
    {
        outReport = QString::fromLocal8Bit("【留出验证失败】留出道样本为空或参考能量为 0");
        return false;
    }

    const double eps = 1e-20;
    outMetrics.nTraces      = m;
    outMetrics.keptTraces   = k;
    outMetrics.heldOutTraces= m - k;
    outMetrics.nSamples     = availNt;
    outMetrics.iterations   = iterations;
    outMetrics.runtimeMs   = (double)(clock() - holdoutStart) / CLOCKS_PER_SEC * 1000.0;
    outMetrics.seed         = seed;
    outMetrics.snr_zero_fill_db = 10.0 * log10(eTruth / (eZero > eps ? eZero : eps));
    outMetrics.snr_linear_db    = 10.0 * log10(eTruth / (eLin  > eps ? eLin  : eps));
    outMetrics.snr_pocs_db      = 10.0 * log10(eTruth / (ePocs > eps ? ePocs : eps));
    outMetrics.gain_pocs_vs_linear_db = outMetrics.snr_pocs_db - outMetrics.snr_linear_db;
    outMetrics.gain_pocs_vs_zero_db   = outMetrics.snr_pocs_db - outMetrics.snr_zero_fill_db;
    outMetrics.rmse_linear = (float)sqrt(eLin  / (double)cnt);
    outMetrics.rmse_pocs   = (float)sqrt(ePocs / (double)cnt);
    outMetrics.dataConsistencyPassed = consistencyError<reconstructionConsistencyTolerance(knownScale); // add by wushanzhou in 2026/10/23
    {   // 2026.10.20: 记录输入文件 SHA1，便于第三方独立复核
        QFile hf(inputCsvPath);
        if (hf.open(QIODevice::ReadOnly))
        {
            outMetrics.inputSha1 = QString(QCryptographicHash::hash(hf.readAll(), QCryptographicHash::Sha1).toHex());
            hf.close();
        }
    }

    // ---- 5. 输出 CSV + 文本报告 ----
    QFile csv(dir.filePath("holdout_metrics.csv"));
    if (!csv.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = QString::fromLocal8Bit("【留出验证失败】无法写入输出目录");
        return false;
    }
    {
        QTextStream o(&csv);
        o.setCodec("UTF-8");
        o << "Metric_Name,Value,Unit,Description\n";
        o << "Validation_Type,Hold-out on measured traces,Text,SNR computed ONLY on withheld real traces\n";
        o << "Geometry_Mode,ActualOffsets_LinearObservationProjection,Text,Held-out amplitudes never enter solver input\n"; // add by wushanzhou in 2026/10/23
        o << "Solver_Amplitude_Scale," << QString::number(knownScale,'g',9) << ",Amplitude,Estimated ONLY from known traces then inverted for evaluation\n"; // add by wushanzhou in 2026/10/23
        o << "Mask_SHA1," << samplingMaskHash(mask) << ",Hash,Canonical binary mask SHA1\n"; // add by wushanzhou in 2026/10/23
        o << "Data_Consistency_Max_Error," << QString::number(consistencyError,'g',9) << ",Amplitude,Measured on known physical coordinates\n"; // add by wushanzhou in 2026/10/23
        o << "Data_Consistency_Tolerance," << QString::number(reconstructionConsistencyTolerance(knownScale),'g',12) << ",Amplitude,1e-5 * max(1 known amplitude scale)\n";
        o << "Data_Consistency_Passed," << (outMetrics.dataConsistencyPassed?1:0) << ",Boolean,Shared solver and report threshold\n";
        o << "Input_File," << QFileInfo(inputCsvPath).fileName() << ",Text,Source gather\n";
        o << "Traces_Used," << m << ",Traces,Real traces in the working grid\n";
        o << "Traces_Kept," << k << ",Traces,Traces given to the solver as known\n";
        o << "Traces_HeldOut," << (m - k) << ",Traces,Measured traces used as truth only\n";
        o << "Time_Samples," << availNt << ",Samples,Samples per trace\n";
        o << "POCS_Iterations," << iterations << ",Iterations,POCS iterations\n";
        o << "Random_Seed," << QString::number(seed) << ",Seed,Seed selecting kept traces\n";
        o << "SNR_Zero_Fill_HeldOut," << QString::number(outMetrics.snr_zero_fill_db, 'f', 2) << ",dB,On held-out traces\n";
        o << "SNR_Linear_HeldOut," << QString::number(outMetrics.snr_linear_db, 'f', 2) << ",dB,On held-out traces\n";
        o << "SNR_POCS_HeldOut," << QString::number(outMetrics.snr_pocs_db, 'f', 2) << ",dB,On held-out traces\n";
        o << "Gain_POCS_vs_Linear," << QString::number(outMetrics.gain_pocs_vs_linear_db, 'f', 2) << ",dB,Held-out gain\n";
        o << "Gain_POCS_vs_Zero," << QString::number(outMetrics.gain_pocs_vs_zero_db, 'f', 2) << ",dB,Held-out gain\n";
        o << "RMSE_Linear," << QString::number(outMetrics.rmse_linear, 'f', 6) << ",Amplitude,Held-out RMSE\n";
        o << "RMSE_POCS," << QString::number(outMetrics.rmse_pocs, 'f', 6) << ",Amplitude,Held-out RMSE\n";
        o << "Input_SHA1," << outMetrics.inputSha1 << ",Hash,SHA1 of the input gather CSV\n";
        o << "Runtime_ms," << QString::number(outMetrics.runtimeMs, 'f', 1) << ",ms,Wall time for this hold-out run\n";
        csv.close();
    }

    outReport = QString::fromLocal8Bit(
        "【留出验证完成】(Hold-out on measured traces)\n"
        "输入文件: %1\n"
        "真实道数 m=%2, 已知 k=%3, 留出 %4 道（真实实测，仅用于评估）\n"
        "时间样点 Nt=%5, POCS 迭代 %6 次, 种子 %7\n"
        "--- 仅在留出道上的 SNR ---\n"
        "  零填充: %8 dB\n"
        "  线性插值: %9 dB\n"
        "  曲波 POCS: %10 dB\n"
        "  POCS 相对线性: %11 dB\n"
        "  POCS 相对零填充: %12 dB\n"
        "  留出 RMSE: 线性 %13 / POCS %14\n"
        "输出: %15")
        .arg(QFileInfo(inputCsvPath).fileName())
        .arg(m).arg(k).arg(m - k).arg(availNt).arg(iterations).arg(QString::number(seed))
        .arg(QString::number(outMetrics.snr_zero_fill_db, 'f', 2))
        .arg(QString::number(outMetrics.snr_linear_db, 'f', 2))
        .arg(QString::number(outMetrics.snr_pocs_db, 'f', 2))
        .arg(fmtSigned(outMetrics.gain_pocs_vs_linear_db))
        .arg(fmtSigned(outMetrics.gain_pocs_vs_zero_db))
        .arg(QString::number(outMetrics.rmse_linear, 'f', 6))
        .arg(QString::number(outMetrics.rmse_pocs, 'f', 6))
        .arg(dir.filePath("holdout_metrics.csv"));

    return true;
}


// =============================================================================
// 批量留出验证并归档 (2026.10.20 新增)
//
// 目的: 让"6 组 keep x seed 配置"成为可独立复算的机器可读记录，
//       而不是散落在审计报告表格里由人工抄录。
//
// 输出: <outputDir>/holdout_runs.csv，每行一组配置，字段包括
//       输入文件与 SHA1、道数、样点、迭代、种子、三种方法 SNR、
//       两个增益、两个 RMSE、实际耗时。
//
// 说明: 所有数值均由 runHoldoutValidation 实际执行产生。
// =============================================================================
bool GeometryExperimentEngine::runHoldoutBatch(const QString& inputCsvPath,
                                               int useTraces,
                                               const QList<int>& keeps,
                                               const QList<unsigned long long>& seeds,
                                               int iterations,
                                               const QString& outputDir,
                                               QString& outReport)
{
    QDir dir(outputDir);
    if (!dir.exists()) QDir().mkpath(outputDir);

    QFile in(inputCsvPath);
    if (!in.open(QIODevice::ReadOnly))
    {
        outReport = QString::fromLocal8Bit("【批量留出验证失败】无法打开输入: %1").arg(inputCsvPath);
        return false;
    }
    QByteArray blob = in.readAll();
    in.close();
    if (blob.isEmpty())
    {
        outReport = QString::fromLocal8Bit("【批量留出验证失败】输入文件为空");
        return false;
    }
    const QString sha1 = QString(QCryptographicHash::hash(blob, QCryptographicHash::Sha1).toHex());

    QFile csv(dir.filePath("holdout_runs.csv"));
    if (!csv.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = QString::fromLocal8Bit("【批量留出验证失败】无法写入 holdout_runs.csv");
        return false;
    }

    QTextStream o(&csv);
    o.setCodec("UTF-8");
    o << "config_index,keep_traces,total_traces,held_out_traces,time_samples,"
         "iterations,seed,input_file,input_sha1,"
         "snr_zero_fill_db,snr_linear_db,snr_pocs_db,"
         "gain_pocs_vs_linear_db,gain_pocs_vs_zero_db,"
         "rmse_linear,rmse_pocs,runtime_ms\n";

    int okCount = 0;
    const int nKeeps = keeps.size();
    const int nSeeds = seeds.size();
    int cfgIndex = 0;

    QStringList lines;
    for (int ki = 0; ki < nKeeps; ki++)
    {
        for (int si = 0; si < nSeeds; si++)
        {
            const int keep = keeps.at(ki);
            const unsigned long long sd = seeds.at(si);
            const QString oneDir = dir.filePath(QString("cfg_%1_%2").arg(keep).arg(QString::number(sd)));

            HoldoutMetrics hm;
            QString oneReport;
            if (!runHoldoutValidation(inputCsvPath, useTraces, keep, iterations, sd, oneDir, hm, oneReport))
            {
                o << cfgIndex << "," << keep << "," << useTraces << ",,,"
                  << QString::number(sd) << "," << QFileInfo(inputCsvPath).fileName() << ","
                  << sha1 << ",,,,,,,,ERROR\n";
                lines << QString::fromLocal8Bit("  cfg%1 keep=%2 seed=%3 -> FAILED")
                         .arg(cfgIndex).arg(keep).arg(QString::number(sd));
                cfgIndex++;
                continue;
            }

            o << cfgIndex << "," << keep << "," << hm.nTraces << "," << hm.heldOutTraces << ","
              << hm.nSamples << "," << hm.iterations << ","
              << QString::number(sd) << "," << QFileInfo(inputCsvPath).fileName() << ","
              << sha1 << ","
              << QString::number(hm.snr_zero_fill_db, 'f', 4) << ","
              << QString::number(hm.snr_linear_db, 'f', 4) << ","
              << QString::number(hm.snr_pocs_db, 'f', 4) << ","
              << QString::number(hm.gain_pocs_vs_linear_db, 'f', 4) << ","
              << QString::number(hm.gain_pocs_vs_zero_db, 'f', 4) << ","
              << QString::number(hm.rmse_linear, 'f', 6) << ","
              << QString::number(hm.rmse_pocs, 'f', 6) << ","
              << QString::number(hm.runtimeMs, 'f', 1) << "\n";

            lines << QString::fromLocal8Bit("  cfg%1 keep=%2/%3 held=%4 seed=%5  zero=%6 linear=%7 POCS=%8 gain=%9 rmse=%10")
                     .arg(cfgIndex).arg(keep).arg(hm.nTraces).arg(hm.heldOutTraces)
                     .arg(QString::number(sd))
                     .arg(QString::number(hm.snr_zero_fill_db, 'f', 2))
                     .arg(QString::number(hm.snr_linear_db, 'f', 2))
                     .arg(QString::number(hm.snr_pocs_db, 'f', 2))
                     .arg(fmtSigned(hm.gain_pocs_vs_linear_db))
                     .arg(QString::number(hm.rmse_pocs, 'f', 6));
            okCount++;
            cfgIndex++;
        }
    }
    csv.close();

    outReport = QString::fromLocal8Bit(
        "【批量留出验证完成】%1/%2 组成功\n"
        "输入: %3\n"
        "SHA1: %4\n"
        "%5\n"
        "归档: %6")
        .arg(okCount).arg(nKeeps * nSeeds)
        .arg(QFileInfo(inputCsvPath).fileName())
        .arg(sha1)
        .arg(lines.join("\n"))
        .arg(dir.filePath("holdout_runs.csv"));

    return (okCount == nKeeps * nSeeds);
}
bool GeometryExperimentEngine::runMiuProbe(const QString& outCsvPath, QString& outReport)
{
    QFile f(outCsvPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = QString::fromLocal8Bit("cannot create probe CSV");
        return false;
    }
    QTextStream os(&f);
    os.setCodec("UTF-8");
    os << "case,N,m,nbscales,nbangles,mu,is_multi_block,diag_dev_max,zero_norm_hit\n";

    const int N = 128;
    std::vector<int> scales;   scales.push_back(1); scales.push_back(2); scales.push_back(3); scales.push_back(4);
    std::vector<int> angles;   angles.push_back(4); angles.push_back(8); angles.push_back(16); angles.push_back(32);

    // ---- 用例 1：全 1 掩码（全采样），未来 μ 应与参考项完全匹配 ----
    {
        int *full = new int[N];
        for (int i = 0; i < N; i++) full[i] = 1;
        for (size_t si = 0; si < scales.size(); si++)
            for (size_t ai = 0; ai < angles.size(); ai++)
            {
                const float mu = CurveletMiu1DEigenParam(vector<int>(full, full + N), N, 1, scales[si], angles[ai]);
                os << "full_mask," << N << "," << N << "," << scales[si] << "," << angles[ai] << ","
                   << QString::number(mu, 'f', 8) << ",,,0\n";
            }
        delete[] full;
    }

    // ---- 用例 2：单点掩码（极端）----
    {
        for (int pos = 0; pos < N; pos += 31)
        {
            int *one = new int[N];
            for (int i = 0; i < N; i++) one[i] = 0;
            one[pos] = 1;
            const float mu = CurveletMiu1DEigenParam(vector<int>(one, one + N), N, 1, 1, 4);
            os << "single_point_" << pos << "," << N << ",1,1,4,"
               << QString::number(mu, 'f', 8) << ",,,0\n";
            delete[] one;
        }
    }

    // ---- 用例 3：两点掩码（距离扫描）：检验 μ 是否随相邻距离单调递增 ----
    {
        for (int d = 1; d <= 32; d *= 2)
        {
            int *two = new int[N];
            for (int i = 0; i < N; i++) two[i] = 0;
            two[0] = 1;
            two[d] = 1;
            const float mu = CurveletMiu1DEigenParam(vector<int>(two, two + N), N, 1, 1, 4);
            os << "two_points_d" << d << "," << N << ",2,1,4,"
               << QString::number(mu, 'f', 8) << ",,,0\n";
            delete[] two;
        }
    }

    // ---- 用例 4：三种采样率 × 四组参数（主结论所在）----
    {
        const float rates[3] = { 0.30f, 0.50f, 0.70f };
        for (int ri = 0; ri < 3; ri++)
        {
            const int m = (int)floor((double)N * rates[ri] + 0.5);
            exprng::seed(42ULL + (unsigned long long)(rates[ri] * 1000.0f));
            int *mask = Sampling1J_Count(N, m);
            for (size_t si = 0; si < scales.size(); si++)
                for (size_t ai = 0; ai < angles.size(); ai++)
                {
                    std::vector<int> idxV;
                    for (int q = 0; q < N; q++) if (mask[q] == 1) idxV.push_back(q);
                    const float mu = CurveletMiu1DEigenParam(idxV, N, 1, scales[si], angles[ai]);
                    os << "jitter_c" << (int)(rates[ri] * 100) << "_s" << scales[si] << "_a" << angles[ai]
                       << "," << N << "," << m << "," << scales[si] << "," << angles[ai] << ","
                       << QString::number(mu, 'f', 8) << ",,,0\n";
                }
            delete[] mask;
        }
    }

    // ---- 用例 5：重复输入稳定性（同一掩码算 5 次）----
    {
        exprng::seed(7ULL);
        int *mask = Sampling1J_Count(N, 64);
        for (int k = 0; k < 5; k++)
        {
            const float mu = CurveletMiu1DEigenParam(vector<int>(mask, mask + N), N, 1, 1, 4);
            os << "repeat_" << k << "," << N << ",64,1,4,"
               << QString::number(mu, 'f', 8) << ",,,0\n";
        }
        delete[] mask;
    }

    f.close();
    outReport = QString::fromLocal8Bit("probe written: %1").arg(outCsvPath);
    return true;
}



// 2026-10-21: 直接探验 FFT1D_fftshift 是否丢失采样点位置信息
bool GeometryExperimentEngine::runFftProbe(const QString& outCsvPath, QString& outReport)
{
    QFile f(outCsvPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = "cannot create fft probe CSV";
        return false;
    }
    QTextStream os(&f);
    os.setCodec("UTF-8");
    os << "case,pos_a,pos_b,row_a_norm,row_b_norm,rows_equal,X_norm_total\n";

    const int N = 128;
    // 构造两个只有 1 个采样点的掩码，位置分别为 0 与 64
    int posA = 0, posB = 64;
    int **PA = (int**)malloc(sizeof(int*) * N);
    int **PB = (int**)malloc(sizeof(int*) * N);
    for (int i = 0; i < N; i++)
    {
        PA[i] = (int*)malloc(sizeof(int) * N);
        PB[i] = (int*)malloc(sizeof(int) * N);
        for (int j = 0; j < N; j++) { PA[i][j] = 0; PB[i][j] = 0; }
    }
    PA[posA][posA] = 1;
    PB[posB][posB] = 1;

    fdct_wrapping_ns::CpxNumMat XA(N, N), XB(N, N);
    FFT1D_fftshift(PA, XA, N, N);
    FFT1D_fftshift(PB, XB, N, N);

    // 比较两个结果矩阵是否相同
    double maxDiff = 0.0;
    double normA = 0.0, normB = 0.0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            const std::complex<double> a = XA(i, j);
            const std::complex<double> b = XB(i, j);
            const double d = std::abs(a - b);
            if (d > maxDiff) maxDiff = d;
            normA += std::abs(a) * std::abs(a);
            normB += std::abs(b) * std::abs(b);
        }
    }
    os << "two_positions," << posA << "," << posB << ","
       << QString::number(sqrt(normA), 'f', 6) << ","
       << QString::number(sqrt(normB), 'f', 6) << ","
       << ((maxDiff < 1e-6) ? "YES" : "NO") << ","
       << QString::number(sqrt(normA + normB), 'f', 6) << "\n";

    // 输出 X 的非零行统计：若只有一行非零，说明信息集中
    int nzRowsA = 0, nzColsA = 0;
    for (int i = 0; i < N; i++)
    {
        bool rz = false;
        for (int j = 0; j < N; j++) if (std::abs(XA(i, j)) > 1e-12) { rz = true; break; }
        if (rz) nzRowsA++;
    }
    for (int j = 0; j < N; j++)
    {
        bool cz = false;
        for (int i = 0; i < N; i++) if (std::abs(XA(i, j)) > 1e-12) { cz = true; break; }
        if (cz) nzColsA++;
    }
    os << "nz_structure,," << nzRowsA << "," << nzColsA << ",,,\n";

    for (int i = 0; i < N; i++) { free(PA[i]); free(PB[i]); }
    free(PA); free(PB);
    f.close();
    outReport = QString::fromLocal8Bit("fft probe written");
    return true;
}


// =============================================================================
// 2026-10-21: 目标函数区分能力探针
// 核心问题：相同点数、不同几何布局的掩码，μ 能否区分？
// 若不能，则“采样点筛选”这一核心命题失去度量支撑。
// =============================================================================
bool GeometryExperimentEngine::runMiuDiscriminateProbe(const QString& outCsvPath, QString& outReport)
{
    QFile f(outCsvPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = "cannot create discriminate probe CSV";
        return false;
    }
    QTextStream os(&f);
    os.setCodec("UTF-8");
    os << "case,N,m,mu,max_gap,nn_cv,note\n";

    const int N = 128;
    const int m = 64;

    // ---- A. 均匀抽样（每 2 点一个）----
    {
        std::vector<int> idx;
        for (int i = 0; i < N; i += 2) idx.push_back(i);
        int *mask = new int[N];
        for (int i = 0; i < N; i++) mask[i] = 0;
        for (size_t k = 0; k < idx.size(); k++) mask[idx[k]] = 1;
        const float mu = TestCurveletMiu1D(N, mask, "EigenReal");
        os << "uniform_every2," << N << "," << (int)idx.size() << ","
           << QString::number(mu, 'f', 6) << "," << uniformity_max_gap(mask, N) << ","
           << QString::number(uniformity_nn_cv(mask, N), 'f', 4) << QString::fromLocal8Bit("均匀间隔") << "\n";
        delete[] mask;
    }

    // ---- B. 两半集中（前 64 点全采 + 后 64 点不采）----
    {
        int *mask = new int[N];
        for (int i = 0; i < N; i++) mask[i] = (i < m) ? 1 : 0;
        const float mu = TestCurveletMiu1D(N, mask, "EigenReal");
        os << "half_block," << N << "," << m << ","
           << QString::number(mu, 'f', 6) << "," << uniformity_max_gap(mask, N) << ","
           << QString::number(uniformity_nn_cv(mask, N), 'f', 4) << QString::fromLocal8Bit("极端聚集") << "\n";
        delete[] mask;
    }

    // ---- C. 完全等间隔（m=64，步长 2）与上面 A 相同；再造一个不规则但尽量均匀的掩码 ----
    {
        // 用 Jitter 对照
        exprng::seed(42ULL);
        int *mask = Sampling1J_Count(N, m);
        const float mu = TestCurveletMiu1D(N, mask, "EigenReal");
        os << "jitter_seed42," << N << "," << m << ","
           << QString::number(mu, 'f', 6) << "," << uniformity_max_gap(mask, N) << ","
           << QString::number(uniformity_nn_cv(mask, N), 'f', 4) << QString::fromLocal8Bit("随机抖动") << "\n";
        delete[] mask;
    }

    // ---- D. 隧变：从均匀渐变到前半集中 ----
    {
        for (int step = 0; step <= 8; step++)
        {
            // step=0 完全均匀；step=8 完全前半
            int *mask = new int[N];
            for (int i = 0; i < N; i++) mask[i] = 0;
            // 保留 m 个点，位置由 step 决定偏移
            for (int k = 0; k < m; k++)
            {
                double base = (double)k * (double)N / (double)m;
                double shift = (double)step / 8.0 * (base < N / 2.0 ? 0.0 : -((double)N / 2.0 - base));
                int pos = (int)(base + shift);
                if (pos < 0) pos = 0;
                if (pos >= N) pos = N - 1;
                mask[pos] = 1;
            }
            int cnt = 0; for (int i = 0; i < N; i++) if (mask[i]) cnt++;
            const float mu = TestCurveletMiu1D(N, mask, "EigenReal");
            os << "sweep_step" << step << "," << N << "," << cnt << ","
               << QString::number(mu, 'f', 6) << "," << uniformity_max_gap(mask, N) << ","
               << QString::number(uniformity_nn_cv(mask, N), 'f', 4) << QString::fromLocal8Bit("几何渐变") << "\n";
            delete[] mask;
        }
    }

    f.close();
    outReport = "discriminate probe written";
    return true;
}

// =============================================================================
// 2026-10-21 新增：修正版互干度的数学性质探针（供 Test 16 与论文取证）
// =============================================================================
bool GeometryExperimentEngine::runMcProbe(const QString& outCsvPath, QString& outReport)
{
    QFile f(outCsvPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReport = "cannot create mc probe CSV";
        return false;
    }
    QTextStream os(&f);
    os.setCodec("UTF-8");
    os << "case,N,m,mu_classic,max_gap,nn_cv,expected,pass\n";

    const int N = 128;

    // 1) 全采样 -> 期望 0
    {
        std::vector<int> idx;
        for (int i = 0; i < N; i++) idx.push_back(i);
        const float mu = SamplingMutualCoherence1D(idx, N);
        os << "full," << N << "," << N << "," << QString::number(mu, 'f', 8)
           << ",,," << "0" << "," << (fabs(mu) < 1e-6f ? "YES" : "NO") << "\n";
    }
    // 2) 单点 -> 期望 1
    {
        std::vector<int> idx; idx.push_back(0);
        const float mu = SamplingMutualCoherence1D(idx, N);
        os << "single," << N << ",1," << QString::number(mu, 'f', 8)
           << ",,," << "1" << "," << (fabs(mu - 1.0f) < 1e-6f ? "YES" : "NO") << "\n";
    }
    // 3) 规则 2 抽 1 -> 期望 1（Nyquist）
    {
        std::vector<int> idx;
        for (int i = 0; i < N; i += 2) idx.push_back(i);
        const float mu = SamplingMutualCoherence1D(idx, N);
        os << "regular2," << N << "," << (int)idx.size() << "," << QString::number(mu, 'f', 8)
           << ",,," << "1" << "," << (fabs(mu - 1.0f) < 1e-4f ? "YES" : "NO") << "\n";
    }
    // 4) 两点间距扫描 -> 期望单调递减
    {
        float prev = 2.0f;
        bool mono = true;
        for (int d = 1; d <= 32; d *= 2)
        {
            std::vector<int> idx; idx.push_back(0); idx.push_back(d);
            const float mu = SamplingMutualCoherence1D(idx, N);
            if (mu > prev + 1e-6f) mono = false;
            prev = mu;
            os << "two_d" << d << "," << N << ",2," << QString::number(mu, 'f', 8)
               << ",,," << "nonincreasing" << "," << (mono ? "YES" : "NO") << "\n";
        }
    }
    // 5) 均匀 vs 极端集中 -> 期望 均匀更低
    {
        const int m = 64;
        std::vector<int> uni;
        for (int i = 0; i < N; i += 2) uni.push_back(i);
        std::vector<int> clu;
        for (int i = 0; i < m; i++) clu.push_back(i);
        const float muU = SamplingMutualCoherence1D(uni, N);
        const float muC = SamplingMutualCoherence1D(clu, N);
        os << "uniform_vs_clustered_uniform," << N << "," << (int)uni.size() << ","
           << QString::number(muU, 'f', 8) << ",,," << "lower" << ","
           << (muU < muC ? "YES" : "NO") << "\n";
        os << "uniform_vs_clustered_clustered," << N << "," << (int)clu.size() << ","
           << QString::number(muC, 'f', 8) << ",,," << "higher" << ","
           << (muC > muU ? "YES" : "NO") << "\n";
    }
    // 6) 随机与抖动
    {
        const int m = 64;
        exprng::seed(42ULL);
        int *jr = Sampling1J_Count(N, m);
        std::vector<int> jit;
        for (int i = 0; i < N; i++) if (jr[i] == 1) jit.push_back(i);
        delete[] jr;
        const float muJ = SamplingMutualCoherence1D(jit, N);
        os << "jitter42," << N << "," << (int)jit.size() << "," << QString::number(muJ, 'f', 8)
           << ",,," << "<1" << "," << (muJ < 1.0f ? "YES" : "NO") << "\n";

        int *rs = Sampling1S_Count(N, m);
        std::vector<int> rnd;
        for (int i = 0; i < N; i++) if (rs[i] == 1) rnd.push_back(i);
        delete[] rs;
        const float muR = SamplingMutualCoherence1D(rnd, N);
        os << "random42," << N << "," << (int)rnd.size() << "," << QString::number(muR, 'f', 8)
           << ",,," << "<1" << "," << (muR < 1.0f ? "YES" : "NO") << "\n";
    }
    // 7) 空掩码 / 异常输入
    {
        std::vector<int> empty;
        const float mu0 = SamplingMutualCoherence1D(empty, N);
        os << "empty," << N << ",0," << QString::number(mu0, 'f', 8)
           << ",,," << "0" << "," << (fabs(mu0) < 1e-9f ? "YES" : "NO") << "\n";
    }

    f.close();
    outReport = "mc probe written";
    return true;
}

// =============================================================================
// 2026-10-21 新增：空间约束优化 A–E 方案对比 + 惩罚系数敏感性
// =============================================================================
// 2026-10-21: 与 OptimizeMiu.cpp 中的实现一致的均匀性度量（供基线评估复用）
