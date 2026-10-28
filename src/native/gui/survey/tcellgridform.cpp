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
#include "../../io/sps/gproject.h"
#include "../../io/sps/gslineset.h"
#include "../../io/sps/grlineset.h"
#include "gcellgrid.h"
#include "../../core/gmath.h"
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
	GSLineSet *pSLineSet = (pProject->mIsNewImfo && pProject->getSLineSetNew() && pProject->getSLineSetNew()->count() > 0)
	                       ? pProject->getSLineSetNew()
	                       : ((pProject->getSLineSet() && pProject->getSLineSet()->count() > 0) ? pProject->getSLineSet() : pProject->getSLineSetNew());
	GRLineSet *pRLineSet = (pProject->mIsNewImfo && pProject->getRLineSetNew() && pProject->getRLineSetNew()->count() > 0)
	                       ? pProject->getRLineSetNew()
	                       : ((pProject->getRLineSet() && pProject->getRLineSet()->count() > 0) ? pProject->getRLineSet() : pProject->getRLineSetNew());
	GSPSParam *pSPSParam = pProject->getSPSParam();
	GCellGrid *pCellGrid = pProject->getCellGrid();

	float minSX, minSY, maxSX, maxSY;
	float minRX, minRY, maxRX, maxRY;

	minSX = minSY = HUGE_VAL;
	maxSX = maxSY = -HUGE_VAL;
	minRX = minRY = HUGE_VAL;
	maxRX = maxRY = -HUGE_VAL;

	if (pSLineSet && pSLineSet->count() > 0)
		pSLineSet->getRange(minSX, minSY, maxSX, maxSY);
	if (pRLineSet && pRLineSet->count() > 0)
		pRLineSet->getRange(minRX, minRY, maxRX, maxRY);

	if (minSX == HUGE_VAL || maxSX == -HUGE_VAL) { minSX = 0.0f; maxSX = 8000.0f; minSY = 0.0f; maxSY = 6000.0f; }
	if (minRX == HUGE_VAL || maxRX == -HUGE_VAL) { minRX = 0.0f; maxRX = 8000.0f; minRY = 0.0f; maxRY = 6000.0f; }

	float spsX0, spsY0, x0, y0, xn, yn, gx, gy;
	mX0 = x0 = (minRX + minSX)/2;
	mY0 = y0 = (minRY + minSY)/2;
	pSPSParam->transTosps(x0, y0, spsX0, spsY0);
	mXn = xn = (maxRX + maxSX)/2;
	mYn = yn = (maxRY + maxSY)/2;

	QString str;
	str.sprintf("(%.2f, %.2f)", spsX0, spsY0);
	ui.cFirstCMPLabel->setText(tr("First CMP Coordinate:")+str);

	gx = pSPSParam->mRPtInv / 2.0f;
	gy = pSPSParam->mSPtInv / 2.0f;
	if (gx <= 0.1f) gx = 25.0f;
	if (gy <= 0.1f) gy = 25.0f;
	
	ui.cXGridEdit->setText(QString::number(gx));
	ui.cYGridEdit->setText(QString::number(gy));

	int numX, numY;
	numX = gRound((xn-x0) / gx);
	numY = gRound((yn-y0) / gy);
	if (numX < 1) numX = 80;
	if (numY < 1) numY = 60;

	ui.cXGridNumBox->setValue(numX);
	ui.cYGridNumBox->setValue(numY);

	ui.cXDiffEdit->setText(QString::number(-gx/2));
	ui.cYDiffEdit->setText(QString::number(-gy/2));
	if (pCellGrid && pCellGrid->mGrid != NULL && pCellGrid->mNumX > 0 && pCellGrid->mNumY > 0 && pCellGrid->mGridX > 0 && pCellGrid->mGridY > 0)
	{
		ui.cXGridEdit->setText(QString::number(pCellGrid->mGridX));
		ui.cYGridEdit->setText(QString::number(pCellGrid->mGridY));
		ui.cXGridNumBox->setValue(pCellGrid->mNumX);
		ui.cYGridNumBox->setValue(pCellGrid->mNumY);
		ui.cXDiffEdit->setText(QString::number(pCellGrid->mDiffX));
		ui.cYDiffEdit->setText(QString::number(pCellGrid->mDiffY));
	}

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
	if (gx <= 0.01f) gx = 25.0f;
	if (gy <= 0.01f) gy = 25.0f;
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
	if (gx > 0.01f)
	{
		float numX = gRound((mXn-mX0) / gx);
		if (numX < 1) numX = 1;
		ui.cXGridNumBox->setValue(numX);
		ui.cXDiffEdit->setText(QString::number(-gx/2));
	}
}

void TCellGridForm::on_cYGridEdit_textChanged(const QString &txt)
{
	float gy=txt.toFloat();
	if (gy > 0.01f)
	{
		float numY = gRound((mYn-mY0) / gy);
		if (numY < 1) numY = 1;
		ui.cYGridNumBox->setValue(numY);
		ui.cYDiffEdit->setText(QString::number(-gy/2));

	}
}