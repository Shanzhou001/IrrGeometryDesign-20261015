#ifndef GSPS_SCANNER_H
#define GSPS_SCANNER_H

#include <qfile.h>
#include <qtextstream.h>
#include <qtextedit.h>
#include "gproject.h"
#include "gshotset.h"
#include "gslineset.h"

//------------------------------------------------------------------------------
class GXFileScanner
{
protected:
	bool scanLine(const QString &line, GShot *pShot);
public:
	GXFileScanner();

	bool scan(const QString &fileName, GShotSet *pShotSet, QTextEdit *spsMemo = NULL);
	void setProject(GProject *pProject){mpProject = pProject;}
private:
	GProject *mpProject;
};
//------------------------------------------------------------------------------
class GSFileScanner
{
public:
	GSFileScanner();

	bool scan(const QString &fileName, GSLineSet *pSLineSet,
			  GShotSet *pShotSet, QTextEdit *spsMemo = NULL);
};
//------------------------------------------------------------------------------
class GRFileScanner
{
public:
	GRFileScanner();

	bool scan(const QString &fileName, GRLineSet *pRLineSet, QTextEdit *spsMemo = NULL
		,QProgressBar *progressBar= NULL );
};

#endif
