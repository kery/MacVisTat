#include "GZipFile.h"
#include <QFileInfo>

GZipFile::GZipFile() :
    _gzFile(nullptr),
    _fileSize(-1)
{
}

GZipFile::~GZipFile()
{
    close();
}

bool GZipFile::open(const QString &path, OpenMode mode)
{
    QByteArray baStr = path.toLocal8Bit();
    _gzFile = gzopen(baStr.data(), mode == ReadOnly ? "rb" : "wb");
    if (_gzFile) {
        gzbuffer(_gzFile, 128 * 1024);
        if (mode == ReadOnly) {
            _fileSize = QFileInfo(path).size();
        }
        return true;
    }
    return false;
}

void GZipFile::close()
{
    if (_gzFile) {
        gzclose(_gzFile);
        _gzFile = nullptr;
        _fileSize = -1;
    }
}

int GZipFile::read(char *buf, unsigned int maxlen)
{
    return gzread(_gzFile, buf, maxlen);
}

bool GZipFile::readLine(std::string &line)
{
    line.clear();

    char buffer[4096];
    while (gzgets(_gzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            line.pop_back();
            if (line.back() == '\r') {
                line.pop_back();
            }
            break;
        }
    }
    return !line.empty() || gzeof(_gzFile) == 0;
}

bool GZipFile::readLineKeepCrLf(std::string &line)
{
    line.clear();

    char buffer[4096];
    while (gzgets(_gzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            break;
        }
    }
    return !line.empty();
}

int GZipFile::write(const char *buf, unsigned int len)
{
    return gzwrite(_gzFile, buf, len);
}

int GZipFile::write(const std::string &str)
{
    return write(str.c_str(), static_cast<unsigned int>(str.length()));
}

bool GZipFile::eof() const
{
    return gzeof(_gzFile) != 0;
}

const char *GZipFile::error() const
{
    return gzerror(_gzFile, nullptr);
}

int GZipFile::progress() const
{
    if (_fileSize > 0) {
        return static_cast<double>(gzoffset(_gzFile)) / _fileSize * 100;
    }
    return -1;
}
