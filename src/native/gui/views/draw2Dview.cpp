#include "math.h"
#include <qmessagebox.h>
#include <qlineedit.h>
#include <QPixmap>
#include <QMouseEvent>
#include "draw2Dview.h"

Draw2DView::Draw2DView(QWidget* parent) : 
Q3ScrollView(parent)
{
	mpBinImage = new QPixmap(0, 0);
	viewport()->setMouseTracking( true );
	zoomxscale=1;
	zoomyscale=1;
}

Draw2DView::~Draw2DView()
{

}

void Draw2DView::refresh()
{
	viewport()->setBackgroundColor(Qt::white);//将背景设置为白色
	resizeContents(0,0);
	mpBinImage->resize(0,0);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter p(mpBinImage);//定义绘图指针
	repaintContents();	
}
//加密炮分析

void Draw2DView::drawEnergy(float **array,int xnumber,int ynumber,float max,float min,QString strfname)//画能量图形
{
	viewx=950;viewy=650;
	viewx=viewx*zoomxscale;
	viewy=viewy*zoomyscale;
	resizeContents(viewx+150,viewy);
	mpBinImage->resize(viewx+150,viewy);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色		
	QPainter p(mpBinImage);//定义绘图指针
	int cx=viewx-50,cy=viewy-50;
	//画能量图形
	bx=1;by=1;
	int colorspac=20;//分几段颜色
	QPoint xx=kedufun(xnumber*bx);
	shotxscale=(float)(cx-50)/(xx.x()*xx.y());
	QPoint yy=kedufun(ynumber*by);
	shotyscale=(float)(cy-50)/(yy.x()*yy.y());
	//	float temp=(max-min)/colorspac;
	float mColor = (fabs(max - min) > 1e-6f) ? (255.0f / (max - min)) : 0.0f;
	QColor windowcolor;
	for(int m=0;m<xnumber;m++)
	{
		for(int n=0;n<ynumber;n++)
		{
			if(array[m][n]!=0) 
			{
				float media=0;
				if(array[m][n]>min) media=array[m][n]-min;                                       //与最小值得差值
				if(array[m][n]>=max) media=max-min;
				media=media*mColor;
				int ka = qBound(0, (int)media, 255);
				if(array[m][n]!=0) windowcolor.setRgb(colorbeltvary[ka][0]*255, colorbeltvary[ka][1]*255, colorbeltvary[ka][2]*255);   //RGB配色
				else windowcolor.setRgb(255,255,255);
				QBrush brush;
				brush.setColor(windowcolor);
				p.setBrush(windowcolor);//定义画刷颜色
				QPen pen3(windowcolor,0);//定义画笔
				p.setPen(pen3);//选入画笔
				int tx=bx*shotxscale+1;
				int ty=by*shotyscale+1;
				if(tx>=4&&ty>=4) p.drawRect(50+m*bx*shotxscale-tx/2,cy-n*by*shotyscale-ty/2,tx,ty);
				else if(tx>=4&&ty<4) p.drawRect(50+m*bx*shotxscale-tx/2,cy-n*by*shotyscale-2,tx,4);
				else if(tx<4&&ty>=4) p.drawRect(50+m*bx*shotxscale-2,cy-n*by*shotyscale-ty/2,4,ty);
				else p.drawRect(50+m*bx*shotxscale-2,cy-n*by*shotyscale-2,4,4);
				//				p.drawRect(50+m*bx*xscale-2,cy-n*by*yscale-2,4,4);//
			}
		}
	}

	QColor color;
	color.setRgb(238,238,238);
	QBrush brush1;
	brush1.setColor(color);
	p.setBrush(color);//定义画刷颜色
	QPen pen7(color,0);//定义画笔
	p.setPen(pen7);//选入画笔
	p.drawRect(0,0,viewx,50);
	p.drawRect(0,50,50,cy);
	p.drawRect(50,viewy-50,cx,50);

	for(int i=0;i<=xx.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p.setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p.setPen(pen);//选入画笔
			QString str;
			int temp=i*xx.x();
			str.sprintf("%d",temp);
			p.drawText(55+i*shotxscale*xx.x(),39,str);
			if(i!=0) p.drawText(45+i*shotxscale*xx.x(),cy+19,str);
			p.drawLine(50+i*shotxscale*xx.x(),50,50+i*shotxscale*xx.x(),40);
			p.drawLine(50+i*shotxscale*xx.x(),cy,50+i*shotxscale*xx.x(),cy+10);
		}
		else 
		{
			p.drawLine(50+i*shotxscale*xx.x(),50,50+i*shotxscale*xx.x(),45);
			p.drawLine(50+i*shotxscale*xx.x(),cy,50+i*shotxscale*xx.x(),cy+5);
		}
	}

	for(int i=0;i<yy.y();i++)//y方向刻度
	{		
		if(i%10==0)	
		{
			p.drawLine(50,cy-i*shotyscale*yy.x(),40,cy-i*shotyscale*yy.x());
			QFont font("Timers",10);
			p.setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p.setPen(pen);//选入画笔
			QString str;
			int temp=i*yy.x();
			str.sprintf("%d",temp);
			p.save();
			p.rotate(-90);
			p.setWindow(0,0,viewx,viewy);
			p.drawText(-cy+i*shotyscale*yy.x(),25,str);
			p.restore();
			p.save();
			p.rotate(-90);
			p.setWindow(0,0,viewx,viewy);
			p.drawText(-cy+i*shotyscale*yy.x(),25,str);
			p.drawText(-cy+viewy/2,15,"Y number");
			p.restore();
		}
		else
		{
			p.drawLine(50,cy-i*shotyscale*yy.x(),45,cy-i*shotyscale*yy.x());
		}
	}	
	
	p.drawText(viewx/2,25,"X number");
	p.drawText(viewx/4,25,strfname);
	
	QPen pen5(Qt::black,2);//定义画笔
	p.setPen(pen5);//选入画笔
	p.drawLine(50,50,viewx,50);
	p.drawLine(50,cy,viewx,cy);
	p.drawLine(50,50,50,cy);

	//画炮点
	//画能量图形
	repaintContents();
}

