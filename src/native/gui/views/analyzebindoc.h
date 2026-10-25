#ifndef ANALYZE_BIN_DOC_H
#define ANALYZE_BIN_DOC_H

#include <qobject.h>
#include <QPoint.h>

//------------------------------------------------------------------------------
class analyzebindoc : public QObject
{
	Q_OBJECT
public:
	analyzebindoc();
	~analyzebindoc();
	void setData(int **data){mpData = data;}
	void setData2(int **data2){mpData2=data2;}
	void setData3(bool data3){mpData3=data3;}
	void setData4(int *data4){mpData4=data4;}
	void setDatatracespac(int tracespac){mpDatatracesapc=tracespac;}
	void setDatapoint(QPoint point){mpDatapoint=point;}
	void setDatamaxoffset(int maxoffset) {mpDatamaxoffset=maxoffset;}
	void setDataXbinnumber(int xbinnumber) {mpDataXbinnumber=xbinnumber;}
	void setDataYbinnumber(int ybinnumber) {mpDataYbinnumber=ybinnumber;}
	void setDataOffsetVSTraces(int *OffsetVSTraces) {mpDataOffsetVSTraces=OffsetVSTraces;}
	void setDataAzimuthVSTraces(int *AzimuthVSTraces) {mpDataAzimuthVSTraces=AzimuthVSTraces;}
	void setDataBinfold(int **binfold) {mpDataBinfold=binfold;}
	void setDataMeiguiVSTraces(int **MeiguiVSTraces) {mpDataMeiguiVSTraces=MeiguiVSTraces;}
	void setDataOffsetVSFoldBins(int **OffsetVSFoldBins) {mpDataOffsetVSFoldBins=OffsetVSFoldBins;}
	void setDataFoldVSBins(int *FoldVSBins){mpDataFoldVSBins =FoldVSBins;}
	void setDatamaxfold(int maxfold) {mpDatamaxfold=maxfold;}
	void setDataValueworkarea(bool valueworkarea) {mpDataValueworkarea=valueworkarea;}

	void setDatalineFoldVSBins(int *lineFoldVSBins){mpDatalineFoldVSBins =lineFoldVSBins;}
	void setDatalineOffsetVSFoldBins(int **lineOffsetVSFoldBins) {mpDatalineOffsetVSFoldBins=lineOffsetVSFoldBins;}
	void setDataValueline(bool valueline) {mpDataValueline=valueline;}
	void setDatalineOffsetVSTraces(int *lineOffsetVSTraces) {mpDatalineOffsetVSTraces=lineOffsetVSTraces;}
	void setDatalineAzimuthVSTraces(int *lineAzimuthVSTraces) {mpDatalineAzimuthVSTraces=lineAzimuthVSTraces;}
	void setDatalineMeiguiVSTraces(int **lineMeiguiVSTraces) {mpDatalineMeiguiVSTraces=lineMeiguiVSTraces;}
	void setDatalinecoordinate(QPoint linecoordinate)	{mpDatalinecoordinate=linecoordinate;}

	int **getData() {return mpData;}
	int **getData2() {return mpData2;}
	bool getData3()	{return mpData3;}
	int *getData4() {return mpData4;}
	int getDatatracesapc() {return mpDatatracesapc;}
	QPoint getDatapoint() {return mpDatapoint;}
	int getDatamaxoffset() {return mpDatamaxoffset;}
	int getDataXbinnumber() {return mpDataXbinnumber;}
	int getDataYbinnumber() {return mpDataYbinnumber;}
	int *getDataOffsetVSTraces() {return mpDataOffsetVSTraces;}
	int *getDataAzimuthVSTraces() {return mpDataAzimuthVSTraces;}
	int **getDataBinfold() {return mpDataBinfold;}
	int **getDataMeiguiVSTraces() {return mpDataMeiguiVSTraces;}
	int **getDataOffsetVSFoldBins() {return mpDataOffsetVSFoldBins;}
	int *getDataFoldVSBins() {return mpDataFoldVSBins;}
	int getDatamaxfold() {return mpDatamaxfold;}
	bool getDataValueworkarea() {return mpDataValueworkarea;}

	int *getDatalineFoldVSBins() {return mpDatalineFoldVSBins;}
	int **getDatalineOffsetVSFoldBins() {return mpDatalineOffsetVSFoldBins;} 
	bool getDataValueline() {return mpDataValueline;}
	int *getDatalineOffsetVSTraces() {return mpDatalineOffsetVSTraces;}
	int *getDatalineAzimuthVSTraces() {return mpDatalineAzimuthVSTraces;}
	int **getDatalineMeiguiVSTraces() {return mpDatalineMeiguiVSTraces;}
	QPoint getDatalinecoordinate() {return mpDatalinecoordinate;}
public:
	int point;
	int *mpData4;
	bool mpData3;
private:
	int **mpData;
	int **mpData2;

	int mpDatatracesapc;
	QPoint mpDatapoint;
	int *mpDataFoldVSBins;
	int mpDatamaxfold;
	bool mpDataValueworkarea;
	int mpDatamaxoffset;
	int mpDataXbinnumber;
	int mpDataYbinnumber;
	int *mpDataOffsetVSTraces;
	int *mpDataAzimuthVSTraces;
	int **mpDataBinfold;//
	int **mpDataMeiguiVSTraces;
	int **mpDataOffsetVSFoldBins;

	bool mpDataValueline;
	QPoint mpDatalinecoordinate;
	int *mpDatalineFoldVSBins;
	int **mpDatalineOffsetVSFoldBins;
	int *mpDatalineOffsetVSTraces;
	int *mpDatalineAzimuthVSTraces;
	int **mpDatalineMeiguiVSTraces;

public:
	//	bool valuelinefoldVSbins;

	bool valuefoldVSbins;
	bool valueoffsetVStraces;//按钮标志
	bool valueazimuthVStraces;
	bool valueroseVStraces;
	bool valueoffsetVSfoldbins;
	bool valuebinfold;//面元覆盖次数
	bool valueOffsetFactor;//炮检距非均匀性系数
public:
	void foldVSbins();
	void offsetVStraces();
	void azimuthVStraces();
	void roseVStraces();
	void offsetVSfoldbins();
public:
	bool showNextpoint();
	bool showProvouspoint();
};

#endif