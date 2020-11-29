#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "third_party/qcustomplot/qcustomplot.h"

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, const QString &module, const QString &name);

    void setShowModule(bool show);
    QString displayName() const;

    void enableScatter(bool enable);
    void enableSuspectFlag(bool enable);

    virtual bool addToLegend();

protected:
    virtual void draw(QCPPainter *painter);
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
    virtual void drawScatterPlot(QCPPainter *painter, QVector<QCPData> *scatterData) const;
    virtual QCPRange getValueRange(bool &foundRange, SignDomain inSignDomain=sdBoth) const;

public:
    static const double ScatterSize;

private:
    bool m_showModule;
    QString m_module;
    QString m_name;
    QCPScatterStyle m_ssSuspect; // scatter style for suspect (<suspect>true</suspect>) value

    friend class CounterLegendItem;
};

#endif // COUNTERGRAPH_H
