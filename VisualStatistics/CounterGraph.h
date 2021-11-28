#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "qcustomplot/qcustomplot.h"

struct CounterData
{
    CounterData();

    QVector<double> suspectKeys;
    QSharedPointer<QCPGraphDataContainer> data;
};

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    QString moduleName() const;
    void setModuleName(const QString &name);
    void setSuspectKeys(QVector<double> *suspectKeys);

    static const QChar nameSeparator;
    static QString getModuleName(const QString &fullName);
    static QPair<QString, QString> separateModuleName(const QString &fullName);

private:
    QString _moduleName;
    QVector<double> *_suspectKeys;
};

#endif // COUNTERGRAPH_H
