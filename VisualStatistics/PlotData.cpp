#include "PlotData.h"
#include "Utils.h"

PlotData::PlotData(int offsetFromUtc) :
    mKeyType(ktUnknown),
    mOffsetFromUtc(offsetFromUtc)
{
}

PlotData::KeyType PlotData::keyType() const
{
    return mKeyType;
}

int PlotData::offsetFromUtc() const
{
    return mOffsetFromUtc;
}

QVector<double> PlotData::dateTimeVector() const
{
    return mDateTimeVector;
}

QString PlotData::dateTimeString(double key)
{
    QDateTime dateTime;
    if (mKeyType == ktDateTime) {
        dateTime = QDateTime::fromSecsSinceEpoch(key);
    } else {
        int index = static_cast<int>(key);
        if (index >= 0 && index < mDateTimeVector.size()) {
            dateTime = QDateTime::fromSecsSinceEpoch(mDateTimeVector[index]);
        }
    }
    return dateTime.toString(DTFMT_DISPLAY);
}

double PlotData::getSampleInterval() const
{
    double interval = std::numeric_limits<double>::max();
    switch (mKeyType) {
    case ktDateTime:
        // TODO
        Q_ASSERT(false);
        break;
    case ktIndex:
        for (int i = 1; i < mDateTimeVector.size(); ++i) {
            double diff = mDateTimeVector[i] - mDateTimeVector[i - 1];
            if (diff < interval) {
                interval = diff;
            }
        }
        break;
    default:
        break;
    }
    return interval;
}

void PlotData::setCounterDataMap(KeyType keyType, CounterDataMap &dataMap)
{
    if (keyType == ktIndex) {
        const CounterData &cdata = dataMap.first();
        mDateTimeVector.resize(cdata.data.size());
        for (int i = 0; i < mDateTimeVector.size(); ++i) {
            mDateTimeVector[i] = cdata.data.at(i)->key;
        }
        for (CounterData &cdata : dataMap) {
            QSet<double> sptKeys;
            for (auto begin = cdata.data.begin(), end = cdata.data.end(), iter = begin; iter != end; ++iter) {
                int index = iter - begin;
                if (cdata.suspectKeys.contains(iter->key)) {
                    sptKeys.insert(index);
                }
                iter->key = index;
            }
            cdata.suspectKeys.swap(sptKeys);
        }
    }
    mKeyType = keyType;
    mDataMap.swap(dataMap);
}

QList<QString> PlotData::counterNames() const
{
    return mDataMap.keys();
}

QSharedPointer<QCPGraphDataContainer> PlotData::graphData(const QString &name, bool delta)
{
    if (!mDataMap.contains(name)) {
        return QSharedPointer<QCPGraphDataContainer>();
    }

    if (delta) {
        const QCPGraphDataContainer &originalData = mDataMap[name].data;
        auto iterBegin = originalData.constBegin(), iterEnd = originalData.constEnd();
        QSharedPointer<QCPGraphDataContainer> deltaData(new QCPGraphDataContainer());

        if (iterBegin != iterEnd) {
            deltaData->add(QCPGraphData(iterBegin->key, 0));
        }
        for (auto iter = iterBegin + 1; iter != iterEnd; ++iter) {
            deltaData->add(QCPGraphData(iter->key, iter->value - (iter - 1)->value));
        }
        return deltaData;
    }

    // Use an empty deleter for normal QCPGraphDataContainer since it is allocated by QMap. Only
    // the delta version needs to be deleted.
    return QSharedPointer<QCPGraphDataContainer>(&mDataMap[name].data, CounterData::dummyDeleter);
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
