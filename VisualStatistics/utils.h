#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include "third_party/qcustomplot/qcustomplot.h"

#define DT_FORMAT QStringLiteral("yyyy-MM-dd HH:mm:ss")
#define DT_FORMAT_IN_FILE QStringLiteral("dd.MM.yyyy;HH:mm:ss")
#define DT_FORMAT_IN_FILE_NAME QStringLiteral("yyyyMMdd.HHmm")

using namespace std;

int versionStringToNumber(const QString &version);

void splitString(const char *str, char ch, vector<string> &out);

void adjustYAxisRange(QCPAxis *yAxis);

QString getAppDataDir();

void showInfoMsgBox(QWidget *parent, const QString &text, const QString &info = QString());
void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info = QString());
int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info = QString(), bool defaultYes=true);

#endif // UTILS_H
