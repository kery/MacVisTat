#ifndef GZIPFILE_H
#define GZIPFILE_H

#include <QString>
#include <QtZlib/zlib.h>

class GZipFile
{
public:
    explicit GZipFile(const QString &path);
    GZipFile(const GZipFile &) = delete;
    GZipFile& operator=(const GZipFile &) = delete;
    ~GZipFile();

    bool readLine(std::string &text);

    int progress() const;

private:
    bool close();

private:
    gzFile m_gzFile;
    qint64 m_size;
};

#endif // GZIPFILE_H
