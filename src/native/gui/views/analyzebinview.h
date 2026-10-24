#ifndef ANALYZE_BIN_VIEW_H
#define ANALYZE_BIN_VIEW_H

#include <q3scrollview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpoint.h> 
//Added by qt3to4:
#include <QMouseEvent>

#include "analyzebindoc.h"

//  [Liu Wei 3/28/2023]
#include "../../io/sps/gproject.h"
#include "../survey/gcellgrid.h"
#include "../../core/gpoint3d.h"
#include <Q3PointArray>

#define CELL_PEN QPen(Qt::gray)
#define CELL_BRUSH QBrush(Qt::NoBrush)

//实际的绘图函数
//------------------------------------------------------------------------------
class analyzebinview : public Q3ScrollView
{
protected:
	analyzebindoc *mpBinDoc;//定义文档指针

	QPixmap *mpBinImage;					//图形缓存
	QLabel	*mCoordinate;					//坐标标签

	int mAmizthNum,mOffsetNum;//方位角和炮检距分的段
	int mShowAzimuth,mShowOffset;
	int mBaseDivAmizth,mBaseDivOffset;
	QString mTextAmizth,mTextOffset;

	int mRoselx,mRosetly;
public:
	analyzebinview(QWidget* parent);
	virtual ~analyzebinview();
	virtual void refresh();
	void drawContents(QPainter* p, int cx, int cy, int cw, int ch);
	void setDoc(analyzebindoc *doc){mpBinDoc = doc;}
	void setAODividNum(int AmizthNum,int OffsetNum,int ShowAzimuth,int ShowOffset,int BaseDivAmizth,int BaseDivOffset,QString TextAmizth,QString TextOffset)//方位角和炮检距分的段
	{
		mAmizthNum=AmizthNum;mOffsetNum=OffsetNum;
		mShowAzimuth=ShowAzimuth;mShowOffset=ShowOffset;
		mBaseDivAmizth=BaseDivAmizth;mBaseDivOffset=BaseDivOffset;
		mTextAmizth=TextAmizth;mTextOffset=TextOffset;
	}
public:
	void setlabel(QLabel *label){mCoordinate=label;};
protected:
	void drawBinpoint(QPainter *p,int ii);//画点面元属性分析图
public:
	void drawworkareaFoldVSBIns(int x,int y);//整个工区面元属性分析,覆盖次数-面元数量
	void drawworkareaOffsetVSFoldBins(int y,int lx,int maxzhufold);//整个工区炮检距-覆盖次数-面元数量
	void drawworkareaRoseVSTraces(int lx,int ly,int maxoffset1);//整个工区面元属性分析,玫瑰花图-炮检对
	void drawworkareaAzimuthVSTraces(int y);//整个工区面元属性分析,方位角-炮检对
	void drawworkareaOffsetVSTraces(int x,int y);//整个工区面元属性分析,炮检距-炮检对
	void drawworkareaBinfold(int **array,int xline,int yline);//整个工区覆盖次数
	void drawoffsetNonuniformfactor(float **array,int x,int y,float max,float min,float NonuniformBinx,float NonuniformBiny,QString FnameStr,float CSQFactor);//画炮检距分析结果

	void drawWorkAreaBinFold(int **array,int xbinnum,int ybinnum);// 整个工区覆盖次数 [Liu Wei 3/28/2023]
	void drawOffsetNonuniformFactor(float **array,int x,int y,float max,float min,float NonuniformBinx,float NonuniformBiny,QString FnameStr,float CSQFactor);// 画炮检距分析结果 [Liu Wei 3/29/2023]

protected:
	void drawlineFoldVSBIns(QPainter *p,int x,int y);//线面元属性分析,覆盖次数-面元数量
	void drawlineOffsetVSTraces(QPainter *p,int x,int y);//线面元属性分析,炮检距-炮检对
	void drawlineRoseVSTraces(QPainter *p);//线面元属性分析,玫瑰花图-炮检对
	void drawlineAzimuthVSTraces(QPainter *p,int y);//线面元属性分析,方位角-炮检对
	void drawlineOffsetVSFoldBins(QPainter *p,int y);//线炮检距-覆盖次数-面元数量
	//	layoutdoc *mpDoc1;//定义主文档指针
protected:
	float xscaleoffset;
	float yscaleoffset;
	int viewx;
	int viewy;
	int **binpointoffset;
	int **binpointazimuth;
	int *binpointfold;
	int tracespac;
	QPoint binmousepoint;
	QPoint binnumber;//
	QPoint binfold;
	QPoint traces;//寻找一个合适的刻度
	QPoint azimuthtraces;//方位角
	QPoint offset;
	QPoint roseoffset;
	QPoint fold;
	int maxoffset;
public:
	int xbinnumber;
	int ybinnumber;
	int **Binfold;
protected:
	int *offsetVStraces;
	int *azimuthVStraces;

	int **roseVStraces;
	int **offsetVSfoldbins;
	int **offsetVSfoldbins2;//如果覆盖次数大于30，则画柱状图时只分成3等份
	int maxfold;
	int *foldVSbins;

	int *linefoldVSbins;
	int *lineoffsetVStraces;
	int *lineazimuthVStraces;
	int **lineoffsetVSfoldbins;
	int **lineoffsetVSfoldbins2;
	int **lineroseVStraces;
	QPoint linecoordinate;

	int xcoordlinefold;//y方向最表最大值
	int xcoordlineoffset;
	int ycoordlinefold;//y方向最表最大值
	int ycoordlineAzimuth;
	int ycoordlineoffset;
	int ycoordlineOffsetVSFoldBins;
public:
	int xcoordworkfold;//x方向最表最大值
	int xcoordworkoffset;
	int ycoordworkfold;//y方向最表最大值
	int ycoordworkAzimuth;
	int ycoordworkoffset;
	int ycoordworkOffsetVSFoldBins;
protected:
	bool valuelineoffseVSfoldbins;
	bool valuelineroseVStraces;
	//	bool valuefoldVSbins;
protected:
	QPoint kedufun(int m);//求某个数适合的刻度
	QColor datatocolor(int m);//数字得到颜色
	void contentsMousePressEvent(QMouseEvent *);
	void contentsMouseMoveEvent( QMouseEvent* );
	void contentsMouseReleaseEvent( QMouseEvent* );
	void contentsMouseDoubleClickEvent( QMouseEvent* );

private://  [Liu Wei 3/28/2023]
	GProject *mpProject;							// 工程指针
	GCellGrid *mpCellGrid;							// 面元网格
	int blkCoordinate;								// 默认留白像素宽度
	double xscale,yscale;							// 绘图比例尺
	float mCoordinateXmin,mCoordinateYmin;			// 实际绘图坐标起点
	QPoint calibrationFunction(double m);			// 求某个数适合的刻度
	double getScaleX() {return xscale;}
	double getScaleY() {return yscale;}
	int		lpXTodpX(double lx);		// 逻辑X坐标转换到屏幕X坐标	
	int		lpYTodpY(double ly);		// 逻辑Y坐标转换到屏幕Y坐标
	double	dpXTolpX(int dx);			// 屏幕X坐标转换到逻辑X坐标
	double	dpYTolpY(int dy);			// 屏幕Y坐标转换到逻辑Y坐标

	// 绘制面元
	void drawCellAttribute(QPainter *p, int i, int j, int cx, int cy, int cw, int ch, QPen &pen, const QBrush &brush);

};

#endif