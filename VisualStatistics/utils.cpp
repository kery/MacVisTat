#include "utils.h"
#include <cstring>

QString getVersionStr()
{
    QStringList list;
    int ver = version;
    while (ver >= 10) {
        int remainder = ver % 10;
        ver = ver / 10;
        list.prepend(QString::number(remainder));
    }
    list.prepend(QString::number(ver));
    return list.join('.');
}

void splitString(const char *str, char ch, vector<string> &out)
{
    const char *ptr;
    while ((ptr = strchr(str, ch)) != NULL) {
        out.push_back(string(str, ptr - str));
        str = ptr + 1;
    }
    if (*str) {
        out.push_back(string(str));
    }
}

QDataStream& operator<< (QDataStream &out, const QCPData &data)
{
    out << data.key << data.value;
    return out;
}

QDataStream& operator>> (QDataStream &in, QCPData &data)
{
    in >> data.key >> data.value;
    return in;
}

QString getAppDataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString getDocumentDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}
