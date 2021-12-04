#include "CounterPlot.h"
#include "CounterGraph.h"
#include "CounterLegendItem.h"
#include "DateTimeTicker.h"

CounterPlot::CounterPlot(QWidget *parent) :
    QCustomPlot(parent)
{
}

CounterPlot::~CounterPlot()
{
    clearGraphs();
}

CounterGraph *CounterPlot::addGraph()
{
    CounterGraph *graph = new CounterGraph(xAxis, yAxis);
    CounterLegendItem *item = new CounterLegendItem(legend, graph);
    legend->addItem(item);
    connect(item, &QCPPlottableLegendItem::selectionChanged, graph, &CounterGraph::setSelected);
    connect(graph, QOverload<bool>::of(&CounterGraph::selectionChanged), item, &QCPPlottableLegendItem::setSelected);
    return graph;
}

CounterGraph *CounterPlot::graph(int index) const
{
    return qobject_cast<CounterGraph*>(QCustomPlot::graph(index));
}

// The call of setFillOrder in QCPLegend::removeItem takes a lot of time, to prevent it from been called
// we override some of the related methods.
bool CounterPlot::removeGraph(CounterGraph *graph)
{
    if (!mGraphs.contains(graph)) {
        return false;
    }
    if (QCPPlottableLegendItem *lip = legend->itemWithPlottable(graph)) {
        if (legend->remove(lip)) {
            legend->simplify(); // Call simplify instead of setFillOrder to save time.
        }
    }
    mGraphs.removeOne(graph);
    delete graph;
    mPlottables.removeOne(graph);
    return true;
}

bool CounterPlot::removeGraph(int index)
{
    if (CounterGraph *g = graph(index)) {
        return removeGraph(g);
    }
    return false;
}

int CounterPlot::clearGraphs()
{
    int c = mGraphs.size();
    for (int i = c - 1; i >= 0; --i)
      removeGraph(i);
    return c;
}

bool CounterPlot::hasSelectedGraphs() const
{
    for (int i = 0; i < graphCount(); ++i) {
        if (graph(i)->selected()) {
            return true;
        }
    }
    return false;
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

    if (legend->rect().contains(event->pos())) {
        const int step = 25;
        QPoint delta = event->angleDelta();
        QColor color = legend->brush().color();
        if (delta.y() < 0) {
            color.setAlpha(qMax(0, color.alpha() - step));
        } else {
            color.setAlpha(qMin(color.alpha() + step, 255));
        }
        legend->setBrush(QBrush(color));
        replot(QCustomPlot::rpImmediateRefresh);
    } else {
        QCustomPlot::wheelEvent(event);
    }
}
