/*************************************************
Copyright (C), 2009-2012, 西南石油大学 
File name:     tdatawizardform.h
Author:  潘树林     Version:  1.0     Date:  2011.1
Description:   
** Form interface generated from reading ui file 'tools\tselshotform.ui'
**
** Created: 鏄熸湡鏃?涓夋湀 14 21:23:19 2010
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
Function List:  	
1、	GDrawSCharGraphDoc              //构造函数
2、	~GDrawSCharGraphDoc()             //析构函数
3、 gLineDenoise                    //自动分析折射道主函数
4、 gWaveletDenoise                 //自动分析面波道主函数
4、 g50HZDenoise                    //自动分析50HZ地震道主函数
5、 gDummyDenoise                   //自动分析空道主函数
6、 get_windows_data                //获取时窗内数据
7、 get_dif_sum                     //确定干扰斜率
8、 denoise_sample                  //相关判断
9、allocMatrix3                    //分配三维数组函数
10、destroyMatrix3                 //注销三维数据函数
11、  correct                       //校正数据函数
12、 correctDoing                   //去强折射干扰函数
13、faceDoing                       //去面波函数
14、 dwt                            //小波变换正变换
15、 idwt                           //小波变换反变换
16、 fft                            //快速傅里叶变换

*************************************************/

#ifndef TDATAWIZARDFORM_H
#define TDATAWIZARDFORM_H

#include <qvariant.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <q3progressdialog.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <q3filedialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <QDragMoveEvent>
#include <QCloseEvent>
#include "../../io/sps/gproject.h"
#include <Q3WidgetStack>
#include "ui_tdatawizardform.h"
//绫荤殑瀹氫箟
class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class Q3WidgetStack;
class QWidget;
class Q3GroupBox;
class QLineEdit;
class QToolButton;
class Q3TextEdit;
class Q3ProgressBar;
class QPushButton;

class TDataWizardForm : public QDialog   //瀹氫箟TDataWizardForm绫?鍏湁鎴愬憳QDialog
{
    Q_OBJECT

public:
    TDataWizardForm(QWidget *parent = 0, Qt::WFlags flags = 0);  //鏋勯�犲嚱鏁?
    ~TDataWizardForm();       //鏋愭瀯鍑芥暟

public slots:  
    virtual void closeEvent( QCloseEvent * e );
    virtual void loadBtnClicked();//*****导入函数*****
    virtual void cancelBtnClicked();
    virtual void nextBtnClicked();//Finish
    virtual void selSFileBtnClicked();
    virtual void selXFileBtnClicked();
    //virtual void selSEGYFileBtnClicked();//SGY文件
    virtual void setProject( GProject * pProject );
    virtual void appendLoQFile();
    virtual void selectRFile();
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dropEvent( QDropEvent * );



private:  
    Q_INT32 mCurrentStep;
    GProject *mpProject;

public: 
    Ui::TDataWizardForm ui;

	//QString getSEGYFileName() {return ui.mSEGYFileNameEdit_2->text();}//赵虎添加 


private slots:  
	//void on_mLoadBtn_clicked();
	//void on_cancelBtn_clicked();
	//void on_mNextBtn_clicked();
};

#endif // TDATAWIZARDFORM_H
