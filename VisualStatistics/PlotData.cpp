#include "PlotData.h"

PlotData::PlotData(int offsetFromUtc) :
    mOffsetFromUtc(offsetFromUtc)
{
}

int PlotData::offsetFromUtc() const
{
    return mOffsetFromUtc;
}

CounterDataMap &PlotData::counterDataMap()
{
    return mDataMap;
}

QList<QString> PlotData::counterNames() const
{
    return mDataMap.keys();
}

QSharedPointer<QCPGraphDataContainer> PlotData::graphData(const QString &name)
{
    if (mDataMap.contains(name)) {
        return mDataMap[name].data;
    }
    return QSharedPointer<QCPGraphDataContainer>();
}

QSharedPointer<QCPGraphDataContainer> PlotData::graphDeltaData(const QString &name)
{
    if (mDataMap.contains(name)) {
        auto originalData = mDataMap[name].data;
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
    if (mDataMap.contains(name)) {
        return &mDataMap[name].suspectKeys;
    }
    return nullptr;
}

void PlotData::removeGraphData(const QString &name)
{
    auto iter = mDataMap.find(name);
    if (iter != mDataMap.end()) {
        mDataMap.erase(iter);
    }
}
