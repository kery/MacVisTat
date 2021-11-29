#include "CounterNameModel.h"
#include "CounterGraph.h"

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
        QString moduleName = CounterGraph::getModuleName(counterName);
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

bool CounterNameModel::moduleNameTest(const QVector<QString> &moduleNames, const QString &counterName)
{
    if (moduleNames.isEmpty()) {
        return true;
    }
    for (const QString &moduleName : moduleNames) {
        if (counterName.length() > moduleName.length() + 1 &&
            counterName[moduleName.length()] == CounterGraph::sNameSeparator &&
            counterName.startsWith(moduleName))
        {
            return true;
        }
    }
    return false;
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
        switch (role) {
        case Qt::DisplayRole:
            return mCounterNames[mMatchedIndexes[index.row()]];
        case Qt::StatusTipRole:
            break;
        case IndexRole:
            // Skip the first 2 columns (##date;time;) in CSV file.
            return mMatchedIndexes[index.row()] + 2;
        }
    }
    return QVariant();
}
