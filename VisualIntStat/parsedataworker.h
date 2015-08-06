#ifndef PARSEDATAWORKER_H
#define PARSEDATAWORKER_H

#include "third_party/qcustomplot/qcustomplot.h"
#include "gzipfile.h"
#include <QObject>

struct ParseDataParam
{
    bool multipleWindows;
    QMap<QString, int> fileInfo; // Key is file name, value is line count
    QVector<QString> statNames;
};

struct ParsedResult
{
    QVector<uint> dateTimes;
    QMap<QString, QCPDataMap*> data; // Key is statistics name
};

class ParseDataWorker : public QObject
{
    Q_OBJECT
public:
    ParseDataWorker(QObject *parent = NULL);
    ~ParseDataWorker();

public slots:
    void parseData(const ParseDataParam &param);

private:
    int parseFile(const QString &file, bool lineCountKnown, int &progress, int &keyVal, ParsedResult &result);
    bool calcStatNamesIndex(GZipFile &gzFile, QMap<QString, int> &indexes);

signals:
    void progressRangeWorkedOut(int min, int max);
    void progressValueUpdated(int value);
    void progressLabelUpdated(const QString &text);
    void dataReady(const ParsedResult &result, bool multipleWindows);

public:
    volatile bool _canceled;
};

#endif // PARSEDATAWORKER_H
