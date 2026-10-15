#include "analyzebindoc.h"
#include <qmessagebox.h>
#include <qlineedit.h>

analyzebindoc::analyzebindoc()
{
	mpData = NULL;
	point=30;
	valuebinfold=false;
	mpDataValueline = false;
}

analyzebindoc::~analyzebindoc()
{

}

void analyzebindoc::foldVSbins()//覆盖次数-面元数量


{
	valuefoldVSbins=true;
	valueoffsetVStraces=false;	
	valueazimuthVStraces=false;
	valueroseVStraces=false;
	valueoffsetVSfoldbins=false;
	valuebinfold=false;
}

void analyzebindoc::offsetVStraces()//炮检距-炮检对
{
	valueoffsetVStraces=true;
	valuefoldVSbins=false;
	valueazimuthVStraces=false;
	valueroseVStraces=false;
	valueoffsetVSfoldbins=false;
	valuebinfold=false;
}

void analyzebindoc::azimuthVStraces()//方位角-炮检对
{
	valueazimuthVStraces=true;
	valueoffsetVStraces=false;
	valuefoldVSbins=false;
	valueroseVStraces=false;
	valueoffsetVSfoldbins=false;
	valuebinfold=false;
}

void analyzebindoc::roseVStraces()//玫瑰花图-炮检对
{
	valueroseVStraces=true;
	valueazimuthVStraces=false;
	valueoffsetVStraces=false;
	valuefoldVSbins=false;
	valueoffsetVSfoldbins=false;
	valuebinfold=false;
}

void analyzebindoc::offsetVSfoldbins()//炮检距-覆盖次数-面元数量
{
	valueoffsetVSfoldbins=true;
	valuefoldVSbins=false;
	valueoffsetVStraces=false;	
	valueazimuthVStraces=false;
	valueroseVStraces=false;
	valuebinfold=false;
}

bool analyzebindoc::showNextpoint()// 显示后一个面元
{
	if(mpData3==1)
	{
		if(mpData3==true&&mpData4[point+1]!=0)
		{
			if(point<60)	point++;	
			else 
			{
				QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("已是记录的最后一个面元"));//消息对话框显示提示用户违法操作
				point=60;
			}
		}
		else
		{
			QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("覆盖次数为零"));//消息对话框显示提示用户违法操作
		}
		return true;
	}
	return false;
}

bool analyzebindoc::showProvouspoint()//显示后一个面元
{
	if(mpData3==1)
	{
		if(mpData4[point-1]!=0)
		{
			if(point>=1)	point--;
			else 
			{
				QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("已是记录的第一个面元"));//消息对话框显示提示用户违法操作
				point=0;
			}
		}
		else
		{
			QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("覆盖次数为零"));//消息对话框显示提示用户违法操作
		}
		return true;
	}
	return false;
}