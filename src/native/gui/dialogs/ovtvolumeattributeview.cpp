#include "ovtvolumeattributeview.h"

OVTVolumeAttributeView::OVTVolumeAttributeView(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.mCustomPlot->axisRect()->setVisible(false);//不显示绘图矩形坐标轴
	in_loop = false;
	offsetarray_ovt=NULL;
	azimutharray_ovt=NULL;
	min_offset=NULL;		min_azimuth=NULL;	
	max_offset=NULL;		max_azimuth=NULL;	
	average_offset=NULL;	average_azimuth=NULL;
	tolerance_offset=NULL;	tolerance_azimuth=NULL;
	dispersion_offset=NULL;	dispersion_azimuth=NULL;
	offset_count=NULL;
}

OVTVolumeAttributeView::~OVTVolumeAttributeView()
{
	offsetarray_ovt=NULL;//指针复位
	azimutharray_ovt=NULL;//指针复位
	//释放空间：炮检距（偏移距）属性
	if(min_offset!=NULL) gFree2(min_offset);
	if(max_offset!=NULL) gFree2(max_offset);
	if(average_offset!=NULL) gFree2(average_offset);
	if(tolerance_offset!=NULL) gFree2(tolerance_offset);
	if(dispersion_offset!=NULL) gFree2(dispersion_offset);
	//释放空间：方位角属性
	if(min_azimuth!=NULL) gFree2(min_azimuth);
	if(max_azimuth!=NULL) gFree2(max_azimuth);
	if(average_azimuth!=NULL) gFree2(average_azimuth);
	if(tolerance_azimuth!=NULL) gFree2(tolerance_azimuth);
	if(dispersion_azimuth!=NULL) gFree2(dispersion_azimuth);

	if(offset_count!=NULL) gFree2(offset_count);
}
//炮检距OVT体属性
void OVTVolumeAttributeView::on_actionOffset_Tolerance_triggered()
{
	QString attributenameStr=tr("Tolerance");//容差
	//调用绘图函数绘图
	HeatmapPlotting(tolerance_offset,Nx_ovt,Ny_ovt,attributenameStr,1);

	//QCustomPlot* customPlot  = ui.mCustomPlot;  //这个是我加的，ui-> + 自己的在ui文件中控件的类名 

	//////////////////////////////////////案例1////////////////////////////////////
	//// add two new graphs and set their look:
	//customPlot->addGraph();
	//customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	//customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
	//customPlot->addGraph();
	//customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	//// generate some points of data (y0 for first, y1 for second graph):
	//QVector<double> x(251), y0(251), y1(251);
	//for (int i=0; i<251; ++i)
	//{
	//	x[i] = i;
	//	y0[i] = qExp(-i/150.0)*qCos(i/10.0); // exponentially decaying cosine
	//	y1[i] = qExp(-i/150.0);              // exponential envelope
	//}
	//// configure right and top axis to show ticks but no labels:
	//// (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
	//customPlot->xAxis2->setVisible(true);
	//customPlot->xAxis2->setTickLabels(false);
	//customPlot->yAxis2->setVisible(true);
	//customPlot->yAxis2->setTickLabels(false);
	//// make left and bottom axes always transfer their ranges to right and top axes:
	//connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
	//connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
	//// pass data points to graphs:
	//customPlot->graph(0)->setData(x, y0);
	//customPlot->graph(1)->setData(x, y1);
	//// let the ranges scale themselves so graph 0 fits perfectly in the visible area:
	//customPlot->graph(0)->rescaleAxes();
	//// same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
	//customPlot->graph(1)->rescaleAxes(true);
	//// Note: we could have also just called customPlot->rescaleAxes(); instead
	//// Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
	//customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	//// 重画图像
	//customPlot->replot();
	///////////////////////////////////案例2///////////////////////////////////////
	//x轴设置
	//QSharedPointer<QCPAxisTickerFixed> intTicker_X(new QCPAxisTickerFixed);
	//intTicker_X->setTickStep(1);                                      //设置刻度之间的步长为1
	//intTicker_X->setScaleStrategy(QCPAxisTickerFixed::ssNone);   //设置缩放策略
	//customPlot->xAxis->setTicker(intTicker_X);                        //应用自定义整形ticker,防止使用放大功能时出现相同的x刻度值
	//customPlot->xAxis->ticker()->setTickCount(13);                    //刻度数量
	//customPlot->xAxis->setNumberFormat("f");                          //x轴刻度值格式
	//customPlot->xAxis->setNumberPrecision(0);                         //刻度值精度
	//customPlot->xAxis->setLabel("X轴");                             //设置标签
	//customPlot->xAxis->setLabelFont(QFont(font().family(),8));        //设置标签字体大小
	////customPlot->xAxis->setRange(-5,5,Qt::AlignLeft);                  //设置x轴范围-这个最好放在colormap等绘图对象数据设置以后，否则会被覆盖
	//customPlot->xAxis->setSubTicks(false);                            //设置子刻度隐藏
	////customPlot->xAxis->setSubTickLength(2,0);                       //子刻度长度
	//customPlot->xAxis->setTickLength(5,0);                            //主刻度长度
	//customPlot->xAxis->grid()->setPen(QPen(Qt::black));               //设置主刻度线网格线显示,设置为Qt::NoPen就不会显示
	//customPlot->xAxis->grid()->setSubGridVisible(false);              //设置子刻度网格线是否可见

	////y轴设置
	//QSharedPointer<QCPAxisTickerFixed> intTicker_Y(new QCPAxisTickerFixed);
	//intTicker_Y->setTickStep(1);
	//intTicker_Y->setScaleStrategy(QCPAxisTickerFixed::ssNone);
	//customPlot->yAxis->setTicker(intTicker_Y);
	//customPlot->yAxis->ticker()->setTickCount(13);
	//customPlot->yAxis->setNumberFormat("f");
	//customPlot->yAxis->setNumberPrecision(0);
	//customPlot->yAxis->setLabel("Y轴");
	//customPlot->yAxis->setLabelFont(QFont(font().family(),8));
	////customPlot->yAxis->setRange(-5,5,Qt::AlignLeft);	//设置x轴范围-这个最好放在colormap等绘图对象数据设置以后，否则会被覆盖
	//customPlot->yAxis->setSubTicks(false);
	////customPlot->yAxis->setSubTickLength(2,0);
	//customPlot->yAxis->setTickLength(5,0);
	//customPlot->yAxis->grid()->setPen(QPen(Qt::black));
	//customPlot->yAxis->grid()->setSubGridVisible(false);

	////模拟数据:{行,列,值}
	//srand((unsigned)time(NULL));
	//QVector<QVector<int> > data;
	//for (int i=0; i<10; i++)
	//{
	//	QVector<int> temp; int vv;
	//	for (int j=0; j<10; j++)
	//	{
	//		vv=(rand() % (9-0+1))+ 0;
	//		temp.append(vv);
	//	}
	//	data.append(temp);
	//}

	//QCPColorMap *heatmap = new QCPColorMap(customPlot->xAxis,customPlot->yAxis);   //构造一个颜色图
	//heatmap->data()->setSize(10,10);   //设置颜色图数据维度,代表你的数据矩阵(10x10),这里可以理解为有多少个小方块
	//heatmap->data()->setRange(QCPRange(-4.5,4.5), QCPRange(-4.5,4.5));   //颜色图在x、y轴上的范围,这里可以查看下setRange函数定义
	////注意这里数据范围很有讲究，长度等于X,Y方向方格个数减一，即NX-1,NY-1，这样才能保证每一个方格长度为1个单位长度。刘伟
	//for(int x=0;x<10;x++)
	//{
	//	for(int y=0;y<10;y++)
	//	{
	//		int z = data.at(x).at(y);
	//		if(z)
	//		{
	//			heatmap->data()->setCell(x, y, z);    //如果z不为0,则设置颜色值的位置
	//		}
	//		else
	//		{
	//			heatmap->data()->setAlpha(x, y, 0);   //z为0,设置为透明,根据需求来设置
	//		}
	//	}
	//}

	//QCPColorScale *colorScale = new QCPColorScale(customPlot);   //构造一个色条
	//colorScale->setType(QCPAxis::atBottom);                      //水平显示
	//customPlot->plotLayout()->addElement(1, 0, colorScale);      //在颜色图下面显示
	//heatmap->setColorScale(colorScale);                          //设置色条——自定义色标
	////QCPColorGradient gradient;                                   //色条使用的颜色渐变
	////gradient.setColorStopAt(0.0, QColor("#54ff9f"));             //设置渐变在指定位置的颜色(从0到1),最小为0.0
	////gradient.setColorStopAt(0.5, QColor("#f6efa6"));             //设置色条分段的颜色
	////gradient.setColorStopAt(1.0, QColor("#cd2626"));             //设置色条结束时的颜色,最大为1.0
	////heatmap->setGradient(gradient);                              //设置颜色渐变
	//heatmap->setGradient(QCPColorGradient::gpHues);
	//heatmap->setDataRange(QCPRange(0, 10));                       //设置数据Z范围
	////heatmap->rescaleDataRange(true);                             //自动计算数据范围，数据范围决定了哪些数据值映射到QCPColorGradient的颜色渐变当中
	//
	//heatmap->setInterpolate(false);                              //禁用插值,界面显示小方块
	//heatmap->setTightBoundary(false);							 //禁用裁剪：Sets whether the outer most data rows and columns are clipped to the specified key and value range	
	//
	////保持色条与轴矩形边距一致
	//QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
	//customPlot->axisRect()->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
	//colorScale->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);

	///************************************************************************/
	///*When the zoom factor is greater than one, scrolling the mouse wheel backwards (towards the user)
	//will zoom in (make the currently visible range smaller). For zoom factors smaller than one, the
	//same scrolling direction will zoom out.*/
	///************************************************************************/
	////customPlot->axisRect()->setRangeZoomFactor(1.2,1.2);//x、y方向伸缩比例一致，但是不能设置为1（要么小于1，要么大于1）；默认情况是x、y两方向设置了伸缩系数(0.85,0.85),故一般不用调用这个函数

	////设置网格线格式
	//customPlot->xAxis->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::PenStyle::DashLine));//网格白色虚线
	//customPlot->yAxis->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::PenStyle::DashLine));//网格白色虚线
	////customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(50, 50, 50), 1, Qt::DotLine));//网格浅色点线
	////customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(50, 50, 50), 1, Qt::DotLine));//网格浅色点线
	////customPlot->xAxis->grid()->setSubGridVisible(true);//显示x轴子网格线
	////customPlot->yAxis->grid()->setSubGridVisible(true);//显示要轴子网格线
	//customPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::black, 1.5, Qt::PenStyle::SolidLine));//x轴0线颜色白色
	//customPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::black, 1.5, Qt::PenStyle::SolidLine));//y轴0线颜色白色
	////刷新界面
	////customPlot->xAxis->setRangeLower(-5);
	////customPlot->xAxis->setRangeUpper(5);
	////customPlot->yAxis->setRangeLower(-5);
	////customPlot->yAxis->setRangeUpper(5);
	//customPlot->xAxis->setRange(-6,6);//与上面两个函数功能一致
	//customPlot->yAxis->setRange(-6,6);//与上面两个函数功能一致
	////customPlot->rescaleAxes();//Rescales the axes such that all plottables (like graphs) in the plot are fully visible.该函数会覆盖参数设置：customPlot->yAxis->setRange();customPlot->xAxis->setRange();
	////因此，colormap绘图最好不采用rescaleAxes()函数,而是采用setRange()函数单独设置。
	//customPlot->replot(QCustomPlot::rpQueuedReplot);

	//////////////////////////////////////案例3////////////////////////////////////
	//// configure axis rect:配置轴矩形
	//customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // 这也将允许通过拖拽/缩放尺度改变颜色范围
	//customPlot->axisRect()->setupFullAxesBox(true);
	//customPlot->xAxis->setLabel("x");
	//customPlot->yAxis->setLabel("y");

	//// set up the QCPColorMap:
	//QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
	//int nx = 200;
	//int ny = 200;
	//colorMap->data()->setSize(nx, ny); // 我们希望彩色地图有nx*ny的数据点
	//colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // 并在键（x）和值（y）维上跨越坐标范围-4..4
	//// ：现在，我们通过访问颜色贴图的QCPColorMapData实例来分配一些数据：
	//double x, y, z;
	//for (int xIndex=0; xIndex<nx; ++xIndex)
	//{
	//	for (int yIndex=0; yIndex<ny; ++yIndex)
	//	{
	//		colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
	//		double r = 3*qSqrt(x*x+y*y)+1e-2;
	//		z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
	//		colorMap->data()->setCell(xIndex, yIndex, z);
	//	}
	//}

	//// 添加色标:
	//QCPColorScale *colorScale = new QCPColorScale(customPlot);
	//customPlot->plotLayout()->addElement(0, 1, colorScale); // 将其添加到主轴矩形的右侧
	//colorScale->setType(QCPAxis::atRight); // 刻度应为垂直条，刻度线/坐标轴标签右侧（实际上，右侧已经是默认值）
	//colorMap->setColorScale(colorScale); // 将颜色图与色标关联
	//colorScale->axis()->setLabel("Magnetic Field Strength");

	//// 将颜色贴图的“颜色渐变”设置为其中一个预设，QCPColorGradient预定义了9种颜色渐变（详见源文件）
	//colorMap->setGradient(QCPColorGradient::gpPolar);
	//// 我们还可以创建一个QCPColorGradient实例并向其中添加自己的颜色
	//// 渐变，请参阅QCPColorGradient的文档以获取可能的效果.

	//// 重新缩放数据维度（颜色），以使所有数据点都位于颜色渐变显示的范围内:
	//colorMap->rescaleDataRange();
	////禁用插值,界面显示小方块
	//colorMap->setInterpolate(false);                             

	////确保轴rect和色标同步其底边距和顶边距（以便它们对齐）:
	//QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
	//customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
	//colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

	//// 重新缩放键（x）和值（y）轴，以便可以看到整个颜色图：
	//customPlot->rescaleAxes();
	//customPlot->replot();
}

