#include <qdrawutil.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3PointArray>
#include <QMouseEvent>
#include <QPaintEvent>
#include "gspsview.h"
#include "gmath.h"
#include "gslineset.h"
#include "gpoint3d.h"
#include "qpainter.h"
#include "qpainterpath.h"


GSPSView::GSPSView(QWidget *parent, const char *name, Qt::WFlags f) :
        Q3ScrollView(parent, name, f|Qt::WStaticContents|Qt::WNoAutoErase)
{
    mIsImpotSps = true;   // 先初始化：默认显示导入的SPS

    mpProject = GProject::getCurrentProject();
    mpSPSParam = mpProject->getSPSParam();
	if(mIsImpotSps==false)// add by Wushanzhou 
	{		
		mpRLineSet = mpProject->getRLineSetNew();//创建的sps
		mpSLineSet = mpProject->getSLineSetNew();
		mpShotSet = mpProject->getShotSetNew();
	}
	else
	{
		mpRLineSet = mpProject->getRLineSet();//导入的sps
		mpSLineSet = mpProject->getSLineSet();
		mpShotSet = mpProject->getShotSet();
	}//add by zhaohu
	mpCellGrid = mpProject->getCellGrid();

	mPreOperation = gNone;
	mOperation = gNone;//gNone;

	mPressCount = 0;
	mPressBtn = Qt::NoButton;
	mMouseX = mMouseY = 0;
	mSelDiff = 5;		// 鼠标选择偏差5

	mIsScaling = false;
	mScale = 1;
	mPreZoomScaleX = mZoomScaleX = 1;
	mPreZoomScaleY = mZoomScaleY = 1;
	mHUnit = mVUnit = 100;
	mZoomMouseStep = 10;
	mZoomStep = 0.01;
	mMaxZoomScale = 10;		mMinZoomScale = 0.1;
	mPressBtn = Qt::NoButton;

	mHRulerHeight = 25;	mVRulerWidth = 25;
	mCartX = 0;			mCartY = 0;
	mScrollX = 0;		mScrollY = 0;

	mpSelSPoint = NULL;

	mXLabel = NULL;		mYLabel = NULL;
	mOffsetLabel = NULL;
	mStatusBar = NULL;

	setBackgroundMode(Qt::NoBackground);
	viewport()->setBackgroundMode(Qt::NoBackground);
	setMargins(mHRulerHeight, mVRulerWidth, 0, 0);
	viewport()->setMouseTracking(true);

//	valueDefineCRPline=false;
// 	beginCRPPoint.isNull();
// 	
// 	endCRPPoint.isNull();
}

GSPSView::~GSPSView()
{

}

void GSPSView::setScale(double scale)
{
	mScale = scale;
}

void GSPSView::setZoomScaleX(double scale)
{
	mZoomScaleX = scale;
}

void GSPSView::setZoomScaleY(double scale)
{
	mZoomScaleY = scale;
}

double GSPSView::getScaleX()
{
	return mScale / mZoomScaleX;
}

double GSPSView::getPreScaleX()
{
	return mScale / mPreZoomScaleX;
}

double GSPSView::getScaleY()
{
	return mScale / mZoomScaleY;
}

double GSPSView::getPreScaleY()
{
	return mScale / mPreZoomScaleY;
}

int GSPSView::getContentsWidth()
{
	return gRound(mpProject->getLength()/getScaleX());
}

int GSPSView::getContentsHeight()
{
	return gRound(mpProject->getWidth()/getScaleY());
}
//
int	GSPSView::lpXTodpX(double lx)
{
	return gRound((lx-mpProject->getX0())/getScaleX()-1);
}

double GSPSView::dpXTolpX(int dx)
{
	return (dx+1)*getScaleX()+mpProject->getX0();	
}

int	GSPSView::lpYTodpY(double ly)
{
	return gRound((ly-mpProject->getZ0())/getScaleY()-1);
}

double GSPSView::dpYTolpY(int dy)
{
	return (dy+1)*getScaleY()+mpProject->getZ0();
}

int GSPSView::getHUnit()
{
	return mHUnit;
}

int GSPSView::getVUnit()
{
	return mVUnit;
}

void GSPSView::refresh()
{
	if(mIsImpotSps==false)// add by zhaohu 
	{		
		mpRLineSet = mpProject->getRLineSetNew();//创建的sps
		mpSLineSet = mpProject->getSLineSetNew();
		mpShotSet = mpProject->getShotSetNew();
	}
	else
	{
		mpRLineSet = mpProject->getRLineSet();//导入的sps
		mpSLineSet = mpProject->getSLineSet();
		mpShotSet = mpProject->getShotSet();
	}//add by zhaohu

	mHUnit = (50 * mScale) / mZoomScaleX;
	mHUnit = (mHUnit / 10) * 10;

	mVUnit = (50 * mScale) / mZoomScaleY;
	mVUnit = (mVUnit / 10) * 10;

	resizeContents(getContentsWidth(), getContentsHeight());
	updateScrollBars();

	//下面的代码确保视图缩放后，图形中心点仍然位于屏幕中心
	if(mPreZoomScaleX != mZoomScaleX || mPreZoomScaleY != mZoomScaleY)
	{
		int valueX = contentsX();
		int midWidth = visibleWidth()/2;
		double lValueX = (valueX+midWidth)*getPreScaleX();
		mPreZoomScaleX = mZoomScaleX;	//更新X缩放比例
		valueX = gRound(lValueX/getScaleX()-midWidth);

		int valueY = contentsY();
		int midHeight = visibleHeight()/2;
		double lValueY = (valueY+midHeight)*getPreScaleY();
		mPreZoomScaleY = mZoomScaleY;	//更新Y缩放比例
		valueY = gRound(lValueY/getScaleY()-midHeight);

		viewport()->setUpdatesEnabled(false);
		setContentsPos(valueX, valueY);
		viewport()->setUpdatesEnabled(true);
	}
	//赵虎添加
//	mpProject->setContentsWidth(getContentsWidth()+mVRulerWidth);
	//赵虎添加
	
	repaintContents();
	repaint();
}

