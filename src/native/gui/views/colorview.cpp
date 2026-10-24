#include "colorview.h"
#include "qerrormessage.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <cmath>
#include <qfont.h> 
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qcolor.h> 
#include <qbrush.h> 
#include <qlineedit.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include <qwidget.h>

tcolorview::tcolorview(QWidget* parent) : 
Q3ScrollView(parent)
{
	mpImage = new QPixmap(0, 0);
	viewport()->setMouseTracking( true );
	alloc2D(colorbeltvary,256,3);
	mWhich=3;
	cacualateColorbelt(mWhich);
}

tcolorview::~tcolorview()
{
	clear2D(colorbeltvary,256);
}

void tcolorview::refresh()
{
	//	viewport()->setBackgroundColor(white);//将背景设置为白色	
	resizeContents(0,0);
	mpImage->resize(0,0);//定义绘图区域
	mpImage->fill(Qt::white);//绘图区域填充色
	QPainter p(mpImage);//定义绘图指针
	update();
}

void tcolorview::drawcolorcode(float max,float min)//画色标
{
	int colorspac=20;//分几段颜色
	int viewx=30;
	int viewy=220;
	resizeContents(viewx+70,viewy+40);
	mpImage->resize(viewx+70,viewy+40);//定义绘图区域
	mpImage->fill(Qt::white);//绘图区域填充色		
	QPainter p(mpImage);//定义绘图指针
	//画色标条
	float temp=(max-min)/colorspac;
	int temp2=viewy/colorspac;
	QColor windowcolor;
	for(int i=1;i<=colorspac;i++)//画色标条
	{		
		windowcolor=datatocolor(i-1);
		QBrush brush;
		brush.setColor(windowcolor);
		p.setBrush(windowcolor);
		QPen pen(windowcolor,0);
		p.setPen(pen);
		p.drawRect(20,viewy+20-i*temp2,viewx,temp2);
		QString str;
		double hh=0;
		if(min!=max||(min=max&&min!=0)) hh=i*temp+min;
		if(i==colorspac) hh=max;
		int n=floor(log10(fabs(hh)));
		float x=hh/pow((float)10,n)+0.1;
		str.sprintf("%2.1fe%d\n", hh/pow((float)10,n),n);//使用科学记数法
		p.save();
		QPen pen2(Qt::black,0);
		p.setPen(pen2);
		p.drawLine(20,viewy+20-i*temp2,viewx+20,viewy+20-i*temp2);
		p.drawText(viewx+22,viewy+20+temp2/2-i*temp2,str);
		p.restore();	
	}
	QPen pen4(Qt::blue,1);
	p.setPen(pen4);
	p.drawText(20,15,QString::fromLocal8Bit("面元能量"));//表示是第几炮图形
	QPen pen3(Qt::black,1);
	p.setPen(pen3);
	p.drawLine(1,viewy+39,viewx+70,viewy+39);
	p.drawLine(0,0,viewx+70,0);
	p.drawLine(viewx+69,0,viewx+69,viewy+40);
	p.drawLine(0,0,0,viewy+40);
	p.drawLine(20,viewy+20-colorspac*temp2,viewx+20,viewy+20-colorspac*temp2);
	p.drawLine(20,viewy+20,viewx+20,viewy+20);
	p.drawLine(viewx+20,viewy+20,viewx+20,viewy+20-colorspac*temp2);
	p.drawLine(20,viewy+20,20,viewy+20-colorspac*temp2);
	//画色标条
	repaintContents();
}



QColor tcolorview::datatocolor(int m)//由数字得到颜色
{
	QColor windowcolor;
	if(m==0) windowcolor.setRgb(255,255,255);
	if(m==1) windowcolor.setRgb(255,242,242);
	if(m==2) windowcolor.setRgb(255,229,229);
	if(m==3) windowcolor.setRgb(255,216,216);
	if(m==4) windowcolor.setRgb(255,203,203);
	if(m==5) windowcolor.setRgb(255,191,191);
	if(m==6) windowcolor.setRgb(255,178,178);
	if(m==7) windowcolor.setRgb(255,165,165);
	if(m==8) windowcolor.setRgb(255,152,152);
	if(m==9) windowcolor.setRgb(255,140,140);
	if(m==10) windowcolor.setRgb(255,127,127);
	if(m==11) windowcolor.setRgb(255,114,114);
	if(m==12) windowcolor.setRgb(255,101,101);
	if(m==13) windowcolor.setRgb(255,89,89);
	if(m==14) windowcolor.setRgb(255,76,76);
	if(m==15) windowcolor.setRgb(255,63,63);
	if(m==16) windowcolor.setRgb(255,50,50);
	if(m==17) windowcolor.setRgb(255,38,38);
	if(m==18) windowcolor.setRgb(255,25,25);
	if(m==19) windowcolor.setRgb(255,12,12);
	if(m>=20) windowcolor.setRgb(255,0,0);
	return windowcolor;
}

