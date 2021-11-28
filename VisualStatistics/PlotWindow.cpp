#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "DateTimeTicker.h"

PlotWindow::PlotWindow(PlotData &plotData) :
    ui(new Ui::PlotWindow),
    _plotData(std::move(plotData))
{
    ui->setupUi(this);
    setupPlot();

    connect(ui->actionRestore, &QAction::triggered, this, &PlotWindow::actionRestoreTriggered);
}

PlotWindow::~PlotWindow()
{
    delete ui;
}

void PlotWindow::actionSaveTriggered()
{
}

void PlotWindow::actionRestoreTriggered()
{
    ui->plot->rescaleAxes();
    adjustYAxisRange();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::skippedTicksChanged(int skipped)
{
    bool visible = skipped == 0;
    for (int i = 0, graphCount = ui->plot->graphCount(); i < graphCount; ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph*>(ui->plot->graph(i));
        graph->setScatterVisible(visible);
    }
}

void PlotWindow::setupPlot()
{
    // QCPAxisTicker isn't derived from QObject, so we use qSharedPointerDynamicCast here.
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    ticker->setOffsetFromUtc(_plotData.offsetFromUtc());
    connect(ticker.data(), &DateTimeTicker::skippedTicksChanged, this, &PlotWindow::skippedTicksChanged);

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
    ui->plot->replot();
}

void PlotWindow::adjustYAxisRange()
{
    QCPRange range = ui->plot->yAxis->range();
    double delta = range.size() * 0.01;
    range.lower -= delta;
    range.upper += delta;
    ui->plot->yAxis->setRange(range);
}
