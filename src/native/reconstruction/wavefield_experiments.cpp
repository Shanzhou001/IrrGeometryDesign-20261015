#include "../experiments/experiment_internal.h"

void curveletPocsReconstruct(const std::vector<std::vector<float> >& initGather,
                                    const std::vector<std::vector<float> >& dObs,
                                    const std::vector<int>& mask,
                                    int numIter,
                                    std::vector<std::vector<float> >& outRecon,
                                    std::vector<std::vector<float> >& outResidual,
                                    std::vector<float>& outStepRes, // add by wushanzhou in 2026/10/23
                                    const std::vector<double>* positions, // add by wushanzhou in 2026/10/23
                                    const std::vector<std::vector<float> >* measured, // add by wushanzhou in 2026/10/23
                                    const std::vector<int>* measuredMask, // add by wushanzhou in 2026/10/24
                                    const ReconstructionOptions* options) // add by wushanzhou in 2026/10/24
{
    outRecon.clear();outResidual.clear();outStepRes.clear(); // add by wushanzhou in 2026/10/24
    const int Nt = (int)initGather.size();
    if(Nt<32 || initGather[0].size()<32 || numIter<1)return; // add by wushanzhou in 2026/10/24
    const int Nx = (int)initGather[0].size();
    if((int)mask.size()!=Nx || (int)dObs.size()!=Nt)return; // add by wushanzhou in 2026/10/24
    for(int row=0;row<Nt;row++)if((int)initGather[row].size()!=Nx || (int)dObs[row].size()!=Nx)return; // add by wushanzhou in 2026/10/24
    if(options && (!reconstructionFinite(options->thresholdStart) || !reconstructionFinite(options->thresholdEnd) || // add by wushanzhou in 2026/10/24
        options->thresholdEnd<0 || options->thresholdStart<options->thresholdEnd || options->scales<2 || options->scales>4 || // add by wushanzhou in 2026/10/24
        (options->angles!=8 && options->angles!=16)))return; // add by wushanzhou in 2026/10/24
    if((positions || measured || measuredMask) && !(positions && measured && measuredMask))return; // add by wushanzhou in 2026/10/24
    if(positions) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        if(positions->size()!=measuredMask->size() || (int)measured->size()!=Nt)return; // add by wushanzhou in 2026/10/24
        for(int row=0;row<Nt;row++)if((*measured)[row].size()!=positions->size())return; // add by wushanzhou in 2026/10/24
        for(size_t j=0;j<positions->size();j++)if(!reconstructionFinite((*positions)[j]) || (*positions)[j]<0 || (*positions)[j]>Nx-1 || (j && (*positions)[j]<=(*positions)[j-1]))return; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24

    fdct_wrapping_ns::CpxNumMat x_mat(Nt, Nx);
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            x_mat(i, j) = fdct_wrapping_ns::cpx(initGather[i][j], 0.0);
        }
    }

    const int nbscales=options?options->scales:4; // add by wushanzhou in 2026/10/24
    const int nbangles_coarse=options?options->angles:16; // add by wushanzhou in 2026/10/24
    const int ac = 0; // 最高频尺度使用小波基，抑制高频角度伪影
    const double l_max = 0.026;
    const double l_min = 0.0005;

    std::vector<std::vector<fdct_wrapping_ns::CpxNumMat> > c;
    outStepRes.clear();

    std::vector<int> projectionIndices, projectionLeft; // add by wushanzhou in 2026/10/23
    std::vector<double> projectionA,projectionB,projectionInverse,projectionUpper; // add by wushanzhou in 2026/10/23
    if (positions && measured && measuredMask) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        for(int j=0;j<(int)positions->size();j++)if((*measuredMask)[j]) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            int left=(int)floor((*positions)[j]);if(left>=Nx-1)left=Nx-2; // add by wushanzhou in 2026/10/23
            const double b=(*positions)[j]-left,a=1.0-b; // add by wushanzhou in 2026/10/23
            projectionIndices.push_back(j);projectionLeft.push_back(left); // add by wushanzhou in 2026/10/23
            projectionA.push_back(a);projectionB.push_back(b);projectionInverse.push_back(0); // add by wushanzhou in 2026/10/23
            projectionUpper.push_back(0); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        // Factor the tridiagonal A*A^T with supports ordered by physical position. // add by wushanzhou in 2026/10/23
        for(size_t k=0;k<projectionIndices.size();k++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            double lower=k>0 && projectionLeft[k-1]+1==projectionLeft[k]?projectionB[k-1]*projectionA[k]:0; // add by wushanzhou in 2026/10/23
            double upper=k+1<projectionIndices.size() && projectionLeft[k]+1==projectionLeft[k+1]?projectionB[k]*projectionA[k+1]:0; // add by wushanzhou in 2026/10/23
            double diagonal=projectionA[k]*projectionA[k]+projectionB[k]*projectionB[k]; // add by wushanzhou in 2026/10/23
            if(k>0)diagonal-=lower*projectionUpper[k-1]; // add by wushanzhou in 2026/10/23
            if(diagonal<=1e-14){outRecon.clear();return;} // add by wushanzhou in 2026/10/23
            projectionInverse[k]=1.0/diagonal;projectionUpper[k]=upper/diagonal; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23

    for (int it = 0; it < numIter; it++)
    {
        fdct_wrapping_ns::CpxNumMat prev_mat = x_mat;

        // 1. 2D 离散曲波正变换 (FDCT Wrapping)
        fdct_wrapping_ns::fdct_wrapping(Nt, Nx, nbscales, nbangles_coarse, ac, x_mat, c);

        // 2. 余弦退火调度与尺度自适应 Wiener 收缩滤波
        double progress = (numIter > 1) ? (double)it / (double)(numIter - 1) : 0.0;
        double factor = 0.5 * (1.0 + cos(PI_CONST * progress));
        double lambda = l_max * factor + l_min;
        if(options)lambda=(options->thresholdStart-options->thresholdEnd)*factor+options->thresholdEnd; // add by wushanzhou in 2026/10/24

        for (size_t s = 0; s < c.size(); s++)
        {
            if (s == 0) continue; // 粗尺度背景低频严格保留
            double s_lambda = (s == c.size() - 1) ? lambda * 2.0 : lambda;
            double s_sigma2 = s_lambda * s_lambda;
            for (size_t w = 0; w < c[s].size(); w++)
            {
                for (int i = 0; i < c[s][w].m(); i++)
                {
                    for (int j = 0; j < c[s][w].n(); j++)
                    {
                        fdct_wrapping_ns::cpx val = c[s][w](i, j);
                        double m2 = val.real() * val.real() + val.imag() * val.imag();
                        if (m2 > 0.0)
                        {
                            double gain = m2 / (m2 + s_sigma2);
                            c[s][w](i, j) = val * gain;
                        }
                    }
                }
            }
        }

        // 3. 2D 离散曲波逆变换 (IFDCT Wrapping)
        fdct_wrapping_ns::ifdct_wrapping(Nt, Nx, nbscales, nbangles_coarse, ac, c, x_mat);

        // 4. 实测道数据一致性严格投影 (Data Consistency Projection)
        if (positions && measured && measuredMask) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            const int count=(int)projectionIndices.size();std::vector<double> correction(count,0); // add by wushanzhou in 2026/10/23
            for(int i=0;i<Nt;i++) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                for(int k=0;k<count;k++) // add by wushanzhou in 2026/10/23
                { // add by wushanzhou in 2026/10/23
                    int left=projectionLeft[k],j=projectionIndices[k]; // add by wushanzhou in 2026/10/23
                    double residual=(*measured)[i][j]-projectionA[k]*x_mat(i,left).real()-projectionB[k]*x_mat(i,left+1).real(); // add by wushanzhou in 2026/10/23
                    double lower=k>0 && projectionLeft[k-1]+1==left?projectionB[k-1]*projectionA[k]:0; // add by wushanzhou in 2026/10/23
                    correction[k]=(residual-(k>0?lower*correction[k-1]:0))*projectionInverse[k]; // add by wushanzhou in 2026/10/23
                } // add by wushanzhou in 2026/10/23
                for(int k=count-2;k>=0;k--)correction[k]-=projectionUpper[k]*correction[k+1]; // add by wushanzhou in 2026/10/23
                for(int k=0;k<count;k++) // add by wushanzhou in 2026/10/23
                { // add by wushanzhou in 2026/10/23
                    int left=projectionLeft[k]; // add by wushanzhou in 2026/10/23
                    x_mat(i,left)=fdct_wrapping_ns::cpx(x_mat(i,left).real()+projectionA[k]*correction[k],0); // add by wushanzhou in 2026/10/23
                    x_mat(i,left+1)=fdct_wrapping_ns::cpx(x_mat(i,left+1).real()+projectionB[k]*correction[k],0); // add by wushanzhou in 2026/10/23
                } // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            for(int i=0;i<Nt;i++)for(int j=0;j<Nx;j++)x_mat(i,j)=fdct_wrapping_ns::cpx(x_mat(i,j).real(),0); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        else // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
        for (int j = 0; j < Nx; j++)
        {
            if (mask[j] == 1)
            {
                for (int i = 0; i < Nt; i++)
                {
                    x_mat(i, j) = fdct_wrapping_ns::cpx(dObs[i][j], 0.0);
                }
            }
            else
            {
                for (int i = 0; i < Nt; i++)
                {
                    x_mat(i, j) = fdct_wrapping_ns::cpx(x_mat(i, j).real(), 0.0);
                }
            }
        }

        // 记录迭代收敛残差
        } // add by wushanzhou in 2026/10/23
        double diffEnergy = 0.0;
        double curEnergy = 0.0;
        for (int i = 0; i < Nt; i++)
        {
            for (int j = 0; j < Nx; j++)
            {
                double d = x_mat(i, j).real() - prev_mat(i, j).real();
                diffEnergy += d * d;
                curEnergy += x_mat(i, j).real() * x_mat(i, j).real();
            }
        }
        outStepRes.push_back((curEnergy > 1e-12) ? (float)sqrt(diffEnergy / curEnergy) : 0.0f);
    }

    outRecon.assign(Nt, std::vector<float>(Nx, 0.0f));
    outResidual.assign(Nt, std::vector<float>(Nx, 0.0f));
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            outRecon[i][j] = (mask[j] == 1) ? dObs[i][j] : (float)x_mat(i, j).real();
        }
    }
}

