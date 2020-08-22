#if defined(_MSC_VER)
// Disable warning C4244: possible loss of data
#pragma warning(disable: 4244)
#endif

#include "statistics.h"
#include "utils.h"

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

Statistics::Statistics(NameDataMap &ndm) :
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
    auto pos = m_ndm.find(name);
    if (pos != m_ndm.end()) {
        m_ndm.erase(pos);
        return true;
    }
    return false;
}

int Statistics::dateTimeCount() const
{
    return m_dateTimes.size();
}

Statistics::DateTimeVector::value_type Statistics::getDateTime(int index) const
{
    if (index >= 0 && index < m_dateTimes.size()) {
        return m_dateTimes.at(index);
    }
    return 0;
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

void Statistics::initDateTimes()
{
    for (const QCPDataMap &dm : m_ndm) {
        QList<double> dateTimes = dm.keys();
        DateTimeVector temp(m_dateTimes.size() + dateTimes.size());
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
