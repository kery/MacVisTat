#pragma once

#include "third_party/qcustomplot/qcustomplot.h"

class CounterGraph : public QCPGraph
{
    Q_OBJECT

public:
    CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

protected:
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
};
