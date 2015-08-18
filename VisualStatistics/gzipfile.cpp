#include "gzipfile.h"
#include <QFile>

GZipFile::GZipFile(const QString &path) :
    _gzFile(gzopen(path.toStdString().c_str(), "rb")),
    _compressedSize(0)
{
    if (_gzFile) {
        gzbuffer(_gzFile, 64 * 1024);
        QFile tmpFile(path);
        if (tmpFile.open(QFile::ReadOnly)) {
            _compressedSize = tmpFile.size();
            tmpFile.close();
        }
    }
}

GZipFile::~GZipFile()
{
    close();
}

#define IF_LINE_RETURN(text) \
if (text.endsWith('\n')) {\
    if (text.endsWith(QLatin1String("\r\n"))) {\
        text.truncate(text.length() - 2);\
        return true;\
    }\
    text.truncate(text.length() - 1);\
    return true;\
}

bool GZipFile::readLine(QString &text)
{
    char buff[4096];
    if (gzgets(_gzFile, buff, sizeof(buff))) {
        text = buff;
        IF_LINE_RETURN(text)
    } else {
        return false;
    }
    while (gzgets(_gzFile, buff, sizeof(buff))) {
        text += buff;
        IF_LINE_RETURN(text)
    }
    return true;
}

#undef IF_LINE_RETURN
#define IF_LINE_RETURN(text) \
if (text.back() == '\n') {\
    text.resize(text.length() - 1);\
    if (text.back() == '\r') {\
        text.resize(text.length() - 1);\
    }\
    return true;\
}

bool GZipFile::readLine(std::string &text)
{
    char buff[4096];
    if (gzgets(_gzFile, buff, sizeof(buff))) {
        text = buff;
        IF_LINE_RETURN(text)
    } else {
        return false;
    }
    while (gzgets(_gzFile, buff, sizeof(buff))) {
        text += buff;
        IF_LINE_RETURN(text)
    }
    return true;
}

int GZipFile::completionRate() const
{
    if (_compressedSize > 0) {
        return ((double)gzoffset(_gzFile) / _compressedSize) * 100;
    } else {
        return -1;
    }
}

bool GZipFile::close()
{
    if (_gzFile) {
        if (gzclose(_gzFile) == Z_OK) {
            _gzFile = NULL;
            return true;
        }
    }
    return false;
}
