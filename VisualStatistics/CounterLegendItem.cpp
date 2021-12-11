#include "CounterLegendItem.h"
#include "CounterGraph.h"

CounterLegendItem::CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) :
    QCPPlottableLegendItem(parent, plottable)
{
}

QSize CounterLegendItem::minimumOuterSizeHint() const
{
    QSize result(0, 0);
    QFontMetrics fontMetrics(getFont());
    QSize iconSize = mParentLegend->iconSize();
    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, iconSize.height(), Qt::TextSingleLine | Qt::AlignVCenter | Qt::TextDontClip,
                                              graph->displayName());
    result.setWidth(iconSize.width() + mParentLegend->iconTextPadding() + textRect.width());
    result.setHeight(qMax(textRect.height(), iconSize.height()));
    result.rwidth() += mMargins.left() + mMargins.right();
    result.rheight() += mMargins.top() + mMargins.bottom();
    return result;
}

void CounterLegendItem::draw(QCPPainter *painter)
{
    painter->setFont(getFont());
    painter->setPen(QPen(getTextColor()));
    QSize iconSize = mParentLegend->iconSize();
    QRect textRect = mRect.adjusted(iconSize.width() + mParentLegend->iconTextPadding(), 0, 0, 0);
    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);
    painter->drawText(textRect, Qt::TextSingleLine | Qt::AlignVCenter | Qt::TextDontClip, graph->displayName());
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
