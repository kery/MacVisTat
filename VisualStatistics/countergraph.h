#ifndef COUNTERGRAPH_H
#define COUNTERGRAPH_H

#include "third_party/qcustomplot/qcustomplot.h"

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    void setSuspectFlagScatterStyle(const QCPScatterStyle &ssSuspect);

protected:
    virtual void draw(QCPPainter *painter);
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
    virtual void drawScatterPlot(QCPPainter *painter, QVector<QCPData> *scatterData) const;

private:
    QCPScatterStyle m_ssSuspect; // scatter style for suspect (<suspect>true</suspect>) value
};

#endif // COUNTERGRAPH_H
