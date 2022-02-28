#include "CounterNameModel.h"
#include "CounterName.h"
#include "CounterDescription.h"
#include <QSet>

CounterNameModel::CounterNameModel(QObject *parent) :
    QAbstractListModel(parent),
    mFetchedCount(0),
    mJitStack(pcre2_jit_stack_create(32 * 1024, 1024 * 1024, nullptr))
{
}

CounterNameModel::~CounterNameModel()
{
    // If its argument is NULL, this function returns immediately, without doing anything.
    pcre2_jit_stack_free(mJitStack);
}

void CounterNameModel::setCounterDescription(CounterDescription *desc)
{
    mCounterDesc = desc;
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
        QString moduleName = CounterName::getModuleName(counterName);
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

    int errCode;
    PCRE2_SIZE errOffset;
    QByteArray baStr = firstPattern.toLocal8Bit();
    pcre2_code *re = pcre2_compile((PCRE2_SPTR)baStr.data(),
                                   baStr.size(),
                                   caseSensitive ? 0 : PCRE2_CASELESS,
                                   &errCode,
                                   &errOffset,
                                   nullptr);
    if (!re) {
        error = "invalid regular expression '";
        error += firstPattern;
        error += "'";
        return error;
    }

    pcre2_match_data *matchData = pcre2_match_data_create_from_pattern(re, nullptr);
    pcre2_match_context *matchCtx = pcre2_match_context_create(nullptr);
    pcre2_jit_stack_assign(matchCtx, nullptr, mJitStack);
    pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);

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
                if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) < 0) {
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
                if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) >= 0) {
                    mMatchedIndexes.append(i);
                }
            }
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
                if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) < 0) {
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
                if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) >= 0) {
                    tempIndexes.insert(i);
                }
            }
        }
        pcre2_match_data_free(matchData);
        matchData = nullptr;
        pcre2_code_free(re);
        re = nullptr;

        for (auto iter = patterns.begin() + 1; iter != patterns.end(); ++iter) {
            invert = iter->startsWith('!');
            if (invert) {
                iter->remove(0, 1);
            }
            baStr = iter->toLocal8Bit();
            re = pcre2_compile((PCRE2_SPTR)baStr.data(),
                               baStr.size(),
                               caseSensitive ? 0 : PCRE2_CASELESS,
                               &errCode,
                               &errOffset,
                               nullptr);
            if (!re) {
                error = "invalid regular expression '";
                error += *iter;
                error += "'";
                break;
            }

            matchData = pcre2_match_data_create_from_pattern(re, nullptr);
            pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);

            if (invert) {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = mCounterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) >= 0) {
                        iter = tempIndexes.erase(iter);
                    } else {
                        ++iter;
                    }
                }
            } else {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = mCounterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre2_match(re, (PCRE2_SPTR)baStr.data(), baStr.size(), 0, 0, matchData, matchCtx) >= 0) {
                        ++iter;
                    } else {
                        iter = tempIndexes.erase(iter);
                    }
                }
            }
        }

        mMatchedIndexes.resize(tempIndexes.size());
        std::copy(tempIndexes.begin(), tempIndexes.end(), mMatchedIndexes.begin());
    }
    mFetchedCount = 0;
    endResetModel();

    pcre2_match_context_free(matchCtx);
    pcre2_match_data_free(matchData);
    pcre2_code_free(re);
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

bool CounterNameModel::canFetchMore(const QModelIndex &/*parent*/) const
{
    return mFetchedCount < mMatchedIndexes.size();
}

void CounterNameModel::fetchMore(const QModelIndex &/*parent*/)
{
    int newCount = qMin(mFetchedCount + 1000, mMatchedIndexes.size());

    beginInsertRows(QModelIndex(), mFetchedCount, newCount - 1);
    mFetchedCount = newCount;
    endInsertRows();
}

int CounterNameModel::rowCount(const QModelIndex &/*parent*/) const
{
    return mFetchedCount;
}

QVariant CounterNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return mCounterNames[mMatchedIndexes[index.row()]];
        } else if (role == Qt::StatusTipRole) {
            return mCounterDesc->getDescription(mCounterNames[mMatchedIndexes[index.row()]]);
        } else if (role == IndexRole) {
            // Skip the first 2 columns (##date;time;) in CSV file.
            return mMatchedIndexes[index.row()] + 2;
        }
    }
    return QVariant();
}

bool CounterNameModel::moduleNameTest(const QVector<QString> &moduleNames, const QString &name)
{
    if (moduleNames.isEmpty()) {
        return true;
    }
    for (const QString &moduleName : moduleNames) {
        if (name.length() > moduleName.length() + 1 &&
            name[moduleName.length()] == CounterName::sModuleSeparator &&
            name.startsWith(moduleName))
        {
            return true;
        }
    }
    return false;
}
