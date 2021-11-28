#ifndef COUNTERNAMEMODEL_H
#define COUNTERNAMEMODEL_H

#include <QAbstractListModel>
#include "pcre/pcre.h"

class CounterNameModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum {
        IndexRole = Qt::UserRole
    };

    CounterNameModel(QObject *parent = nullptr);
    ~CounterNameModel();

    void setCounterNames(QVector<QString> &names);
    QStringList moduleNames() const;
    void clear();

    static bool moduleNameTest(const QVector<QString> &moduleNames, const QString &counterName);
    QString setFilterPattern(const QVector<QString> &moduleNames, const QString &pattern, bool caseSensitive);
    int matchedCount() const;
    int totalCount() const;

    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    int _fetchedCount;
    QVector<int> _matchedIndexes;
    QVector<QString> _counterNames;
    pcre_jit_stack *_jitStack;
};

#endif // COUNTERNAMEMODEL_H