void GSPSView::drawCoordinate(QPainter *p)
{
	p->save();

	QFont font("Arial", 10);
	font.setBold(true);
	p->setFont(font);
	//按照屏幕的坐标系来画的坐标轴（左上角为原点，向右手方向x增大，向下y增大）
	p->setPen(Qt::red);
	p->drawLine(10, 10, 40, 10);
	p->drawLine(40, 10, 35, 5);
	p->drawLine(40, 10, 35, 15);
	p->drawText(45, 15, "X");

	p->setPen(Qt::blue);
	p->drawLine(10, 10, 10, 40);
	p->drawLine(10, 40, 5, 35);
	p->drawLine(10, 40, 15, 35);
	p->drawText(7, 55, "Y");
}

void GSPSView::drawCrossNet(QPainter *p, int cx, int cy, int cw, int ch)
{
	double ly, lx;
	int dx, dy;
	
	p->save();
	
	QPen pen(Qt::lightGray);
	pen.setStyle(Qt::DotLine);
	p->setPen(pen);

	ly = dpYTolpY(cy);
	ly = gTrunc(ly / mVUnit)*mVUnit;
	dy = lpYTodpY(ly);
	while(dy <= cy+ch) 
	{
		p->drawLine( cx, dy, cx+cw-1, dy );
		ly += mVUnit;
		dy = lpYTodpY(ly);
	}

	lx = dpXTolpX(cx);
	lx = gTrunc(lx/mHUnit)*mHUnit;
	dx = lpXTodpX(lx);
	while(dx <= cx+cw) 
	{
		p->drawLine(dx, cy, dx, cy+ch-1);
		lx += mHUnit;
		dx = lpXTodpX(lx);
	}

    p->restore();
}

void GSPSView::drawHRuler(QPainter *p, int sx, int cx, int cy, int cw)
{
	int dx, txWidth, tx;
	double lx;
	QString tagText;		
	int subUnit = mHUnit / 10; 

	if(sx+cw > contentsWidth()) 
	{
		p->fillRect(contentsWidth(), 1, cw-contentsWidth()+mVRulerWidth+1, 
					mHRulerHeight, Qt::darkGray);
		cw = contentsWidth()-sx;
	}

	p->save();
	p->setClipRect(mVRulerWidth+cx, cy, cw, mHRulerHeight);
	p->setFont(getTagFont());	//重设字体

	p->translate(mVRulerWidth+cx, cy);
	p->setPen(Qt::black);
	
	p->fillRect(0, 0, cw, mHRulerHeight, paletteBackgroundColor());
	p->fillRect(2, 6, cw-4, mHRulerHeight-12, Qt::white);
	p->setPen(Qt::gray);
	p->drawLine(0, mHRulerHeight-1, cw, mHRulerHeight-1);
	p->setPen(Qt::black);

	lx = dpXTolpX(sx);
	lx = gTrunc(lx/subUnit)*subUnit;
	dx = lpXTodpX(lx)-sx;

	while(dx < cw) 
	{
		p->drawLine(dx, 0, dx, 2);
		p->drawLine(dx, mHRulerHeight-2, dx, mHRulerHeight-4);
		lx += subUnit;
		dx = lpXTodpX(lx)-sx;
	}

	lx = dpXTolpX(sx);
    lx = gTrunc(lx/mHUnit)*mHUnit;
	dx = lpXTodpX(lx)-sx;
	tx = dx;
	while(dx < cw)
	{
		p->setPen(Qt::black);
		p->drawLine(dx, 0, dx, 5);
		p->drawLine(dx, mHRulerHeight-2, dx, mHRulerHeight-7);
		tagText.sprintf("%d", gRound(lx));
		txWidth = p->fontMetrics().width(tagText);
		if(dx > txWidth/2 && cw-dx > txWidth/2)// && dx - txWidth/2 >= tx)
		{		
			if(gTrunc(lx/mHUnit) % 2 == 0) p->setPen(Qt::red);
			else p->setPen(Qt::blue);
			p->drawText(dx-txWidth/2+1, 0, txWidth, mHRulerHeight, Qt::AlignCenter, tagText);
			tx = dx+txWidth/2;
		}
		lx += mHUnit;
		dx = lpXTodpX(lx)-sx;
    }
	p->setClipping(false);
	p->restore();
}

