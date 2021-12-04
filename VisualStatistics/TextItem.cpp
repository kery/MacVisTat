#include "TextItem.h"

TextItem::TextItem(QCustomPlot *plot) :
    QCPItemText(plot)
{
    QLinearGradient lg(0, 0, 0, 1);
    lg.setCoordinateMode(QGradient::ObjectBoundingMode);
    lg.setColorAt(0, Qt::white);
    lg.setColorAt(1, QColor(250, 250, 250));

    setPen(QPen(QColor(160, 160, 160)));
    setBrush(lg);
    setPadding(QMargins(6, 4, 6, 4));
    setTextAlignment(Qt::AlignLeft);
    setAntialiased(false);
    setSelectable(false);
}

void TextItem::draw(QCPPainter *painter)
{
    QPointF pos(position->pixelPosition());
    QTransform transform = painter->transform();
    transform.translate(pos.x(), pos.y());
    if (!qFuzzyIsNull(mRotation)) {
        transform.rotate(mRotation);
    }
    painter->setFont(mainFont());
    QRect textRect = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
    QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
    QPointF textPos = getTextDrawPoint(QPointF(0, 0), textBoxRect, mPositionAlignment); // 0, 0 because the transform does the translation
    textRect.moveTopLeft(textPos.toPoint() + QPoint(mPadding.left(), mPadding.top()));
    textBoxRect.moveTopLeft(textPos.toPoint());
    int clipPad = qCeil(mainPen().widthF());
    QRect boundingRect = textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
    if (transform.mapRect(boundingRect).intersects(painter->transform().mapRect(clipRect()))) {
        painter->setTransform(transform);
        if ((mainBrush().style() != Qt::NoBrush && mainBrush().color().alpha() != 0) ||
            (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0))
        {
            painter->setPen(mainPen());
            painter->setBrush(mainBrush());
            painter->drawRoundedRect(textBoxRect, 2, 2);
        }
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(mainColor()));
        painter->drawText(textRect, Qt::TextDontClip | mTextAlignment, mText);
    }
}
