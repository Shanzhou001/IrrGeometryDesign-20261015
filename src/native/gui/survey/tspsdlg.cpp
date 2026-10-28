/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include "../../core/gmath.h"
#include "tspsdlg.h"
#include "../../io/sps/gproject.h"

TSPSDlg::TSPSDlg( QWidget* parent ):QDialog(parent)
{
	ui.setupUi(this);
	init();
	// 人工手动修改检波器原点Y坐标，炮线和接收线参数时，联动修改炮点纵向最小坐标SY0。 [Liu Wei 6/5/2023]
	connect(ui.cRY0Edit,SIGNAL(textEdited(const QString &)),this, SLOT(AutosetSY0()));
	connect(ui.cRLineCountBox,SIGNAL(valueChanged(int)),this, SLOT(AutosetSY0()));
	connect(ui.cRLineInvEdit,SIGNAL(textEdited(const QString &)),this, SLOT(AutosetSY0()));
	connect(ui.cSPtCountBox,SIGNAL(valueChanged(int)),this, SLOT(AutosetSY0()));
	connect(ui.cSPtInvEdit,SIGNAL(textEdited(const QString &)),this, SLOT(AutosetSY0()));
	// 人工手动修改检波器原点X坐标时，联动修改炮点纵向最小坐标SX0。 [Liu Wei 7/31/2023]
	connect(ui.cRX0Edit,SIGNAL(textEdited(const QString &)),this, SLOT(AutosetSX0()));
}
TSPSDlg::~TSPSDlg()
{
	destroy();
}
void TSPSDlg::init()
{
	mpSPSParam = NULL;
	GProject *pProject = GProject::getCurrentProject();
	if (pProject && pProject->getSPSParam())
		setSPS(pProject->getSPSParam());
}

void TSPSDlg::is2Dinit()
{
	ui.cRLineCountBox->setValue(1);
	ui.cRLineCountBox->setEnabled(false);
	ui.cRLineInvEdit->setEnabled(false);
	ui.cRY0Edit->setText(QString::number(0));
	ui.cRY0Edit->setEnabled(false);
	ui.cSPtCountBox->setValue(1);
	ui.cSPtCountBox->setEnabled(false);
	ui.cSPtInvEdit->setEnabled(false);
	ui.cSY0Edit->setText(QString::number(0));
	ui.cSY0Edit->setEnabled(false);

}

void TSPSDlg::destroy()
{
}

void TSPSDlg::setSPS(GSPSParam *pSPSParam)
{
	mpSPSParam = pSPSParam;

	ui.cLMinOffEdit->setText(QString::number(mpSPSParam->mMinLOffset));
	ui.cLMaxOffEdit->setText(QString::number(mpSPSParam->mMaxLOffset));
	ui.cRMinOffEdit->setText(QString::number(mpSPSParam->mMinROffset));
	ui.cRMaxOffEdit->setText(QString::number(mpSPSParam->mMaxROffset));

	ui.cRX0Edit->setText(QString::number(mpSPSParam->mRX0,'f',1));
	ui.cRY0Edit->setText(QString::number(mpSPSParam->mRY0,'f',1));

	ui.cRLineInvEdit->setText(QString::number(mpSPSParam->mRLineInv));	
	ui.cRLineCountBox->setValue(mpSPSParam->mRLineCount);// 寻找更好的信号 [Liu Wei 6/21/2023]
	ui.cATypeBox->setCurrentItem(mpSPSParam->mSLineType);

	ui.cRPtCountBox->setValue(mpSPSParam->mRPtCount);
	ui.cRPtInvEdit->setText(QString::number(mpSPSParam->mRPtInv));

	ui.cSX0Edit->setText(QString::number(mpSPSParam->mSX0,'f',1));//默认值是0
	ui.cSY0Edit->setText(QString::number(mpSPSParam->mSY0,'f',1));//默认值是0

	ui.cSLineCountBox->setValue(mpSPSParam->mSLineCount);
	ui.cSLineInvEdit->setText(QString::number(mpSPSParam->mSLineInv));

	ui.cSPtInvEdit->setText(QString::number(mpSPSParam->mSPtInv));	
	ui.cSPtCountBox->setValue(mpSPSParam->mSPtCount);// 寻找更好的信号 [Liu Wei 6/21/2023]

	ui.cSAngleEdit->setText(QString::number((mpSPSParam->mSAf-mpSPSParam->mAf)*RADIAN_TO_ANGLE)); // RADIAN_TO_ANGLE是什么？ 答：弧度值转角度值。 [Liu Wei 5/29/2023]
	ui.cAngleEdit->setText(QString::number(mpSPSParam->mAf*RADIAN_TO_ANGLE));//初始化角度信息
	ui.cMoveX0Edit->setText(QString::number(mpSPSParam->mMoveX0));//add by zhaohu  检波点Y方向平移量（旋转成水平坐标时的值）
	ui.cMoveY0Edit->setText(QString::number(mpSPSParam->mMoveY0));//add by zhaohu  检波点Y方向平移量（旋转成水平坐标时的值）

	ui.OrginalByReceiver->setChecked(mpSPSParam->mOrginalByReceiver);//add by zhaohu 以第一个检波点为原点设计观测系统
	ui.OrginalByShot->setChecked(mpSPSParam->mOrginalByShot);//add by zhaohu 以第一个炮点为原点设计观测系统

}