void GSPSView::drawVRuler(QPainter *p, int sy, int cx, int cy, int ch)
{
	int dy, txWidth, ty;
	double ly;
	QString tagText;
	int subUnit = mVUnit / 10;

	if(sy+ch > contentsHeight()) 
	{
		p->fillRect(1, contentsHeight(), mVRulerWidth, 
					ch-contentsHeight()+mHRulerHeight+1, Qt::darkGray);
		ch = contentsHeight()-sy;
	}

	p->save();						//存储绘图设备状态
	
	p->setFont(getTagFont());		//重设字体

	p->translate(cx, mHRulerHeight+cy);
	p->setPen(Qt::black);
	p->fillRect(0, 0, mVRulerWidth, ch, paletteBackgroundColor());
	p->fillRect(6, 2, mVRulerWidth-12, ch-4, Qt::white);
	p->setPen(Qt::gray);
	p->drawLine(mVRulerWidth-1, -1, mVRulerWidth-1, ch-1);
	p->setPen(Qt::black);

	p->setClipRect(cx, mHRulerHeight+cy, mVRulerWidth, ch);
	//变换绘图坐标系
	p->rotate(90);
	p->translate(0, -mVRulerWidth+1);
	
	//定位绘图起始坐标
	ly = dpYTolpY(sy);
	ly = gRound(ly/subUnit)*subUnit;
	dy = lpYTodpY(ly) - sy;
	
	while(dy < ch) 
	{
		p->drawLine(dy, 1, dy, 3);
		p->drawLine(dy, mVRulerWidth-1, dy, mVRulerWidth-3);
		ly += subUnit;
		dy = lpYTodpY(ly) - sy;
	}

	ly = dpYTolpY(sy);
	ly = gRound(ly/mVUnit)*mVUnit;
	dy = lpYTodpY(ly) - sy;
	ty = dy;
	while(dy < ch)
	{
		p->setPen(Qt::black);
		p->drawLine(dy, 1, dy, 5);
		p->drawLine(dy, mVRulerWidth-1, dy, mVRulerWidth-5);
		tagText.sprintf( "%d", gRound(ly) );
		txWidth = p->fontMetrics().width(tagText);
		if(dy > txWidth/2 && ch-dy > txWidth/2)
		{			
			if(gTrunc(ly/mVUnit) % 2 == 0) p->setPen(Qt::red);
			else p->setPen(Qt::blue);
			p->drawText(dy-txWidth/2+1, -1, txWidth, mVRulerWidth, Qt::AlignCenter, tagText);
			ty = dy + txWidth/2;
		}
		ly += mVUnit;
		dy = lpYTodpY(ly) - sy;
    }
	p->setClipping(false);
	p->restore();
}

void GSPSView::drawHCart(QPainter *p, int x)
{
	int fw = frameWidth();
	p->save();
	p->setPen(Qt::red);
	p->setCompositionMode(QPainter::CompositionMode_Xor);
	p->drawLine(x, fw+1, x, mHRulerHeight+fw-2 );
	p->setCompositionMode( QPainter::CompositionMode_SourceOver);
	p->restore();
}

void GSPSView::drawVCart(QPainter *p, int y)
{
	int fw = frameWidth();
	p->save();
	p->setPen(Qt::red);
	p->setCompositionMode(QPainter::CompositionMode_Xor);
	p->drawLine(fw+1, y, mVRulerWidth+fw-2, y);
	p->setCompositionMode(QPainter::CompositionMode_SourceOver);
	p->restore();
}

