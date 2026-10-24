/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void TDataWizardForm::init()
{
	mCurrentStep = 0;
	mpProject = NULL;
	mStepLabel->setText( tr("Step %1").arg(mCurrentStep+1) );	
}

void TDataWizardForm::destroy()
{
	
}

void TDataWizardForm::closeEvent ( QCloseEvent * e )
{
	if( result() !=	QDialog::Accepted )
	{
		if(mpProject != NULL) mpProject->reLoad();
	}
	e->accept();
}

void TDataWizardForm::loadBtnClicked()
{
	if(mpProject == NULL) return;
	if(mCurrentStep == 0)
	{
		mpProject->scanForShotRecorderRange(mSEGYFileNameEdit_2->text(),
			mSEGYProgressBar_2,mSPSMemo);
		if(mpProject->appendSPSInfo(mSFileNameEdit->text(), mRFileNameEdit->text(),
									mXFileNameEdit->text(), mSPSMemo,mSEGYProgressBar_2 ))
		{
			mLoadBtn->setEnabled( false );
			mNextBtn->setEnabled( true );
		}

	}
	else
	{
		if(mpProject->appendSEGYInfo(mSEGYFileNameEdit_2->text(), mSEGYProgressBar, mSEGYMemo))
		{
			mLoadBtn->setEnabled( false );
			mNextBtn->setEnabled( true );
		}
	}
}


void TDataWizardForm::cancelBtnClicked()
{
	reject();
}

void TDataWizardForm::nextBtnClicked()
{
	if( mCurrentStep == 0 )
	{
		mLoadBtn->setEnabled( true );
		mNextBtn->setEnabled( false );
		mNextBtn->setText( tr("&Finish") );
		mCurrentStep++;
		mWidgetStack->raiseWidget( mCurrentStep );
		mStepLabel->setText( tr("Step %1").arg(mCurrentStep+1) );

	}
	else
	{
		//<1>重整数据结构
		mpProject->refresh();	
		
		//<2>写入工程文件
		mpProject->createFolders();  //for debug
		if(!mpProject->save())
		{
			QMessageBox::information( 0, QString::fromLocal8Bit("错误"),
				QString::fromLocal8Bit("无法保存数!") );
			return;
		}
        appendLogFile();
		accept();
	}
}

void TDataWizardForm::selSFileBtnClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		QString::null, tr("Shot files (*.s);;All files (*.*)"),
		this, "open shot file" );
	if( fileName != QString::null )
	{
		mSFileNameEdit->setText( fileName );
		QFileInfo fi( fileName );
		QFile file;
		QString name = fi.dirPath()+"/"+fi.baseName(true)+".r";
		file.setName( name );
		if( file.exists() ) mRFileNameEdit->setText( name );
		name = fi.dirPath()+"/"+fi.baseName(true)+".x";
		file.setName( name );
		if( file.exists() ) mXFileNameEdit->setText( name );
	}
			
}


void TDataWizardForm::selXFileBtnClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		QString::null, tr("Relationship files (*.x);;All files (*.*)"),
		this, "Open Relationship File" );
	if( fileName != QString::null )
	{
		mXFileNameEdit->setText( fileName );
		
	}
}


void TDataWizardForm::selSEGYFileBtnClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		QString::null, tr("SEG-Y Files (*.sgy);;All files (*.*)"),
		this, "Open SEG-Y File" );
	if( fileName != QString::null )
	{
		mSEGYFileNameEdit_2->setText( fileName );
		mcopySEGYFileNameEdit->setText( fileName );
	}
}


void TDataWizardForm::setProject(GProject *pProject)
{
	mpProject = pProject;
}

void TDataWizardForm::appendLogFile()
{
	QFile spsFile( mpProject->getSPSLogFileName() );
	if( !spsFile.open(IO_WriteOnly | IO_Append) ) return ;

	QTextStream spsStream( &spsFile );
	spsStream << QString::fromLocal8Bit("\n\n\n//----载入时间 ");
	spsStream << QDateTime::currentDateTime().toString();
	spsStream << "----//\r\n";
	spsStream << mSPSMemo->text();
	spsFile.close();

	QFile sgyFile( mpProject->getSGYLogFileName() );
	if( !sgyFile.open(IO_WriteOnly | IO_Append) ) return ;

	QTextStream sgyStream( &sgyFile );
	sgyStream << QString::fromLocal8Bit("\n\n\n//----载入时间 ");
	sgyStream << QDateTime::currentDateTime().toString();
	sgyStream << "----//\r\n";
	sgyStream << mSEGYMemo->text();
	sgyFile.close();
}


void TDataWizardForm::selectRFile()
{
   QString fileName = QFileDialog::getOpenFileName(
		QString::null, tr("Shot files (*.r);;All files (*.*)"),
		this, "open r file" );
	if( fileName != QString::null )
	{
		mRFileNameEdit->setText( fileName );
		
	}
			
}
