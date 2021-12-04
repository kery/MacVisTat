#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "DateTimeTicker.h"
#include "ValueTipItem.h"
#include "Utils.h"

PlotWindow::PlotWindow(PlotData &plotData) :
    ui(new Ui::PlotWindow),
    mValueTip(nullptr),
    mPlotData(std::move(plotData)),
    mLastSelLegItemIndex(-1)
{
    ui->setupUi(this);
    setupPlot();

    mValueTip = new ValueTipItem(ui->plot);

    connect(ui->actionSave, &QAction::triggered, this, &PlotWindow::actionSaveTriggered);
    connect(ui->actionCopy, &QAction::triggered, this, &PlotWindow::actionCopyTriggered);
    connect(ui->actionRestore, &QAction::triggered, this, &PlotWindow::actionRestoreTriggered);
    connect(ui->actionShowDelta, &QAction::triggered, this, &PlotWindow::actionShowDeltaTriggered);
    connect(ui->actionDisplayUtc, &QAction::triggered, this, &PlotWindow::actionDisplayUtcTriggered);
    connect(ui->actionRemoveZeroCounters, &QAction::triggered, this, &PlotWindow::actionRemoveZeroCountersTriggered);
    connect(ui->actionScript, &QAction::triggered, this, &PlotWindow::actionScriptTriggered);

    setFocus();
    initGraphs();
    highlightTimeGap();
    updateWindowTitle();
    updatePlotTitle();
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
    QApplication::clipboard()->setPixmap(ui->plot->toPixmap());
}

void PlotWindow::actionRestoreTriggered()
{
    ui->plot->rescaleAxes();
    adjustYAxisRange();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionShowDeltaTriggered(bool checked)
{
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        CounterGraph *graph = ui->plot->graph(i);
        graph->setData(mPlotData.graphData(graph->fullName(), checked));
    }

    // TODO

    ui->plot->yAxis->rescale();
    adjustYAxisRange();
    updatePlotTitle();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionDisplayUtcTriggered(bool checked)
{
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    ticker->setDisplayUtc(checked);
    updatePlotTitle();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionRemoveZeroCountersTriggered()
{
    QVector<CounterGraph*> graphsToRemove;
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        CounterGraph *graph = ui->plot->graph(i);
        if (CounterData::isAllZero(graph->data())) {
            graphsToRemove.append(graph);
        }
    }
    removeGraphs(graphsToRemove);
}

void PlotWindow::actionScriptTriggered()
{
}

void PlotWindow::actionShowLegendTriggered(bool checked)
{
    ui->plot->legend->setVisible(checked);
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionMoveLegend()
{
    QAction *action = qobject_cast<QAction*>(sender());
    int align = action->data().toInt();
    QCPLayoutInset *inset = ui->plot->axisRect()->insetLayout();
    inset->setInsetPlacement(0, QCPLayoutInset::ipBorderAligned);
    inset->setInsetAlignment(0, static_cast<Qt::Alignment>(align));
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionReverseSelection()
{
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        CounterGraph *graph = ui->plot->graph(i);
        graph->setSelected(!graph->selected());
    }
    selectionChanged();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionRemoveSelectedGraphs()
{
    QVector<CounterGraph*> graphsToRemove;
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        CounterGraph *graph = ui->plot->graph(i);
        if (graph->selected()) {
            graphsToRemove.append(graph);
        }
    }
    removeGraphs(graphsToRemove);
}

void PlotWindow::selectionChanged()
{
    auto selectedLegendItems = ui->plot->legend->selectedItems();
    if (selectedLegendItems.size() == 1) {
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && mLastSelLegItemIndex >= 0) {
            int curSelLegItemIndex = legendItemIndex(selectedLegendItems.first());
            int min = 0, max = 0;
            if (curSelLegItemIndex < mLastSelLegItemIndex) {
                min = curSelLegItemIndex + 1;
                max = qMin(mLastSelLegItemIndex + 1, ui->plot->graphCount());
            } else if (curSelLegItemIndex > mLastSelLegItemIndex) {
                min = mLastSelLegItemIndex;
                max = curSelLegItemIndex;
            }
            for (int i = min; i < max; ++i) {
                ui->plot->graph(i)->setSelected(true);
            }
        } else {
            mLastSelLegItemIndex = legendItemIndex(selectedLegendItems.first());
        }
    }

    if (ui->plot->legend->selectedItems().isEmpty()) {
        mLastSelLegItemIndex = -1;
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

    if (mValueTip->visible() && mValueTip->tracerGraph() != nullptr && !mValueTip->tracerGraph()->visible()) {
        mValueTip->hideWithAnimation();
        mValueTip->setTracerGraph(nullptr);
    }
}

void PlotWindow::skippedTicksChanged(int skipped)
{
    bool visible = skipped == 0;
    for (int i = 0, graphCount = ui->plot->graphCount(); i < graphCount; ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph*>(ui->plot->graph(i));
        graph->setScatterVisible(visible);
    }
}

void PlotWindow::contextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    // TODO

    QAction *actionShowLegend = menu->addAction(QStringLiteral("Show Legend"), this, &PlotWindow::actionShowLegendTriggered);
    actionShowLegend->setCheckable(true);
    actionShowLegend->setChecked(ui->plot->legend->visible());

    QMenu *menuMoveLegend = menu->addMenu(QStringLiteral("Move Legend to"));
    menuMoveLegend->addAction(QStringLiteral("Top Left"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignLeft));
    menuMoveLegend->addAction(QStringLiteral("Top Center"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignCenter));
    menuMoveLegend->addAction(QStringLiteral("Top Right"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignRight));
    menuMoveLegend->addSeparator();
    menuMoveLegend->addAction(QStringLiteral("Bottom Left"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignLeft));
    menuMoveLegend->addAction(QStringLiteral("Bottom Center"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignCenter));
    menuMoveLegend->addAction(QStringLiteral("Bottom Right"), this, &PlotWindow::actionMoveLegend)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignRight));

    menu->addSeparator();

    menu->addAction(QStringLiteral("Reverse Selection"), this, &PlotWindow::actionReverseSelection);
    QAction *actionRemove = menu->addAction(QStringLiteral("Remove Selected Graphs"), this, &PlotWindow::actionRemoveSelectedGraphs);

    if (!ui->plot->hasSelectedGraphs()) {
        actionRemove->setEnabled(false);
    }

    menu->popup(ui->plot->mapToGlobal(pos));
}

