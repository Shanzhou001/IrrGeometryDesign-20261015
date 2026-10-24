#include "segyio.h"

void CSegyIO::initialize_read(QString file_name)
{
    file_read.setFileName(file_name);
	file_information.setFile(file_read);
	file_size = file_information.size();
	file_read.open(QIODevice::ReadOnly);
	in.setDevice(&file_read);
	in.setByteOrder(QDataStream::BigEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);


//	in.setByteOrder(QDataStream::LittleEndian);
}

void CSegyIO::initinitialize_ReadWrite(QString file_name)
{
	file_read.setFileName(file_name);
	file_information.setFile(file_read);
	file_size = file_information.size();
	file_read.open(QIODevice::ReadWrite);
	in.setDevice(&file_read);
	in.setByteOrder(QDataStream::BigEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
}

void CSegyIO::file_seek(int tracenumber)
{
	long long postemp=240+binary_header.hns*4;
	long long pos=3600+tracenumber*postemp;
//	in.skipRawData(pos);
	file_read.seek(pos);
}

void CSegyIO::file_seekPos(long long datapos)
{
	file_read.seek(datapos);
}

void CSegyIO::file_writeSeekPos(long long datapos)
{
	file_write.seek(datapos);
}

void CSegyIO::read_ebcbuf()
{
    in.readRawData(ebcbuf,3200);     //读入卷头说明文字
}

void CSegyIO::read_binary_header()//读文件头
{
    in>>binary_header.jobid>>binary_header.lino>>binary_header.reno>>binary_header.ntrpr>>binary_header.nart
	   >>binary_header.hdt>>binary_header.dto>>binary_header.hns>>binary_header.nso>>binary_header.format
	   >>binary_header.fold>>binary_header.tsort>>binary_header.vscode>>binary_header.hsfs>>binary_header.hsfe
	   >>binary_header.hslen>>binary_header.hstyp>>binary_header.schn>>binary_header.hstas>>binary_header.hstae
	   >>binary_header.htatyp>>binary_header.hcorr>>binary_header.bgrcv>>binary_header.rcvm>>binary_header.mfeet
	   >>binary_header.polyt>>binary_header.vpol;
	for (int i=0;i<170;i++)
	{
		in>>binary_header.hunass[i];
	}
	sample_num_every_trace = binary_header.hns;
	data = new float[sample_num_every_trace];
	for (int i=0;i<sample_num_every_trace;i++)
	{
		data[i] = 0.0;
	}
}
//一道往下追加的数据
void CSegyIO::alloc_append(int appendlen)
{
	appenddata=new float[appendlen];
	for (int i=0;i<appendlen;i++)
	{
		appenddata[i] = 0.0;
	}
}

void CSegyIO::read_trace_header()//读道头
{
	in>>trace_header.tracl>>trace_header.tracr>>trace_header.fldr>>trace_header.tracf>>trace_header.ep
		>>trace_header.cdp>>trace_header.cdpt>>trace_header.trid>>trace_header.nvs>>trace_header.nhs
		>>trace_header.duse>>trace_header.offset>>trace_header.gelev>>trace_header.selev>>trace_header.sdepth
		>>trace_header.gdel>>trace_header.sdel>>trace_header.swdep>>trace_header.gwdep>>trace_header.scalel
		>>trace_header.scalco>>trace_header.sx>>trace_header.sy>>trace_header.gx>>trace_header.gy>>trace_header.counit
		>>trace_header.wevel>>trace_header.swevel>>trace_header.sut>>trace_header.gut>>trace_header.sstat
		>>trace_header.gstat>>trace_header.tstat>>trace_header.laga>>trace_header.lagb>>trace_header.delrt
		>>trace_header.muts>>trace_header.mute>>trace_header.ns>>trace_header.dt>>trace_header.gain
		>>trace_header.igc>>trace_header.igi>>trace_header.corr>>trace_header.sfs>>trace_header.sfe
		>>trace_header.slen>>trace_header.styp>>trace_header.stas>>trace_header.stae>>trace_header.tatyp
		>>trace_header.afilf>>trace_header.afils>>trace_header.nofilf>>trace_header.nofils>>trace_header.lcf
		>>trace_header.hcf>>trace_header.lcs>>trace_header.hcs>>trace_header.year>>trace_header.day>>trace_header.hour
		>>trace_header.minute>>trace_header.sec>>trace_header.timbas>>trace_header.trwf>>trace_header.grnors
		>>trace_header.grnofr>>trace_header.grnlof>>trace_header.gaps>>trace_header.otrav>>trace_header.d1
		>>trace_header.f1>>trace_header.d2>>trace_header.f2>>trace_header.ungpow>>trace_header.unscale
		>>trace_header.ntr>>trace_header.mark>>trace_header.shortpad;
	for (int i=0;i<14;i++)
	{
		in>>trace_header.unass[i];
	}
}

void CSegyIO::read_trace_data(int length)//读道数据
{
    for (int i=0;i<length;i++)
    {
		in>>data[i];
    }
	IBM_to_FLOAT((int *) data,(int *) data,length,1);
}

void CSegyIO::colse_read()//关闭数据
{
	delete data;
    file_read.close();
}

void CSegyIO::IBM_to_FLOAT(int from[], int to[], int n, int endian)
{
	register int fconv, fmant, i, t;

	for (i = 0;i < n; ++i) 
	{
		fconv = from[i];
		/* if little endian, i.e. endian=0 do this */
		if (endian == 0) fconv = (fconv << 24) | ((fconv >> 24) & 0xff) |
			((fconv & 0xff00) << 8) | ((fconv & 0xff0000) >> 8);

		if (fconv)
		{
			fmant = 0x00ffffff & fconv;
			/* The next two lines were added by Toralf Foerster */
			/* to trap non-IBM format data i.e. conv=0 data  */

			t = (int) ((0x7f000000 & fconv) >> 22) - 130;
			while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
			if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
			else if (t <= 0) fconv = 0;
			else fconv =   (0x80000000 & fconv) | (t << 23)
				| (0x007fffff & fmant);
		}
		to[i] = fconv;
	}
	return;
}

void CSegyIO::initialize_write(QString file_name)//写文件
{
	file_write.setFileName(file_name);
	file_write.open(QIODevice::WriteOnly);
	out.setDevice(&file_write);
	out.setByteOrder(QDataStream::BigEndian);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);

//	out.setByteOrder(QDataStream::LittleEndian);
}

