#ifndef GSPSVIEW_H
#define GSPSVIEW_H

#include <q3scrollview.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <q3ptrlist.h>
#include <qstatusbar.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include "gpoint3d.h"
#include "grlineset.h"
#include "gshotset.h"
#include "gproject.h"
#include "gcellgrid.h"
//#include "gcmpview.h"

#define SPOINT_PEN_COLOR Qt::black
#define SPOINT_BRUSH_COLOR QColor(255, 255, 128)

#define RPOINT_PEN_COLOR Qt::blue
#define RPOINT_BRUSH_COLOR Qt::white

#define CELL_PEN QPen(Qt::gray)
#define CELL_BRUSH QBrush(Qt::NoBrush)

#define RAYPOINT_PEN_COLOR Qt::black
#define RAYPOINT_BRUSH_COLOR QColor(155, 155, 28)

#define RAYRLINE_PEN_COLOR Qt::black
#define RAYRLINE_BRUSH_COLOR QColor(174, 26, 93)

// 观测系统二维视图类
class GSPSView : public Q3ScrollView
{
	Q_OBJECT
public:
	//----图形编辑操作类型定义----
	enum GOperation {gNone, gZoom, gEditSPoint};

	bool mIsImpotSps;//add by zhaohu 是否显示输入的观测系统

protected:
	GProject *mpProject;							// 工程指针
	GSPSParam *mpSPSParam;
	GRLineSet *mpRLineSet;							// 检波线集合
	GSLineSet *mpSLineSet;							// 炮线集合
	GShotSet *mpShotSet;							// 炮集合
	GCellGrid *mpCellGrid;							// 面元网格
	GOperation mOperation;							// 编辑操作类型
	GOperation mPreOperation;						// 上次操作类型
	int mPressCount;								// 鼠标按下次数
	Qt::ButtonState mPressBtn;						// 鼠标已按下按键
	int mMouseX, mMouseY;							// 鼠标所在位置
	int mCartX, mCartY;								// 定标器坐标
	int mScrollX, mScrollY;							// 滚动坐标

	double mScale;
	double mZoomScaleX, mZoomScaleY;				// 缩放比例
	double mPreZoomScaleX, mPreZoomScaleY;			// 上一次缩放比例
	bool mIsScaling;								// 标记是否是缩放的滑动过程(此时不要重绘)
	int mZoomMouseStep;								// 鼠标缩放步长
	double mZoomStep;								// 鼠标缩放逻辑步长
	double mMaxZoomScale, mMinZoomScale;			// 最大, 最小缩放比例
	int mHUnit, mVUnit;								// 标注间距(与比例尺有关)
	int mHRulerHeight, mVRulerWidth;				// 尺高
	
	int mSelDiff;									// 鼠标选取偏差(像素)
	GSPoint *mpSelSPoint;							// 当前选择炮点指针

	QLabel *mXLabel, *mYLabel;						// 状态条坐标控件
	QLabel *mOffsetLabel;							// 当前偏离原设计位置距离
	QStatusBar *mStatusBar;							// 状态条

	//赵虎添加
//	Gcmpview *mpCMPView;
	//赵虎添加
	
signals:
	void zoomScaleChanged(double zoomScaleX, double zoomScaleY,
						  double preZoomScaleX, double preZoomScaleY);
protected:
	int getContentsWidth();				// 计算剖面图的屏幕宽度(像素)
	int getContentsHeight();			// 计算剖面图的屏幕高度(像素)
	double getMinX();					// 得到逻辑最小X坐标
	double getMinY();					// 得到逻辑最小Y坐标
	double getMaxX();					// 得到逻辑最大X坐标
	double getMaxY();					// 得到逻辑最大Y坐标
	int getHUnit();						// 得到X(逻辑)
	int getVUnit();						// 得到Y标注间隔(逻辑)
	int		lpXTodpX(double lx);		// 逻辑X坐标转换到屏幕X坐标
	double	dpXTolpX(int dx);			// 屏幕X坐标转换到逻辑X坐标
	int		lpYTodpY(double ly);		// 逻辑Y坐标转换到屏幕Y坐标
	double	dpYTolpY(int dy);			// 屏幕Y坐标转换到逻辑Y坐标
	
	QFont getTagFont()					// 得到标注字体
	{ return QFont("Arial", 8); }

	void drawCoordinate(QPainter *p);	// 绘制坐标轴 
	void drawCrossNet(QPainter *p, int cx, int cy, int cw, int ch);	// 绘制网格
	void drawHRuler(QPainter *p, int sx, int cx, int cy, int cw);	// 绘制水平卷尺
	void drawVRuler(QPainter *p, int sy, int cx, int cy, int ch);	// 绘制垂直卷
	void drawHCart(QPainter *p, int x);		// 绘制水平定标器
	void drawVCart(QPainter *p, int x);		// 绘制垂直定标器

