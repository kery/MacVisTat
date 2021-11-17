#include "GzipFile.h"

#include <QFileInfo>

GzipFile::GzipFile() :
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
    _gzFile = gzopen(path.toLocal8Bit().data(), mode == QIODevice::ReadOnly ? "rb" : "wb");
    if (_gzFile != nullptr) {
        gzbuffer(_gzFile, 128 * 1024);
        if (mode == QIODevice::ReadOnly) {
            _fileSize = QFileInfo(path).size();
        }
        return true;
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
}

int GzipFile::read(char *data, unsigned int maxlen)
{
    Q_ASSERT(_fileSize > -1);

    return gzread(_gzFile, data, maxlen);
}

int GzipFile::write(const char *data, unsigned int len)
{
    Q_ASSERT(_fileSize == -1);

    return gzwrite(_gzFile, data, len);
}

int GzipFile::write(const std::string &data)
{
    return write(data.c_str(), static_cast<unsigned int>(data.length()));
}

bool GzipFile::readLine(std::string &line, bool rmNewline)
{
    Q_ASSERT(_fileSize > -1);

    line.clear();

    char buffer[4096];
    while (gzgets(_gzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            if (rmNewline) {
                line.pop_back();
                if (line.back() == '\r') {
                    line.pop_back();
                }
            }
            break;
        }
    }

    return !(line.empty() && gzeof(_gzFile));
}

int GzipFile::progress() const
{
    Q_ASSERT(_fileSize > -1);

    if (_fileSize > 0) {
        return static_cast<double>(gzoffset(_gzFile)) / _fileSize * 100;
    }
    return -1;
}