void CSegyIO::initialize_writeReplace(QString file_name)//写文件,更新文件中的数据
{
	file_write.setFileName(file_name);
//	file_write.open(IO_ReadWrite);
	file_write.open(QIODevice::ReadWrite);
	out.setDevice(&file_write);
	out.setByteOrder(QDataStream::BigEndian);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);

	//	out.setByteOrder(QDataStream::LittleEndian);
}

void CSegyIO::initialize_append(QString file_name)//写文件
{
	file_write.setFileName(file_name);
	file_write.open(QIODevice::Append);
	out.setDevice(&file_write);
	out.setByteOrder(QDataStream::BigEndian);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);
}

void CSegyIO::write_ebcbuf()//写卷头
{
    out.writeRawData(ebcbuf,3200);
}

void CSegyIO::write_binary_header()//写文件头
{
	out<<binary_header.jobid<<binary_header.lino<<binary_header.reno<<binary_header.ntrpr<<binary_header.nart
	   <<binary_header.hdt<<binary_header.dto<<binary_header.hns<<binary_header.nso<<binary_header.format
	   <<binary_header.fold<<binary_header.tsort<<binary_header.vscode<<binary_header.hsfs<<binary_header.hsfe
	   <<binary_header.hslen<<binary_header.hstyp<<binary_header.schn<<binary_header.hstas<<binary_header.hstae
	   <<binary_header.htatyp<<binary_header.hcorr<<binary_header.bgrcv<<binary_header.rcvm<<binary_header.mfeet
	   <<binary_header.polyt<<binary_header.vpol;
	for (int i=0;i<170;i++)
	{
		out<<binary_header.hunass[i];
	}
}

