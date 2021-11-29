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

QSharedPointer<QCPGraphDataContainer> PlotData::graphDeltaData(const QString &name)
{
    if (_dataMap.contains(name)) {
        auto originalData = _dataMap[name].data;
        auto iterBegin = originalData->begin(), iterEnd = originalData->end();
        QSharedPointer<QCPGraphDataContainer> deltaData(new QCPGraphDataContainer());

        if (iterBegin != iterEnd) {
            deltaData->add(QCPGraphData(iterBegin->key, 0));
        }
        for (auto iter = iterBegin + 1; iter != iterEnd; ++iter) {
            deltaData->add(QCPGraphData(iter->key, iter->value - (iter - 1)->value));
        }
        return deltaData;
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

void PlotData::removeGraphData(const QString &name)
{
    auto iter = _dataMap.find(name);
    if (iter != _dataMap.end()) {
        _dataMap.erase(iter);
    }
}
