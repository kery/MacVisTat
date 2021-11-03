#include "Utils.h"
#include <cstring>

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

void adjustYAxisRange(QCPAxis *yAxis)
{
    QCPRange range = yAxis->range();
    double delta = range.size() * 0.02;
    range.lower -= delta;
    range.upper += delta;
    yAxis->setRange(range);
}

QString getAppDataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString getUserName()
{
#if defined(Q_OS_WIN)
    return qgetenv("USERNAME");
#else
    return qgetenv("USER");
#endif
}

bool isValieOffsetFromUtc(int offset)
{
    // https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
    // >= -12:00 <= +14:00
    return offset >= -(12 * 3600) && offset <= (14 * 3600);
}

void showInfoMsgBox(QWidget *parent, const QString &text, const QString &info)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QStringLiteral("Visual Statistics"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);
    if (!info.isEmpty()) {
        msgBox.setInformativeText(info);
    }
    msgBox.exec();
}

void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QStringLiteral("Visual Statistics"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    if (!info.isEmpty()) {
        msgBox.setInformativeText(info);
    }
    msgBox.exec();
}

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info, bool defaultYes)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QStringLiteral("Visual Statistics"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (!info.isEmpty()) {
        msgBox.setInformativeText(info);
    }
    msgBox.setDefaultButton(defaultYes ? QMessageBox::Yes : QMessageBox::No);
    return msgBox.exec();
}
