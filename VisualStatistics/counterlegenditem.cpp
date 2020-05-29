#include "counterlegenditem.h"
#include "countergraph.h"

CounterLegendItem::CounterLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) :
    QCPPlottableLegendItem(parent, plottable)
{
}

QSize CounterLegendItem::minimumSizeHint() const
{
    if (!mPlottable) return QSize();

    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);

    QSize result(0, 0);
    QRect textRect;
    QFontMetrics fontMetrics(getFont());
    QSize iconSize = mParentLegend->iconSize();
    textRect = fontMetrics.boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, graph->realDisplayName());
    result.setWidth(iconSize.width() + mParentLegend->iconTextPadding() + textRect.width() + mMargins.left() + mMargins.right());
    result.setHeight(qMax(textRect.height(), iconSize.height()) + mMargins.top() + mMargins.bottom());
    return result;
}

void CounterLegendItem::draw(QCPPainter *painter)
{
    if (!mPlottable) return;

    CounterGraph *graph = qobject_cast<CounterGraph *>(mPlottable);
    QString text(graph->realDisplayName());

    painter->setFont(getFont());
    painter->setPen(QPen(getTextColor()));
    QSizeF iconSize = mParentLegend->iconSize();
    QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, text);
    QRectF iconRect(mRect.topLeft(), iconSize);
    int textHeight = qMax(textRect.height(), iconSize.height());  // if text has smaller height than icon, center text vertically in icon height, else align tops
    painter->drawText(mRect.x()+iconSize.width()+mParentLegend->iconTextPadding(), mRect.y(), textRect.width(), textHeight, Qt::TextDontClip, text);
    // draw icon:
    painter->save();
    painter->setClipRect(iconRect, Qt::IntersectClip);
    graph->drawLegendIcon(painter, iconRect);
    painter->restore();
    // draw icon border:
    if (getIconBorderPen().style() != Qt::NoPen)
    {
      painter->setPen(getIconBorderPen());
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(iconRect);
    }
}