	// 绘制炮点
	void drawSPoint(QPainter *p, int dx, int dy, const QColor &pColor = SPOINT_PEN_COLOR,
					const QColor &bColor = SPOINT_BRUSH_COLOR);
	// 绘制当前选中炮点
	void drawCurrentSPoint(QPainter *p);

	//画炮点原点add by zhaohu
	void drawBeginPoint(QPainter *p,double x,double y);
	// 绘制全部炮点
	void drawSPoints(QPainter *p, int cx, int cy, int cw, int ch);

	// 绘制检波点
	void drawRPoint(QPainter *p, int dx, int dy, const QColor &pColor = RPOINT_PEN_COLOR, 
					const QColor &bColor = RPOINT_BRUSH_COLOR);
	// 绘制全部检波点
	void drawRPoints(QPainter *p, int cx, int cy, int cw, int ch);

	// 绘制面元
	void drawCell(QPainter *p, int i, int j, int cx, int cy, int cw, int ch,
				  const QPen &pen = CELL_PEN, const QBrush &brush = CELL_BRUSH);
	// 绘制全部面元
	void drawCells(QPainter *p, int cx, int cy, int cw, int ch);

	// 绘制视图内容
	virtual void drawContents(QPainter* p, int cx, int cy, int cw, int ch);
	// 绘图事件函数
	virtual void paintEvent(QPaintEvent *e);

	// 鼠标按下事件函数
	virtual void contentsMousePressEvent(QMouseEvent *e);
	virtual void contentsMouseMoveEvent(QMouseEvent *e);
	virtual void contentsMouseReleaseEvent(QMouseEvent *e);

	GSPoint * ptInSPoint(int x, int y, int diff);
	void editSPtMousePress(QMouseEvent *e);
	void editSPtMouseMove(QMouseEvent *e);
	void editSPtMouseRelease(QMouseEvent *e);

	void zoomMousePress(QMouseEvent *e);
	void zoomMouseMove(QMouseEvent *e);
	void zoomMouseRelease(QMouseEvent *e);

	void startZoom();
	void stopZoom();

	double getScaleX();
	double getScaleY();
	double getPreScaleX();
	double getPreScaleY();
public:
	GSPSView(QWidget *parent = 0, const char *name = "SectionEditView", Qt::WFlags f = 0);
	virtual ~GSPSView();

	void refresh();

	void start(GOperation operation);

	void stop();

	void setScale(double scale);
	void setZoomScaleX(double scale);
	void setZoomScaleY(double scale);

	void setXLabel(QLabel *label)
	{ mXLabel = label; }

	void setYLabel(QLabel *label)
	{ mYLabel = label; }

	void setOffsetLabel(QLabel *label)
	{ mOffsetLabel = label; }

	void setStatusBar(QStatusBar *bar)
	{ mStatusBar = bar; }

	//赵虎添加
// 	void setPointinit()
// 	{
// 		beginCRPPoint.setX(NULL);
// 		beginCRPPoint.setY(NULL);
// 		GmProject::getProject()->getDefineDpArea(beginCRPPoint,endCRPPoint,beginup,begindown,endup,enddown);
// 	}
// 	bool valueDefineCRPline;//定义中心线
// 	GPoint2d beginCRPPoint;
// 	GPoint2d endCRPPoint;
// 	GPoint2d beginup,begindown;
// 	GPoint2d endup,enddown;
// 	void rotatecoord(GPoint2d &point,double angle);
// 	void CalDefineRenge();//计算定义的区域
	// 绘制全部落在区域里射线的出射点

	// 绘制炮点
// 	void drawRaySurPoint(QPainter *p, int dx, int dy, const QColor &pColor = RAYPOINT_PEN_COLOR,
// 		const QColor &bColor =RAYPOINT_BRUSH_COLOR);

//	void drawRaySurPoints(QPainter *p, int cx, int cy, int cw, int ch);

// 	void drawRayRline(QPainter *p,int cx, int cy, int cw, int ch,const QColor &pColor = RAYRLINE_PEN_COLOR,
// 		const QColor &bColor = RAYRLINE_BRUSH_COLOR);

	// 绘制覆盖次数
//	void drawFold(QPainter *p, int i, int j ,int fold, int cx, int cy, int cw, int ch);

//	QColor datatocolor(int m);//由数字得到颜色

//	double getRaydy(double *p,double x,int m);

//	vector<vector<GVector3d > > RayRPoint;

//	vector<vector<GVector3d > > RayRPointRotate;

//	void setCMPView(Gcmpview *view) {mpCMPView=view;}

};

#endif