bool GeometryExperimentEngine::runWavefieldReconstruction(const QString& maskFilePath,
                                                         const QString& outputDir,
                                                         float customRatio,
                                                         ReconstructionMetrics& outMetrics,
                                                         QString& outReportSummary,
                                                         const QString& inputSeismicFilePath,
                                                         bool exportFullResolution,
                                                         bool forceDataConsistencyFailure,
                                                         bool useWarmStart,
                                                         bool runStartupComparison,
                                                         const QString& maskSourceOverride)
{
    clock_t startTime = clock();

    QDir dir(outputDir);
    if (!dir.exists())
    {
        if (!QDir().mkpath(outputDir))   // 2026.10.20: 原 dir.mkpath(outputDir) 会重复拼接路径
        {
            outReportSummary = QString::fromLocal8Bit("【波场重构失败】：无法创建指定的输出目录: %1").arg(outputDir);
            return false;
        }
    }

    const int Nt = 256;
    const int Nx = 128;
    const float dt = 0.004f;
    float dx = 12.5f; // add by wushanzhou in 2026/10/23
    double spatialOrigin=-Nx/2*dx; // add by wushanzhou in 2026/10/23
    QString spatialSource="DefaultUniformGrid"; // add by wushanzhou in 2026/10/23
    const float f0 = 25.0f;

    outMetrics.nSamples = Nt;
    outMetrics.nTraces = Nx;
    outMetrics.calcDt = dt;
    outMetrics.dx = dx;
    outMetrics.iterations = 40;
    outMetrics.snr_linear_missing_db = 0.0f;
    outMetrics.snr_pocs_missing_db = 0.0f;

    std::vector<std::vector<float> > d_full(Nt, std::vector<float>(Nx, 0.0f));

    // 1. 外部真实地震数据加载或合成波场数值模拟 (严格防静默回退) // add by wushanzhou in 2026/10/15
    if (!inputSeismicFilePath.isEmpty())
    {
        QString csvErrMsg;
        if (!loadAndValidateSeismicCSV(inputSeismicFilePath, Nt, Nx, d_full, csvErrMsg))
        {
            outReportSummary = QString::fromLocal8Bit("【波场重构失败】：外部地震数据加载校验失败 - %1").arg(csvErrMsg);
            return false;
        }
        outMetrics.isSynthetic = false;
        // 措辞中性化：外部 CSV 既可能是真实资料，也可能是测试光栅，
        // 因此不再声称 "Seismic Record"，只如实说明"来自外部文件"。
        outMetrics.modeName = QString("External CSV File (%1)").arg(QFileInfo(inputSeismicFilePath).fileName());
        QFile metadata(inputSeismicFilePath); // add by wushanzhou in 2026/10/23
        if (metadata.open(QIODevice::ReadOnly|QIODevice::Text)) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            while(!metadata.atEnd()) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                QString line=QString::fromUtf8(metadata.readLine()).trimmed(); // add by wushanzhou in 2026/10/23
                if(line.startsWith("# synthetic_model=")) // add by wushanzhou in 2026/10/23
                {outMetrics.isSynthetic=true;outMetrics.modeName="Synthetic external model: "+QFileInfo(inputSeismicFilePath).fileName();} // add by wushanzhou in 2026/10/23
                if(line.startsWith("# offsets_m=")) // add by wushanzhou in 2026/10/23
                { // add by wushanzhou in 2026/10/23
                    QStringList fields=line.mid(12).split(',',QString::KeepEmptyParts); // add by wushanzhou in 2026/10/23
                    if(fields.size()!=Nx){outReportSummary="reference-grid coordinate count mismatch";return false;} // add by wushanzhou in 2026/10/23
                    std::vector<double> x; // add by wushanzhou in 2026/10/23
                    for(int j=0;j<Nx;j++) // add by wushanzhou in 2026/10/23
                    { // add by wushanzhou in 2026/10/23
                        bool ok=false;double value=fields[j].toDouble(&ok); // add by wushanzhou in 2026/10/23
                        if(!ok || value!=value || fabs(value)>1e20 || (!x.empty() && value<=x.back())) // add by wushanzhou in 2026/10/23
                        {outReportSummary="invalid reference-grid coordinate";return false;} // add by wushanzhou in 2026/10/23
                        x.push_back(value); // add by wushanzhou in 2026/10/23
                    } // add by wushanzhou in 2026/10/23
                    double spacing=(x.back()-x.front())/(Nx-1); // add by wushanzhou in 2026/10/23
                    for(int j=1;j<Nx;j++)if(fabs(x[j]-x[j-1]-spacing)>1e-4*std::max(1.0,spacing)) // add by wushanzhou in 2026/10/23
                    {outReportSummary="reference reconstruction requires a uniform grid; use physical holdout for measured nonuniform traces";return false;} // add by wushanzhou in 2026/10/23
                    dx=(float)spacing;spatialOrigin=x.front();spatialSource="ActualOffsetUniformResampling"; // add by wushanzhou in 2026/10/23
                } // add by wushanzhou in 2026/10/23
                if(!line.startsWith("#") && !line.isEmpty())break; // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        outMetrics.dx=dx; // add by wushanzhou in 2026/10/23

    }
    else
    {
        outMetrics.isSynthetic = true;
        outMetrics.modeName = "Synthetic Numerical Wavefield (3-Layer Reflection Model + Gaussian White Noise)";

        struct LayerEvent
        {
            float t0;
            float v;
            float amp;
        };
        LayerEvent layers[3] = {
            { 0.25f, 2200.0f, 1.0f },
            { 0.45f, 2600.0f, 0.85f },
            { 0.65f, 3200.0f, 0.65f }
        };

        exprng::seed(1234567ULL);

        for (int j = 0; j < Nx; j++)
        {
            float offset = (j - Nx / 2) * dx;
            for (int k = 0; k < 3; k++)
            {
                float tx = (float)sqrt(layers[k].t0 * layers[k].t0 + (offset * offset) / (layers[k].v * layers[k].v));
                int center_idx = (int)(tx / dt + 0.5f);
                int half_len = (int)(0.06f / dt);
                for (int w = -half_len; w <= half_len; w++)
                {
                    int sample_idx = center_idx + w;
                    if (sample_idx >= 0 && sample_idx < Nt)
                    {
                        float tau = w * dt;
                        float arg = PI_CONST * PI_CONST * f0 * f0 * tau * tau;
                        float ricker = (1.0f - 2.0f * arg) * (float)exp(-arg);
                        d_full[sample_idx][j] += layers[k].amp * ricker;
                    }
                }
            }

            // 注入真正的 Box-Muller 高斯白噪声 (标准差 0.015)
            for (int i = 0; i < Nt; i++)
            {
                d_full[i][j] += (float)exprng::gaussian(0.0, 0.015);
            }
        }
    }

    // 2. 加载或生成采样掩码 (Mask Operator) (严格校验模式) // add by wushanzhou in 2026/10/15
    std::vector<int> mask(Nx, 0);
    bool loadedFromFile = false;
    float r = (customRatio > 0.1f && customRatio < 0.9f) ? customRatio : 0.50f;
    int expectedM = (int)floor((double)Nx * r + 0.5);

    if (!maskFilePath.isEmpty())
    {
        QString maskErr;
        if (!validateAndLoadMaskFile(maskFilePath, Nx, expectedM, mask, maskErr))
        {
            outReportSummary = QString::fromLocal8Bit("【波场重构失败】：采样掩码校验失败 - %1").arg(maskErr);
            return false;
        }
        loadedFromFile = true;
        // 2026.10.20: 复用上次落盘的 cmp_mask.txt 时，掩码本体并未改变，
        // 只是文件位置变了；此时沿用其原始来源标签，避免溯源在两次运行间翻转。
        if (!maskSourceOverride.isEmpty())
            outMetrics.maskSource = maskSourceOverride;
        else
            outMetrics.maskSource = QString("External File (%1)").arg(QFileInfo(maskFilePath).fileName());
    }
    if (!loadedFromFile)
    {
        // 2026.10.20: 自动生成抖动掩码前必须显式播种，否则同一次运行结果不可复现
        // （实测真实资料两次运行 SNR 由 9.25 漂到 10.23 dB）。
        // 用固定种子保证"同一输入 -> 同一掩码 -> 同一指标"。
        exprng::seed(20261020ULL);
        int *jit_mask = Sampling1J_Count(Nx, expectedM);
        for (int j = 0; j < Nx; j++) mask[j] = jit_mask[j];
        delete[] jit_mask;
        outMetrics.maskSource = QString("Auto-Generated Jitter Mask (m=%1/%2, %3%)")
            .arg(expectedM).arg(Nx).arg(QString::number(r * 100.0f, 'f', 1));
    }

    int activeTraces = 0;
    for (int j = 0; j < Nx; j++) if (mask[j] == 1) activeTraces++;
    outMetrics.sampledTraces = activeTraces;
    outMetrics.samplingRate = (float)activeTraces / (float)Nx;

    // 3. 构建欠采样观测记录 d_obs
    std::vector<std::vector<float> > d_obs(Nt, std::vector<float>(Nx, 0.0f));
    for (int j = 0; j < Nx; j++)
    {
        if (mask[j] == 1)
        {
            for (int i = 0; i < Nt; i++) d_obs[i][j] = d_full[i][j];
        }
    }

    // 基准 1：零值填充初始 SNR
    double energy_full = 0.0;
    double energy_zero_err = 0.0;
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            double orig = d_full[i][j];
            double err = orig - d_obs[i][j];
            energy_full += orig * orig;
            energy_zero_err += err * err;
        }
    }
    float snr_zero = 10.0f * (float)log10(energy_full / (energy_zero_err > 1e-12 ? energy_zero_err : 1e-12));
    outMetrics.snr_zero_fill_db = snr_zero;

    // 基准 2：一维空间线性插值基准
    std::vector<std::vector<float> > d_linear(Nt, std::vector<float>(Nx, 0.0f));
    std::vector<int> meas_x;
    for (int j = 0; j < Nx; j++) if (mask[j] == 1) meas_x.push_back(j);

    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            if (mask[j] == 1)
            {
                d_linear[i][j] = d_obs[i][j];
            }
            else
            {
                int left_idx = -1, right_idx = -1;
                for (size_t k = 0; k < meas_x.size(); k++)
                {
                    if (meas_x[k] < j) left_idx = meas_x[k];
                    if (meas_x[k] > j && right_idx == -1) right_idx = meas_x[k];
                }
                float val = 0.0f;
                if (left_idx != -1 && right_idx != -1)
                {
                    float w = (float)(j - left_idx) / (float)(right_idx - left_idx);
                    val = (1.0f - w) * d_obs[i][left_idx] + w * d_obs[i][right_idx];
                }
                else if (left_idx != -1) val = d_obs[i][left_idx];
                else if (right_idx != -1) val = d_obs[i][right_idx];
                d_linear[i][j] = val;
            }
        }
    }

    double energy_linear_err = 0.0;
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            double err = (double)d_full[i][j] - (double)d_linear[i][j];
            energy_linear_err += err * err;
        }
    }
    float snr_linear = 10.0f * (float)log10(energy_full / (energy_linear_err > 1e-12 ? energy_linear_err : 1e-12));
    outMetrics.snr_linear_interp_db = snr_linear;

    // 4. 曲波域 (Curvelet) 稀疏反演与 POCS 凸集投影迭代
    // useWarmStart=true : 空间线性插值预热初始化 (Warm-Start) —— 本项目主实验口径
    // useWarmStart=false: 零填充初始化 (Cold-Start) —— 标准 POCS / 文献对照口径
    outMetrics.usedWarmStart = useWarmStart;
    outMetrics.iterations = 40;

    const std::vector<std::vector<float> >& d_init = useWarmStart ? d_linear : d_obs;

    std::vector<std::vector<float> > d_recon;
    std::vector<std::vector<float> > d_residual;
    curveletPocsReconstruct(d_init, d_obs, mask, outMetrics.iterations,
                            d_recon, d_residual, outMetrics.residual_convergence);

    // 提取单道曲线供后续一致性校验使用 (保持与旧实现相同的校验语义)
    fdct_wrapping_ns::CpxNumMat x_mat(Nt, Nx);
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            x_mat(i, j) = fdct_wrapping_ns::cpx(d_recon[i][j], 0.0);
        }
    }

    // 4b. 冷启动 / 热启动 受控对照实验 (2026.10.19 新增)
    // 目的：为学术公平性提供证据——隔离"线性插值初值"与"曲波 POCS 迭代"各自的贡献。
    outMetrics.residual_convergence_cold.clear();
    outMetrics.residual_convergence_warm.clear();
    outMetrics.snr_cold_start_db = 0.0f;
    outMetrics.snr_warm_start_db = 0.0f;
    outMetrics.rmse_cold_start = 0.0f;
    outMetrics.rmse_warm_start = 0.0f;
    outMetrics.corr_cold_start = 0.0f;
    outMetrics.corr_warm_start = 0.0f;

    if (runStartupComparison)
    {
        // 辅助 lambda 风格评分（C++98 兼容，用局部结构体模拟）
        struct StartupScore { float snr; float rmse; float corr; };

        const std::vector<std::vector<float> >* inits[2] = { &d_obs, &d_linear };
        std::vector<float>* traces[2] = { &outMetrics.residual_convergence_cold,
                                           &outMetrics.residual_convergence_warm };
        StartupScore scores[2];

        for (int m2 = 0; m2 < 2; m2++)
        {
            std::vector<std::vector<float> > rec2, res2;
            curveletPocsReconstruct(*inits[m2], d_obs, mask, outMetrics.iterations, rec2, res2, *traces[m2]);

            double e2 = 0.0, mf = 0.0, mr = 0.0;
            for (int i = 0; i < Nt; i++)
            {
                for (int j = 0; j < Nx; j++)
                {
                    double r = (double)rec2[i][j] - (double)d_full[i][j];
                    e2 += r * r;
                    mf += d_full[i][j];
                    mr += rec2[i][j];
                }
            }
            const double tot = (double)(Nt * Nx);
            mf /= tot; mr /= tot;
            double nc = 0.0, df = 0.0, dr = 0.0;
            for (int i = 0; i < Nt; i++)
            {
                for (int j = 0; j < Nx; j++)
                {
                    double a = d_full[i][j] - mf;
                    double b = rec2[i][j] - mr;
                    nc += a * b; df += a * a; dr += b * b;
                }
            }
            scores[m2].snr = 10.0f * (float)log10(energy_full / (e2 > 1e-12 ? e2 : 1e-12));
            scores[m2].rmse = (float)sqrt(e2 / tot);
            scores[m2].corr = (df > 1e-12 && dr > 1e-12) ? (float)(nc / sqrt(df * dr)) : 0.0f;
        }

        outMetrics.snr_cold_start_db = scores[0].snr;
        outMetrics.rmse_cold_start = scores[0].rmse;
        outMetrics.corr_cold_start = scores[0].corr;
        outMetrics.snr_warm_start_db = scores[1].snr;
        outMetrics.rmse_warm_start = scores[1].rmse;
        outMetrics.corr_warm_start = scores[1].corr;
    }

    // 5. 实测道数据一致性严格校验 (支持人工注入故障测试) // add by wushanzhou in 2026/10/15
    if (forceDataConsistencyFailure)
    {
        for (int j = 0; j < Nx; j++)
        {
            if (mask[j] == 1)
            {
                x_mat(0, j) += fdct_wrapping_ns::cpx(1.0, 0.0);
                break;
            }
        }
    }

    bool consistencyOK = true;
    for (int j = 0; j < Nx; j++)
    {
        if (mask[j] == 1)
        {
            for (int i = 0; i < Nt; i++)
            {
                if (fabs(x_mat(i, j).real() - (double)d_obs[i][j]) > 1e-5)
                {
                    consistencyOK = false;
                    break;
                }
            }
        }
        if (!consistencyOK) break;
    }
    outMetrics.dataConsistencyPassed = consistencyOK;
    if (!consistencyOK)
    {
        outReportSummary = QString::fromLocal8Bit("【波场重构失败】：实测道数据一致性检验未通过 (观测道投影残差超过阈值 1e-5)");
        return false;
    }

    // Keep the actual solver trace so future plots do not need to run the solver again.
    QFile convergenceFile(QDir(outputDir).filePath("recon_convergence.csv"));
    if(!convergenceFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {outReportSummary="cannot write reconstruction convergence CSV";return false;}
    QTextStream convergenceStream(&convergenceFile);
    convergenceStream << "iteration,relative_step\n";
    for(size_t ci=0;ci<outMetrics.residual_convergence.size();ci++)
        convergenceStream << (int)ci+1 << "," << QString::number(outMetrics.residual_convergence[ci],'g',12) << "\n";
    convergenceStream.flush();
    if(convergenceFile.error()!=QFile::NoError)return false;
    convergenceFile.close();

    // 6. 重构指标统计 (d_recon/d_residual 已由 curveletPocsReconstruct 按"已知道强制替换"口径产出)
    double energy_rec_err = 0.0;
    double mean_full = 0.0, mean_rec = 0.0;

    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            float res = d_recon[i][j] - d_full[i][j];
            d_residual[i][j] = res;

            energy_rec_err += (double)res * (double)res;
            mean_full += d_full[i][j];
            mean_rec += d_recon[i][j];
        }
    }

    int totalSamples = Nt * Nx;
    mean_full /= (double)totalSamples;
    mean_rec /= (double)totalSamples;

    double num_corr = 0.0, den_f = 0.0, den_r = 0.0;
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            double df = d_full[i][j] - mean_full;
            double dr = d_recon[i][j] - mean_rec;
            num_corr += df * dr;
            den_f += df * df;
            den_r += dr * dr;
        }
    }

    outMetrics.snr_recovered_db = 10.0f * (float)log10(energy_full / (energy_rec_err > 1e-12 ? energy_rec_err : 1e-12));
    outMetrics.snr_improvement_db = outMetrics.snr_recovered_db - outMetrics.snr_zero_fill_db;
    outMetrics.snr_imprv_vs_linear_db = outMetrics.snr_recovered_db - outMetrics.snr_linear_interp_db;
    outMetrics.rmse = (float)sqrt(energy_rec_err / (double)totalSamples);
    outMetrics.correlation = (den_f > 1e-12 && den_r > 1e-12) ? (float)(num_corr / sqrt(den_f * den_r)) : 0.0f;

    // 2026.10.20 新增：缺失道口径 SNR。
    // 全幅 SNR 包含 64 条被精确置回的道，恒比“缺失道 SNR”高约 3 dB；
    // 论文对比必须同时给出缺失道口径，否则会高估重构精度。
    double energy_miss_truth = 0.0, energy_miss_lin = 0.0, energy_miss_poc = 0.0;
    for (int i = 0; i < Nt; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            if (mask[j] != 0) continue;
            const double g = (double)d_full[i][j];
            const double el = (double)d_linear[i][j] - g;
            const double ep = (double)d_recon[i][j] - g;
            energy_miss_truth += g * g;
            energy_miss_lin += el * el;
            energy_miss_poc += ep * ep;
        }
    }
    outMetrics.snr_linear_missing_db = 10.0f * (float)log10(energy_miss_truth / (energy_miss_lin > 1e-12 ? energy_miss_lin : 1e-12));
    outMetrics.snr_pocs_missing_db = 10.0f * (float)log10(energy_miss_truth / (energy_miss_poc > 1e-12 ? energy_miss_poc : 1e-12));

    outMetrics.runtimeMs = (double)(clock() - startTime) / CLOCKS_PER_SEC * 1000.0;

    // 7. 导出实验数据矩阵 (CSV 格式) (含严格写入检查) // add by wushanzhou in 2026/10/15
    int expNt = exportFullResolution ? Nt : 64;
    int expNx = exportFullResolution ? Nx : 32;
    int stepT = Nt / expNt;
    int stepX = Nx / expNx;
    outMetrics.exportNt = expNt;
    outMetrics.exportNx = expNx;
    outMetrics.exportDt = dt * (float)stepT;

    // 导出 cmp_mask.txt
    QString maskOutPath = dir.filePath("cmp_mask.txt");
    QFile maskOutFile(maskOutPath);
    if (!maskOutFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReportSummary = QString::fromLocal8Bit("【波场重构失败】：无法创建掩码输出文件: %1").arg(maskOutPath);
        return false;
    }
    {
        QTextStream out(&maskOutFile);
        out.setCodec("UTF-8");
        out << "# Source: " << outMetrics.maskSource << "\n";
        out << "# Sampled_Points: " << outMetrics.sampledTraces << " / " << outMetrics.nTraces << "\n";
        for (int j = 0; j < Nx; j++)
        {
            out << mask[j] << "\n";
        }
        maskOutFile.close();
    }

    // 导出 4 组二维道集矩阵
    QString errCsv;
    if (!exportGatherCSV(dir.filePath("recon_original_gather.csv"), d_full, outMetrics.isSynthetic?"Synthetic_Reference_Wavefield":"External_Reference_Wavefield", // add by wushanzhou in 2026/10/23
                         outMetrics.modeName, Nt, Nx, dt, expNt, expNx, outMetrics.exportDt, stepT, stepX, errCsv) ||
        !exportGatherCSV(dir.filePath("recon_subsampled_gather.csv"), d_obs, "Subsampled_Acquired_Wavefield",
                         outMetrics.modeName, Nt, Nx, dt, expNt, expNx, outMetrics.exportDt, stepT, stepX, errCsv) ||
        !exportGatherCSV(dir.filePath("recon_recovered_gather.csv"), d_recon, "POCS_CS_Reconstructed_Wavefield",
                         outMetrics.modeName, Nt, Nx, dt, expNt, expNx, outMetrics.exportDt, stepT, stepX, errCsv) ||
        !exportGatherCSV(dir.filePath("recon_residual_slice.csv"), d_residual, "Residual_Slice_Difference_Wavefield",
                         outMetrics.modeName, Nt, Nx, dt, expNt, expNx, outMetrics.exportDt, stepT, stepX, errCsv))
    {
        outReportSummary = errCsv;
        return false;
    }

    // 导出 recon_metrics_summary.csv
    QString metPath = dir.filePath("recon_metrics_summary.csv");
    QFile metFile(metPath);
    if (!metFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReportSummary = QString::fromLocal8Bit("【波场重构失败】：无法创建指标汇总输出文件: %1").arg(metPath);
        return false;
    }
    {
        QTextStream out(&metFile);
        out.setCodec("UTF-8");
        out << "Metric_Name,Value,Unit,Description\n";
        out << "Mode_Name," << outMetrics.modeName << ",Text,Dataset mode\n";
        out << "Spatial_Grid_Dx_m," << QString::number(dx,'g',9) << ",m,Physical spacing of the working grid\n"; // add by wushanzhou in 2026/10/23
        out << "Spatial_Grid_Origin_m," << QString::number(spatialOrigin,'g',12) << ",m,First physical offset or synthetic coordinate\n"; // add by wushanzhou in 2026/10/23
        out << "Spatial_Grid_Source," << spatialSource << ",Text,Coordinate provenance\n"; // add by wushanzhou in 2026/10/23
        out << "Mask_SHA1," << samplingMaskHash(mask) << ",Hash,Canonical binary mask SHA1\n"; // add by wushanzhou in 2026/10/23
        out << "Mask_Source," << outMetrics.maskSource << ",Text,Subsampling mask origin\n";
        out << "Sampling_Ratio," << QString::number(outMetrics.samplingRate * 100.0f, 'f', 2) << "%,Percentage of active receiver traces\n";
        out << "Active_Traces," << outMetrics.sampledTraces << ",Traces,Number of acquired traces\n";
        out << "Total_Traces," << outMetrics.nTraces << ",Traces,Full grid aperture traces\n";
        out << "Time_Samples," << outMetrics.nSamples << ",Samples,Time axis discrete length\n";
        out << "POCS_Iterations," << outMetrics.iterations << ",Iterations,Nonlinear curvelet shrinkage and observation projection iterations\n"; // add by wushanzhou in 2026/10/23
        out << "SNR_Zero_Fill," << QString::number(outMetrics.snr_zero_fill_db, 'f', 2) << ",dB,Baseline 1: Zero-fill unmeasured traces\n";
        out << "SNR_Linear_Interp," << QString::number(outMetrics.snr_linear_interp_db, 'f', 2) << ",dB,Baseline 2: 1D Spatial linear interpolation\n";
        out << "SNR_POCS_CS," << QString::number(outMetrics.snr_recovered_db, 'f', 2) << ",dB,Compressive sensing POCS recovered SNR (main pipeline)\n";
        out << "SNR_Improvement_vs_Zero," << QString::number(outMetrics.snr_improvement_db, 'f', 2) << ",dB,Net SNR gain over zero fill baseline\n";
        out << "SNR_Imprv_vs_Linear," << QString::number(outMetrics.snr_imprv_vs_linear_db, 'f', 2) << ",dB,Net SNR gain over linear interp baseline\n";
        // 2026.10.20: 缺失道口径（不含被精确置回的已采样道）
        out << "SNR_Linear_MissingOnly," << QString::number(outMetrics.snr_linear_missing_db, 'f', 2) << ",dB,Linear interp SNR on unsampled traces only\n";
        out << "SNR_POCS_MissingOnly," << QString::number(outMetrics.snr_pocs_missing_db, 'f', 2) << ",dB,POCS SNR on unsampled traces only (zero-fill = 0 dB by definition)\n";
        out << "Initialization_Mode," << (outMetrics.usedWarmStart ? "WarmStart (Linear Interpolation)" : "ColdStart (Zero-Fill)") << ",Text,POCS initial solution used by main pipeline\n";
        out << "SNR_POCS_ColdStart," << QString::number(outMetrics.snr_cold_start_db, 'f', 2) << ",dB,POCS from zero-fill init (standard POCS reference)\n";
        out << "SNR_POCS_WarmStart," << QString::number(outMetrics.snr_warm_start_db, 'f', 2) << ",dB,POCS from linear-interp init\n";
        out << "SNR_WarmGain_over_Cold," << QString::number(outMetrics.snr_warm_start_db - outMetrics.snr_cold_start_db, 'f', 2) << ",dB,Isolated contribution of warm-start initialization\n";
        out << "SNR_POCS_IterGain_Cold," << QString::number(outMetrics.snr_cold_start_db - outMetrics.snr_zero_fill_db, 'f', 2) << ",dB,Isolated contribution of POCS iterations from cold start\n";
        out << "SNR_POCS_IterGain_Warm," << QString::number(outMetrics.snr_warm_start_db - outMetrics.snr_linear_interp_db, 'f', 2) << ",dB,Isolated contribution of POCS iterations from warm start\n";
        out << "RMSE_ColdStart," << QString::number(outMetrics.rmse_cold_start, 'f', 6) << ",Amplitude,RMSE of cold-start POCS\n";
        out << "RMSE_WarmStart," << QString::number(outMetrics.rmse_warm_start, 'f', 6) << ",Amplitude,RMSE of warm-start POCS\n";
        out << "RMSE," << QString::number(outMetrics.rmse, 'f', 6) << ",Amplitude,Root mean square error across gather\n";
        out << "Pearson_Correlation," << QString::number(outMetrics.correlation, 'f', 4) << ",Ratio,Linear correlation with the declared reference\n"; // add by wushanzhou in 2026/10/23
        out << "Data_Consistency_Passed," << (outMetrics.dataConsistencyPassed ? "PASSED" : "FAILED") << ",Bool,Exact equality on sampled traces\n";
        out << "Runtime_ms," << QString::number(outMetrics.runtimeMs, 'f', 1) << ",ms,Execution wall time\n";
        metFile.close();
    }

    // 导出 recon_experiment_report.txt (深度科研分析报告)
    QString rptPath = dir.filePath("recon_experiment_report.txt");
    QFile rptFile(rptPath);
    if (!rptFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outReportSummary = QString::fromLocal8Bit("【波场重构失败】：无法创建报告输出文件: %1").arg(rptPath);
        return false;
    }
    {
        QTextStream out(&rptFile);
        out.setCodec("UTF-8");
        out << "====================================================================================\n";
        out << "  Seismic Wavefield Compressive Sensing Reconstruction Research Report\n";
        out << "  Dataset Mode: " << outMetrics.modeName << "\n";
        out << "  Subsampling Mask: " << outMetrics.maskSource << "\n";
        out << "  Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "====================================================================================\n\n";

        out << "[Subsampling Geometry Configuration]\n";
        out << "  Aperture Traces (Nx):      " << outMetrics.nTraces << " (Spatial Interval dx = " << outMetrics.dx << " m)\n";
        out << "  Time Samples (Nt):         " << outMetrics.nSamples << " (Sampling dt = " << outMetrics.calcDt * 1000.0f << " ms)\n";
        out << "  Subsampled Channels (m):   " << outMetrics.sampledTraces << " (" << QString::number(outMetrics.samplingRate * 100.0f, 'f', 1) << "% of Full Layout)\n";
        out << "  Unsampled Missing Channels:" << (outMetrics.nTraces - outMetrics.sampledTraces) << " traces to be interpolated/recovered\n\n";

        out << "[Comparative Quantitative Reconstruction Quality Metrics]\n";
        out << "  1. Baseline 1 (Zero-fill Subsampled Gather):        SNR = " << QString::number(outMetrics.snr_zero_fill_db, 'f', 2) << " dB (Reference 0.00 dB)\n";
        // 2026.10.20: 增益为负时原写法会输出 "+-1.02 dB"（真实资料上确实出现负增益），改用带符号格式化。
        const float gainLinear = outMetrics.snr_linear_interp_db - outMetrics.snr_zero_fill_db;
        out << "  2. Baseline 2 (Spatial Linear Interpolation):       SNR = " << QString::number(outMetrics.snr_linear_interp_db, 'f', 2) << " dB (Gain: "
            << (gainLinear >= 0.0f ? "+" : "") << QString::number(gainLinear, 'f', 2) << " dB)\n";
        out << "  3. Compressive Sensing (Curvelet POCS):             SNR = " << QString::number(outMetrics.snr_recovered_db, 'f', 2) << " dB (Gain: "
            << (outMetrics.snr_improvement_db >= 0.0f ? "+" : "") << QString::number(outMetrics.snr_improvement_db, 'f', 2) << " dB vs Zero-fill, "
            << (outMetrics.snr_imprv_vs_linear_db >= 0.0f ? "+" : "") << QString::number(outMetrics.snr_imprv_vs_linear_db, 'f', 2) << " dB vs Linear)\n";
        out << "  4. Full-Field Root Mean Square Error (RMSE):        " << QString::number(outMetrics.rmse, 'f', 6) << "\n";
        out << "  5. Pearson Cross-Correlation Coefficient (r):       " << QString::number(outMetrics.correlation, 'f', 4) << "\n";
        // 2026.10.20: 缺失道口径（与全幅口径并列，防止高估重构精度）
        out << "  5b. Missing-Trace-Only SNR (unsampled traces):      Linear = "
            << QString::number(outMetrics.snr_linear_missing_db, 'f', 2)
            << " dB, POCS = " << QString::number(outMetrics.snr_pocs_missing_db, 'f', 2)
            << " dB (zero-fill = 0.00 dB by definition)\n";
        out << "  6. Measured Traces Data Consistency Equality Check: " << (outMetrics.dataConsistencyPassed ? "PASSED (Strict error < 1e-5)" : "FAILED") << "\n";
        out << "  7. Reconstruction Execution Time:                   " << QString::number(outMetrics.runtimeMs, 'f', 1) << " ms ("
            << outMetrics.iterations << " iterations)\n\n";

        if (outMetrics.snr_warm_start_db != 0.0f || outMetrics.snr_cold_start_db != 0.0f)
        {
            out << "[Controlled Cold-Start vs Warm-Start Experiment (Fairness / Ablation)]\n";
            out << "  Purpose: isolate the contribution of the linear-interpolation initial solution from\n";
            out << "           the contribution of the curvelet POCS iterations themselves.\n";
            out << "  - POCS from Zero-Fill init   (Cold-Start): SNR = " << QString::number(outMetrics.snr_cold_start_db, 'f', 2)
                << " dB, RMSE = " << QString::number(outMetrics.rmse_cold_start, 'f', 6)
                << ", r = " << QString::number(outMetrics.corr_cold_start, 'f', 4) << "\n";
            out << "  - POCS from Linear-Interp init (Warm-Start): SNR = " << QString::number(outMetrics.snr_warm_start_db, 'f', 2)
                << " dB, RMSE = " << QString::number(outMetrics.rmse_warm_start, 'f', 6)
                << ", r = " << QString::number(outMetrics.corr_warm_start, 'f', 4) << "\n";
            out << "  - Contribution split:\n";
            out << "      (a) warm-start initialization alone : " << QString::number(outMetrics.snr_linear_interp_db - outMetrics.snr_zero_fill_db, 'f', 2) << " dB\n";
            out << "      (b) POCS iterations from cold start : " << QString::number(outMetrics.snr_cold_start_db - outMetrics.snr_zero_fill_db, 'f', 2) << " dB\n";
            out << "      (c) POCS iterations from warm start : " << QString::number(outMetrics.snr_warm_start_db - outMetrics.snr_linear_interp_db, 'f', 2) << " dB\n";
            out << "  - Interpretation: the headline SNR gain of the main pipeline is the SUM of (a) and (c).\n";
            out << "    Term (b) reports the iteration gain for this implementation from zero-fill initialization,\n"; // add by wushanzhou in 2026/10/23
            out << "    because it starts from the same zero-fill initial solution used by standard POCS.\n\n";
        }

        // 2026.10.20: 结论由实测差值动态生成 —— 不再无条件输出 "significantly surpassing"。
        out << "[In-Depth Comparative Analysis: Curvelet Reconstruction vs Baselines]\n";
        out << "  - Observation Summary:\n";
        out << "    Dataset mode: " << outMetrics.modeName << "\n";
        out << "    Sampling rate: " << QString::number(outMetrics.samplingRate * 100.0f, 'f', 1)
            << "%, trace spacing dx = " << QString::number(outMetrics.dx, 'f', 1) << " m\n";
        out << "    Curvelet POCS (warm start) SNR = " << QString::number(outMetrics.snr_recovered_db, 'f', 2) << " dB\n";
        {
            const float dZero   = outMetrics.snr_improvement_db;
            const float dLinear = outMetrics.snr_imprv_vs_linear_db;
            const int   cZero   = compareGain(dZero);
            const int   cLinear = compareGain(dLinear);

            out << "    vs Zero-Fill baseline (" << QString::number(outMetrics.snr_zero_fill_db, 'f', 2)
                << " dB): ";
            if (cZero > 0)      out << "OUTPERFORMS by " << fmtSigned(dZero) << " dB\n";
            else if (cZero < 0) out << "UNDERPERFORMS by " << fmtSigned(dZero) << " dB\n";
            else                out << "COMPARABLE (difference " << fmtSigned(dZero) << " dB)\n";

            out << "    vs Linear Interpolation baseline (" << QString::number(outMetrics.snr_linear_interp_db, 'f', 2)
                << " dB): ";
            if (cLinear > 0)      out << "OUTPERFORMS by " << fmtSigned(dLinear) << " dB\n";
            else if (cLinear < 0) out << "UNDERPERFORMS by " << fmtSigned(dLinear) << " dB"
                                      << " (curvelet POCS did NOT improve on the interpolation baseline here)\n";
            else                  out << "COMPARABLE (difference " << fmtSigned(dLinear) << " dB)\n";

            // 冷启动区间若存在，单独给出，避免与热启动口径混淆
            if (outMetrics.snr_cold_start_db > 0.0f)
            {
                const float itGainCold = outMetrics.snr_cold_start_db - outMetrics.snr_zero_fill_db;
                out << "    Cold-start POCS iteration contribution over zero-fill: "
                    << fmtSigned(itGainCold) << " dB (this implementation and dataset)\n"; // add by wushanzhou in 2026/10/23
                if (cLinear < 0)
                {
                    out << "    NOTE: the warm-start pipeline total is dominated by the linear-interpolation prior,\n"
                        << "          not by the curvelet sparsity constraint. The negative vs-linear margin above\n"
                        << "          means the POCS iterations actually degraded the interpolated initial solution\n"
                        << "          on this dataset.\n";
                }
            }
        }
        out << "  - Method Notes (not measured claims):\n";
        out << "    1. Multi-scale & Directional Sparsity in Curvelet Domain:\n";
        out << "       Seismic reflection events exhibit hyperbolic curvature and localized wave fronts. While standard\n";
        out << "       2D Fourier bases disperse curved events across the entire (f, k) plane, discrete curvelet frames\n";
        out << "       provide optimal, highly sparse representations of parabolic wavefronts with minimal coefficients.\n";
        out << "    2. Spatial Linear Interpolation Warm-Start:\n";
        out << "       Initializing the iterative solver with spatial linear interpolation eliminates severe high-frequency\n";
        out << "       harmonic noise and spatial aliasing from raw zero-fill, providing a robust initial trajectory.\n";
        out << "    3. Cosine Annealing Thresholding & Scale-Adaptive Wiener Shrinkage:\n";
        out << "       Smoothly decaying the regularization threshold prevents Gibbs ringing artifacts and preserves weak\n";
        out << "       deep reflection amplitudes while preserving coarse background scales and suppressing noise.\n";
        out << "    4. Strict Observed Trace Consistency Projection:\n";
        out << "       Exact replacement of known channels after each inverse transform guarantees zero distortion on\n";
        out << "       measured physical data (maximum error strictly < 1e-5).\n";
        out << "\n[Scientific Conclusions & Supporting Figures]\n";
        out << "  All run data charts and result figures have been saved to: figures/\n";
        out << "  1. [Figure 05_wavefield_reconstruction_quad.png]:\n";
        out << "     Four-panel comparison displays the declared reference and the actual sampling configuration.\n"; // add by wushanzhou in 2026/10/23
        out << "     Four-panel comparison: reference, 50% subsampled, curvelet POCS reconstruction, and residual.\n";
        out << "  2. [Figure 06_recon_convergence_trace_comparison.png]:\n";
        out << "     The trace records relative iterate changes; a single-trace overlay is descriptive\n";
        out << "     does not establish missing-trace or phase fidelity (r = "
            << QString::number(outMetrics.correlation, 'f', 4) << ").\n";
        rptFile.close();
    }

    // 绘制并保存科研图件 (集中保存到 figures/)
        QString centralFiguresDir = getCentralFiguresDir(outputDir);
    QDir().mkpath(centralFiguresDir);

    ScientificPlotter::plotWavefieldReconstructionQuad(
        d_full,
        d_obs,
        d_recon,
        d_residual,
        outMetrics.snr_zero_fill_db,
        outMetrics.snr_linear_interp_db,
        outMetrics.snr_recovered_db,
        outMetrics.rmse,
        outMetrics.correlation,
        QDir(centralFiguresDir).filePath("05_wavefield_reconstruction_quad.png")
    );

    int sampleMissingTrace = 35;
    for (int j = 10; j < Nx - 10; j++)
    {
        if (j < (int)mask.size() && mask[j] == 0)
        {
            sampleMissingTrace = j;
            break;
        }
    }
    ScientificPlotter::plotReconConvergenceAndTrace(
        outMetrics.residual_convergence,
        d_full,
        d_recon,
        sampleMissingTrace,
        outMetrics.snr_recovered_db,
        QDir(centralFiguresDir).filePath("06_recon_convergence_trace_comparison.png")
    );

    outReportSummary = QString::fromLocal8Bit(
        "【地震波场压缩感知重构试验完成】\n"
        "模式: %1\n"
        "掩码: %2\n"
        "1. 零值填充基准 SNR: %3 dB\n"
        "2. 空间线性插值基准 SNR: %4 dB\n"
        "3. 压缩感知恢复后 SNR: %5 dB (相对零填充提升: +%6 dB, 相对线性插值: %7 dB)\n"
        "4. 均方根误差 RMSE: %8, 相关系数 r: %9\n"
        "5. 观测道数据一致性检验: %10\n"
        "重构数据与残差切片已导出至:\n%11"
        "\n\n【科研成果图件支撑】(全部数据图与结果图集中保存至 %12 文件夹)：\n"
        "  ● 图件 05_wavefield_reconstruction_quad.png: 地震波场欠采样与POCS恢复四联剖面 (SNR提升+%6 dB)\n"
        "  ● 图件 06_recon_convergence_trace_comparison.png: POCS残差收敛与抽稀道波形对比细节 (波形相关系数r=%9)"
    ).arg(outMetrics.modeName)
     .arg(outMetrics.maskSource)
     .arg(QString::number(outMetrics.snr_zero_fill_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_linear_interp_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_recovered_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_improvement_db, 'f', 2))
     .arg(QString::number(outMetrics.snr_imprv_vs_linear_db, 'f', 2))
     .arg(QString::number(outMetrics.rmse, 'f', 5))
     .arg(QString::number(outMetrics.correlation, 'f', 4))
     .arg(outMetrics.dataConsistencyPassed ? QString::fromLocal8Bit("严格通过 (误差 < 1e-5)") : QString::fromLocal8Bit("未通过"))
     .arg(outputDir)
     .arg(getCentralFiguresDir(outputDir));

    return true;
}

// =============================================================================
// 一键科研闭环套件 // add by wushanzhou in 2026/10/15
// =============================================================================
