#ifndef STATISTICSPROXYMODEL_H
#define STATISTICSPROXYMODEL_H

#include <QModelIndex>
#include <QAbstractItemModel>

class StatisticsComponent;

class StatisticsProxyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    StatisticsProxyModel(QObject *parent = NULL);
    ~StatisticsProxyModel();

    void attachSourceModel(QAbstractItemModel *model);
    void detachSourceModel();
    QAbstractItemModel* sourceModel() const;

    virtual QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex	parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int	columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    void addStatistics(const QString &text);
    void reset();

private:
    QAbstractItemModel *_sourceModel;
    StatisticsComponent *_rootComponent;
};

#endif // STATISTICSPROXYMODEL_H