void OVTVolumeAttributeView::on_actionOffset_Dispersion_triggered()
{
	//申请数据空间
	gAlloc2(dispersion_offset,Nx_ovt,Ny_ovt);
	//计算标准差（标准差是方差的算术平方根。标准差能反映一个数据集的离散程度，是应用最广泛的统计离散程度的测度方法。）
	for (int ix=0; ix<Nx_ovt; ix++)
	{
		for (int iy=0; iy<Ny_ovt; iy++)
		{//遍历每一OVT片，求取每一个OVT片离散度（标准差）属性
			int mcount=0; //计算变量
			double tmp_sum_offset=0; //累加中间变量
			for (int iz=0; iz<offsetarray_ovt[ix][iy].count(); iz++)
			{
				double diff;//中间变量
				diff=offsetarray_ovt[ix][iy][iz]-average_offset[ix][iy];
				tmp_sum_offset += pow(diff,2);
				mcount++;
			}
			dispersion_offset[ix][iy]=short(sqrt(tmp_sum_offset/mcount));//方差的算术平方根
		}
	}
	QString attributenameStr=tr("Dispersion");//离散度
	//调用绘图函数绘图
	HeatmapPlotting(dispersion_offset,Nx_ovt,Ny_ovt,attributenameStr,1);
}

