# Maintained Qt4 build of the same source list as the VS2008 project.
TEMPLATE = app
TARGET = IrrGeometryDesign
QT += core gui qt3support opengl
CONFIG += warn_on
DEFINES += QT3_SUPPORT QT_QT3SUPPORT_LIB
DESTDIR = $$OUT_PWD/bin
OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/generated/moc
UI_DIR = $$OUT_PWD/generated
RCC_DIR = $$OUT_PWD/generated
INCLUDEPATH += $$UI_DIR ../../third_party/eigen-x64
win32:LIBS += -L../../third_party/legacy-libs -lfftw -lfdct_wrapping_param -lopengl32 -lglu32 -lcomdlg32
win32-msvc*:QMAKE_CXXFLAGS += /openmp
RESOURCES += ../../resources/qt/datareconstruction.qrc
TRANSLATIONS += ../../resources/qt/DataReconstruction_zh.ts
SOURCES += \
    ../../native/app/datareconstruction.cpp \
    ../../native/app/main.cpp \
    ../../native/core/ExpRng.cpp \
    ../../native/core/gmath.cpp \
    ../../native/core/gmatrix3d.cpp \
    ../../native/core/gpoint3d.cpp \
    ../../native/core/gquaternion3d.cpp \
    ../../native/core/grotation3d.cpp \
    ../../native/core/gutils.cpp \
    ../../native/core/gvector3d.cpp \
    ../../native/experiments/auditqueries.cpp \
    ../../native/experiments/geometry_experiments.cpp \
    ../../native/experiments/geometryexperimentengine.cpp \
    ../../native/experiments/reviewexperiments.cpp \
    ../../native/experiments/sampling_experiments.cpp \
    ../../native/experiments/samplingstudies.cpp \
    ../../native/experiments/workflow.cpp \
    ../../native/geometry/gAttrAnalyzeParam.cpp \
    ../../native/geometry/spsloadcal.cpp \
    ../../native/gui/dialogs/analyzebinform.cpp \
    ../../native/gui/dialogs/changecoordinatedlg.cpp \
    ../../native/gui/dialogs/draw2dfrm.cpp \
    ../../native/gui/dialogs/geometrytemplateparameter.cpp \
    ../../native/gui/dialogs/ovtvolumeattributeview.cpp \
    ../../native/gui/dialogs/spssampledlg.cpp \
    ../../native/gui/dialogs/tchangeshowareadlg.cpp \
    ../../native/gui/dialogs/tdatawizardform.cpp \
    ../../native/gui/dialogs/tgeometryimfordlg.cpp \
    ../../native/gui/dialogs/tqfdividedlg.cpp \
    ../../native/gui/dialogs/trealovtdlg.cpp \
    ../../native/gui/dialogs/trosedividedlg.cpp \
    ../../native/gui/survey/gcellgrid.cpp \
    ../../native/gui/survey/gspsparam.cpp \
    ../../native/gui/survey/gspsview.cpp \
    ../../native/gui/survey/tcellgridform.cpp \
    ../../native/gui/survey/tspsdlg.cpp \
    ../../native/gui/survey/tspsform.cpp \
    ../../native/gui/views/analyzebindoc.cpp \
    ../../native/gui/views/analyzebinview.cpp \
    ../../native/gui/views/colorview.cpp \
    ../../native/gui/views/draw2Dview.cpp \
    ../../native/io/sps/gcmpset.cpp \
    ../../native/io/sps/gproject.cpp \
    ../../native/io/sps/grlineset.cpp \
    ../../native/io/sps/gsegyfilescanner.cpp \
    ../../native/io/sps/gshotset.cpp \
    ../../native/io/sps/gslineset.cpp \
    ../../native/io/sps/gspsscanner.cpp \
    ../../native/io/sps/segyio.cpp \
    ../../native/optimization/OptimizeMiu.cpp \
    ../../native/reconstruction/coordinateholdout.cpp \
    ../../native/reconstruction/wavefield_experiments.cpp \
    ../../native/sampling/CurveletMiu.cpp \
    ../../native/sampling/Diagmul.cpp \
    ../../native/sampling/Sampling.cpp \
    ../../native/sampling/spsmaskbuilder.cpp \
    ../../native/tests/engine_regression.cpp \
    ../../third_party/curvelet/Curvelet2d/fdct_wrapping.cpp \
    ../../third_party/curvelet/Curvelet2d/ifdct_wrapping.cpp \
    ../../third_party/qcustomplot/qcustomplot.cpp