void Draw2DView::drawworkareaBinfold(int **array,int xline,int yline,QString mTextOffset)//整个工区覆盖次数
{
	float xscaleoffset=zoomxscale;
	float yscaleoffset=zoomyscale;
	viewx=xscaleoffset*xline+100;
	viewy=yscaleoffset*yline+100;
	if(viewx<=950) 
	{
		viewx=950;
		QPoint xx=kedufun(xline);
		xscaleoffset=(float)(viewx-100)/(xx.x()*xx.y());
	}
	if(viewy<=650) 
	{
		viewy=650;
		QPoint yy=kedufun(yline);
		yscaleoffset=(float)(viewy-100)/(yy.x()*yy.y());
	}
	int minfold=0;int maxfold=0;
	for(int i=0;i<xline;i++)
		for(int j=0;j<yline;j++)
		{
			if(minfold>array[i][j]) minfold=array[i][j];
			if(maxfold<array[i][j]) maxfold=array[i][j];
		}
		if(maxfold<=0) return;
		resizeContents(viewx+100,viewy);
		mpBinImage->resize(viewx+100,viewy);//定义绘图区域
		mpBinImage->fill(Qt::white);//绘图区域填充色		
		QPainter p(mpBinImage);//定义绘图指针
		p.save();
		QFont font("Timers",20);
		p.setFont(font);
		p.restore();
		p.drawText(100,20,mTextOffset);
		int cx=viewx-50,cy=viewy-50;
		QColor windowcolor;	
		float temp2 = (maxfold > minfold) ? ((float)(maxfold - minfold) / 15.0f) : 1.0f;
		for(int i=0;i<xline;i++)
		{
			for(int j=0;j<yline;j++)
			{
				if(array[i][j]!=0) windowcolor=datatocolor((array[i][j]-minfold)/temp2);
				else windowcolor.setRgb(255,255,255);
				QBrush brush;
				brush.setColor(windowcolor);
				p.setBrush(windowcolor);//定义画刷颜色
				QPen pen3(windowcolor,0);//定义画笔
				p.setPen(pen3);//选入画笔
				p.drawRect(50+i*xscaleoffset,cy-j*yscaleoffset-yscaleoffset,xscaleoffset+1,yscaleoffset+1);
			}
		}

		QPen pen5(Qt::black,2);//定义画笔
		p.setPen(pen5);//选入画笔
		p.drawLine(0,50,viewx,50);
		p.drawLine(0,cy,viewx,cy);
		p.drawLine(50,0,50,viewy);


		QPoint xx=kedufun(xline);
		for(int i=0;i<=xx.y();i++)//x方向刻度坐标
		{
			if(i%10==0)
			{
				QFont font("Timers",10);
				p.setFont(font);
				QPen pen(Qt::black,0);//定义画笔
				p.setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i*xx.x());
				p.drawText(55+i*xscaleoffset*xx.x(),39,str);
				if(i!=0) p.drawText(45+i*xscaleoffset*xx.x(),cy+19,str);
				p.drawLine(50+i*xscaleoffset*xx.x(),50,50+i*xscaleoffset*xx.x(),40);
				p.drawLine(50+i*xscaleoffset*xx.x(),cy,50+i*xscaleoffset*xx.x(),cy+10);
				p.drawText(viewx/2,20,"x number");
			}
			else 
			{
				p.drawLine(50+i*xscaleoffset*xx.x(),50,50+i*xscaleoffset*xx.x(),45);
				p.drawLine(50+i*xscaleoffset*xx.x(),cy,50+i*xscaleoffset*xx.x(),cy+5);
			}
		}
		QPoint yy=kedufun(yline);
		for(int i=0;i<yy.y();i++)//y方向刻度
		{		
			if(i%10==0)	
			{
				p.drawLine(50,cy-i*yscaleoffset*yy.x(),40,cy-i*yscaleoffset*yy.x());
				QFont font("Timers",10);
				p.setFont(font);
				QPen pen(Qt::black,0);//定义画笔
				p.setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i*yy.x());
				p.save();
				p.rotate(-90);
				p.setWindow(0,0,viewx,viewy);
				p.drawText(-cy+i*yscaleoffset*yy.x(),25,str);
				p.drawText(-cy+cy/2,15,"y number");
				p.restore();
			}
			else
			{
				p.drawLine(50,cy-i*yscaleoffset*yy.x(),45,cy-i*yscaleoffset*yy.x());
			}
		}
		//画色标条
		float temp=(cy-50)/(2*15);
		for(int i=0;i<=15;i++)//画色标条
		{		
			windowcolor=datatocolor(i-1);
			QBrush brush;
			brush.setColor(windowcolor);
			p.setBrush(windowcolor);
			QPen pen(windowcolor,0);
			p.setPen(pen);
			if(i>0) p.drawRect(viewx,cy-50-(cy-50-15*temp)/2-i*temp,20,temp);
			QString str;
			int hh=0;
			if(minfold!=maxfold||(minfold==maxfold&&minfold!=0)) hh=i*temp2+minfold;
			if(i==15) str.sprintf("%d",maxfold);
			else str.sprintf("%d",hh);
			QPen pen2(Qt::black,0);
			p.setPen(pen2);
			p.drawLine(viewx,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2-temp*15);
			p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2);
			p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2);
			p.drawLine(viewx,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2-temp*15);
			p.drawText(viewx+20,cy-50-(cy-50-15*temp)/2-i*temp,str);	
		}
		//画色标条
		repaintContents();
}

