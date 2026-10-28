#ifndef ANALYZEBINFORM_H
#define ANALYZEBINFORM_H

#include <QMainWindow>
#include "ui_analyzebinform.h"
#include "../views/analyzebindoc.h"
#include "../views/analyzebinview.h"
#include "../../core/gvaluevector.h"
// 
// [11/3/2022 Liu Wei]OVT
class analyzebinform : public QMainWindow
{
	Q_OBJECT

public:
	analyzebinform(QWidget *parent = 0);
	~analyzebinform();

private:
	Ui::analyzebinformClass ui;

protected:
	virtual void closeEvent(QCloseEvent *e);

public slots:

	void on_actionChangeShowarea_triggered();
	void on_actionQFDivide_triggered();
	void on_actionRoseDivide_triggered();
	//void on_actionQFDivide_triggered();//
	//void on_actionRoseDivide_triggered();//

	void on_actionGeometryImformation_triggered();//
	void on_actionCSQFactor_triggered();//
	void on_actionFootFactor_triggered();//
	void on_actionAzimuthFactor_triggered();//
	void on_actionOffsetFactor_triggered();//
	void on_Show_previous_point_triggered();//
	void on_Show_next_point_triggered();//
	void on_Bin_Fold_triggered();//
	void on_Offset_Fold_Bins_triggered();//
	void on_Rose_Traces_triggered();//
	void on_Azimuth_Traces_triggered();//-
	void on_Offset_Traces_triggered();//-
	void on_Fold_Bins_triggered();//-
private:
	analyzebindoc *mpBinDoc;
	analyzebinview *mpBinView;
private:
	bool in_loop ;
	int CMPxbinnumber,CMPybinnumber,CMPmaxfold;
	GValueVector<short> **offsetarray;	
	GValueVector<short> **azimutharray;
	int CMPdlgmaxoffset;
	bool CMPvalueanalyzebinWorkArea;//
	int *CMPFoldVSBins;//-
	int *CMPOffsetVSTraces;//-
	int **CMPOffsetVSFoldBins;//--
	int *CMPAzimuthVSTraces;//-
	int **CMPmeiguiVStraces;//-
	int **CMPBinfold;//

	float **m_dataNonuniform;//
	float **m_dataAngle;//
	float **m_dataFoot;//
	float **m_dataCSDFPercent;//*100
	float m_maxNonuniformfactor;//
	float m_minNonuniformfactor;
	float m_maxAnglefactor;//
	float m_minAnglefactor;//
	float m_maxFootfactor;//
	float m_minFootfactor;//
	float m_maxCSDFPfactor;//
	float m_minCSDFPfactor;
	float m_CSQFfactor;//
	float mBinx,mBiny;//
	float mCircleR;//
	int mAmizthNum,mOffsetNum;//
	int mShowAzimuth,mShowOffset;
	int mBaseDivAmizth,mBaseDivOffset;
	QString mTextAmizth,mTextOffset;
	int lxRose;//
	int lyRose;//
	int lxzhu;//
	int maxzhufold;

	int lxCSQF;//
	int lyCSQF;//
	int maxoffsetCSQF;//
	bool bShowOffset;//truefalse
	bool bShowangle;
	bool bShowFoot;

	void setData3(bool i);
	void setData(int **data);
	void setData2(int **data2);
	void setData4(int *i);
	void setDatatracespac(int i);
	void setDatapoint(QPoint point);
	void setDataMeiguiVSTraces(int **MeiguiVSTraces);//-
	void setDataBinfold(int **Binfold);//
	void setDataOffsetVSFoldBins(int **OffsetVSFoldBins);//--
	void setDatamaxoffset(int maxoffset);//
	void setDataXbinnumber(int xbinnumber);//
	void setDataYbinnumber(int ybinnumber);//
	void setDataOffsetVSTraces(int *OffsetVSTraces);// -
	void setDataAzimuthVSTraces(int *AzimuthVSTraces);// -
	void setDataFoldVSBins(int *FoldVSBins);
	void setDatamaxfold(int maxfold);//

	void setDatalineOffsetVSTraces(int *lineOffsetVSTraces);// -
	void setDatalineAzimuthVSTraces(int *lineAzimuthVSTraces);// -
	void setDatalineFoldVSBins(int *lineFoldVSBins);//
	void setDatalineOffsetVSFoldBins(int **lineOffsetVSFoldBins);//--
	void setDatalineMeiguiVSTraces(int **lineMeiguiVSTraces);//-
	void setDatalinecoordinate(QPoint linecoordinate);
	void setDataValueline(int valueline);//
	void computeNonuniformfactorfun(GValueVector<short> **array,int xl,int yl,int zl,int foldup,int folddown);//
	void computeAnglefactorfun(GValueVector<short> **array,int xl,int yl,int zl,int foldup,int folddown);//
	void Footprintfun(GValueVector<short> **offset,int xline,int yline,int zline,int foldup,int folddown);//
	void CSDFactorfun(GValueVector<short> **offset,GValueVector<short> **azimuth,int xline,int yline,int zline,float maxoffst);//
public:
	int exec();
public:
	void setAnalyzedata(GValueVector<short> **offset1,GValueVector<short> **azimuth1,int xline,int yline,int zline,float bx,float by,float CircleR)
	{
		offsetarray=offset1;
		azimutharray=azimuth1;
		CMPxbinnumber=xline;
		CMPybinnumber=yline;
		CMPmaxfold=zline;
		mBinx = (bx > 0.0f) ? bx : 25.0f;
		mBiny = (by > 0.0f) ? by : 25.0f;
		mCircleR = (CircleR >= 0.0f) ? CircleR : 0.0f;
	}
	void setAODividNum(int AmizthNum,int OffsetNum,int ShowAzimuth,int ShowOffset,int BaseDivAmizth,int BaseDivOffset,QString TextAmizth,QString TextOffset)//
	{
		mAmizthNum=AmizthNum;mOffsetNum=OffsetNum;
		mShowAzimuth=ShowAzimuth;mShowOffset=ShowOffset;
		mBaseDivAmizth=BaseDivAmizth;mBaseDivOffset=BaseDivOffset;
		mTextAmizth=TextAmizth;mTextOffset=TextOffset;
	}
	void CMPLayoutAttibuteConculate(int **foldData);//CMP-

	void setDataValueworkarea(int valueworkarea);
	

};

#endif // ANALYZEBINFORM_H
