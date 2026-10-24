#include "analyzebinview.h"
#include "math.h"
#include <qmessagebox.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include "changecoordinatedlg.h"
#include "qmessagebox.h"

analyzebinview::analyzebinview(QWidget* parent) : 
Q3ScrollView(parent)
{
	mpProject = GProject::getCurrentProject();
	mpCellGrid = mpProject->getCellGrid();
	blkCoordinate = 50;//默认留白像素宽度：50
	mpBinImage = new QPixmap(0, 0);
	viewport()->setMouseTracking( true );
	valuelineoffseVSfoldbins=false;
	valuelineroseVStraces=false;

	xcoordlinefold=0;//x方向最表最大值
	xcoordlineoffset=0;
	ycoordlinefold=0;//y方向最表最大值
	ycoordlineAzimuth=0;
	ycoordlineoffset=0;
	ycoordlineOffsetVSFoldBins=0;

	xcoordworkfold=0;//x方向最表最大值
	xcoordworkoffset=0;
	ycoordworkfold=0;//y方向最表最大值
	ycoordworkAzimuth=0;
	ycoordworkoffset=0;
	ycoordworkOffsetVSFoldBins=0;
	//  [Liu Wei 3/28/2023]
	xscaleoffset=0.1;
	yscaleoffset=0.1;
}

analyzebinview::~analyzebinview()
{

}

void analyzebinview::refresh()
{

	viewport()->setBackgroundColor(Qt::white);//将背景设置为白色
	resizeContents(0,0);
	mpBinImage->resize(0,0);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter p(mpBinImage);//定义绘图指针
	bool drawpointbin=mpBinDoc->getData3();//得到分析点面元标志
	binpointoffset= mpBinDoc->getData();//得到点面元偏移距数组
	binpointazimuth=mpBinDoc->getData2();//得到点面元方位角数组
	binpointfold=mpBinDoc->getData4();//得到点面元覆盖次数
	binmousepoint=mpBinDoc->getDatapoint();//得到鼠标点坐标
	tracespac=mpBinDoc->getDatatracesapc();//得到道间距
	if(drawpointbin==TRUE&&binpointfold!=0) 
	{
		resizeContents(0,0);
		mpBinImage->resize(0,0);//定义绘图区域
		mpBinImage->fill(Qt::white);//绘图区域填充色
		QPainter p(mpBinImage);//定义绘图指针
		drawBinpoint(&p,mpBinDoc->point);//点面元分析	
	}

	//点面元属性分析
	maxoffset=mpBinDoc->getDatamaxoffset();//得到最大炮检距
	maxfold=mpBinDoc->getDatamaxfold();//得到最大覆盖次数
	xbinnumber=mpBinDoc->getDataXbinnumber();//x方向面元个数
	ybinnumber=mpBinDoc->getDataYbinnumber();//y方向面元个数
	lineoffsetVStraces=mpBinDoc->getDatalineOffsetVSTraces();
	lineazimuthVStraces=mpBinDoc->getDatalineAzimuthVSTraces();
	lineroseVStraces=mpBinDoc->getDatalineMeiguiVSTraces();
	linefoldVSbins=mpBinDoc->getDatalineFoldVSBins();//线面元，覆盖次数-面元数量
	linecoordinate=mpBinDoc->getDatalinecoordinate();//得到线的位置	
	lineoffsetVSfoldbins=mpBinDoc->getDatalineOffsetVSFoldBins();//得到炮检距-覆盖次数-面元数量数组
	bool drawlinebin=mpBinDoc->getDataValueline();//得到线面元属性分析标志
	if(drawlinebin==true)
	{
		//resizeContents(960,670);
		//mpBinImage->resize(960,670);//定义绘图区域
		//  [Liu Wei 3/28/2023]
		resizeContents(960,700);
		mpBinImage->resize(960,700);//定义绘图区域
		//  [Liu Wei 3/28/2023]
		mpBinImage->fill(Qt::white);//绘图区域填充色
		QPainter p(mpBinImage);//定义绘图指针
		p.save();
		QFont font("Timers",20);
		p.setFont(font);
		p.drawText(550,20,"analyze line:"+QString::number(linecoordinate.x())+"m  -  "+QString::number(linecoordinate.y())+"m");
		p.restore();
		if(mpBinDoc->valuefoldVSbins==true) drawlineFoldVSBIns(&p,xcoordlinefold,ycoordlinefold);//线面元覆盖次数-面元个数分析
		if(mpBinDoc->valueoffsetVStraces==true) drawlineOffsetVSTraces(&p,xcoordlineoffset,ycoordlineoffset);//线面元炮检距-炮检对
		if(mpBinDoc->valueazimuthVStraces==true) drawlineAzimuthVSTraces(&p,ycoordlineAzimuth); //线面元属性分析,方位角-炮检对 
		if(mpBinDoc->valueroseVStraces==true) 
		{
			drawlineRoseVSTraces(&p); //线面元属性分析,玫瑰花图-炮检对
			valuelineroseVStraces=true;
		}
		if(mpBinDoc->valueoffsetVSfoldbins==true) 
		{
			drawlineOffsetVSFoldBins(&p,ycoordlineOffsetVSFoldBins);//得到炮检距-覆盖次数-面元数量数组，覆盖次数没有为零
			valuelineoffseVSfoldbins=true;
		}
	}
	//线属性分析	
	offsetVStraces=mpBinDoc->getDataOffsetVSTraces();//得到炮检距-炮检对数组
	Binfold=mpBinDoc->getDataBinfold();//得到覆盖次数
	azimuthVStraces=mpBinDoc->getDataAzimuthVSTraces();//得到方位角-炮检对数组
	roseVStraces=mpBinDoc->getDataMeiguiVSTraces();//得到玫瑰花图-炮检对数组
	offsetVSfoldbins=mpBinDoc->getDataOffsetVSFoldBins();//得到炮检距-覆盖次数-面元数量数组，覆盖次数没有为零
	foldVSbins=mpBinDoc->getDataFoldVSBins();//得到覆盖次数-面元数量数组
	bool drawworkarea=mpBinDoc->getDataValueworkarea();//得到整个工区面元分析标志	

	if(drawworkarea==TRUE)
	{
		resizeContents(0,0);
		mpBinImage->resize(0,0);//定义绘图区域
		mpBinImage->fill(Qt::white);//绘图区域填充色
		QPainter p(mpBinImage);//定义绘图指针
		p.save();
		QFont font("Timers",20);
		p.setFont(font);
		//		p.drawText(700,20,"analyze work area");
		p.restore();
		//		if(mpBinDoc->valueoffsetVStraces==true) drawworkareaOffsetVSTraces(&p,xcoordworkoffset,ycoordworkoffset);//整个工区炮检距-炮检对
		//		if(mpBinDoc->valueazimuthVStraces==true) drawworkareaAzimuthVSTraces(&p,ycoordworkAzimuth); //整个工区面元属性分析,方位角-炮检对 
		//		if(mpBinDoc->valueroseVStraces==true) drawworkareaRoseVSTraces(&p); //整个工区面元属性分析,玫瑰花图-炮检对
		//		if(mpBinDoc->valueoffsetVSfoldbins==true) drawworkareaOffsetVSFoldBins(&p,ycoordworkOffsetVSFoldBins);//整个工区面元属性分析，炮检距-覆盖次数-面元数量

		if(mpBinDoc->valuefoldVSbins==true) drawworkareaFoldVSBIns(xcoordworkfold,ycoordworkfold);//整个工区覆盖次数-面元个数分析
	}

	//整个工区属性分析
	repaint();//刷新
	repaintContents();	
}

void analyzebinview::drawBinpoint(QPainter *p,int ii)//点面元属性分析
{
	QPen pen(Qt::black,3);//定义画笔
	p->setPen(pen);//选入画笔
	p->drawEllipse(100,100,300,300);
	QPen pen2(Qt::black,0);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(250,100,250,400);
	p->drawLine(100,250,400,250);
	int maxoffset=0,minoffset=100000;
	for(int i=0;i<binpointfold[ii];i++)
	{
		if(maxoffset<binpointoffset[ii][i])	maxoffset=binpointoffset[ii][i];
		if(minoffset>binpointoffset[ii][i]) minoffset=binpointoffset[ii][i];
	}
	if (maxoffset <= 0) maxoffset = 1;
	float scale=(float)150/maxoffset;
	float x=0,y=0;

	QPen pen3(Qt::blue,0);//定义画笔
	p->setPen(pen3);//选入画笔
	for(int i=0;i<binpointfold[ii];i++)
	{
		if(binpointoffset[ii][i]!=-1)
		{
			//			y=-binpointoffset[ii][i]*scale*cos(binpointazimuth[ii][i]*3.14159265/180)+250;
			//			x=binpointoffset[ii][i]*scale*sin(binpointazimuth[ii][i]*3.14159265/180)+250;
			y=-maxoffset*scale*cos(binpointazimuth[ii][i]*3.14159265/180)+250;
			x=maxoffset*scale*sin(binpointazimuth[ii][i]*3.14159265/180)+250;
			p->drawLine(250,250,x,y);
		}		
	}
	float xbegin=500,xend=800;
	float ybegin=100,yend=400;
	float xscale=(xend-xbegin)/maxoffset;//x方向刻度
	float yscale=(yend-ybegin)/maxoffset;//y方向刻度
	QPen pen4(Qt::black,1);//定义画笔
	p->setPen(pen4);//选入画笔
	for(int i=0;i<binpointfold[ii];i++)//画炮检距分布图
	{
		x=xbegin+binpointoffset[ii][i]*xscale;
		y=yend-binpointoffset[ii][i]*yscale;
		p->drawLine(x,yend,x,y);
	}
	p->drawText(100,80,QString::fromLocal8Bit("覆盖次数: ")+QString::number(binpointfold[ii]));
	p->drawText(200,80,QString::fromLocal8Bit("最小炮检距: ")+QString::number(minoffset));
	p->drawText(300,80,QString::fromLocal8Bit("最大炮检距: ")+QString::number(maxoffset));
	int xx=binmousepoint.x()+(ii-61/2)*tracespac/2;
	p->drawText(100,50,QString::fromLocal8Bit("x相对坐标: ")+QString::number(xx));
	p->drawText(300,50,QString::fromLocal8Bit("y相对坐标: ")+QString::number(binmousepoint.y()));
}

void analyzebinview::drawlineFoldVSBIns(QPainter *p,int xcoord,int ycoord)//覆盖次数-面元个数分析
{
	int maxbins=0;
	if(ycoord==0)
	{		
		for(int i=0;i<maxfold;i++)
		{
			if(maxbins<linefoldVSbins[i]) maxbins=linefoldVSbins[i];
		}
	}
	else maxbins=ycoord;
	if(xcoord==0) binfold=kedufun(maxfold);
	else binfold=kedufun(xcoord);
	binnumber=kedufun(maxbins);//

	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);
	float xscale=(float)(950-50)/(binfold.x()*binfold.y());
	for(int i=0;i<=binfold.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*binfold.x());
			p->drawText(45+i*xscale*binfold.x(),39,str);
			if(i!=0) p->drawText(45+i*xscale*binfold.x(),669,str);
			p->drawLine(50+i*xscale*binfold.x(),50,50+i*xscale*binfold.x(),40);
			p->drawLine(50+i*xscale*binfold.x(),650,50+i*xscale*binfold.x(),660);
		}
		else 
		{
			p->drawLine(50+i*xscale*binfold.x(),50,50+i*xscale*binfold.x(),45);
			p->drawLine(50+i*xscale*binfold.x(),650,50+i*xscale*binfold.x(),655);
		}
	}
	float yscale=(float)(650-50)/(binnumber.x()*binnumber.y());
	for(int i=0;i<binnumber.y();i++)
	{
		if(i%10==0)	
		{
			p->drawLine(50,650-i*yscale*binnumber.x(),40,650-i*yscale*binnumber.x());
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*binnumber.x());
			p->drawText(20,650-i*yscale*binnumber.x(),str);
		}
		else
		{
			p->drawLine(50,650-i*yscale*binnumber.x(),45,650-i*yscale*binnumber.x());
		}
	}
	p->drawText(450,20,"FOLD");
	p->drawText(10,335,"BINS");
	for(int i=0;i<maxfold-1;i++)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+(i+1)*xscale,650-(linefoldVSbins[i]*yscale),50+(i+2)*xscale,650-(linefoldVSbins[i+1]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+(i+1)*xscale-3,650-(linefoldVSbins[i]*yscale),50+(i+1)*xscale+3,650-(linefoldVSbins[i]*yscale));
		p->drawLine(50+(i+1)*xscale,650-(linefoldVSbins[i]*yscale)-3,50+(i+1)*xscale,650-(linefoldVSbins[i]*yscale)+3);
	}
	p->drawLine(50+(maxfold)*xscale-3,650-(linefoldVSbins[maxfold-1]*yscale),50+(maxfold)*xscale+3,650-(linefoldVSbins[maxfold-1]*yscale));
	p->drawLine(50+(maxfold)*xscale,650-(linefoldVSbins[maxfold-1]*yscale)-3,50+(maxfold)*xscale,650-(linefoldVSbins[maxfold-1]*yscale)+3);
}

