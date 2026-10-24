#ifndef IRR_FIGURE_PATHS_H
#define IRR_FIGURE_PATHS_H
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QTextStream>

// Native image paths are independent of the working directory.
class FigurePaths
{
public:
    static QString root()
    {
        QStringList starts; starts << QCoreApplication::applicationDirPath() << QDir::currentPath();
        for(int s=0;s<starts.size();s++)
        {
            QDir dir(starts[s]);
            for(int i=0;i<12;i++)
            {
                if(QFile::exists(dir.filePath("build_release.bat")))return dir.filePath("figures");
                if(!dir.cdUp())break;
            }
        }
        return QDir::current().filePath("figures");
    }
    static bool contained(const QString& path)
    {
        const QString base=QDir::cleanPath(QFileInfo(root()).absoluteFilePath())+"/";
        return QDir::cleanPath(QFileInfo(path).absoluteFilePath()).startsWith(base,Qt::CaseInsensitive);
    }
    static QString directory(const QString& output)
    {
        QString requested=QString::fromLocal8Bit(qgetenv("IRR_FIGURES_DIR"));
        if(!requested.isEmpty() && contained(requested))return QDir::cleanPath(QFileInfo(requested).absoluteFilePath());
        QString group=QString::fromLocal8Bit(qgetenv("IRR_FIGURE_GROUP"));
        if(group.isEmpty())group=output.contains("unit_test_sandbox")?"validation":"legacy/current";
        QString target=QDir(root()).filePath(group);
        if(!contained(target))target=QDir(root()).filePath("legacy/current");
        QDir().mkpath(target);return target;
    }
    static bool save(const QImage& image,const QString& path)
    {
        if(qgetenv("IRR_DISABLE_FIGURES")=="1")return true;
        if(!contained(path))return false;
        if(!QDir().mkpath(QFileInfo(path).absolutePath()))return false;
        return image.save(path,"PNG");
    }
    static bool capture(QWidget* widget,const QString& name)
    {
        return save(QPixmap::grabWidget(widget).toImage(),QDir(root()).filePath("gui/"+name));
    }
    static QString galleryDirectory()
    {
        QString current=QDir(root()).filePath("legacy/current");
        if(QFile::exists(QDir(current).filePath("index.html")))return current;
        return QDir(root()).filePath("legacy/v3");
    }
    static QString galleryImage(const QString& name)
    {
        QString path=QDir(galleryDirectory()).filePath(name);
        if(QFile::exists(path))return path;
        QFile aliases(QDir(root()).filePath("aliases.tsv"));
        if(aliases.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QString key=QDir(root()).relativeFilePath(path);QTextStream stream(&aliases);
            while(!stream.atEnd())
            {
                QStringList fields=stream.readLine().split('\t');
                if(fields.size()==2 && fields[0]==key)return QDir(root()).filePath(fields[1]);
            }
        }
        return path;
    }
};
#endif
