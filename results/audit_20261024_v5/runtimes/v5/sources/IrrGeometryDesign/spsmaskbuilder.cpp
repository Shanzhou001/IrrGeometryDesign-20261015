// // add by wushanzhou in 2026/10/15
#include "spsmaskbuilder.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <math.h>

SPSMaskBuilder::SPSMaskBuilder()
{
    mpTraceMask = NULL;
    mTraceShotCount = 0;
    mTraceReceiverCount = 0;

    mpCMPMask = NULL;
    mpCMPFold = NULL;
    mCMPXCount = 0;
    mCMPYCount = 0;
    mCMPPairCount = 0;
    mCMPMinX = 0.0f;
    mCMPMaxX = 0.0f;
    mCMPMinY = 0.0f;
    mCMPMaxY = 0.0f;
    mCMPBinX = 0.0f;
    mCMPBinY = 0.0f;
}

SPSMaskBuilder::~SPSMaskBuilder()
{
    clear();
}

void SPSMaskBuilder::clear()
{
    freeTraceMask();
    freeCMPMask();

    mShotKeys.clear();
    mReceiverKeys.clear();
    mReceiverKeyToIndex.clear();
}

void SPSMaskBuilder::freeTraceMask()
{
    if (mpTraceMask != NULL)
    {
        gFree2(mpTraceMask);
    }

    mpTraceMask = NULL;
    mTraceShotCount = 0;
    mTraceReceiverCount = 0;
}

void SPSMaskBuilder::freeCMPMask()
{
    if (mpCMPMask != NULL)
    {
        gFree2(mpCMPMask);
    }
    if (mpCMPFold != NULL)
    {
        gFree2(mpCMPFold);
    }

    mpCMPMask = NULL;
    mpCMPFold = NULL;
    mCMPXCount = 0;
    mCMPYCount = 0;
    mCMPPairCount = 0;
    mCMPMinX = 0.0f;
    mCMPMaxX = 0.0f;
    mCMPMinY = 0.0f;
    mCMPMaxY = 0.0f;
    mCMPBinX = 0.0f;
    mCMPBinY = 0.0f;
}

QString SPSMaskBuilder::makeReceiverKey(GRPoint *pRPoint) const
{
    if (pRPoint == NULL) return QString();
    return pRPoint->getLineName() + QString("_") + pRPoint->getPointNum();
}

QString SPSMaskBuilder::makeShotKey(GShot *pShot) const
{
    if (pShot == NULL) return QString();
    // 记录号 + 线号 + 点号，保证唯一性更稳
    return QString::number(pShot->getRecNum()) + QString("_")
         + pShot->getLineName() + QString("_")
         + pShot->getPointNum();
}

bool SPSMaskBuilder::buildReceiverIndex(GRLineSet *pRLineSet)
{
    if (pRLineSet == NULL) return false;

    mReceiverKeys.clear();
    mReceiverKeyToIndex.clear();

    int idx = 0;
    int nLine = pRLineSet->count();
    for (int i = 0; i < nLine; ++i)
    {
        GRLine *pLine = pRLineSet->get(i);
        if (pLine == NULL) continue;

        int nPt = pLine->count();
        for (int j = 0; j < nPt; ++j)
        {
            GRPoint *pPt = pLine->get(j);
            if (pPt == NULL) continue;

            QString key = makeReceiverKey(pPt);
            if (!mReceiverKeyToIndex.contains(key))
            {
                mReceiverKeyToIndex.insert(key, idx);
                mReceiverKeys.append(key);
                ++idx;
            }
        }
    }

    return (idx > 0);
}

// ======================================================
// 1) trace mask
// ======================================================
bool SPSMaskBuilder::buildTraceMaskFromProject(GProject *pProject)
{
    freeTraceMask();
    mShotKeys.clear();

    if (pProject == NULL) return false;

    GShotSet *pShotSet = pProject->getShotSet();
    GRLineSet *pRLineSet = pProject->getRLineSet();

    if (pShotSet == NULL || pRLineSet == NULL) return false;
    if (pShotSet->count() <= 0 || pRLineSet->count() <= 0) return false;

    if (!buildReceiverIndex(pRLineSet)) return false;

    mTraceShotCount = pShotSet->count();
    mTraceReceiverCount = mReceiverKeys.count();

    if (mTraceShotCount <= 0 || mTraceReceiverCount <= 0) return false;

    gAlloc2(mpTraceMask, mTraceShotCount, mTraceReceiverCount);
    gFill2(mpTraceMask, 0, mTraceShotCount, mTraceReceiverCount);

    for (int iShot = 0; iShot < mTraceShotCount; ++iShot)
    {
        GShot *pShot = pShotSet->get(iShot);
        if (pShot == NULL) continue;

        mShotKeys.append(makeShotKey(pShot));

        int trcCount = pShot->getTrcCount();
        for (int iTrc = 0; iTrc < trcCount; ++iTrc)
        {
            GRPoint *pRPoint = pShot->getRPointByTrcIndex(iTrc, pRLineSet);
            if (pRPoint == NULL) continue;

            QString rKey = makeReceiverKey(pRPoint);
            if (!mReceiverKeyToIndex.contains(rKey)) continue;

            int rIdx = mReceiverKeyToIndex.value(rKey, -1);
            if (rIdx >= 0 && rIdx < mTraceReceiverCount)
            {
                mpTraceMask[iShot][rIdx] = 1;
            }
        }
    }

    return true;
}

