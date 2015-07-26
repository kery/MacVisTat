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

    bool close();

private:
    gzFile _gzFile;
};

#endif // GZIPFILE_H