void analyzebinview::drawlineOffsetVSFoldBins(QPainter *p,int ycoord)//炮检距-覆盖次数-面元数量
{
	QPoint point=kedufun(maxoffset);
	int offset=point.x()*point.y();
	int scaleoffset=offset/4;//偏移距4等份分组
	int max=0;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<maxfold;j++)
		{
			if(max<lineoffsetVSfoldbins[i][j]) max=lineoffsetVSfoldbins[i][j];
		}
	}
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,650,50);
	p->drawLine(0,650,650,650);
	p->drawLine(50,0,50,650);
	if(ycoord==0) fold=kedufun(maxfold);
	else fold=kedufun(ycoord);
	QString str;
	float xscale=(float)(650-50)/offset;
	QPen pen(Qt::black,0);//定义画笔
	p->setPen(pen);//选入画笔
	//画色标条
	QPoint color=kedufun(max);
	int colorvalue=color.x()*color.y()/15;
	if (colorvalue <= 0) colorvalue = 1;
	float colorvalue2=(float)max/15;
	if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
	float yyscale=(float)600/15;
	p->drawRect(750,50,20,600);
	int scalefold=fold.x()*fold.y()/5;
	float yscale=(float)(650-50)/(fold.x()*fold.y());
	float fold2=(float)maxfold/30;//将纵坐标分成最多30等分
	int value=0;
	QColor windowcolor;
	if(fold.x()*fold.y()<=30)
	{
		for(int i=1;i<=15;i++)//画色标条
		{
			windowcolor=datatocolor(i-1);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);
			p->drawRect(750,650-i*yyscale,20,yyscale);
			QString str;
			int hh=i*colorvalue2;
			if(i==15) str.sprintf("%d",max);
			else str.sprintf("%d",hh);
			p->drawText(770,650-i*yyscale,str);
		}
		p->drawText(770,650,"1");	//画色标条
		for(int i=0;i<4;i++)//填充颜色
		{
			for(int j=0;j<maxfold;j++)
			{
				p->save();
				windowcolor=datatocolor(lineoffsetVSfoldbins[i][j]/colorvalue);
				QBrush brush;
				brush.setColor(windowcolor);
				p->setBrush(windowcolor);
				QPen pen(windowcolor,2);
				p->setPen(pen);
				if(lineoffsetVSfoldbins[i][j]!=0) p->drawRect(50+i*scaleoffset*xscale+1,650-j*yscale-yscale+1,scaleoffset*xscale,yscale);
				str.sprintf("%d",lineoffsetVSfoldbins[i][j]);
				p->restore();
				p->drawText(50+(i+0.5)*scaleoffset*xscale,650-(j+1-0.5)*yscale,str);
			}
		}
	}
	else
	{
		lineoffsetVSfoldbins2=new int*[4];
		for(int i=0;i<4;i++)
		{
			lineoffsetVSfoldbins2[i]=new int[30];
			for(int j=0;j<30;j++)
				lineoffsetVSfoldbins2[i][j]=0;
		}
		max=0;
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<maxfold;j=j++)
			{
				float temp=(float)j/fold2;
				int temp2=(int)temp;
				lineoffsetVSfoldbins2[i][temp2]=lineoffsetVSfoldbins2[i][temp2]+lineoffsetVSfoldbins[i][j];
				if(max<lineoffsetVSfoldbins2[i][temp2]) max=lineoffsetVSfoldbins2[i][temp2];
			}
		}
		//画色标条
		QPoint color=kedufun(max);
		int colorvalue=color.x()*color.y()/15;
		if (colorvalue <= 0) colorvalue = 1;
		float colorvalue2=(float)max/15;
		if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
		float yyscale=(float)600/15;
		p->drawRect(750,50,20,600);
		for(int i=1;i<=15;i++)//画色标条
		{
			windowcolor=datatocolor(i-1);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);
			QPen pen(Qt::black,1);
			p->setPen(pen);
			p->drawRect(750,650-i*yyscale,20,yyscale);
			QString str;
			int hh=i*colorvalue2;
			if(i==15) str.sprintf("%d",max);
			else str.sprintf("%d",hh);
			p->drawText(770,650-i*yyscale,str);
		}
		p->drawText(770,650,"1");
		//画色标条
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<30;j++)
			{
				p->save();
				windowcolor=datatocolor(lineoffsetVSfoldbins2[i][j]/colorvalue);
				QBrush brush;
				brush.setColor(windowcolor);
				p->setBrush(windowcolor);
				QPen pen(windowcolor,2);
				p->setPen(pen);
				if(lineoffsetVSfoldbins2[i][j]!=0) p->drawRect(50+i*scaleoffset*xscale+1,650-j*fold2*yscale-yscale*fold2+1,scaleoffset*xscale,yscale*fold2);
				str.sprintf("%d",lineoffsetVSfoldbins2[i][j]);
				p->restore();
				p->drawText(50+(i+0.5)*scaleoffset*xscale,650-(j+1-0.5)*fold2*yscale,str);
			}
		}
	}

	for(int i=0;i<offset;i++)//x方向刻度坐标
	{
		if(i%scaleoffset==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			str.sprintf("%d",i);
			p->drawText(45+i*xscale,39,str);
			if(i!=0) p->drawText(45+i*xscale,669,str);
			p->drawLine(50+i*xscale,50,50+i*xscale,40);
			p->drawLine(50+i*xscale,650,50+i*xscale,660);
			p->drawLine(50+i*xscale,50,50+i*xscale,650);
		}
		if(5*i%scaleoffset==0) 
		{
			p->drawLine(50+i*xscale,50,50+i*xscale,45);
			p->drawLine(50+i*xscale,650,50+i*xscale,655);
		}
	}
	p->drawLine(650,50,650,40);
	p->drawLine(650,650,650,660);
	p->drawLine(650,50,650,650);
	str.sprintf("%d",point.x()*point.y());
	p->drawText(650,39,str);
	p->drawText(650,669,str);

	if(fold.x()*fold.y()<=30)
	{
		for(int i=0;i<fold.x()*fold.y();i++)//y方向刻度
		{	
			if(i%scalefold==0)
			{
				p->drawLine(50,650-i*yscale,40,650-i*yscale);
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,1);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i);
				p->drawText(20,650-i*yscale,str);
			}
			if(i%fold.x()==0)
			{
				p->drawLine(50,650-i*yscale,45,650-i*yscale);
				p->drawLine(50,650-i*yscale,650,650-i*yscale);
			}
		}
	}
	else
	{
		for(int i=0;i<=30;i++)
		{
			if(i%5==0)	
			{
				p->drawLine(50,650-i*yscale*fold2,40,650-i*yscale*fold2);
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,1);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				int temp=i*fold2;
				str.sprintf("%d",temp);
				p->drawText(20,650-i*yscale*fold2,str);
			}
			if(i%1==0)
			{
				p->drawLine(50,650-i*yscale*fold2,45,650-i*yscale*fold2);
				p->drawLine(50,650-i*yscale*fold2,650,650-i*yscale*fold2);
			}
		}
	}
	p->drawText(450,20,"OFFSET");
	p->drawText(10,335,"FOLD");
	QPen pen3(Qt::black,2);//定义画笔
	p->setPen(pen3);//选入画笔
	p->drawLine(0,50,650,50);
	p->drawLine(0,650,650,650);
	p->drawLine(50,0,50,650);

}

void analyzebinview::drawlineOffsetVSTraces(QPainter *p,int xcoord,int ycoord)//线面元属性分析
{
	int maxtraces=0;//某一炮检距对应的最多的炮检对
	if(ycoord==0)
	{
		for(int i=0;i<maxoffset;i++)
		{
			if(maxtraces<lineoffsetVStraces[i]) maxtraces=lineoffsetVStraces[i];
		}
	}
	else maxtraces=ycoord;
	if(xcoord==0) offset=kedufun(maxoffset);
	else offset=kedufun(xcoord);
	if(maxtraces<=10) 
	{
		traces.setY(maxtraces);
		traces.setX(1);
	}
	else traces=kedufun(maxtraces);//寻找一个合适的刻度

	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);
	float xscale=(float)(950-50)/(offset.x()*offset.y());
	for(int i=0;i<=offset.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*offset.x());
			p->drawText(45+i*xscale*offset.x(),39,str);
			if(i!=0) p->drawText(45+i*xscale*offset.x(),669,str);
			p->drawLine(50+i*xscale*offset.x(),50,50+i*xscale*offset.x(),40);
			p->drawLine(50+i*xscale*offset.x(),650,50+i*xscale*offset.x(),660);
		}
		else 
		{
			p->drawLine(50+i*xscale*offset.x(),50,50+i*xscale*offset.x(),45);
			p->drawLine(50+i*xscale*offset.x(),650,50+i*xscale*offset.x(),655);
		}
	}
	float yscale=(float)(650-50)/(traces.x()*traces.y());
	for(int i=0;i<traces.y();i++)//y方向刻度
	{	
		if(maxtraces<=10)
		{
			if(i%1==0)	
			{
				p->drawLine(50,650-i*yscale*traces.x(),40,650-i*yscale*traces.x());
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,0);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i*traces.x());
				p->drawText(20,650-i*yscale*traces.x(),str);
			}
			else
			{
				p->drawLine(50,650-i*yscale*traces.x(),45,650-i*yscale*traces.x());
			}
		}
		else
		{
			if(i%10==0)	
			{
				p->drawLine(50,650-i*yscale*traces.x(),40,650-i*yscale*traces.x());
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,0);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i*traces.x());
				p->drawText(20,650-i*yscale*traces.x(),str);
			}
			else
			{
				p->drawLine(50,650-i*yscale*traces.x(),45,650-i*yscale*traces.x());
			}
		}
	}
	p->drawText(450,20,"OFFSET");
	p->drawText(10,335,"TRACES");
	int t=2;//画线的间隔
	for(int i=0;i<maxoffset-t;i=i+t)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+i*xscale,650-(lineoffsetVStraces[i]*yscale),50+(i+t)*xscale,650-(lineoffsetVStraces[i+t]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+i*xscale-3,650-(lineoffsetVStraces[i]*yscale),50+i*xscale+3,650-(lineoffsetVStraces[i]*yscale));
		p->drawLine(50+i*xscale,650-(lineoffsetVStraces[i]*yscale)-3,50+i*xscale,650-(lineoffsetVStraces[i]*yscale)+3);
	}
}

void analyzebinview::drawlineAzimuthVSTraces(QPainter *p,int ycoord)
{
	int maxtraces=0;//某一炮检距对应的最多的炮检对
	if(ycoord==0)
	{
		for(int i=0;i<360;i++)
		{
			if(maxtraces<lineazimuthVStraces[i]) maxtraces=lineazimuthVStraces[i];
		}
	}
	else maxtraces=ycoord;
	azimuthtraces=kedufun(maxtraces);//寻找一个合适的刻度
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);
	float xscale=(float)(950-50)/360;
	for(int i=0;i<360;i++)//x方向刻度坐标
	{
		if(i%50==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i);
			p->drawText(45+i*xscale,39,str);
			if(i!=0) p->drawText(45+i*xscale,669,str);
			p->drawLine(50+i*xscale,50,50+i*xscale,40);
			p->drawLine(50+i*xscale,650,50+i*xscale,660);
		}
		if(i%10==0) 
		{
			p->drawLine(50+i*xscale,50,50+i*xscale,45);
			p->drawLine(50+i*xscale,650,50+i*xscale,655);
		}
	}
	float yscale=(float)(650-50)/(azimuthtraces.x()*azimuthtraces.y());
	for(int i=0;i<azimuthtraces.y();i++)//y方向刻度
	{		
		if(i%10==0)	
		{
			p->drawLine(50,650-i*yscale*azimuthtraces.x(),40,650-i*yscale*azimuthtraces.x());
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*azimuthtraces.x());
			p->drawText(20,650-i*yscale*azimuthtraces.x(),str);
		}
		else
		{
			p->drawLine(50,650-i*yscale*azimuthtraces.x(),45,650-i*yscale*azimuthtraces.x());
		}
	}
	p->drawText(450,20,"AZIMUTH");
	p->drawText(10,335,"TRACES");
	int t=2;//画线的间隔
	for(int i=0;i<360-t;i=i+t)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+i*xscale,650-(lineazimuthVStraces[i]*yscale),50+(i+t)*xscale,650-(lineazimuthVStraces[i+t]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+i*xscale-3,650-(lineazimuthVStraces[i]*yscale),50+i*xscale+3,650-(lineazimuthVStraces[i]*yscale));
		p->drawLine(50+i*xscale,650-(lineazimuthVStraces[i]*yscale)-3,50+i*xscale,650-(lineazimuthVStraces[i]*yscale)+3);
	}
}

