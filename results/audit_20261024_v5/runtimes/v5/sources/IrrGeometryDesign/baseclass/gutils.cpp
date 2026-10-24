#include "gutils.h"
#include "qfile.h"
#include <ctype.h>
#include <stdlib.h>
#include <QDesktopWidget>
//----默认颜色处理--------------------------------------------------------------
QString gmDefaultColorNames[] = 
{
	" Qt::white",		" Qt::black",		" Qt::red",
	" darkRed",		" green",		" darkGreen",
	" Qt::blue",		" darkBlue",	" cyan", 
	" darkCyan",	" magenta",		" darkMagenta", 
	" yellow",		" darkYellow",	" Qt::gray",
	" Qt::darkGray",	" lightGray",	" custom"
};

QColor gmDefaultColors[] =
{
	Qt::white,		Qt::black,		Qt::red,
	Qt::darkRed,	Qt::green,		Qt::darkGreen,
	Qt::blue,		Qt::darkBlue,	Qt::cyan,
	Qt::darkCyan,	Qt::magenta,	Qt::darkMagenta,
	Qt::yellow,		Qt::darkYellow,	Qt::gray,
	Qt::darkGray,	Qt::lightGray,	QColor(128, 128, 192)
};

//----默认纹理处理--------------------------------------------------------------
QString gmDefaultTextureNames[] = 
{
	QString::fromLocal8Bit("空"),
	QString::fromLocal8Bit("混合岩"),
	QString::fromLocal8Bit("渗透状混合岩"),
	QString::fromLocal8Bit("斑点状混合岩"),
	QString::fromLocal8Bit("眼球状混合岩"),
	"custom"
};

QString gmDefaultTextureFileNames[] = 
{
	"",
	"b001.png",
	"b002.png",
	"b003.png",
	"b004.png"
};

//----实用全局函数--------------------------------------------------------------
float gmMMToPixel()
{
	QDesktopWidget *d = QApplication::desktop();
    Q3PaintDeviceMetrics pdm(d);
	return (float)pdm.width() / pdm.widthMM(); 
}

void gSwapByte4(char *x)
{
	unsigned char tmp;
	tmp = x[0];
	x[0] = x[3];
	x[3] = tmp;
	tmp = x[1];
	x[1] = x[2];
	x[2] = tmp;
}

void gSwapByte2(char *x)
{
	unsigned char tmp;
	tmp = x[0];
	x[0] = x[1];
	x[1] = tmp;
}

void gIBM2IEEE(void *to, const void *from, int len)
{
	register unsigned fr;		// fraction
	register int exp;			// exponent
	register int sgn;			// sign 

	for (; len-- > 0; to = (char *)to + 4, from = (char *)from + 4) {
		// split into sign, exponent, and fraction 
		fr = *(long *)from;			// pick up value
		gSwapByte4((char *)&fr);
		sgn = fr >> 31;				// save sign
		fr <<= 1;					// shift sign out
		exp = fr >> 25;				// save exponent
		fr <<= 7;					// shift exponent out

		if (fr == 0) {				// short-circuit for zero
			exp = 0;
			goto done;
		}

		// adjust exponent from base 16 offset 64 radix point before first digit
		// to base 2 offset 127 radix point after first digit
		// (exp - 64) * 4 + 127 - 1 == exp * 4 - 256 + 126 == (exp << 2) - 130
		exp = (exp << 2) - 130;

		// (re)normalize 
		while (fr < 0x80000000) { // 3 times max for normalized input
			--exp;
			fr <<= 1;
		}

		if (exp <= 0) {			// underflow 
			if (exp < -24)		// complete underflow - return properly signed zero 
				fr = 0;
			else // partial underflow - return denormalized number 
				fr >>= -exp;
			exp = 0;
		} else if (exp >= 255) { // overflow - return infinity 
			fr = 0;
			exp = 255;
		} else {				// just a plain old number - remove the assumed high bit
			fr <<= 1;
		}

done:
		// put the pieces back together and return it
		*(unsigned *)to = (fr >> 9) | (exp << 23) | (sgn << 31);
	}
}

