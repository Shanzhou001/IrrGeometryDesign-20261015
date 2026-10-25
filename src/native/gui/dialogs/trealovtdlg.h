#ifndef TREALOVTDLG_H
#define TREALOVTDLG_H

#include <QDialog>
#include <QElapsedTimer> // 新增用于计时
#include <QMessageBox>
#include "ui_trealovtdlg.h"
#include "../../io/sps/gproject.h"
#include "draw2dfrm.h"
#include "analyzebinform.h"
#include "../../geometry/gAttrAnalyzeParam.h"
#include "../../core/gvaluevector.h"
#include "../survey/gcellgrid.h"
#include "../../optimization/OptimizeMiu.h" // 引入优化算法头文件

class TRealOVTdlg : public QDialog
{
	Q_OBJECT

public:
	TRealOVTdlg(QWidget *parent = 0);
	~TRealOVTdlg();

	void init(GAttrAnalyzeParam *param);

private:
	Ui::TRealOVTdlgClass ui;
	GAttrAnalyzeParam *pAttrAnalyzeParam;

private slots:
	void on_OVT_PatchAttri_clicked();
	void on_Cancel_clicked();
};

#endif // TREALOVTDLG_H

/*#ifndef TREALOVTDLG_H
#define TREALOVTDLG_H

#include <QDialog>
#include "ui_trealovtdlg.h"
#include "../../io/sps/gproject.h"
#include "draw2dfrm.h"
#include "analyzebinform.h"
#include "../../geometry/gAttrAnalyzeParam.h"
#include "../../core/gvaluevector.h"
#include "../survey/gcellgrid.h"

class TRealOVTdlg : public QDialog
{
	Q_OBJECT

public:
	TRealOVTdlg(QWidget *parent = 0);
	~TRealOVTdlg();

	void init(GAttrAnalyzeParam *param);

private:
	Ui::TRealOVTdlgClass ui;
	GAttrAnalyzeParam *pAttrAnalyzeParam;

private slots:
	void on_OVT_PatchAttri_clicked();
//	void on_OVT_ArbitraryPatchAttri_clicked();
	void on_Cancel_clicked();
	
};

#endif // TREALOVTDLG_H
*/