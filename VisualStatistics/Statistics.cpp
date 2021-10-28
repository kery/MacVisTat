#if defined(_MSC_VER)
// Disable warning C4244: possible loss of data
#pragma warning(disable: 4244)
#endif

#include "Statistics.h"
#include "Utils.h"

QVector<Statistics::NameDataMap> Statistics::divideNameDataMap(NameDataMap &ndm)
{
    QVector<Statistics::NameDataMap> result;
    for (auto iter = ndm.begin(); iter != ndm.end(); ++iter) {
        Statistics::NameDataMap tempNdm;
        QCPDataMap &dm = tempNdm[iter.key()];
        dm.swap(iter.value());
        result.append(tempNdm);
    }
    return result;
}

bool Statistics::isConstantDataMap(const QCPDataMap &dm)
{
    if (dm.isEmpty()) {
        return true;
    }

    // compare with 2 decimal places precision
    qint64 firstValue = qint64(dm.first().value * 100);
    for (auto iter = dm.begin() + 1; iter != dm.end(); ++iter) {
        if (qint64(iter.value().value * 100) != firstValue) {
            return false;
        }
    }
    return true;
}

QString Statistics::getModuleFromStatName(const std::string &statName)
{
    std::string::size_type pos = statName.find(',');
    if (pos != std::string::npos && pos != 0) {
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

Statistics::Statistics(NameDataMap &ndm, int offsetFromUtc) :
    m_offsetFromUtc(offsetFromUtc),
    m_utcMode(false),
    m_ndm(std::move(ndm))
{
    initDateTimes();
    translateKeys();
}

QList<QString> Statistics::getNames() const
{
    return m_ndm.keys();
}

int Statistics::nameCount() const
{
    return m_ndm.size();
}

QCPDataMap* Statistics::getDataMap(const QString &name)
{
    if (m_ndm.contains(name)) {
        return &(m_ndm[name]);
    }
    return nullptr;
}

QCPDataMap* Statistics::addDataMap(const QString &name)
{
    if (m_ndm.contains(name)) {
        return nullptr;
    }

    return &m_ndm[name];
}

bool Statistics::removeDataMap(const QString &name)
{
    auto iter = m_ndm.find(name);
    if (iter != m_ndm.end()) {
        m_ndm.erase(iter);
        return true;
    }
    return false;
}

uint Statistics::getSampleInterval() const
{
    uint interval = std::numeric_limits<uint>::max();
    for (int i = 1; i < m_dateTimes.size(); ++i) {
        uint diff = m_dateTimes[i] - m_dateTimes[i - 1];
        if (diff < interval) {
            interval = diff;
        }
    }
    return interval;
}

int Statistics::offsetFromUtc() const
{
    return m_offsetFromUtc;
}

bool Statistics::utcMode() const
{
    return m_utcMode;
}

bool Statistics::setUtcMode(bool utcMode)
{
    if (isValieOffsetFromUtc(m_offsetFromUtc)) {
        m_utcMode = utcMode;
        return true;
    }
    return false;
}

int Statistics::dateTimeCount() const
{
    return m_dateTimes.size();
}

uint Statistics::getDateTime(int index) const
{
    if (index >= 0 && index < m_dateTimes.size()) {
        return m_dateTimes.at(index);
    }
    return 0;
}

QString Statistics::getDateTimeString(int index) const
{
    QDateTime dt = QDateTime::fromTime_t(getDateTime(index));
    if (m_utcMode) {
        dt.setOffsetFromUtc(m_offsetFromUtc);
        dt = dt.toUTC();
    }
    return dt.toString(DT_FORMAT);
}

uint Statistics::getFirstDateTime() const
{
    return m_dateTimes.first();
}

uint Statistics::getLastDateTime() const
{
    return m_dateTimes.last();
}

int Statistics::firstIndexAfterTime_t(uint time) const
{
    auto iter = std::upper_bound(m_dateTimes.begin(), m_dateTimes.end(), time);
    if (iter != m_dateTimes.end()) {
        return iter - m_dateTimes.begin();
    } else {
        return -1;
    }
}

void Statistics::initDateTimes()
{
    for (const QCPDataMap &dm : m_ndm) {
        QList<double> dateTimes = dm.keys();
        QVector<uint> temp(m_dateTimes.size() + dateTimes.size());
        auto iter = std::set_union(m_dateTimes.begin(), m_dateTimes.end(),
                                   dateTimes.begin(), dateTimes.end(),
                                   temp.begin());
        temp.resize(iter - temp.begin());
        m_dateTimes.swap(temp);
    }
}

void Statistics::translateKeys()
{
    for (QCPDataMap &dataMap : m_ndm) {
        QCPDataMap tempDataMap;
        QVector<uint>::difference_type curKey, lastKey = -1;
        auto searchFrom = m_dateTimes.begin();
        for (const QCPData &data : dataMap) {
            QCPData tempData;
            auto iter = std::lower_bound(searchFrom, m_dateTimes.end(), data.key);
            curKey = iter - m_dateTimes.begin();
            tempData.key = curKey;
            tempData.value = data.value;
            tempData.valueErrorMinus = data.valueErrorMinus;
            if (lastKey != -1 && curKey - lastKey > 1) {
                tempData.valueErrorPlus = 1.0;
            }
            lastKey = curKey;
            tempDataMap.insert(tempData.key, tempData);
            searchFrom = iter;
        }
        dataMap.swap(tempDataMap);
    }
}