void TSPSDlg::accept()
{
	mpSPSParam->mRLineCount = ui.cRLineCountBox->value();
	mpSPSParam->mRLineInv = ui.cRLineInvEdit->text().toFloat();
	mpSPSParam->mAf = ui.cAngleEdit->text().toFloat()*ANGLE_TO_RADIAN;
	mpSPSParam->mSinAf = sin(mpSPSParam->mAf);
	mpSPSParam->mCosAf = cos(mpSPSParam->mAf);
	mpSPSParam->mSLineType = (GSPSParam::GSLineType)ui.cATypeBox->currentItem();
	if(mpSPSParam->mSLineType == GSPSParam::gInLine)
	{
		mpSPSParam->mSAf = mpSPSParam->mAf;
	}
	else if(mpSPSParam->mSLineType == GSPSParam::gCrossLine)
	{
		mpSPSParam->mSAf = mpSPSParam->mAf+PI/2;
	}
	else
	{
		mpSPSParam->mSAf = mpSPSParam->mAf + ui.cSAngleEdit->text().toFloat()*ANGLE_TO_RADIAN;
	}
	
	mpSPSParam->mRX0 = ui.cRX0Edit->text().toFloat();
	mpSPSParam->mRY0 = ui.cRY0Edit->text().toFloat();
	mpSPSParam->mRPtCount = ui.cRPtCountBox->value();
	mpSPSParam->mRPtInv = ui.cRPtInvEdit->text().toFloat();

	mpSPSParam->mSX0 = ui.cSX0Edit->text().toFloat();
	mpSPSParam->mSY0 = ui.cSY0Edit->text().toFloat();
	mpSPSParam->mSLineCount = ui.cSLineCountBox->value();
	mpSPSParam->mSLineInv = ui.cSLineInvEdit->text().toFloat();
	mpSPSParam->mSPtCount = ui.cSPtCountBox->value();
	mpSPSParam->mSPtInv = ui.cSPtInvEdit->text().toFloat();

	mpSPSParam->mMinLOffset = ui.cLMinOffEdit->text().toFloat();
	mpSPSParam->mMaxLOffset = ui.cLMaxOffEdit->text().toFloat();
	mpSPSParam->mMinROffset = ui.cRMinOffEdit->text().toFloat();
	mpSPSParam->mMaxROffset = ui.cRMaxOffEdit->text().toFloat();

	mpSPSParam->mMoveX0 = ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）
	mpSPSParam->mMoveY0 = ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）

	mpSPSParam->mOrginalByReceiver=ui.OrginalByReceiver->isChecked();//add by zhaohu 以第一个检波点为原点设计观测系统
	mpSPSParam->mOrginalByShot=ui.OrginalByShot->isChecked();//add by zhaohu 以第一个炮点为原点设计观测系统

	QDialog::accept();
}

