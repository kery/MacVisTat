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

void ValueText::setValueInfo(const QString &name, const QString &dt, const QString &value, bool suspectFlag)
{
    m_graphName = name;
    m_dateTime = dt;
    m_graphValue = value;
    m_suspectFlag = suspectFlag;

    QString text = m_graphName;
    text += '\n';
    text += m_dateTime;
    text += '\n';
    text += m_graphValue;
    if (m_suspectFlag) {
        text += " (suspect)";
    }

    setText(text);
}

QString ValueText::graphName() const
{
    return m_graphName;
}

QString ValueText::graphValue() const
{
    return m_graphValue;
}

void ValueText::draw(QCPPainter *painter)
{
    QPointF coords(PosOffset, PosOffset);
    QPointF tracerPos = position->parentAnchor()->pixelPoint();
    double width = right->pixelPoint().x() - left->pixelPoint().x();
    double height = bottom->pixelPoint().y() - top->pixelPoint().y();

    double widthOutside = (tracerPos.x() + PosOffset + width) - clipRect().right();
    if (widthOutside >= 0) {
        coords.rx() -= widthOutside + 1;
    }

    double heightOutside = (tracerPos.y() + PosOffset + height) - clipRect().bottom();
    if (heightOutside >= 0) {
        widthOutside >= 0 ? coords.ry() = -PosOffset - height : coords.ry() -= heightOutside + 2;
    }

    position->setCoords(coords);

    QCPItemText::draw(painter);
}
