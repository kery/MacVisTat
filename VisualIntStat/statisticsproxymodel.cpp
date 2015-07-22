#include "statisticsproxymodel.h"
// TODO: remove
#include <QDebug>

class StatisticsComponent
{
public:
    StatisticsComponent(const QString text, StatisticsComponent *parent = NULL);
    ~StatisticsComponent();

    QString text() const;
    StatisticsComponent* child(int index) const;
    StatisticsComponent* parent() const;

    int index() const;
    int childCount() const;

    StatisticsComponent* addChild(const QString &text);
    void deleteAllChildren();

private:
    QString _text;
    QMap<QString, StatisticsComponent*> _children;
    StatisticsComponent *_parent;
};

StatisticsComponent::StatisticsComponent(const QString text, StatisticsComponent *parent) :
    _text(text),
    _parent(parent)
{
}

StatisticsComponent::~StatisticsComponent()
{
    qDeleteAll(_children);
}

QString StatisticsComponent::text() const
{
    return _text;
}

StatisticsComponent* StatisticsComponent::child(int index) const
{
    int i = 0;
    for (auto iter = _children.begin(); iter != _children.end(); ++iter, ++i) {
        // with QMap items are always sorted by key
        if (i == index) {
            return *iter;
        }
    }
    return NULL;
}

StatisticsComponent* StatisticsComponent::parent() const
{
    return _parent;
}

int StatisticsComponent::index() const
{
    if (_parent) {
        int i = 0;
        for (auto iter = _parent->_children.begin(); iter != _parent->_children.end(); ++i, ++iter) {
            if (this == *iter) {
                return i;
            }
        }
    }
    return 0;
}

int StatisticsComponent::childCount() const
{
    return _children.size();
}

StatisticsComponent* StatisticsComponent::addChild(const QString &text)
{
    StatisticsComponent *childComponent = _children.value(text);
    if (childComponent) {
        return childComponent;
    }

    return *_children.insert(text, new StatisticsComponent(text, this));
}

void StatisticsComponent::deleteAllChildren()
{
    qDeleteAll(_children);
    _children.clear();
}

///////////////////////////////////////////////////////////////////////////////

StatisticsProxyModel::StatisticsProxyModel(QObject *parent) :
    QAbstractItemModel(parent),
    _sourceModel(NULL),
    _rootComponent(new StatisticsComponent("root"))
{
}

StatisticsProxyModel::~StatisticsProxyModel()
{
    delete _rootComponent;
}

void StatisticsProxyModel::attachSourceModel(QAbstractItemModel *model)
{
    if (_sourceModel) {
        return;
    }
    _sourceModel = model;
    reset();

    connect(model, &QAbstractItemModel::rowsInserted, this, &StatisticsProxyModel::reset);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &StatisticsProxyModel::reset);
}

void StatisticsProxyModel::detachSourceModel()
{
    if (_sourceModel) {
        disconnect(_sourceModel, &QAbstractItemModel::rowsInserted, this, &StatisticsProxyModel::reset);
        disconnect(_sourceModel, &QAbstractItemModel::rowsRemoved, this, &StatisticsProxyModel::reset);
        _sourceModel = NULL;
    }
}

QAbstractItemModel* StatisticsProxyModel::sourceModel() const
{
    return _sourceModel;
}

QModelIndex	StatisticsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    StatisticsComponent *parentComponent;
    if (parent.isValid()) {
        parentComponent = static_cast<StatisticsComponent*>(parent.internalPointer());
    } else {
        parentComponent = _rootComponent;
    }

    StatisticsComponent *childComponent = parentComponent->child(row);
    if (childComponent) {
        return createIndex(row, column, childComponent);
    }
    return QModelIndex();
}

QModelIndex	StatisticsProxyModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    StatisticsComponent *childComponent = static_cast<StatisticsComponent*>(index.internalPointer());
    StatisticsComponent *parentComponent = childComponent->parent();
    if (parentComponent == _rootComponent) {
        return QModelIndex();
    }
    return createIndex(parentComponent->index(), 0, parentComponent);
}

int	StatisticsProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    StatisticsComponent *parentComponent;
    if (parent.isValid()) {
        parentComponent = static_cast<StatisticsComponent*>(parent.internalPointer());
    } else {
        parentComponent = _rootComponent;
    }
    return parentComponent->childCount();
}

int	StatisticsProxyModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant StatisticsProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }

    StatisticsComponent *component = static_cast<StatisticsComponent*>(index.internalPointer());
    return component->text();
}

void StatisticsProxyModel::addStatistics(const QString &text)
{
    QStringList strList = text.split('.', QString::SkipEmptyParts);
    StatisticsComponent *component = _rootComponent;
    for (const QString &str : strList) {
        component = component->addChild(str);
    }
}

void StatisticsProxyModel::reset()
{
    if (_sourceModel) {
        emit beginResetModel();
        _rootComponent->deleteAllChildren();
        for (int i = 0; i < _sourceModel->rowCount(); ++i) {
            QModelIndex index = _sourceModel->index(i, 0);
            addStatistics(_sourceModel->data(index).toString());
        }
        emit endResetModel();
    }
}
