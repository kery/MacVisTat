#include "PlotData.h"

PlotData::PlotData(int offsetFromUtc) :
    _offsetFromUtc(offsetFromUtc)
{
}

int PlotData::offsetFromUtc() const
{
    return _offsetFromUtc;
}

CounterDataMap &PlotData::counterDataMap()
{
    return _dataMap;
}

QList<QString> PlotData::counterNames() const
{
    return _dataMap.keys();
}

QSharedPointer<QCPGraphDataContainer> PlotData::graphData(const QString &name)
{
    if (_dataMap.contains(name)) {
        return _dataMap[name].data;
    }
    return QSharedPointer<QCPGraphDataContainer>();
}

const QSet<double> *PlotData::suspectKeys(const QString &name)
{
    if (_dataMap.contains(name)) {
        return &_dataMap[name].suspectKeys;
    }
    return nullptr;
}
