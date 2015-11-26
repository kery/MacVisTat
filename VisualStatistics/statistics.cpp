// Disable warning C4244: possible loss of data
#pragma warning(disable: 4244)

#include "statistics.h"
#include "utils.h"

QMap<QString, Statistics::NodeNameDataMap>
Statistics::groupNodeNameDataMapByName(NodeNameDataMap &&nndm)
{
    QMap<QString, Statistics::NodeNameDataMap> grouped;
    for (auto nndmIter = nndm.begin(); nndmIter != nndm.end(); ++nndmIter) {
        const QString &node = nndmIter.key();
        NameDataMap &ndm = nndmIter.value();
        for (auto ndmIter = ndm.begin(); ndmIter != ndm.end(); ++ndmIter) {
            const QString &name = ndmIter.key();
            QCPDataMap &dm = ndmIter.value();
            grouped[name][node][name] = std::move(dm);
        }
    }
    return grouped;
}

Statistics::Statistics(NodeNameDataMap &nndm) :
    m_nndm(std::move(nndm))
{
    initDateTimes();
    updateDataKeys();
}

QList<QString> Statistics::getNodes() const
{
    return m_nndm.keys();
}

QString Statistics::getNodesString() const
{
    return QStringList(getNodes()).join(',');
}

QList<QString> Statistics::getNames(const QString &node) const
{
    return m_nndm.value(node).keys();
}

int Statistics::totalNameCount() const
{
    int count = 0;
    for (const NameDataMap &ndm : m_nndm) {
        count += ndm.size();
    }
    return count;
}

QCPDataMap* Statistics::getDataMap(const QString &node, const QString &name)
{
    if (m_nndm.contains(node)) {
        if (m_nndm[node].contains(name)) {
            return &(m_nndm[node][name]);
        }
    }
    return nullptr;
}

QCPDataMap* Statistics::getDataMap(const QString &formattedName)
{
    QString node, name;
    parseFormattedName(formattedName, node, name);
    return getDataMap(node, name);
}

bool Statistics::removeDataMap(const QString &node, const QString &name)
{
    if (m_nndm.contains(node)) {
        NameDataMap &ndm = m_nndm[node];
        auto pos = ndm.find(name);
        if (pos != ndm.end()) {
            ndm.erase(pos);
            if (ndm.empty()) {
                m_nndm.erase(m_nndm.find(node));
            }
            return true;
        }
    }
    return false;
}

bool Statistics::removeDataMap(const QString &formattedName)
{
    QString node, name;
    parseFormattedName(formattedName, node, name);
    return removeDataMap(node, name);
}

QString Statistics::formatName(const QString &node, const QString &name) const
{
    if (m_nndm.size() > 1) {
        return (QString(node) += ':') += name;
    }
    return name;
}

void Statistics::parseFormattedName(const QString &formattedName,
                                    QString &node, QString &name) const
{
    if (m_nndm.size() > 1) {
        Q_ASSERT(formattedName.indexOf(':') > -1);
        QStringList strList = formattedName.split(':');
        node = strList[0];
        name = strList[1];
    } else {
        Q_ASSERT(formattedName.indexOf(':') == -1);
        node = m_nndm.firstKey();
        name = formattedName;
    }
}

int Statistics::dateTimeCount() const
{
    return m_dateTimes.size();
}

Statistics::DateTimeVector::value_type Statistics::getDateTime(int index) const
{
    Q_ASSERT(index >= 0 && index < m_dateTimes.size());
    return m_dateTimes.at(index);
}

QString Statistics::getDateTimeString(int index) const
{
    return QDateTime::fromTime_t((uint)getDateTime(index)).toString(DT_FORMAT);
}

int Statistics::getFirstDateTime() const
{
    return m_dateTimes.first();
}

int Statistics::getLastDateTime() const
{
    return m_dateTimes.last();
}

int Statistics::firstGreaterDateTimeIndex(int dateTime) const
{
    auto iter = std::upper_bound(m_dateTimes.begin(), m_dateTimes.end(), dateTime);
    if (iter != m_dateTimes.end()) {
        return iter - m_dateTimes.begin();
    } else {
        return -1;
    }
}

QMap<int, qint32> Statistics::getIndexDateTimeMap(const QString &node) const
{
    QMap<int, qint32> map;
    if (m_nndm.contains(node)) {
        const QCPDataMap &dm = m_nndm[node].first();
        for (const QCPData &data : dm) {
            map.insert((int)data.key, getDateTime(data.key));
        }
    }
    return map;
}

void Statistics::initDateTimes()
{
    for (NameDataMap &ndm : m_nndm) {
        QList<double> dateTimesOfNode = ndm.first().keys();

        DateTimeVector temp(m_dateTimes.size() + dateTimesOfNode.size());
        auto iter = std::set_union(m_dateTimes.begin(), m_dateTimes.end(),
                                   dateTimesOfNode.begin(), dateTimesOfNode.end(),
                                   temp.begin());
        temp.resize(iter - temp.begin());
        m_dateTimes.swap(temp);
    }
}

void Statistics::updateDataKeys()
{
    for (NameDataMap &ndm : m_nndm) {
        updateFirstDataKeys(ndm);
        updateOtherDataKeys(ndm);
    }
}

void Statistics::updateFirstDataKeys(NameDataMap &ndm)
{
    QCPData tempData;
    QCPDataMap tempDataMap, &dataMap = ndm.first();
    auto searchFrom = m_dateTimes.begin();
    for (const QCPData &data : dataMap) {
        auto index = std::lower_bound(searchFrom, m_dateTimes.end(), data.key);
        Q_ASSERT(index != m_dateTimes.end());
        tempData.key = index - m_dateTimes.begin();
        tempData.value = data.value;
        tempDataMap.insert(tempData.key, tempData);
        searchFrom = index;
    }
    dataMap.swap(tempDataMap);
}

void Statistics::updateOtherDataKeys(NameDataMap &ndm)
{
    const QCPDataMap &first = ndm.first();
    for (auto iter = ndm.begin() + 1; iter != ndm.end(); ++iter) {
        QCPDataMap &item = iter.value();
        updateDataKeys(first, item);
    }
}

void Statistics::updateDataKeys(const QCPDataMap &src, QCPDataMap &dest)
{
    Q_ASSERT(src.size() == dest.size());

    QCPData tempData;
    QCPDataMap tempDataMap;
    auto iterSrc = src.begin();
    auto iterDest = dest.begin();
    for (; iterSrc != src.end(); ++iterSrc, ++iterDest) {
        tempData.key = iterSrc.key();
        tempData.value = iterDest.value().value;
        tempDataMap.insert(tempData.key, tempData);
    }
    dest.swap(tempDataMap);
}
