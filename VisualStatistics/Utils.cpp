#include "Utils.h"
#include "GlobalDefines.h"

double pointDistance(const QPointF &pt1, const QPointF &pt2)
{
    return std::sqrt(std::pow(pt1.x() - pt2.x(), 2) + std::pow(pt1.y() - pt2.y(), 2));
}

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info,
                       QMessageBox::StandardButtons buttons,
                       QMessageBox::StandardButton def)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(APP_NAME);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setInformativeText(info);
    msgBox.setText(text);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(def);
    return msgBox.exec();
}

void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(APP_NAME);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setInformativeText(info);
    msgBox.setText(text);
    msgBox.exec();
}
