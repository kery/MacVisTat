#include "CounterLegendItem.h"
#include "CounterGraph.h"

CounterLegendItem::CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) :
    QCPPlottableLegendItem(parent, plottable)
{
}

void CounterLegendItem::draw(QCPPainter *painter)
{
    CounterGraph *graph = qobject_cast<CounterGraph*>(mPlottable);
    if (graph == nullptr) {
        QCPPlottableLegendItem::draw(painter);
        return;
    }
    painter->setFont(getFont());
    painter->setPen(QPen(getTextColor()));
    QSize iconSize = mParentLegend->iconSize();
    QRect textRect = mRect.adjusted(iconSize.width() + mParentLegend->iconTextPadding(), 0, 0, 0);
    painter->drawText(textRect, Qt::TextSingleLine | Qt::AlignVCenter, graph->name());
    // draw icon:
    QRect iconRect(QPoint(0, 0), iconSize);
    iconRect.moveCenter(mRect.center());
    iconRect.moveLeft(mRect.left());
    graph->drawLegendIcon(painter, iconRect);
    // draw icon border:
    if (getIconBorderPen().style() != Qt::NoPen)
    {
        iconRect.adjust(-2, -2, 2, 2);
        painter->setClipping(false);
        painter->setPen(getIconBorderPen());
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(iconRect);
    }
}