HEADERS += \
    ../../native/app/datareconstruction.h \
    ../../native/core/ExpRng.h \
    ../../native/core/gmath.h \
    ../../native/core/gmatrix3d.h \
    ../../native/core/gpoint3d.h \
    ../../native/core/gptrvector.h \
    ../../native/core/gquaternion3d.h \
    ../../native/core/grotation3d.h \
    ../../native/core/gutils.h \
    ../../native/core/gvaluevector.h \
    ../../native/core/gvector3d.h \
    ../../native/experiments/experiment_internal.h \
    ../../native/experiments/geometryexperimentengine.h \
    ../../native/geometry/gAttrAnalyzeParam.h \
    ../../native/geometry/spsloadcal.h \
    ../../native/gui/dialogs/analyzebinform.h \
    ../../native/gui/dialogs/changecoordinatedlg.h \
    ../../native/gui/dialogs/draw2dfrm.h \
    ../../native/gui/dialogs/geometrytemplateparameter.h \
    ../../native/gui/dialogs/ovtvolumeattributeview.h \
    ../../native/gui/dialogs/spssampledlg.h \
    ../../native/gui/dialogs/tchangeshowareadlg.h \
    ../../native/gui/dialogs/tdatawizardform.h \
    ../../native/gui/dialogs/tgeometryimfordlg.h \
    ../../native/gui/dialogs/tqfdividedlg.h \
    ../../native/gui/dialogs/trealovtdlg.h \
    ../../native/gui/dialogs/trosedividedlg.h \
    ../../native/gui/survey/gcellgrid.h \
    ../../native/gui/survey/gspsparam.h \
    ../../native/gui/survey/gspsview.h \
    ../../native/gui/survey/gvaluevector.h \
    ../../native/gui/survey/tcellgridform.h \
    ../../native/gui/survey/tspsdlg.h \
    ../../native/gui/survey/tspsform.h \
    ../../native/gui/views/analyzebindoc.h \
    ../../native/gui/views/analyzebinview.h \
    ../../native/gui/views/colorview.h \
    ../../native/gui/views/draw2Dview.h \
    ../../native/io/sps/gcmpset.h \
    ../../native/io/sps/gproject.h \
    ../../native/io/sps/grlineset.h \
    ../../native/io/sps/gsegyfilescanner.h \
    ../../native/io/sps/gshotset.h \
    ../../native/io/sps/gslineset.h \
    ../../native/io/sps/gspsscanner.h \
    ../../native/io/sps/segyio.h \
    ../../native/optimization/adaptivesearch.inc \
    ../../native/optimization/OptimizeMiu.h \
    ../../native/plotting/figurepaths.h \
    ../../native/sampling/CalculateMiu.h \
    ../../native/sampling/DiagMulmatrix.h \
    ../../native/sampling/sample.h \
    ../../native/sampling/spsmaskbuilder.h \
    ../../native/tests/reviewregression.inc \
    ../../native/tests/scienceregression.inc \
    ../../third_party/curvelet/Curvelet2d/fdct_wrapping.hpp \
    ../../third_party/qcustomplot/qcustomplot.h
FORMS += \
    ../../native/app/datareconstruction.ui \
    ../../native/gui/dialogs/analyzebinform.ui \
    ../../native/gui/dialogs/changecoordinatedlg.ui \
    ../../native/gui/dialogs/draw2dfrm.ui \
    ../../native/gui/dialogs/geometrytemplateparameter.ui \
    ../../native/gui/dialogs/ovtvolumeattributeview.ui \
    ../../native/gui/dialogs/spssampledlg.ui \
    ../../native/gui/dialogs/tchangeshowareadlg.ui \
    ../../native/gui/dialogs/tdatawizardform.ui \
    ../../native/gui/dialogs/tgeometryimfordlg.ui \
    ../../native/gui/dialogs/tqfdividedlg.ui \
    ../../native/gui/dialogs/trealovtdlg.ui \
    ../../native/gui/dialogs/trosedividedlg.ui \
    ../../native/gui/survey/tcellgridform.ui \
    ../../native/gui/survey/tspsdlg.ui \
    ../../native/gui/survey/tspsform.ui

INCLUDEPATH += ../../native/app ../../native/core ../../native/experiments ../../native/geometry ../../native/gui/dialogs ../../native/gui/survey ../../native/gui/views ../../native/io/sps ../../native/optimization ../../native/plotting ../../native/reconstruction ../../native/sampling ../../third_party/curvelet/Curvelet2d ../../third_party/curvelet/Curvelet3d ../../third_party/qcustomplot

SOURCES += ../../native/cli/commandline.cpp ../../native/gui/capture.cpp
HEADERS += ../../native/cli/commandline.h ../../native/gui/capture.h
