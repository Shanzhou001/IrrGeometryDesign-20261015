#ifndef ANALYZEBINFORM_H
#define ANALYZEBINFORM_H

#include <QMainWindow>
#include "ui_analyzebinform.h"
#include "analyzebindoc.h"
#include "analyzebinview.h"
#include "gvaluevector.h"
// 观测系统属性绘制窗口类
// [11/3/2022 Liu Wei]基于该类重新定义绘制OVT体属性的绘制函数
class analyzebinform : public QMainWindow
{
	Q_OBJECT

public:
	analyzebinform(QWidget *parent = 0);
	~analyzebinform();

private:
	Ui::analyzebinformClass ui;

private slots:

	//void on_actionChangeShowarea_triggered();//改变炮检距、方位角非均匀性系数，采集脚印显示范围
	//void on_actionQFDivide_triggered();//综合质量因子统计重新划分
	//void on_actionRoseDivide_triggered();//显示玫瑰图，随意输入分割次数

	void on_actionGeometryImformation_triggered();//观测系统信息显示板
	void on_actionCSQFactor_triggered();//观测系统设计质量系数
	void on_actionFootFactor_triggered();//采集脚印系数
	void on_actionAzimuthFactor_triggered();//方位角非均匀性系数显示
	void on_actionOffsetFactor_triggered();//炮检距非均匀性系数显示
	void on_Show_previous_point_triggered();//显示上一个
	void on_Show_next_point_triggered();//显示下一个
	void on_Bin_Fold_triggered();//覆盖次数
	void on_Offset_Fold_Bins_triggered();//柱状图
	void on_Rose_Traces_triggered();//玫瑰花图
	void on_Azimuth_Traces_triggered();//方位角-炮检对个数
	void on_Offset_Traces_triggered();//炮检距（偏移距）-炮检对个数
	void on_Fold_Bins_triggered();//覆盖次数-面元数量
private:
	analyzebindoc *mpBinDoc;
	analyzebinview *mpBinView;
private:
	bool in_loop ;
	int CMPxbinnumber,CMPybinnumber,CMPmaxfold;
	GValueVector<short> **offsetarray;	
	GValueVector<short> **azimutharray;
	int CMPdlgmaxoffset;
	bool CMPvalueanalyzebinWorkArea;//分析工区
	int *CMPFoldVSBins;//覆盖次数-面元数量数组
	int *CMPOffsetVSTraces;//炮检距-炮检对
	int **CMPOffsetVSFoldBins;//炮检距-覆盖次数-面元数量
	int *CMPAzimuthVSTraces;//方位角-炮检对
	int **CMPmeiguiVStraces;//玫瑰花图-炮检对
	int **CMPBinfold;//面元覆盖次数

	float **m_dataNonuniform;//存放非均匀性系数
	float **m_dataAngle;//存放方位角非均匀性系数
	float **m_dataFoot;//存放采集脚印数组
	float **m_dataCSDFPercent;//存放观测系统设计面元所占比例数组*100
	float m_maxNonuniformfactor;//存放最大的那个系数
	float m_minNonuniformfactor;
	float m_maxAnglefactor;//记录最大的系数
	float m_minAnglefactor;//记录最小的系数
	float m_maxFootfactor;//记录最大的系数
	float m_minFootfactor;//记录最小的系数
	float m_maxCSDFPfactor;//记录面元比例系数（百分比）
	float m_minCSDFPfactor;
	float m_CSQFfactor;//观测系统质量系数
	float mBinx,mBiny;//面元尺寸
	float mCircleR;//环形半径
	int mAmizthNum,mOffsetNum;//方位角和炮检距分的段
	int mShowAzimuth,mShowOffset;
	int mBaseDivAmizth,mBaseDivOffset;
	QString mTextAmizth,mTextOffset;
	int lxRose;//玫瑰图炮检距分段数
	int lyRose;//玫瑰图方位角分段数
	int lxzhu;//柱状图分段
	int maxzhufold;

