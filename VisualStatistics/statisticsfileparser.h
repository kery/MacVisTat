#ifndef STATISTICSFILEPARSER_H
#define STATISTICSFILEPARSER_H

#include "statistics.h"

class ProgressDialog;

class StatisticsFileParser
{
public:
    typedef QMap<int, QString> IndexNameMap;

    struct Result {
        QVector<QString> failedFiles;
        Statistics::NodeNameDataMap nndm;
    };

    StatisticsFileParser(ProgressDialog &dialog);
    StatisticsFileParser(const StatisticsFileParser &) = delete;
    StatisticsFileParser& operator=(const StatisticsFileParser &) = delete;

    bool parseFileData(const IndexNameMap &inm,
                       const QVector<QString> &fileNames,
                       Result &result);
    std::string parseFileHeader(QStringList &filePaths, QStringList &failInfo);
    void checkFileHeader(QStringList &filePaths, QStringList &failInfo);
    void parseTimeDuration(const QStringList &filePaths, QObject *receiver, const char *slot);

    static QString getNodeFromFileName(const QString &fileName);

private:
    ProgressDialog &m_dialog;
};

#endif // STATISTICSFILEPARSER_H
