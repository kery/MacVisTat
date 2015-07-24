#ifndef STATNAMELISTMODEL_H
#define STATNAMELISTMODEL_H

#include <QAbstractListModel>

class StatNameListModel : public QAbstractListModel
{
public:
    explicit StatNameListModel(QObject *parent = NULL);
    ~StatNameListModel();

    bool setFilterPattern(const QString &pattern);

    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QString _pattern;
    QVector<QString> _statNames;
    std::vector<int> _indexes;
};

#endif // STATNAMELISTMODEL_H
