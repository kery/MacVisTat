#include "gzipfile.h"

#include <QFileInfo>

GzipFile::GzipFile(QObject *parent) :
    QIODevice(parent),
    _gzFile(nullptr),
    _fileSize(-1)
{
}

GzipFile::~GzipFile()
{
    close();
}

bool GzipFile::open(const QString &path, int mode)
{
    Q_ASSERT(_gzFile == nullptr);

    _gzFile = gzopen(path.toLocal8Bit().data(), mode == QIODevice::ReadOnly ? "rb" : "wb");
    if (_gzFile != nullptr) {
        gzbuffer(_gzFile, 128 * 1024);
        if (mode == QIODevice::ReadOnly) {
            _fileSize = QFileInfo(path).size();
        }
        return QIODevice::open(QIODevice::ReadOnly);
    }
    return false;
}

void GzipFile::close()
{
    if (_gzFile != nullptr) {
        gzclose(_gzFile);
        _gzFile = nullptr;

        _fileSize = -1;
    }
    QIODevice::close();
}

int GzipFile::read(char *data, int maxlen)
{
    Q_ASSERT(_fileSize > -1);

    return (int)readData(data, maxlen);
}

int GzipFile::write(const char *data, int len)
{
    Q_ASSERT(_fileSize == -1);

    return writeData(data, len);
}

int GzipFile::write(const QString &data)
{
    QByteArray ba = data.toLatin1();
    return write(ba.data(), ba.length());
}

bool GzipFile::readLine(std::string &line)
{
    Q_ASSERT(_fileSize > -1);

    char buffer[4096];

    line.clear();

    while (gzgets(_gzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            line.pop_back();
            if (line.back() == '\r') {
                line.pop_back();
            }
            return true;
        }
    }

    return gzeof(_gzFile);
}

int GzipFile::progress() const
{
    Q_ASSERT(_fileSize > -1);

    if (_fileSize > 0) {
        return static_cast<double>(gzoffset(_gzFile)) / _fileSize * 100;
    }
    return -1;
}

qint64 GzipFile::readData(char *data, qint64 maxlen)
{
    Q_ASSERT(_fileSize > -1);
    Q_ASSERT(maxlen <= UINT_MAX);

    return gzread(_gzFile, data, maxlen);
}

qint64 GzipFile::writeData(const char *data, qint64 len)
{
    Q_ASSERT(_fileSize == -1);
    Q_ASSERT(len <= UINT_MAX);

    return gzwrite(_gzFile, data, len);
}