void PlotWindow::plotMouseMove(QMouseEvent *event)
{
    if ((ui->plot->legend->visible() && ui->plot->legend->selectTest(event->pos(), false) > 0) || ui->plot->graphCount() == 0) {
        return;
    }

    QCPGraphData data;
    CounterGraph *graph = findNearestGraphData(event->pos(), data);
    if (graph == nullptr) {
        if (mValueTip->visible()) {
            mValueTip->hideWithAnimation();
        }
    } else if (graph != mValueTip->tracerGraph() || !qFuzzyCompare(mValueTip->tracerGraphKey(), data.key)) {
        mValueTip->setTracerGraphKey(data.key);
        if (graph != mValueTip->tracerGraph()) {
            mValueTip->setTracerPen(graph->pen());
            mValueTip->setTracerGraph(graph);
        }
        mValueTip->setValueInfo(graph->name(), mPlotData.dateTimeString(data.key), QString::number(data.value, 'f', 2), graph->isSuspect(data.key));
        mValueTip->showWithAnimation();
    }
}

void PlotWindow::setupPlot()
{
    QColor color(70, 50, 200);
    QPen pen(Qt::DashLine);
    pen.setColor(color);
    ui->plot->selectionRect()->setPen(pen);
    color.setAlpha(30);
    ui->plot->selectionRect()->setBrush(color);

    QCPLegend *legend = ui->plot->legend;
    color = legend->brush().color();
    color.setAlpha(200);
    legend->setIconSize(15, 8);
    legend->setBrush(QBrush(color));
    legend->setSelectableParts(QCPLegend::spItems);
    legend->setVisible(true);

    QSharedPointer<DateTimeTicker> ticker(new DateTimeTicker(ui->plot->xAxis));
    ticker->setOffsetFromUtc(mPlotData.offsetFromUtc());
    if (mPlotData.keyType() == PlotData::ktIndex) {
        ticker->setDateTimeVector(mPlotData.dateTimeVector());
    }
    connect(ticker.data(), &DateTimeTicker::skippedTicksChanged, this, &PlotWindow::skippedTicksChanged);

    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->xAxis2->setTicks(false);
    ui->plot->xAxis2->setTickLabels(false);
    ui->plot->yAxis2->setTicks(false);
    ui->plot->yAxis2->setTickLabels(false);
    ui->plot->xAxis->setTicker(ticker);
    ui->plot->setNoAntialiasingOnDrag(true);
    ui->plot->setAutoAddPlottableToLegend(false);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect | QCP::iSelectPlottables |
                              QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectItems);
    ui->plot->addLayer(ValueTipItem::layerName(), ui->plot->layer(QStringLiteral("axes")));
    ui->plot->layer(ValueTipItem::layerName())->setMode(QCPLayer::lmBuffered);

    connect(ui->plot, &CounterPlot::selectionChangedByUser, this, &PlotWindow::selectionChanged);
    connect(ui->plot, &CounterPlot::customContextMenuRequested, this, &PlotWindow::contextMenuRequested);
    connect(ui->plot, &CounterPlot::mouseMove, this, &PlotWindow::plotMouseMove);
}

