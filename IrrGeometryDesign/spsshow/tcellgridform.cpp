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
#include "gproject.h"
#include "gslineset.h"
#include "grlineset.h"
#include "gcellgrid.h"
#include "gmath.h"
#include <math.h>
#include "tcellgridform.h"

TCellGridForm::TCellGridForm( QWidget* parent ):QDialog(parent)
{
	ui.setupUi(this);
	init();

}
TCellGridForm::~TCellGridForm()
{
	destroy();
}
void TCellGridForm::init()
{
	GProject *pProject = GProject::getCurrentProject();
	GSLineSet *pSLineSet = pProject->getSLineSet();
	GRLineSet *pRLineSet = pProject->getRLineSet();
	GSPSParam *pSPSParam = pProject->getSPSParam();
	GCellGrid *pCellGrid = pProject->getCellGrid();

	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;

	minSX = minSY = HUGE_VAL;
	maxSX = maxSY = -HUGE_VAL;
	minRX = minRY = HUGE_VAL;
	maxRX = maxRY = -HUGE_VAL;

	pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
	pRLineSet->getRange(minRX, minRY, maxRX, maxRY);

	float spsX0, spsY0, x0, y0, xn, yn, gx, gy;
	mX0 = x0 = (minRX + minSX)/2;
	mY0 = y0 = (minRY + minSY)/2;
	pSPSParam->transTosps(x0, y0, spsX0, spsY0);
	//  [Liu Wei 3/29/2023]
	mXn = xn = (maxRX + maxSX)/2;// yn = (maxRY + maxRY)/2;//原始代码（错误），赵虎	
	mYn = yn = (maxRY + maxSY)/2;// 修改 [3/18/2023 Liu Wei]
	//  [Liu Wei 3/29/2023]
	QString str;
	str.sprintf("(%.2f, %.2f)", spsX0, spsY0);//格式化字符串：最小的共中点坐标
	ui.cFirstCMPLabel->setText(tr("First CMP Coordinate:")+str);// 界面中的字符串被改写，需要做动态翻译时需要小心处理（改写的情况建议直接采用目标语言字符混合英文字符。） [Liu Wei 4/14/2023]

	gx = pSPSParam->mRPtInv/2;
	gy = pSPSParam->mSPtInv/2;
	
	// 界面参数计算和显示 [Liu Wei 6/21/2023]
	ui.cXGridEdit->setText(QString::number(gx));
	ui.cYGridEdit->setText(QString::number(gy));

	int numX, numY;
	numX = gRound((xn-x0) / gx);// //numX = gRound((xn-x0) / gx) + 1;// 原始代码 为什么+1？ [3/18/2023 Liu Wei]
	numY = gRound((yn-y0) / gy);

	ui.cXGridNumBox->setValue(numX);
	ui.cYGridNumBox->setValue(numY);

	ui.cXDiffEdit->setText(QString::number(-gx/2));
	ui.cYDiffEdit->setText(QString::number(-gy/2));

	// 赵老师的原始代码 [Liu Wei 6/21/2023]
	//if(pCellGrid->mGrid == NULL)
	//{
	//	ui.cXGridEdit->setText(QString::number(gx));
	//	ui.cYGridEdit->setText(QString::number(gy));

	//	int numX, numY;
	//	//numX = gRound((xn-x0) / gx) + 1;// 原始代码 为什么+1？ [3/18/2023 Liu Wei]
	//	numX = gRound((xn-x0) / gx);// [3/18/2023 Liu Wei]
	//	numY = gRound((yn-y0) / gy);
	//	ui.cXGridNumBox->setValue(numX);
	//	ui.cYGridNumBox->setValue(numY);

	//	ui.cXDiffEdit->setText(QString::number(-gx/2));
	//	ui.cYDiffEdit->setText(QString::number(-gy/2));
	//}
	//else
	//{
	//	ui.cXGridEdit->setText(QString::number(pCellGrid->mGridX));
	//	ui.cYGridEdit->setText(QString::number(pCellGrid->mGridY));
	//	
	//	ui.cXGridNumBox->setValue(pCellGrid->mNumX);
	//	ui.cYGridNumBox->setValue(pCellGrid->mNumY);
	//	
	//	ui.cXDiffEdit->setText(QString::number(pCellGrid->mDiffX));
	//	ui.cYDiffEdit->setText(QString::number(pCellGrid->mDiffY));
	//}
	// 赵老师的原始代码 [Liu Wei 6/21/2023]
}

void TCellGridForm::destroy()
{
}

//划分面元
void TCellGridForm::accept()
{
	GProject *pProject = GProject::getCurrentProject();
	GCellGrid *pCellGrid = pProject->getCellGrid();
	GSPSParam *pSPSParam = pProject->getSPSParam();

	float x0, y0, gx, gy;
	int numX, numY;
	gx = ui.cXGridEdit->text().toFloat();
	gy = ui.cYGridEdit->text().toFloat();
	x0 = mX0 + ui.cXDiffEdit->text().toFloat();
	y0 = mY0 + ui.cYDiffEdit->text().toFloat();
	numX = ui.cXGridNumBox->value();
	numY = ui.cYGridNumBox->value();
	pCellGrid->mDiffX = ui.cXDiffEdit->text().toFloat();
	pCellGrid->mDiffY = ui.cYDiffEdit->text().toFloat();
	pCellGrid->create(pSPSParam, x0, y0, numX, numY, gx, gy);
	pCellGrid->calculateCoverCount();
	QDialog::accept();
}


void TCellGridForm::on_cXGridEdit_textChanged(const QString &txt)
{
	float gx=txt.toFloat();
	float numX = gRound((mXn-mX0) / gx);
	ui.cXGridNumBox->setValue(numX);
	ui.cXDiffEdit->setText(QString::number(-gx/2));
}

void TCellGridForm::on_cYGridEdit_textChanged(const QString &txt)
{
	float gy=txt.toFloat();
	float numY = gRound((mYn-mY0) / gy);
	ui.cYGridNumBox->setValue(numY);
	ui.cYDiffEdit->setText(QString::number(-gy/2));
}