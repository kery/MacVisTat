#ifndef UTILS_H
#define UTILS_H

#include <QMessageBox>

std::string doubleToStdString(double value);

double pointDistance(const QPointF &pt1, const QPointF &pt2);

bool isValidOffsetFromUtc(int offset);

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info = QString(),
                       QMessageBox::StandardButtons buttons=QMessageBox::Yes|QMessageBox::No,
                       QMessageBox::StandardButton def=QMessageBox::NoButton);
void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info = QString());

#endif // UTILS_H
