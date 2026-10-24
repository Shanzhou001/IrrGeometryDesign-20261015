// Targeted standalone checks for the preserved Qt 4 SEG-Y utility class.
#include "spssegy/segyio.h"
#include <QFile>
#include <QByteArray>
#include <cmath>
#include <cstring>
#include <cstdio>

static void put16(QByteArray& bytes,int offset,unsigned value)
{bytes[offset]=(char)(value>>8);bytes[offset+1]=(char)value;}
static void put32(QByteArray& bytes,int offset,unsigned value)
{for(int k=0;k<4;k++)bytes[offset+k]=(char)(value>>(24-8*k));}
static bool fixture(const QString& name,int format)
{
    QByteArray bytes(3848,0);put16(bytes,3220,2);put16(bytes,3224,format);
    put16(bytes,3714,2);put16(bytes,3716,8000);
    put32(bytes,3840,format==1?0x41100000U:0x3f800000U);
    put32(bytes,3844,format==1?0xc1200000U:0xc0000000U);
    QFile file(name);return file.open(QIODevice::WriteOnly) && file.write(bytes)==bytes.size();
}
static void load(CSegyIO& io,const QString& path)
{io.initialize_read(path);io.read_ebcbuf();io.read_binary_header();io.read_trace_header();}

int main(int argc,char** argv)
{
    if(argc!=2)return 2;
    const char* mode=argv[1];CSegyIO io;
    if(strcmp(mode,"ibm_zero")==0)
    {
        int words[3]={0,0x41000000,(int)0x80000000U},output[3]={1,1,1};
        io.IBM_to_FLOAT(words,output,3,1);
        return output[0] || output[1] || output[2];
    }
    if(strcmp(mode,"ibm_known")==0)
    {
        int words[3]={0x41100000,(int)0xc1200000U,0x40800000},output[3];float f[3];
        io.IBM_to_FLOAT(words,output,3,1);memcpy(f,output,sizeof(f));
        if(f[0]!=1 || f[1]!=-2 || f[2]!=.5f)return 1;
        int swapped=0x00001041,value=0;io.IBM_to_FLOAT(&swapped,&value,1,0);memcpy(f,&value,4);
        return f[0]!=1;
    }
    const int format=strcmp(mode,"ibm_file")==0?1:5;
    if(!fixture("codec_fixture.sgy",format))return 2;
    load(io,"codec_fixture.sgy");
    if(strcmp(mode,"bounds")==0)
    {
        float before0=io.data[0],before1=io.data[1];io.read_trace_data(3);
        return io.read_ok() || io.data[0]!=before0 || io.data[1]!=before1;
    }
    if(strcmp(mode,"lifetime")==0)
    {
        io.file_seekPos(3200);io.read_binary_header();io.colse_read();
        if(io.data!=NULL)return 1;
        io.colse_read();io.alloc_append(8);io.alloc_append(4);io.alloc_append(0);
        return io.appenddata!=NULL;
    }
    if(strcmp(mode,"ieee_file")!=0 && strcmp(mode,"ibm_file")!=0)return 2;
    io.read_trace_data(2);bool ok=fabs(io.data[0]-1.f)<1e-7 && fabs(io.data[1]+2.f)<1e-7;
    fprintf(stdout,"decoded %.9g %.9g\n",io.data[0],io.data[1]);io.colse_read();
    return ok?0:1;
}
