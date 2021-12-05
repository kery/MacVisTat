#ifndef COUNTERNAMEMODEL_H
#define COUNTERNAMEMODEL_H

#include <QAbstractListModel>
#include "pcre/pcre.h"

class CounterId
{
public:
    CounterId(const QString &module, const QString &group, const QString &object);

    bool operator==(const CounterId &other) const;

private:
    QString mModule, mGroup, mObject;

    friend uint qHash(const CounterId &cid, uint seed);
};

uint qHash(const CounterId &cid, uint seed);

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
    QString setFilterPattern(const QVector<QString> &moduleNames, const QString &pattern, bool caseSensitive);
    int matchedCount() const;
    int totalCount() const;
    void parseCounterDescription(const QString &path);

    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    static void initSeparators();
    static QString getModuleName(const QString &name);
    static QString getObjectName(const QString &name);
    static QPair<QString, QString> separateModuleName(const QString &name);

private:
    struct CsvCallbackUserData
    {
        QVector<QString> columns;
        QHash<CounterId, QString> *description;
    };

    static void libcsvCbEndOfField(void *field, size_t len, void *ud);
    static void libcsvCbEndOfRow(int, void *ud);
    static CounterId getCounterId(const QString &name);
    static bool moduleNameTest(const QVector<QString> &moduleNames, const QString &name);

    // Module              Group                           Indexes                                                      KPI/KCI Object
    // -------------------,-------------------------------,------------------------------------------------------------,--------------
    // KPIReferencePointGX,KPI=ReferencePoint,GroupName=GX,vprnRouterName=vprn20,vrId=3,address=10.234.110.34,port=3868,VS.RxCcaI
    static QChar sModuleSeparator;
    static QChar sGroupSeparator;
    static QChar sIndexesSeparator;

    int mFetchedCount;
    QVector<int> mMatchedIndexes;
    QVector<QString> mCounterNames;
    QHash<CounterId, QString> mCounterDescription;
    pcre_jit_stack *mJitStack;
};

#endif // COUNTERNAMEMODEL_H
