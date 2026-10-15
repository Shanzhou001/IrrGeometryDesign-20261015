#ifndef DATARECONSTRUCTION_H
#define DATARECONSTRUCTION_H

#include <QtGui/QDialog>
#include "ui_datareconstruction.h"
#include "gproject.h"
#include <qdatetime.h>
#include "segyio.h"
#include <QProgressBar>
#include <QVector>
#include <QtAlgorithms>

#include <QMessageBox>
#include "geometrytemplateparameter.h"

class DataReconstruction : public QDialog
{
	Q_OBJECT

public:
	DataReconstruction(QWidget *parent = 0, Qt::WFlags flags = 0);
	~DataReconstruction();
	
	void setProject(GProject *pProject);

private:
	Ui::DataReconstructionClass ui;
	
	GProject *mpProject;

private slots:
	void on_mLoadSPSFilesBtn_clicked();
	void on_mDisplayandAnalyseBtn_clicked();
	void on_mRegularOVTVolumeBtn_clicked();
	void on_cancelBtn_clicked();
};

#endif // DATARECONSTRUCTION_H
