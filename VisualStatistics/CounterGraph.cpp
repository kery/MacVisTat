#include "CounterGraph.h"

CounterData::CounterData() :
    data(new QCPGraphDataContainer())
{
}

const QChar CounterGraph::nameSeparator(',');

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPGraph(keyAxis, valueAxis),
    _suspectKeys(nullptr)
{
}

QString CounterGraph::moduleName() const
{
    return _moduleName;
}

void CounterGraph::setModuleName(const QString &name)
{
    _moduleName = name;
}

void CounterGraph::setSuspectKeys(QVector<double> *suspectKeys)
{
    _suspectKeys = suspectKeys;
}

QString CounterGraph::getModuleName(const QString &fullName)
{
    int index = fullName.indexOf(nameSeparator);
    if (index > 0) {
        return fullName.left(index);
    }
    return QString();
}

QPair<QString, QString> CounterGraph::separateModuleName(const QString &fullName)
{
    QPair<QString, QString> result;
    int index = fullName.indexOf(nameSeparator);
    if (index > 0) {
        result.first = fullName.left(index);
        result.second = fullName.mid(index + 1);
    } else {
        result.second = fullName;
    }
    return result;
}