void OVTVolumeAttributeView::on_actionMin_Offset_triggered()
{
	QString attributenameStr=tr("Minimum");
	//调用绘图函数绘图
	HeatmapPlotting(min_offset,Nx_ovt,Ny_ovt,attributenameStr,1);
}

void OVTVolumeAttributeView::on_actionMax_Offset_triggered()
{
	QString attributenameStr=tr("Maximum");
	//调用绘图函数绘图
	HeatmapPlotting(max_offset,Nx_ovt,Ny_ovt,attributenameStr,1);
}

void OVTVolumeAttributeView::on_actionAverage_Offset_triggered()
{
	QString attributenameStr=tr("Average");
	//调用绘图函数绘图
	HeatmapPlotting(average_offset,Nx_ovt,Ny_ovt,attributenameStr,1);
}

//方位角OVT体属性
void OVTVolumeAttributeView::on_actionAzimuth_Tolerance_triggered()
{
	QString attributenameStr=tr("Tolerance");//容差
	//调用绘图函数绘图
	HeatmapPlotting(tolerance_azimuth,Nx_ovt,Ny_ovt,attributenameStr);
}

void OVTVolumeAttributeView::on_actionAzimuth_Dispersion_triggered()
{
	//申请数据空间
	gAlloc2(dispersion_azimuth,Nx_ovt,Ny_ovt);
	//计算标准差（标准差是方差的算术平方根。标准差能反映一个数据集的离散程度，是应用最广泛的统计离散程度的测度方法。）
	for (int ix=0; ix<Nx_ovt; ix++)
	{
		for (int iy=0; iy<Ny_ovt; iy++)
		{//遍历每一OVT片，求取每一个OVT片离散度（标准差）属性
			int mcount=0; //计算变量
			double tmp_sum_azimuth=0; //累加中间变量
			for (int iz=0; iz<offsetarray_ovt[ix][iy].count(); iz++)//方位角和炮检距是成对出现的
			{
				double diff;//中间变量
				diff=azimutharray_ovt[ix][iy][iz]-average_azimuth[ix][iy];
				tmp_sum_azimuth += pow(diff,2);
				mcount++;
			}
			dispersion_azimuth[ix][iy]=short(sqrt(tmp_sum_azimuth/mcount));//方差的算术平方根
		}
	}
	QString attributenameStr=tr("Dispersion");//离散度
	//调用绘图函数绘图
	HeatmapPlotting(dispersion_azimuth,Nx_ovt,Ny_ovt,attributenameStr);
}

