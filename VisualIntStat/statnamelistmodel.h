#ifndef STATNAMELISTMODEL_H
#define STATNAMELISTMODEL_H

#include <pcre.h>
#include <QAbstractListModel>

class StatNameListModel : public QAbstractListModel
{
public:
    explicit StatNameListModel(QObject *parent = NULL);
    ~StatNameListModel();

    void setStatNames(const QStringList &statNames);
    void clearStatNames();

    bool setFilterPattern(const QString &pattern);
    int actualCount() const;

    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    int _fetchedCount;
    int _fetchIncrement;
    QString _pattern;
    QStringList _statNames;
    std::vector<int> _indexes;
    pcre16_jit_stack *_jitStack;
};

#endif // STATNAMELISTMODEL_H
