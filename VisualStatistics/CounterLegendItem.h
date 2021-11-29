#ifndef COUNTERLEGENDITEM_H
#define COUNTERLEGENDITEM_H

#include "qcustomplot/qcustomplot.h"

class CounterLegendItem : public QCPPlottableLegendItem
{
    Q_OBJECT

public:
    CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable);

protected:
    virtual void draw(QCPPainter *painter) override;
};

#endif // COUNTERLEGENDITEM_H