void analyzebinview::drawlineRoseVSTraces(QPainter *p)
{
	roseoffset=kedufun(maxoffset);
	int roseoffsetscale=roseoffset.x()*roseoffset.y()/4;
	//	float roseoffsetscale=(float)maxoffset/4;
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	float t=1.1;//图形在屏幕中的大小比例因子
	p->drawLine(0,50,650*t,50);
	p->drawLine(0,650,650*t,650);
	p->drawLine(50,0,50,650);	
	float xscale=(float)(650-50)/(2*roseoffset.x()*roseoffset.y()*t);
	for(int i=0;i<=roseoffset.y()*t;i++)//x,y方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,1);//定义画笔
			p->setPen(pen);//选入画笔
			QString str,str1;
			str.sprintf("%d",i*roseoffset.x());
			str1.sprintf("%d",-i*roseoffset.x());
			p->drawText(350+i*xscale*roseoffset.x(),39,str);
			p->drawText(350-i*xscale*roseoffset.x(),39,str1);//x 负半轴
			p->drawText(10,350+i*xscale*roseoffset.x(),str1);//y负半轴
			p->drawText(10,350-i*xscale*roseoffset.x(),str);//y
			p->drawLine(350+i*xscale*roseoffset.x(),50,350+i*xscale*roseoffset.x(),40);
			p->drawLine(350+i*xscale*roseoffset.x(),650,350+i*xscale*roseoffset.x(),660);
			p->drawLine(350-i*xscale*roseoffset.x(),50,350-i*xscale*roseoffset.x(),40);
			p->drawLine(350-i*xscale*roseoffset.x(),650,350-i*xscale*roseoffset.x(),660);
			p->drawLine(50,350-i*xscale*roseoffset.x(),40,350-i*xscale*roseoffset.x());
			p->drawLine(50,350+i*xscale*roseoffset.x(),40,350+i*xscale*roseoffset.x());
		}
		else 
		{
			p->drawLine(350+i*xscale*roseoffset.x(),50,350+i*xscale*roseoffset.x(),45);
			p->drawLine(350+i*xscale*roseoffset.x(),650,350+i*xscale*roseoffset.x(),655);
			p->drawLine(350-i*xscale*roseoffset.x(),50,350-i*xscale*roseoffset.x(),45);
			p->drawLine(350-i*xscale*roseoffset.x(),650,350-i*xscale*roseoffset.x(),655);
			p->drawLine(50,350-i*xscale*roseoffset.x(),45,350-i*xscale*roseoffset.x());
			p->drawLine(50,350+i*xscale*roseoffset.x(),45,350+i*xscale*roseoffset.x());
		}
	}
	int maxrosetraces=0;
	for(int i=0;i<4;i++)//寻找最大炮检对
	{
		for(int j=0;j<36;j++)
		{
			if(maxrosetraces<lineroseVStraces[i][j]) maxrosetraces=lineroseVStraces[i][j];
		}
	}

	p->save();
	//	float value1=(float)255/maxrosetraces;
	QColor windowcolor;
	QPoint color=kedufun(maxrosetraces);
	int colorvalue=color.x()*color.y()/15;
	if (colorvalue <= 0) colorvalue = 1;
	float colorvalue2=(float)maxrosetraces/15;
	if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
	float yscale=(float)600/15;
	p->drawRect(750,50,20,600);
	for(int i=1;i<=15;i++)//画色标条
	{
		windowcolor=datatocolor(i-1);
		QBrush brush;
		brush.setColor(windowcolor);
		p->setBrush(windowcolor);
		QPen pen(Qt::black,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawRect(750,650-i*yscale,20,yscale);
		QString str;
		int hh=i*colorvalue2;
		if(i==15) str.sprintf("%d",maxrosetraces);
		else str.sprintf("%d",hh);
		p->drawText(770,650-i*yscale,str);
	}
	p->drawText(770,650,"1");
	//画色标条

	for(int i=3;i>=0;i--)//画填充颜色
	{
		for(int j=0;j<36;j++)
		{		
			windowcolor=datatocolor(lineroseVStraces[i][j]/colorvalue2);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);//定义画刷颜色
			QPen pen(windowcolor,0);
			p->setPen(pen);	
			if(lineroseVStraces[i][j]!=0) p->drawPie(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale+1,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale,(90-j*10)*16,-10*16);
			if(i<3)
			{
				if(lineroseVStraces[i][j]==0&&lineroseVStraces[i+1][j]!=0) 
				{
					QColor windowcolor1(255,255,255);
					QBrush brush1;
					brush1.setColor(windowcolor1);
					p->setBrush(windowcolor1);
					QPen pen1(windowcolor1,0);
					p->setPen(pen1);
					p->drawPie(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale+1,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale,(90-j*10)*16,-10*16);
				}
			}		
		}
	}
	p->restore();
	float x=0,y=0;
	for(int i=0;i<4;i++)//画圆形及放射线
	{
		p->drawEllipse(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale+1,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale);
		for(int j=0;j<36;j++)
		{
			y=-4*roseoffsetscale*xscale*cos(j*10*3.14159265/180)+350;
			x=4*roseoffsetscale*xscale*sin(j*10*3.14159265/180)+350;
			p->drawLine(350,350,x,y);
		}
	}

	for(int i=0;i<4;i++)//画度数数字,为了让字体旋转，所以分开画
	{
		for(int j=0;j<36;j++)
		{	
			y=-4*roseoffsetscale*xscale-5;
			x=0;
			p->save();					
			QFont font("Timers",10);
			p->setFont(font);
			QString str;
			str.sprintf("%d",j*10);
			p->rotate(10*j);
			p->setWindow(-350,-350,960,670);			
			p->drawText(x,y,str);
			p->restore();
		}
	}
}

void analyzebinview::drawworkareaFoldVSBIns(int xcoord,int ycoord)//整个工区覆盖次数-面元个数分析
{
	int Viewport_X_Width=960;
	int Viewport_Y_Height=700;
	//resizeContents(960,670);
	//mpBinImage->resize(960,670);//定义绘图区域
	// 增大了视口的纵向高度 [Liu Wei 3/28/2023]
	resizeContents(Viewport_X_Width,Viewport_Y_Height);
	mpBinImage->resize(Viewport_X_Width,Viewport_Y_Height);//定义绘图区域

	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter *p=new QPainter(mpBinImage);//定义绘图指针

	p->save();
	QFont font("Timers",20);
	p->setFont(font);
	//	p->drawText(700,20,"analyze work area");
	p->restore();

	int maxbins=0;
	if(ycoord==0)
	{
		for(int i=0;i<maxfold;i++)
		{
			if(maxbins<foldVSbins[i]) maxbins=foldVSbins[i];
		}
	}
	else maxbins=ycoord;
	binnumber=kedufun(maxbins);//
	if(xcoord==0) binfold=kedufun(maxfold);
	else binfold=kedufun(xcoord);
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);

	float xscale=(float)(950-50)/(binfold.x()*binfold.y());
	for(int i=0;i<=binfold.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*binfold.x());
			p->drawText(45+i*xscale*binfold.x(),39,str);
			if(i!=0) p->drawText(45+i*xscale*binfold.x(),669,str);
			p->drawLine(50+i*xscale*binfold.x(),50,50+i*xscale*binfold.x(),40);
			p->drawLine(50+i*xscale*binfold.x(),650,50+i*xscale*binfold.x(),660);
		}
		else 
		{
			p->drawLine(50+i*xscale*binfold.x(),50,50+i*xscale*binfold.x(),45);
			p->drawLine(50+i*xscale*binfold.x(),650,50+i*xscale*binfold.x(),655);
		}
	}
	float yscale=(float)(650-50)/(binnumber.x()*binnumber.y());
	for(int i=0;i<binnumber.y();i++)
	{
		if(i%10==0)	
		{
			p->drawLine(50,650-i*yscale*binnumber.x(),40,650-i*yscale*binnumber.x());
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*binnumber.x());
			p->drawText(20,650-i*yscale*binnumber.x(),str);
		}
		else
		{
			p->drawLine(50,650-i*yscale*binnumber.x(),45,650-i*yscale*binnumber.x());
		}
	}
	//p->drawText(450,20,"FOLD");
	//p->drawText(10,335,"BINS");
	//  [Liu Wei 3/28/2023]
	p->drawText(450,650+35,"FOLD");
	//p->drawText(5,335,"BINS");
	p->save();
	p->rotate(-90);// 逆时针旋转90度 [Liu Wei 4/6/2023]
	p->drawText(-350,11,"BINS");
	p->restore();

	//  [Liu Wei 3/28/2023]
	p->drawText(75,20,mTextAmizth+"       "+mTextOffset);

	//	p->drawText(100,20,QString::fromLocal8Bit("方位角:")+QString::number(mShowAzimuth*mBaseDivAmizth)+"—"+QString::number((mShowAzimuth+1)*mBaseDivAmizth)+"       "+
	//		QString::fromLocal8Bit("炮检距:")+QString::number(mShowOffset*mBaseDivOffset)+"—"+QString::number((mShowOffset+1)*mBaseDivOffset));

	for(int i=0;i<maxfold-1;i++)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+(i+1)*xscale,650-(foldVSbins[i]*yscale),50+(i+2)*xscale,650-(foldVSbins[i+1]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+(i+1)*xscale-3,650-(foldVSbins[i]*yscale),50+(i+1)*xscale+3,650-(foldVSbins[i]*yscale));
		p->drawLine(50+(i+1)*xscale,650-(foldVSbins[i]*yscale)-3,50+(i+1)*xscale,650-(foldVSbins[i]*yscale)+3);
	}
	p->drawLine(50+(maxfold)*xscale-3,650-(foldVSbins[maxfold-1]*yscale),50+(maxfold)*xscale+3,650-(foldVSbins[maxfold-1]*yscale));
	p->drawLine(50+(maxfold)*xscale,650-(foldVSbins[maxfold-1]*yscale)-3,50+(maxfold)*xscale,650-(foldVSbins[maxfold-1]*yscale)+3);
	repaintContents();
	delete p;
}

void analyzebinview::drawworkareaOffsetVSTraces(int xcoord,int ycoord)//整个工区炮检距-炮检对
{
	int Viewport_X_Width=980;
	int Viewport_Y_Height=700;
	//resizeContents(980,670);
	//mpBinImage->resize(980,670);//定义绘图区域
	// 增大了视口的纵向高度 [Liu Wei 3/28/2023]
	resizeContents(Viewport_X_Width,Viewport_Y_Height);
	mpBinImage->resize(Viewport_X_Width,Viewport_Y_Height);//定义绘图区域

	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter *p=new QPainter(mpBinImage);//定义绘图指针
	p->save();
	QFont font("Timers",20);
	p->setFont(font);
	//	p->drawText(700,20,"analyze work area");
	p->restore();

	int maxtraces=0;//某一炮检距对应的最多的炮检对
	if(ycoord==0)
	{
		for(int i=0;i<maxoffset;i++)
		{
			if(maxtraces<offsetVStraces[i]) maxtraces=offsetVStraces[i];
		}
	}
	else maxtraces=ycoord;
	traces=kedufun(maxtraces);//寻找一个合适的刻度
	if(xcoord==0) offset=kedufun(maxoffset);
	else offset=kedufun(xcoord);
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);
	float xscale=(float)(950-50)/(offset.x()*offset.y());
	for(int i=0;i<=offset.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*offset.x());
			p->drawText(45+i*xscale*offset.x(),39,str);
			if(i!=0) p->drawText(45+i*xscale*offset.x(),669,str);
			p->drawLine(50+i*xscale*offset.x(),50,50+i*xscale*offset.x(),40);
			p->drawLine(50+i*xscale*offset.x(),650,50+i*xscale*offset.x(),660);
		}
		else 
		{
			p->drawLine(50+i*xscale*offset.x(),50,50+i*xscale*offset.x(),45);
			p->drawLine(50+i*xscale*offset.x(),650,50+i*xscale*offset.x(),655);
		}
	}
	float yscale=(float)(650-50)/(traces.x()*traces.y());
	for(int i=0;i<traces.y();i++)//y方向刻度
	{		
		if(i%10==0)	
		{
			p->drawLine(50,650-i*yscale*traces.x(),40,650-i*yscale*traces.x());
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*traces.x());
			p->drawText(20,650-i*yscale*traces.x(),str);
		}
		else
		{
			p->drawLine(50,650-i*yscale*traces.x(),45,650-i*yscale*traces.x());
		}
	}
	//p->drawText(450,20,"OFFSET");
	//p->drawText(10,335,"TRACES");
	//  [Liu Wei 3/28/2023]
	QFont _font("Timers",10);
	p->setFont(_font);
	QPen _pen(Qt::black,1);//定义画笔
	p->setPen(_pen);//选入画笔
	p->drawText(450,650+35,"OFFSET");
	//p->drawText(5,335,"TRACES");
	p->save();
	p->rotate(-90);
	p->drawText(-350,11,"TRACES");
	p->restore();
	//  [Liu Wei 3/28/2023]
	p->drawText(75,20,mTextAmizth+"       "+mTextOffset);

	int t=2;//画线的间隔
	for(int i=0;i<maxoffset-t;i=i+t)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+i*xscale,650-(offsetVStraces[i]*yscale),50+(i+t)*xscale,650-(offsetVStraces[i+t]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+i*xscale-3,650-(offsetVStraces[i]*yscale),50+i*xscale+3,650-(offsetVStraces[i]*yscale));
		p->drawLine(50+i*xscale,650-(offsetVStraces[i]*yscale)-3,50+i*xscale,650-(offsetVStraces[i]*yscale)+3);
	}
	repaintContents();
	delete p;
}

