#include "geometrytemplateparameter.h"

GeometryTemplateParameter::GeometryTemplateParameter(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);	
	connect(ui.mSline_num_PerTemplate,SIGNAL(textChanged(const QString &)), this, SLOT(setFirstshotX()));
	connect(ui.mSline_interval,SIGNAL(textChanged(const QString &)), this, SLOT(setFirstshotX()));
	connect(ui.mShot_num_PerSline,SIGNAL(textChanged(const QString &)), this, SLOT(setFirstshotY()));
	connect(ui.mShotpoint_interval,SIGNAL(textChanged(const QString &)), this, SLOT(setFirstshotY()));
}

GeometryTemplateParameter::~GeometryTemplateParameter()
{

}

void GeometryTemplateParameter::on_Ok_Btn_clicked()
{
	if (ui.mChannel_num_PerRline->text().isEmpty() || ui.mRline_num_PerTemplate->text().isEmpty() || 
		ui.mSline_num_PerTemplate->text().isEmpty() || ui.mShot_num_PerSline->text().isEmpty() ||
		ui.mRline_interval->text().isEmpty() || ui.mReceivingpoint_interval->text().isEmpty() ||
		ui.mSline_interval->text().isEmpty() || ui.mShotpoint_interval->text().isEmpty() ||
		ui.mFirstRP_X0->text().isEmpty() || ui.mFirstRP_Y0->text().isEmpty() ||
		ui.mFirstSP_X0->text().isEmpty() || ui.mFirstSP_Y0->text().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Warning"), QObject::tr("Please make sure your template parameters are available!"));
		return;
	} // 用户必须给定参数才能进行下一步操作 [Liu Wei 4/5/2023]
	else if (ui.mChannel_num_PerRline->text().toInt()<=0 || ui.mRline_num_PerTemplate->text().toInt()<=0 || 
		ui.mSline_num_PerTemplate->text().toInt()<=0 || ui.mShot_num_PerSline->text().toInt()<=0 ||
		ui.mRline_interval->text().toFloat()<=0 || ui.mReceivingpoint_interval->text().toFloat()<=0 ||
		ui.mSline_interval->text().toFloat()<=0 || ui.mShotpoint_interval->text().toFloat()<=0 ||
		ui.mFirstRP_X0->text().toFloat()<0 || ui.mFirstRP_Y0->text().toFloat()<0 ||
		ui.mFirstSP_X0->text().toFloat()<0 || ui.mFirstSP_Y0->text().toFloat()<0)
	{
		QMessageBox::warning(this, QObject::tr("Warning"), QObject::tr("Please make sure your template parameters are correct!"));
		return;
	} // 用户必须给定参数才能进行下一步操作 [Liu Wei 4/5/2023]
	else
	{
		ChannelNumPerRline=ui.mChannel_num_PerRline->text().toInt();
		RlineNumPerTemplate=ui.mRline_num_PerTemplate->text().toInt();
		ShotpointNumPerSline=ui.mShot_num_PerSline->text().toInt();
		SlineNumPerTemplate=ui.mSline_num_PerTemplate->text().toInt();	
		RlineInterval=ui.mRline_interval->text().toFloat();
		RpointInterval=ui.mReceivingpoint_interval->text().toFloat();
		SlineInterval=ui.mSline_interval->text().toFloat();
		SpointInterval=ui.mShotpoint_interval->text().toFloat();
		FirstRpointX0=ui.mFirstRP_X0->text().toFloat();
		FirstRpointY0=ui.mFirstRP_Y0->text().toFloat();
		FirstSpointX0=ui.mFirstSP_X0->text().toFloat();
		FirstSpointY0=ui.mFirstSP_Y0->text().toFloat();
		accept();
	}
}

void GeometryTemplateParameter::on_Cancel_Btn_clicked()
{
	reject();
}

void GeometryTemplateParameter::setFirstshotX()
{
	float SXmin=((ui.mChannel_num_PerRline->text().toInt()-1)*ui.mReceivingpoint_interval->text().toFloat()-
		(ui.mSline_num_PerTemplate->text().toInt()-1)*ui.mSline_interval->text().toFloat())/2;
	ui.mFirstSP_X0->setText(QString::number(SXmin));
}

void GeometryTemplateParameter::setFirstshotY()
{
	float SYmin=((ui.mRline_num_PerTemplate->text().toInt()-1)*ui.mRline_interval->text().toFloat()-
		(ui.mShot_num_PerSline->text().toInt()-1)*ui.mShotpoint_interval->text().toFloat())/2;
	ui.mFirstSP_Y0->setText(QString::number(SYmin));
}