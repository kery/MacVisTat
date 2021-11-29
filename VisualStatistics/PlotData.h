#ifndef PLOTDATA_H
#define PLOTDATA_H

#include "CounterGraph.h"

typedef QMap<QString, CounterData> CounterDataMap;

class PlotData
{
public:
    PlotData(int offsetFromUtc);
    PlotData(const PlotData &) = delete;
    PlotData(PlotData &&) = default;
    PlotData &operator=(const PlotData &) = delete;
    PlotData &operator=(PlotData &&) = default;

    int offsetFromUtc() const;
    CounterDataMap &counterDataMap();
    QList<QString> counterNames() const;
    QSharedPointer<QCPGraphDataContainer> graphData(const QString &name);
    QSharedPointer<QCPGraphDataContainer> graphDeltaData(const QString &name);
    const QSet<double> *suspectKeys(const QString &name);

private:
    int _offsetFromUtc;
    CounterDataMap _dataMap;
};

#endif // PLOTDATA_H
