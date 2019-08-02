#include "countergraph.h"

CounterGraph::CounterGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPGraph(keyAxis, valueAxis)
{
}

void CounterGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
    QSizeF iconSize = parentPlot()->legend->iconSize();
    QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, name());
    if (textRect.height() > iconSize.height()) {
        const_cast<QRectF &>(rect).translate(0, ((textRect.height() - iconSize.height()) / 2 + 1));
        painter->setClipRect(rect);
        painter->fillRect(rect, mPen.color());
    } else {
        painter->fillRect(rect, mPen.color());
    }
}
