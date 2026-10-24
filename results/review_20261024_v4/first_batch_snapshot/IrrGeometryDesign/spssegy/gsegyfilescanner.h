#ifndef GSEGYFILE_SCANNER_H
#define GSEGYFILE_SCANNER_H

#include <qprogressbar.h>
#include <qtextedit.h>
#include "qiodevice.h"
#include "qfile.h"
#include "qdatastream.h"
#include "gproject.h"

class GSEGYFileScanner
{
private:
	GProject *mpProject;
public:
	enum GErrorType {gErrorTrcCount, gNoSPSInfo, gNoXInfo};
protected:
	//bool scanLine(const QString &line, GShot *pShot);
public:
	GSEGYFileScanner();

	bool scan(const QString &fileName, QProgressBar *progressBar = NULL, 
			  QTextEdit *sgyMemo = NULL);

	QString getShotInfo(GShot *pShot);
	QString getErrorInfo(int recNum, GErrorType type);
	void setProject(GProject *pProject){mpProject = pProject;}
	bool scanForShotRecorderRange(const QString &fileName,
		                           QProgressBar *progressBar = NULL, 
			                   QTextEdit *sgyMemo = NULL);


	void ExportShotSgy(const QString &fileName);//zhaohu
};

#endif
