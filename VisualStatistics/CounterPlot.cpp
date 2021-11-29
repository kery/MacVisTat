#include "CounterPlot.h"
#include "CounterGraph.h"
#include "DateTimeTicker.h"

CounterPlot::CounterPlot(QWidget *parent) :
    QCustomPlot(parent)
{
    setupSelectionRect();

    axisRect()->setupFullAxesBox();
    xAxis2->setTicks(false);
    xAxis2->setTickLabels(false);
    yAxis2->setTicks(false);
    yAxis2->setTickLabels(false);
    xAxis->setTicker(QSharedPointer<DateTimeTicker>(new DateTimeTicker(xAxis)));

    QColor color = legend->brush().color();
    color.setAlpha(200);
    legend->setIconSize(20, 10);
    legend->setBrush(QBrush(color));
    legend->setSelectableParts(QCPLegend::spItems);
    legend->setVisible(true);

    setNoAntialiasingOnDrag(true);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect | QCP::iSelectPlottables |
                    QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectItems);
}

CounterGraph *CounterPlot::addGraph()
{
    CounterGraph *graph = new CounterGraph(xAxis, yAxis);
    QCPPlottableLegendItem *item = legend->itemWithPlottable(graph);
    connect(item, &QCPPlottableLegendItem::selectionChanged, graph, &CounterGraph::setSelected);
    connect(graph, QOverload<bool>::of(&CounterGraph::selectionChanged), item, &QCPPlottableLegendItem::setSelected);
    return graph;
}

CounterGraph *CounterPlot::graph(int index)
{
    return qobject_cast<CounterGraph*>(QCustomPlot::graph(index));
}

QList<CounterGraph*> CounterPlot::selectedGraphs() const
{
    QList<CounterGraph*> result;
    const QList<QCPGraph*> graphs = QCustomPlot::selectedGraphs();
    for (QCPGraph *graph : graphs) {
        result.append(qobject_cast<CounterGraph*>(graph));
    }
    return result;
}

void CounterPlot::setupSelectionRect()
{
    QColor color(70, 50, 200);
    QPen pen(Qt::DashLine);
    pen.setColor(color);
    mSelectionRect->setPen(pen);
    color.setAlpha(30);
    mSelectionRect->setBrush(color);
}

void CounterPlot::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    setViewport(rect());
    // DateTimeTicker's createTickVector method depends on geometry of the axis rect, so call
    // updateLayout to update it.
    updateLayout();
    replot(rpQueuedRefresh);
}

void CounterPlot::mousePressEvent(QMouseEvent *event)
{
    Qt::Orientations rangeDrag;
    if (xAxis->selectedParts() != QCPAxis::spNone || xAxis2->selectedParts() != QCPAxis::spNone) {
        rangeDrag |= Qt::Horizontal;
    }
    if (yAxis->selectedParts() != QCPAxis::spNone || yAxis2->selectedParts() != QCPAxis::spNone) {
        rangeDrag |= Qt::Vertical;
    }
    if (!rangeDrag) {
        rangeDrag = Qt::Horizontal | Qt::Vertical;
    }
    axisRect()->setRangeDrag(rangeDrag);

    setSelectionRectMode(event->modifiers() & Qt::ControlModifier ? QCP::srmZoom : QCP::srmNone);

    // TODO: adjust transperancy of legend.

    QCustomPlot::mousePressEvent(event);
}

void CounterPlot::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        axisRect()->setRangeZoom(Qt::Horizontal);
    } else if (event->modifiers() & Qt::ShiftModifier) {
        axisRect()->setRangeZoom(Qt::Vertical);
    } else {
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }
    QCustomPlot::wheelEvent(event);
}
