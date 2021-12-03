#ifndef PLOTDATA_H
#define PLOTDATA_H

#include "CounterGraph.h"

typedef QMap<QString, CounterData> CounterDataMap;

class PlotData
{
public:
    enum KeyType {
        ktUnknown,
        ktDateTime,
        ktIndex,
    };

    PlotData(int offsetFromUtc);
    PlotData(const PlotData &) = delete;
    PlotData(PlotData &&) = default;
    PlotData &operator=(const PlotData &) = delete;
    PlotData &operator=(PlotData &&) = default;

    KeyType keyType() const;
    int offsetFromUtc() const;
    QVector<double> dateTimeVector() const;
    double getSampleInterval() const;
    void setCounterDataMap(KeyType keyType, CounterDataMap &dataMap);
    QList<QString> counterNames() const;
    QSharedPointer<QCPGraphDataContainer> graphData(const QString &name, bool delta = false);
    const QSet<double> *suspectKeys(const QString &name);
    void removeGraphData(const QString &name);

private:
    KeyType mKeyType;
    int mOffsetFromUtc;
    QVector<double> mDateTimeVector;
    CounterDataMap mDataMap;
};

#endif // PLOTDATA_H