void OVTVolumeAttributeView::on_actionMin_Azimuth_triggered()
{
	QString attributenameStr=tr("Minimum");
	//调用绘图函数绘图
	HeatmapPlotting(min_azimuth,Nx_ovt,Ny_ovt,attributenameStr);
}

void OVTVolumeAttributeView::on_actionMax_Azimuth_triggered()
{
	QString attributenameStr=tr("Maximum");
	//调用绘图函数绘图
	HeatmapPlotting(max_azimuth,Nx_ovt,Ny_ovt,attributenameStr);
}

void OVTVolumeAttributeView::on_actionAverage_Azimuth_triggered()
{
	QString attributenameStr=tr("Average");
	//调用绘图函数绘图
	HeatmapPlotting(average_azimuth,Nx_ovt,Ny_ovt,attributenameStr);
}

int OVTVolumeAttributeView::exec() // [11/9/2022 Liu Wei] MainWindow阻塞显示（值得研究一下）
{
	if ( in_loop ) {
		qWarning( "QDialog::exec: Recursive call detected" );
		return -1;
	}
	show();
	in_loop = true;
	return qApp->enter_loop();		
}

//传递参数并计算相关OVT属性
void OVTVolumeAttributeView::setOVTcubeData( GValueVector<short> **offset_ovt,GValueVector<short> **azimuth_ovt,int nx_ovt,int ny_ovt,float dx_ovt/*=150*/,float dy_ovt/*=150*/,float gmax_offsetx/*=2500*/,float gmax_offsety/*=2500*/ )
{
	//根据实际数据设置窗口大小
	int wd_w=size().width();
	int wd_h=size().height();
	int cw_w=ui.centralWidget->size().width();
	int h0=ui.centralWidget->size().height();
	int w0=(h0*nx_ovt*dx_ovt)/(ny_ovt*dy_ovt)+(wd_w-cw_w);
	resize(w0,wd_h);
	//////////////////////////////////传递参数////////////////////////////////////////	
	offsetarray_ovt=offset_ovt;//指针赋值
	azimutharray_ovt=azimuth_ovt;//指针赋值
	Nx_ovt=nx_ovt;//横向OVT片数
	Ny_ovt=ny_ovt;//纵向OVT片数
	Dx_ovt=dx_ovt;//横向OVT片长度
	Dy_ovt=dy_ovt;//纵向OVT片长度
	global_maximum_offsetx=gmax_offsetx;//纵向最大炮检距分量（模）
	global_maximum_offsety=gmax_offsety;//横向最大炮检距分量（模）
	//////////////////////////////////计算相关OVT属性//////////////////////////////////
	//申请数据空间
	gAlloc2(tolerance_offset,Nx_ovt,Ny_ovt);
	gAlloc2(tolerance_azimuth,Nx_ovt,Ny_ovt);
	gAlloc2(min_offset,Nx_ovt,Ny_ovt);
	gAlloc2(min_azimuth,Nx_ovt,Ny_ovt);
	gAlloc2(max_offset,Nx_ovt,Ny_ovt);
	gAlloc2(max_azimuth,Nx_ovt,Ny_ovt);
	gAlloc2(average_offset,Nx_ovt,Ny_ovt);
	gAlloc2(average_azimuth,Nx_ovt,Ny_ovt);
	gAlloc2(offset_count,Nx_ovt,Ny_ovt);
	//计算相关OVT属性：最大、最小、均值和容差
	for (int ix=0; ix<Nx_ovt; ix++)
	{
		for (int iy=0; iy<Ny_ovt; iy++)
		{//遍历每一OVT片，求取每一个OVT片的属性
			int mcount=0;//计数变量
			double temp_sum_offset=0, temp_sum_azimuth=0;//累加中间变量
			int minv_offset=LONG_MAX, maxv_offset=LONG_MIN;
			int minv_azimuth=LONG_MAX, maxv_azimuth=LONG_MIN;
			for (int iz=0; iz<offsetarray_ovt[ix][iy].count(); iz++)//方位角和炮检距是成对出现的
			{
				if (offsetarray_ovt[ix][iy][iz]>maxv_offset)
				{
					maxv_offset=offsetarray_ovt[ix][iy][iz];
				}
				if (azimutharray_ovt[ix][iy][iz]>maxv_azimuth)
				{
					maxv_azimuth=azimutharray_ovt[ix][iy][iz];
				}

				if (offsetarray_ovt[ix][iy][iz]<minv_offset)
				{
					minv_offset=offsetarray_ovt[ix][iy][iz];
				}
				if (azimutharray_ovt[ix][iy][iz]<minv_azimuth)
				{
					minv_azimuth=azimutharray_ovt[ix][iy][iz];
				}
				//累加属性
				mcount += 1;
				temp_sum_offset += offsetarray_ovt[ix][iy][iz];
				temp_sum_azimuth += azimutharray_ovt[ix][iy][iz];
			}
			//容差：OVT片内最大值减去最小值
			tolerance_offset[ix][iy]=maxv_offset-minv_offset;
			tolerance_azimuth[ix][iy]=maxv_azimuth-minv_azimuth;
			//最大、最小偏移距
			min_offset[ix][iy]=minv_offset;
			max_offset[ix][iy]=maxv_offset;
			min_azimuth[ix][iy]=minv_azimuth;
			max_azimuth[ix][iy]=maxv_azimuth;
			//均值
			average_azimuth[ix][iy]=short(temp_sum_azimuth/mcount);
			average_offset[ix][iy]=short(temp_sum_offset/mcount);
			//炮检距道数
			offset_count[ix][iy]=mcount;
		}
	}
}

