#include "PlotData.h"
#include "GlobalDefines.h"

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

int PlotData::size() const
{
    return mDataMap.size();
}

int PlotData::dataCount() const
{
    if (mDataMap.isEmpty()) {
        return 0;
    }
    return mDataMap.first().data.size();
}

QDateTime PlotData::dateTimeFromKey(double key) const
{
    QDateTime result;
    if (mKeyType == ktDateTime) {
        result = QDateTime::fromSecsSinceEpoch(key);
    } else {
        int index = static_cast<int>(key);
        if (index >= 0 && index < mDateTimeVector.size()) {
            result = QDateTime::fromSecsSinceEpoch(mDateTimeVector[index]);
        }
    }
    return result;
}

QDateTime PlotData::dateTimeFromIndex(int index) const
{
    QDateTime result;
    if (mKeyType == ktDateTime) {
        const QCPGraphDataContainer &data = mDataMap.first().data;
        if (index >= 0 && index < data.size()) {
            result = QDateTime::fromSecsSinceEpoch(data.at(index)->key);
        }
    } else {
        if (index >= 0 && index < mDateTimeVector.size()) {
            result = QDateTime::fromSecsSinceEpoch(mDateTimeVector[index]);
        }
    }
    return result;
}

double PlotData::getSampleInterval() const
{
    double interval = std::numeric_limits<double>::max();
    if (mKeyType == ktIndex) {
        for (int i = 1; i < mDateTimeVector.size(); ++i) {
            double diff = mDateTimeVector[i] - mDateTimeVector[i - 1];
            if (diff < interval) {
                interval = diff;
            }
        }
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

QString PlotData::firstCounterName() const
{
    if (mDataMap.isEmpty()) {
        return QString();
    }
    return mDataMap.firstKey();
}

bool PlotData::contains(const QString &name) const
{
    return mDataMap.contains(name);
}

QSharedPointer<QCPGraphDataContainer> PlotData::firstCounterData()
{
    if (mDataMap.isEmpty()) {
        return QSharedPointer<QCPGraphDataContainer>();
    }
    return QSharedPointer<QCPGraphDataContainer>(&mDataMap.first().data, CounterData::dummyDeleter);
}

QSharedPointer<QCPGraphDataContainer> PlotData::counterData(const QString &name, bool delta)
{
    if (!mDataMap.contains(name)) { return QSharedPointer<QCPGraphDataContainer>(); }

    if (delta) {
        const QCPGraphDataContainer &originalData = mDataMap[name].data;
        auto iterBegin = originalData.constBegin(), iterEnd = originalData.constEnd();
        QSharedPointer<QCPGraphDataContainer> deltaData(new QCPGraphDataContainer());

        if (iterBegin != iterEnd) {
            deltaData->add(QCPGraphData(iterBegin->key, NAN));
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

QSharedPointer<QCPGraphDataContainer> PlotData::addCounterData(const QString &name)
{
    if (mDataMap.contains(name)) {
        return QSharedPointer<QCPGraphDataContainer>();
    }
    return QSharedPointer<QCPGraphDataContainer>(&mDataMap[name].data, CounterData::dummyDeleter);
}

QSet<double> * PlotData::suspectKeys(const QString &name)
{
    if (mDataMap.contains(name)) {
        return &mDataMap[name].suspectKeys;
    }
    return nullptr;
}

void PlotData::removeCounterData(const QString &name)
{
    auto iter = mDataMap.find(name);
    if (iter != mDataMap.end()) {
        mDataMap.erase(iter);
    }
}

std::unique_ptr<PlotData[]> PlotData::split()
{
    int i = 0;
    std::unique_ptr<PlotData[]> result(new PlotData[mDataMap.size()]);
    for (auto iter = mDataMap.begin(); iter != mDataMap.end(); ++iter, ++i) {
        result[i].mKeyType = mKeyType;
        result[i].mOffsetFromUtc = mOffsetFromUtc;
        result[i].mDateTimeVector = mDateTimeVector;
        result[i].mDataMap[iter.key()] = std::move(iter.value());
    }
    return result;
}

PlotData::PlotData()
{
}
