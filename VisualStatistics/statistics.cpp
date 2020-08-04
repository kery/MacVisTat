#if defined(_MSC_VER)
// Disable warning C4244: possible loss of data
#pragma warning(disable: 4244)
#endif

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

QString Statistics::getModuleFromStatName(const std::string &statName)
{
    auto pos = statName.find(',');
    if (pos > 0) {
        return QString(statName.substr(0, pos).c_str());
    }
    return QString();
}

QString Statistics::splitStatNameToModuleAndName(const QString &statName, QString &name)
{
    int pos = statName.indexOf(',');
    if (pos > 0) {
        name = statName.mid(pos + 1);
        return statName.left(pos);
    }
    name = statName;
    return QString();
}

int Statistics::getSampleInterval() const
{
    std::vector<int> timeDiff;

    const QCPDataMap &dataMap = m_nndm.first().first();
    for (const QCPData &data : dataMap) {
        timeDiff.push_back(getDateTime(data.key));
    }

    int interval = 60;
    if (timeDiff.size() > 1) {
        for (auto iter = timeDiff.begin(); iter != timeDiff.end() - 1; ++iter) {
            *iter = *(iter + 1) - *iter;
        }

        timeDiff.pop_back();

        size_t nth = timeDiff.size() / 2;
        std::nth_element(timeDiff.begin(), timeDiff.begin() + nth, timeDiff.end());
        interval = timeDiff[nth];
    }

    return interval;
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

int Statistics::getNodeCount() const
{
    return m_nndm.size();
}

QList<QString> Statistics::getNames(const QString &node) const
{
    return m_nndm.value(node).keys();
}

QList<double> Statistics::getDataKeys(const QString &node) const
{
    if (m_nndm.contains(node)) {
        const QCPDataMap &dm = m_nndm[node].first();
        if (!dm.empty()) {
            return dm.keys();
        }
    }
    return QList<double>();
}

QList<double> Statistics::getDataKeys() const
{
    if (!m_nndm.isEmpty()) {
        const QCPDataMap &dm = m_nndm.first().first();
        if (!dm.empty()) {
            return dm.keys();
        }
    }
    return QList<double>();
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

QCPDataMap* Statistics::addDataMap(const QString &node, const QString &name)
{
    if (!m_nndm.contains(node)) {
        return nullptr;
    }

    NameDataMap &ndm = m_nndm[node];
    if (ndm.contains(name)) {
        return nullptr;
    }

    return &ndm[name];
}

bool Statistics::removeDataMap(const QString &node, const QString &name)
{
    if (m_nndm.contains(node)) {
        NameDataMap &ndm = m_nndm[node];
        auto pos = ndm.find(name);
        if (pos != ndm.end()) {
            ndm.erase(pos);
            return true;
        }
    }
    return false;
}

void Statistics::removeEmptyNode()
{
    for (auto iter = m_nndm.begin(); iter != m_nndm.end();) {
        if (iter->empty()) {
            iter = m_nndm.erase(iter);
        } else {
            ++iter;
        }
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
    for (const NameDataMap &ndm : m_nndm) {
        for (const QCPDataMap &dm : ndm) {
            QList<double> dateTimesOfName = dm.keys();
            DateTimeVector temp(m_dateTimes.size() + dateTimesOfName.size());
            auto iter = std::set_union(m_dateTimes.begin(), m_dateTimes.end(),
                                       dateTimesOfName.begin(), dateTimesOfName.end(),
                                       temp.begin());
            temp.resize(iter - temp.begin());
            m_dateTimes.swap(temp);
        }
    }
}

void Statistics::updateDataKeys()
{
    for (NameDataMap &ndm : m_nndm) {
        for (QCPDataMap &dataMap : ndm) {
            QCPDataMap tempDataMap;
            auto searchFrom = m_dateTimes.begin();
            for (const QCPData &data : dataMap) {
                QCPData tempData;
                auto iter = std::lower_bound(searchFrom, m_dateTimes.end(), data.key);
                tempData.key = iter - m_dateTimes.begin();
                tempData.value = data.value;
                tempData.valueErrorMinus = data.valueErrorMinus;
                tempDataMap.insert(tempData.key, tempData);
                searchFrom = iter;
            }
            dataMap.swap(tempDataMap);
        }
    }
}
