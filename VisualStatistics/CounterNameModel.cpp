#include "CounterNameModel.h"
#include "GzipFile.h"
#include "GlobalDefines.h"
#include "libcsv/csv.h"
#include <QSet>
#include <QSettings>

QChar CounterNameModel::sModuleSeparator;
QChar CounterNameModel::sGroupSeparator;
QChar CounterNameModel::sIndexesSeparator;

CounterId::CounterId(const QString &module, const QString &group, const QString &object) :
    mModule(module),
    mGroup(group),
    mObject(object)
{
}

bool CounterId::operator==(const CounterId &other) const
{
    return other.mModule == mModule && other.mGroup == mGroup && other.mObject == mObject;
}

uint qHash(const CounterId &cid, uint seed)
{
    return qHash(cid.mModule, seed) ^ qHash(cid.mModule, seed) ^ qHash(cid.mObject, seed);
}

CounterNameModel::CounterNameModel(QObject *parent) :
    QAbstractListModel(parent),
    mFetchedCount(0),
    mJitStack(pcre_jit_stack_alloc(32 * 1024, 1024 * 1024))
{
}

CounterNameModel::~CounterNameModel()
{
    if (mJitStack) {
        pcre_jit_stack_free(mJitStack);
    }
}

void CounterNameModel::setCounterNames(QVector<QString> &names)
{
    beginResetModel();
    mCounterNames.swap(names);
    mMatchedIndexes.reserve(mCounterNames.size());
    for (int i = 0; i < mCounterNames.size(); ++i) {
        mMatchedIndexes.append(i);
    }
    mFetchedCount = 0;
    endResetModel();
}

QStringList CounterNameModel::moduleNames() const
{
    QSet<QString> result;
    for (const QString &counterName : mCounterNames) {
        QString moduleName = getModuleName(counterName);
        if (!moduleName.isEmpty()) {
            result.insert(moduleName);
        }
    }
    return result.toList();
}

void CounterNameModel::clear()
{
    beginResetModel();
    mFetchedCount = 0;
    mMatchedIndexes.clear();
    mCounterNames.clear();
    endResetModel();
}

// Use pcre for regular expression matching because the QRegExp
// is much slower in some situations
QString CounterNameModel::setFilterPattern(const QVector<QString> &moduleNames, const QString &pattern, bool caseSensitive)
{
    QString error;
    if (mCounterNames.isEmpty()) {
        return error;
    }

    QStringList patterns = pattern.split('#');
    QString &firstPattern = patterns.first();
    bool invert = firstPattern.startsWith('!');
    if (invert) {
        firstPattern.remove(0, 1);
    }

    const char *err;
    int errOffset;
    QByteArray baStr = firstPattern.toLocal8Bit();
    pcre *re = pcre_compile(baStr.data(), caseSensitive ? 0 : PCRE_CASELESS, &err, &errOffset, NULL);
    if (!re) {
        error = "invalid regular expression '";
        error += firstPattern;
        error += "'";
        return error;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];
    pcre_extra *extra = NULL;
    extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
    // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
    pcre_assign_jit_stack(extra, NULL, mJitStack);

    beginResetModel();

    if (patterns.size() == 1) {
        mMatchedIndexes.clear();
        if (invert) {
            for (int i = 0; i < mCounterNames.size(); ++i) {
                const QString &counterName = mCounterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, mJitStack) == PCRE_ERROR_NOMATCH)
                {
                    mMatchedIndexes.append(i);
                }
            }
        } else {
            for (int i = 0; i < mCounterNames.size(); ++i) {
                const QString &counterName = mCounterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, mJitStack) != PCRE_ERROR_NOMATCH)
                {
                    mMatchedIndexes.append(i);
                }
            }
        }
        pcre_free(re);
        if (extra) {
            pcre_free_study(extra);
        }
    } else {
        QSet<int> tempIndexes;
        if (invert) {
            for (int i = 0; i < mCounterNames.size(); ++i) {
                const QString &counterName = mCounterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, mJitStack) == PCRE_ERROR_NOMATCH)
                {
                    tempIndexes.insert(i);
                }
            }
        } else {
            for (int i = 0; i < mCounterNames.size(); ++i) {
                const QString &counterName = mCounterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, mJitStack) != PCRE_ERROR_NOMATCH)
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
            baStr = iter->toLocal8Bit();
            re = pcre_compile(baStr.data(), caseSensitive ? 0 : PCRE_CASELESS, &err, &errOffset, NULL);
            if (!re) {
                error = "invalid regular expression '";
                error += *iter;
                break;
            }

            extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
            // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
            pcre_assign_jit_stack(extra, NULL, mJitStack);

            if (invert) {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = mCounterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                      0, 0, ovector, OVECCOUNT, mJitStack) != PCRE_ERROR_NOMATCH)
                    {
                        iter = tempIndexes.erase(iter);
                    } else {
                        ++iter;
                    }
                }
            } else {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = mCounterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                      0, 0, ovector, OVECCOUNT, mJitStack) != PCRE_ERROR_NOMATCH)
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

        mMatchedIndexes.resize(tempIndexes.size());
        std::copy(tempIndexes.begin(), tempIndexes.end(), mMatchedIndexes.begin());
    }
    mFetchedCount = 0;
    endResetModel();

    return error;
}