	int lxCSQF;//质量因子炮检距分段数
	int lyCSQF;//质量因子方位角分段数
	int maxoffsetCSQF;//质量因子统计最大炮检距
	bool bShowOffset;//显示炮检距非均匀系数的标准，显示true，不显示false
	bool bShowangle;
	bool bShowFoot;

	void setData3(bool i);
	void setData(int **data);
	void setData2(int **data2);
	void setData4(int *i);
	void setDatatracespac(int i);
	void setDatapoint(QPoint point);
	void setDataMeiguiVSTraces(int **MeiguiVSTraces);//传玫瑰花图-炮检对
	void setDataBinfold(int **Binfold);//
	void setDataOffsetVSFoldBins(int **OffsetVSFoldBins);//传炮检距-覆盖次数-面元数量
	void setDatamaxoffset(int maxoffset);//传最
	void setDataXbinnumber(int xbinnumber);//传最
	void setDataYbinnumber(int ybinnumber);//传最
	void setDataOffsetVSTraces(int *OffsetVSTraces);// 传炮检距-炮检对数
	void setDataAzimuthVSTraces(int *AzimuthVSTraces);// 传-炮检对数组
	void setDataFoldVSBins(int *FoldVSBins);
	void setDatamaxfold(int maxfold);//传最大

	void setDatalineOffsetVSTraces(int *lineOffsetVSTraces);// 传线面元炮检距-炮检对数组
	void setDatalineAzimuthVSTraces(int *lineAzimuthVSTraces);// 传线面元方位角-炮检对数组
	void setDatalineFoldVSBins(int *lineFoldVSBins);//线面元
	void setDatalineOffsetVSFoldBins(int **lineOffsetVSFoldBins);//传炮检距-覆盖次数-面元数量
	void setDatalineMeiguiVSTraces(int **lineMeiguiVSTraces);//传玫瑰花图-炮检对
	void setDatalinecoordinate(QPoint linecoordinate);
	void setDataValueline(int valueline);//画
	void computeNonuniformfactorfun(GValueVector<short> **array,int xl,int yl,int zl,int foldup,int folddown);//输入炮检距数据计算非均匀性系数（计算函数）
	void computeAnglefactorfun(GValueVector<short> **array,int xl,int yl,int zl,int foldup,int folddown);//输入方位角数据计算非均匀性系数（计算函数）
	void Footprintfun(GValueVector<short> **offset,int xline,int yline,int zline,int foldup,int folddown);//采集脚印计算函数
	void CSDFactorfun(GValueVector<short> **offset,GValueVector<short> **azimuth,int xline,int yline,int zline,float maxoffst);//观测系统设计质量系数函数
public:
	int exec();
public:
	void setAnalyzedata(GValueVector<short> **offset1,GValueVector<short> **azimuth1,int xline,int yline,int zline,float bx,float by,float CircleR)
	{
		offsetarray=offset1;
		azimutharray=azimuth1;
		CMPxbinnumber=xline;
		CMPybinnumber=yline;
		CMPmaxfold=zline;
		mBinx=bx;mBiny=by;
		mCircleR=CircleR;//环形半径
	}
	void setAODividNum(int AmizthNum,int OffsetNum,int ShowAzimuth,int ShowOffset,int BaseDivAmizth,int BaseDivOffset,QString TextAmizth,QString TextOffset)//方位角和炮检距分的段
	{
		mAmizthNum=AmizthNum;mOffsetNum=OffsetNum;
		mShowAzimuth=ShowAzimuth;mShowOffset=ShowOffset;
		mBaseDivAmizth=BaseDivAmizth;mBaseDivOffset=BaseDivOffset;
		mTextAmizth=TextAmizth;mTextOffset=TextOffset;
	}
	void CMPLayoutAttibuteConculate(int **foldData);//CMP属性分析-观测系统属性计算

	void setDataValueworkarea(int valueworkarea);
	

};

#endif // ANALYZEBINFORM_H
