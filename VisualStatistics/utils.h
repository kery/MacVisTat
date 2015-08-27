#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <QDataStream>
#include "version.h"
#include "third_party/qcustomplot/qcustomplot.h"

using namespace std;

const qint32 version = VER_FILEVERSION_NUM;

QString getVersionStr();

void splitString(const char *str, char ch, vector<string> &out);

const quint32 plotFileMagicNum = 0x40992872;
QDataStream& operator<< (QDataStream &out, const QCPData &data);
QDataStream& operator>> (QDataStream &in, QCPData &data);

QString getAppDataDir();
QString getDocumentDir();

#endif // UTILS_H