int CounterNameModel::matchedCount() const
{
    return mMatchedIndexes.size();
}

int CounterNameModel::totalCount() const
{
    return mCounterNames.size();
}

void CounterNameModel::parseCounterDescription(const QString &path)
{
    GzipFile reader;
    if (!reader.open(path, GzipFile::ReadOnly)) {
        return;
    }

    CsvCallbackUserData ud;
    ud.columns.reserve(4);
    ud.description = &mCounterDescription;

    struct csv_parser parser;
    csv_init(&parser, CSV_STRICT | CSV_STRICT_FINI | CSV_EMPTY_IS_NULL);

    std::string line;
    reader.readLineKeepCrLf(line); // Consume the header line
    while (reader.readLineKeepCrLf(line)) {
        csv_parse(&parser, line.c_str(), line.length(), libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    }

    csv_fini(&parser, libcsvCbEndOfField, libcsvCbEndOfRow, &ud);
    csv_free(&parser);
}

bool CounterNameModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mFetchedCount < mMatchedIndexes.size();
}

void CounterNameModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    int newCount = qMin(mFetchedCount + 100, mMatchedIndexes.size());

    beginInsertRows(QModelIndex(), mFetchedCount, newCount - 1);
    mFetchedCount = newCount;
    endInsertRows();
}

int CounterNameModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mFetchedCount;
}

QVariant CounterNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return mCounterNames[mMatchedIndexes[index.row()]];
        } else if (role == Qt::StatusTipRole) {
            CounterId cid = getCounterId(mCounterNames[mMatchedIndexes[index.row()]]);
            auto iter = mCounterDescription.find(cid);
            if (iter != mCounterDescription.end()) {
                return iter.value();
            }
        } else if (role == IndexRole) {
            // Skip the first 2 columns (##date;time;) in CSV file.
            return mMatchedIndexes[index.row()] + 2;
        }
    }
    return QVariant();
}

void CounterNameModel::initSeparators()
{
    QSettings setting;
    sModuleSeparator = setting.value(SETTING_KEY_MODULE_SEP, ',').toChar();
    sGroupSeparator = setting.value(SETTING_KEY_GROUP_SEP, ',').toChar();
    sIndexesSeparator = setting.value(SETTING_KEY_INDEXES_SEP, ',').toChar();
}

QString CounterNameModel::getModuleName(const QString &name)
{
    int index = name.indexOf(sModuleSeparator);
    if (index > 0) {
        return name.left(index);
    }
    return QString();
}

QString CounterNameModel::getObjectName(const QString &name)
{
    return name.mid(name.lastIndexOf(sIndexesSeparator) + 1);
}

QPair<QString, QString> CounterNameModel::separateModuleName(const QString &name)
{
    QPair<QString, QString> result;
    int index = name.indexOf(sModuleSeparator);
    if (index > 0) {
        result.first = name.left(index);
        result.second = name.mid(index + 1);
    } else {
        result.second = name;
    }
    return result;
}

void CounterNameModel::libcsvCbEndOfField(void *field, size_t len, void *ud)
{
    auto ccud = static_cast<CsvCallbackUserData*>(ud);
    ccud->columns.append(field ? QString::fromLatin1(static_cast<const char*>(field), static_cast<int>(len)) : QString());
}

void CounterNameModel::libcsvCbEndOfRow(int, void *ud)
{
    auto ccud = static_cast<CsvCallbackUserData*>(ud);
    if (ccud->columns.size() == 4) {
        CounterId cid(ccud->columns[0], ccud->columns[1], ccud->columns[2]);
        ccud->description->insert(cid, ccud->columns[3]);
    }
    ccud->columns.clear();
}

CounterId CounterNameModel::getCounterId(const QString &name)
{
    QString module, group, object;
    int pos1 = name.indexOf(sModuleSeparator);
    if (pos1 != -1) {
        module = name.mid(0, pos1);
        pos1 = name.indexOf(QLatin1String("GroupName="), pos1);
        if (pos1 != -1) {
            pos1 += 10;
            int pos2 = name.indexOf(sGroupSeparator, pos1);
            if (pos2 != -1) {
                group = name.mid(pos1, pos2 - pos1);
            }
        }
        // NRD counter has no GroupName, so we continue to get the KPI-KCI Object field
        pos1 = name.lastIndexOf(sIndexesSeparator);
        if (pos1 != -1) {
            object = name.mid(pos1 + 1);
        }
    }
    return CounterId(module, group, object);
}

bool CounterNameModel::moduleNameTest(const QVector<QString> &moduleNames, const QString &name)
{
    if (moduleNames.isEmpty()) {
        return true;
    }
    for (const QString &moduleName : moduleNames) {
        if (name.length() > moduleName.length() + 1 &&
            name[moduleName.length()] == sModuleSeparator &&
            name.startsWith(moduleName))
        {
            return true;
        }
    }
    return false;
}
