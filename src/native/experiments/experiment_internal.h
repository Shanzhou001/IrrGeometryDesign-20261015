#ifndef IRR_EXPERIMENT_INTERNAL_H
#define IRR_EXPERIMENT_INTERNAL_H
#include "../plotting/scientificplotter.h"
// add by wushanzhou in 2026/10/15
#include "geometryexperimentengine.h"
#include "../gui/survey/gcellgrid.h"
#include "../sampling/sample.h"
#include "../sampling/CalculateMiu.h"
#include "../optimization/OptimizeMiu.h"
#include "../core/ExpRng.h"
#include "../reconstruction/reconstructionoptions.h" // add by wushanzhou in 2026/10/24
#include "../geometry/spsloadcal.h" // add by wushanzhou in 2026/10/24
#include "../../third_party/curvelet/Curvelet2d/fdct_wrapping.hpp"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QTextCodec>
#include <QCoreApplication>
#include <QList>
#include <QByteArray>
#include <QCryptographicHash>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>
#include <vector>
#include <complex>
#include <map>
#include <utility>
#include <limits> // add by wushanzhou in 2026/10/24


extern const float PI_CONST;
bool checkFileExistsAndNonEmpty(const QString& filePath);
bool reconstructionFinite(double v);
QString samplingMaskHash(const std::vector<int>& mask);
bool saveSamplingMask(const QString& path, const ConstrainedResult& result, 
                              unsigned long long seed, int budget, const QString& objective, const QString& protocol=QString());
QString fmtSigned(float v, int prec = 2);
int compareGain(float delta, float tol = 0.05f);
QString getCentralFiguresDir(const QString& currentOutputDir);
QString findRepositorySpsDir();
GProject* loadRepositorySpsProject();
void internal_fft1d(std::vector<std::complex<float> >& a, bool invert);
void internal_fft2d(std::vector<std::vector<std::complex<float> > >& mat, bool invert);
bool exportGatherCSV(const QString& filePath,
                           const std::vector<std::vector<float> >& matData,
                           const QString& headerNote,
                           const QString& modeName,
                           int Nt, int Nx, float dt,
                           int expNt, int expNx, float exportDt,
                           int stepT, int stepX,
                           QString& outErrorMsg);
int uniformity_max_gap(const int* mask, int N);
float uniformity_nn_cv(const int* mask, int N);
float uniformity_unit_occupancy(const int* mask, int N, int m);
bool runMiuParamRobustness(int N, float c, const QString& outputDir, QString& outReportSummary);
void curveletPocsReconstruct(const std::vector<std::vector<float> >& initGather,
                                    const std::vector<std::vector<float> >& dObs,
                                    const std::vector<int>& mask,
                                    int numIter,
                                    std::vector<std::vector<float> >& outRecon,
                                    std::vector<std::vector<float> >& outResidual,
                                    std::vector<float>& outStepRes, 
                                    const std::vector<double>* positions = NULL, 
                                    const std::vector<std::vector<float> >* measured = NULL, 
                                    const std::vector<int>* measuredMask = NULL, 
                                    const ReconstructionOptions* options = NULL);

void log_test(FILE *fp, const char *fmt, ...);
bool verifyErrMsg(const QString& msg);
bool removeDirRecursive(const QString& path);
bool loadHoldoutGather(const QString& path, std::vector<std::vector<double> >& data, 
                              std::vector<double>& coordinates, QString& error);
float interpolateKnown(const std::vector<float>& values,const std::vector<int>& known, 
                               const std::vector<double>& x,double position);
bool coordinateHoldoutReconstruct(const std::vector<std::vector<float> >& truth, 
    const std::vector<int>& mask,const std::vector<double>& x,int iterations, 
    std::vector<std::vector<float> >& linear,std::vector<std::vector<float> >& reconstructed, 
    double& consistencyError,double* outputScale=NULL,const ReconstructionOptions* options=NULL, 
    std::vector<float>* convergence=NULL);
#endif
