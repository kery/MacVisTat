#include "ValueText.h"
#include "PlotWindow.h"

const double ValueText::PosOffset = PlotWindow::TracerSize/2 + 1;

ValueText::ValueText(const QCPItemTracer *tracer) :
    QCPItemText(tracer->parentPlot())
{
    setPen(QPen(QColor(118, 118, 118)));
    setBrush(QBrush(Qt::white));
    setPadding(QMargins(4, 2, 4, 2));
    setLayer(tracer->layer());
    setTextAlignment(Qt::AlignLeft);
    setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
    setAntialiased(false);
    setVisible(false);
    setSelectable(false);

    position->setParentAnchor(tracer->position);
}

void ValueText::setGraphName(const QString &name)
{
    m_graphName = name;
}

void ValueText::setDateTime(const QString &dt)
{
    m_dateTime = dt;
}

void ValueText::setGraphValue(const QString &value)
{
    m_graphValue = value;
}

QString ValueText::graphName() const
{
    return m_graphName;
}

QString ValueText::graphValue() const
{
    return m_graphValue;
}

void ValueText::updateText()
{
    QString text = m_graphName;
    text += '\n';
    text += m_dateTime;
    text += '\n';
    text += m_graphValue;

    setText(text);
}

void ValueText::draw(QCPPainter *painter)
{
    QPointF coords(PosOffset, PosOffset);
    QPointF tracerPos = position->parentAnchor()->pixelPoint();
    double width = right->pixelPoint().x() - left->pixelPoint().x();
    double height = bottom->pixelPoint().y() - top->pixelPoint().y();

    double widthOutside = (tracerPos.x() + PosOffset + width) - clipRect().right();
    if (widthOutside > 0) {
        coords.rx() -= widthOutside;
    }

    double heightOutside = (tracerPos.y() + PosOffset + height) - clipRect().bottom();
    if (heightOutside > 0) {
        widthOutside > 0 ? coords.ry() = -PosOffset - height : coords.ry() -= heightOutside;
    }

    position->setCoords(coords);

    QCPItemText::draw(painter);
}