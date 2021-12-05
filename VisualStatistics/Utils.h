#ifndef UTILS_H
#define UTILS_H

#include <QString>

class QPointF;
class QWidget;

double pointDistance(const QPointF &pt1, const QPointF &pt2);

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info = QString(), bool defaultYes = true);
void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info = QString());

#endif // UTILS_H
