#include "Utils.h"
#include <QMessageBox>

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info, bool defaultYes)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(APP_NAME);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setInformativeText(info);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(defaultYes ? QMessageBox::Yes : QMessageBox::No);
    return msgBox.exec();
}