void analyzebinview::drawworkareaAzimuthVSTraces(int ycoord)//整个工区面元属性分析,方位角-炮检对
{
	int Viewport_X_Width=960;
	int Viewport_Y_Height=700;
	//resizeContents(960,670);
	//mpBinImage->resize(960,670);//定义绘图区域
	// 增大了视口的纵向高度 [Liu Wei 3/28/2023]
	resizeContents(Viewport_X_Width,Viewport_Y_Height);
	mpBinImage->resize(Viewport_X_Width,Viewport_Y_Height);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter *p=new QPainter(mpBinImage);//定义绘图指针
	p->save();
	QFont font("Timers",20);
	p->setFont(font);
	//	p->drawText(700,20,"analyze work area");
	p->restore();

	int maxtraces=0;//某一炮检距对应的最多的炮检对
	if(ycoord==0)
	{
		for(int i=0;i<360;i++)
		{
			if(maxtraces<azimuthVStraces[i]) maxtraces=azimuthVStraces[i];
		}
	}
	else maxtraces=ycoord;
	azimuthtraces=kedufun(maxtraces);//寻找一个合适的刻度
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,950,50);
	p->drawLine(0,650,950,650);
	p->drawLine(50,0,50,650);
	float xscale=(float)(950-50)/360;
	for(int i=0;i<360;i++)//x方向刻度坐标
	{
		if(i%50==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i);
			p->drawText(45+i*xscale,39,str);
			if(i!=0) p->drawText(45+i*xscale,669,str);
			p->drawLine(50+i*xscale,50,50+i*xscale,40);
			p->drawLine(50+i*xscale,650,50+i*xscale,660);
		}
		if(i%10==0) 
		{
			p->drawLine(50+i*xscale,50,50+i*xscale,45);
			p->drawLine(50+i*xscale,650,50+i*xscale,655);
		}
	}
	float yscale=(float)(650-50)/(azimuthtraces.x()*azimuthtraces.y());
	for(int i=0;i<azimuthtraces.y();i++)//y方向刻度
	{		
		if(i%10==0)	
		{
			p->drawLine(50,650-i*yscale*azimuthtraces.x(),40,650-i*yscale*azimuthtraces.x());
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*azimuthtraces.x());
			p->drawText(20,650-i*yscale*azimuthtraces.x(),str);
		}
		else
		{
			p->drawLine(50,650-i*yscale*azimuthtraces.x(),45,650-i*yscale*azimuthtraces.x());
		}
	}
	//p->drawText(450,20,"AZIMUTH");
	//p->drawText(10,335,"TRACES");
	//  [Liu Wei 3/28/2023]
	QFont _font("Timers",10);
	p->setFont(_font);
	QPen _pen(Qt::black,1);//定义画笔
	p->setPen(_pen);//选入画笔
	p->drawText(450,650+35,"AZIMUTH");
	//p->drawText(5,335,"TRACES");
	p->save();
	p->rotate(-90);
	p->drawText(-350,11,"TRACES");
	p->restore();
	//  [Liu Wei 3/28/2023]
	p->drawText(75,20,mTextAmizth+"       "+mTextOffset);


	int t=2;//画线的间隔
	for(int i=0;i<360-t;i=i+t)
	{
		QPen pen2(Qt::blue,0);//定义画笔
		p->setPen(pen2);//选入画笔
		p->drawLine(50+i*xscale,650-(azimuthVStraces[i]*yscale),50+(i+t)*xscale,650-(azimuthVStraces[i+t]*yscale));
		QPen pen(Qt::red,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawLine(50+i*xscale-3,650-(azimuthVStraces[i]*yscale),50+i*xscale+3,650-(azimuthVStraces[i]*yscale));
		p->drawLine(50+i*xscale,650-(azimuthVStraces[i]*yscale)-3,50+i*xscale,650-(azimuthVStraces[i]*yscale)+3);
	}
	repaintContents();
	delete p;
}

void analyzebinview::drawworkareaRoseVSTraces(int lx,int ly,int maxoffset1)//画玫瑰花图-炮检对
{
	mRoselx=lx;mRosetly=ly;
	int yinv=360/ly;
	resizeContents(960,670);
	mpBinImage->resize(960,670);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter *p=new QPainter(mpBinImage);//定义绘图指针
	p->save();
	QFont font("Timers",20);
	p->setFont(font);
	//	p->drawText(700,20,"analyze work area");
	p->restore();

	if(maxoffset1<=0) return;
	roseoffset=kedufun(maxoffset1);
	int roseoffsetscale=roseoffset.x()*roseoffset.y()/lx;
	//	float roseoffsetscale=(float)maxoffset/4;
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	float t=1.1;//图形在屏幕中的大小比例因子
	p->drawLine(0,50,650*t,50);
	p->drawLine(0,650,650*t,650);
	p->drawLine(50,0,50,650);	
	float xscale=(float)(650-50)/(2*roseoffset.x()*roseoffset.y()*t);
	for(int i=0;i<=roseoffset.y()*t;i++)//x,y方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			QString str,str1;
			str.sprintf("%d",i*roseoffset.x());
			str1.sprintf("%d",-i*roseoffset.x());
			p->drawText(350+i*xscale*roseoffset.x(),39,str);
			p->drawText(350-i*xscale*roseoffset.x(),39,str1);//x 负半轴
			p->drawText(10,350+i*xscale*roseoffset.x(),str1);//y负半轴
			p->drawText(10,350-i*xscale*roseoffset.x(),str);//y
			p->drawLine(350+i*xscale*roseoffset.x(),50,350+i*xscale*roseoffset.x(),40);
			p->drawLine(350+i*xscale*roseoffset.x(),650,350+i*xscale*roseoffset.x(),660);
			p->drawLine(350-i*xscale*roseoffset.x(),50,350-i*xscale*roseoffset.x(),40);
			p->drawLine(350-i*xscale*roseoffset.x(),650,350-i*xscale*roseoffset.x(),660);
			p->drawLine(50,350-i*xscale*roseoffset.x(),40,350-i*xscale*roseoffset.x());
			p->drawLine(50,350+i*xscale*roseoffset.x(),40,350+i*xscale*roseoffset.x());
		}
		else 
		{
			p->drawLine(350+i*xscale*roseoffset.x(),50,350+i*xscale*roseoffset.x(),45);
			p->drawLine(350+i*xscale*roseoffset.x(),650,350+i*xscale*roseoffset.x(),655);
			p->drawLine(350-i*xscale*roseoffset.x(),50,350-i*xscale*roseoffset.x(),45);
			p->drawLine(350-i*xscale*roseoffset.x(),650,350-i*xscale*roseoffset.x(),655);
			p->drawLine(50,350-i*xscale*roseoffset.x(),45,350-i*xscale*roseoffset.x());
			p->drawLine(50,350+i*xscale*roseoffset.x(),45,350+i*xscale*roseoffset.x());
		}
	}
	int maxrosetraces=0;
	for(int i=0;i<lx;i++)//寻找最大炮检对
	{
		for(int j=0;j<ly;j++)
		{
			if(maxrosetraces<roseVStraces[i][j]) maxrosetraces=roseVStraces[i][j];
		}
	}


	p->save();

	p->drawText(75,20,mTextAmizth+"       "+mTextOffset);


	//	float value1=(float)255/maxrosetraces;
	QColor windowcolor;
	QPoint color=kedufun(maxrosetraces);
	int colorvalue=color.x()*color.y()/15;
	if (colorvalue <= 0) colorvalue = 1;
	float colorvalue2=(float)maxrosetraces/15;
	if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
	float yscale=(float)600/15;
	p->drawRect(750,50,20,600);
	for(int i=1;i<=15;i++)//画色标条
	{
		windowcolor=datatocolor(i-1);
		QBrush brush;
		brush.setColor(windowcolor);
		p->setBrush(windowcolor);
		QPen pen(Qt::black,0);//定义画笔
		p->setPen(pen);//选入画笔
		p->drawRect(750,650-i*yscale,20,yscale);
		QString str;
		int hh=i*colorvalue2;
		if(i==15) str.sprintf("%d",maxrosetraces);
		else str.sprintf("%d",hh);
		p->drawText(770,650-i*yscale,str);
	}
	p->drawText(770,650,"1");
	//画色标条

	for(int i=lx-1;i>=0;i--)//画填充颜色
	{
		for(int j=0;j<ly;j++)
		{	
			if(roseVStraces[i][j]<=100) continue;
			windowcolor=datatocolor(roseVStraces[i][j]/colorvalue2);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);//定义画刷颜色
			QPen pen(windowcolor,0);
			p->setPen(pen);
			if(roseVStraces[i][j]!=0) p->drawPie(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale+1,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale-1,(90-j*yinv)*16,-yinv*16);
			if(i<1)
			{
				if(roseVStraces[i][j]==0&&roseVStraces[i+1][j]!=0) 
				{
					QColor windowcolor1(255,255,255);
					QBrush brush1;
					brush1.setColor(windowcolor1);
					p->setBrush(windowcolor1);
					QPen pen1(windowcolor1,0);
					p->setPen(pen1);
					p->drawPie(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale+1,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale-1,(90-j*yinv)*16,-yinv*16);
				}
			}
		}
	}
	p->restore();
	float x=0,y=0;
	for(int i=0;i<lx;i++)//画圆形及放射线
	{
		p->drawEllipse(350-(i+1)*xscale*roseoffsetscale+1,350-(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale,2*(i+1)*xscale*roseoffsetscale);
		for(int j=0;j<ly;j++)
		{
			//			y=-4*roseoffsetscale*xscale*cos(j*10*3.14159265/180)+350;
			//			x=4*roseoffsetscale*xscale*sin(j*10*3.14159265/180)+350;
			y=-lx*roseoffsetscale*xscale*cos(j*yinv*3.14159265/180)+350;
			x=lx*roseoffsetscale*xscale*sin(j*yinv*3.14159265/180)+350;
			p->drawLine(350,350,x,y);
		}
	}
	for(int i=0;i<lx;i++)//画度数数字,为了让字体旋转，所以分开画
	{
		for(int j=0;j<ly;j++)
		{	
			//			y=-4*roseoffsetscale*xscale-5;
			y=-lx*roseoffsetscale*xscale-5;
			x=0;
			p->save();					
			QFont font("Timers",10);
			p->setFont(font);
			QString str;
			str.sprintf("%d",j*yinv);
			p->rotate(yinv*j);
			p->setWindow(-350,-350,960,670);			
			p->drawText(x,y,str);
			p->restore();
		}
	}
	repaintContents();
	delete p;
}

