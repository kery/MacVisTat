#ifndef STATISTICS_H
#define STATISTICS_H

#include <qcustomplot.h>

class Statistics
{
public:
    typedef QMap<QString, QCPDataMap> NameDataMap;
    typedef QVector<qint32> DateTimeVector;

    Statistics(NameDataMap &ndm);

    QList<QString> getNames() const;
    int nameCount() const;
    QCPDataMap* getDataMap(const QString &name);
    QCPDataMap* addDataMap(const QString &name);
    bool removeDataMap(const QString &name);

    int dateTimeCount() const;
    DateTimeVector::value_type getDateTime(int index) const;
    QString getDateTimeString(int index) const;
    int getFirstDateTime() const;
    int getLastDateTime() const;
    int firstGreaterDateTimeIndex(int dateTime) const;

    static QVector<NameDataMap> divideNameDataMap(NameDataMap &ndm);
    static bool isConstantDataMap(const QCPDataMap &dm);
    static QString getModuleFromStatName(const std::string &statName);
    static QString splitStatNameToModuleAndName(const QString &statName, QString &name);

private:
    void initDateTimes();
    void translateKeys();

private:
    NameDataMap m_ndm;
    DateTimeVector m_dateTimes;
};

#endif // STATISTICS_H
