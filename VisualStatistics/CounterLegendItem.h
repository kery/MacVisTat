#ifndef COUNTERLEGENDITEM_H
#define COUNTERLEGENDITEM_H

#include "QCustomPlot/src/layoutelements/layoutelement-legend.h"

class CounterLegendItem : public QCPPlottableLegendItem
{
    Q_OBJECT

public:
    CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable);

private:
    virtual QSize minimumOuterSizeHint() const override;
    virtual void draw(QCPPainter *painter) override;
};

#endif // COUNTERLEGENDITEM_H
