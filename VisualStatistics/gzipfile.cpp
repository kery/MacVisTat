#include "gzipfile.h"

GZipFile::GZipFile(const QString &path) :
    _gzFile(gzopen(path.toStdString().c_str(), "rb"))
{
    if (_gzFile) {
        gzbuffer(_gzFile, 8192);
    }
}

GZipFile::~GZipFile()
{
    close();
}

#define IF_LINE_RETURN(text) \
if (text.endsWith('\n')) {\
    if (text.endsWith("\r\n")) {\
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