void CSegyIO::write_trace_header()//写道头
{
	out<<trace_header.tracl<<trace_header.tracr<<trace_header.fldr<<trace_header.tracf<<trace_header.ep
		<<trace_header.cdp<<trace_header.cdpt<<trace_header.trid<<trace_header.nvs<<trace_header.nhs
		<<trace_header.duse<<trace_header.offset<<trace_header.gelev<<trace_header.selev<<trace_header.sdepth
		<<trace_header.gdel<<trace_header.sdel<<trace_header.swdep<<trace_header.gwdep<<trace_header.scalel
		<<trace_header.scalco<<trace_header.sx<<trace_header.sy<<trace_header.gx<<trace_header.gy<<trace_header.counit
		<<trace_header.wevel<<trace_header.swevel<<trace_header.sut<<trace_header.gut<<trace_header.sstat
		<<trace_header.gstat<<trace_header.tstat<<trace_header.laga<<trace_header.lagb<<trace_header.delrt
		<<trace_header.muts<<trace_header.mute<<trace_header.ns<<trace_header.dt<<trace_header.gain
		<<trace_header.igc<<trace_header.igi<<trace_header.corr<<trace_header.sfs<<trace_header.sfe
		<<trace_header.slen<<trace_header.styp<<trace_header.stas<<trace_header.stae<<trace_header.tatyp
		<<trace_header.afilf<<trace_header.afils<<trace_header.nofilf<<trace_header.nofils<<trace_header.lcf
		<<trace_header.hcf<<trace_header.lcs<<trace_header.hcs<<trace_header.year<<trace_header.day<<trace_header.hour
		<<trace_header.minute<<trace_header.sec<<trace_header.timbas<<trace_header.trwf<<trace_header.grnors
		<<trace_header.grnofr<<trace_header.grnlof<<trace_header.gaps<<trace_header.otrav<<trace_header.d1
		<<trace_header.f1<<trace_header.d2<<trace_header.f2<<trace_header.ungpow<<trace_header.unscale
		<<trace_header.ntr<<trace_header.mark<<trace_header.shortpad;
	for (int i=0;i<14;i++)
	{
		out<<trace_header.unass[i];
	}
}

void CSegyIO::write_trace_data(int length)//写数据
{
	FLOAT_to_IBM((int *) data,(int *) data,length,1);
	for (int i=0;i<length;i++)
	{
		out<<data[i];
	}
}

void CSegyIO::write_replace(short length)//替换一个数据
{
	out<<length;
}

void CSegyIO::write_trace_data_renyi(int length,float *renyidata)//写数据
{
	//	FLOAT_to_IBM((int *) data,(int *) data,length,1);
	for (int i=0;i<length;i++)
	{
		out<<renyidata[i];
	}
}
//写入一道追加的数据
void CSegyIO::write_append_data(int length)
{
	for (int i=0;i<length;i++)
	{
		out<<appenddata[i];
	}
}

void CSegyIO::close_write()
{
	file_write.close();
}

void CSegyIO::remove_write()
{
	file_write.remove();
}

void CSegyIO::FLOAT_to_IBM(int from[], int to[], int n, int endian)
{
	register int fconv, fmant, i, t, dummy;

	dummy = endian;

	for (i=0;i<n;++i) {
		fconv = from[i];
		if (fconv) {
			fmant = (0x007fffff & fconv) | 0x00800000;
			t = (int) ((0x7f800000 & fconv) >> 23) - 126;
			while (t & 0x3) { ++t; fmant >>= 1; }
			fconv = (0x80000000 & fconv) | (((t>>2) + 64) << 24) | fmant;
		}
		to[i] = fconv;
	}
	return;
}