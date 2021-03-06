#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QDir>
#include <QFileInfo>

void convert(const QString &path, int min_size)
{
    const auto files = QDir(path).entryList({"*.jpg", "*.JPG", "*.png", "*.PNG", "*.JPEG", "*.jpeg"}, QDir::Files);
    for (const auto &f: files)
    {
        const auto file_path = path + '/' + f;

        QFileInfo inf(file_path);
        const auto baseName = inf.baseName();
        const auto suffix = inf.completeSuffix();

        const auto temp_path = path + '/' + baseName + ".tmp." + suffix;

        QFile::rename(file_path, temp_path);

        QImageReader reader(temp_path);

        auto img = reader.read();
        auto transformation = reader.transformation();
        const auto size = img.size();
        if (size.width() < min_size || size.height() < min_size)
        {
            qDebug() << f << "Ignored";
            continue;
        }

        if (size.width() > size.height() && size.height() > min_size)
            img = img.scaledToHeight(min_size, Qt::SmoothTransformation);
        else if (size.width() > min_size)
            img = img.scaledToWidth(min_size, Qt::SmoothTransformation);

        QImageWriter writer(file_path);
        writer.setQuality(90);
        writer.setTransformation(transformation);

        if (!writer.write(img))
        {
            qDebug() << f << "Failed to write. Reverting...";
            QFile::rename(temp_path, file_path);
        }
        else
        {
            QFileInfo img_inf(file_path);
            QFileInfo tmp_inf(temp_path);
            if (img_inf.size() > tmp_inf.size())
            {
                qDebug() << f << "Resize not effective. Reverting...";
                QFile::remove(file_path);
                QFile::rename(temp_path, file_path);
            }
            else
            {
                qDebug() << f << "Done";
                QFile::remove(temp_path);
            }
        }
    }

    const auto dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &d: dirs)
        convert(path + '/' + d, min_size);
}

void fixNames(const QString &path)
{
    const auto files = QDir(path).entryList({"*.jpg", "*.JPG", "*.png", "*.PNG", "*.JPEG", "*.jpeg"}, QDir::Files);
    for (const auto &f: files)
    {
        auto fixed = QString(f).replace(".tmp.", ".");
        if (fixed == f)
            continue;

        qDebug() << "Rename" << (path + '/' + f) << "to" << (path + '/' + fixed);
        QFile::rename(path + '/' + f, path + '/' + fixed);
    }

    const auto dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &d: dirs)
        fixNames(path + '/' + d);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const auto args = app.arguments();

    if (args.count() == 2)
    {
        fixNames(args.at(1));
    }
    else
    if (args.count() == 3)
    {
        auto path = args.at(1);
        auto min_size = args.at(2).toInt();
        if (min_size == 0)
        {
            min_size = args.at(1).toInt();
            path = args.at(2);
        }
        if (min_size == 0)
        {
            qDebug() << "Size cannot be zero";
            qDebug() << "Usage:" << app.applicationName().toStdString().c_str() << "2048 \"/path/to/root/dir\"";
            return 0;
        }

        convert(path, min_size);
    }
    else
    {
        qDebug() << "Usage:" << app.applicationName().toStdString().c_str() << "2048 \"/path/to/root/dir\"";
        return 0;
    }

    return 0;
}