void GSPSView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{

	if(contentsWidth() < visibleWidth() || contentsHeight() < visibleHeight())
	{
		p->fillRect(cx, cy, cw, ch, Qt::darkGray);
	}
	if(cx > contentsWidth() || cy > contentsHeight()) return ;

	if(cx+cw > contentsWidth()) cw = contentsWidth()-cx;
	if(cy+ch > contentsHeight()) ch = contentsHeight()-cy;

	p->fillRect(cx, cy, cw, ch, Qt::white);

	drawCrossNet(p, cx, cy, cw, ch);
	drawCells(p, cx, cy, cw, ch);
	//赵虎添加
//	drawRaySurPoints(p,cx,cy,cw,ch);
	//赵虎添加	
	drawRPoints(p, cx, cy, cw, ch);
	drawSPoints(p, cx, cy, cw, ch);
	//赵虎添加
//	drawRayRline(p,cx,cy,cw,ch);
	//赵虎添加	

	drawCoordinate(p);

	QPainter pb(this);	
	if(contentsX() != mScrollX) 
	{
		drawHRuler(&pb, contentsX(), frameWidth(), frameWidth(), visibleWidth());
		drawHCart(&pb, mCartX);
		mScrollX = contentsX();
	}
	if(contentsY() != mScrollY)
	{
		drawVRuler(&pb, contentsY(), frameWidth(), frameWidth(), visibleHeight());
		drawVCart(&pb, mCartY );
		mScrollY = contentsY();
	}
	pb.end();

	//赵虎添加
	//画首尾炮点连线
	/*
	p->save();
	p->setPen(Qt::blue);
	p->setPen(Qt::DotLine);

		double sumX=0,sumY=0;
		GShotSet *pShotSet=GmProject::getProject()->getShotSet();
		if(pShotSet->count()>0)
		{
			for(int i=0;i<pShotSet->count();i++)
			{
				GShot *pShot = pShotSet->get(i);
				if(pShot != 0)
				{						
					sumX=sumX+(pShot->getSPoint()->getSPSX()-pShotSet->get(0)->getSPoint()->getSPSX());
					sumY=sumY+(pShot->getSPoint()->getSPSY()-pShotSet->get(0)->getSPoint()->getSPSY());
				}
			}
			double k=sumY/sumX;
			double endx=pShotSet->get(pShotSet->count()-1)->getSPoint()->getSPSX();
			double endy=k*(endx-pShotSet->get(0)->getSPoint()->getSPSX())+pShotSet->get(0)->getSPoint()->getSPSY();
			p->drawLine(lpXTodpX(pShotSet->get(0)->getSPoint()->getSPSX()),lpYTodpY(pShotSet->get(0)->getSPoint()->getSPSY()),lpXTodpX(endx),lpYTodpY(endy));
		}

	p->restore();
	*/
	//画首尾炮点连线
// 	GPoint2d a1,b1,a2,b2,a3,b3;
// 	GmProject::getProject()->getDefineDpArea(a1,b1,a2,b2,a3,b3);
// //	if(!a1.isNull()&&!b1.isNull()&&a1.x()!=b1.x())
// 	if(a1.x()!=0&&a1.y()!=0&&b1.x()!=0&&b1.y()!=0&&a1.x()!=b1.x())
// 	{
//          GmProject::getProject()->getDefineDpArea(beginCRPPoint,endCRPPoint,beginup,begindown,endup,enddown);//保存定义的区域坐标
// 	}
	
// 	if(beginCRPPoint.x()!=0&&beginCRPPoint.y()!=0&&endCRPPoint.x()!=0&&endCRPPoint.y()!=0)
// 	{
// 		p->save();
// 		p->setPen(Qt::blue);
// 		p->drawLine(lpXTodpX(beginCRPPoint.x()),lpYTodpY(beginCRPPoint.y()),lpXTodpX(endCRPPoint.x()),lpYTodpY(endCRPPoint.y()));
// 			p->setPen(Qt::green);
// 			p->drawLine(lpXTodpX(beginup.x()),lpYTodpY(beginup.y()),lpXTodpX(begindown.x()),lpYTodpY(begindown.y()));
// 			p->drawLine(lpXTodpX(beginup.x()),lpYTodpY(beginup.y()),lpXTodpX(endup.x()),lpYTodpY(endup.y()));
// 			p->drawLine(lpXTodpX(endup.x()),lpYTodpY(endup.y()),lpXTodpX(enddown.x()),lpYTodpY(enddown.y()));
// 			p->drawLine(lpXTodpX(enddown.x()),lpYTodpY(enddown.y()),lpXTodpX(begindown.x()),lpYTodpY(begindown.y()));
// 		p->restore();
// 	}
 	update();

}
// 
// void GSPSView::CalDefineRenge()//计算定义的区域
// {
// 	double tank=(double)-(endCRPPoint.y()-beginCRPPoint.y())/(endCRPPoint.x()-beginCRPPoint.x());
// 	double angle=atan(tank);
// 	float Wdith=GmProject::getProject()->getAreaWidth();
// 	int dis=(int)gDistance2D(beginCRPPoint.x(),beginCRPPoint.y(),endCRPPoint.x(),endCRPPoint.y());
// 	if(beginCRPPoint.x()>endCRPPoint.x()) dis=-dis;
// 	
// 	GPoint2d bPoint=beginCRPPoint;
// 	rotatecoord(bPoint,-angle);
// 	beginup.setX(bPoint.x());beginup.setY(bPoint.y()-Wdith);
//  	rotatecoord(beginup,angle);
//  	begindown.setX(bPoint.x());begindown.setY(bPoint.y()+Wdith);
// 	rotatecoord(begindown,angle);
// 	endup.setX(bPoint.x()+dis);endup.setY(bPoint.y()-Wdith);
// 	rotatecoord(endup,angle);
// 	enddown.setX(bPoint.x()+dis);enddown.setY(bPoint.y()+Wdith);
// 	rotatecoord(enddown,angle);
// 	
// 	GmProject *pProject=GmProject::getProject();
// 	GSPSParam *pSPSParam=pProject->getSPSParam();
// 	float LeftOffset=pSPSParam->mMaxLOffset*cos(angle);
// 	float RigthOffset=pSPSParam->mMaxROffset*cos(angle);
// 	pProject->setAngle(angle);
// 
// }
// 
// //赵虎添加，旋转坐标
// void GSPSView::rotatecoord(GPoint2d &point,double angle)
// {
// //	double sx=point.x()-beginCRPPoint.x();
// //	double sy=point.y()-beginCRPPoint.y();
//  	double sx=point.x();
//  	double sy=point.y();
// 	double sina;
// 	double cosa;
// 	double sxr=0;double syr=0;
// 	sina=(double)sin((double)angle);
// 	cosa=(double)cos((double)angle);
// 	sxr=sx*cosa+sy*sina;
// 	syr=-sx*sina+sy*cosa;
// 	sx=sxr;
// 	sy=syr;
// // 	sx=sxr+beginCRPPoint.x();
// // 	sy=syr+beginCRPPoint.y();
// 	point.setX(sx);point.setY(sy);
// //	mX = mSPSX*cosAF+mSPSY*sinAF;
// //	mY = -mSPSX*sinAF+mSPSY*cosAF;
// }

void GSPSView::paintEvent(QPaintEvent *e)
{
	Q3ScrollView::paintEvent(e);

	updateScrollBars();

	QPainter pb(this);
	drawHRuler(&pb, contentsX(), frameWidth(), frameWidth(), visibleWidth());
	drawVRuler(&pb, contentsY(), frameWidth(), frameWidth(), visibleHeight());
	if(mCartX >= mVRulerWidth+frameWidth())
		drawHCart(&pb, mCartX);
	if(mCartY >= mHRulerHeight+frameWidth())
		drawVCart(&pb, mCartY);
	
	pb.fillRect(1, 1, mVRulerWidth, mHRulerHeight, paletteBackgroundColor());
	pb.setPen(Qt::gray);
	pb.drawPoint(mVRulerWidth, mHRulerHeight);
	
	qDrawShadeRect(&pb, frameWidth()+4, frameWidth()+4, mVRulerWidth-8, mHRulerHeight-8, 
				  colorGroup(), true, 0, 1, &QBrush(paletteBackgroundColor()));

	QScrollBar *hb = horizontalScrollBar();
	QScrollBar *vb = verticalScrollBar();
	if(hb->isVisible() && vb->isVisible())
	{
		pb.fillRect(hb->x()+hb->width(), vb->y()+vb->height(), 
					vb->width()-1, hb->height()-1, paletteBackgroundColor());
	}
}