void TSPSDlg::AutosetSY0()
{
	float m_sy0=ui.cRY0Edit->text().toFloat()+
		((ui.cRLineCountBox->value()-1)*ui.cRLineInvEdit->text().toFloat()-(ui.cSPtCountBox->value()-1)*ui.cSPtInvEdit->text().toFloat())/2;
	ui.cSY0Edit->setText(QString::number(m_sy0,'f',1));
}

void TSPSDlg::AutosetSX0()
{
	float m_sx0=ui.cRX0Edit->text().toFloat()+ui.cLMaxOffEdit->text().toFloat();
	ui.cSX0Edit->setText(QString::number(m_sx0,'f',1));
}

      /*void TSPSDlg::on_cAngleEdit_textChanged(const QString &)
{
	GProject *pProject = GProject::getCurrentProject();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	if(pSPSParam->mOrginalByReceiver==true)//以第一个检波点为原点
	{
		//double beginX=pSPSParam->mRX0;
		//double beginY=pSPSParam->mRY0;	
		double beginX=ui.cRX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
		double beginY=ui.cRY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
		float Af = ui.cAngleEdit->text().toFloat()*ANGLE_TO_RADIAN;
		float SinAf = sin(Af);
		float CosAf = cos(Af);
		double rx0,ry0;
		gRotate((double)-SinAf, (double)CosAf,beginX,beginY,rx0,ry0);
		rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
		ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
		double SX0,SY0;
		gRotate((double)SinAf,(double)CosAf,rx0,ry0,SX0,SY0);//计算检波点起始坐标
		ui.cSX0Edit->setText(QString::number(SX0,'f',1));
		ui.cSY0Edit->setText(QString::number(SY0,'f',1));
	}
	if(pSPSParam->mOrginalByShot==true)//以第一个炮点为原点
	{	
		//double beginX=pSPSParam->mSX0;
		//double beginY=pSPSParam->mSY0;	
		double beginX=ui.cSX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
		double beginY=ui.cSY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
		float Af = ui.cAngleEdit->text().toFloat()*ANGLE_TO_RADIAN;
		float SinAf = sin(Af);
		float CosAf = cos(Af);
		double rx0,ry0;
		gRotate((double)-SinAf, (double)CosAf,beginX,beginY,rx0,ry0);
		rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
		ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
		double RX0,RY0;
		gRotate((double)SinAf,(double)CosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
		ui.cRX0Edit->setText(QString::number(RX0,'f',1));
		ui.cRY0Edit->setText(QString::number(RY0,'f',1));
	}

}   */
void TSPSDlg::on_cAngleEdit_textChanged(const QString &)
{
    // 缓存 UI 数值，减少重复转换开销
    float angle = ui.cAngleEdit->text().toFloat();
    float Af = angle * ANGLE_TO_RADIAN;
    float sinAf = sin(Af);
    float cosAf = cos(Af);
    float moveX = ui.cMoveX0Edit->text().toFloat();
    float moveY = ui.cMoveY0Edit->text().toFloat();
    
    // 优化：将常用的转换提前，避免重复调用 sin/cos 和文本转换
    if (GSPSParam *pSPSParam = GProject::getCurrentProject()->getSPSParam())
    {
        if(pSPSParam->mOrginalByReceiver == true) // 以第一个检波点为原点
        {
            float beginX = ui.cRX0Edit->text().toFloat();
            float beginY = ui.cRY0Edit->text().toFloat();
            double tempX, tempY;
            // 第一次旋转：将原点坐标转换
            gRotate(-sinAf, cosAf, beginX, beginY, tempX, tempY);
            // 加上平移量
            tempX += moveX;
            tempY += moveY;
            double SX0, SY0;
            // 第二次旋转：计算最终起始坐标
            gRotate(sinAf, cosAf, tempX, tempY, SX0, SY0);
            ui.cSX0Edit->setText(QString::number(SX0, 'f', 1));
            ui.cSY0Edit->setText(QString::number(SY0, 'f', 1));
        }
        if(pSPSParam->mOrginalByShot == true) // 以第一个炮点为原点
        {
            float beginX = ui.cSX0Edit->text().toFloat();
            float beginY = ui.cSY0Edit->text().toFloat();
            double tempX, tempY;
            gRotate(-sinAf, cosAf, beginX, beginY, tempX, tempY);
            tempX += moveX;
            tempY += moveY;
            double RX0, RY0;
            gRotate(sinAf, cosAf, tempX, tempY, RX0, RY0);
            ui.cRX0Edit->setText(QString::number(RX0, 'f', 1));
            ui.cRY0Edit->setText(QString::number(RY0, 'f', 1));
        }
    }
}   //通过先进行一次旋转、平移，再进行第二次旋转的方式，保持计算流程不变，但减少了重复计算和函数调用开销






