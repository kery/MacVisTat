#include "valuetext.h"

ValueText::ValueText(const QCPItemTracer *tracer) :
    QCPItemText(tracer->parentPlot())
{
    setPen(QPen(QColor(118, 118, 118)));
    setBrush(QBrush(Qt::white));
    setPadding(QMargins(4, 2, 4, 2));
    setLayer(tracer->layer());
    setTextAlignment(Qt::AlignLeft);
    setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);

    position->setParentAnchor(tracer->position);
}

void ValueText::draw(QCPPainter *painter)
{
    const double OFFSET = 5;

    QPointF coords(OFFSET, OFFSET);
    QPointF tracerPos = position->parentAnchor()->pixelPoint();
    double width = right->pixelPoint().x() - left->pixelPoint().x();
    double height = bottom->pixelPoint().y() - top->pixelPoint().y();

    double clippedX = tracerPos.x() + OFFSET + width - clipRect().right();
    if (clippedX > 0) {
        coords.rx() = OFFSET - clippedX - 3;
    }

    double clippedY = tracerPos.y() + OFFSET + height - clipRect().bottom();
    if (clippedY > 0) {
        coords.ry() = OFFSET - clippedY - 3;
    }

    position->setCoords(coords);

    QCPItemText::draw(painter);
}
