#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "DateTimeTicker.h"

PlotWindow::PlotWindow(PlotData &plotData) :
    ui(new Ui::PlotWindow),
    _plotData(std::move(plotData)),
    _lastSelLegItemIndex(-1)
{
    ui->setupUi(this);

    // QCPAxisTicker isn't derived from QObject, so we use qSharedPointerDynamicCast here.
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    ticker->setOffsetFromUtc(_plotData.offsetFromUtc());

    connect(ui->actionSave, &QAction::triggered, this, &PlotWindow::actionSaveTriggered);
    connect(ui->actionCopy, &QAction::triggered, this, &PlotWindow::actionCopyTriggered);
    connect(ui->actionRestore, &QAction::triggered, this, &PlotWindow::actionRestoreTriggered);
    connect(ui->actionShowDelta, &QAction::triggered, this, &PlotWindow::actionShowDeltaTriggered);
    connect(ui->actionDisplayUtc, &QAction::triggered, this, &PlotWindow::actionDisplayUtcTriggered);
    connect(ui->actionRemoveZeroCounters, &QAction::triggered, this, &PlotWindow::actionRemoveZeroCountersTriggered);
    connect(ui->actionScript, &QAction::triggered, this, &PlotWindow::actionScriptTriggered);
    connect(ui->plot, &CounterPlot::selectionChangedByUser, this, &PlotWindow::selectionChanged);
    connect(ticker.data(), &DateTimeTicker::skippedTicksChanged, this, &PlotWindow::skippedTicksChanged);

    initGraphs();
}

PlotWindow::~PlotWindow()
{
    delete ui;
}

void PlotWindow::actionSaveTriggered()
{
}

void PlotWindow::actionCopyTriggered()
{
}

void PlotWindow::actionRestoreTriggered()
{
    ui->plot->rescaleAxes();
    adjustYAxisRange();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionShowDeltaTriggered(bool checked)
{
}

void PlotWindow::actionDisplayUtcTriggered(bool checked)
{
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    ticker->setDisplayUtc(checked);
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionRemoveZeroCountersTriggered()
{
}

void PlotWindow::actionScriptTriggered()
{
}

void PlotWindow::selectionChanged()
{
    auto selectedLegendItems = ui->plot->legend->selectedItems();
    if (selectedLegendItems.size() == 1) {
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && _lastSelLegItemIndex >= 0) {
            int curSelLegItemIndex = legendItemIndex(selectedLegendItems.first());
            int min = 0, max = 0;
            if (curSelLegItemIndex < _lastSelLegItemIndex) {
                min = curSelLegItemIndex + 1;
                max = qMin(_lastSelLegItemIndex + 1, ui->plot->graphCount());
            } else if (curSelLegItemIndex > _lastSelLegItemIndex) {
                min = _lastSelLegItemIndex;
                max = curSelLegItemIndex;
            }
            for (int i = min; i < max; ++i) {
                ui->plot->graph(i)->setSelected(true);
            }
        } else {
            _lastSelLegItemIndex = legendItemIndex(selectedLegendItems.first());
        }
    }

    if (ui->plot->legend->selectedItems().isEmpty()) {
        _lastSelLegItemIndex = -1;
        for (int i = 0; i < ui->plot->graphCount(); ++i) {
            CounterGraph *graph = ui->plot->graph(i);
            graph->setVisible(true);

//            const QVector<CommentText *> cmtVec = commentsOfGraph(graph);
//            for (CommentText *cmtText : cmtVec) {
//                cmtText->setVisible(true);
//            }
        }
    } else {
        for (int i = 0; i < ui->plot->graphCount(); ++i) {
            CounterGraph *graph = ui->plot->graph(i);
            graph->setVisible(graph->selected());

//            const QVector<CommentText *> cmtVec = commentsOfGraph(graph);
//            for (CommentText *cmtText : cmtVec) {
//                cmtText->setVisible(graph->selected());
//            }
        }

        if (QApplication::keyboardModifiers() & Qt::AltModifier) {
            ui->plot->yAxis->rescale(true);
            adjustYAxisRange();
        }
    }

//    if ((m_tracer->visible() || m_valueText->visible()) &&
//            m_tracer->graph() != nullptr && !m_tracer->graph()->visible())
//    {
//        m_tracer->setVisible(false);
//        m_valueText->setVisible(false);
//        setTracerGraph(nullptr);
//    }
}

void PlotWindow::skippedTicksChanged(int skipped)
{
    bool visible = skipped == 0;
    for (int i = 0, graphCount = ui->plot->graphCount(); i < graphCount; ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph*>(ui->plot->graph(i));
        graph->setScatterVisible(visible);
    }
}

void PlotWindow::initGraphs()
{
    const QList<QString> counterNames = _plotData.counterNames();
    for (const QString &name : counterNames) {
        auto pair = CounterGraph::separateModuleName(name);
        CounterGraph *graph = ui->plot->addGraph();
        graph->setPen(QPen(_colorPool.getColor()));
        graph->setModuleName(pair.first);
        graph->setName(pair.second);
        graph->setData(_plotData.graphData(name), _plotData.suspectKeys(name));
    }

    ui->plot->rescaleAxes();
    adjustYAxisRange();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::adjustYAxisRange()
{
    QCPRange range = ui->plot->yAxis->range();
    double delta = range.size() * 0.01;
    range.lower -= delta;
    range.upper += delta;
    ui->plot->yAxis->setRange(range);
}

int PlotWindow::legendItemIndex(QCPAbstractLegendItem *item) const
{
    for (int i = 0; i < ui->plot->legend->itemCount(); ++i) {
        if (item == ui->plot->legend->item(i)) {
            return i;
        }
    }
    return -1;
}

int PlotWindow::graphIndex(CounterGraph *graph) const
{
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        if (graph == ui->plot->graph(i)) {
            return i;
        }
    }
    return -1;
}

CounterGraph *PlotWindow::prevGraph(CounterGraph *graph) const
{
    int index = graphIndex(graph);
    return index <= 0 ? nullptr : ui->plot->graph(index - 1);
}

CounterGraph *PlotWindow::nextGraph(CounterGraph *graph) const
{
    int nextIndex = graphIndex(graph) + 1;
    return nextIndex == 0 || nextIndex >= ui->plot->graphCount() ? nullptr : ui->plot->graph(nextIndex);
}

void PlotWindow::keyPressEvent(QKeyEvent *event)
{
    QList<CounterGraph*> selectedGraphs = ui->plot->selectedGraphs();
    if (selectedGraphs.size() != 1 || (event->key() != Qt::Key_Up && event->key() != Qt::Key_Down)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    CounterGraph *selGraph = event->key() == Qt::Key_Down ? nextGraph(selectedGraphs.first()) : prevGraph(selectedGraphs.first());
    if (selGraph) {
        selectedGraphs.first()->setSelected(false);
        selGraph->setSelected(true);
        selectionChanged();
        ui->plot->replot(QCustomPlot::rpQueuedReplot);
    }
}
