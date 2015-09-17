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

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QStringLiteral("Question"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (!info.isEmpty()) {
        msgBox.setInformativeText(info);
    }
    return msgBox.exec();
}
