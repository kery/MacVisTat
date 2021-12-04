#ifndef UTILS_H
#define UTILS_H

#include <QString>

class QPointF;
class QWidget;

#define APP_NAME "Visual Statistics"

#define DTFMT_DISPLAY     "yyyy-MM-dd HH:mm:ss"
#define DTFMT_IN_CSV      "dd.MM.yyyy;HH:mm:ss"
#define DTFMT_IN_CSV_LEN  19
#define DTFMT_IN_FILENAME "yyyyMMdd.HHmm"

double pointDistance(const QPointF &pt1, const QPointF &pt2);

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info = QString(), bool defaultYes = true);
void showErrorMsgBox(QWidget *parent, const QString &text, const QString &info = QString());

#endif // UTILS_H