void tcolorview::cacualateColorbelt(short which)
{
	mWhich=which;
	//float value2=255*0.2;  
	//色带均分为5段
	if(which==1)
	{
		for (int j=0;j<5;j++)
		{
			for (int i=0;i<=51;i++)
			{
				switch (j)
				{
				case 0:
					colorbeltvary[i+51*j][0]=1-i*0.0196;
					colorbeltvary[i+51*j][1]=i*0.0196;
					colorbeltvary[i+51*j][2]=0;
					break;
				case 1:
					colorbeltvary[i+51*j][0]=i*0.0196;
					colorbeltvary[i+51*j][1]=1;
					colorbeltvary[i+51*j][2]=0;
					break;
				case 2:
					colorbeltvary[i+51*j][0]=1-0.0085*i;
					colorbeltvary[i+51*j][1]=1-0.0051*i;
					colorbeltvary[i+51*j][2]=0.01608*i;
					break;
				case 3:
					colorbeltvary[i+51*j][0]=0.5664-0.00383*i;
					colorbeltvary[i+51*j][1]=0.7422-0.0023*i;
					colorbeltvary[i+51*j][2]=0.8203-0.00153*i;
					break;
				case 4:
					colorbeltvary[i+51*j][0]=0.3711-0.0072*i;
					colorbeltvary[i+51*j][1]=0.625-i*0.01225;
					colorbeltvary[i+51*j][2]=0.7422+i*0.00505;
					break;

				}
			}
		}
	}
	if(which==2)
	{
		for (int j=0;j<5;j++)
		{
			for (int i=0;i<=51;i++)
			{
				switch (j)
				{
				case 0:
					colorbeltvary[i+51*j][0]=0;
					colorbeltvary[i+51*j][1]=0.00490196*i;
					colorbeltvary[i+51*j][2]=1;
					break;
				case 1:
					colorbeltvary[i+51*j][0]=0;
					colorbeltvary[i+51*j][1]=0.25+0.00490196*i;
					colorbeltvary[i+51*j][2]=1-0.005882*i;
					break;
				case 2:
					colorbeltvary[i+51*j][0]=0;
					colorbeltvary[i+51*j][1]=0.5+0.009804*i;
					colorbeltvary[i+51*j][2]=0.66-0.005882*i;
					break;
				case 3:
					colorbeltvary[i+51*j][0]=0.0196078*i;
					colorbeltvary[i+51*j][1]=1;
					colorbeltvary[i+51*j][2]=0.33-0.005882*i;
					break;
				case 4:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0196078*i;
					colorbeltvary[i+51*j][2]=0;
					break;

				}
			}
		}
	}

	if(which==3)
	{
		for (int j=0;j<5;j++)
		{
			for (int i=0;i<=51;i++)
			{
				switch (j)
				{
				case 0:
					colorbeltvary[i+51*j][0]=1-0.0196078*i;
					colorbeltvary[i+51*j][1]=0;
					colorbeltvary[i+51*j][2]=1;
					break;
				case 1:
					colorbeltvary[i+51*j][0]=0;
					colorbeltvary[i+51*j][1]=0+0.0196078*i;
					colorbeltvary[i+51*j][2]=1;
					break;
				case 2:
					colorbeltvary[i+51*j][0]=0;
					colorbeltvary[i+51*j][1]=1;
					colorbeltvary[i+51*j][2]=1-0.0196078*i;
					break;
				case 3:
					colorbeltvary[i+51*j][0]=0.0196078*i;
					colorbeltvary[i+51*j][1]=1;
					colorbeltvary[i+51*j][2]=0;
					break;
				case 4:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0196078*i;
					colorbeltvary[i+51*j][2]=0;
					break;

				}
			}
		}
	}
	if(which==4)
	{
		for (int j=0;j<5;j++)
		{
			for (int i=0;i<=51;i++)
			{
				switch (j)
				{
				case 0:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0039216*i;
					colorbeltvary[i+51*j][2]=1-0.0039216*i;
					break;
				case 1:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0039216*(i+51);
					colorbeltvary[i+51*j][2]=1-0.0039216*(i+51);
					break;
				case 2:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0039216*(i+102);
					colorbeltvary[i+51*j][2]=1-0.0039216*(i+102);
					break;
				case 3:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0039216*(i+153);
					colorbeltvary[i+51*j][2]=1-0.0039216*(i+153);
					break;
				case 4:
					colorbeltvary[i+51*j][0]=1;
					colorbeltvary[i+51*j][1]=1-0.0039216*(i+204);
					colorbeltvary[i+51*j][2]=1-0.0039216*(i+204);
					break;
				}
			}
		}
	}

}
void tcolorview::drawcolorcodevary(float max,float min)//画色标(渐变色)
{
	mMaxvalueColor=max;
	mMinvalueColor=min;
	cacualateColorbelt(mWhich);
	int colorspac=255;                       //均分255分
	float value1=255/colorspac;              //每次绘制的增量
	double hh=0;
	int viewx=30;
	int viewy=300;
	resizeContents(viewx+70,viewy+40);
	mpImage->resize(viewx+70,viewy+40);//定义绘图区域
	mpImage->fill(Qt::white);//绘图区域填充色		
	QPainter p(mpImage);//定义绘图指针
	//画色标条
	float temp=(max-min)/colorspac;
	int   temp2=viewy/256;
	QColor windowcolor;
	for(int i=0;i<=255;i++)//画色标条
	{	
		windowcolor.setRgb(colorbeltvary[i][0]*255, colorbeltvary[i][1]*255, colorbeltvary[i][2]*255);   //RGB配色
		QBrush brush;
		brush.setColor(windowcolor);
		p.setBrush(windowcolor);
		QPen pen(windowcolor,0);
		p.setPen(pen);
		p.drawRect(20,viewy+20-i*temp2,viewx,temp2);


		QString str;
		if(min!=max||(min=max&&min!=0)) hh=i*temp+min;
		if(i==colorspac) hh=max;
		if(i%15==0&&i!=0) 
		{
			int n=floor(log10(fabs(hh)));
			float x=hh/pow((float)10,n)+0.1;
			str.sprintf("%2.1fe%d\n", hh/pow((float)10,n),n);//使用科学记数法
		}
		p.save();
		QPen pen2(Qt::black,0);
		p.setPen(pen2);
		p.drawLine(20,viewy+20-i*temp2,viewx+20,viewy+20-i*temp2);
		p.drawText(viewx+22,viewy+22-i*temp2,str);
		p.restore();	
	}
	QPen pen4(Qt::blue,1);
	p.setPen(pen4);
	p.drawText(20,15,QString::fromLocal8Bit("能量显示"));              //表示是第几炮图形
	QPen pen3(Qt::black,1);
	p.setPen(pen3);
	p.drawLine(1,viewy+39,viewx+70,viewy+39);
	p.drawLine(0,0,viewx+70,0);
	p.drawLine(viewx+69,0,viewx+69,viewy+40);
	p.drawLine(0,0,0,viewy+40);
	p.drawLine(20,viewy+20-colorspac*temp2,viewx+20,viewy+20-colorspac*temp2);
	p.drawLine(20,viewy+20,viewx+20,viewy+20);
	p.drawLine(viewx+20,viewy+20,viewx+20,viewy+20-colorspac*temp2);
	p.drawLine(20,viewy+20,20,viewy+20-colorspac*temp2);
	//画色标条
	repaintContents();
}

