#include "StatisticsNameModel.h"
#include "Statistics.h"
#include "GzipFile.h"
#include "libcsv/csv.h"

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
    beginResetModel();
    m_statNames.swap(sns);
    m_indexes.reserve(m_statNames.size());
    for (int i = 0; i < (int)m_statNames.size(); ++i) {
        m_indexes.push_back(i);
    }
    m_fetchedCount = 0;
    endResetModel();
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
    beginResetModel();
    m_statNames.resize(0);
    m_indexes.resize(0);
    m_fetchedCount = 0;
    endResetModel();
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

    beginResetModel();

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
    endResetModel();
}

int StatisticsNameModel::matchedCount() const
{
    return (int)m_indexes.size();
}

int StatisticsNameModel::totalCount() const
{
    return (int)m_statNames.size();
}

struct CsvCbUserData {
    std::vector<std::string> columns;
    std::unordered_map<StatisticsNameModel::StatId, std::string, StatisticsNameModel::StatIdHasher> *desc;
};

static void libcsvCbEndOfField(void *field, size_t len, void *ud)
{
    auto ccud = static_cast<CsvCbUserData *>(ud);
    ccud->columns.push_back(field ? std::string((const char *)field, len) : std::string());
}

static void libcsvCbEndOfRow(int, void *ud)
{
    auto ccud = static_cast<CsvCbUserData *>(ud);
    if (ccud->columns.size() == 4) {
        ccud->desc->emplace(StatisticsNameModel::StatId(ccud->columns[0], ccud->columns[1], ccud->columns[2]), ccud->columns[3]);
    }
    ccud->columns.clear();
}

void StatisticsNameModel::parseStatDescription(const QString &path)
{
    GzipFile fileReader;
    if (!fileReader.open(path)) {
        return;
    }

    CsvCbUserData ud;
    ud.columns.reserve(4);
    ud.desc = &m_statDesc;

    struct csv_parser p;
    csv_init(&p, CSV_STRICT|CSV_STRICT_FINI|CSV_EMPTY_IS_NULL);

    std::string line;
    fileReader.readLine(line, false); // Consume the header line
    while (fileReader.readLine(line, false)) {
        csv_parse(&p, line.c_str(), line.length(), libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    }

    csv_fini(&p, libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    csv_free(&p);
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
    beginInsertRows(QModelIndex(), m_fetchedCount, newCount - 1);
    m_fetchedCount = newCount;
    endInsertRows();
}

int StatisticsNameModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_fetchedCount;
}

static StatisticsNameModel::StatId getStatId(const std::string &name)
{
    StatisticsNameModel::StatId sid;
    size_t pos1 = name.find(',');
    if (pos1 != std::string::npos) {
        sid.module = name.substr(0, pos1);
        pos1 = name.find("GroupName=", pos1);
        if (pos1 != std::string::npos) {
            pos1 += 10;
            size_t pos2 = name.find(',', pos1);
            if (pos2 != std::string::npos) {
                sid.group = name.substr(pos1, pos2 - pos1);
            }
        }
        // NRD counter has no GroupName, so we continue to get the KPI-KCI Object field
        pos1 = name.rfind(',');
        if (pos1 != std::string::npos) {
            sid.object = name.substr(pos1 + 1);
        }
    }
    return sid;
}

QVariant StatisticsNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return m_statNames[m_indexes[index.row()]].c_str();
        } else if (role == Qt::StatusTipRole) {
            const std::string &statName = m_statNames[m_indexes[index.row()]];
            StatisticsNameModel::StatId sid = getStatId(statName);
            if (m_statDesc.find(sid) != m_statDesc.end()) {
                return m_statDesc.at(sid).c_str();
            }
        } else if (role == Qt::UserRole) {
            // Consider the first 2 column: ##date;time;shm_xxx
            return m_indexes[index.row()] + 2;
        }
    }
    return QVariant();
}
