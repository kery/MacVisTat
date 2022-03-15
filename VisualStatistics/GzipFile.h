#ifndef GZIPFILE_H
#define GZIPFILE_H

#include <QString>
#include <string>
#if defined(Q_OS_WIN)
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

class GzipFile
{
public:
    enum OpenMode {
        ReadOnly,
        WriteOnly
    };

    GzipFile();
    ~GzipFile();

    bool open(const QString &path, OpenMode mode);
    void close();

    int read(char *buf, unsigned int maxlen);
    bool readLine(std::string &line);
    bool readLineKeepCrLf(std::string &line);
    int write(const char *buf, unsigned int len);
    int write(const std::string &str);
    bool eof() const;
    const char * error() const;
    int progress() const;

private:
    gzFile mGzFile;
    qint64 mFileSize;
};

#endif // GZIPFILE_H
