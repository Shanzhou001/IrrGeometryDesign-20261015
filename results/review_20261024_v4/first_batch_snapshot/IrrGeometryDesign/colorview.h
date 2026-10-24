#ifndef COLOR_VIEW_H
#define	COLOR_VIEW_H

#include <q3scrollview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpoint.h> 
#include <qimage.h>
//Added by qt3to4:
#include <QMouseEvent>
#include "draw2Dview.h"

class tCRPview;
//------------------------------------------------------------------------------
class tcolorview : public Q3ScrollView
{
protected:
	QLabel	*mCoordinate;					//坐标标签
	QPixmap *mpImage;//图形缓存
	Draw2DView *mpPview;
	
public:
	void setlabel(QLabel *label){mCoordinate=label;}
	void setCRPview(Draw2DView *view){mpPview=view;}
private:
	float mMaxvalueColor;
	float mMinvalueColor;
						
public:
	tcolorview(QWidget* parent);
	virtual ~tcolorview();
	virtual void refresh();
	void drawContents(QPainter* p, int cx, int cy, int cw, int ch);
	float getMaxvalue() {return mMaxvalueColor;}
	float getMinvalue() {return mMinvalueColor;}
public:
	void drawcolorcode(float max,float min);//画色标
	void cacualateColorbelt(short which);
	void drawcolorcodevary(float max,float min);//画色标(渐变的)
	float** getcolorbeltvary(){return colorbeltvary;}
	//常用函数
protected:
	float** colorbeltvary;
	short mWhich;
	QColor datatocolor(int m);
	bool alloc2D(float ** &matrix, int ln, int col);
	void clear2D(float ** &matrix, int ln);//删除内存
protected:
	void contentsMousePressEvent(QMouseEvent *);
	void contentsMouseMoveEvent( QMouseEvent* );
	void contentsMouseReleaseEvent( QMouseEvent* );
	void contentsMouseDoubleClickEvent( QMouseEvent* );
	
	// 常用函数
};


#endif