void analyzebinview::drawworkareaOffsetVSFoldBins(int ycoord,int lx,int maxzhufold)//整个工区面元属性分析，炮检距-覆盖次数-面元数量
{
	int Viewport_X_Width=960;
	int Viewport_Y_Height=700;
	//resizeContents(960,670);
	//mpBinImage->resize(960,670);//定义绘图区域
	// 增大了视口的纵向高度 [Liu Wei 3/28/2023]
	resizeContents(Viewport_X_Width,Viewport_Y_Height);
	mpBinImage->resize(Viewport_X_Width,Viewport_Y_Height);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter *p=new QPainter(mpBinImage);//定义绘图指针
	p->save();
	QFont font("Timers",20);
	p->setFont(font);
	//	p->drawText(700,20,"analyze work area");
	p->restore();

	if(maxoffset<=0) return;
	QPoint point=kedufun(maxoffset);
	int offset=point.x()*point.y();
	int scaleoffset=offset/lx;//偏移距4等份分组
	int max=0;
	for(int i=0;i<lx;i++)
	{
		for(int j=0;j<maxzhufold;j++)
		{
			if(max<offsetVSfoldbins[i][j]) max=offsetVSfoldbins[i][j];
		}
	}
	if(max<=0) return;//2023.9.8
	QPen pen2(Qt::black,2);//定义画笔
	p->setPen(pen2);//选入画笔
	p->drawLine(0,50,650,50);
	p->drawLine(0,650,650,650);
	p->drawLine(50,0,50,650);
	//	if(ycoord==0) fold=kedufun(maxfold);
	//	else fold=kedufun(ycoord);
	if(ycoord!=0) maxzhufold=ycoord;
	fold=kedufun(maxzhufold);
	QString str;
	float xscale=(float)(650-50)/offset;
	QPen pen(Qt::black,0);//定义画笔
	p->setPen(pen);//选入画笔
	//画色标条
	QPoint color=kedufun(max);
	int colorvalue=color.x()*color.y()/15;
	if (colorvalue <= 0) colorvalue = 1;
	float colorvalue2=(float)max/15;
	if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
	float yyscale=(float)600/15;
	p->drawRect(750,50,20,600);

	int scalefold=fold.x()*fold.y()/5;
	if(fold.x()*fold.y()<5) scalefold=fold.x()*fold.y();

	float yscale=(float)(650-50)/(fold.x()*fold.y());
	//	int fold2=maxfold/30;//将纵坐标分成最多30等分
	float fold2=(float)maxzhufold/30;//将纵坐标分成最多30等分
	int value=0;
	QColor windowcolor;
	if(fold.x()*fold.y()<=30)
	{
		for(int i=1;i<=15;i++)//画色标条
		{
			windowcolor=datatocolor(i-1);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);
			p->drawRect(750,650-i*yyscale,20,yyscale);
			QString str;
			int hh=i*colorvalue2;
			if(i==15) str.sprintf("%d",max);
			else str.sprintf("%d",hh);
			p->drawText(770,650-i*yyscale,str);
		}
		p->drawText(770,650,"1");	//画色标条
		for(int i=0;i<lx;i++)//填充颜色
		{
			for(int j=0;j<maxzhufold;j++)
			{
				p->save();
				windowcolor=datatocolor(offsetVSfoldbins[i][j]/colorvalue);
				QBrush brush;
				brush.setColor(windowcolor);
				p->setBrush(windowcolor);
				QPen pen(windowcolor,2);
				p->setPen(pen);
				//				if(j!=0&&offsetVSfoldbins[i][j]!=0||j==0&&offsetVSfoldbins[i][j]==0) p->drawRect(50+i*scaleoffset*xscale,650-j*yscale-yscale,scaleoffset*xscale,yscale);
				if(offsetVSfoldbins[i][j]!=0) p->drawRect(50+i*scaleoffset*xscale+1,650-j*yscale-yscale+1,scaleoffset*xscale,yscale-1);
				str.sprintf("%d",offsetVSfoldbins[i][j]);
				p->restore();
				p->drawText(50+(i+0.5)*scaleoffset*xscale,650-(j+1-0.5)*yscale,str);
			}
		}
	}
	else
	{
		offsetVSfoldbins2=new int*[lx];
		for(int i=0;i<lx;i++)
		{
			offsetVSfoldbins2[i]=new int[30];
			for(int j=0;j<30;j++)
				offsetVSfoldbins2[i][j]=0;
		}
		max=0;
		for(int i=0;i<lx;i++)
		{
			for(int j=0;j<maxzhufold;j=j++)
			{
				float temp=(float)j/fold2;
				int temp2=(int)temp;
				offsetVSfoldbins2[i][temp2]=offsetVSfoldbins2[i][temp2]+offsetVSfoldbins[i][j];
				if(max<offsetVSfoldbins2[i][temp2]) max=offsetVSfoldbins2[i][temp2];
			}
		}
		//画色标条
		QPoint color=kedufun(max);
		int colorvalue=color.x()*color.y()/15;
		if (colorvalue <= 0) colorvalue = 1;
		float colorvalue2=(float)max/15;
		if (colorvalue2 <= 0.0f) colorvalue2 = 1.0f;
		float yyscale=(float)600/15;
		p->drawRect(750,50,20,600);
		for(int i=1;i<=15;i++)//画色标条
		{
			windowcolor=datatocolor(i-1);
			QBrush brush;
			brush.setColor(windowcolor);
			p->setBrush(windowcolor);
			QPen pen(Qt::black,1);
			p->setPen(pen);
			p->drawRect(750,650-i*yyscale,20,yyscale);
			QString str;
			int hh=i*colorvalue2;
			if(i==15) str.sprintf("%d",max);
			else str.sprintf("%d",hh);
			p->drawText(770,650-i*yyscale,str);
		}
		p->drawText(770,650,"1");
		//画色标条
		for(int i=0;i<lx;i++)
		{
			for(int j=0;j<30;j++)
			{
				p->save();
				windowcolor=datatocolor(offsetVSfoldbins2[i][j]/colorvalue);
				QBrush brush;
				brush.setColor(windowcolor);
				p->setBrush(windowcolor);
				QPen pen(windowcolor,2);
				p->setPen(pen);
				if(offsetVSfoldbins2[i][j]!=0) p->drawRect(50+i*scaleoffset*xscale+1,650-j*fold2*yscale-yscale*fold2+1,scaleoffset*xscale,yscale*fold2-1);
				str.sprintf("%d",offsetVSfoldbins2[i][j]);
				p->restore();
				p->drawText(50+(i+0.5)*scaleoffset*xscale,650-(j+1-0.5)*fold2*yscale,str);				
			}
		}
	}

	for(int i=0;i<offset;i++)//x方向刻度坐标
	{
		if(i%scaleoffset==0)
		{
			QFont font("Timers",10);
			p->setFont(font);
			QPen pen(Qt::black,0);//定义画笔
			p->setPen(pen);//选入画笔
			str.sprintf("%d",i);
			p->drawText(45+i*xscale,39,str);
			if(i!=0) p->drawText(45+i*xscale,669,str);
			p->drawLine(50+i*xscale,50,50+i*xscale,40);
			p->drawLine(50+i*xscale,650,50+i*xscale,660);
			p->drawLine(50+i*xscale,50,50+i*xscale,650);
		}
		if(5*i%scaleoffset==0) 
		{
			p->drawLine(50+i*xscale,50,50+i*xscale,45);
			p->drawLine(50+i*xscale,650,50+i*xscale,655);
		}
	}
	p->drawLine(650,50,650,40);
	p->drawLine(650,650,650,660);
	p->drawLine(650,50,650,650);
	str.sprintf("%d",point.x()*point.y());
	p->drawText(650,39,str);
	p->drawText(650,669,str);

	if(fold.x()*fold.y()<=30)
	{
		for(int i=0;i<fold.x()*fold.y();i++)//y方向刻度
		{	
			if(i%scalefold==0)
			{
				p->drawLine(50,650-i*yscale,40,650-i*yscale);
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,1);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				str.sprintf("%d",i);
				p->drawText(20,650-i*yscale,str);
			}
			if(i%fold.x()==0)
			{
				p->drawLine(50,650-i*yscale,45,650-i*yscale);
				p->drawLine(50,650-i*yscale,650,650-i*yscale);
			}
		}
	}
	else
	{
		for(int i=0;i<=30;i++)
		{
			if(i%5==0)	
			{
				p->drawLine(50,650-i*yscale*fold2,40,650-i*yscale*fold2);
				QFont font("Timers",10);
				p->setFont(font);
				QPen pen(Qt::black,1);//定义画笔
				p->setPen(pen);//选入画笔
				QString str;
				int temp=i*fold2;
				str.sprintf("%d",temp);
				p->drawText(20,650-i*yscale*fold2,str);
			}
			if(i%1==0)
			{
				p->drawLine(50,650-i*yscale*fold2,45,650-i*yscale*fold2);
				p->drawLine(50,650-i*yscale*fold2,650,650-i*yscale*fold2);
			}
		}
	}
	//p->drawText(450,20,"OFFSET");
	//p->drawText(10,335,"FOLD");
	//  [Liu Wei 3/28/2023]
	QFont _font("Monaco",10);//"Timers"是什么字体？
	QPen _pen(Qt::black,1);
	p->setFont(_font);
	p->setPen(_pen);
	p->drawText(330,650+35,"OFFSET");// 显示在底端
	p->save();
	p->rotate(-90);// 导致绘制失败？原因是坐标没有给对，正确坐标如下 [Liu Wei 3/28/2023]
	p->drawText(-350,11,"FOLD");
	p->restore();
	//  [Liu Wei 3/28/2023]
	p->drawText(75,20, mTextAmizth+"       "+mTextOffset);

	QPen pen3(Qt::black,2);//定义画笔
	p->setPen(pen3);//选入画笔
	p->drawLine(0,50,650,50);
	p->drawLine(0,650,650,650);
	p->drawLine(50,0,50,650);
	repaintContents();
	delete p;
}

void analyzebinview::drawworkareaBinfold(int **array,int xline,int yline)//整个工区覆盖次数
{// 该功能已经被取代，改进函数：void drawWorkAreaBinFold(int **array,int xbinnum,int ybinnum) [Liu Wei 3/29/2023]
	xscaleoffset=0.08;
	yscaleoffset=0.08;
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
		//		p.drawText(700,20,"analyze work area");
		p.restore();

		p.drawText(75,20,mTextAmizth+"       "+mTextOffset);

		int cx=viewx-50,cy=viewy-50;
		QColor windowcolor;	
		float temp2=(float)(maxfold-minfold)/15;
		if (temp2 <= 0.0f) temp2 = 1.0f;
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
				p.drawRect(50+i*xscaleoffset,cy-j*yscaleoffset-yscaleoffset,xscaleoffset+1,yscaleoffset+1);// 解决镜像问题需要参考这段代码 [Liu Wei 3/28/2023]
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
				//if(i!=0) p.drawText(45+i*xscaleoffset*xx.x(),cy+19,str);// 为什么要显示成不一致的情况？ [Liu Wei 3/28/2023]
				p.drawText(55+i*xscaleoffset*xx.x(),cy+19,str);// 保证上下坐标标签显示一致 [Liu Wei 3/28/2023]
				p.drawLine(50+i*xscaleoffset*xx.x(),50,50+i*xscaleoffset*xx.x(),40);
				p.drawLine(50+i*xscaleoffset*xx.x(),cy,50+i*xscaleoffset*xx.x(),cy+10);
				//p.drawText(viewx/2,20,"x number");
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
				p.rotate(-90);// 逆时针旋转90° [Liu Wei 3/28/2023]
				p.setWindow(0,0,viewx,viewy);
				p.drawText(-cy+i*yscaleoffset*yy.x(),25,str);
				//p.drawText(-cy+cy/2,15,"y number");
				p.restore();
				//	p.drawText(20,cy-i*yscaleoffset*yy.x(),str);
			}
			else
			{
				p.drawLine(50,cy-i*yscaleoffset*yy.x(),45,cy-i*yscaleoffset*yy.x());// 镜像的问题！ [Liu Wei 3/28/2023]
			}
		}
		// 自定义字体和画笔 [Liu Wei 3/28/2023]
		QFont _font("Timers",10);
		p.setFont(_font);
		QPen pen(Qt::black,1);//定义画笔
		p.setPen(pen);//选入画笔
		// 绘制x坐标轴标题 [Liu Wei 3/28/2023]
		QString xLabelText="X number";
		QFontMetrics xFontMetrics(p.font());
		int xFontWidthPixel=xFontMetrics.width(xLabelText);// 计算显示字符串的宽度
		p.drawText(cx/2-xFontWidthPixel/2,cy+35,xLabelText);// 将坐标标题显示在底端
		//  [Liu Wei 3/28/2023]
		p.save();
		p.rotate(-90);
		// 绘制y坐标轴标题 [Liu Wei 3/28/2023]
		QString yLabelText="Y number";
		QFontMetrics yFontMetrics(p.font());
		int yFontWidthPixel=yFontMetrics.width(yLabelText);// 计算显示字符串的宽度
		p.drawText(-cy+cy/2-yFontWidthPixel/2,15,yLabelText);
		//  [Liu Wei 3/28/2023]
		p.restore();
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

