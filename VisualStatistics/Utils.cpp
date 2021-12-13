#include "Utils.h"
#include "GlobalDefines.h"
#include <sstream>

std::string doubleToStdString(double value)
{
    if (qIsNaN(value)) { return std::string(); }

    std::ostringstream oss;
    oss << value;
    return oss.str();
}

double pointDistance(const QPointF &pt1, const QPointF &pt2)
{
    return std::sqrt(std::pow(pt1.x() - pt2.x(), 2) + std::pow(pt1.y() - pt2.y(), 2));
}

bool isValidOffsetFromUtc(int offset)
{
    // https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
    // >= -12:00 <= +14:00
    return offset >= -(12 * 3600) && offset <= (14 * 3600);
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
