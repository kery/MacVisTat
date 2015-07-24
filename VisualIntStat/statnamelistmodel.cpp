#include "statnamelistmodel.h"

StatNameListModel::StatNameListModel(QObject *parent) :
    QAbstractListModel(parent),
    _fetchedCount(0),
    _fetchIncrement(100),
    _jitStack(pcre16_jit_stack_alloc(32 * 1024, 1024 * 1024))
{
}

StatNameListModel::~StatNameListModel()
{
    if (_jitStack) {
        pcre16_jit_stack_free(_jitStack);
    }
}

void StatNameListModel::setStatNames(const QVector<QString> &statNames)
{
    emit beginResetModel();
    _statNames = statNames;
    _indexes.reserve(_statNames.size());
    for (int i = 0; i < _statNames.size(); ++i) {
        _indexes.push_back(i);
    }
    _fetchedCount = 0;
    emit endResetModel();
}

void StatNameListModel::clearStatNames()
{
    emit beginResetModel();
    _statNames.clear();
    _indexes.resize(0);
    _fetchedCount = 0;
    emit endResetModel();
}

// Use pcre for regular expression matching because the QRegExp
// is much slower in some situations

bool StatNameListModel::setFilterPattern(const QString &pattern)
{
    if (pattern == _pattern) {
        return false;
    }

    const char *err;
    int errOffset;
    pcre16 *re = pcre16_compile(pattern.utf16(), 0, &err, &errOffset, NULL);
    if (!re) {
        return false;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];
    pcre16_extra *extra = NULL;

    extra = pcre16_study(re, PCRE_STUDY_EXTRA_NEEDED | PCRE_STUDY_JIT_COMPILE, &err);
    // pcre_assign_jit_stack() does nothing unless the extra argument is non-NULL
    pcre16_assign_jit_stack(extra, NULL, _jitStack);

    emit beginResetModel();
    _pattern = pattern;
    _indexes.resize(0);
    for (int i = 0; i < _statNames.size(); ++i) {
        const QString &statName = _statNames.at(i);
        if (pcre16_jit_exec(re, extra, statName.utf16(), statName.length(),
                            0, 0, ovector, OVECCOUNT, _jitStack) > -1) {
            _indexes.push_back(i);
        }
    }
    _fetchedCount = 0;
    emit endResetModel();

    pcre16_free(re);
    if (extra) {
        pcre16_free_study(extra);
    }
    return true;
}

int StatNameListModel::actualCount() const
{
    return _statNames.size();
}

bool StatNameListModel::canFetchMore(const QModelIndex &parent) const
{
    return _fetchedCount < _indexes.size() && !parent.isValid();
}

void StatNameListModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    int newCount = _fetchedCount + _fetchIncrement;
    if (newCount > _indexes.size()) {
        newCount = _indexes.size();
    }
    emit beginInsertRows(QModelIndex(), _fetchedCount, newCount - 1);
    _fetchedCount = newCount;
    emit endInsertRows();
}

int	StatNameListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : _fetchedCount;
}

QVariant StatNameListModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid()) {
        return QVariant();
    }
    return _statNames.at(_indexes[index.row()]);
}