void GSPSView::drawSPoint(QPainter *p, int dx, int dy, const QColor &pColor, const QColor &bColor)
{
	p->setPen(pColor);
	p->setBrush(bColor);
	p->drawEllipse(dx-3, dy-3, 7, 7);
}

void GSPSView::drawCurrentSPoint(QPainter *p)
{
	if(mpSelSPoint == NULL) return ;

	GSPoint *pSPoint = mpSelSPoint;
	p->save();
	p->setCompositionMode(QPainter::CompositionMode_Xor);

	int dx = lpXTodpX(pSPoint->getSPSX());
	int dy = lpYTodpY(pSPoint->getSPSY());
	drawSPoint(p, dx, dy, Qt::black, Qt::red);

	p->setCompositionMode(QPainter::CompositionMode_SourceOver);
	p->restore();
}

//画炮点原点add by zhaohu
void GSPSView::drawBeginPoint(QPainter *p,double x,double y)
{
	p->save();
	p->setCompositionMode(QPainter::CompositionMode_Xor);

	int dx = lpXTodpX(x);
	int dy = lpYTodpY(y);
	drawSPoint(p, dx, dy, Qt::black, Qt::red);

	p->setCompositionMode(QPainter::CompositionMode_SourceOver);
	p->restore();
}

void GSPSView::drawSPoints(QPainter *p, int cx, int cy, int cw, int ch)
{
	int i, j;
	for(i=0; i<mpSLineSet->count(); i++)
	{
		GSLine *pSLine = mpSLineSet->get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			GSPoint *pSPoint = pSLine->get(j);
			if(pSPoint == mpSelSPoint)
			{
				drawCurrentSPoint(p);
				continue ;
			}

			// 我已经修改getSPoint()中的内存访问报错，结果的正确性待进一步验证 [Liu Wei 5/31/2023]
			if(mIsImpotSps==false)// add by zhaohu 
			{
				GSPoint *pBeginSPoint = mpShotSet->get(0)->getSPoint();//画炮点原点
				drawBeginPoint(p,pBeginSPoint->getSPSX(),pBeginSPoint->getSPSY());
				float rx = mpRLineSet->get(0)->get(0)->getSPSX();//画检波点原点
				float ry = mpRLineSet->get(0)->get(0)->getSPSY();//画检波点原点
				drawBeginPoint(p,rx,ry);
			}// 该功能模块的作用：在第一个炮点和检波点处绘制标记。 [Liu Wei 6/5/2023]		

			int dx = lpXTodpX(pSPoint->getSPSX());
			int dy = lpYTodpY(pSPoint->getSPSY());
			if(dx >= cx-40 && dx <= cx+cw+40 && 
			   dy >= cy-40 && dy <= cy+ch+40)
			{
				drawSPoint(p, dx, dy);
			}
		}
	}
}
//赵虎添加
// void GSPSView::drawRaySurPoints(QPainter *p, int cx, int cy, int cw, int ch)
// {
// 
// 	if(GmProject::getProject()->getRaySurfacePoint().size()>0)
// 	{
// 		vector<vector<GVector3d > > mSurfacePoint=GmProject::getProject()->getRaySurfacePoint();
// 	for(int i=0; i<mSurfacePoint.size(); i++)
// 	{
// 		for(int j=0;j<mSurfacePoint[i].size();j++)
// 		{
// 			int dx = lpXTodpX(mSurfacePoint[i][j].x());
// 			int dy = lpYTodpY(mSurfacePoint[i][j].z());
// 			if(dx >= cx-40 && dx <= cx+cw+40 && 
//  				dy >= cy-40 && dy <= cy+ch+40)
//  			{
// 				drawRaySurPoint(p,dx,dy);
//  			}
// 		}
// 	}
// 	}
// }
//赵虎添加
// void GSPSView::drawRaySurPoint(QPainter *p, int dx, int dy, const QColor &pColor ,	const QColor &bColor)
// {
// 	p->setPen(pColor);
// 	p->setBrush(bColor);
// 	p->drawEllipse(dx-1, dy-1, 3, 3);
// }
//赵虎添加画拟合的接收线
// void GSPSView::drawRayRline(QPainter *p,int cx, int cy, int cw, int ch,const QColor &pColor,const QColor &bColor)
// {
// 	if(RayRPoint.size()>0)
// 	{
// 		p->save();
// 		QPen pen1(bColor,3);
// 		p->setPen(pen1);
// 		p->setBrush(Qt::NoBrush);
// /*		QPainterPath path;
// 		int dx=lpXTodpX(RayRPoint[0].x());
// 		int dy=lpYTodpY(RayRPoint[0].y());
// 		path.moveTo(dx,dy);
// 		for(int i=1;i<RayRPoint.size();i++)
// 		{
// 			int dx=lpXTodpX(RayRPoint[i].x());
// 			int dy=lpYTodpY(RayRPoint[i].y());
// 			path.lineTo(dx,dy);	
// 		}
// 		p->drawPath(path);	
// 		*/
// 		p->restore();
// 	}
// 	
// }
 
//  double GSPSView::getRaydy(double *p,double x,int m)
//  {
//  	double y=0;
//  	for(int i=0;i<m;i++)
//  	{
// 		y=y+p[i]*pow(x,i);
//  	}
//  	return y;
//  }

