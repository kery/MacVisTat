#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "qcustomplot/qcustomplot.h"

struct CounterData
{
    CounterData();

    QSet<double> suspectKeys;
    QSharedPointer<QCPGraphDataContainer> data;
};

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    void setPen(const QPen &pen);
    void setData(QSharedPointer<QCPGraphDataContainer> data, const QSet<double> *suspectKeys);

    QString moduleName() const;
    void setModuleName(const QString &name);
    void setScatterVisible(bool visible);

    static const QChar nameSeparator;
    static QString getModuleName(const QString &fullName);
    static QPair<QString, QString> separateModuleName(const QString &fullName);

private:
    void getScatters(QVector<QPointF> *scatters, QVector<QPointF> *suspectScatters, const QCPDataRange &dataRange) const;

    virtual void draw(QCPPainter *painter) override;

    QString _moduleName;
    const QSet<double> *_suspectKeys;
    QCPScatterStyle _suspectScatterStyle;
};

#endif // COUNTERGRAPH_H
