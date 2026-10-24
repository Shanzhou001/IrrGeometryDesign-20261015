// // add by wushanzhou in 2026/10/15
#ifndef SPSMASKBUILDER_H
#define SPSMASKBUILDER_H

#include <qstring.h>
#include <qvector.h>
#include <qmap.h>
#include "../io/sps/gproject.h"
#include "../core/gutils.h"

// ======================================================
// SPSMaskBuilder
// 1) Trace mask : shot x receiver
// 2) CMP/bin mask : yBin x xBin
//
// 说明：
// - traceMask[row=shotIndex][col=globalReceiverIndex]
// - cmpMask[row=yBin][col=xBin]，有采样为1，否则0
// - cmpFold[row=yBin][col=xBin]，记录落入该bin的炮检对数量
// ======================================================

class SPSMaskBuilder
{
public:
    SPSMaskBuilder();
    ~SPSMaskBuilder();

    void clear();

    // ---------------------------
    // 1. Trace mask
    // ---------------------------
    bool buildTraceMaskFromProject(GProject *pProject);

    int **getTraceMask() const { return mpTraceMask; }
    int getTraceShotCount() const { return mTraceShotCount; }
    int getTraceReceiverCount() const { return mTraceReceiverCount; }

    const QVector<QString> &getShotKeys() const { return mShotKeys; }
    const QVector<QString> &getReceiverKeys() const { return mReceiverKeys; }

    bool exportTraceMaskTxt(const QString &fileName) const;
    bool exportShotIndexTxt(const QString &fileName) const;
    bool exportReceiverIndexTxt(const QString &fileName) const;

    // ---------------------------
    // 2. CMP/bin mask
    // ---------------------------
    // 若 min/max 未指定（max<=min），则自动根据 midpoint 范围扫描
    bool buildCMPMaskFromProject(GProject *pProject,
                                 float binX,
                                 float binY,
                                 float minX = 0.0f,
                                 float maxX = -1.0f,
                                 float minY = 0.0f,
                                 float maxY = -1.0f);

    int **getCMPMask() const { return mpCMPMask; }
    int **getCMPFold() const { return mpCMPFold; }

    int getCMPXCount() const { return mCMPXCount; }
    int getCMPYCount() const { return mCMPYCount; }

    float getCMPMinX() const { return mCMPMinX; }
    float getCMPMaxX() const { return mCMPMaxX; }
    float getCMPMinY() const { return mCMPMinY; }
    float getCMPMaxY() const { return mCMPMaxY; }

    float getCMPBinX() const { return mCMPBinX; }
    float getCMPBinY() const { return mCMPBinY; }

    int getCMPPairCount() const { return mCMPPairCount; }

    bool exportCMPMaskTxt(const QString &fileName) const;
    bool exportCMPFoldTxt(const QString &fileName) const;
    bool exportCMPMetaTxt(const QString &fileName) const;

private:
    // trace helpers
    bool buildReceiverIndex(GRLineSet *pRLineSet);
    QString makeReceiverKey(GRPoint *pRPoint) const;
    QString makeShotKey(GShot *pShot) const;

    // memory helpers
    void freeTraceMask();
    void freeCMPMask();

private:
    // ---------------------------
    // Trace mask
    // ---------------------------
    int **mpTraceMask;
    int mTraceShotCount;
    int mTraceReceiverCount;

    QVector<QString> mShotKeys;
    QVector<QString> mReceiverKeys;
    QMap<QString, int> mReceiverKeyToIndex;

    // ---------------------------
    // CMP/bin mask
    // ---------------------------
    int **mpCMPMask;
    int **mpCMPFold;

    int mCMPXCount;
    int mCMPYCount;
    int mCMPPairCount;

    float mCMPMinX;
    float mCMPMaxX;
    float mCMPMinY;
    float mCMPMaxY;
    float mCMPBinX;
    float mCMPBinY;
};

#endif // SPSMASKBUILDER_H