bool SPSMaskBuilder::exportTraceMaskTxt(const QString &fileName) const
{
    if (mpTraceMask == NULL) return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# TraceMask rows=shot cols=receiver\n";
    out << "# shotCount=" << mTraceShotCount
        << " receiverCount=" << mTraceReceiverCount << "\n";

    for (int i = 0; i < mTraceShotCount; ++i)
    {
        for (int j = 0; j < mTraceReceiverCount; ++j)
        {
            out << mpTraceMask[i][j];
            if (j != mTraceReceiverCount - 1) out << "\t";
        }
        out << "\n";
    }

    file.close();
    return true;
}

bool SPSMaskBuilder::exportShotIndexTxt(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# shot_index\tshot_key\n";
    for (int i = 0; i < mShotKeys.count(); ++i)
    {
        out << i << "\t" << mShotKeys[i] << "\n";
    }

    file.close();
    return true;
}

bool SPSMaskBuilder::exportReceiverIndexTxt(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# receiver_index\treceiver_key\n";
    for (int i = 0; i < mReceiverKeys.count(); ++i)
    {
        out << i << "\t" << mReceiverKeys[i] << "\n";
    }

    file.close();
    return true;
}

// ======================================================
// 2) CMP/bin mask
// ======================================================
bool SPSMaskBuilder::buildCMPMaskFromProject(GProject *pProject,
                                             float binX,
                                             float binY,
                                             float minX,
                                             float maxX,
                                             float minY,
                                             float maxY)
{
    freeCMPMask();

    if (pProject == NULL) return false;
    if (binX <= 0.0f || binY <= 0.0f) return false;

    GShotSet *pShotSet = pProject->getShotSet();
    GRLineSet *pRLineSet = pProject->getRLineSet();

    if (pShotSet == NULL || pRLineSet == NULL) return false;
    if (pShotSet->count() <= 0 || pRLineSet->count() <= 0) return false;

    bool useInputRange = (maxX > minX && maxY > minY);

    float autoMinX = 0.0f, autoMaxX = 0.0f;
    float autoMinY = 0.0f, autoMaxY = 0.0f;
    bool firstValid = true;

    // -------- pass 1: 扫 midpoint 范围 --------
    for (int iShot = 0; iShot < pShotSet->count(); ++iShot)
    {
        GShot *pShot = pShotSet->get(iShot);
        if (pShot == NULL) continue;

        GSPoint *pSPoint = pShot->getSPoint();
        if (pSPoint == NULL) continue;

        float sx = pSPoint->getX();
        float sy = pSPoint->getY();

        int trcCount = pShot->getTrcCount();
        for (int iTrc = 0; iTrc < trcCount; ++iTrc)
        {
            GRPoint *pRPoint = pShot->getRPointByTrcIndex(iTrc, pRLineSet);
            if (pRPoint == NULL) continue;

            float rx = pRPoint->getX();
            float ry = pRPoint->getY();

            float mx = 0.5f * (sx + rx);
            float my = 0.5f * (sy + ry);

            if (firstValid)
            {
                autoMinX = autoMaxX = mx;
                autoMinY = autoMaxY = my;
                firstValid = false;
            }
            else
            {
                if (mx < autoMinX) autoMinX = mx;
                if (mx > autoMaxX) autoMaxX = mx;
                if (my < autoMinY) autoMinY = my;
                if (my > autoMaxY) autoMaxY = my;
            }
        }
    }

    if (firstValid) return false;

    if (useInputRange)
    {
        mCMPMinX = minX;
        mCMPMaxX = maxX;
        mCMPMinY = minY;
        mCMPMaxY = maxY;
    }
    else
    {
        mCMPMinX = autoMinX;
        mCMPMaxX = autoMaxX;
        mCMPMinY = autoMinY;
        mCMPMaxY = autoMaxY;
    }

    mCMPBinX = binX;
    mCMPBinY = binY;

    mCMPXCount = (int)floor((mCMPMaxX - mCMPMinX) / mCMPBinX) + 1;
    mCMPYCount = (int)floor((mCMPMaxY - mCMPMinY) / mCMPBinY) + 1;

    if (mCMPXCount <= 0) mCMPXCount = 1;
    if (mCMPYCount <= 0) mCMPYCount = 1;

    gAlloc2(mpCMPMask, mCMPYCount, mCMPXCount);
    gAlloc2(mpCMPFold, mCMPYCount, mCMPXCount);
    gFill2(mpCMPMask, 0, mCMPYCount, mCMPXCount);
    gFill2(mpCMPFold, 0, mCMPYCount, mCMPXCount);

    mCMPPairCount = 0;

    // -------- pass 2: 统计 fold & mask --------
    for (int iShot = 0; iShot < pShotSet->count(); ++iShot)
    {
        GShot *pShot = pShotSet->get(iShot);
        if (pShot == NULL) continue;

        GSPoint *pSPoint = pShot->getSPoint();
        if (pSPoint == NULL) continue;

        float sx = pSPoint->getX();
        float sy = pSPoint->getY();

        int trcCount = pShot->getTrcCount();
        for (int iTrc = 0; iTrc < trcCount; ++iTrc)
        {
            GRPoint *pRPoint = pShot->getRPointByTrcIndex(iTrc, pRLineSet);
            if (pRPoint == NULL) continue;

            float rx = pRPoint->getX();
            float ry = pRPoint->getY();

            float mx = 0.5f * (sx + rx);
            float my = 0.5f * (sy + ry);

            int ix = (int)floor((mx - mCMPMinX) / mCMPBinX);
            int iy = (int)floor((my - mCMPMinY) / mCMPBinY);

            if (ix < 0 || ix >= mCMPXCount) continue;
            if (iy < 0 || iy >= mCMPYCount) continue;

            mpCMPMask[iy][ix] = 1;
            mpCMPFold[iy][ix] += 1;
            ++mCMPPairCount;
        }
    }

    return true;
}