//画炮检距非均匀性系数
void analyzebinview::drawoffsetNonuniformfactor(float **array,int x,int y,float max,float min,float NonuniformBinx,float NonuniformBiny,QString FnameStr,float CSQFactor)//画炮检距分析结果
{
	if(max<=0) return;

	float m_zoomscale=1;
	float m_xscaleoffset=0.08;
	float m_yscaleoffset=0.08;
	m_xscaleoffset=m_xscaleoffset*m_zoomscale;
	m_yscaleoffset=m_yscaleoffset*m_zoomscale;// 视图比例
	float m_viewx=m_xscaleoffset*x*NonuniformBinx+100;
	float m_viewy=m_yscaleoffset*y*NonuniformBiny+100;
	if(m_viewx<=950) 
	{
		m_viewx=950;
		QPoint xx=kedufun(x*NonuniformBinx);
		m_xscaleoffset=(float)(m_viewx-100)/(xx.x()*xx.y());
	}
	if(m_viewy<=650) 
	{
		m_viewy=650;
		QPoint yy=kedufun(y*NonuniformBiny);
		m_yscaleoffset=(float)(m_viewy-100)/(yy.x()*yy.y());
	}

	resizeContents(m_viewx+100,m_viewy);
	mpBinImage->resize(m_viewx+100,m_viewy);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色		
	QPainter p(mpBinImage);//定义绘图指针

	int cx=m_viewx-50,cy=m_viewy-50;
	QColor windowcolor;	
	float temp2=(max-min)/15;
	if (temp2 <= 0.0f) temp2 = 1.0f;
	for(int i=0;i<x;i++)
	{
		for(int j=0;j<y;j++)
		{
			if(array[i][j]<min||array[i][j]>max) continue;
			if(array[i][j]!=0) windowcolor=datatocolor((array[i][j]-min)/temp2);
			else windowcolor.setRgb(255,255,255);
			QBrush brush;
			brush.setColor(windowcolor);
			p.setBrush(windowcolor);//定义画刷颜色
			QPen pen3(windowcolor,0);//定义画笔
			p.setPen(pen3);//选入画笔
			if(NonuniformBinx*m_xscaleoffset<3&&NonuniformBiny*m_yscaleoffset<3)	
				p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-3,3,3);
			else if(NonuniformBinx*m_xscaleoffset<3&&NonuniformBiny*m_yscaleoffset>=3) 
			{
				p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-NonuniformBiny*m_yscaleoffset,3,NonuniformBiny*m_yscaleoffset+1);
			}
			else if(NonuniformBinx*m_xscaleoffset>=3&&NonuniformBiny*m_yscaleoffset<3) p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-3,NonuniformBinx*m_xscaleoffset+1,3);
			else p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-NonuniformBiny*m_yscaleoffset,NonuniformBinx*m_xscaleoffset+1,NonuniformBiny*m_yscaleoffset+1);
		}
	}
	QPen pen5(Qt::black,2);//定义画笔
	p.setPen(pen5);//选入画笔
	p.drawLine(0,50,m_viewx,50);
	p.drawLine(0,cy,m_viewx,cy);
	p.drawLine(50,0,50,m_viewy);
	QPoint xx=kedufun(x*NonuniformBinx);


	p.drawText(75,20,mTextAmizth+"       "+mTextOffset);


	for(int i=0;i<=xx.y();i++)//x方向刻度坐标
	{
		if(i%10==0)
		{
			QFont font("Timers",10);
			p.setFont(font);
			QPen pen(Qt::black,1);//定义画笔
			p.setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*xx.x());
			p.drawText(55+i*m_xscaleoffset*xx.x(),39,str);
			//if(i!=0) p.drawText(45+i*m_xscaleoffset*xx.x(),cy+19,str);// 为什么要错误显示呢？ [Liu Wei 3/28/2023]
			p.drawText(55+i*m_xscaleoffset*xx.x(),cy+19,str);// 保证上下坐标轴刻度标签一致 [Liu Wei 3/28/2023]
			p.drawLine(50+i*m_xscaleoffset*xx.x(),50,50+i*m_xscaleoffset*xx.x(),40);
			p.drawLine(50+i*m_xscaleoffset*xx.x(),cy,50+i*m_xscaleoffset*xx.x(),cy+10);
		}
		else 
		{
			p.drawLine(50+i*m_xscaleoffset*xx.x(),50,50+i*m_xscaleoffset*xx.x(),45);
			p.drawLine(50+i*m_xscaleoffset*xx.x(),cy,50+i*m_xscaleoffset*xx.x(),cy+5);
		}
	}
	QPoint yy=kedufun(y*NonuniformBiny);
	for(int i=0;i<yy.y();i++)//y方向刻度
	{		
		if(i%10==0)	
		{
			p.drawLine(50,cy-i*m_yscaleoffset*yy.x(),40,cy-i*m_yscaleoffset*yy.x());
			QFont font("Timers",10);
			p.setFont(font);
			QPen pen(Qt::black,1);//定义画笔
			p.setPen(pen);//选入画笔
			QString str;
			str.sprintf("%d",i*yy.x());
			//			p.drawText(20,cy-i*m_yscaleoffset*yy.x(),str);
			p.save();
			p.rotate(-90);
			p.setWindow(0,0,m_viewx,m_viewy);
			p.drawText(-cy+i*m_yscaleoffset*yy.x(),25,str);
			p.restore();
		}
		else
		{
			p.drawLine(50,cy-i*m_yscaleoffset*yy.x(),45,cy-i*m_yscaleoffset*yy.x());
		}
	}
	p.save();
	p.rotate(-90);
	p.setWindow(0,0,m_viewx,m_viewy);
	//p.drawText(-cy+cy/2,15,"y number");
	//  [Liu Wei 3/28/2023]
	QString yLabelText="Y number";
	QFontMetrics mFontMetrics(p.font());
	int nFontWidthPixel=mFontMetrics.width(yLabelText);// 计算显示字符串的宽度
	p.drawText(-cy+cy/2-nFontWidthPixel/2,15,yLabelText);
	//  [Liu Wei 3/28/2023]
	p.restore();
	//p.drawText(m_viewx/2,20,"x number");
	//  [Liu Wei 3/28/2023]
	QString xLabelText="X number";
	nFontWidthPixel=mFontMetrics.width(xLabelText);// 计算显示字符串的宽度
	p.drawText(cx/2-nFontWidthPixel/2,cy+35,xLabelText);// 将坐标标题显示在底端
	//  [Liu Wei 3/28/2023]

	if(CSQFactor!=-9999)
	{
		p.drawText(m_viewx-150,20,"QF : "+QString::number(CSQFactor));//属性名称
	}
	else
	{
		p.drawText(m_viewx-150,20,FnameStr);//属性名称
	}
	//画色标条
	float temp=(cy-50)/(2*15);
	for(int i=1;i<=15;i++)//画色标条
	{		
		windowcolor=datatocolor(i-1);
		QBrush brush;
		brush.setColor(windowcolor);
		p.setBrush(windowcolor);
		QPen pen(windowcolor,0);
		p.setPen(pen);
		p.drawRect(m_viewx,cy-50-(cy-50-15*temp)/2-i*temp,20,temp);
		QString str;
		float hh=0;
		if(min!=max||(min=max&&min!=0)) hh=i*temp2+min;
		if(i==15) str.sprintf("%3.2f",max);
		else str.sprintf("%3.2f",hh);
		QPen pen2(Qt::black,0);
		p.setPen(pen2);
		p.drawLine(m_viewx,cy-50-(cy-50-15*temp)/2-temp*15,m_viewx+20,cy-50-(cy-50-15*temp)/2-temp*15);
		p.drawLine(m_viewx+20,cy-50-(cy-50-15*temp)/2-temp*15,m_viewx+20,cy-50-(cy-50-15*temp)/2);
		p.drawLine(m_viewx+20,cy-50-(cy-50-15*temp)/2,m_viewx,cy-50-(cy-50-15*temp)/2);
		p.drawLine(m_viewx,cy-50-(cy-50-15*temp)/2,m_viewx,cy-50-(cy-50-15*temp)/2-temp*15);
		p.drawText(m_viewx+20,cy-50-(cy-50-15*temp)/2-i*temp,str);	
	}
	//画色标条
	//valueShowNonuiform=true;//显示非均匀系数标志
	repaintContents();
}

QPoint analyzebinview::kedufun(int m)//为一个数选择合适的刻度函数
{
	QPoint point;
	if (m <= 0)
	{
		point.setX(1);
		point.setY(1);
		return point;
	}
	float kedu=(float)m;
	int ii=1,a=0;
	if(kedu<=10.0f)
	{
		kedu=1.0f;
		a=m;
		if (a < 1) a = 1;
	}
	else
	{
		while(kedu/ii>1)//选择一个合适的刻度
		{
			a=(int)(kedu/ii);
			ii=ii*10;	
		}
		kedu=(float)ii/100.0f;
		if (kedu <= 0.0f) kedu = 1.0f;
		a=(int)(m/kedu)+1;	
		if (a < 1) a = 1;
	}
	point.setX((int)kedu);
	point.setY(a);
	return point;	
}

QColor analyzebinview::datatocolor(int m)//由数字得到颜色
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

void analyzebinview::contentsMousePressEvent(QMouseEvent *e)
{

}

void analyzebinview::contentsMouseMoveEvent(QMouseEvent *e)
{
	//以下代码屏蔽，因为代码中有错误，会导致程序崩溃
	/*
	QPoint a=e->pos();
	int x=0,y=0;
	if(mpBinDoc->valuefoldVSbins==true)//覆盖次数-面元数量
	{
	if(a.x()<=950&&a.x()>=50&&a.y()<=650&&a.y()>=50) 
	{
	x=binfold.x()*binfold.y()*(a.x()-50)/(950-50);
	y=binnumber.x()*binnumber.y()*(650-a.y())/(650-50);
	}
	mCoordinate->setText(QString::fromLocal8Bit("坐标: ")+"fold="+QString::number(x)+", bins="+QString::number(y));//坐标状态栏
	}
	if(mpBinDoc->valueoffsetVStraces==true)//炮检距-炮检对
	{
	if(a.x()<=950&&a.x()>=50&&a.y()<=650&&a.y()>=50) 
	{
	x=offset.x()*offset.y()*(a.x()-50)/(950-50);
	y=traces.x()*traces.y()*(650-a.y())/(650-50);
	}
	mCoordinate->setText(QString::fromLocal8Bit("坐标: ")+"offset="+QString::number(x)+", traces="+QString::number(y));//坐标状态栏
	}
	if(mpBinDoc->valueazimuthVStraces==true)//方位角-炮检对
	{
	if(a.x()<=950&&a.x()>=50&&a.y()<=650&&a.y()>=50) 
	{
	x=360*(a.x()-50)/(950-50);
	y=azimuthtraces.x()*azimuthtraces.y()*(650-a.y())/(650-50);
	}
	mCoordinate->setText(QString::fromLocal8Bit("坐标: ")+"azimuth="+QString::number(x)+", traces="+QString::number(y));//坐标状态栏
	}
	roseoffset=kedufun(maxoffset);
	int roseoffsetscale=roseoffset.x()*roseoffset.y()/mRoselx;
	//	float roseoffsetscale=(float)maxoffset/4;
	if(mpBinDoc->valueroseVStraces==true)//玫瑰花图-炮检对
	{
	float t=1.1;
	float xscale=(float)(650-50)/(2*roseoffset.x()*roseoffset.y()*t);
	float angle=0;
	int i=0,j=0,traces=0;
	x=sqrt((float)((a.x()-350)*(a.x()-350)+(a.y()-350)*(a.y()-350)))/xscale;
	if(x<roseoffset.x()*roseoffset.y()) 
	{
	//			x=sqrt((a.x()-350)*(a.x()-350)+(a.y()-350)*(a.y()-350))/xscale;
	angle=atan((float)((a.y()-350))/((float)(a.x()-350)))*180/3.14159265;
	if(a.x()>350&&a.y()<350) angle=90-abs(angle);
	if(a.x()>350&&a.y()>350) angle=90+abs(angle);
	if(a.x()<350&&a.y()<350) angle=270+abs(angle);
	if(a.x()<350&&a.y()>350) angle=270-abs(angle);
	if(a.x()==350&&a.y()<=350) angle=0;
	if(a.x()==350&&a.y()>350) angle=180;
	if(a.y()==350&&a.x()>350) angle=90;
	if(a.y()==350&&a.x()<350) angle=270;
	i=x/roseoffsetscale;
	if(i>=mRoselx) i=mRoselx-1;
	j=angle/10;
	if(valuelineroseVStraces==true) traces=lineroseVStraces[i][j];
	else traces=roseVStraces[i][j];
	}
	else
	{
	x=0;
	}
	mCoordinate->setText(QString::fromLocal8Bit("")+"offset="+QString::number(x)+", azimuth="+QString::number(angle)+", traces="+QString::number(traces));//坐标状态栏
	}
	if(mpBinDoc->valueoffsetVSfoldbins==true)//炮检距-覆盖次数-面元数量
	{
	if(a.x()<=650&&a.x()>=50&&a.y()<=650&&a.y()>=50) 
	{
	QPoint point=kedufun(maxoffset);
	int offset=point.x()*point.y();
	int scaleoffset=offset/4;//偏移距4等份分组
	int bins=0;float fold2;
	x=point.x()*point.y()*(a.x()-50)/(650-50);
	y=fold.x()*fold.y()*(650-a.y())/(650-50);
	if(fold.x()*fold.y()<=30)
	{				
	if(y<maxfold&&x<offset)
	{
	if (valuelineoffseVSfoldbins==true)
	{
	bins=lineoffsetVSfoldbins[x/scaleoffset][y];
	}
	else bins=offsetVSfoldbins[x/scaleoffset][y];
	}					
	}
	else
	{
	fold2=(float)fold.x()*fold.y()/30;//将纵坐标分成最多30等分
	if(y<maxfold&&x<offset) 
	{
	float temp=(float)y/fold2;
	int temp2=(int)temp;
	if(valuelineoffseVSfoldbins==true)
	{
	bins=lineoffsetVSfoldbins2[x/scaleoffset][temp2];
	}
	else	bins=offsetVSfoldbins2[x/scaleoffset][temp2];
	}
	}
	mCoordinate->setText(QString::fromLocal8Bit("")+"offset="+QString::number(x)+",fold="+QString::number(y)+",bins="+QString::number(bins));//坐标状态栏
	}
	}
	if(mpBinDoc->valuebinfold==true)//覆盖次数
	{
	int temp=0;
	if(a.x()<=viewx&&a.x()>=50&&a.y()<=viewy-50&&a.y()>=50) 
	{
	x=(a.x()-50)/xscaleoffset;
	y=(viewy-50-a.y())/yscaleoffset+1;
	if(x<mpBinDoc->getDataXbinnumber()&&y<mpBinDoc->getDataYbinnumber()+1)
	{
	if(Binfold[x][y-1]>0) temp=Binfold[x][y-1];
	}			
	}
	mCoordinate->setText(QString::fromLocal8Bit("坐标: ")+"xbin="+QString::number(x)+", ybin="+QString::number(y)+", fold="+QString::number(temp));//坐标状态栏
	}
	*/
}

void analyzebinview::contentsMouseReleaseEvent(QMouseEvent *e)
{

}

