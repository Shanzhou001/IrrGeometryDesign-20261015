#ifndef GEOMETRYTEMPLATEPARAMETER_H
#define GEOMETRYTEMPLATEPARAMETER_H

#include <QDialog>
#include "ui_geometrytemplateparameter.h"

#include <QMessageBox>

class GeometryTemplateParameter : public QDialog
{
	Q_OBJECT

public:
	GeometryTemplateParameter(QWidget *parent = 0);
	~GeometryTemplateParameter();

	int getChannelNumPerRline()		{return ChannelNumPerRline;}
	int getRlineNumPerTemplate()	{return RlineNumPerTemplate;}
	int getSpointNumPerSline()		{return ShotpointNumPerSline;}	
	int getSlineNumPerTemplate()	{return SlineNumPerTemplate;}
	float getRlineInterval()		{return RlineInterval;}
	float getRpointInterval()		{return RpointInterval;}
	float getSlineInterval()		{return SlineInterval;}
	float getSpointInterval()		{return SpointInterval;}
	float getFirstRpointX0()		{return FirstRpointX0;}
	float getFirstRpointY0()		{return FirstRpointY0;}
	float getFirstSpointX0()		{return FirstSpointX0;}
	float getFirstSpointY0()		{return FirstSpointY0;}

private:
	Ui::GeometryTemplateParameterClass ui;
	
	int ChannelNumPerRline;
	int RlineNumPerTemplate;
	int SlineNumPerTemplate;
	int ShotpointNumPerSline;

	float RlineInterval;
	float RpointInterval;
	float SlineInterval;//等效于纵向滚动距离
	float SpointInterval;
	float FirstRpointX0;
	float FirstRpointY0;
	float FirstSpointX0;
	float FirstSpointY0;

	private slots:
		void on_Ok_Btn_clicked();
		void on_Cancel_Btn_clicked();
		void setFirstshotX();
		void setFirstshotY();

};

#endif // GEOMETRYTEMPLATEPARAMETER_H
