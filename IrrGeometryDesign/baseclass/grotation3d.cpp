#include "gvector3d.h"
#include "gmatrix3d.h"
#include "gquaternion3d.h"
#include "grotation3d.h"

#include <stdio.h>


GRotation3d::GRotation3d()
{   
     mAxis.set(0,0,1);
     mAngle = 0;
}

GRotation3d::GRotation3d(double axisX, double axisY, double axisZ, double angleW)
{
     mAxis.set(axisX, axisY, axisZ);
     mAngle = angleW;
}

GRotation3d::GRotation3d(const GVector3d &axisV, double angleD)
{
     mAxis = axisV;
     mAngle = angleD;                                 
}
GRotation3d::GRotation3d(const GRotation3d &v)
{
     mAxis = v.mAxis;
     mAngle = v.mAngle;
}

void GRotation3d::setAngle(double x) 
{ 
     mAngle = x; 
}

void GRotation3d::setAxis(const GVector3d &x) 
{ 
     mAxis = x; 
}

void GRotation3d::set(const GVector3d &x, double y)
{ 
     mAxis = x;
     mAngle = y;
}

void GRotation3d::set(double axisX, double axisY, double axisZ, double angleW)
{
     mAxis.set(axisX, axisY, axisZ);
     mAngle = angleW;
}

void GRotation3d::operator = (const GRotation3d &v)
{
     mAxis = v.mAxis;
     mAngle = v.mAngle;                                 
}

GRotation3d GRotation3d::operator * (const GRotation3d &v) const
{
     GRotation3d m; 
	 GQuaternion3d q1 = toQuaternion();
	 GQuaternion3d q2 = v.toQuaternion();
	 GQuaternion3d q  = q2 * q1;                     //зЂвт 
	 m = m.fromQuaternion(q);
     return(m);                            
}

void GRotation3d::operator *= (const GRotation3d &v)
{
     (*this) = (*this) * v;
}

GQuaternion3d GRotation3d::toQuaternion() const
{
    GQuaternion3d m; 
    double theta		= mAngle * ANGLE_TO_RADIAN;
	double cosValue	= cos(theta/2);  
	double sinValue	= sin(theta/2);  
	m.mAxis[0]	= sinValue * mAxis.mVector[0];
	m.mAxis[1]	= sinValue * mAxis.mVector[1];
	m.mAxis[2]	= sinValue * mAxis.mVector[2];
	m.mAxis[3]	= cosValue;

    return m;  
}

GMatrix3d GRotation3d::toMatrix(const GRotation3d &v) const
{
    GQuaternion3d m;
    GMatrix3d a;
    m = v.toQuaternion();
    return m.toMatrix();
}

GRotation3d GRotation3d::fromQuaternion(const GQuaternion3d &q) const
{
    GRotation3d m;
    double halfTheta= acos(q.mAxis[3]);
	double sinValue = sin(halfTheta);  
    m.mAngle = halfTheta * RADIAN_TO_ANGLE * 2;
	m.mAxis.mVector[0] = q.mAxis[0]/sinValue;
	m.mAxis.mVector[1] = q.mAxis[1]/sinValue;
	m.mAxis.mVector[2] = q.mAxis[2]/sinValue;
	return(m);
}
