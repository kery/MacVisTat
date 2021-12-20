#include "CounterLegendItem.h"
#include "CounterGraph.h"

CounterLegendItem::CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) :
    QCPPlottableLegendItem(parent, plottable)
{
}

QSize CounterLegendItem::minimumOuterSizeHint() const
{
    QFontMetrics fontMetrics(getFont());
    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);
    QSize result = fontMetrics.size(Qt::TextSingleLine | Qt::AlignVCenter, graph->displayName());
    QSize iconSize = mParentLegend->iconSize();

    result.rwidth() += iconSize.width() + mParentLegend->iconTextPadding() + mMargins.left() + mMargins.right();
    if (iconSize.height() > result.height()) { result.setHeight(iconSize.height()); }
    result.rheight() += mMargins.top() + mMargins.bottom();
    return result;
}

void CounterLegendItem::draw(QCPPainter *painter)
{
    painter->setFont(getFont());
    painter->setPen(QPen(getTextColor()));
    painter->setClipping(false);

    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);
    QSize iconSize = mParentLegend->iconSize();
    QRect iconRect(QPoint(0, 0), iconSize);
    QRect textRect = mRect.adjusted(iconSize.width() + mParentLegend->iconTextPadding(), 0, 0, 0);
    painter->drawText(textRect, Qt::TextSingleLine | Qt::AlignVCenter, graph->displayName());
    iconRect.moveCenter(mRect.center());
    iconRect.moveLeft(mRect.left());
    graph->drawLegendIcon(painter, iconRect);
    if (getIconBorderPen().style() != Qt::NoPen) {
        iconRect.adjust(-2, -2, 2, 2);
        painter->setPen(getIconBorderPen());
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(iconRect);
    }
}
