#ifndef STATISTICSNAMEMODEL_H
#define STATISTICSNAMEMODEL_H

#include <pcre.h>
#include <QAbstractListModel>

class StatisticsNameModel : public QAbstractListModel
{
public:
    typedef std::vector<std::string> StatisticsNames;

    explicit StatisticsNameModel(QObject *parent);
    StatisticsNameModel(const StatisticsNameModel &) = delete;
    StatisticsNameModel& operator=(const StatisticsNameModel &) = delete;
    ~StatisticsNameModel();

    void setStatisticsNames(StatisticsNames &sns);
    QStringList getModules() const;

    void clearStatisticsNames();

    bool setFilterPattern(const QStringList &modules, const QString &pattern, bool caseSensitive, QStringList &errList);
    int filteredCount() const;
    int totalCount() const;

    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    int m_fetchedCount;
    StatisticsNames m_statNames;
    std::vector<int> m_indexes;
    pcre_jit_stack *m_jitStack;
};

#endif // STATISTICSNAMEMODEL_H
