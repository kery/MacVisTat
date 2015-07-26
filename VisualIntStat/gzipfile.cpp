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

bool GZipFile::readLine(QString &text)
{
    char buff[4096];
    text = "";
    while (gzgets(_gzFile, buff, sizeof(buff))) {
        text += buff;
        if (text.endsWith('\n')) {
            break;
        }
    }
    return !text.isEmpty();
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