bool SPSMaskBuilder::exportCMPMaskTxt(const QString &fileName) const
{
    if (mpCMPMask == NULL) return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# CMPMask rows=yBin cols=xBin\n";
    out << "# xCount=" << mCMPXCount
        << " yCount=" << mCMPYCount
        << " minX=" << mCMPMinX
        << " maxX=" << mCMPMaxX
        << " minY=" << mCMPMinY
        << " maxY=" << mCMPMaxY
        << " binX=" << mCMPBinX
        << " binY=" << mCMPBinY << "\n";

    for (int iy = 0; iy < mCMPYCount; ++iy)
    {
        for (int ix = 0; ix < mCMPXCount; ++ix)
        {
            out << mpCMPMask[iy][ix];
            if (ix != mCMPXCount - 1) out << "\t";
        }
        out << "\n";
    }

    file.close();
    return true;
}

bool SPSMaskBuilder::exportCMPFoldTxt(const QString &fileName) const
{
    if (mpCMPFold == NULL) return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# CMPFold rows=yBin cols=xBin\n";
    out << "# xCount=" << mCMPXCount
        << " yCount=" << mCMPYCount
        << " pairCount=" << mCMPPairCount << "\n";

    for (int iy = 0; iy < mCMPYCount; ++iy)
    {
        for (int ix = 0; ix < mCMPXCount; ++ix)
        {
            out << mpCMPFold[iy][ix];
            if (ix != mCMPXCount - 1) out << "\t";
        }
        out << "\n";
    }

    file.close();
    return true;
}

bool SPSMaskBuilder::exportCMPMetaTxt(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "CMP_XCount=" << mCMPXCount << "\n";
    out << "CMP_YCount=" << mCMPYCount << "\n";
    out << "CMP_MinX=" << mCMPMinX << "\n";
    out << "CMP_MaxX=" << mCMPMaxX << "\n";
    out << "CMP_MinY=" << mCMPMinY << "\n";
    out << "CMP_MaxY=" << mCMPMaxY << "\n";
    out << "CMP_BinX=" << mCMPBinX << "\n";
    out << "CMP_BinY=" << mCMPBinY << "\n";
    out << "CMP_PairCount=" << mCMPPairCount << "\n";

    file.close();
    return true;
}