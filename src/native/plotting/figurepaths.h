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
        QString configured=QString::fromLocal8Bit(qgetenv("IRR_PROJECT_ROOT"));
        if(!configured.isEmpty() && QFile::exists(QDir(configured).filePath("build.bat")))
            return QDir(configured).absoluteFilePath("figures");
        QStringList starts; starts << QCoreApplication::applicationDirPath() << QDir::currentPath();
        for(int s=0;s<starts.size();s++)
        {
            QDir dir(starts[s]);
            for(int i=0;i<12;i++)
            {
                if(QFile::exists(dir.filePath("build.bat")))return dir.filePath("figures");
                if(!dir.cdUp())break;
            }
        }
        return QDir::current().filePath("figures");
    }
    static QString artifactPath(const QString& relative)
    {
        QDir project(QFileInfo(root()).absolutePath());
        if(relative=="results/current" || relative.startsWith("results/current/"))
        {
            QFile pointer(project.filePath("results/active.txt"));
            if(pointer.open(QIODevice::ReadOnly|QIODevice::Text))
            {
                QString active=QString::fromUtf8(pointer.readAll()).trimmed();
                QString target=QDir::cleanPath(project.absoluteFilePath(active));
                QString base=QDir::cleanPath(project.absoluteFilePath("results"))+"/";
                if(target.startsWith(base,Qt::CaseInsensitive))
                    return target+relative.mid(QString("results/current").size());
                return project.filePath("results/invalid_active_experiment");
            }
        }
        return project.absoluteFilePath(relative);
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
        if(group.isEmpty())group=output.contains("unit_test_sandbox")?"validation":"interactive";
        QString target=QDir(root()).filePath(group);
        if(!contained(target))target=QDir(root()).filePath("interactive");
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
};
#endif
