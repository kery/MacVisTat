#ifndef STATISTICSFILEPARSER_H
#define STATISTICSFILEPARSER_H

#include "statistics.h"

class ProgressDialog;

class StatisticsFileParser
{
public:
    typedef QMap<int, QString> IndexNameMap;

    StatisticsFileParser(ProgressDialog &dialog);
    StatisticsFileParser(const StatisticsFileParser &) = delete;
    StatisticsFileParser& operator=(const StatisticsFileParser &) = delete;

    bool parseFileData(const IndexNameMap &inm, const QVector<QString> &fileNames,
        Statistics::NodeNameDataMap &nndm, QVector<QString> &errors);
    std::string parseFileHeader(QStringList &filePaths, QStringList &failInfo);
    void checkFileHeader(QStringList &filePaths, QStringList &failInfo);

    QString kciKpiToCsvFormat(QStringList &filePaths, QString &error);

    static QString getNodeFromFileName(const QString &fileName);

private:
    ProgressDialog &m_dialog;
};

#endif // STATISTICSFILEPARSER_H
