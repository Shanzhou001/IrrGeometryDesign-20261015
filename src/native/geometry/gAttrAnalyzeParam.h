#ifndef GATTRANALYZE_PARAM_H
#define GATTRANALYZE_PARAM_H

#include <qfile.h>
#include <qdatastream.h>

//#define GFORWARDPARAM_PAD_SIZE 512

// 创建SPS基本参数
class GAttrAnalyzeParam
{
public:
	bool mAllAzimuth,mEqualAzimuth;
	int mEqualNumAzimuth;
	bool mAllOffset,mEqualOffset;
	int mEqualNumOffset;
	bool mAreaOffset;
	int mBeginAreaOffset,mEndAreaOffset;

	bool mEqualInOffset,mAreaInOffset;
	int mEqualNumInOffset,mBeginAreaInOffset,mEndAreaInOffset;
	bool mEqualCLOffset,mAreaCLOffset;
	int mEqualNumCLOffset,mBeginAreaCLOffset,mEndAreaCLOffset;
	int mInMaxOffset,mCLMaxOffset;
	// 赵老师测试用参数变量
	int mRealBeginAreaInOffset,mRealEndAreaInOffset;
	int mRealBeginAreaCLOffset,mRealEndAreaCLOffset;

public:
	GAttrAnalyzeParam();

	void reset();

	// 保存
	bool save(QDataStream &stream);
	// 载入
	bool load(QDataStream &stream);
};

#endif