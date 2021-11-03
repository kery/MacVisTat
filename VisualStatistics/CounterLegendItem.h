#ifndef COUNTERLEGENDITEM_H
#define COUNTERLEGENDITEM_H

#include <qcustomplot.h>

class CounterLegendItem : public QCPPlottableLegendItem
{
    Q_OBJECT

public:
    CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable);

protected:
    virtual QSize minimumSizeHint() const;
    virtual void draw(QCPPainter *painter);
};

#endif // COUNTERLEGENDITEM_H