void analyzebinview::contentsMouseDoubleClickEvent( QMouseEvent* e)//鼠标双击事件
{
	if(mpBinDoc->getDataValueline()==true||mpBinDoc->getDataValueworkarea()==TRUE)
	{
		if(e->x()<54&&e->x()>46&&mpBinDoc->valueroseVStraces==false&&mpBinDoc->valueoffsetVSfoldbins==false) //修改y
		{
			Changecoordinatedlg *dlg=new Changecoordinatedlg();
			if(dlg->exec())//从对话框中获得参数
			{
				if(dlg->getcoordmaxvalue()!=0)
				{
					if(mpBinDoc->getDataValueline()==true)
					{
						if(mpBinDoc->valuefoldVSbins==true) 
						{
							ycoordlinefold=dlg->getcoordmaxvalue();//修改坐标对话框

						}
						if(mpBinDoc->valueoffsetVStraces==true) 
						{
							ycoordlineoffset=dlg->getcoordmaxvalue();//修改坐标对话框

						}
						if(mpBinDoc->valueazimuthVStraces==true) 
						{
							ycoordlineAzimuth=dlg->getcoordmaxvalue();//修改坐标对话框

						}
					}
					if(mpBinDoc->getDataValueworkarea()==true)
					{
						if(mpBinDoc->valuefoldVSbins==true) 
						{
							ycoordworkfold=dlg->getcoordmaxvalue();//修改坐标对话框
							drawworkareaFoldVSBIns(xcoordworkfold,ycoordworkfold);
						}
						if(mpBinDoc->valueoffsetVStraces==true) 
						{ycoordworkoffset=dlg->getcoordmaxvalue();//修改坐标对话框
						drawworkareaOffsetVSTraces(xcoordworkoffset,ycoordworkoffset);
						}
						if(mpBinDoc->valueazimuthVStraces==true) 
						{ycoordworkAzimuth=dlg->getcoordmaxvalue();//修改坐标对话框
						drawworkareaAzimuthVSTraces(ycoordworkAzimuth);
						}
					}
				}				
			}
			delete dlg;
		}
		if((e->y()<54&&e->y()>46||e->y()<654&&e->y()>646)&&mpBinDoc->valueoffsetVSfoldbins==false&&mpBinDoc->valueazimuthVStraces==false&&mpBinDoc->valueroseVStraces==false) //修改x
		{
			Changecoordinatedlg *dlg=new Changecoordinatedlg();
			if(dlg->exec())//从对话框中获得参数
			{
				if(dlg->getcoordmaxvalue()!=0)
				{
					if(mpBinDoc->getDataValueline()==true)
					{
						if(mpBinDoc->valuefoldVSbins==true) xcoordlinefold=dlg->getcoordmaxvalue();//修改坐标对话框
						if(mpBinDoc->valueoffsetVStraces==true) xcoordlineoffset=dlg->getcoordmaxvalue();//修改坐标对话框
					}
					if(mpBinDoc->getDataValueworkarea()==true)
					{
						if(mpBinDoc->valuefoldVSbins==true) 
						{xcoordworkfold=dlg->getcoordmaxvalue();//修改坐标对话框
						drawworkareaFoldVSBIns(xcoordworkfold,ycoordworkfold);
						}
						if(mpBinDoc->valueoffsetVStraces==true) 
						{xcoordworkoffset=dlg->getcoordmaxvalue();//修改坐标对话框
						drawworkareaOffsetVSTraces(xcoordworkoffset,ycoordworkoffset);
						}
					}
				}
			}
			delete dlg;			
		}
	}
}

void analyzebinview::drawContents(QPainter* p, int cx, int cy, int cw, int ch)//不断的刷新贴图
{
	p->drawPixmap(cx, cy, *mpBinImage, cx, cy, cw, ch);
}

QPoint analyzebinview::calibrationFunction( double m )
{
	QPoint point;
	float kedu=m;
	int ii=1,a=0;
	if(kedu<=10)
	{
		kedu=1;
		a=m;
	}
	else
	{
		while(kedu/ii>1)//选择一个合适的刻度
		{
			a=kedu/ii;
			ii=ii*10;	
		}
		kedu=ii/100;
		a=m/kedu+1;	
	}
	point.setX(kedu);
	point.setY(a);
	return point;	
}
int analyzebinview::lpXTodpX( double lx )
{
	return gRound((lx-mCoordinateXmin)/getScaleX()+blkCoordinate*2);
}

int analyzebinview::lpYTodpY( double ly )
{
	return gRound((ly-mCoordinateYmin)/getScaleY()+blkCoordinate*2);
}

double analyzebinview::dpXTolpX( int dx )
{
	return (dx-blkCoordinate*2)*getScaleX()+mCoordinateXmin;
}

double analyzebinview::dpYTolpY( int dy )
{
	return (dy-blkCoordinate*2)*getScaleY()+mCoordinateYmin;
}
void analyzebinview::drawWorkAreaBinFold( int **array,int xbinnum,int ybinnum )
{
	/*********************************LiuWei***************************************/
	/* 注意：这里输入的面元个数xbinnum为测线方向的面元个数、ybinnum为联络线方向的面元个数；
	目前程序中的面元属性是在转换坐标系中计算的，其数组二维下标与网格坐标数组下标是相反的。*/
	/*********************************LiuWei***************************************/
	//自定义视口大小
	int colorbar_width=100;//色标绘制宽度
	int viewport_width=1150;//整个视口宽度
	int viewport_height=925;//整个视口高度
	//判断绘图数据是否正确
	if (!mpCellGrid || !mpCellGrid->mGrid || array == NULL) return;
	int minfold=0;int maxfold=0;
	for(int i=0;i<xbinnum;i++)
	{
		for(int j=0;j<ybinnum;j++)
		{
			if(minfold>array[i][j]) minfold=array[i][j];
			if(maxfold<array[i][j]) maxfold=array[i][j];
		}
	}
	if(maxfold<=0) return;
	//绘制数据范围
	double binlength=mpCellGrid->mXmax-mpCellGrid->mXmin;
	double binheight=mpCellGrid->mYmax-mpCellGrid->mYmin;
	if (binlength <= 0) binlength = 1000.0;
	if (binheight <= 0) binheight = 1000.0;
	mCoordinateXmin=mpCellGrid->mXmin;
	mCoordinateYmin=mpCellGrid->mYmin;
	//刷新绘图区域
	resizeContents(viewport_width,viewport_height);//+100是为色标留位置
	mpBinImage->resize(viewport_width,viewport_height);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter p(mpBinImage);//定义绘图指针
	QFont font0("SimSun",10);
	p.setFont(font0);	//初始字符设置：宋体
	p.drawText(75,20,mTextAmizth+"       "+mTextOffset);

	//  [Liu Wei 3/29/2023]
	viewx=viewport_width-colorbar_width;//数据绘图区域1050
	viewy=viewport_height;				//数据绘图区域925
	//设置绘图留白/坐标轴及标签区域
	int cx=viewx-blkCoordinate,cy=viewy-blkCoordinate;
	int clipx=blkCoordinate*2, clipy=blkCoordinate*2;//实际绘图范围中的坐标原点（单边留白100个像素宽度）
	int clipw=viewx-blkCoordinate*4, cliph=viewy-blkCoordinate*4;//实际绘图范围中的区域大小
	//计算并确定缩放比例
	if (binlength*5 < binheight)// 为了防止纵横向范围差过大导致图形显示不美观进行的优化 [Liu Wei 4/5/2023]
	{
		float wh_ratio=binlength*2/binheight;
		yscale = binheight / cliph;
		xscale = yscale*wh_ratio;
	}
	else if (binlength > binheight*5)
	{
		float wh_ratio=binheight*2/binlength;
		xscale = binlength / clipw;
		yscale = xscale*wh_ratio;
	}
	else
	{
		xscale = binlength / clipw;
		yscale = binheight / cliph;
		if(xscale < yscale) xscale = yscale;
	}
	//  [Liu Wei 3/29/2023]
	QColor windowcolor;	
	float temp2=(float)(maxfold-minfold)/15;//值域区间（颜色）划分为15段
	if (temp2 <= 0.0f) temp2 = 1.0f;

	for(int i=0;i<xbinnum;i++)
	{
		for(int j=0;j<ybinnum;j++)
		{
			if(array[i][j]!=0) windowcolor=datatocolor((array[i][j]-minfold)/temp2);
			else windowcolor.setRgb(255,255,255);
			QBrush brush;
			brush.setColor(windowcolor);
			p.setBrush(windowcolor);//定义画刷颜色
			QPen pen3(windowcolor,0);//定义画笔
			p.setPen(pen3);//选入画笔
			drawCellAttribute(&p,j,i,clipx,clipy,clipw,cliph,pen3,brush);//注意array和cellgrid的维度顺序不同
		}
	}
	//绘制坐标轴
	QPen pen5(Qt::black,2);//定义坐标轴画笔
	p.setPen(pen5);//选入画笔
	p.drawLine(0,blkCoordinate,viewx,blkCoordinate);
	p.drawLine(0,cy,viewx,cy);
	p.drawLine(blkCoordinate,0,blkCoordinate,viewy);
	p.drawLine(cx,0,cx,viewy);

	//绘制坐标轴刻度与标签
	QPoint xx=calibrationFunction(binlength);
	for(int i=0;i<=xx.y();i++)//x方向刻度坐标
	{
		QFont font("Timers",10);
		p.setFont(font);
		QPen pen(Qt::black,0);//定义画笔
		p.setPen(pen);//选入画笔
		if(i%10 == 0)
		{
			QString str;
			str.sprintf("%d",gRound(i*xx.x()+mCoordinateXmin));
			int txtwidth=p.fontMetrics().width(str);	//字符串宽度
			p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,blkCoordinate-11,str);	// 顶端坐标轴标签
			p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,cy+19,str);				// 保证上下坐标标签显示一致 [Liu Wei 3/28/2023]
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate, blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate-10);
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),cy, blkCoordinate*2+i*(1/xscale)*xx.x(),cy+10);
		}
		else 
		{
			if (i == xx.y() && i%10 > 5)// 最后一个刻度超过最大刻度间隔的一半则标注数值标签 [Liu Wei 4/6/2023]
			{
				QString str;
				str.sprintf("%d",gRound(i*xx.x()+mCoordinateXmin));
				int txtwidth=p.fontMetrics().width(str);	//字符串宽度
				p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,blkCoordinate-11,str);	// 顶端坐标轴标签
				p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,cy+19,str);				// 保证上下坐标标签显示一致 [Liu Wei 3/28/2023]
			}
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate, blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate-5);
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),cy, blkCoordinate*2+i*(1/xscale)*xx.x(),cy+5);
		}
	}

	QPoint yy=calibrationFunction(binheight);
	for(int i=0;i<=yy.y();i++)//y方向刻度
	{		
		QFont font("Timers",10);
		p.setFont(font);
		QPen pen(Qt::black,0);//定义画笔
		p.setPen(pen);//选入画笔
		if(i%10 == 0)
		{
			QString str;
			str.sprintf("%d",gRound(i*yy.x()+mCoordinateYmin));//注意：对于整型格式（"%d"），第二个参数必须为整型，否则转换为错误的数值字符串！
			int txtwidth=p.fontMetrics().width(str);	//字符串宽度
			p.save();
			p.rotate(-90);// 逆时针旋转90° [Liu Wei 3/28/2023]
			p.drawText(-blkCoordinate*2-i*(1/yscale)*yy.x()-txtwidth/2,35,str);// 注意参数x,y的值和顺序，值得思考！！ [Liu Wei 3/29/2023]
			p.restore();
			p.drawLine(blkCoordinate,blkCoordinate*2+i*(1/yscale)*yy.x(), blkCoordinate-10,blkCoordinate*2+i*(1/yscale)*yy.x());
		}
		else
		{
			if (i == yy.y() && i%10 > 5)// 最后一个刻度超过最大刻度间隔的一半则标注数值标签 [Liu Wei 4/6/2023]
			{
				QString str;
				str.sprintf("%d",gRound(i*yy.x()+mCoordinateYmin));//注意：对于整型格式（"%d"），第二个参数必须为整型，否则转换为错误的数值字符串！
				int txtwidth=p.fontMetrics().width(str);	//字符串宽度
				p.save();
				p.rotate(-90);// 逆时针旋转90° [Liu Wei 3/28/2023]
				p.drawText(-blkCoordinate*2-i*(1/yscale)*yy.x()-txtwidth/2,35,str);// 注意参数x,y的值和顺序，值得思考！！ [Liu Wei 3/29/2023]
				p.restore();
			}
			p.drawLine(blkCoordinate,blkCoordinate*2+i*(1/yscale)*yy.x(), blkCoordinate-5,blkCoordinate*2+i*(1/yscale)*yy.x());
		}
	}

	// 自定义字体和画笔 [Liu Wei 3/28/2023]
	QFont _font("Timers",10);
	p.setFont(_font);
	QPen _pen(Qt::black,1);//定义画笔
	p.setPen(_pen);//选入画笔
	// 绘制x坐标轴标题 [Liu Wei 3/28/2023]
	QString xLabelText="X Axis";
	QFontMetrics xFontMetrics(p.font());
	int xFontWidthPixel=xFontMetrics.width(xLabelText);// 计算显示字符串的宽度
	p.drawText(viewx/2-xFontWidthPixel/2,cy+35,xLabelText);// 将坐标标题显示在底端
	//  [Liu Wei 3/28/2023]
	p.save();
	p.rotate(-90);
	// 绘制y坐标轴标题 [Liu Wei 3/28/2023]
	QString yLabelText="Y Axis";
	QFontMetrics yFontMetrics(p.font());
	int yFontWidthPixel=yFontMetrics.width(yLabelText);// 计算显示字符串的宽度
	p.drawText(-viewy/2+yFontWidthPixel/2,15,yLabelText);
	//  [Liu Wei 3/28/2023]
	p.restore();

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
		if(minfold!=maxfold||(minfold=maxfold&&minfold!=0)) hh=i*temp2+minfold;
		if(i==15) str.sprintf("%d",maxfold);
		else str.sprintf("%d",hh);
		QPen pen2(Qt::black,0);
		p.setPen(pen2);
		p.drawLine(viewx,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2-temp*15);
		p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2);
		p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2);
		p.drawLine(viewx,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2-temp*15);
		int txtheight=p.fontMetrics().height(); //字符串高度
		p.drawText(viewx+21,cy-50-(cy-50-15*temp)/2-i*temp+txtheight/2,str);	
	}
	//画色标条
	repaintContents();
}

