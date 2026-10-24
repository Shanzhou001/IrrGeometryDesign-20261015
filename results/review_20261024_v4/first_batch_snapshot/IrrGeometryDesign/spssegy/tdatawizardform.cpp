/****************************************************************************
** Form implementation generated from reading ui file 'datainput\tdatawizardform.ui'
**
** Created: 星期四 四月 14 22:45:43 2011
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "tdatawizardform.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "../datainput/tdatawizardform.ui.h"
/*
 *  Constructs a TDataWizardForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TDataWizardForm::TDataWizardForm( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "TDataWizardForm" );
    TDataWizardFormLayout = new QVBoxLayout( this, 11, 6, "TDataWizardFormLayout"); 

    mStepLabel = new QLabel( this, "mStepLabel" );
    mStepLabel->setMinimumSize( QSize( 0, 20 ) );
    mStepLabel->setMaximumSize( QSize( 32767, 20 ) );
    TDataWizardFormLayout->addWidget( mStepLabel );

    mWidgetStack = new QWidgetStack( this, "mWidgetStack" );
    mWidgetStack->setFrameShape( QWidgetStack::Box );
    mWidgetStack->setFrameShadow( QWidgetStack::Sunken );

    spsPage = new QWidget( mWidgetStack, "spsPage" );
    spsPageLayout = new QVBoxLayout( spsPage, 11, 6, "spsPageLayout"); 

    groupBox1 = new QGroupBox( spsPage, "groupBox1" );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    groupBox1Layout = new QGridLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    layout17 = new QHBoxLayout( 0, 0, 6, "layout17"); 

    layout12 = new QVBoxLayout( 0, 0, 6, "layout12"); 

    textLabel1 = new QLabel( groupBox1, "textLabel1" );
    layout12->addWidget( textLabel1 );

    textLabel2 = new QLabel( groupBox1, "textLabel2" );
    layout12->addWidget( textLabel2 );

    textLabel3 = new QLabel( groupBox1, "textLabel3" );
    layout12->addWidget( textLabel3 );

    textLabel4_2 = new QLabel( groupBox1, "textLabel4_2" );
    layout12->addWidget( textLabel4_2 );
    layout17->addLayout( layout12 );

    layout14 = new QVBoxLayout( 0, 0, 6, "layout14"); 

    mSFileNameEdit = new QLineEdit( groupBox1, "mSFileNameEdit" );
    mSFileNameEdit->setFrameShape( QLineEdit::LineEditPanel );
    mSFileNameEdit->setFrameShadow( QLineEdit::Sunken );
    layout14->addWidget( mSFileNameEdit );

    mRFileNameEdit = new QLineEdit( groupBox1, "mRFileNameEdit" );
    layout14->addWidget( mRFileNameEdit );

    mXFileNameEdit = new QLineEdit( groupBox1, "mXFileNameEdit" );
    mXFileNameEdit->setFrameShape( QLineEdit::LineEditPanel );
    mXFileNameEdit->setFrameShadow( QLineEdit::Sunken );
    layout14->addWidget( mXFileNameEdit );

    mSEGYFileNameEdit_2 = new QLineEdit( groupBox1, "mSEGYFileNameEdit_2" );
    layout14->addWidget( mSEGYFileNameEdit_2 );
    layout17->addLayout( layout14 );

    layout15 = new QVBoxLayout( 0, 0, 6, "layout15"); 

    mSFileBtn = new QToolButton( groupBox1, "mSFileBtn" );
    layout15->addWidget( mSFileBtn );

    mRFileBtn = new QToolButton( groupBox1, "mRFileBtn" );
    layout15->addWidget( mRFileBtn );

    mXFileBtn = new QToolButton( groupBox1, "mXFileBtn" );
    layout15->addWidget( mXFileBtn );

    mSEGYFileBtn_2 = new QToolButton( groupBox1, "mSEGYFileBtn_2" );
    layout15->addWidget( mSEGYFileBtn_2 );
    layout17->addLayout( layout15 );

    groupBox1Layout->addLayout( layout17, 0, 0 );
    spsPageLayout->addWidget( groupBox1 );

    mSPSMemo = new QTextEdit( spsPage, "mSPSMemo" );
    QFont mSPSMemo_font(  mSPSMemo->font() );
    mSPSMemo_font.setFamily( "Courier New" );
    mSPSMemo->setFont( mSPSMemo_font ); 
    mSPSMemo->setWordWrap( QTextEdit::NoWrap );
    mSPSMemo->setReadOnly( TRUE );
    spsPageLayout->addWidget( mSPSMemo );

    mSEGYProgressBar_2 = new QProgressBar( spsPage, "mSEGYProgressBar_2" );
    spsPageLayout->addWidget( mSEGYProgressBar_2 );
    mWidgetStack->addWidget( spsPage, 0 );

    page = new QWidget( mWidgetStack, "page" );
    pageLayout = new QGridLayout( page, 1, 1, 11, 6, "pageLayout"); 

    mSEGYMemo = new QTextEdit( page, "mSEGYMemo" );
    mSEGYMemo->setWordWrap( QTextEdit::NoWrap );
    mSEGYMemo->setReadOnly( TRUE );

    pageLayout->addMultiCellWidget( mSEGYMemo, 1, 1, 0, 1 );

    mSEGYProgressBar = new QProgressBar( page, "mSEGYProgressBar" );

    pageLayout->addMultiCellWidget( mSEGYProgressBar, 2, 2, 0, 1 );

    layout23 = new QHBoxLayout( 0, 0, 6, "layout23"); 

    textLabel4_3 = new QLabel( page, "textLabel4_3" );
    layout23->addWidget( textLabel4_3 );

    mcopySEGYFileNameEdit = new QLineEdit( page, "mcopySEGYFileNameEdit" );
    mcopySEGYFileNameEdit->setReadOnly( TRUE );
    layout23->addWidget( mcopySEGYFileNameEdit );

    pageLayout->addLayout( layout23, 0, 0 );
    mWidgetStack->addWidget( page, 1 );
    TDataWizardFormLayout->addWidget( mWidgetStack );

    layout3 = new QHBoxLayout( 0, 0, 6, "layout3"); 

    mLoadBtn = new QPushButton( this, "mLoadBtn" );
    mLoadBtn->setDefault( TRUE );
    layout3->addWidget( mLoadBtn );
    spacer1 = new QSpacerItem( 170, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer1 );

    mNextBtn = new QPushButton( this, "mNextBtn" );
    mNextBtn->setEnabled( FALSE );
    layout3->addWidget( mNextBtn );

    cancelBtn = new QPushButton( this, "cancelBtn" );
    layout3->addWidget( cancelBtn );
    TDataWizardFormLayout->addLayout( layout3 );
    languageChange();
    resize( QSize(490, 424).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( mLoadBtn, SIGNAL( clicked() ), this, SLOT( loadBtnClicked() ) );
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( cancelBtnClicked() ) );
    connect( mNextBtn, SIGNAL( clicked() ), this, SLOT( nextBtnClicked() ) );
    connect( mSEGYFileBtn_2, SIGNAL( clicked() ), this, SLOT( selSEGYFileBtnClicked() ) );
    connect( mXFileBtn, SIGNAL( clicked() ), this, SLOT( selXFileBtnClicked() ) );
    connect( mRFileBtn, SIGNAL( clicked() ), this, SLOT( selectRFile() ) );
    connect( mSFileBtn, SIGNAL( clicked() ), this, SLOT( selSFileBtnClicked() ) );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
TDataWizardForm::~TDataWizardForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void TDataWizardForm::languageChange()
{
    setCaption( tr( "Data Input Wizard" ) );
    mStepLabel->setText( tr( "Step %1" ) );
    groupBox1->setTitle( tr( "SPS File Group" ) );
    textLabel1->setText( tr( "Shot FileName(*.s)" ) );
    textLabel2->setText( tr( "Receive FileName(*.r)" ) );
    textLabel3->setText( tr( "Relationship FileName(*.x)" ) );
    textLabel4_2->setText( tr( "SEG-Y FileName(*.sgy)" ) );
    mSFileBtn->setText( tr( "..." ) );
    mRFileBtn->setText( tr( "..." ) );
    mXFileBtn->setText( tr( "..." ) );
    mSEGYFileBtn_2->setText( tr( "..." ) );
    textLabel4_3->setText( tr( "SEG-Y FileName(*.sgy)" ) );
    mLoadBtn->setText( tr( "&Load" ) );
    mLoadBtn->setAccel( QKeySequence( tr( "Alt+L" ) ) );
    mNextBtn->setText( tr( "&Next" ) );
    cancelBtn->setText( tr( "&Cancel" ) );
}

