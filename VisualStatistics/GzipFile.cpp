#include "GZipFile.h"
#include <QFileInfo>

GZipFile::GZipFile() :
    mGzFile(nullptr),
    mFileSize(-1)
{
}

GZipFile::~GZipFile()
{
    close();
}

bool GZipFile::open(const QString &path, OpenMode mode)
{
    QByteArray baStr = path.toLocal8Bit();
    mGzFile = gzopen(baStr.data(), mode == ReadOnly ? "rb" : "wb");
    if (mGzFile) {
        gzbuffer(mGzFile, 128 * 1024);
        if (mode == ReadOnly) {
            mFileSize = QFileInfo(path).size();
        }
        return true;
    }
    return false;
}

void GZipFile::close()
{
    if (mGzFile) {
        gzclose(mGzFile);
        mGzFile = nullptr;
        mFileSize = -1;
    }
}

int GZipFile::read(char *buf, unsigned int maxlen)
{
    return gzread(mGzFile, buf, maxlen);
}

bool GZipFile::readLine(std::string &line)
{
    line.clear();

    char buffer[4096];
    while (gzgets(mGzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            line.pop_back();
            if (line.back() == '\r') {
                line.pop_back();
            }
            break;
        }
    }
    return !line.empty() || gzeof(mGzFile) == 0;
}

bool GZipFile::readLineKeepCrLf(std::string &line)
{
    line.clear();

    char buffer[4096];
    while (gzgets(mGzFile, buffer, sizeof(buffer))) {
        line.append(buffer);
        if (line.back() == '\n') {
            break;
        }
    }
    return !line.empty();
}

int GZipFile::write(const char *buf, unsigned int len)
{
    return gzwrite(mGzFile, buf, len);
}

int GZipFile::write(const std::string &str)
{
    return write(str.c_str(), static_cast<unsigned int>(str.length()));
}

bool GZipFile::eof() const
{
    return gzeof(mGzFile) != 0;
}

const char *GZipFile::error() const
{
    return gzerror(mGzFile, nullptr);
}

int GZipFile::progress() const
{
    if (mFileSize > 0) {
        return static_cast<double>(gzoffset(mGzFile)) / mFileSize * 100;
    }
    return -1;
}
