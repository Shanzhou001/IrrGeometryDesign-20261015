#include "gAttrAnalyzeParam.h"

GAttrAnalyzeParam::GAttrAnalyzeParam()
{
	reset();
}

void GAttrAnalyzeParam::reset()
{
	mAllAzimuth=true;
	mEqualAzimuth=false;
	mEqualNumAzimuth=1;
	mAllOffset=true;mEqualOffset=false;
	mEqualNumOffset=1;
	mAreaOffset=false;
	mBeginAreaOffset=100;
	mEndAreaOffset=200;

	mEqualInOffset=true;
	mAreaInOffset=false;
	mEqualNumInOffset=1;
	mBeginAreaInOffset=100;
	mEndAreaInOffset=200;
	mEqualCLOffset=true;
	mAreaCLOffset=false;
	mEqualNumCLOffset=1;
	mBeginAreaCLOffset=150;
	mEndAreaCLOffset=250;
	mInMaxOffset=0;
	mCLMaxOffset=0;

	mRealBeginAreaInOffset=100;
	mRealEndAreaInOffset=200;
	mRealBeginAreaCLOffset=150;
	mRealEndAreaCLOffset=250;
}



bool GAttrAnalyzeParam::save(QDataStream &stream)
{
	stream << mAllAzimuth;
	stream << mEqualAzimuth;
	stream << mEqualNumAzimuth;
	stream << mAllOffset;
	stream << mEqualOffset;
	stream << mEqualNumOffset;
	stream << mAreaOffset;
	stream << mBeginAreaOffset;
	stream << mEndAreaOffset;

	stream << mEqualInOffset;
	stream << mAreaInOffset;
	stream << mEqualNumInOffset;
	stream << mBeginAreaInOffset;
	stream << mEndAreaInOffset;
	stream << mEqualCLOffset;
	stream << mAreaCLOffset;
	stream << mEqualNumCLOffset;
	stream << mBeginAreaCLOffset;
	stream << mEndAreaCLOffset;
	stream << mInMaxOffset;
	stream << mCLMaxOffset;

	stream << mRealBeginAreaInOffset;
	stream << mRealEndAreaInOffset;
	stream << mRealBeginAreaCLOffset;
	stream << mRealEndAreaCLOffset;
	return true;
}

bool GAttrAnalyzeParam::load(QDataStream &stream)
{
	stream >> mAllAzimuth;
	stream >> mEqualAzimuth;
	stream >> mEqualNumAzimuth;
	stream >> mAllOffset;
	stream >> mEqualOffset;
	stream >> mEqualNumOffset;
	stream >> mAreaOffset;
	stream >> mBeginAreaOffset;
	stream >> mEndAreaOffset;

	stream >> mEqualInOffset;
	stream >> mAreaInOffset;
	stream >> mEqualNumInOffset;
	stream >> mBeginAreaInOffset;
	stream >> mEndAreaInOffset;
	stream >> mEqualCLOffset;
	stream >> mAreaCLOffset;
	stream >> mEqualNumCLOffset;
	stream >> mBeginAreaCLOffset;
	stream >> mEndAreaCLOffset;
	stream >> mInMaxOffset;
	stream >> mCLMaxOffset;

	stream >> mRealBeginAreaInOffset;
	stream >> mRealEndAreaInOffset;
	stream >> mRealBeginAreaCLOffset;
	stream >> mRealEndAreaCLOffset;
	return true;
}
