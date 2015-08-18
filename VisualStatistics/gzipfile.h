#ifndef GZIPFILE_H
#define GZIPFILE_H

#include <QString>
#include <QtZlib/zlib.h>

class GZipFile
{
public:
    explicit GZipFile(const QString &path);
    ~GZipFile();

    bool readLine(QString &text);

    bool readLine(std::string &text);

    int completionRate() const;

    bool close();

private:
    gzFile _gzFile;
    qint64 _compressedSize;
};

#endif // GZIPFILE_H
