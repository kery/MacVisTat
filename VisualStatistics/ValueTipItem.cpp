#include "ValueTipItem.h"
#include "CounterGraph.h"
#include "GlobalDefines.h"
#include "QCustomPlot/src/items/item-tracer.h"

#define TRACER_SIZE 10.0

const QString ValueTipItem::sLayerName("valuetip");

ValueTipItem::ValueTipItem(QCustomPlot *plot) :
    TextItem(plot),
    mTracer(new QCPItemTracer(plot))
{
    setVisible(false);

    mTracer->setLayer(sLayerName);
    mTracer->setStyle(QCPItemTracer::tsCircle);

    setLayer(sLayerName);
    setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
    position->setParentAnchor(mTracer->position);

    mAnimation.setTargetObject(mTracer);
    mAnimation.setPropertyName("size");
    mAnimation.setDuration(250);
    mAnimation.setStartValue(0);
    mAnimation.setEndValue(TRACER_SIZE);
    mAnimation.setEasingCurve(QEasingCurve::OutQuad);

    connect(&mAnimation, &QPropertyAnimation::valueChanged, this, &ValueTipItem::animationValueChange);
    connect(&mAnimation, &QPropertyAnimation::finished, this, &ValueTipItem::animationFinished);
}

QString ValueTipItem::graphName() const
{
    return mGraphName;
}

QString ValueTipItem::graphValue() const
{
    return mGraphValue;
}

void ValueTipItem::setSelected(bool selected)
{
    mTracer->setSelected(selected);
}

CounterGraph * ValueTipItem::tracerGraph() const
{
    return qobject_cast<CounterGraph *>(mTracer->graph());
}

QCPItemPosition * ValueTipItem::tracerPosition() const
{
    return mTracer->position;
}

void ValueTipItem::setTracerGraph(CounterGraph *graph)
{
    CounterGraph *preGraph = qobject_cast<CounterGraph *>(mTracer->graph());
    if (preGraph != nullptr) {
        disconnect(mTracer, &QCPItemTracer::selectionChanged, preGraph, &CounterGraph::setSelected);
        disconnect(preGraph, QOverload<bool>::of(&CounterGraph::selectionChanged), mTracer, &QCPItemTracer::setSelected);
    }

    mTracer->setGraph(graph);
    if (graph != nullptr) {
        connect(mTracer, &QCPItemTracer::selectionChanged, graph, &CounterGraph::setSelected);
        connect(graph, QOverload<bool>::of(&CounterGraph::selectionChanged), mTracer, &QCPItemTracer::setSelected);
    }
}

double ValueTipItem::tracerGraphKey() const
{
    return mTracer->graphKey();
}

void ValueTipItem::setTracerGraphKey(double key)
{
    mTracer->setGraphKey(key);
}

void ValueTipItem::setTracerPen(const QPen &pen)
{
    QColor color = pen.color();
    QPen borderPen(color.darker());
    borderPen.setWidth(2);

    mTracer->setPen(borderPen);
    mTracer->setSelectedPen(borderPen);
    mTracer->setBrush(color);
    mTracer->setSelectedBrush(color);
}

void ValueTipItem::setValueInfo(const QString &name, const QDateTime &dateTime, const QString &value, bool suspect)
{
    mGraphName = name;
    mGraphValue = value;

    QString text = name;
    text += '\n';
    text += dateTime.toString(DTFMT_DISPLAY);
    if (dateTime.timeSpec() == Qt::UTC) {
        text += " (UTC)";
    }
    text += '\n';
    text += value;
    if (suspect) {
        text += " (suspect)";
    }
    setText(text);
}

void ValueTipItem::showWithAnimation()
{
    setVisible(true);
    if (mAnimation.state() == QAbstractAnimation::Running) {
        mAnimation.stop();
    }
    mAnimation.setDirection(QAbstractAnimation::Forward);
    mAnimation.start();
}

void ValueTipItem::hideWithAnimation()
{
    if (mAnimation.state() != QAbstractAnimation::Running) {
        mAnimation.setDirection(QAbstractAnimation::Backward);
        mAnimation.start();
    }
}

void ValueTipItem::setVisible(bool on)
{
    QCPItemText::setVisible(on);
    mTracer->setVisible(on);
}

void ValueTipItem::animationValueChange(const QVariant &/*value*/)
{
    mLayer->replot();
}

void ValueTipItem::animationFinished()
{
    if (mAnimation.direction() == QAbstractAnimation::Backward) {
        setVisible(false);
        setTracerGraph(nullptr);
        mLayer->replot();
    }
}

void ValueTipItem::draw(QCPPainter *painter)
{
    double offset = TRACER_SIZE / 2 + 1;
    QPointF coords(offset, offset);
    QPointF tracerPos = mTracer->position->pixelPosition();
    double width = right->pixelPosition().x() - left->pixelPosition().x();
    double height = bottom->pixelPosition().y() - top->pixelPosition().y();

    double widthOutside = (tracerPos.x() + offset + width) - clipRect().right();
    if (widthOutside > 0) {
        coords.rx() -= widthOutside;
    }
    double heightOutside = (tracerPos.y() + offset + height) - clipRect().bottom();
    if (heightOutside > 0) {
        widthOutside > 0 ? coords.ry() = -offset - height : coords.ry() -= heightOutside;
    }
    position->setCoords(coords);

    TextItem::draw(painter);
}
