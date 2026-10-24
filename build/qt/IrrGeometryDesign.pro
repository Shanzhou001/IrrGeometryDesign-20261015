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
    ../../src/native/app/datareconstruction.cpp \
    ../../src/native/app/main.cpp \
    ../../src/native/core/ExpRng.cpp \
    ../../src/native/core/gmath.cpp \
    ../../src/native/core/gmatrix3d.cpp \
    ../../src/native/core/gpoint3d.cpp \
    ../../src/native/core/gquaternion3d.cpp \
    ../../src/native/core/grotation3d.cpp \
    ../../src/native/core/gutils.cpp \
    ../../src/native/core/gvector3d.cpp \
    ../../src/native/experiments/auditqueries.cpp \
    ../../src/native/experiments/geometry_experiments.cpp \
    ../../src/native/experiments/geometryexperimentengine.cpp \
    ../../src/native/experiments/reviewexperiments.cpp \
    ../../src/native/experiments/sampling_experiments.cpp \
    ../../src/native/experiments/samplingstudies.cpp \
    ../../src/native/experiments/workflow.cpp \
    ../../src/native/geometry/gAttrAnalyzeParam.cpp \
    ../../src/native/geometry/spsloadcal.cpp \
    ../../src/native/gui/dialogs/analyzebinform.cpp \
    ../../src/native/gui/dialogs/changecoordinatedlg.cpp \
    ../../src/native/gui/dialogs/draw2dfrm.cpp \
    ../../src/native/gui/dialogs/geometrytemplateparameter.cpp \
    ../../src/native/gui/dialogs/ovtvolumeattributeview.cpp \
    ../../src/native/gui/dialogs/spssampledlg.cpp \
    ../../src/native/gui/dialogs/tchangeshowareadlg.cpp \
    ../../src/native/gui/dialogs/tdatawizardform.cpp \
    ../../src/native/gui/dialogs/tgeometryimfordlg.cpp \
    ../../src/native/gui/dialogs/tqfdividedlg.cpp \
    ../../src/native/gui/dialogs/trealovtdlg.cpp \
    ../../src/native/gui/dialogs/trosedividedlg.cpp \
    ../../src/native/gui/survey/gcellgrid.cpp \
    ../../src/native/gui/survey/gspsparam.cpp \
    ../../src/native/gui/survey/gspsview.cpp \
    ../../src/native/gui/survey/tcellgridform.cpp \
    ../../src/native/gui/survey/tspsdlg.cpp \
    ../../src/native/gui/survey/tspsform.cpp \
    ../../src/native/gui/views/analyzebindoc.cpp \
    ../../src/native/gui/views/analyzebinview.cpp \
    ../../src/native/gui/views/colorview.cpp \
    ../../src/native/gui/views/draw2Dview.cpp \
    ../../src/native/io/sps/gcmpset.cpp \
    ../../src/native/io/sps/gproject.cpp \
    ../../src/native/io/sps/grlineset.cpp \
    ../../src/native/io/sps/gsegyfilescanner.cpp \
    ../../src/native/io/sps/gshotset.cpp \
    ../../src/native/io/sps/gslineset.cpp \
    ../../src/native/io/sps/gspsscanner.cpp \
    ../../src/native/io/sps/segyio.cpp \
    ../../src/native/optimization/OptimizeMiu.cpp \
    ../../src/native/reconstruction/coordinateholdout.cpp \
    ../../src/native/reconstruction/wavefield_experiments.cpp \
    ../../src/native/sampling/CurveletMiu.cpp \
    ../../src/native/sampling/Diagmul.cpp \
    ../../src/native/sampling/Sampling.cpp \
    ../../src/native/sampling/spsmaskbuilder.cpp \
    ../../src/native/tests/engine_regression.cpp \
    ../../third_party/curvelet/Curvelet2d/fdct_wrapping.cpp \
    ../../third_party/curvelet/Curvelet2d/ifdct_wrapping.cpp \
    ../../third_party/qcustomplot/qcustomplot.cpp
