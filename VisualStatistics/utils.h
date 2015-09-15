#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <QDataStream>
#include "third_party/qcustomplot/qcustomplot.h"

using namespace std;

int versionStringToNumber(const QString &version);

void splitString(const char *str, char ch, vector<string> &out);

const quint32 plotFileMagicNum = 0x40992872;
QDataStream& operator<< (QDataStream &out, const QCPData &data);
QDataStream& operator>> (QDataStream &in, QCPData &data);

QString getAppDataDir();
QString getDocumentDir();

#endif // UTILS_H
