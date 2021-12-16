#ifndef COUNTERNAMEMODEL_H
#define COUNTERNAMEMODEL_H

#include <QAbstractListModel>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

class CounterDescription;

class CounterNameModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum {
        IndexRole = Qt::UserRole
    };

    CounterNameModel(QObject *parent);
    ~CounterNameModel();

    void setCounterDescription(CounterDescription *desc);
    void setCounterNames(QVector<QString> &names);
    QStringList moduleNames() const;
    void clear();
    QString setFilterPattern(const QVector<QString> &moduleNames, const QString &pattern, bool caseSensitive);
    int matchedCount() const;
    int totalCount() const;

    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    static bool moduleNameTest(const QVector<QString> &moduleNames, const QString &name);

    int mFetchedCount;
    QVector<int> mMatchedIndexes;
    QVector<QString> mCounterNames;
    CounterDescription *mCounterDesc;
    pcre2_jit_stack *mJitStack;
};

#endif // COUNTERNAMEMODEL_H