void GSPSView::drawRPoint(QPainter *p, int dx, int dy, const QColor &pColor, const QColor &bColor)
{
	p->setPen(pColor);
    QPainterPath path;
	path.moveTo(dx-2, dy);	path.lineTo(dx+2, dy);
	path.moveTo(dx, dy-2);	path.lineTo(dx, dy+2);
	/*p->moveTo(dx-2, dy);	p->lineTo(dx+2, dy);
	p->moveTo(dx, dy-2);	p->lineTo(dx, dy+2);*/
	p->setBrush(Qt::NoBrush);
	p->drawPath(path);
}

void GSPSView::drawRPoints(QPainter *p, int cx, int cy, int cw, int ch)
{
	for(int i=0; i<mpRLineSet->count(); i++)
	{
		GRLine *pRLine = mpRLineSet->get(i);
		for(int j=0; j<pRLine->count(); j++)
		{
			GRPoint *pRPoint = pRLine->get(j);
			int dx = lpXTodpX(pRPoint->getSPSX());
			int dy = lpYTodpY(pRPoint->getSPSY());
			if(dx >= cx-40 && dx <= cx+cw+40 && 
			   dy >= cy-40 && dy <= cy+ch+40)
			{
				drawRPoint(p, dx, dy);
			}
		}
	}
}
//赵虎添加
// 绘制覆盖次数
// void GSPSView::drawFold(QPainter *p, int i, int j ,int fold, int cx, int cy, int cw, int ch)
// {
// 	int dx, dy;
// 	Q3PointArray pts(5);
// 
// 	GPoint3d pt = mpCellGrid->mGrid[i][j];
// 	dx = lpXTodpX(pt.x());
// 	dy = lpYTodpY(pt.z());
// 	if(dx >= cx-40 && dx <= cx+cw+40 && 
// 		dy >= cy-40 && dy <= cy+ch+40)
// 	{
// 		pts[0].setX(dx);	pts[0].setY(dy);
// 
// 		pt = mpCellGrid->mGrid[i+1][j];
// 		dx = lpXTodpX(pt.x());
// 		dy = lpYTodpY(pt.z());
// 		pts[1].setX(dx);	pts[1].setY(dy);
// 
// 		pt = mpCellGrid->mGrid[i+1][j+1];
// 		dx = lpXTodpX(pt.x());
// 		dy = lpYTodpY(pt.z());
// 		pts[2].setX(dx);	pts[2].setY(dy);
// 
// 		pt = mpCellGrid->mGrid[i][j+1];
// 		dx = lpXTodpX(pt.x());
// 		dy = lpYTodpY(pt.z());
// 		pts[3].setX(dx);	pts[3].setY(dy);
// 
// 		pts[4] = pts[0];
// 		p->save();
// 		QColor windowcolor;
// 		windowcolor=datatocolor(fold*15/mpCellGrid->setFold);
// 		QBrush brush;
// 		brush.setColor(windowcolor);
// 		p->setBrush(windowcolor);//定义画刷颜色
// 		QPen pen3(windowcolor,0);//定义画笔
// 		p->setPen(pen3);//选入画笔
// 		p->drawPolyline(pts);
// 		p->drawPolygon(pts);
// 		p->restore();
// 	}
// 
// }
// 
// QColor GSPSView::datatocolor(int m)//由数字得到颜色
// {
// 	QColor windowcolor;
// 	if(m==0) windowcolor.setRgb(255,0,254);
// 	if(m==1) windowcolor.setRgb(170,0,255);
// 	if(m==2) windowcolor.setRgb(85,0,255);
// 	if(m==3) windowcolor.setRgb(0,0,255);
// 	if(m==4) windowcolor.setRgb(0,84,255);
// 	if(m==5) windowcolor.setRgb(0,169,255);
// 	if(m==6) windowcolor.setRgb(0,254,255);
// 	if(m==7) windowcolor.setRgb(0,255,170);
// 	if(m==8) windowcolor.setRgb(0,255,85);
// 	if(m==9) windowcolor.setRgb(0,255,0);
// 	if(m==10) windowcolor.setRgb(169,255,0);
// 	if(m==11) windowcolor.setRgb(255,255,0);
// 	if(m==12) windowcolor.setRgb(255,170,0);
// 	if(m==13) windowcolor.setRgb(255,85,0);
// 	if(m>=14) windowcolor.setRgb(255,0,0);
// 	return windowcolor;
// }

void GSPSView::drawCell(QPainter *p, int i, int j, int cx, int cy, int cw, int ch,
						const QPen &pen, const QBrush &brush)
{
	int dx, dy;
	Q3PointArray pts(5);
	
	GPoint3d pt = mpCellGrid->mGrid[i][j];
	dx = lpXTodpX(pt.x());
	dy = lpYTodpY(pt.z());
	if(dx >= cx-40 && dx <= cx+cw+40 && 
		dy >= cy-40 && dy <= cy+ch+40)
	{
		pts[0].setX(dx);	pts[0].setY(dy);
		
		pt = mpCellGrid->mGrid[i+1][j];
		dx = lpXTodpX(pt.x());
		dy = lpYTodpY(pt.z());
		pts[1].setX(dx);	pts[1].setY(dy);
		
		pt = mpCellGrid->mGrid[i+1][j+1];
		dx = lpXTodpX(pt.x());
		dy = lpYTodpY(pt.z());
		pts[2].setX(dx);	pts[2].setY(dy);
		
		pt = mpCellGrid->mGrid[i][j+1];
		dx = lpXTodpX(pt.x());
		dy = lpYTodpY(pt.z());
		pts[3].setX(dx);	pts[3].setY(dy);
		
		pts[4] = pts[0];
		
		p->setPen(pen);
		p->drawPolyline(pts);
		
		if(brush.style() != Qt::NoBrush)
		{
			p->setBrush(brush);
			p->drawPolygon(pts);
		}
	}
}

