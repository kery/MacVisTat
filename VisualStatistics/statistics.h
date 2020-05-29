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
    QString getNodesString() const;
    QList<QString> getNames(const QString &node) const;
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

    QMap<int, qint32> getIndexDateTimeMap(const QString &node) const;

    static QMap<QString, NodeNameDataMap>
        groupNodeNameDataMapByName(NodeNameDataMap &&nndm);

    static QString getModuleFromStatName(const std::string &statName);
    static QString removeModuleFromStatName(const QString &statName);

    int getSampleInterval() const;

private:
    void initDateTimes();
    void updateDataKeys();

    void updateFirstDataKeys(NameDataMap &ndm);
    void updateOtherDataKeys(NameDataMap &ndm);
    void updateDataKeys(const QCPDataMap &src, QCPDataMap &dest);

private:
    NodeNameDataMap m_nndm;
    DateTimeVector m_dateTimes;
};

#endif // STATISTICS_H