void analyzebinview::drawOffsetNonuniformFactor( float **array,int xbinnum,int ybinnum,float max,float min,float NonuniformBinx,float NonuniformBiny,QString FnameStr,float CSQFactor )
{
	if(max<=0 || !mpCellGrid || !mpCellGrid->mGrid || array == NULL) return;	//判断绘图数据是否正确
	//
	/*********************************LiuWei***************************************/
	/*	注意：这里输入的面元个数xbinnum为测线方向的面元个数、ybinnum为联络线方向的面元个数；
	目前程序中的面元属性是在转换坐标系中计算的，其数组二维下标与网格坐标数组下标是相反的。
	原函数中的参数NonuniformBiny，NonuniformBiny都取值为1，因此本次实现直接忽略。     */
	/*********************************LiuWei***************************************/
	//自定义视口大小
	int colorbar_width=100;//色标绘制宽度
	int viewport_width=1150;//整个视口宽度
	int viewport_height=925;//整个视口高度

	//绘制数据范围
	double binlength=mpCellGrid->mXmax-mpCellGrid->mXmin;
	double binheight=mpCellGrid->mYmax-mpCellGrid->mYmin;
	if (binlength <= 0) binlength = 1000.0;
	if (binheight <= 0) binheight = 1000.0;
	mCoordinateXmin=mpCellGrid->mXmin;
	mCoordinateYmin=mpCellGrid->mYmin;
	//刷新绘图区域
	resizeContents(viewport_width,viewport_height);//+100是为色标留位置
	mpBinImage->resize(viewport_width,viewport_height);//定义绘图区域
	mpBinImage->fill(Qt::white);//绘图区域填充色
	QPainter p(mpBinImage);//定义绘图指针
	QFont font0("SimSun",10);
	p.setFont(font0);	//初始字符设置：宋体
	p.drawText(75,20,mTextAmizth+"       "+mTextOffset);

	//  [Liu Wei 3/29/2023]
	viewx=viewport_width-colorbar_width;//数据绘图区域
	viewy=viewport_height;				//数据绘图区域
	//设置绘图留白/坐标轴及标签区域
	int cx=viewx-blkCoordinate,cy=viewy-blkCoordinate;
	int clipx=blkCoordinate*2, clipy=blkCoordinate*2;//实际绘图范围中的坐标原点（单边留白100个像素宽度）
	int clipw=viewx-blkCoordinate*4, cliph=viewy-blkCoordinate*4;//实际绘图范围中的区域大小
	//计算并确定缩放比例
	if (binlength*5 < binheight)// 为了防止纵横向范围差过大导致图形显示不美观进行的优化 [Liu Wei 4/5/2023]
	{
		float wh_ratio=binlength*2/binheight;
		yscale = binheight / cliph;
		xscale = yscale*wh_ratio;
	}
	else if (binlength > binheight*5)
	{
		float wh_ratio=binheight*2/binlength;
		xscale = binlength / clipw;
		yscale = xscale*wh_ratio;
	}
	else
	{
		xscale = binlength / clipw;
		yscale = binheight / cliph;
		if(xscale < yscale) xscale = yscale;
	}
	//  [Liu Wei 3/29/2023]

	QColor windowcolor;	
	float temp2=(max-min)/15;//值域区间（颜色）划分为15段
	if (temp2 <= 0.0f) temp2 = 1.0f;

	for(int i=0;i<xbinnum;i++)
	{
		for(int j=0;j<ybinnum;j++)
		{
			if(array[i][j]<min||array[i][j]>max) continue;
			if(array[i][j]!=0) windowcolor=datatocolor((array[i][j]-min)/temp2);
			else windowcolor.setRgb(255,255,255);
			QBrush brush;
			brush.setColor(windowcolor);
			p.setBrush(windowcolor);//定义画刷颜色
			QPen pen3(windowcolor,0);//定义画笔
			p.setPen(pen3);//选入画笔
			drawCellAttribute(&p,j,i,clipx,clipy,clipw,cliph,pen3,brush);//注意array和cellgrid的维度顺序不同
			//if(NonuniformBinx*m_xscaleoffset<3&&NonuniformBiny*m_yscaleoffset<3)	
			//	p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-3,3,3);
			//else if(NonuniformBinx*m_xscaleoffset<3&&NonuniformBiny*m_yscaleoffset>=3) 
			//{
			//	p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-NonuniformBiny*m_yscaleoffset,3,NonuniformBiny*m_yscaleoffset+1);
			//}
			//else if(NonuniformBinx*m_xscaleoffset>=3&&NonuniformBiny*m_yscaleoffset<3) p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-3,NonuniformBinx*m_xscaleoffset+1,3);
			//else p.drawRect(50+i*NonuniformBinx*m_xscaleoffset,cy-j*NonuniformBiny*m_yscaleoffset-NonuniformBiny*m_yscaleoffset,NonuniformBinx*m_xscaleoffset+1,NonuniformBiny*m_yscaleoffset+1);
		}
	}
	//绘制坐标轴
	QPen pen5(Qt::black,2);//定义坐标轴画笔
	p.setPen(pen5);//选入画笔
	p.drawLine(0,blkCoordinate,viewx,blkCoordinate);
	p.drawLine(0,cy,viewx,cy);
	p.drawLine(blkCoordinate,0,blkCoordinate,viewy);
	p.drawLine(cx,0,cx,viewy);

	//绘制坐标轴刻度与标签
	QPoint xx=calibrationFunction(binlength);
	for(int i=0;i<=xx.y();i++)//x方向刻度坐标
	{
		QFont font("Timers",10);
		p.setFont(font);
		QPen pen(Qt::black,0);//定义画笔
		p.setPen(pen);//选入画笔	
		if(i%10 == 0)
		{
			QString str;
			str.sprintf("%d",gRound(i*xx.x()+mCoordinateXmin));
			int txtwidth=p.fontMetrics().width(str);	//字符串宽度
			p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,blkCoordinate-11,str);	// 顶端坐标轴标签
			p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,cy+19,str);				// 保证上下坐标标签显示一致 [Liu Wei 3/28/2023]
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate, blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate-10);
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),cy, blkCoordinate*2+i*(1/xscale)*xx.x(),cy+10);
		}
		else 
		{
			if (i==xx.y() && i%10 > 5)
			{
				QString str;
				str.sprintf("%d",gRound(i*xx.x()+mCoordinateXmin));
				int txtwidth=p.fontMetrics().width(str);	//字符串宽度
				p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,blkCoordinate-11,str);	// 顶端坐标轴标签
				p.drawText(blkCoordinate*2+i*(1/xscale)*xx.x()-txtwidth/2,cy+19,str);				// 保证上下坐标标签显示一致 [Liu Wei 3/28/2023]
			}
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate, blkCoordinate*2+i*(1/xscale)*xx.x(),blkCoordinate-5);
			p.drawLine(blkCoordinate*2+i*(1/xscale)*xx.x(),cy, blkCoordinate*2+i*(1/xscale)*xx.x(),cy+5);
		}
	}

	QPoint yy=calibrationFunction(binheight);
	for(int i=0;i<=yy.y();i++)//y方向刻度
	{	
		QFont font("Timers",10);
		p.setFont(font);
		QPen pen(Qt::black,0);//定义画笔
		p.setPen(pen);//选入画笔
		if(i%10 == 0)
		{
			QString str;
			str.sprintf("%d",gRound(i*yy.x()+mCoordinateYmin));//注意：对于整型格式（"%d"），第二个参数必须为整型，否则转换为错误的数值字符串！
			int txtwidth=p.fontMetrics().width(str);	//字符串宽度
			p.save();
			p.rotate(-90);// 逆时针旋转90° [Liu Wei 3/28/2023]
			p.drawText(-blkCoordinate*2-i*(1/yscale)*yy.x()-txtwidth/2,35,str);// 注意参数x,y的值和顺序，值得思考！！ [Liu Wei 3/29/2023]
			p.restore();
			p.drawLine(blkCoordinate,blkCoordinate*2+i*(1/yscale)*yy.x(), blkCoordinate-10,blkCoordinate*2+i*(1/yscale)*yy.x());
		}
		else
		{
			if (i==yy.y() && i%10 > 5)
			{
				QString str;
				str.sprintf("%d",gRound(i*yy.x()+mCoordinateYmin));//注意：对于整型格式（"%d"），第二个参数必须为整型，否则转换为错误的数值字符串！
				int txtwidth=p.fontMetrics().width(str);	//字符串宽度
				p.save();
				p.rotate(-90);// 逆时针旋转90° [Liu Wei 3/28/2023]
				p.drawText(-blkCoordinate*2-i*(1/yscale)*yy.x()-txtwidth/2,35,str);// 注意参数x,y的值和顺序，值得思考！！ [Liu Wei 3/29/2023]
				p.restore();
			}
			p.drawLine(blkCoordinate,blkCoordinate*2+i*(1/yscale)*yy.x(), blkCoordinate-5,blkCoordinate*2+i*(1/yscale)*yy.x());
		}
	}

	// 自定义字体和画笔 [Liu Wei 3/28/2023]
	QFont _font("Timers",10);
	p.setFont(_font);
	QPen _pen(Qt::black,1);//定义画笔
	p.setPen(_pen);//选入画笔
	// 绘制x坐标轴标题 [Liu Wei 3/28/2023]
	QString xLabelText="X Axis";
	QFontMetrics xFontMetrics(p.font());
	int xFontWidthPixel=xFontMetrics.width(xLabelText);// 计算显示字符串的宽度
	p.drawText(viewx/2-xFontWidthPixel/2,cy+35,xLabelText);// 将坐标标题显示在底端
	//  [Liu Wei 3/28/2023]
	p.save();
	p.rotate(-90);
	// 绘制y坐标轴标题 [Liu Wei 3/28/2023]
	QString yLabelText="Y Axis";
	QFontMetrics yFontMetrics(p.font());
	int yFontWidthPixel=yFontMetrics.width(yLabelText);// 计算显示字符串的宽度
	p.drawText(-viewy/2+yFontWidthPixel/2,15,yLabelText);
	//  [Liu Wei 3/28/2023]
	p.restore();

	if(CSQFactor!=-9999)
	{
		p.drawText(viewx-150,20,"QF : "+QString::number(CSQFactor));//属性名称
	}
	else
	{
		p.drawText(viewx-150,20,FnameStr);//属性名称
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
		float hh=0;
		if(min!=max||(min=max&&min!=0)) hh=i*temp2+min;
		if(i==15) str.sprintf("%3.2f",max);
		else str.sprintf("%3.2f",hh);
		QPen pen2(Qt::black,0);
		p.setPen(pen2);
		p.drawLine(viewx,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2-temp*15);
		p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2-temp*15,viewx+20,cy-50-(cy-50-15*temp)/2);
		p.drawLine(viewx+20,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2);
		p.drawLine(viewx,cy-50-(cy-50-15*temp)/2,viewx,cy-50-(cy-50-15*temp)/2-temp*15);
		int txtheight=p.fontMetrics().height(); //字符串高度
		p.drawText(viewx+21,cy-50-(cy-50-15*temp)/2-i*temp+txtheight/2,str);	
	}
	//画色标条
	//valueShowNonuiform=true;//显示非均匀系数标志
	repaintContents();
}

void analyzebinview::drawCellAttribute( QPainter *p, int i, int j, int cx, int cy, int cw, int ch, QPen &pen, const QBrush &brush )
{
	if (!mpCellGrid || !mpCellGrid->mGrid) return;
	if (i < 0 || i >= mpCellGrid->mNumY || j < 0 || j >= mpCellGrid->mNumX) return;

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

		if (p->brush().style() == Qt::NoBrush)
		{
			p->setBrush(brush);
		}
		p->drawPolygon(pts);

	}
}
