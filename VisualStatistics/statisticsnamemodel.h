#ifndef STATISTICSNAMEMODEL_H
#define STATISTICSNAMEMODEL_H

#include <pcre.h>
#include <QAbstractListModel>

class StatisticsNameModel : public QAbstractListModel
{
public:
    explicit StatisticsNameModel(QObject *parent = NULL);
    ~StatisticsNameModel();

    void beforeDataContainerUpdate();
    std::vector<std::string>& getDataContainer();
    void endDataContainerUpdate();

    void clearStatisticsNames();

    bool setFilterPattern(const QString &pattern);
    int filteredCount() const;
    int totalCount() const;

    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    int _fetchedCount;
    int _fetchIncrement;
    QString _pattern;
    std::vector<std::string> _statisticsNames;
    std::vector<int> _indexes;
    pcre_jit_stack *_jitStack;
};

#endif // STATISTICSNAMEMODEL_H
