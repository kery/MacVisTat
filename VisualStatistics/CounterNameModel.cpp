#include "CounterNameModel.h"
#include "CounterGraph.h"

CounterNameModel::CounterNameModel(QObject *parent) :
    QAbstractListModel(parent),
    _fetchedCount(0),
    _jitStack(pcre_jit_stack_alloc(32 * 1024, 1024 * 1024))
{
}

CounterNameModel::~CounterNameModel()
{
    if (_jitStack) {
        pcre_jit_stack_free(_jitStack);
    }
}

void CounterNameModel::setCounterNames(QVector<QString> &names)
{
    beginResetModel();
    _counterNames.swap(names);
    _matchedIndexes.reserve(_counterNames.size());
    for (int i = 0; i < _counterNames.size(); ++i) {
        _matchedIndexes.append(i);
    }
    _fetchedCount = 0;
    endResetModel();
}

QStringList CounterNameModel::moduleNames() const
{
    QSet<QString> result;
    for (const QString &counterName : _counterNames) {
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
    _fetchedCount = 0;
    _matchedIndexes.clear();
    _counterNames.clear();
    endResetModel();
}

bool CounterNameModel::moduleNameTest(const QVector<QString> &moduleNames, const QString &counterName)
{
    if (moduleNames.isEmpty()) {
        return true;
    }
    for (const QString &moduleName : moduleNames) {
        if (counterName.length() > moduleName.length() + 1 &&
            counterName[moduleName.length()] == CounterGraph::nameSeparator &&
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
    if (_counterNames.isEmpty()) {
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
    pcre_assign_jit_stack(extra, NULL, _jitStack);

    beginResetModel();

    if (patterns.size() == 1) {
        _matchedIndexes.clear();
        if (invert) {
            for (int i = 0; i < _counterNames.size(); ++i) {
                const QString &counterName = _counterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, _jitStack) == PCRE_ERROR_NOMATCH)
                {
                    _matchedIndexes.append(i);
                }
            }
        } else {
            for (int i = 0; i < _counterNames.size(); ++i) {
                const QString &counterName = _counterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, _jitStack) != PCRE_ERROR_NOMATCH)
                {
                    _matchedIndexes.append(i);
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
            for (int i = 0; i < _counterNames.size(); ++i) {
                const QString &counterName = _counterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, _jitStack) == PCRE_ERROR_NOMATCH)
                {
                    tempIndexes.insert(i);
                }
            }
        } else {
            for (int i = 0; i < _counterNames.size(); ++i) {
                const QString &counterName = _counterNames[i];
                if (!moduleNameTest(moduleNames, counterName)) {
                    continue;
                }
                baStr = counterName.toLocal8Bit();
                if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                  0, 0, ovector, OVECCOUNT, _jitStack) != PCRE_ERROR_NOMATCH)
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
            pcre_assign_jit_stack(extra, NULL, _jitStack);

            if (invert) {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = _counterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                      0, 0, ovector, OVECCOUNT, _jitStack) != PCRE_ERROR_NOMATCH)
                    {
                        iter = tempIndexes.erase(iter);
                    } else {
                        ++iter;
                    }
                }
            } else {
                for (auto iter = tempIndexes.begin(); iter != tempIndexes.end();) {
                    const QString &counterName = _counterNames[*iter];
                    baStr = counterName.toLocal8Bit();
                    if (pcre_jit_exec(re, extra, baStr.data(), counterName.length(),
                                      0, 0, ovector, OVECCOUNT, _jitStack) != PCRE_ERROR_NOMATCH)
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

        _matchedIndexes.resize(tempIndexes.size());
        std::copy(tempIndexes.begin(), tempIndexes.end(), _matchedIndexes.begin());
    }
    _fetchedCount = 0;
    endResetModel();

    return error;
}

int CounterNameModel::matchedCount() const
{
    return _matchedIndexes.size();
}

int CounterNameModel::totalCount() const
{
    return _counterNames.size();
}

bool CounterNameModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _fetchedCount < _matchedIndexes.size();
}

void CounterNameModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    int newCount = qMin(_fetchedCount + 100, _matchedIndexes.size());

    beginInsertRows(QModelIndex(), _fetchedCount, newCount - 1);
    _fetchedCount = newCount;
    endInsertRows();
}

int CounterNameModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _fetchedCount;
}

QVariant CounterNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
            return _counterNames[_matchedIndexes[index.row()]];
        case Qt::StatusTipRole:
            break;
        case IndexRole:
            // Skip the first 2 columns (##date;time;) in CSV file.
            return _matchedIndexes[index.row()] + 2;
        }
    }
    return QVariant();
}