HEADERS += \
    ../../src/native/app/datareconstruction.h \
    ../../src/native/core/ExpRng.h \
    ../../src/native/core/gmath.h \
    ../../src/native/core/gmatrix3d.h \
    ../../src/native/core/gpoint3d.h \
    ../../src/native/core/gptrvector.h \
    ../../src/native/core/gquaternion3d.h \
    ../../src/native/core/grotation3d.h \
    ../../src/native/core/gutils.h \
    ../../src/native/core/gvaluevector.h \
    ../../src/native/core/gvector3d.h \
    ../../src/native/experiments/experiment_internal.h \
    ../../src/native/experiments/geometryexperimentengine.h \
    ../../src/native/geometry/gAttrAnalyzeParam.h \
    ../../src/native/geometry/spsloadcal.h \
    ../../src/native/gui/dialogs/analyzebinform.h \
    ../../src/native/gui/dialogs/changecoordinatedlg.h \
    ../../src/native/gui/dialogs/draw2dfrm.h \
    ../../src/native/gui/dialogs/geometrytemplateparameter.h \
    ../../src/native/gui/dialogs/ovtvolumeattributeview.h \
    ../../src/native/gui/dialogs/spssampledlg.h \
    ../../src/native/gui/dialogs/tchangeshowareadlg.h \
    ../../src/native/gui/dialogs/tdatawizardform.h \
    ../../src/native/gui/dialogs/tgeometryimfordlg.h \
    ../../src/native/gui/dialogs/tqfdividedlg.h \
    ../../src/native/gui/dialogs/trealovtdlg.h \
    ../../src/native/gui/dialogs/trosedividedlg.h \
    ../../src/native/gui/survey/gcellgrid.h \
    ../../src/native/gui/survey/gspsparam.h \
    ../../src/native/gui/survey/gspsview.h \
    ../../src/native/gui/survey/gvaluevector.h \
    ../../src/native/gui/survey/tcellgridform.h \
    ../../src/native/gui/survey/tspsdlg.h \
    ../../src/native/gui/survey/tspsform.h \
    ../../src/native/gui/views/analyzebindoc.h \
    ../../src/native/gui/views/analyzebinview.h \
    ../../src/native/gui/views/colorview.h \
    ../../src/native/gui/views/draw2Dview.h \
    ../../src/native/io/sps/gcmpset.h \
    ../../src/native/io/sps/gproject.h \
    ../../src/native/io/sps/grlineset.h \
    ../../src/native/io/sps/gsegyfilescanner.h \
    ../../src/native/io/sps/gshotset.h \
    ../../src/native/io/sps/gslineset.h \
    ../../src/native/io/sps/gspsscanner.h \
    ../../src/native/io/sps/segyio.h \
    ../../src/native/optimization/adaptivesearch.inc \
    ../../src/native/optimization/OptimizeMiu.h \
    ../../src/native/plotting/figurepaths.h \
    ../../src/native/sampling/CalculateMiu.h \
    ../../src/native/sampling/DiagMulmatrix.h \
    ../../src/native/sampling/sample.h \
    ../../src/native/sampling/spsmaskbuilder.h \
    ../../src/native/tests/reviewregression.inc \
    ../../src/native/tests/scienceregression.inc \
    ../../third_party/curvelet/Curvelet2d/fdct_wrapping.hpp \
    ../../third_party/qcustomplot/qcustomplot.h
FORMS += \
    ../../src/native/app/datareconstruction.ui \
    ../../src/native/gui/dialogs/analyzebinform.ui \
    ../../src/native/gui/dialogs/changecoordinatedlg.ui \
    ../../src/native/gui/dialogs/draw2dfrm.ui \
    ../../src/native/gui/dialogs/geometrytemplateparameter.ui \
    ../../src/native/gui/dialogs/ovtvolumeattributeview.ui \
    ../../src/native/gui/dialogs/spssampledlg.ui \
    ../../src/native/gui/dialogs/tchangeshowareadlg.ui \
    ../../src/native/gui/dialogs/tdatawizardform.ui \
    ../../src/native/gui/dialogs/tgeometryimfordlg.ui \
    ../../src/native/gui/dialogs/tqfdividedlg.ui \
    ../../src/native/gui/dialogs/trealovtdlg.ui \
    ../../src/native/gui/dialogs/trosedividedlg.ui \
    ../../src/native/gui/survey/tcellgridform.ui \
    ../../src/native/gui/survey/tspsdlg.ui \
    ../../src/native/gui/survey/tspsform.ui

INCLUDEPATH += ../../src/native/app ../../src/native/core ../../src/native/experiments ../../src/native/geometry ../../src/native/gui/dialogs ../../src/native/gui/survey ../../src/native/gui/views ../../src/native/io/sps ../../src/native/optimization ../../src/native/plotting ../../src/native/reconstruction ../../src/native/sampling ../../third_party/curvelet/Curvelet2d ../../third_party/curvelet/Curvelet3d ../../third_party/qcustomplot

SOURCES += ../../src/native/cli/commandline.cpp ../../src/native/gui/capture.cpp
HEADERS += ../../src/native/cli/commandline.h ../../src/native/gui/capture.h