void GSPSView::drawCells(QPainter *p, int cx, int cy, int cw, int ch)
{
	int i, j, nx, ny;
	nx = mpCellGrid->mNumX;
	ny = mpCellGrid->mNumY;
	for(i=0; i<ny; i++)
	{
		for(j=0; j<nx; j++)
		{
			drawCell(p, i, j, cx, cy, cw, ch);
			if(mpCellGrid->mFold!=NULL)
			{
				//if(mpCellGrid->mFold[i][j]>0) drawFold(p, i, j,mpCellGrid->mFold[i][j], cx, cy, cw, ch);
			}
		}
	}
}

void GSPSView::contentsMousePressEvent(QMouseEvent *e)
{
	Q3ScrollView::contentsMousePressEvent( e );

	setFocus();

	QPoint pos(e->pos());
	if(pos.x() < 0) pos.setX(0);
	if(pos.x() > contentsWidth()) pos.setX(contentsWidth());
	if(pos.y() < 0) pos.setY(0);
	if(pos.y() > contentsHeight()) pos.setY(contentsHeight());

	QMouseEvent me(e->type(), pos, e->button(), e->state());
	

	switch( mOperation )
	{
	case gZoom:		zoomMousePress(e);
						break;
	case gEditSPoint: editSPtMousePress(&me);
						break;
	/*case gmMovePoints:	movePointsMousePresse(x, y, e->button(), e->state());
						break;
	case gmZoom:		zoomMousePress(x, y, e->button());
						break;*/
	}
}

void GSPSView::contentsMouseMoveEvent(QMouseEvent *e)
{
	Q3ScrollView::contentsMouseMoveEvent(e);

	QPoint pos(e->pos());
	if(pos.x() < 0) pos.setX(0);
	if(pos.x() > contentsWidth()) pos.setX(contentsWidth());
	if(pos.y() < 0) pos.setY(0);
	if(pos.y() > contentsHeight()) pos.setY(contentsHeight());

	QMouseEvent me(e->type(), pos, e->button(), e->state());

	//----绘制定标器----
	int fw = frameWidth();
	int x = me.x()+mVRulerWidth+fw-contentsX();
	QPainter pb(this);
	if(mCartX != x) 
	{
		if(mCartX > 0) drawHCart(&pb, mCartX);
		mCartX = x;
		drawHCart(&pb, mCartX);
	}
	int y = me.y()+mHRulerHeight+fw-contentsY();
	if(mCartY != y)
	{
		if(mCartY > 0) drawVCart(&pb, mCartY);
		mCartY = y;
		drawVCart(&pb, mCartY);
	}
	
	//----更新坐标控件----
	QString tag;
	if(mXLabel != NULL)
	{
		double lx = dpXTolpX(e->x());
		tag.sprintf("X: %.2f", lx);
		mXLabel->setText(tag);
	}
	if(mYLabel != NULL)
	{
		double ly = dpYTolpY(e->y());
		tag.sprintf("Y: %.2f", ly);
		mYLabel->setText(tag);
	}

	switch(mOperation)
	{
	case gZoom:		zoomMouseMove(e);
						break;
	case gEditSPoint: editSPtMouseMove(&me);
						break;
	/*case gmMovePoints:	movePointsMouseMove(x, y, e->state());
						break;
	case gmZoom:		zoomMouseMove(x, y);
						break;*/
	}

	//赵虎添加
// 	if(valueDefineCRPline==true&&beginCRPPoint.x()!=0&&beginCRPPoint.y()!=0)
// 	{
// 		endCRPPoint.setX(dpXTolpX(pos.x()));
// 		endCRPPoint.setY(dpYTolpY(pos.y()));
// 		refresh();//刷新
// 	}
// 	if(mpCellGrid->maxFold>0)
// 	{
// 		float minSX, minSY, maxSX, maxSY;
// 		float minRX, minRY, maxRX, maxRY;
// 		minSX = minSY = HUGE_VAL;
// 		maxSX = maxSY = -HUGE_VAL;
// 		minRX = minRY = HUGE_VAL;
// 		maxRX = maxRY = -HUGE_VAL;
// 		mpSLineSet->getRange(minSX, minSY, maxSX, maxSY);
// 		mpRLineSet->getRange(minRX, minRY, maxRX, maxRY);
// 
// 		float minbxcoord=(minRX+minSX)/2+mpCellGrid->mDiffX;
// 		float minbycoord=(minRY+minSY)/2+mpCellGrid->mDiffY;
// 
// 		float angle=mpProject->getAngle();
// 		float sinAF = sin(-angle);
// 		float cosAF = cos(-angle);
// 
// 		int xbinnumber=mpCellGrid->mNumX;//x方面面元长度
// 		int ybinnumber=mpCellGrid->mNumY;
// 		double lx = dpXTolpX(e->x());
// 		double ly = dpYTolpY(e->y());
// 		float crpx,crpy;
// 		mpSPSParam->spsTotrans2(-sinAF, cosAF,lx,ly,crpx,crpy);
// 		//需要旋转crp点坐标
// 		int x=(crpx-minbxcoord)/mpCellGrid->mGridX;	
// 		if(x>xbinnumber) x=xbinnumber;
// 		if(x<0) x=0;
// 		mpProject->setXNum(x);//第几个面元
// 	}
// 	mpCMPView->repaintContents();//视图刷新
	repaintContents();
}