void OVTVolumeAttributeView::HeatmapPlotting(short **OVT_Vol_Attribute, int nx, int ny, QString attributeName/*="AttributeName"*/, int mMark/*=0*/ )
{
	//计算绘图属性的值域、均值
	float OVT_count=nx*ny;
	float Attribute_Average=0; 
	double min_data=OVT_Vol_Attribute[0][0];
	double max_data=OVT_Vol_Attribute[0][0];
	for (int ix=0; ix<nx; ix++)
	{
		for (int iy=0; iy<ny; iy++)
		{
			if(OVT_Vol_Attribute[ix][iy]<min_data) min_data=OVT_Vol_Attribute[ix][iy];
			if(OVT_Vol_Attribute[ix][iy]>max_data) max_data=OVT_Vol_Attribute[ix][iy];
			Attribute_Average += OVT_Vol_Attribute[ix][iy]/OVT_count;
		}
	}
	//计算总体的离散系数（变异系数）
	float Attribute_Stand_Deviation=0;//标准差（Standard Deviation）
	for (int ix=0; ix<nx; ix++)
	{
		for (int iy=0; iy<ny; iy++)
		{
			float diffValue=OVT_Vol_Attribute[ix][iy]-Attribute_Average;
			Attribute_Stand_Deviation += powf(diffValue,2)/OVT_count;
		}
	}
	float Coef_Variation=sqrtf(Attribute_Stand_Deviation)/Attribute_Average; 
	//CV(Coefficient of Variation):标准差与均值的比率。事实上，可以认为变异系数和极差、标准差和方差一样，都是反映数据离散程度的绝对值。
	
	//窗口显示判断
	if (!ui.mCustomPlot->axisRect()->visible())
	{
		ui.mCustomPlot->axisRect()->setVisible(true);//显示绘图矩形窗口
	}
	//绘图设置
	QCustomPlot* customPlot  = ui.mCustomPlot;  //这个是我添加的绘图控件：自己的在ui文件中控件的类名 
	customPlot->plotLayout()->clear();//清除默认的axis rectangle
	//创建子布局并添加进总体布局
	QCPLayoutGrid *centralLayout = new QCPLayoutGrid;
	customPlot->plotLayout()->addElement(0, 0, centralLayout);
	//添加一个axis rect in the central layout
	QCPAxisRect *mAxisRect=new QCPAxisRect(customPlot);
	centralLayout->addElement(0,0,mAxisRect);
	
	//customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);//设置交互动作标识，激活对应的交互功能

#pragma region "绘制OVT体属性"
	//x轴设置（实际炮检距数据（字符串））
	QSharedPointer<QCPAxisTickerText> textTicker_x(new QCPAxisTickerText);
	int max_xi=int(global_maximum_offsetx/Dx_ovt)+1; //四舍五入
	if (max_xi > nx/2)//自定义坐标轴标签
	{
		textTicker_x->addTick(0,"0");
		textTicker_x->addTick(nx/2,QString::number((nx*Dx_ovt)/2));
		textTicker_x->addTick(-nx/2,QString::number(-(nx*Dx_ovt)/2));
		textTicker_x->addTick(max_xi,QString::number(max_xi*Dx_ovt));
		textTicker_x->addTick(-max_xi,QString::number(-max_xi*Dx_ovt));
	}
	else
	{
		textTicker_x->addTick(0,"0");
		textTicker_x->addTick(nx/2,QString::number((nx*Dx_ovt)/2));
		textTicker_x->addTick(-nx/2,QString::number(-(nx*Dx_ovt)/2));
	}

	mAxisRect->axis(QCPAxis::atBottom)->setTicker(textTicker_x);	//应用自定义整形ticker,防止使用放大功能时出现相同的x刻度值
	//mAxisRect->axis(QCPAxis::atBottom)->ticker()->setTickCount(3);//刻度数量-程序会自动计算,取最大值max(设置数值,计算数值)
	mAxisRect->axis(QCPAxis::atBottom)->setLabel("Inline (m)");     //设置标签
	mAxisRect->axis(QCPAxis::atBottom)->setLabelFont(QFont("Microsoft YaHei",8));//设置标签字体、大小，font().family()：SimSun
	//mAxisRect->axis(QCPAxis::atBottom)->setSubTicks(false);			//设置子刻度隐藏
	//mAxisRect->axis(QCPAxis::atBottom)->setSubTickLength(2,0);		//子刻度长度
	mAxisRect->axis(QCPAxis::atBottom)->setTickLength(5,0);				//主刻度长度

	//y轴设置
	QSharedPointer<QCPAxisTickerText> textTicker_y(new QCPAxisTickerText);
	int max_yi=int(global_maximum_offsety/Dy_ovt)+1; //四舍五入
	if (max_yi > ny/2)//自定义坐标轴标签
	{
		textTicker_y->addTick(0,"0");
		textTicker_y->addTick(ny/2,QString::number((ny*Dy_ovt)/2));
		textTicker_y->addTick(-ny/2,QString::number(-(ny*Dy_ovt)/2));
		textTicker_y->addTick(max_yi,QString::number(max_yi*Dy_ovt));
		textTicker_y->addTick(-max_yi,QString::number(-max_yi*Dy_ovt));
	}
	else
	{
		textTicker_y->addTick(0,"0");
		textTicker_y->addTick(ny/2,QString::number((ny*Dy_ovt)/2));
		textTicker_y->addTick(-ny/2,QString::number(-(ny*Dy_ovt)/2));
	}

	mAxisRect->axis(QCPAxis::atLeft)->setTicker(textTicker_y);
	mAxisRect->axis(QCPAxis::atLeft)->setLabel("Crossline (m)");
	mAxisRect->axis(QCPAxis::atLeft)->setLabelFont(QFont("Microsoft YaHei",8));//设置标签字体、大小，font().family()：SimSun
	mAxisRect->axis(QCPAxis::atLeft)->setTickLength(5,0);

	//---------设置网格线格式
	mAxisRect->axis(QCPAxis::atLeft)->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));//设置主刻度线网格线显示,设置为Qt::NoPen就不会显示 网格灰色虚线
	mAxisRect->axis(QCPAxis::atBottom)->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));//设置主刻度线网格线显示,设置为Qt::NoPen就不会显示 网格灰色虚线
	mAxisRect->axis(QCPAxis::atLeft)->grid()->setSubGridVisible(false);	//设置y轴子刻度网格线是否可见：不可见
	mAxisRect->axis(QCPAxis::atBottom)->grid()->setSubGridVisible(false);	//设置x轴子刻度网格线是否可见：不可见
	//mAxisRect->axis(QCPAxis::atLeft)->grid()->setSubGridPen(QPen(QColor(50, 50, 50), 1, Qt::DotLine));//网格浅色点线
	//mAxisRect->axis(QCPAxis::atBottom)->grid()->setSubGridPen(QPen(QColor(50, 50, 50), 1, Qt::DotLine));//网格浅色点线

	//零值网格线单独设置格式
	mAxisRect->axis(QCPAxis::atLeft)->grid()->setZeroLinePen(QPen(Qt::black, 1.5, Qt::SolidLine));//x轴0线黑色
	mAxisRect->axis(QCPAxis::atBottom)->grid()->setZeroLinePen(QPen(Qt::black, 1.5, Qt::SolidLine));//y轴0线黑色
	//设置上、下/左、右坐标轴同时显示
	mAxisRect->setupFullAxesBox(true);

	mAxisRect->axis(QCPAxis::atTop)->setTicks(false);			//设置主刻度隐藏
	mAxisRect->axis(QCPAxis::atRight)->setTicks(false);			//设置主刻度隐藏
	mAxisRect->axis(QCPAxis::atTop)->setSubTicks(false);		//设置子刻度隐藏
	mAxisRect->axis(QCPAxis::atRight)->setSubTicks(false);		//设置子刻度隐藏

	QCPColorMap *heatmap_c = new QCPColorMap(mAxisRect->axis(QCPAxis::atBottom),mAxisRect->axis(QCPAxis::atLeft));   //构造一个颜色图
	heatmap_c->data()->setSize(nx,ny);   //设置颜色图数据维度,代表你的数据矩阵(10x10),这里可以理解为有多少个小方块
	double cell_minx=double(1-nx)/2;
	double cell_maxx=double(nx-1)/2;
	double cell_miny=double(1-ny)/2;
	double cell_maxy=double(ny-1)/2;
	heatmap_c->data()->setRange(QCPRange(cell_minx,cell_maxx), QCPRange(cell_miny,cell_maxy));   //颜色图在x、y轴上的范围,这里可以查看下setRange函数定义
	//注意这里数据范围很有讲究，长度等于X,Y方向方格个数减一，即NX-1,NY-1，这样才能保证每一个方格长度为1个单位长度。刘伟
	for(int x=0;x<nx;x++)
	{
		for(int y=0;y<ny;y++)
		{
			int z = OVT_Vol_Attribute[x][y];
			if(z)
			{
				heatmap_c->data()->setCell(x, y, z);    //如果z不为0,则设置颜色值的位置
			}
			else
			{
				//heatmap_c->data()->setAlpha(x, y, 0);   //z为0,设置为透明,根据需求来设置
				//角度值可能为零
			}
		}
	}

	QCPColorScale *colorScale = new QCPColorScale(customPlot);   //构造一个色条
	colorScale->setType(QCPAxis::atRight);                      //刻度为垂直条，刻度线/坐标轴标签右侧（实际上，右侧已经是默认值）
	customPlot->plotLayout()->addElement(0, 1, colorScale);      //在颜色图下面显示
	if(mMark){//炮检距属性
		colorScale->axis()->setLabel(tr("Distance (m)"));			//设置色标标题	
	}
	else{//方位角属性
		colorScale->axis()->setLabel(tr("Azimuth")+QString::fromLocal8Bit("(°)"));				//设置色标标题
	}
	colorScale->axis()->setLabelFont(QFont("SimSun",10,QFont::Normal));	
	heatmap_c->setColorScale(colorScale);                        //设置色条——自定义色标
	QCPColorGradient gradient;                                   //自定义色标-颜色渐变
	gradient.setColorStopAt(0.0, QColor("#1800CC"));             //设置渐变在指定位置的颜色(从0到1),最小为0.0
	gradient.setColorStopAt(0.25, QColor("#24DCF5"));             //设置色条分段的颜色,0.25
	gradient.setColorStopAt(0.5, QColor("#24F52B"));             //设置色条分段的颜色,0.50
	gradient.setColorStopAt(0.75, QColor("#F1F524"));             //设置色条分段的颜色,0.75
	gradient.setColorStopAt(1.0, QColor("#FF0000"));             //设置色条结束时的颜色,最大为1.0
	heatmap_c->setGradient(gradient);                              //使用自定义渐变色标
	//heatmap_c->setGradient(QCPColorGradient::gpHues);				   //使用内置渐变色标
	heatmap_c->setDataRange(QCPRange(min_data, max_data));         //*****设置数据Z范围*****
	//heatmap_c->rescaleDataRange(true);                             //自动计算数据范围，数据范围决定了哪些数据值映射到QCPColorGradient的颜色渐变当中

	heatmap_c->setInterpolate(false);	//禁用插值,界面显示小方块
	heatmap_c->setTightBoundary(false);	//禁用裁剪：Sets whether the outer most data rows and columns are clipped to the specified key and value range	

	//保持色条与轴矩形边距一致
	//QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
	//mAxisRect->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
	//colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
	//mAxisRect->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
	//colorScale->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);

	/************************************************************************/
	/*When the zoom factor is greater than one, scrolling the mouse wheel backwards (towards the user)
	will zoom in (make the currently visible range smaller). For zoom factors smaller than one, the
	same scrolling direction will zoom out.*/
	/************************************************************************/
	//customPlot->axisRect()->setRangeZoomFactor(1.2,1.2);//x、y方向伸缩比例一致，但是不能设置为1（要么小于1，要么大于1）；默认情况是x、y两方向设置了伸缩系数(0.85,0.85),故一般不用调用这个函数

	//重新设置x、y轴的刻度（显示）范围
	mAxisRect->axis(QCPAxis::atBottom)->setRange(-max_xi -1,max_xi +1);//与函数(setRangeLower,setRangeUpper)功能一致
	mAxisRect->axis(QCPAxis::atLeft)->setRange(-max_yi -1,max_yi +1);//与函数(setRangeLower,setRangeUpper)功能一致
	//customPlot->rescaleAxes();//Rescales the axes such that all plottables (like graphs) in the plot are fully visible.
	//该函数会覆盖参数设置：customPlot->yAxis->setRange();customPlot->xAxis->setRange();因此，colormap绘图最好不采用rescaleAxes()函数,而是采用setRange()函数单独设置。
	//添加地块标题
	// first we create and prepare a text layout element:
	QCPTextElement *title=new QCPTextElement(customPlot);
	QString titleStr;
	if(mMark){//炮检距属性
		titleStr=QString::fromLocal8Bit("炮检距的OVT体属性：")+attributeName;
	}
	else{//方位角属性
		titleStr=QString::fromLocal8Bit("方位角的OVT体属性：")+attributeName;
	}
	title->setText(titleStr);
	QFont mfont("Microsoft YaHei UI", 12, QFont::Bold);
	mfont.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	title->setFont(mfont);
	// then we add it to the main plot layout:
	centralLayout->insertRow(0);// insert an empty row above the axis rect
	centralLayout->addElement(0,0,title);// place the title in the empty cell we've just created

