#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QDebug>
#include <stdio.h>

int main(int argc, char *argv[])
{
    // Before setting codec
    printf("Default codecForLocale: %s\n", QTextCodec::codecForLocale() ? QTextCodec::codecForLocale()->name().constData() : "null");
    printf("Default codecForCStrings: %s\n", QTextCodec::codecForCStrings() ? QTextCodec::codecForCStrings()->name().constData() : "null");

    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec) codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    printf("After setting, codec name: %s\n", codec->name().constData());

    QString s1 = QString::fromLocal8Bit("【多采样率基准对比试验完成】");
    QString s2 = QString::fromUtf8("【多采样率基准对比试验完成】");
    QString s3 = QString("【多采样率基准对比试验完成】");

    printf("s1 utf8 len: %d, local8bit len: %d\n", s1.toUtf8().size(), s1.toLocal8Bit().size());
    printf("s2 utf8 len: %d, local8bit len: %d\n", s2.toUtf8().size(), s2.toLocal8Bit().size());
    printf("s3 utf8 len: %d, local8bit len: %d\n", s3.toUtf8().size(), s3.toLocal8Bit().size());

    // Write to a file in utf8 and gbk
    FILE *f1 = fopen("test_s1_local.txt", "wb");
    QByteArray b1 = s1.toLocal8Bit();
    fwrite(b1.constData(), 1, b1.size(), f1);
    fclose(f1);

    FILE *f2 = fopen("test_s2_local.txt", "wb");
    QByteArray b2 = s2.toLocal8Bit();
    fwrite(b2.constData(), 1, b2.size(), f2);
    fclose(f2);

    return 0;
}
