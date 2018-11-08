#include "GzipFileReader.h"

#include <QFileInfo>

GzipFileReader::GzipFileReader(QObject *parent) :
    QIODevice(parent),
    _gzFile(nullptr),
    _fileSize(-1)
{
}

GzipFileReader::~GzipFileReader()
{
    close();
}

bool GzipFileReader::open(const QString &path)
{
    Q_ASSERT(_gzFile == nullptr);

    _gzFile = gzopen(path.toLocal8Bit().data(), "rb");
    if (_gzFile != nullptr) {
        gzbuffer(_gzFile, 128 * 1024);
        _fileSize = QFileInfo(path).size();
        return QIODevice::open(QIODevice::ReadOnly);
    }
    return false;
}

void GzipFileReader::close()
{
    if (_gzFile != nullptr) {
        gzclose(_gzFile);
        _gzFile = nullptr;

        _fileSize = -1;
    }
    QIODevice::close();
}

int GzipFileReader::read(char *data, int maxlen)
{
    return (int)readData(data, maxlen);
}

bool GzipFileReader::readLine(std::string &line)
{
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

int GzipFileReader::progress() const
{
    if (_fileSize > 0) {
        return static_cast<double>(gzoffset(_gzFile)) / _fileSize * 100;
    }
    return -1;
}

qint64 GzipFileReader::readData(char *data, qint64 maxlen)
{
    Q_ASSERT(maxlen <= UINT_MAX);

    return gzread(_gzFile, data, maxlen);
}

qint64 GzipFileReader::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    Q_ASSERT(false);

    return -1;
}
