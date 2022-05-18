#include "GzipFile.h"
#include <QFileInfo>

GzipFile::GzipFile() :
    mGzFile(nullptr),
    mFileSize(-1)
{
}

GzipFile::~GzipFile()
{
    close();
}

bool GzipFile::open(const QString &path, OpenMode mode)
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

void GzipFile::close()
{
    if (mGzFile) {
        gzclose(mGzFile);
        mGzFile = nullptr;
        mFileSize = -1;
    }
}

int GzipFile::read(char *buf, unsigned int maxlen)
{
    return gzread(mGzFile, buf, maxlen);
}

bool GzipFile::readLine(std::string &line)
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

int GzipFile::write(const char *buf, unsigned int len)
{
    return gzwrite(mGzFile, buf, len);
}

int GzipFile::write(const std::string &str)
{
    return write(str.c_str(), static_cast<unsigned int>(str.length()));
}

bool GzipFile::eof() const
{
    return gzeof(mGzFile) != 0;
}

const char * GzipFile::error() const
{
    return gzerror(mGzFile, nullptr);
}

int GzipFile::progress() const
{
    if (mFileSize > 0) {
        return static_cast<double>(gzoffset(mGzFile)) / mFileSize * 100;
    }
    return -1;
}
