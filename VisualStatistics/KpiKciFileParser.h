#ifndef KPIKCIFILEPARSER_H
#define KPIKCIFILEPARSER_H

#include <QString>

class QWidget;
class ProgressDialog;

class KpiKciFileParser
{
public:
    KpiKciFileParser(QWidget *parent);

    QString convertToCsv(QStringList &paths, QVector<QString> &errors);

private:
    static void sortFiles(QStringList &paths, QVector<QString> &failedFiles, QVector<QString> &errors);

    QWidget *mParent;
};

#endif // KPIKCIFILEPARSER_H