//常用函数
QPoint Draw2DView::kedufun(int m)//为一个数选择合适的刻度函数
{
	QPoint point;
	float kedu=m;
	int ii=1,a=0;
	if(kedu<1&&kedu>=-1)
	{
		while(kedu*ii<10)
		{
			a=kedu*ii;
			ii=ii*10;
		}
		kedu=-ii;
		a=(a+1)*ii/10;
	}
	else if(kedu==1)
	{
		a=10;
		kedu=-10;
	}
	else if(kedu<=10)
	{
		while(kedu/ii>1)
		{
			a=kedu/ii;
			ii=ii*10;
		}
		kedu=ii/10;
		a=m/kedu;
	}
	else
	{
		while(kedu/ii>1)//选择一个合适的刻度
		{
			a=kedu/ii;
			if(kedu-a*ii!=0)
				a=a+1;
			ii=ii*10;	
		}
		kedu=ii/100;
		a=m/kedu+1;	
	}
	point.setX(kedu);
	point.setY(a);
	return point;		
}

QColor Draw2DView::datatocolor(int m)//由数字得到颜色
{
	QColor windowcolor;
	if(m==0) windowcolor.setRgb(255,0,254);
	if(m==1) windowcolor.setRgb(170,0,255);
	if(m==2) windowcolor.setRgb(85,0,255);
	if(m==3) windowcolor.setRgb(0,0,255);
	if(m==4) windowcolor.setRgb(0,84,255);
	if(m==5) windowcolor.setRgb(0,169,255);
	if(m==6) windowcolor.setRgb(0,254,255);
	if(m==7) windowcolor.setRgb(0,255,170);
	if(m==8) windowcolor.setRgb(0,255,85);
	if(m==9) windowcolor.setRgb(0,255,0);
	if(m==10) windowcolor.setRgb(169,255,0);
	if(m==11) windowcolor.setRgb(255,255,0);
	if(m==12) windowcolor.setRgb(255,170,0);
	if(m==13) windowcolor.setRgb(255,85,0);
	if(m>=14) windowcolor.setRgb(255,0,0);
	return windowcolor;
}

void Draw2DView::contentsMousePressEvent(QMouseEvent *e)
{

}

void Draw2DView::contentsMouseMoveEvent(QMouseEvent *e)
{
	
}

void Draw2DView::contentsMouseReleaseEvent(QMouseEvent *e)
{

}

void Draw2DView::contentsMouseDoubleClickEvent( QMouseEvent* e)//鼠标双击事件
{
	
}

void Draw2DView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)//不断的刷新贴图
{
	p->drawPixmap(cx, cy, *mpBinImage, cx, cy, cw, ch);
}