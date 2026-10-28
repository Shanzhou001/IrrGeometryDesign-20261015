#ifndef OVTVOLUMEATTRIBUTEVIEW_H
#define OVTVOLUMEATTRIBUTEVIEW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "ui_ovtvolumeattributeview.h"

#include "../../core/gvaluevector.h"
#include "../../core/gutils.h"
#include "../../core/gmath.h"

#include "../../../third_party/qcustomplot/qcustomplot.h"

class OVTVolumeAttributeView : public QMainWindow
{
	Q_OBJECT

public:
	OVTVolumeAttributeView(QWidget *parent = 0);
	~OVTVolumeAttributeView();

protected:
	virtual void closeEvent(QCloseEvent *e);

private:
	Ui::OVTVolumeAttributeViewClass ui;

	//////////////////////////////////////////////////////////////////////////	刘伟添加
	bool in_loop ; //窗口阻塞指示参数
	int Nx_ovt;	//纵向（检波线/测线方向）OVT片数
	int Ny_ovt;	//横向（炮线方向）OVT片数
	float Dx_ovt;  //纵向OVT片尺寸
	float Dy_ovt;  //横向OVT片尺寸
	float global_maximum_offsetx;	//纵向最大炮检距=全局最大炮检距矢量的X分量
	float global_maximum_offsety;	//最大非纵炮检距=全局最大炮检距矢量的Y分量
	GValueVector<short> **offsetarray_ovt;	//OVT体属性-偏移距，该指针变量不需要释放空间
	GValueVector<short> **azimutharray_ovt;	//OVT体属性-方位角，该指针变量不需要释放空间

	short **min_offset;			//最小炮检距
	short **max_offset;			//最大炮检距
	short **average_offset;		//平均炮检距
	short **min_azimuth;		//最小方位角
	short **max_azimuth;		//最大方位角
	short **average_azimuth;	//平均方位角
	short **tolerance_offset;	//炮检距容差
	short **tolerance_azimuth;	//方位角容差

	short **dispersion_offset;	//炮检距离散度
	short **dispersion_azimuth;	//方位角离散度

	int **offset_count;			//炮检距对数
	
	//////////////////////////////////////////////////////////////////////////	刘未添加

	private slots:

		//炮检距的OVT体属性
		void on_actionOffset_Tolerance_triggered();//计算并显示OVT容差属性
		void on_actionOffset_Dispersion_triggered();//计算并显示OVT离散度属性_标准差
		void on_actionMin_Offset_triggered();//计算并显示OVT最小值属性
		void on_actionMax_Offset_triggered();//计算并显示OVT最大值属性
		void on_actionAverage_Offset_triggered();//计算并显示OVT平均值属性

		//方位角的OVT体属性
		void on_actionAzimuth_Tolerance_triggered();//计算并计算并显示OVT容差属性
		void on_actionAzimuth_Dispersion_triggered();//计算并显示OVT离散度属性_标准差
		void on_actionMin_Azimuth_triggered();//计算并显示OVT最小值属性
		void on_actionMax_Azimuth_triggered();//计算并显示OVT最大值属性
		void on_actionAverage_Azimuth_triggered();//计算并显示OVT平均值属性

		//void on_actionTest_ColorMap_triggered();

public:
	int exec();//阻塞显示MainWindow
	//////////////////////////////////OVT体属性////////////////////////////////////////
	void setOVTcubeData(GValueVector<short> **offset_ovt,GValueVector<short> **azimuth_ovt,int nx_ovt,int ny_ovt,float dx_ovt=150,float dy_ovt=150,float gmax_offsetx=2500,float gmax_offsety=2500);//传递OVT属性计算参数

private:
	void HeatmapPlotting(short **OVT_Vol_Attribute, int nx, int ny, QString attributeName="AttributeName", int mMark=0);
};

#endif // OVTVOLUMEATTRIBUTEVIEW_H
