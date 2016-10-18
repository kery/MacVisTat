#include "gzipfile.h"
#include <QFile>

GZipFile::GZipFile(const QString &path) :
    m_size(0)
{
    QByteArray pathLocal8Bit = path.toLocal8Bit();
    m_gzFile = gzopen(pathLocal8Bit.data(), "rb");
    if (m_gzFile) {
        gzbuffer(m_gzFile, 64 * 1024);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            m_size = file.size();
            file.close();
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
    if (gzgets(m_gzFile, buff, sizeof(buff))) {
        text = buff;
        IF_LINE_RETURN(text)
    } else {
        return false;
    }
    while (gzgets(m_gzFile, buff, sizeof(buff))) {
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
    if (gzgets(m_gzFile, buff, sizeof(buff))) {
        text = buff;
        IF_LINE_RETURN(text)
    } else {
        return false;
    }
    while (gzgets(m_gzFile, buff, sizeof(buff))) {
        text += buff;
        IF_LINE_RETURN(text)
    }
    return true;
}

int GZipFile::progress() const
{
    if (m_size > 0) {
        return ((double)gzoffset(m_gzFile) / m_size) * 100;
    } else {
        return -1;
    }
}

bool GZipFile::close()
{
    if (m_gzFile) {
        if (gzclose(m_gzFile) == Z_OK) {
            m_gzFile = NULL;
            return true;
        }
    }
    return false;
}
