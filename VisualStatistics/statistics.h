#ifndef STATISTICS_H
#define STATISTICS_H

#include <qcustomplot.h>

class Statistics
{
public:
    typedef QMap<QString, QCPDataMap> NameDataMap;
    typedef QMap<QString, NameDataMap> NodeNameDataMap;
    typedef QVector<qint32> DateTimeVector;

    Statistics(NodeNameDataMap &nndm);

    QList<QString> getNodes() const;
    int getNodeCount() const;
    QList<QString> getNames(const QString &node) const;
    // TODO: need to remove?
    QList<double> getDataKeys(const QString &node) const;
    int totalNameCount() const;
    QCPDataMap* getDataMap(const QString &node, const QString &name);
    QCPDataMap* addDataMap(const QString &node, const QString &name);
    bool removeDataMap(const QString &node, const QString &name);
    void removeEmptyNode();

    int dateTimeCount() const;
    DateTimeVector::value_type getDateTime(int index) const;
    QString getDateTimeString(int index) const;
    int getFirstDateTime() const;
    int getLastDateTime() const;
    int firstGreaterDateTimeIndex(int dateTime) const;

    // TODO: need to remove?
    QMap<int, qint32> getIndexDateTimeMap(const QString &node) const;

    static QMap<QString, NodeNameDataMap>
        groupNodeNameDataMapByName(NodeNameDataMap &&nndm);

    static QString getModuleFromStatName(const std::string &statName);
    static QString splitStatNameToModuleAndName(const QString &statName, QString &name);

    // TODO: need change?
    int getSampleInterval() const;

private:
    void initDateTimes();
    void updateDataKeys();

private:
    NodeNameDataMap m_nndm;
    DateTimeVector m_dateTimes;
};

#endif // STATISTICS_H
