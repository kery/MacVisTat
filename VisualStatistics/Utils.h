#ifndef UTILS_H
#define UTILS_H

#include <QString>

class QWidget;

#define APP_NAME "Visual Statistics"

#define DTFMT_DISPLAY     "yyyy-MM-dd HH:mm:ss"
#define DTFMT_IN_CSV      "dd.MM.yyyy;HH:mm:ss"
#define DTFMT_IN_CSV_LEN  19
#define DTFMT_IN_FILENAME "yyyyMMdd.HHmm"

int showQuestionMsgBox(QWidget *parent, const QString &text, const QString &info = QString(), bool defaultYes = true);

#endif // UTILS_H
