#ifndef STATISTICS_H
#define STATISTICS_H

#include <qcustomplot.h>

class Statistics
{
public:
    typedef QMap<QString, QCPDataMap> NameDataMap;

    Statistics(NameDataMap &ndm, int offsetFromUtc);

    QList<QString> getNames() const;
    int nameCount() const;
    QCPDataMap* getDataMap(const QString &name);
    QCPDataMap* addDataMap(const QString &name);
    bool removeDataMap(const QString &name);

    int offsetFromUtc() const;
    bool utcMode() const;
    bool setUtcMode(bool utcMode);
    int dateTimeCount() const;
    uint getDateTime(int index) const;
    QString getDateTimeString(int index) const;
    uint getFirstDateTime() const;
    uint getLastDateTime() const;
    int firstIndexAfterTime_t(uint time) const;

    static QVector<NameDataMap> divideNameDataMap(NameDataMap &ndm);
    static bool isConstantDataMap(const QCPDataMap &dm);
    static QString getModuleFromStatName(const std::string &statName);
    static QString splitStatNameToModuleAndName(const QString &statName, QString &name);

private:
    void initDateTimes();
    void translateKeys();

private:
    int m_offsetFromUtc;
    bool m_utcMode;
    NameDataMap m_ndm;
    QVector<uint> m_dateTimes;
};

#endif // STATISTICS_H