void gIEEE2IBM(void *to, const void *from, int len)
{
	register unsigned fr;		// fraction
	register int exp;			// exponent
	register int sgn;			// sign 

	for (; len-- > 0; to = (char *)to + 4, from = (char *)from + 4) {
		// split into sign, exponent, and fraction 
		fr = *(unsigned *)from; // pick up value 
		sgn = fr >> 31;			// save sign 
		fr <<= 1;				// shift sign out 
		exp = fr >> 24;			// save exponent
		fr <<= 8;				// shift exponent out

		if (exp == 255) {		// infinity (or NAN) - map to largest
			fr = 0xffffff00;
			exp = 0x7f;
			goto done;
		}
		else if (exp > 0)		// add assumed digit
			fr = (fr >> 1) | 0x80000000;
		else if (fr == 0)		// short-circuit for zero
			goto done;

		// adjust exponent from base 2 offset 127 radix point after first digit
		// to base 16 offset 64 radix point before first digit
		exp += 130;
		fr >>= -exp & 3;
		exp = (exp + 3) >> 2;

		// (re)normalize 
		while (fr < 0x10000000) { // never executed for normalized input 
			--exp;
			fr <<= 4;
		}

done:
		// put the pieces back together and return it 
		fr = (fr >> 8) | (exp << 24) | (sgn << 31);
		gSwapByte4((char *)&fr);
		*(unsigned *)to = fr;//htonl(fr);
	}
}

QString gEnStackNumber( const QString &lineName, const QString &pointNumber )
{
	return lineName.stripWhiteSpace()+pointNumber.stripWhiteSpace();
}

int gExtractInt(const char *str, int &bit)
{
	int i;
	int len = strlen(str);
	char *strNum = new char[len+1];
	memset(strNum, 0, len+1);

	bool isFindDigit = false;
	for(i=0, bit=0; i<len; i++)
	{
		if(!isdigit(str[i]))
		{
			if(isFindDigit) break;	//已经发现了整数串，后出现非数字跳出
		}
		else 
		{
			isFindDigit = true;
			strNum[bit] = str[i];
			bit++;
		}
	}
	
	int r = atoi(strNum);
	delete []strNum;
	return r;
}

void gSkipPadBytes(QDataStream &stream, int nbytes)
{
	QIODevice *dev = stream.device();
	if(dev->isWritable()) 
	{
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		dev->writeBlock(buf, nbytes);
	}
	else stream.device()->at(stream.device()->at()+nbytes);
}

bool gFileCopy(const QString &dst, const QString &src)
{
	QFile srcFile(src);
	QFile dstFile(dst);

	if(!srcFile.open(QIODevice::ReadOnly)) return false;
	if(!dstFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
	{
		srcFile.close();
		return false;
	}

	uint len, bufLen, size;
	char buf[1024];
	bufLen = 1024;
	size = srcFile.size();
	for(len=bufLen; len<size; len+=bufLen)
	{
		srcFile.readBlock(buf, bufLen);
		dstFile.writeBlock(buf, bufLen);
	}
	len = size-len+bufLen;
	srcFile.readBlock( buf, len );
	dstFile.writeBlock( buf, len );
	dstFile.flush();

	srcFile.close();
	if(dstFile.status() != IO_Ok) 
	{
		dstFile.close();
		return false;
	}
	dstFile.close();
	return true;
}

float gCorrStep(float step0, int dot)
{
	int i, e, p, s;
	float step;
	
	e = 0;
	p = 1;
	step = step0;
	for(i=0; i<dot; i++)
	{
		p *= 10;
	}
	if(step < p)
	{
		while(step < p)
		{
			step *= 10;
			e++;
		}
	}
	else
	{
		while(step > p)
		{
			step /= 10;
			e--;
		}
	}
	
	s = (int)(step/5);
	if(s*5 < step) step = (s+1)*5;
	else step = s*5;
	
	if(e > 0)
	{
		while(e > 0)
		{
			step /= 10;
			e--;
		}		
	}
	else
	{
		while(e < 0)
		{
			step *= 10;
			e++;
		}
	}
	return step;
}