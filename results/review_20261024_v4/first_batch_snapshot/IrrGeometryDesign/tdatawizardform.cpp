/*************************************************
Copyright (C), 2009-2012, 西南石油大学 
File name:     tdatawizardform.cpp
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
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextStream>
#include <QDragMoveEvent>
#include <QCloseEvent>
#include "qdialog.h"
#include "tdatawizardform.h"
#include "qlineedit.h"
#include "qpushbutton.h"
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

 TDataWizardForm::TDataWizardForm(QWidget *parent, Qt::WFlags flags)
	 	 : QDialog(parent, flags)   //调用构造函数
{
	ui.setupUi(this);
	mCurrentStep = 0;
//	mpProject = NULL;
	mpProject=GProject::getCurrentProject();
	ui.mStepLabel->setText( tr("Step") +QString("%1").arg(mCurrentStep+1) );
	setUpdatesEnabled( true );
	setMouseTracking(true);
	ui.mNextBtn->setText( tr("&Finish") );   //完成
	//ui.mSPSMemo->setVisible(true);
	//ui.mSEGYProgressBar->setVisible(true);

}

TDataWizardForm::~TDataWizardForm()
{
	
}

void TDataWizardForm::closeEvent ( QCloseEvent * e )  //声明关闭函数  指针类型
{
	if( result() !=	QDialog::Accepted )   //判断result() !=	QDialog::Accepted
	{
		if(mpProject != NULL) mpProject->reLoad();
	}
	e->accept();   //赋值
}

void TDataWizardForm::loadBtnClicked()   //声明载入按钮点击函数
{
	if(mpProject == NULL) return;  //判断 返回
	if(mCurrentStep == 0)
	{
		//mpProject->scanForShotRecorderRange(ui.mSEGYFileNameEdit->text(),   
		//	ui.mSEGYProgressBar,ui.mSPSMemo);   //扫描并读取SEGY数据
		QString sFileName = ui.mSFileNameEdit->text();
		QString rFileName = ui.mRFileNameEdit->text();
		QString xFileName = ui.mXFileNameEdit->text();

		if(mpProject->appendSPSInfo(sFileName, rFileName, xFileName,
			ui.mSPSMemo, ui.mSEGYProgressBar))//扫描并读取SPS文件数据
		{
			ui.mLoadBtn->setEnabled( false );  //设置载入按钮不允许
			ui.mNextBtn->setEnabled( true );   //设置下一按钮允许
		}
		
	}
}


void TDataWizardForm::cancelBtnClicked()   //声明取消按钮点击函数
{
	reject();
}

void TDataWizardForm::nextBtnClicked()
{
		//<1>重整数据结构
		mpProject->refresh();	
		////////////////////////////////////////////////根据不参评检波点进行更新
		//<2>写入工程文件
		mpProject->createFolders();  //for debug
		mpProject->save();
        appendLoQFile();   //调用appendLoQFile函数
		accept();
}

void TDataWizardForm::selSFileBtnClicked()   //声明selSFileBtnClicked成员函数
{
	QString fileName = Q3FileDialog::getOpenFileName(
		QString::null, tr("Shot files (*.s);;Shot files (*.sps);;All files (*.*)"),
		this, "open shot file" );
	if( fileName != QString::null )   //判断fileName != QString::null
	{
		ui.mSFileNameEdit->setText( fileName );
		QFileInfo fi( fileName );
		QString tempS = fi.completeSuffix();
		if(tempS=="s"||tempS=="S")   //判断tempS=="s"||tempS=="S"
		{
			QFile file;
			QString name = fi.dirPath()+"/"+fi.baseName(true)+".r";   //文件类型.r
			file.setName( name );
			if( file.exists() ) ui.mRFileNameEdit->setText( name );
			name = fi.dirPath()+"/"+fi.baseName(true)+".x";     //文件类型.x
			file.setName( name );
			if( file.exists() ) ui.mXFileNameEdit->setText( name );
		}
		else
		{
			QFile file;
			QString name = fi.dirPath()+"/"+fi.baseName(true)+".rps"; //文件类型.rps
			file.setName( name ); 
			if( file.exists() ) ui.mRFileNameEdit->setText( name );
			name = fi.dirPath()+"/"+fi.baseName(true)+".xps";    //文件类型.xps
			file.setName( name );
			if( file.exists() ) ui.mXFileNameEdit->setText( name );
		}
	}
			
}



void TDataWizardForm::selXFileBtnClicked()  //声明selXFileBtnClicked成员函数
{
	QString fileName = Q3FileDialog::getOpenFileName(
		QString::null, tr("Relationship files (*.x);;Relationship files (*.xps);;All files (*.*)"),
		this, "Open Relationship File" );
	if( fileName != QString::null )    //判断fileName != QString::null
	{
		ui.mXFileNameEdit->setText( fileName );
	}
}


//void TDataWizardForm::selSEGYFileBtnClicked()   //声明selSEGYFileBtnClicked成员函数
//{
//	QString fileName = Q3FileDialog::getOpenFileName(
//		QString::null, tr("SEG-Y Files (*.sgy);;All files (*.*)"),
//		this, "Open SEG-Y File" );   
//	if( fileName != QString::null )    //判断fileName != QString::null
//	{
//		ui.mSEGYFileNameEdit->setText( fileName );
//	}
//}


void TDataWizardForm::setProject(GProject *pProject)   //设置工程函数  指针类型
{
	mpProject = pProject;
}

void TDataWizardForm::appendLoQFile()
{
//	QFile spsFile( mpProject->getSPSLoQFileName() );
	QFile spsFile( mpProject->getSPSLogFileName() );
	if( !spsFile.open(QIODevice::WriteOnly | QIODevice::Append) ) return ;

	QTextStream spsStream( &spsFile );
	spsStream << QString::fromLocal8Bit("\n\n\n//----载入时间 ");  //载入时间
	spsStream << QDateTime::currentDateTime().toString();
	spsStream << "----//\r\n";  //换行
	spsStream << ui.mSPSMemo->text();
	spsFile.close();

//	QFile sgyFile( mpProject->getSGYLoQFileName() );
	QFile sgyFile( mpProject->getSGYLogFileName() );
	if( !sgyFile.open(QIODevice::WriteOnly | QIODevice::Append) ) return ; //载入时间

	QTextStream sgyStream( &sgyFile );
	sgyStream << QString::fromLocal8Bit("\n\n\n//----载入时间 ");
	sgyStream << QDateTime::currentDateTime().toString();
	sgyStream << "----//\r\n";  //换行
	sgyStream << "SEGYMemo";//ui.mSEGYMemo->text(),刘伟
	sgyFile.close();
}


void TDataWizardForm::selectRFile()
{
   QString fileName = Q3FileDialog::getOpenFileName(
		QString::null, tr("R files (*.r);;R files (*.rps);;All files (*.*)"),
		this, "open r file" );
	if( fileName != QString::null )    //判断fileName != QString::null
	{
		ui.mRFileNameEdit->setText( fileName );		
	}
			
}

void TDataWizardForm::dragEnterEvent ( QDragEnterEvent * )   //定义公有dragEnterEvent成员函数 指针变量
{
	//repaint();
}

void TDataWizardForm::dragMoveEvent ( QDragMoveEvent * )  //定义公有ragMoveEvent成员函数 指针变量
{
	//repaint();
}

void TDataWizardForm::dropEvent ( QDropEvent * )   //定义公有dropEvent成员函数 指针变量
{
//		repaint();
////		bigVisualChanges();
//		setUpdatesEnabled( TRUE );
//        repaint();
}


//void TDataWizardForm::on_mNextBtn_clicked()  //定义私有on_mNextBtn_clicked成员函数
//{
//
//}
//
//void TDataWizardForm::on_cancelBtn_clicked()   //定义私有on_cancelBtn_clicked成员函数
//{
//
//}
//
//void TDataWizardForm::on_mLoadBtn_clicked()
//{
//
//}