void TSPSDlg::on_cMoveX0Edit_textChanged(const QString &)
{
	//GProject *pProject = GProject::getCurrentProject();
	//GSPSParam *pSPSParam = pProject->getSPSParam();
	//if(pSPSParam->mOrginalByReceiver==true)//以第一个检波点为原点
	//{
	//	//double beginX=pSPSParam->mRX0;
	//	//double beginY=pSPSParam->mRY0;	
	//	double beginX=ui.cRX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
	//	double beginY=ui.cRY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
	//	double rx0,ry0;
	//	gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
	//	rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
	//	ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
	//	double SX0,SY0;
	//	gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,SX0,SY0);//计算检波点起始坐标
	//	ui.cSX0Edit->setText(QString::number(SX0,'f',1));
	//	ui.cSY0Edit->setText(QString::number(SY0,'f',1));
	//}
	//if(pSPSParam->mOrginalByShot==true)//以第一个炮点为原点
	//{
	//	//double beginX=pSPSParam->mSX0;
	//	//double beginY=pSPSParam->mSY0;	
	//	double beginX=ui.cSX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
	//	double beginY=ui.cSY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
	//	double rx0,ry0;
	//	gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
	//	rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
	//	ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
	//	double RX0,RY0;
	//	gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
	//	ui.cRX0Edit->setText(QString::number(RX0,'f',1));
	//	ui.cRY0Edit->setText(QString::number(RY0,'f',1));
	//}
}

void TSPSDlg::on_cMoveY0Edit_textChanged(const QString &)
{
	//GProject *pProject = GProject::getCurrentProject();
	//GSPSParam *pSPSParam = pProject->getSPSParam();
	//if(pSPSParam->mOrginalByReceiver==true)//以第一个检波点为原点
	//{
	//	//double beginX=pSPSParam->mRX0;
	//	//double beginY=pSPSParam->mRY0;	
	//	double beginX=ui.cRX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
	//	double beginY=ui.cRY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
	//	double rx0,ry0;
	//	gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
	//	rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
	//	ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
	//	double SX0,SY0;
	//	gRotate((double)pSPSParam->mSinAf,(double)pSPSParam->mCosAf,rx0,ry0,SX0,SY0);//计算检波点起始坐标
	//	ui.cSX0Edit->setText(QString::number(SX0,'f',1));
	//	ui.cSY0Edit->setText(QString::number(SY0,'f',1));
	//}
	//if(pSPSParam->mOrginalByShot==true)//以第一个炮点为原点
	//{
	//	//double beginX=pSPSParam->mSX0;
	//	//double beginY=pSPSParam->mSY0;	
	//	double beginX=ui.cSX0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]
	//	double beginY=ui.cSY0Edit->text().toFloat();// 修改 [Liu Wei 6/5/2023]	
	//	double rx0,ry0;
	//	gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
	//	rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
	//	ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
	//	double RX0,RY0;
	//	gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
	//	ui.cRX0Edit->setText(QString::number(RX0,'f',1));
	//	ui.cRY0Edit->setText(QString::number(RY0,'f',1));
	//}
}

