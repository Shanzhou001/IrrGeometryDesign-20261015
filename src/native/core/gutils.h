#ifndef G_UTILS_H
#define G_UTILS_H

#include <qwidget.h>
#include <qapplication.h>
#include <q3paintdevicemetrics.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qdatastream.h>
#include "gvaluevector.h"

#define DEFAULT_COLOR_COUNT 18
extern QString gmDefaultColorNames[];
extern QColor gmDefaultColors[];

#define DEFAULT_TEXTURE_COUNT 6
extern QString gmDefaultTextureNames[];
extern QString gmDefaultTextureFileNames[];

// 将1毫米长度转换成象素长度
float gmMMToPixel();

#define UN_PICK_UP_FBT -1

// 桩号编码
QString gEnStackNumber( const QString &lineName, const QString &pointNumber );

// 从字符串中提取整数（bit为整数的十进制位数)
int gExtractInt(const char *str, int &bit);

// 修正步长
float gCorrStep(float step0, int dot);

// IBM浮点格式转换为IEEE浮点格式
void gIBM2IEEE(void *to, const void *from, int len);

// IEEE浮点格式转换为IBM浮点格式
void gIEEE2IBM(void *to, const void *from, int len);

// 交换2字节高低顺序
void gSwapByte2(char *x);

// 交换4字节高低顺序
void gSwapByte4(char *x);

// 在文件流中跳过指定字节
void gSkipPadBytes(QDataStream &stream, int nbytes);

// 文件拷贝
bool gFileCopy(const QString &dst, const QString &src);

// 交换变量
template <class T>
void gSwap(T &a, T &b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

// 分配1维数组
template <class T>
T * gAlloc1(T *&p, int n1)
{
	p = new T[n1];
	return p;
}

// 释放1维数组
template <class T>
void gFree1(T *&p)
{
	delete []p;
	p = NULL;
}

// 为1维数组填充指定数值v
template <class T>
void gFill1(T *p, T v, int n1)
{
	int i;
	for(i=0; i<n1; i++) p[i] = v;
}

// 1维数组拷贝
template <class T>
void gCopy1(T *dst, const T *src, int n1)
{
	int i;
	for(i=0; i<n1; i++) dst[i] = src[i];
}

// 分配2维数组
template <class T>
T ** gAlloc2(T **&p, int n2, int n1)
{
	int i, size;

	size = sizeof(T);
	p = new T *[n2];
	if(p == NULL) return NULL;

	p[0] = new T[n1*n2];
	if(p[0] == NULL) 
	{
		delete []p;
		p = NULL;
		return NULL;
	}

	for(i=0; i<n2; i++)
	{
		p[i] = p[0] + n1*i;
	}
	return p;
}

// 释放2维数组
template <class T>
void gFree2(T **&p)
{
	delete []p[0];
	delete []p;
	p = NULL;
}

// 为2维数组填充指定数值v
template <class T>
void gFill2(T **p, T v, int n2, int n1)
{
	int i, j;
	for(i=0; i<n2; i++) 
	{
		for(j=0; j<n1; j++)
			p[i][j] = v;
	}
}

// 2维数组拷贝
template <class T>
void gCopy2(T **dst, const T **src, int n2, int n1)
{
	int i, j;

	for(i=0; i<n2; i++) 
	{
		for(j=0; j<n1; j++)
			dst[i][j] = src[i][j];
	}
}

// 分配3维数组
template <class T>
T *** gAlloc3(T ***&p, int n3, int n2, int n1)
{
	int i, j;

	p = new T **[n3];
	if(p == NULL) return NULL;

	p[0] = new T *[n3*n2];
	if(p[0] == NULL)
	{
		delete []p;
		p = NULL;
		return NULL;
	}

	p[0][0] = new T [n3*n2*n1];
	if(p[0][0] == NULL)
	{
		delete []p[0];
		delete []p;
		p = NULL;
		return NULL;
	}

	for(i=0; i<n3; i++)
	{
		p[i] = p[0]+n2*i;
		for(j=0; j<n2; j++)
		{
			p[i][j] = p[0][0]+n1*(j+n2*i);
		}
	}

	return p;
}

// 释放三维数组
template <class T>
void gFree3(T ***&p)
{
	delete []p[0][0];
	delete []p[0];
	delete []p;
	p = NULL;
}

// 为3维数组填充指定数值v
template <class T>
void gFill3(T ***p, T v, int n3, int n2, int n1)
{
	int i, j, k;
	for(i=0; i<n3; i++) 
	{	
		for(j=0; j<n2; j++)
		{
			for(k=0; k<n1; k++)
				p[i][j][k] = v;
		}
	}
}

// 3维数组拷贝
template <class T>
void gCopy3(T ***dst, const T ***src, int n3, int n2, int n1)
{
	int i, j, k;
	
	for(i=0; i<n3; i++) 
	{
		for(j=0; j<n2; j++)
		{
			for(k=0; k<n1; k++)
				dst[i][j][k] = src[i][j][k];
		}
	}
}

// 从缓存中载入GVector数组内容
template <class T>
void gReadValues(const QByteArray &buf, GValueVector<T> &v)
{
	Q_INT32 i, n;
	T d;
	v.clear();
	if(buf.count() <= 0) return ;

	QDataStream stream(buf, QIODevice::ReadOnly);
	stream >> n;
	for(i=0; i<n; i++)
	{
		stream >> d;
		v.append(d);
	}
}

// 将GVector数组中的内容写入缓存中
template <class T>
void gWriteValues(QByteArray &buf, GValueVector<T> &v)
{
	int i;
	QDataStream stream(buf, QIODevice::WriteOnly);
	stream << (Q_INT32)v.count();
	for(i=0; i<v.count(); i++)
	{
		stream << v[i];
	}
}

#endif
