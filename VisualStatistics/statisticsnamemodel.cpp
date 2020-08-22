#include "statisticsnamemodel.h"
#include "statistics.h"
#include <set>
#include <QSet>

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
    m_statNames.swap(sns);
    m_indexes.reserve(m_statNames.size());
    for (int i = 0; i < (int)m_statNames.size(); ++i) {
        m_indexes.push_back(i);
    }
    m_fetchedCount = 0;
    emit endResetModel();
}

QStringList StatisticsNameModel::getModules() const
{
    QSet<QString> modules;
    for (const std::string &statName : m_statNames) {
        QString mod = Statistics::getModuleFromStatName(statName);
        if (!mod.isEmpty()) {
            modules.insert(mod);
        }
    }
    return modules.toList();
}

void StatisticsNameModel::clearStatisticsNames()
{
    emit beginResetModel();
    m_statNames.resize(0);
    m_indexes.resize(0);
    m_fetchedCount = 0;
    emit endResetModel();
}

static bool modulesTest(const std::vector<std::string> &modules, const std::string &name)
{
    if (modules.empty()) {
        return true;
    }

    for (const std::string &mod : modules) {
        // Check for ',' in case of one module name is the prefix of another
        if (name.length() > mod.length() + 1 && name[mod.length()] == ',' &&
                name.compare(0, mod.length(), mod) == 0)
        {
            return true;
        }
    }

    return false;
}

// Use pcre for regular expression matching because the QRegExp
// is much slower in some situations
void StatisticsNameModel::setFilterPattern(const QStringList &modules, const QString &pattern, bool caseSensitive, QString &error)
{
    if (m_statNames.empty()) {
        return;
    }

    QStringList patterns = pattern.split('#');
    QString &firstPattern = patterns.first();
    bool invert = firstPattern.startsWith('!');
    if (invert) {
        firstPattern.remove(0, 1);
    }

    const char *err;
    int errOffset;
    pcre *re = pcre_compile(firstPattern.toStdString().c_str(), caseSensitive ? 0 : PCRE_CASELESS, &err, &errOffset, NULL);
    if (!re) {
        error = "invalid regular expression: ";
        error += firstPattern;
        return;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];
    pcre_extra *extra = NULL;

    extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
    // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
    pcre_assign_jit_stack(extra, NULL, m_jitStack);

    std::vector<std::string> modulesStdStr;
    for (const QString &mod : modules) {
        modulesStdStr.push_back(mod.toStdString());
    }

    emit beginResetModel();

    if (patterns.size() == 1) {
        m_indexes.resize(0);
        if (invert) {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (!modulesTest(modulesStdStr, statName)) {
                    continue;
                }
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) == PCRE_ERROR_NOMATCH)
                {
                    m_indexes.push_back(i);
                }
            }
        } else {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (!modulesTest(modulesStdStr, statName)) {
                    continue;
                }
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) != PCRE_ERROR_NOMATCH)
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
                if (!modulesTest(modulesStdStr, statName)) {
                    continue;
                }
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) == PCRE_ERROR_NOMATCH)
                {
                    tempIndexes.insert(i);
                }
            }
        } else {
            for (int i = 0; i < (int)m_statNames.size(); ++i) {
                const std::string &statName = m_statNames[i];
                if (!modulesTest(modulesStdStr, statName)) {
                    continue;
                }
                if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                  0, 0, ovector, OVECCOUNT, m_jitStack) != PCRE_ERROR_NOMATCH)
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
            re = pcre_compile((*iter).toStdString().c_str(), caseSensitive ? 0 : PCRE_CASELESS, &err, &errOffset, NULL);
            if (!re) {
                error = "invalid regular expression: ";
                error += *iter;
                break;
            }

            extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
            // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
            pcre_assign_jit_stack(extra, NULL, m_jitStack);

            if (invert) {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const std::string &statName = m_statNames[*iter];
                    if (pcre_jit_exec(re, extra, statName.c_str(), (int)statName.length(),
                                      0, 0, ovector, OVECCOUNT, m_jitStack) != PCRE_ERROR_NOMATCH)
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
                                      0, 0, ovector, OVECCOUNT, m_jitStack) != PCRE_ERROR_NOMATCH)
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
