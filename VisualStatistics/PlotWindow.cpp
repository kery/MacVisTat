#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "CounterGraph.h"
#include "CounterData.h"
#include "DateTimeTicker.h"
#include "ValueTipItem.h"
#include "CommentItem.h"
#include "MultiLineInputDialog.h"
#include "CounterNameModel.h"
#include "Utils.h"
#include "GlobalDefines.h"

#define WND_TITLE_SEP ", "

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
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save As Image"), defaultSaveFileName(),
                                                QStringLiteral("PNG File (*.png)"));
    if (!path.isEmpty()) {
        ui->plot->savePng(path);
    }
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
        graph->setData(mPlotData.counterData(graph->name(), checked));
    }

    ui->plot->yAxis->rescale();
    adjustYAxisRange();
    updatePlotTitle();
    mValueTip->hideWithAnimation();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionDisplayUtcTriggered(bool checked)
{
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    ticker->setUtcMode(checked);
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

void PlotWindow::actionMoveLegendTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    int align = action->data().toInt();
    QCPLayoutInset *inset = ui->plot->axisRect()->insetLayout();
    inset->setInsetPlacement(0, QCPLayoutInset::ipBorderAligned);
    inset->setInsetAlignment(0, static_cast<Qt::Alignment>(align));
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionAddCommentTriggered()
{
    QString comment = getInputComment(mValueTip->visible() ? mValueTip->text() : QString());
    if (comment.isEmpty()) {
        return;
    }

    CommentItem *ci = new CommentItem(ui->plot);
    ci->setText(comment);

    if (mValueTip->visible()) {
        QCPAxisRect *axisRect = ui->plot->axisRect();
        QSizeF ciSize = ci->size();
        QPointF pos = mValueTip->tracerPosition()->pixelPosition() + QPointF(30, -30);
        if (pos.x() + ciSize.width() > axisRect->right()) {
            pos.rx() = axisRect->right() - ciSize.width() / 2;
        } else {
            pos.rx() += ciSize.width() / 2;
        }
        if (pos.y() - ciSize.height() < axisRect->top()) {
            pos.ry() = axisRect->top() + ciSize.height() / 2;
        } else {
            pos.ry() -= ciSize.height() / 2;
        }

        pos.setX(ui->plot->xAxis->pixelToCoord(pos.x()));
        pos.setY(ui->plot->yAxis->pixelToCoord(pos.y()));
        ci->position->setCoords(pos);
        ci->setGraphAndKey(mValueTip->tracerGraph(), mValueTip->tracerGraphKey());
        ci->updateLineStartAnchor();
        mValueTip->hideWithAnimation();
        mValueTip->setTracerGraph(nullptr);
    } else {
        QAction *action = qobject_cast<QAction*>(sender());
        QPointF pos = action->data().toPoint();
        pos.setX(ui->plot->xAxis->pixelToCoord(pos.x()));
        pos.setY(ui->plot->yAxis->pixelToCoord(pos.y()));
        ci->position->setCoords(pos);
    }

    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionEditCommentTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    CommentItem *ci = static_cast<CommentItem*>(action->data().value<void*>());
    QString comment = getInputComment(ci->text());
    if (comment.isEmpty()) {
        return;
    }
    ci->setText(comment);
    ci->updateLineStartAnchor();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionRemoveCommentTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    CommentItem *ci = static_cast<CommentItem*>(action->data().value<void*>());
    ui->plot->removeItem(ci);
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionAddAggregateGraphTriggered()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(APP_NAME);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setLabelText(QStringLiteral("Graph name:"));
    dlg.resize(500, 0);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    QString graphName = dlg.textValue();
    if (graphName.isEmpty()) {
        return;
    }
    QSharedPointer<QCPGraphDataContainer> newData = mPlotData.addCounterData(graphName);
    if (!newData) {
        showErrorMsgBox(this, QStringLiteral("Graph name \"%1\" already exists!").arg(graphName));
        return;
    }

    QSet<double> *suspectKeys = mPlotData.suspectKeys(graphName);
    QVector<QCPGraphData> sumDataVector;
    const auto selectedGraphs = ui->plot->selectedGraphs();
    if (selectedGraphs.size() > 1) {
        CounterGraph *graph = selectedGraphs.first();
        QSharedPointer<QCPGraphDataContainer> data = graph->data();
        sumDataVector.resize(data->size());

        suspectKeys->unite(*graph->suspectKeys());
        for (int i = 0; i < data->size(); ++i) {
            sumDataVector[i].key = data->at(i)->key;
            sumDataVector[i].value = data->at(i)->value;
        }

        for (int i = 1; i < selectedGraphs.size(); ++i) {
            CounterGraph *graph = selectedGraphs[i];
            QSharedPointer<QCPGraphDataContainer> data = graph->data();
            suspectKeys->unite(*graph->suspectKeys());
            for (int j = 0; j < data->size(); ++j) {
                sumDataVector[j].value += data->at(j)->value;
            }
        }
    } else {
        CounterGraph *graph = ui->plot->graph(0);
        QSharedPointer<QCPGraphDataContainer> data = graph->data();
        sumDataVector.resize(data->size());

        suspectKeys->unite(*graph->suspectKeys());
        for (int i = 0; i < data->size(); ++i) {
            sumDataVector[i].key = data->at(i)->key;
            sumDataVector[i].value = data->at(i)->value;
        }

        for (int i = 1; i < ui->plot->graphCount(); ++i) {
            CounterGraph *graph = ui->plot->graph(i);
            QSharedPointer<QCPGraphDataContainer> data = graph->data();
            suspectKeys->unite(*graph->suspectKeys());
            for (int j = 0; j < data->size(); ++j) {
                sumDataVector[j].value += data->at(j)->value;
            }
        }
    }
    newData->set(sumDataVector, true);

    CounterGraph *newGraph = ui->plot->addGraph();
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    newGraph->setDisplayName(graphName);
    newGraph->setName(graphName);
    newGraph->setPen(QPen(mColorPool.getColor()));
    newGraph->setData(newData);
    newGraph->setSuspectKeys(suspectKeys);
    newGraph->setScatterVisible(ticker->skippedTicks() == 0);

    updateWindowTitle();
    updatePlotTitle();
    ui->plot->yAxis->rescale();
    adjustYAxisRange();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionSetGraphColorTriggered()
{
    QVector<CounterGraph*> graphVector;
    if (mValueTip->visible() && mValueTip->tracerGraph()) {
        graphVector.append(mValueTip->tracerGraph());
    } else {
        const auto selectedGraphs = ui->plot->selectedGraphs();
        for (CounterGraph *graph : selectedGraphs) {
            graphVector.append(graph);
        }
    }
    if (graphVector.isEmpty()) {
        return;
    }
    QColor color = QColorDialog::getColor(graphVector.first()->pen().color(), this);
    if (!color.isValid()) {
        return;
    }
    for (CounterGraph *graph : graphVector) {
        graph->setPen(color);
    }
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionCopyGraphNameTriggered()
{
    if (mValueTip->visible()) {
        QApplication::clipboard()->setText(mValueTip->graphName());
    } else {
        QStringList strList;
        const auto selectedGraphs = ui->plot->selectedGraphs();
        for (const CounterGraph *graph : selectedGraphs) {
            strList << graph->name();
        }
        QApplication::clipboard()->setText(strList.join('\n'));
    }
}

void PlotWindow::actionCopyGraphValueTriggered()
{
    QGuiApplication::clipboard()->setText(mValueTip->graphValue());
}

void PlotWindow::actionReverseSelectionTriggered()
{
    for (int i = 0; i < ui->plot->graphCount(); ++i) {
        CounterGraph *graph = ui->plot->graph(i);
        graph->setSelected(!graph->selected());
    }
    selectionChanged();
    ui->plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotWindow::actionRemoveSelectedGraphsTriggered()
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

    if (!ui->plot->hasSelectedGraphs()) {
        mLastSelLegItemIndex = -1;
        for (int i = 0; i < ui->plot->graphCount(); ++i) {
            CounterGraph *graph = ui->plot->graph(i);
            graph->setVisible(true);

            const QVector<CommentItem*> ciVector = commentItemsOfGraph(graph);
            for (CommentItem *ci : ciVector) {
                ci->setVisible(true);
            }
        }
    } else {
        for (int i = 0; i < ui->plot->graphCount(); ++i) {
            CounterGraph *graph = ui->plot->graph(i);
            graph->setVisible(graph->selected());

            const QVector<CommentItem*> ciVector = commentItemsOfGraph(graph);
            for (CommentItem *ci : ciVector) {
                ci->setVisible(graph->selected());
            }
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

    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    QAction *actionDisplayUtc = menu->addAction(QStringLiteral("Display UTC Time"), this, &PlotWindow::actionDisplayUtcTriggered);
    actionDisplayUtc->setCheckable(true);
    actionDisplayUtc->setChecked(ticker->isUtcMode());
    QAction *actionShowLegend = menu->addAction(QStringLiteral("Show Legend"), this, &PlotWindow::actionShowLegendTriggered);
    actionShowLegend->setCheckable(true);
    actionShowLegend->setChecked(ui->plot->legend->visible());

    QMenu *menuMoveLegend = menu->addMenu(QStringLiteral("Move Legend to"));
    menuMoveLegend->addAction(QStringLiteral("Top Left"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignLeft));
    menuMoveLegend->addAction(QStringLiteral("Top Center"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignCenter));
    menuMoveLegend->addAction(QStringLiteral("Top Right"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignRight));
    menuMoveLegend->addSeparator();
    menuMoveLegend->addAction(QStringLiteral("Bottom Left"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignLeft));
    menuMoveLegend->addAction(QStringLiteral("Bottom Center"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignCenter));
    menuMoveLegend->addAction(QStringLiteral("Bottom Right"), this, &PlotWindow::actionMoveLegendTriggered)->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignRight));

    menu->addSeparator();

    QAction *actionAddComment = menu->addAction(QStringLiteral("Add Comment"), this, &PlotWindow::actionAddCommentTriggered);
    QAction *actionEditComment = menu->addAction(QStringLiteral("Edit Comment"), this, &PlotWindow::actionEditCommentTriggered);
    QAction *actionRmComment = menu->addAction(QStringLiteral("Remove Comment"), this, &PlotWindow::actionRemoveCommentTriggered);
    actionAddComment->setData(pos);
    actionEditComment->setEnabled(false);
    actionRmComment->setEnabled(false);

    menu->addSeparator();

    QAction *actionAddGraph = menu->addAction(QStringLiteral("Add Aggregate Graph"), this, &PlotWindow::actionAddAggregateGraphTriggered);
    QAction *actionSetColor = menu->addAction(QStringLiteral("Set Graph Color"), this, &PlotWindow::actionSetGraphColorTriggered);
    QAction *actionCopyName = menu->addAction(QStringLiteral("Copy Graph Name"), this, &PlotWindow::actionCopyGraphNameTriggered);
    QAction *actionCopyValue = menu->addAction(QStringLiteral("Copy Graph Value"), this, &PlotWindow::actionCopyGraphValueTriggered);

    menu->addSeparator();

    menu->addAction(QStringLiteral("Reverse Selection"), this, &PlotWindow::actionReverseSelectionTriggered);
    QAction *actionRmGraphs = menu->addAction(QStringLiteral("Remove Selected Graphs"), this, &PlotWindow::actionRemoveSelectedGraphsTriggered);

    CommentItem *ci = ui->plot->commentItemAt(pos, true);
    if (ci) {
        actionEditComment->setEnabled(true);
        actionRmComment->setEnabled(true);
        actionEditComment->setData(QVariant::fromValue<void*>(ci));
        actionRmComment->setData(QVariant::fromValue<void*>(ci));
    }
    if (!ui->plot->hasSelectedGraphs()) {
        if (!mValueTip->visible()) {
            actionSetColor->setEnabled(false);
            actionCopyName->setEnabled(false);
        }
        actionRmGraphs->setEnabled(false);
    }
    if (ui->plot->graphCount() < 2 || ui->plot->selectedGraphCount() == 1) {
        actionAddGraph->setEnabled(false);
    }
    if (!mValueTip->visible()) {
        actionCopyValue->setEnabled(false);
    }

    menu->popup(ui->plot->mapToGlobal(pos));
}

void PlotWindow::plotMouseMove(QMouseEvent *event)
{
    if (ui->plot->pointInVisibleLegend(event->pos()) || ui->plot->graphCount() == 0) {
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
        auto pair = CounterNameModel::separateModuleName(name);
        CounterGraph *graph = ui->plot->addGraph();
        graph->setModuleName(pair.first);
        graph->setDisplayName(pair.second);
        graph->setName(name);
        graph->setPen(QPen(mColorPool.getColor()));
        graph->setData(mPlotData.counterData(name));
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
        QString objName = CounterNameModel::getObjectName(ui->plot->graph(i)->name());
        if (!strList.contains(objName)) {
            if (strList.size() < 3){
                strList.append(objName);
            } else {
                appendEllipsis = true;
                break;
            }
        }
    }

    QString title = strList.join(QLatin1String(WND_TITLE_SEP));
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

    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(ui->plot->xAxis->ticker());
    if (ui->actionShowDelta->isChecked()) {
        title += ", Delta";
    }
    if (ticker->isUtcMode()) {
        title += ", UTC";
    }

    title += ')';
    ui->plot->xAxis2->setLabel(title);
}

QString PlotWindow::defaultSaveFileName() const
{
    QString wndTitle = windowTitle();
    QStringList strList = wndTitle.split(WND_TITLE_SEP, QString::SkipEmptyParts);
    if (!strList.isEmpty() && strList.last().endsWith(QLatin1String("..."))) {
        strList.last().chop(3);
    }
    return strList.join('_');
}

QString PlotWindow::getInputComment(const QString &text)
{
    MultiLineInputDialog dlg(this);
    dlg.setLabelText(QStringLiteral("Comment:"));
    dlg.setTextValue(text);
    if (dlg.exec() != QDialog::Accepted) {
        return QString();
    }
    return dlg.textValue();
}

QVector<CommentItem*> PlotWindow::commentItemsOfGraph(CounterGraph *graph) const
{
    QVector<CommentItem*> ciVector;
    for (int i = ui->plot->itemCount() - 1; i >= 0; --i) {
        CommentItem *ci = qobject_cast<CommentItem*>(ui->plot->item(i));
        if (ci && ci->graph() == graph) {
            ciVector.append(ci);
        }
    }
    return ciVector;
}

void PlotWindow::removeGraphs(const QVector<CounterGraph *> &graphs)
{
    if (graphs.isEmpty()) { return; }

    for (CounterGraph *graph : graphs) {
        mPlotData.removeCounterData(graph->name());
        if (mValueTip->tracerGraph() == graph) {
            mValueTip->hideWithAnimation();
            mValueTip->setTracerGraph(nullptr);
            mValueTip->setSelected(false);
        }
        const QVector<CommentItem*> ciVector = commentItemsOfGraph(graph);
        for (CommentItem *ci : ciVector) {
            ui->plot->removeItem(ci);
        }
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

    for (int i = 0; i < ui->plot->graphCount(); ++i) {
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
