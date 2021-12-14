#ifndef COUNTERFILEPARSER_H
#define COUNTERFILEPARSER_H

#include "PlotData.h"

class QWidget;
class ProgressDialog;

class CounterFileParser
{
public:
    typedef QMap<int, QString> IndexNameMap;

    CounterFileParser(QWidget *parent);
    int offsetFromUtc() const;

    QString parseHeader(const QString &path, QVector<QString> &names);
    QString parseData(const QString &path, const IndexNameMap &inm, CounterDataMap &cdm);

private:
    static std::string parseHeaderInternal(const QString &path, int &offsetFromUtc, QString &error);
    static QString parseDataInternal(const QString &path, const IndexNameMap &inm,
                                     const volatile bool &working, ProgressDialog &dlg, CounterDataMap &cmd);
    static void splitHeader(const char *str, QVector<QString> &out);

    QWidget *mParent;
    int mOffsetFromUtc;
};

#endif // COUNTERFILEPARSER_H