/*****************************************************
 * spsloadcal.H
 * Function: 根据导入的sps文件转换成0-1矩阵并计算μ值
 * Created on Aug 14th, 2023
 * Author:lwj
 *****************************************************/
#ifndef _SPSLOADCAL_H_
#define _SPSLOADCAL_H_
#include "../sampling/sample.h"
#include "../sampling/CalculateMiu.h"
#include <QVector>

int ArrayLineNum(QVector<QVector<float> > &Phi,string calctype);//寻找累加的最大间隔数值

int **SpsArrayTrans(QVector<QVector<float> > &Phi,string calctype);//寻找实际坐标之间的间隔，并转化为0-1矩阵

float SearchMin(QVector<QVector<float> > &Phi);//寻找数组中最小值

float SearchMax(QVector<QVector<float> > &Phi);//寻找数组中最大值


#endif