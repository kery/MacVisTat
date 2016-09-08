#include "statisticsnamemodel.h"
#include <set>

StatisticsNameModel::StatisticsNameModel(QObject *parent) :
    QAbstractListModel(parent),
    m_fetchedCount(0),
    m_jitStack(pcre_jit_stack_alloc(32 * 1024, 1024 * 1024))
{
}

StatisticsNameModel::~StatisticsNameModel()
{
    if (m_jitStack) {
        pcre_jit_stack_free(m_jitStack);
    }
}

void StatisticsNameModel::setStatisticsNames(StatisticsNames &sns)
{
    emit beginResetModel();
    m_pattern = "";
    m_statNames.swap(sns);
    m_indexes.reserve(m_statNames.size());
    for (int i = 0; i < (int)m_statNames.size(); ++i) {
        m_indexes.push_back(i);
    }
    m_fetchedCount = 0;
    emit endResetModel();
}

void StatisticsNameModel::clearStatisticsNames()
{
    emit beginResetModel();
    m_statNames.resize(0);
    m_indexes.resize(0);
    m_fetchedCount = 0;
    emit endResetModel();
}

// Use pcre for regular expression matching because the QRegExp
// is much slower in some situations
bool StatisticsNameModel::setFilterPattern(const QString &pattern, QStringList &errList)
{
    if (pattern == m_pattern || m_statNames.empty()) {
        return true;
    }

    QStringList patterns = pattern.split('#');
    QString &firstPattern = patterns.first();
    bool invert = firstPattern.startsWith('!');
    if (invert) {
        firstPattern.remove(0, 1);
    }

    const char *err;
    int errOffset;
    pcre *re = pcre_compile(firstPattern.toStdString().c_str(), 0, &err, &errOffset, NULL);
    if (!re) {
        errList << "PCRE compile failed: " + firstPattern;
        return false;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];
    pcre_extra *extra = NULL;

    extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
    // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
    pcre_assign_jit_stack(extra, NULL, m_jitStack);

    emit beginResetModel();
    m_pattern = pattern;

    if (patterns.size() == 1) {
        m_indexes.resize(0);
        if (invert) {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) == -1)
                {
                    m_indexes.push_back(i);
                }
            }
        } else {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) > -1)
                {
                    m_indexes.push_back(i);
                }
            }
        }
        pcre_free(re);
        if (extra) {
            pcre_free_study(extra);
        }
    } else {
        std::set<int> tempIndexes;
        if (invert) {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) == -1)
                {
                    tempIndexes.insert(i);
                }
            }
        } else {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) > -1)
                {
                    tempIndexes.insert(i);
                }
            }
        }
        pcre_free(re);
        if (extra) {
            pcre_free_study(extra);
        }

        for (auto iter = patterns.begin() + 1; iter != patterns.end(); ++iter) {
            invert = iter->startsWith('!');
            if (invert) {
                iter->remove(0, 1);
            }
            re = pcre_compile((*iter).toStdString().c_str(), 0, &err, &errOffset, NULL);
            if (!re) {
                errList << "PCRE compile failed: " + *iter;
                break;
            }

            extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
            // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
            pcre_assign_jit_stack(extra, NULL, m_jitStack);

            if (invert) {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const std::string &statName = m_statNames[*iter];
                    if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                      0, 0, ovector, OVECCOUNT, m_jitStack) > -1)
                    {
                        iter = tempIndexes.erase(iter);
                    } else {
                        ++iter;
                    }
                }
            } else {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const std::string &statName = m_statNames[*iter];
                    if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                      0, 0, ovector, OVECCOUNT, m_jitStack) > -1)
                    {
                        ++iter;
                    } else {
                        iter = tempIndexes.erase(iter);
                    }
                }
            }
            pcre_free(re);
            if (extra) {
                pcre_free_study(extra);
            }
        }

        m_indexes.resize(tempIndexes.size());
        std::copy(tempIndexes.begin(), tempIndexes.end(), m_indexes.begin());
    }
    m_fetchedCount = 0;
    emit endResetModel();

    return true;
}

int StatisticsNameModel::filteredCount() const
{
    return (int)m_indexes.size();
}

int StatisticsNameModel::totalCount() const
{
    return (int)m_statNames.size();
}

bool StatisticsNameModel::canFetchMore(const QModelIndex &parent) const
{
    return m_fetchedCount < (int)m_indexes.size() && !parent.isValid();
}

void StatisticsNameModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    int newCount = m_fetchedCount + 100;
    if (newCount > (int)m_indexes.size()) {
        newCount = static_cast<int>(m_indexes.size());
    }
    emit beginInsertRows(QModelIndex(), m_fetchedCount, newCount - 1);
    m_fetchedCount = newCount;
    emit endInsertRows();
}

int	StatisticsNameModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_fetchedCount;
}

QVariant StatisticsNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return m_statNames[m_indexes[index.row()]].c_str();
        } else if (role == Qt::UserRole) {
            // Consider the first 2 column: ##date;time;shm_xxx
            return m_indexes[index.row()] + 2;
        }
    }
    return QVariant();
}
