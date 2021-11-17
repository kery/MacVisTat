#ifndef GZIPFILE_H
#define GZIPFILE_H

#include <string>
#include <QIODevice>
#include <QtZlib/zlib.h>

class GzipFile
{
public:
    GzipFile();
    ~GzipFile();

    bool open(const QString &path, int mode=QIODevice::ReadOnly);
    void close();

    int read(char *data, unsigned int maxlen);
    int write(const char *data, unsigned int len);
    int write(const std::string &data);

    bool readLine(std::string &line, bool rmNewline=true);
    int progress() const;

private:
    gzFile _gzFile;
    qint64 _fileSize;
};

#endif // GZIPFILE_H