#pragma endregion "绘制OVT体属性"

	//状态栏显示最大最小值和整体变异系数-折中的一个方案
	QString MaxMinstr=QString("Offset (Max:%1, Min:%2)").arg(QString::number(max_data)).arg(QString::number(min_data));
	QString coefVariancestr=QString("Coefficient of Variation:%1").arg(QString::number(Coef_Variation));
	QString DisplayedStr=MaxMinstr+"; "+coefVariancestr;
	ui.statusBar->showMessage(DisplayedStr,MAX_INT32);//显示一个很长的时间

	//刷新界面
	customPlot->replot(QCustomPlot::rpQueuedReplot);

}

//void OVTVolumeAttributeView::on_actionTest_ColorMap_triggered()
//{
//	ui.mCustomPlot->axisRect()->setVisible(true);
//	QCustomPlot *m_pPlotWidget = ui.mCustomPlot; //创建一个画图指针
//	QCPAxisRect *mAxisRect;//备用绘图
//	if (ui.mCustomPlot->plotLayout()->elementCount()>1)
//	{
//		ui.mCustomPlot->plotLayout()->clear();
//		mAxisRect = new QCPAxisRect(m_pPlotWidget);
//		m_pPlotWidget->plotLayout()->addElement(mAxisRect);
//	}
//	////////////////////////////////////基于官方示例修改////////////////////////////////////
//	// configure axis rect:配置轴矩形
//	m_pPlotWidget->axisRect()->setupFullAxesBox(true);
//	m_pPlotWidget->xAxis->setLabel("x");
//	m_pPlotWidget->yAxis->setLabel("y");
//
//	// set up the QCPColorMap:
//	QCPColorMap *colorMap = new QCPColorMap(m_pPlotWidget->xAxis, m_pPlotWidget->yAxis);
//	colorMap->setName("测试名称");
//	int nx = 200;
//	int ny = 200;
//	colorMap->data()->setSize(nx, ny); // 我们希望彩色地图有nx*ny的数据点
//	colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // 并在键（x）和值（y）维上跨越坐标范围-4..4
//	// ：现在，我们通过访问颜色贴图的QCPColorMapData实例来分配一些数据：
//	double x, y, z;
//	for (int xIndex=0; xIndex<nx; ++xIndex)
//	{
//		for (int yIndex=0; yIndex<ny; ++yIndex)
//		{
//			colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
//			double r = 3*qSqrt(x*x+y*y)+1e-2;
//			z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
//			colorMap->data()->setCell(xIndex, yIndex, z);
//		}
//	}
//
//	// 添加色标:
//	QCPColorScale *colorScale = new QCPColorScale(m_pPlotWidget);
//	colorScale->setType(QCPAxis::atRight); // 刻度应为垂直条，刻度线/坐标轴标签右侧（实际上，右侧已经是默认值）->setTicker(textTicker_color)
//	colorScale->axis()->setLabel("Magnetic Field Strength");
//	//QSharedPointer<QCPAxisTickerText> textTicker_color(new QCPAxisTickerText);
//	colorScale->axis()->ticker()->setTickCount(5);//主刻度5个
//	colorScale->axis()->ticker()->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);//数量设置优先;QCPAxisTicker::tssReadability,可读性优先
//	
//	m_pPlotWidget->plotLayout()->addElement(0, 1, colorScale); // 将其添加到主轴矩形的右侧
//	colorMap->setColorScale(colorScale); // 将颜色图与色标关联
//
//	// 将颜色贴图的“颜色渐变”设置为其中一个预设，QCPColorGradient预定义了9种颜色渐变（详见源文件）
//	colorMap->setGradient(QCPColorGradient::gpPolar);
//	// 我们还可以创建一个QCPColorGradient实例并向其中添加自己的颜色
//	// 渐变，请参阅QCPColorGradient的文档以获取可能的效果.
//
//	// 重新缩放数据维度（颜色），以使所有数据点都位于颜色渐变显示的范围内:
//	colorMap->rescaleDataRange();
//	//禁用插值,界面显示小方块
//	colorMap->setInterpolate(false); 
//
//	//确保轴rect和色标同步其底边距和顶边距（以便它们对齐）:
//	QCPMarginGroup *marginGroup = new QCPMarginGroup(m_pPlotWidget);
//	m_pPlotWidget->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
//	colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
//
//#pragma region “设置坐标轴字体、颜色”
//	QFont font;
//	font.setPixelSize(14);
//
//	m_pPlotWidget->xAxis->setLabelFont(font);
//	m_pPlotWidget->yAxis->setLabelFont(font);
//
//	font.setPixelSize(14);
//	m_pPlotWidget->xAxis->setTickLabelFont(font);
//	m_pPlotWidget->yAxis->setTickLabelFont(font);
//
//
//	m_pPlotWidget->xAxis->setLabelColor(QColor(255, 0, 0));
//	m_pPlotWidget->xAxis->setTickLabelColor(QColor(255, 0, 0));
//	m_pPlotWidget->yAxis->setLabelColor(QColor(255, 0, 0));
//	m_pPlotWidget->yAxis->setTickLabelColor(QColor(255, 0, 0));
//
//
//	QPen pen = m_pPlotWidget->xAxis->basePen();
//	pen.setColor(QColor(255, 0, 0));
//	m_pPlotWidget->xAxis->setTickPen(pen);
//	m_pPlotWidget->xAxis->setBasePen(pen);
//	m_pPlotWidget->xAxis->setSubTickPen(pen);
//
//
//	pen = m_pPlotWidget->yAxis->basePen();
//	pen.setColor(QColor(255, 0, 0));
//	m_pPlotWidget->yAxis->setTickPen(pen);
//	m_pPlotWidget->yAxis->setBasePen(pen);
//	m_pPlotWidget->yAxis->setSubTickPen(pen);
//#pragma endregion “设置坐标轴字体、颜色”
//
//	// 立即刷新图像
//	m_pPlotWidget->rescaleAxes();//// 重新缩放键（x）和值（y）轴，以便可以看到整个颜色图：自适应大小
//	m_pPlotWidget->replot();
//
//}