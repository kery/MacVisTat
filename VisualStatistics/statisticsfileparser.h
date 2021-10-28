#ifndef STATISTICSFILEPARSER_H
#define STATISTICSFILEPARSER_H

#include "Statistics.h"

class ProgressDialog;

class StatisticsFileParser
{
public:
    typedef QMap<int, QString> IndexNameMap;

    StatisticsFileParser(ProgressDialog &dialog);
    StatisticsFileParser(const StatisticsFileParser &) = delete;
    StatisticsFileParser& operator=(const StatisticsFileParser &) = delete;

    std::string parseFileHeader(const QString &path, int &offsetFromUtc, QString &error);

    bool parseFileData(const IndexNameMap &inm, const QString &path,
        Statistics::NameDataMap &ndm, QString &error);

    QString kpiKciToCsvFormat(QStringList &filePaths, QString &error);

private:
    ProgressDialog &m_dialog;
};

#endif // STATISTICSFILEPARSER_H