void PlotWindow::initGraphs()
{
    const QList<QString> counterNames = mPlotData.counterNames();
    for (const QString &name : counterNames) {
        auto pair = CounterGraph::separateModuleName(name);
        CounterGraph *graph = ui->plot->addGraph();
        graph->setModuleName(pair.first);
        graph->setName(pair.second);
        graph->setFullName(name);
        graph->setPen(QPen(mColorPool.getColor()));
        graph->setData(mPlotData.graphData(name));
        graph->setSuspectKeys(mPlotData.suspectKeys(name));
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

void PlotWindow::highlightTimeGap()
{
    if (mPlotData.keyType() != PlotData::ktIndex) {
        return;
    }
    QPen pen(Qt::red);
    pen.setStyle(Qt::DotLine);
    double interval = mPlotData.getSampleInterval();
    QVector<double> dtVector = mPlotData.dateTimeVector();
    for (int i = 1; i < dtVector.size(); ++i) {
        double diff = dtVector[i] - dtVector[i - 1];
        if (diff >= interval * 1.5) {
            QCPItemStraightLine *line = new QCPItemStraightLine(ui->plot);
            line->setPen(pen);
            line->point1->setCoords(i, 0);
            line->point2->setCoords(i, 1);
        }
    }
}

void PlotWindow::updateWindowTitle()
{
    QStringList strList;
    bool appendEllipsis = false;
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        QString rightPart = CounterGraph::getNameRightPart(ui->plot->graph(i)->name());
        if (!strList.contains(rightPart)) {
            if (strList.size() < 3){
                strList.append(rightPart);
            } else {
                appendEllipsis = true;
                break;
            }
        }
    }

    QString title = strList.join(QLatin1String(", "));
    if (appendEllipsis) {
        title += "...";
    }
    setWindowTitle(title);
}

void PlotWindow::updatePlotTitle()
{
    QString title(windowTitle());
    title += " (";
    title += QString::number(ui->plot->graphCount());
    title += ui->plot->graphCount() > 1 ? " Graphs" : " Graph";

    if (ui->actionShowDelta->isChecked()) {
        title += ", Delta";
    }
    if (ui->actionDisplayUtc->isChecked()) {
        title += ", UTC";
    }

    title += ')';
    ui->plot->xAxis2->setLabel(title);
}

void PlotWindow::removeGraphs(const QVector<CounterGraph *> &graphs)
{
    if (graphs.isEmpty()) { return; }

    for (CounterGraph *graph : graphs) {
        mPlotData.removeGraphData(graph->fullName());
        if (mValueTip->tracerGraph() == graph) {
            mValueTip->hideWithAnimation();
            mValueTip->setTracerGraph(nullptr);
            mValueTip->setSelected(false);
        }
        // TODO
        ui->plot->removeGraph(graph);
    }

    updateWindowTitle();
    updatePlotTitle();
    selectionChanged();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
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

CounterGraph *PlotWindow::findNearestGraphData(const QPoint &pos, QCPGraphData &data) const
{
    CounterGraph *resultGraph = nullptr;
    double minDistance = std::numeric_limits<double>::max();
    QCPAxis *xAxis = ui->plot->xAxis, *yAxis = ui->plot->yAxis;
    const double radius = 10;
    const double minKey = xAxis->pixelToCoord(pos.x() - radius);
    const double maxKey = xAxis->pixelToCoord(pos.x() + radius);

    for (int i = ui->plot->graphCount() - 1; i >= 0; --i) {
        CounterGraph *graph = ui->plot->graph(i);
        if (!graph->visible()) {
            continue;
        }
        QSharedPointer<QCPGraphDataContainer> dataContainer = graph->data();
        if (dataContainer->isEmpty()) {
            continue;
        }
        for (auto iter = dataContainer->findBegin(minKey); iter != dataContainer->end() && iter->key < maxKey; ++iter) {
            QPointF dataPos(xAxis->coordToPixel(iter->key), yAxis->coordToPixel(iter->value));
            double distance = pointDistance(pos, dataPos);
            if (distance < radius && distance < minDistance) {
                resultGraph = graph;
                data = *iter;
            }
        }
    }

    return resultGraph;
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
