#include "statisticsnamemodel.h"

StatisticsNameModel::StatisticsNameModel(QObject *parent) :
    QAbstractListModel(parent),
    _fetchedCount(0),
    _fetchIncrement(100),
    _jitStack(pcre_jit_stack_alloc(32 * 1024, 1024 * 1024))
{
}

StatisticsNameModel::~StatisticsNameModel()
{
    if (_jitStack) {
        pcre_jit_stack_free(_jitStack);
    }
}

void StatisticsNameModel::beforeDataContainerUpdate()
{
    emit beginResetModel();
    _pattern = "";
}

std::vector<std::string>& StatisticsNameModel::getDataContainer()
{
    return _statisticsNames;
}

void StatisticsNameModel::endDataContainerUpdate()
{
    _indexes.reserve(_statisticsNames.size());
    for (int i = 0; i < _statisticsNames.size(); ++i) {
        _indexes.push_back(i);
    }
    _fetchedCount = 0;
    emit endResetModel();
}

void StatisticsNameModel::clearStatisticsNames()
{
    emit beginResetModel();
    _statisticsNames.resize(0);
    _indexes.resize(0);
    _fetchedCount = 0;
    emit endResetModel();
}

// Use pcre for regular expression matching because the QRegExp
// is much slower in some situations

bool StatisticsNameModel::setFilterPattern(const QString &pattern)
{
    if (pattern == _pattern || _statisticsNames.empty()) {
        return true;
    }

    const char *err;
    int errOffset;
    pcre *re = pcre_compile(pattern.toStdString().c_str(), 0, &err, &errOffset, NULL);
    if (!re) {
        return false;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];
    pcre_extra *extra = NULL;

    extra = pcre_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
    // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
    pcre_assign_jit_stack(extra, NULL, _jitStack);

    emit beginResetModel();
    _pattern = pattern;
    _indexes.resize(0);
    for (int i = 0; i < _statisticsNames.size(); ++i) {
        const std::string &statisticsName = _statisticsNames[i];
        if (pcre_jit_exec(re, extra, statisticsName.c_str(), (int)statisticsName.length(),
                            0, 0, ovector, OVECCOUNT, _jitStack) > -1) {
            _indexes.push_back(i);
        }
    }
    _fetchedCount = 0;
    emit endResetModel();

    pcre_free(re);
    if (extra) {
        pcre_free_study(extra);
    }
    return true;
}

int StatisticsNameModel::actualCount() const
{
    return (int)_statisticsNames.size();
}

bool StatisticsNameModel::canFetchMore(const QModelIndex &parent) const
{
    return _fetchedCount < _indexes.size() && !parent.isValid();
}

void StatisticsNameModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    int newCount = _fetchedCount + _fetchIncrement;
    if (newCount > _indexes.size()) {
        newCount = static_cast<int>(_indexes.size());
    }
    emit beginInsertRows(QModelIndex(), _fetchedCount, newCount - 1);
    _fetchedCount = newCount;
    emit endInsertRows();
}

int	StatisticsNameModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : _fetchedCount;
}

QVariant StatisticsNameModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return _statisticsNames[_indexes[index.row()]].c_str();
        } else if (role == Qt::UserRole) {
            // Consider the first 2 column: ##date;time;shm_xxx
            return _indexes[index.row()] + 2;
        }
    }
    return QVariant();
}