void GSPSView::contentsMouseReleaseEvent( QMouseEvent *e )
{
	Q3ScrollView::contentsMouseReleaseEvent(e);

	QPoint pos(e->pos());
	if(pos.x() < 0) pos.setX(0);
	if(pos.x() > contentsWidth()) pos.setX(contentsWidth());
	if(pos.y() < 0) pos.setY(0);
	if(pos.y() > contentsHeight()) pos.setY(contentsHeight());

	QMouseEvent me(e->type(), pos, e->button(), e->state());

	switch(mOperation)
	{
	case gZoom:		zoomMouseRelease(e);
						break;
	case gEditSPoint: editSPtMouseRelease(&me);
						break;
	/*case gmZoom:		zoomMouseRelease(x, y);
						break;
	case gmMovePoints:	movePointsMouseRelease(x, y, e->state());
						break;*/
	}

	//赵虎添加
// 	if(valueDefineCRPline==true)
// 	{
// 		if(beginCRPPoint.x()==0&&beginCRPPoint.y()==0) 
// 		{beginCRPPoint.setX(dpXTolpX(pos.x()));
// 		beginCRPPoint.setY(dpYTolpY(pos.y()));
// 		}
// 		else
// 		{
// 			valueDefineCRPline=false;
// 			CalDefineRenge();//计算定义的区域
// 			GmProject::getProject()->setDefineDpArea(beginCRPPoint,endCRPPoint,beginup,begindown,endup,enddown);//保存定义的区域坐标
// 			refresh();//刷新
// 		}
// 	}
}

GSPoint * GSPSView::ptInSPoint(int x, int y, int diff)
{
	int i, j;
	int dx, dy;
	GPoint3d pt;

	for(i=0; i<mpSLineSet->count(); i++)
	{
		GSLine *pSLine = mpSLineSet->get(i);
		for(j=0; j<pSLine->count(); j++)
		{
			GSPoint *pSPoint = pSLine->get(j);
			dx = lpXTodpX(pSPoint->getSPSX());
			dy = lpYTodpY(pSPoint->getSPSY());
			if(gDistance2D(x, y, dx, dy) < diff)
			{
				return pSPoint;
			}
		}
	}
	return NULL;
}

void GSPSView::editSPtMousePress(QMouseEvent *e)
{
	if(e->button() != Qt::LeftButton) return ;

	mpSelSPoint = ptInSPoint(e->x(), e->y(), mSelDiff);
	if(mpSelSPoint != NULL) repaintContents();
}

void GSPSView::editSPtMouseMove(QMouseEvent *e)
{
	if(mpSelSPoint == NULL) return ;

	QPainter p( viewport() );
	p.save();
	p.translate(-contentsX(), -contentsY());

	drawCurrentSPoint(&p);
	GSPoint *pSPoint = mpSelSPoint;
	pSPoint->setSPSX(dpXTolpX(e->x()));
	pSPoint->setSPSY(dpYTolpY(e->y()));
	drawCurrentSPoint(&p);
	p.restore();
}

void GSPSView::editSPtMouseRelease(QMouseEvent *e)
{
	if(mpSelSPoint == NULL) return ;

	QPainter p( viewport() );
	p.save();
	p.translate(-contentsX(), -contentsY());

	drawCurrentSPoint(&p);
	GSPoint *pSPoint = mpSelSPoint;
	pSPoint->setSPSX(dpXTolpX(e->x()));
	pSPoint->setSPSY(dpYTolpY(e->y()));
	p.restore();

	mpSelSPoint = NULL;
	repaintContents();
}

void GSPSView::zoomMousePress(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		QPoint pos = viewport()->mapToGlobal(contentsToViewport(e->pos()));
		mMouseX = pos.x();		mMouseY = pos.y();
		mPressBtn = Qt::LeftButton;
	}
}

void GSPSView::zoomMouseMove(QMouseEvent *e)
{
	if(mPressBtn == Qt::LeftButton)
	{
		QPoint pos = viewport()->mapToGlobal(contentsToViewport(e->pos()));
		double d = gDistance2D(mMouseX, mMouseY, pos.x(), pos.y());
		if(d < mZoomMouseStep) return ;

		int sign = mMouseY > pos.y() ? -1 : 1;	// 缩放标志
		int n = d / mZoomMouseStep;
		double s = sign * n * mZoomStep;
		if(mOperation == gZoom)
		{
			if(mZoomScaleX + s >= mMinZoomScale && mZoomScaleX + s <= mMaxZoomScale)
			{
				mZoomScaleX += s;
			}
		}
		
		if(mOperation == gZoom)
		{
			if(mZoomScaleY + s >= mMinZoomScale && mZoomScaleY + s <= mMaxZoomScale)
			{
				mZoomScaleY += s;
			}
		}
		refresh();
		emit zoomScaleChanged(mZoomScaleX, mZoomScaleY, mPreZoomScaleX, mPreZoomScaleY);
		mPreZoomScaleX = mZoomScaleX;
		mPreZoomScaleY = mZoomScaleY;

		mMouseX = pos.x();		mMouseY = pos.y();
	}
}

void GSPSView::zoomMouseRelease(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		mPressBtn = Qt::NoButton;
		mOperation = mPreOperation;
	}
}

void GSPSView::start(GOperation operation)
{
	stop();
	mPreOperation = mOperation;
	mOperation = operation;
	switch(operation)
	{
	case gEditSPoint:		mpSelSPoint = NULL;
							repaintContents();
							break;
	}
}

void GSPSView::stop()
{
	switch(mOperation)
	{
	case gEditSPoint:
		if(mpSelSPoint != NULL)
		{
			mpSelSPoint = NULL;
			repaintContents();
		}
		break;
	}
}