//常用函数

bool tcolorview::alloc2D(float ** &matrix, int ln, int col)//分配内存
{
	matrix = new float *[ln];
	if(matrix == NULL) return false;	
	for(int i=0; i<ln; i++)
	{
		matrix[i] = new float[col];
		//检查该行数是否分配成功，如果分配失败，则删除以前分配的空间
		if(matrix[i] == NULL)	
		{
			for(int j=0; j<i; j++)
			{
				delete []matrix[j];
			}
			delete []matrix;
			return false;
		}
	}
	return true;
}

void tcolorview::clear2D(float ** &matrix, int ln)//删除内存
{
	if( matrix != NULL ) 
	{
		for( Q_INT32 i=0; i<ln; i++ )
		{
			delete [](matrix[i]);
		}
		delete []matrix;
		matrix = NULL;
	}
}
void tcolorview::contentsMouseDoubleClickEvent( QMouseEvent* e)
{
	QPoint a=e->pos();
}
void tcolorview::contentsMousePressEvent(QMouseEvent *e)//单击
{

}

void tcolorview::contentsMouseMoveEvent(QMouseEvent *e)
{

}

void tcolorview::contentsMouseReleaseEvent(QMouseEvent *e)
{

}
void tcolorview::drawContents(QPainter* p, int cx, int cy, int cw, int ch)//不断的刷新贴图
{
	p->drawPixmap(cx, cy, *mpImage, cx, cy, cw, ch);
}