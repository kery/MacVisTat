#include "utils.h"
#include <cstring>

int versionStringToNumber(const QString &version)
{
    QString temp(version);
    temp.remove('.');
    return temp.toInt();
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

void showInfoMsgBox(QWidget *parent, const QString &text, const QString &info)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QStringLiteral("Information"));
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
    msgBox.setWindowTitle(QStringLiteral("Error"));
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
    msgBox.setWindowTitle(QStringLiteral("Question"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (!info.isEmpty()) {
        msgBox.setInformativeText(info);
    }
    msgBox.setDefaultButton(defaultYes ? QMessageBox::Yes : QMessageBox::No);
    return msgBox.exec();
}
