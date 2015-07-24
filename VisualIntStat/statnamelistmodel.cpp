#include "statnamelistmodel.h"
#include "third_party/pcre/pcre.h"

StatNameListModel::StatNameListModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

StatNameListModel::~StatNameListModel()
{

}

bool StatNameListModel::setFilterPattern(const QString &pattern)
{
    if (pattern == _pattern) {
        return false;
    }

    const char err;
    int errOffset;
    pcre16 *re = pcre16_compile(pattern.utf16(), 0, &err, &errOffset, NULL);
    if (!re) {
        return false;
    }
}

bool StatNameListModel::canFetchMore(const QModelIndex &parent) const
{
    return false;
}

void StatNameListModel::fetchMore(const QModelIndex &parent)
{
}

int	StatNameListModel::rowCount(const QModelIndex &parent) const
{
    return _indexes.size();
}

QVariant StatNameListModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}
