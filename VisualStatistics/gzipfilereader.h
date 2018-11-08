#pragma once

#include <QIODevice>
#include <QtZlib/zlib.h>

class GzipFileReader : public QIODevice
{
    Q_OBJECT

public:
    GzipFileReader(QObject *parent = nullptr);
    ~GzipFileReader();

    bool open(const QString &path);
    virtual void close();

    int read(char *data, int maxlen);

    bool readLine(std::string &line);
    int progress() const;

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    gzFile _gzFile;
    qint64 _fileSize;
};