void TSPSDlg::on_OrginalByReceiver_clicked()//第一个检波点为原点
{
/*	GProject *pProject = GProject::getCurrentProject();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	//将模型原点坐标传入观测系统类中
		GRLineSet *pRLineSet = pProject->getRLineSet();
		if(pRLineSet == NULL) return ;
		int beginX=0;
		int beginY=1000000000;
		for(int i=0; i<pRLineSet->count(); i++)
		{
			GRLine *rLine = pRLineSet->get(i);
			{
				for(int j=0;j<rLine->count();j++)
				{
					GRPoint *rPoint=rLine->get(j);
					if(rPoint->getSPSY()<beginY) //找y坐标最小的点
					{
						beginX=rPoint->getSPSX();
						beginY=rPoint->getSPSY();
					}
				}
			}		
		}
		pSPSParam->mRX0=beginX;
		pSPSParam->mRY0=beginY;	
		double sx0,sy0;
		gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,sx0,sy0);
		sx0=sx0+pSPSParam->mMoveX0;//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
		sy0=sy0+pSPSParam->mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
		double SX0,SY0;
		gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,sx0,sy0,SX0,SY0);//计算检波点起始坐标
		pSPSParam->mSX0=SX0;
		pSPSParam->mSY0=SY0;
		*/
}

void TSPSDlg::on_OrginalByShot_clicked()//第一炮为原点
{
/*	GProject *pProject = GProject::getCurrentProject();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	//将模型原点坐标传入观测系统类中
		GShotSet *pShotSet = pProject->getShotSet();
		if(pShotSet == NULL) return ;
		int beginX=0;
		int beginY=1000000000;
		for(int i=0;i<pShotSet->count();i++)
		{
			GShot *pShot = pShotSet->get(i);//
			if(pShot->getSPoint()->getSPSY()<beginY)//找y坐标最小的点
			{
				beginX=pShot->getSPoint()->getSPSX();
				beginY=pShot->getSPoint()->getSPSY();
			}

		}
		pSPSParam->mSX0=beginX;
		pSPSParam->mSY0=beginY;	
		double rx0,ry0;
		gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
		rx0=rx0+pSPSParam->mMoveX0;//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
		ry0=ry0+pSPSParam->mMoveY0;//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）
		double RX0,RY0;
		gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
		pSPSParam->mRX0=RX0;
		pSPSParam->mRY0=RY0;
		*/
}

//void TSPSDlg::on_cLMaxOffEdit_textChanged(QString)
//{
//	GProject *pProject = GProject::getCurrentProject();
//	GSPSParam *pSPSParam = pProject->getSPSParam();
//	if(pSPSParam->mOrginalByReceiver==true)//以第一个检波点为原点
//	{
//		double beginX=pSPSParam->mRX0;
//		double beginY=pSPSParam->mRY0;	
//		double rx0,ry0;
//		gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
//		rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
//		ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
//		double SX0,SY0;
//		gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,SX0,SY0);//计算检波点起始坐标
//		ui.cSX0Edit->setText(QString::number(SX0,'f',1));
//		ui.cSY0Edit->setText(QString::number(SY0,'f',1));
//	}
//
//	if(pSPSParam->mOrginalByShot==true)//以第一个炮点为原点
//	{
//		double beginX=pSPSParam->mSX0;
//		double beginY=pSPSParam->mSY0;	
//		double rx0,ry0;
//		gRotate((double)-pSPSParam->mSinAf, (double)pSPSParam->mCosAf,beginX,beginY,rx0,ry0);
//		rx0=rx0+ui.cMoveX0Edit->text().toFloat();//add by zhaohu 检波点x方向平移量（旋转成水平坐标时的值）,往左是负值
//		ry0=ry0+ui.cMoveY0Edit->text().toFloat();//add by zhaohu 检波点Y方向平移量（旋转成水平坐标时的值）,往上是负值
//		double RX0,RY0;
//		gRotate((double)pSPSParam->mSinAf,(double) pSPSParam->mCosAf,rx0,ry0,RX0,RY0);//计算检波点起始坐标
//		ui.cRX0Edit->setText(QString::number(RX0,'f',1));
//		ui.cRY0Edit->setText(QString::number(RY0,'f',1));
//	}
//// 	pSPSParam->mRX0=RX0;
//// 	pSPSParam->mRY0=RY0;
//}