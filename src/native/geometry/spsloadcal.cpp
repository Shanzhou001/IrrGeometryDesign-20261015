/***************************************************
 * spsloadcal.cpp -- legacy SPS GUI coordinate grid // add by wushanzhou in 2026/10/24
 * Original author: Li Wenjun (lwj). // add by wushanzhou in 2026/10/24
 * Replaced unsafe temporary arrays with one checked 10 m coordinate mapping. // add by wushanzhou in 2026/10/24
 * add by wushanzhou in 2026/10/24
 ***************************************************/ // add by wushanzhou in 2026/10/24
#include "spsloadcal.h" // add by wushanzhou in 2026/10/24
#include <cmath> // add by wushanzhou in 2026/10/24
#include <cstdlib> // add by wushanzhou in 2026/10/24
#include <vector> // add by wushanzhou in 2026/10/24

static bool sps_coordinate_finite(float v){return v==v && fabs((double)v)<=1e30;} // add by wushanzhou in 2026/10/24

static bool sps_grid_indices(const QVector<QVector<float> >& values,const std::string& type, // add by wushanzhou in 2026/10/24
                            std::vector<std::vector<int> >& indices,int& columns) // add by wushanzhou in 2026/10/24
{
    columns=0;indices.clear(); // add by wushanzhou in 2026/10/24
    if(type!="R" && type!="S")return false; // add by wushanzhou in 2026/10/24
    double origin=0;bool found=false; // add by wushanzhou in 2026/10/24
    for(int i=0;i<values.size();i++)for(int j=0;j<values[i].size();j++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        const float v=values[i][j];if(!sps_coordinate_finite(v))return false; // add by wushanzhou in 2026/10/24
        if(!found || v<origin)origin=v;found=true; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    if(!found)return false; // add by wushanzhou in 2026/10/24
    indices.resize(values.size()); // add by wushanzhou in 2026/10/24
    for(int i=0;i<values.size();i++)for(int j=0;j<values[i].size();j++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        // Global origin retains line-to-line position; zero and descending coordinates are valid. // add by wushanzhou in 2026/10/24
        const double position=floor(((double)values[i][j]-origin)/10.0+.5); // add by wushanzhou in 2026/10/24
        if(position<0 || position>65535)return false; // add by wushanzhou in 2026/10/24
        const int index=(int)position;indices[i].push_back(index); // add by wushanzhou in 2026/10/24
        if(index+1>columns)columns=index+1; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    return (long long)values.size()*columns<=16777216; // add by wushanzhou in 2026/10/24
}

int ArrayLineNum(QVector<QVector<float> >& values,std::string type) // add by wushanzhou in 2026/10/24
{
    std::vector<std::vector<int> > indices;int columns; // add by wushanzhou in 2026/10/24
    return sps_grid_indices(values,type,indices,columns)?columns:0; // add by wushanzhou in 2026/10/24
}

int** SpsArrayTrans(QVector<QVector<float> >& values,std::string type) // add by wushanzhou in 2026/10/24
{
    std::vector<std::vector<int> > indices;int columns; // add by wushanzhou in 2026/10/24
    if(!sps_grid_indices(values,type,indices,columns))return NULL; // add by wushanzhou in 2026/10/24
    // Preserve the legacy malloc/free ownership contract used by the GUI. // add by wushanzhou in 2026/10/24
    int** result=(int**)calloc(values.size(),sizeof(int*));if(!result)return NULL; // add by wushanzhou in 2026/10/24
    for(int i=0;i<values.size();i++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        result[i]=(int*)calloc(columns,sizeof(int)); // add by wushanzhou in 2026/10/24
        if(!result[i]){for(int k=0;k<i;k++)free(result[k]);free(result);return NULL;} // add by wushanzhou in 2026/10/24
        for(size_t j=0;j<indices[i].size();j++)result[i][indices[i][j]]=1; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    return result; // add by wushanzhou in 2026/10/24
}

float SearchMin(QVector<QVector<float> >& values) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    float result=0;bool found=false; // add by wushanzhou in 2026/10/24
    for(int i=0;i<values.size();i++)for(int j=0;j<values[i].size();j++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        if(!sps_coordinate_finite(values[i][j]))return 0; // add by wushanzhou in 2026/10/24
        if(!found || values[i][j]<result)result=values[i][j];found=true; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    return result; // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24

float SearchMax(QVector<QVector<float> >& values) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    float result=0;bool found=false; // add by wushanzhou in 2026/10/24
    for(int i=0;i<values.size();i++)for(int j=0;j<values[i].size();j++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        if(!sps_coordinate_finite(values[i][j]))return 0; // add by wushanzhou in 2026/10/24
        if(!found || values[i][j]>result)result=values[i][j];found=true; // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    return result; // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/24
