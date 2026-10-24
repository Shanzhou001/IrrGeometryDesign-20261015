#ifndef DRAW2D_VIEW_H
#define DRAW2D_VIEW_H

#include <q3scrollview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpoint.h> 
#include <QMouseEvent>

//------------------------------------------------------------------------------
class Draw2DView : public Q3ScrollView
{
protected:
	QPixmap *mpBinImage;					//图形缓存
	QLabel	*mCoordinate;					//坐标标签
public:
	Draw2DView(QWidget* parent);
	virtual ~Draw2DView();
	virtual void refresh();
	void drawContents(QPainter* p, int cx, int cy, int cw, int ch);
public:
	void setlabel(QLabel *label){mCoordinate=label;}
	void setminbx(float minbx1){bx=minbx1;}
	void setminby(float minby1){by=minby1;}
	void drawEnergy(float **array,int xnumber,int ynumber,float max,float min,QString strfname);//画能量图形
	void drawworkareaBinfold(int **array,int xline,int yline,QString mTextOffset);//整个工区覆盖次数
	void setcolorbelt(float **a) 
	{		colorbeltvary=a;	}
private:
	float bx;
	float by;
	float zoomxscale;
	float zoomyscale;
	float shotxscale;
	float shotyscale;
	int viewx;
	int viewy;

	float **colorbeltvary;//渐变的色带
protected:
	QPoint kedufun(int m);//求某个数适合的刻度
	QColor datatocolor(int m);//数字得到颜色
	void contentsMousePressEvent(QMouseEvent *);
	void contentsMouseMoveEvent( QMouseEvent* );
	void contentsMouseReleaseEvent( QMouseEvent* );
	void contentsMouseDoubleClickEvent( QMouseEvent* );

};

#endif