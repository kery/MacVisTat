#pragma once

#include <QIODevice>
#if defined(Q_OS_WIN)
    #include <QtZlib/zlib.h>
#elif defined(Q_OS_LINUX)
    #include <zlib.h>
#endif

class GzipFile : public QIODevice
{
    Q_OBJECT

public:
    GzipFile(QObject *parent = nullptr);
    ~GzipFile();

    bool open(const QString &path, int mode=QIODevice::ReadOnly);
    virtual void close();

    int read(char *data, int maxlen);
    int write(const char *data, int len);
    int write(const QString &data);

    bool readLine(std::string &line);
    int progress() const;

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    gzFile _gzFile;
    qint64 _fileSize;
};
