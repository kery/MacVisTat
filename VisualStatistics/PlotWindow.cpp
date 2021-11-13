#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "ScriptWindow.h"
#include "MultiLineInputDialog.h"
#include "Utils.h"
#include "Version.h"

const double PlotWindow::TracerSize = CounterGraph::ScatterSize + 4.0;
const int PlotWindow::AnimationMaxGraphs = 100;

PlotWindow::PlotWindow(Statistics &stat) :
    QMainWindow(nullptr),
    m_agggraph_idx(0),
    m_lastSelLegitemIndex(-1),
    m_ui(new Ui::PlotWindow),
    m_hasScatter(false),
    m_showModule(false),
    m_stat(std::move(stat))
{
    m_ui->setupUi(this);

    connect(m_ui->actionSaveAsImage, &QAction::triggered, this, &PlotWindow::actionSaveAsImageTriggered);
    connect(m_ui->actionRestoreScale, &QAction::triggered, this, &PlotWindow::actionRestoreScaleTriggered);
    connect(m_ui->actionShowDelta, &QAction::triggered, this, &PlotWindow::actionShowDeltaTriggered);
    connect(m_ui->actionShowSuspectFlag, &QAction::triggered, this, &PlotWindow::actionShowSuspectFlagTriggered);
    connect(m_ui->actionScript, &QAction::triggered, this, &PlotWindow::actionScriptTriggered);
    connect(m_ui->actionRemoveZeroCounters, &QAction::triggered, this, &PlotWindow::actionRemoveZeroCountersTriggered);
    connect(m_ui->actionCopyToClipboard, &QAction::triggered, this, &PlotWindow::actionCopyToClipboardTriggered);

    // Show tracer below legend layer, above other layers
    // Value tip will use the same layer as tracer
    m_ui->customPlot->addLayer(QStringLiteral("valuetip"), 0, QCustomPlot::limBelow);

    m_timer.setInterval(1000);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &PlotWindow::timerTimeout);

    // Must be called after setupUi because member customPlot is initialized
    // in it. QCustomPlot takes ownership of tracer.
    m_tracer = new QCPItemTracer(m_ui->customPlot);
    m_tracer->setInterpolating(true);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setLayer(QStringLiteral("valuetip"));
    m_tracer->setVisible(false);

    m_animation.setTargetObject(m_tracer);
    m_animation.setPropertyName("size");
    m_animation.setDuration(250);
    m_animation.setStartValue(0);
    m_animation.setEndValue(TracerSize);
    m_animation.setEasingCurve(QEasingCurve::OutQuad);

    connect(&m_animation, &QPropertyAnimation::valueChanged, [this] () {
        this->m_ui->customPlot->replot();
    });

    connect(&m_animation, &QPropertyAnimation::finished, [this] () {
        if (this->m_animation.direction() == QAbstractAnimation::Backward) {
            m_valueText->setVisible(false);
            m_tracer->setVisible(false);
            setTracerGraph(nullptr);
            this->m_ui->customPlot->replot(QCustomPlot::rpQueued);
        }
    });

    m_valueText = new ValueText(m_tracer);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_ui->toolBar->addWidget(spacer);
    m_dtEditFrom = new QDateTimeEdit();
    m_dtEditFrom->setDisplayFormat(DT_FORMAT_IN_PLOT);
    m_ui->toolBar->addWidget(m_dtEditFrom);
    spacer = new QWidget();
    spacer->setMinimumWidth(5);
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_ui->toolBar->addWidget(spacer);
    m_dtEditTo = new QDateTimeEdit();
    m_dtEditTo->setDisplayFormat(DT_FORMAT_IN_PLOT);
    m_ui->toolBar->addWidget(m_dtEditTo);

    connectXAxisRangeChanged();
    connectDateTimeEditChange();

    setFocus();
    initializePlot();
    markDiscontinuousTime();

    updateWindowTitle();
    updatePlotTitle();
}

PlotWindow::~PlotWindow()
{
    delete m_tracer;
    delete m_valueText;
    delete m_ui;
}

CounterGraph * PlotWindow::addCounterGraph(const QString &name, const QString &module)
{
    CounterGraph *graph = new CounterGraph(m_ui->customPlot->xAxis, m_ui->customPlot->yAxis,
                                           module, name);
    if (m_ui->customPlot->addPlottable(graph)) {
        graph->setPen(QPen(m_colorManager.getColor()));
        graph->setSelectedPen(graph->pen());

        if (m_hasScatter) {
            graph->enableScatter(true);
        }
        return graph;
    } else {
        delete graph;
        return nullptr;
    }
}

void PlotWindow::updateWindowTitle()
{
    QStringList names;
    bool appendEllipsis = false;
    int graphCount = m_ui->customPlot->graphCount();
    for (int i = 0; i < graphCount; ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(m_ui->customPlot->graph(i));
        QString tempName = graph->displayName();
        QString rightPart = tempName.mid(tempName.lastIndexOf('.') + 1);
        if (!names.contains(rightPart)) {
            if (names.size() < 3) {
                names.append(rightPart);
            } else {
                appendEllipsis = true;
                break;
            }
        }
    }

    QString title = names.join(QLatin1String(", "));
    if (appendEllipsis) {
        title.append("...");
    }
    setWindowTitle(title);
}

void PlotWindow::updatePlotTitle()
{
    QCustomPlot *plot = m_ui->customPlot;
    QString title(windowTitle());
    title += " (";
    title += QString::number(plot->graphCount());

    if (m_ui->actionShowDelta->isChecked()) {
        title += ", DELTA";
    }

    if (m_stat.utcMode()) {
        title += ", UTC";
    }

    title += ')';

    plot->xAxis2->setLabel(title);
}

Statistics& PlotWindow::getStat()
{
    return m_stat;
}

QCustomPlot* PlotWindow::getPlot()
{
    return m_ui->customPlot;
}

void PlotWindow::initializePlot()
{
    QCustomPlot *plot = m_ui->customPlot;

    plot->axisRect()->setupFullAxesBox();
    plot->xAxis->setAutoTicks(false);
    plot->xAxis->setAutoTickLabels(false);
    plot->xAxis->setAutoSubTicks(false);
    plot->xAxis->setSubTickCount(0);
    plot->xAxis->setTickLabelRotation(90);
    plot->xAxis2->setTicks(false);
    plot->yAxis2->setTicks(false);
    plot->legend->setSelectableParts(QCPLegend::spItems);
    plot->legend->setIconSize(15, 8);
    plot->legend->setVisible(true);

    QColor color = plot->legend->brush().color();
    color.setAlpha(200);
    plot->legend->setBrush(QBrush(color));
    plot->setNoAntialiasingOnDrag(true);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect | QCP::iSelectLegend |
                          QCP::iSelectPlottables | QCP::iSelectItems);

    connect(plot->xAxis, &QCPAxis::ticksRequest, this, &PlotWindow::adjustTicks);
    connect(plot, &QCustomPlot::selectionChangedByUser, this, &PlotWindow::selectionChanged);
    connect(plot, &QCustomPlot::mousePress, this, &PlotWindow::mousePress);
    connect(plot, &QCustomPlot::mouseMove, this, &PlotWindow::mouseMove);
    connect(plot, &QCustomPlot::mouseWheel, this, &PlotWindow::mouseWheel);
    connect(plot, &QCustomPlot::customContextMenuRequested, this, &PlotWindow::contextMenuRequest);

    QSettings settings;
    bool showSuspectFlag = settings.value(QStringLiteral("showSuspectFlag"), true).toBool();
    if (showSuspectFlag) {
        m_ui->actionShowSuspectFlag->setChecked(true);
    }

    const auto statNames = m_stat.getNames();
    for (const QString &statName : statNames) {
        QString name;
        QString module = Statistics::splitStatNameToModuleAndName(statName, name);
        CounterGraph *graph = addCounterGraph(name, module);
        graph->setName(statName);
        // Set copy to true to avoid the data being deleted if show delta function is used
        graph->setData(m_stat.getDataMap(statName), true);
        if (showSuspectFlag) {
            graph->enableSuspectFlag(true);
        }
        graph->enableDiscontinuousFlag(true);
    }

    plot->rescaleAxes();
    adjustYAxisRange(plot->yAxis);
}

void PlotWindow::markDiscontinuousTime()
{
    QPen pen(Qt::red);
    pen.setStyle(Qt::DotLine);

    uint interval = m_stat.getSampleInterval();

    for (int i = 1; i < m_stat.dateTimeCount(); ++i) {
        double diff = m_stat.getDateTime(i) - m_stat.getDateTime(i - 1);
        if (diff > double(interval * 1.5)) {
            QCPItemStraightLine *line = new QCPItemStraightLine(m_ui->customPlot);
            line->setPen(pen);
            line->point1->setCoords(i, -1.0);
            line->point2->setCoords(i, 1.0);
            m_ui->customPlot->addItem(line);
        }
    }
}

void PlotWindow::connectXAxisRangeChanged()
{
    connect(m_ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(updateDateTimeEdit(QCPRange)));
}

void PlotWindow::disconnectXAxisRangeChanged()
{
    disconnect(m_ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(updateDateTimeEdit(QCPRange)));
}

void PlotWindow::connectDateTimeEditChange()
{
    connect(m_dtEditFrom, &QDateTimeEdit::dateTimeChanged, this, &PlotWindow::fromDateTimeChanged);
    connect(m_dtEditTo, &QDateTimeEdit::dateTimeChanged, this, &PlotWindow::toDateTimeChanged);
}

void PlotWindow::disconnectDateTimeEditChange()
{
    disconnect(m_dtEditFrom, &QDateTimeEdit::dateTimeChanged, this, &PlotWindow::fromDateTimeChanged);
    disconnect(m_dtEditTo, &QDateTimeEdit::dateTimeChanged, this, &PlotWindow::toDateTimeChanged);
}

uint PlotWindow::localTime_t(const QDateTime &dt) const
{
    if (m_stat.utcMode()) {
        QDateTime tempDt = dt.toOffsetFromUtc(m_stat.offsetFromUtc());
        tempDt.setOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc());
        return tempDt.toTime_t();
    }
    return dt.toTime_t();
}

QVector<double> PlotWindow::calcTickVector(int plotWidth, int fontHeight, const QCPRange &range)
{
    QVector<double> result;
    int count = plotWidth / (fontHeight * 1.2);
    if (count > 0) {
        int step = qMax(1, static_cast<int>(range.size() / count));
        int upper = qMin(static_cast<int>(range.upper), m_stat.dateTimeCount() - 1);
        for (int i = qMax(static_cast<int>(range.lower), 0); i <= upper; i += step) {
            result << i;
        }
    }

    updateScatter(result, plotWidth, fontHeight);

    return result;
}

QVector<QString> PlotWindow::calcTickLabelVector(const QVector<double> &ticks)
{
    QVector<QString> result;
    for (double index : ticks) {
        result << m_stat.getDateTimeString(index);
    }
    return result;
}

void PlotWindow::calcDelta(QCPGraph *graph)
{
    QCPDataMap *data = graph->data();
    if (data->isEmpty()) {
        return;
    }

    for (auto iter = data->end() - 1; iter != data->begin(); --iter) {
        iter->value = iter->value - (iter - 1)->value;
    }

    data->begin()->value = 0;
}

bool PlotWindow::shouldDrawScatter(const QVector<double> &tickVector, int plotWidth, int fontHeight) const
{
    if (tickVector.size() < 2) {
        return false;
    }

    int step = tickVector[1] - tickVector[0];
    if (step > 1) {
        return false;
    }

    int tickCount = tickVector.size();
    QCPRange range = m_ui->customPlot->xAxis->range();
    if (tickVector.first() > range.lower) {
        tickCount += tickVector.first() - range.lower;
    }
    if (range.upper > tickVector.last()) {
        tickCount += range.upper - tickVector.last();
    }

    return (plotWidth / tickCount) >= fontHeight;
}

void PlotWindow::updateScatter(const QVector<double> &tickVector, int plotWidth, int fontHeight)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (shouldDrawScatter(tickVector, plotWidth, fontHeight)) {
        if (!m_hasScatter) {
            m_hasScatter = true;
            for (int i = 0; i < plot->graphCount(); ++i) {
                qobject_cast<CounterGraph *>(plot->graph(i))->enableScatter(true);
            }
        }
    } else {
        if (m_hasScatter) {
            m_hasScatter = false;
            for (int i = 0; i < plot->graphCount(); ++i) {
                qobject_cast<CounterGraph *>(plot->graph(i))->enableScatter(false);
            }
        }
    }
}

QCPGraph * PlotWindow::findNearestGraphValue(int index, double yCoord, double &value, bool &suspect)
{
    QCustomPlot *plot = m_ui->customPlot;
    QCPGraph *retGraph = nullptr;
    double minDist = std::numeric_limits<double>::max();

    for (int i = plot->graphCount() - 1; i >= 0; --i) {
        QCPGraph *graph = plot->graph(i);
        if (!graph->visible()) {
            continue;
        }

        QCPDataMap *data = graph->data();
        auto iter = data->find(index);
        if (iter == data->end()) {
            continue;
        }

        double yDistance = qAbs(iter->value - yCoord);
        if (yDistance < minDist) {
            minDist = yDistance;
            value = iter->value;
            suspect = iter->valueErrorMinus > 0;
            retGraph = graph;
        }
    }
    return retGraph;
}

QString PlotWindow::genAggregateGraphName()
{
    return QStringLiteral("aggregate_graph_%1").arg(++m_agggraph_idx);
}

void PlotWindow::removeGraphs(const QVector<CounterGraph *> &graphs)
{
    if (graphs.isEmpty()) {
        return;
    }

    QCustomPlot *plot = m_ui->customPlot;
    for (CounterGraph *graph : graphs) {
        m_stat.removeDataMap(graph->name());
        if (m_tracer->graph() == graph) {
            m_tracer->setVisible(false);
            m_valueText->setVisible(false);
            setTracerGraph(nullptr);
        }
        const QVector<CommentText *> cmtVec = commentsOfGraph(graph);
        for (CommentText *cmtText : cmtVec) {
            plot->removeItem(cmtText);
        }
        plot->removeGraph(graph);
    }

    updateWindowTitle();
    updatePlotTitle();

    selectionChanged();
    plot->replot(QCustomPlot::rpQueued);
}

QVector<CommentText *> PlotWindow::commentsOfGraph(const QCPGraph *graph) const
{
    QVector<CommentText *> cmtVec;
    for (int i = m_ui->customPlot->itemCount() - 1; i >= 0; --i) {
        CommentText *cmtText = qobject_cast<CommentText *>(m_ui->customPlot->item(i));
        if (cmtText && cmtText->graph() == graph) {
            cmtVec.append(cmtText);
        }
    }
    return cmtVec;
}

QString PlotWindow::defaultSaveFileName() const
{
    QString wndTitle = windowTitle();
    QStringList nameParts = wndTitle.split(", ", QString::SkipEmptyParts);
    if (!nameParts.isEmpty() && nameParts.last().endsWith(QLatin1String("..."))) {
        nameParts.last().chop(3);
    }
    return nameParts.join('_');
}

int PlotWindow::getLegendItemIndex(QCPAbstractLegendItem *item) const
{
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->legend->itemCount(); ++i) {
        if (item == plot->legend->item(i)) {
            return i;
        }
    }
    return -1;
}

void PlotWindow::setTracerGraph(QCPGraph *graph)
{
    if (m_tracer->graph() != nullptr) {
        disconnect(m_tracer, &QCPItemTracer::selectionChanged, m_tracer->graph(), &QCPGraph::setSelected);
        disconnect(m_tracer->graph(), &QCPGraph::selectionChanged, m_tracer, &QCPItemTracer::setSelected);
    }

    m_tracer->setGraph(graph);
    if (graph != nullptr) {
        connect(m_tracer, &QCPItemTracer::selectionChanged, graph, &QCPGraph::setSelected);
        connect(graph, &QCPGraph::selectionChanged, m_tracer, &QCPItemTracer::setSelected);
    }
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

int PlotWindow::graphIndex(QCPGraph *graph) const
{
    for (int i = 0; i < m_ui->customPlot->graphCount(); ++i) {
        if (graph == m_ui->customPlot->graph(i)) {
            return i;
        }
    }
    return -1;
}

QCPGraph * PlotWindow::prevGraph(QCPGraph *graph) const
{
    int index = graphIndex(graph);
    return index <= 0 ? nullptr : m_ui->customPlot->graph(index - 1);
}

QCPGraph * PlotWindow::nextGraph(QCPGraph *graph) const
{
    int nextIndex = graphIndex(graph) + 1;
    return nextIndex == 0 || nextIndex >= m_ui->customPlot->graphCount() ? nullptr : m_ui->customPlot->graph(nextIndex);
}

void PlotWindow::keyPressEvent(QKeyEvent *event)
{
    QList<QCPGraph *> graphs = m_ui->customPlot->selectedGraphs();
    if (graphs.size() != 1 || (event->key() != Qt::Key_Up && event->key() != Qt::Key_Down)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    QCPGraph *selGraph = event->key() == Qt::Key_Down ? nextGraph(graphs.first()) : prevGraph(graphs.first());
    if (selGraph) {
        graphs.first()->setSelected(false);
        selGraph->setSelected(true);
        selectionChanged();
        m_ui->customPlot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::adjustTicks()
{
    QCustomPlot *plot = m_ui->customPlot;
    const QCPRange range = plot->xAxis->range();
    int fontHeight = QFontMetrics(plot->xAxis->tickLabelFont()).height();

    QVector<double> ticks = calcTickVector(plot->size().width(), fontHeight, range);
    plot->xAxis->setTickVector(ticks);
    plot->xAxis->setTickVectorLabels(calcTickLabelVector(ticks));
}

void PlotWindow::selectionChanged()
{
    QCustomPlot *plot = m_ui->customPlot;

    QList<QCPAbstractLegendItem *> selectedLegendItems = plot->legend->selectedItems();
    if (selectedLegendItems.size() == 1) {
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && m_lastSelLegitemIndex >= 0) {
            int curSelLegitemIndex = getLegendItemIndex(selectedLegendItems.first());
            int min = 0, max = 0;
            if (curSelLegitemIndex < m_lastSelLegitemIndex) {
                min = curSelLegitemIndex + 1;
                max = qMin(m_lastSelLegitemIndex + 1, plot->graphCount());
            } else if (curSelLegitemIndex > m_lastSelLegitemIndex) {
                min = m_lastSelLegitemIndex;
                max = curSelLegitemIndex;
            }
            for (int i = min; i < max; ++i) {
                plot->graph(i)->setSelected(true);
            }
        } else {
            m_lastSelLegitemIndex = getLegendItemIndex(selectedLegendItems.first());
        }
    }

    if (plot->legend->selectedItems().isEmpty()) {
        m_lastSelLegitemIndex = -1;
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            graph->setVisible(true);

            const QVector<CommentText *> cmtVec = commentsOfGraph(graph);
            for (CommentText *cmtText : cmtVec) {
                cmtText->setVisible(true);
            }
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            graph->setVisible(graph->selected());

            const QVector<CommentText *> cmtVec = commentsOfGraph(graph);
            for (CommentText *cmtText : cmtVec) {
                cmtText->setVisible(graph->selected());
            }
        }

        if (QApplication::keyboardModifiers() & Qt::AltModifier) {
            plot->yAxis->rescale(true);
            adjustYAxisRange(plot->yAxis);
        }
    }

    if ((m_tracer->visible() || m_valueText->visible()) &&
            m_tracer->graph() != nullptr && !m_tracer->graph()->visible())
    {
        m_tracer->setVisible(false);
        m_valueText->setVisible(false);
        setTracerGraph(nullptr);
    }
}

void PlotWindow::mousePress(QMouseEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    // also support key modifiers
    QCustomPlot *plot = m_ui->customPlot;

    if (event->modifiers() & Qt::ControlModifier) {
        plot->axisRect()->setRangeDrag(plot->xAxis->orientation());
    } else if (event->modifiers() & Qt::ShiftModifier) {
        plot->axisRect()->setRangeDrag(plot->yAxis->orientation());
    } else {
        plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
}

void PlotWindow::mouseMove(QMouseEvent *event)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (plot->legend->visible() && plot->legend->selectTest(event->pos(), false) >= 0) {
        return;
    }
    m_timer.start();

    const double MAX_DIST = 20;

    bool suspectFlag;
    QCPGraph *graph = nullptr;
    double value = 0, dist = MAX_DIST;

    int index = qRound(plot->xAxis->pixelToCoord(event->pos().x()));
    QCPRange xRange = plot->xAxis->range();

    if (xRange.contains(index)) {
        double yCoord = plot->yAxis->pixelToCoord(event->pos().y());
        graph = findNearestGraphValue(index, yCoord, value, suspectFlag);
        dist = qAbs(plot->yAxis->coordToPixel(value) - event->pos().y());
    }

    if (dist >= MAX_DIST)
    {
        if (m_tracer->visible()) {
            if (plot->graphCount() <= AnimationMaxGraphs) {
                if (m_animation.state() != QAbstractAnimation::Running) {
                    m_animation.setDirection(QAbstractAnimation::Backward);
                    m_animation.start();
                }
            } else {
                m_tracer->setVisible(false);
                m_valueText->setVisible(false);
                setTracerGraph(nullptr);
                plot->replot(QCustomPlot::rpQueued);
            }
        }
    } else if (graph && (graph != m_tracer->graph() || (int)m_tracer->graphKey() != index)) {
        if (graph != m_tracer->graph()) {
            QColor color = graph->pen().color();
            QPen pen(color.darker(128));
            pen.setWidth(2);

            m_tracer->setPen(pen);
            m_tracer->setSelectedPen(pen);
            m_tracer->setBrush(color);
            m_tracer->setSelectedBrush(color);
        }

        m_tracer->setGraphKey(index);
        setTracerGraph(graph);
        m_tracer->setVisible(true);
        m_valueText->setValueInfo(qobject_cast<CounterGraph *>(graph)->displayName(),
                                  m_stat.getDateTimeString(index),
                                  QString::number(value, 'f', 2),
                                  suspectFlag);
        m_valueText->setVisible(true);

        if (plot->graphCount() <= AnimationMaxGraphs) {
            if (m_animation.state() == QAbstractAnimation::Running) {
                m_animation.stop();
            }
            m_animation.setDirection(QAbstractAnimation::Forward);
            m_animation.start();
        } else {
            m_tracer->setSize(TracerSize);
            plot->replot(QCustomPlot::rpQueued);
        }
    }
}

void PlotWindow::mouseWheel(QWheelEvent *event)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (event->modifiers() & Qt::ControlModifier) {
        plot->axisRect()->setRangeZoom(plot->xAxis->orientation());
    } else if (event->modifiers() & Qt::ShiftModifier) {
        plot->axisRect()->setRangeZoom(plot->yAxis->orientation());
    } else {
        plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }

    if (plot->legend->rect().contains(event->pos())) {
        const int STEP = 25;

        QPoint delta = event->angleDelta();
        QColor color = plot->legend->brush().color();
        if (delta.y() < 0) {
            color.setAlpha(qMax(0, color.alpha() - STEP));
        } else {
            color.setAlpha(qMin(color.alpha() + STEP, 255));
        }
        plot->legend->setBrush(QBrush(color));
        plot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::timerTimeout()
{
    if (!m_tracer->visible()) {
        return;
    }
    QPoint pos = m_ui->customPlot->mapFromGlobal(QCursor::pos());
    if (m_tracer->selectTest(pos, false) > m_ui->customPlot->selectionTolerance()) {
        m_valueText->setVisible(false);
        m_tracer->setVisible(false);
        setTracerGraph(nullptr);
        this->m_ui->customPlot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::contextMenuRequest(const QPoint &pos)
{
    CustomPlot *plot = m_ui->customPlot;
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    m_timer.stop();

    QAction *actionShowLegend = menu->addAction(QStringLiteral("Show Legend"), this, &PlotWindow::showLegendTriggered);
    actionShowLegend->setCheckable(true);
    actionShowLegend->setChecked(plot->legend->visible());

    QMenu *subMenu = menu->addMenu(QStringLiteral("Move Legend to"));
    subMenu->addAction(QStringLiteral("Top Left"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignLeft));
    subMenu->addAction(QStringLiteral("Top Center"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignCenter));
    subMenu->addAction(QStringLiteral("Top Right"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignRight));
    subMenu->addSeparator();
    subMenu->addAction(QStringLiteral("Bottom Left"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignLeft));
    subMenu->addAction(QStringLiteral("Bottom Center"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignCenter));
    subMenu->addAction(QStringLiteral("Bottom Right"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignRight));

    QAction *actionShowModuleName = menu->addAction(QStringLiteral("Show Module Name"), this, &PlotWindow::showModuleNameTriggered);
    actionShowModuleName->setCheckable(true);
    actionShowModuleName->setChecked(m_showModule);

    QAction *actionDisplayUtc = menu->addAction(QStringLiteral("Display UTC time"), this, &PlotWindow::displayUtcTimeTriggered);
    actionDisplayUtc->setCheckable(true);
    actionDisplayUtc->setChecked(m_stat.utcMode());
    if (!isValieOffsetFromUtc(m_stat.offsetFromUtc())) {
        actionDisplayUtc->setEnabled(false);
    }

    menu->addSeparator();

    QAction *actionAddComment = menu->addAction(QStringLiteral("Add Comment"), this, &PlotWindow::addComment);
    QAction *actionEditComment = menu->addAction(QStringLiteral("Edit Comment"), this, &PlotWindow::editComment);
    QAction *actionRmComment = menu->addAction(QStringLiteral("Remove Comment"), this, &PlotWindow::removeComment);
    actionAddComment->setData(pos);
    actionEditComment->setEnabled(false);
    actionRmComment->setEnabled(false);

    menu->addSeparator();

    QAction *actionAddGraph = menu->addAction(QStringLiteral("Add Aggregate Graph"), this, &PlotWindow::addAggregateGraph);
    QAction *actionSetColor = menu->addAction(QStringLiteral("Set Graph Color"), this, &PlotWindow::setGraphColor);
    QAction *actionCopyName = menu->addAction(QStringLiteral("Copy Graph Name"), this, &PlotWindow::copyGraphName);
    QAction *actionCopyValue = menu->addAction(QStringLiteral("Copy Graph Value"), this, &PlotWindow::copyGraphValue);

    menu->addSeparator();

    menu->addAction(QStringLiteral("Reverse Selection"), this, &PlotWindow::reverseSelection);
    QAction *actionRemove = menu->addAction(QStringLiteral("Remove Selected Graphs"), this, &PlotWindow::removeSelectedGraphs);

    CommentText *cmtText = plot->commentTextAt(pos, true);
    if (cmtText) {
        actionEditComment->setEnabled(true);
        actionRmComment->setEnabled(true);
        actionEditComment->setData(QVariant::fromValue<void *>(cmtText));
        actionRmComment->setData(QVariant::fromValue<void *>(cmtText));
    }

    auto selectedLegendItems = plot->legend->selectedItems();
    if (selectedLegendItems.isEmpty()) {
        if (!m_valueText->visible()) {
            actionSetColor->setEnabled(false);
            actionCopyName->setEnabled(false);
        }
        actionRemove->setEnabled(false);
    }

    if (plot->graphCount() < 2 || plot->legend->selectedItems().size() == 1) {
        actionAddGraph->setEnabled(false);
    }

    if (!m_valueText->visible()) {
        actionCopyValue->setEnabled(false);
    }

    menu->popup(plot->mapToGlobal(pos));
}

static QRectF translateToInsetRect(QCPLayoutInset *layout, const QRectF &rect)
{
    return QRectF(rect.x()/(layout->rect().width()),
                  rect.y()/(layout->rect().height()),
                  0, 0);
}

void PlotWindow::moveLegend()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    bool ok;
    int align = action->data().toInt(&ok);
    if (!ok) {
        return;
    }

    QCustomPlot *plot = m_ui->customPlot;
    QCPLayoutInset *layout = plot->axisRect()->insetLayout();
    layout->setInsetPlacement(0, QCPLayoutInset::ipBorderAligned);
    layout->setInsetAlignment(0, (Qt::Alignment)align);

    if (!plot->legend->visible()) {
        return;
    }

    if (plot->graphCount() <= AnimationMaxGraphs) {
        QRect originRect = plot->legend->outerRect();
        originRect.translate(-layout->rect().x(), -layout->rect().y());

        layout->updateLayout();

        QRect newRect = plot->legend->outerRect();
        newRect.translate(-layout->rect().x(), -layout->rect().y());

        QVariantAnimation *anim = new QVariantAnimation();
        anim->setDuration(250);
        anim->setStartValue(translateToInsetRect(layout, originRect));
        anim->setEndValue(translateToInsetRect(layout, newRect));
        anim->setEasingCurve(QEasingCurve::OutQuad);
        connect(anim, &QVariantAnimation::valueChanged, [layout, plot] (const QVariant &value) {
            layout->setInsetRect(0, value.toRectF());
            plot->replot();
        });
        connect(anim, &QVariantAnimation::finished, [layout, plot] () {
            layout->setInsetPlacement(0, QCPLayoutInset::ipBorderAligned);
            plot->replot(QCustomPlot::rpQueued);
        });
        layout->setInsetPlacement(0, QCPLayoutInset::ipFree);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        plot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::addComment()
{
    QString comment = getInputComment(m_valueText->visible() ? m_valueText->text() : QString());
    if (comment.isEmpty()) {
        return;
    }

    CommentText *textItem = new CommentText(m_ui->customPlot);
    textItem->setText(comment);

    if (m_tracer->visible() && m_tracer->graph()) {
        QCPAxisRect *axisRect = m_ui->customPlot->axisRect();
        QSize itemSize = textItem->size();
        QPointF pos = m_tracer->position->pixelPoint() + QPointF(30, -30);
        if (pos.x() + itemSize.width() > axisRect->right()) {
            pos.rx() = axisRect->right() - itemSize.width()/2;
        } else {
            pos.rx() += itemSize.width()/2;
        }
        if (pos.y() - itemSize.height() < axisRect->top()) {
            pos.ry() = axisRect->top() + itemSize.height()/2;
        } else {
            pos.ry() -= itemSize.height()/2;
        }

        pos.setX(m_ui->customPlot->xAxis->pixelToCoord(pos.x()));
        pos.setY(m_ui->customPlot->yAxis->pixelToCoord(pos.y()));
        textItem->position->setCoords(pos);
        textItem->setGraphAndKey(m_tracer->graph(), m_tracer->graphKey());
        textItem->updateLineStartAnchor();
        m_tracer->setVisible(false);
        m_valueText->setVisible(false);
        setTracerGraph(nullptr);
    } else {
        QAction *action = qobject_cast<QAction*>(sender());
        QPointF pos = action->data().toPoint();
        pos.setX(m_ui->customPlot->xAxis->pixelToCoord(pos.x()));
        pos.setY(m_ui->customPlot->yAxis->pixelToCoord(pos.y()));
        textItem->position->setCoords(pos);
    }

    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::editComment()
{
    QAction *action = qobject_cast<QAction*>(sender());
    CommentText *cmtText = static_cast<CommentText *>(action->data().value<void *>());
    QString comment = getInputComment(cmtText->text());
    if (comment.isEmpty()) {
        return;
    }
    cmtText->setText(comment);
    cmtText->updateLineStartAnchor();
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::removeComment()
{
    QAction *action = qobject_cast<QAction*>(sender());
    CommentText *cmtText = static_cast<CommentText *>(action->data().value<void *>());
    m_ui->customPlot->removeItem(cmtText);
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::addAggregateGraph()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Input graph name"));
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setLabelText(QStringLiteral("Graph name:"));
    dlg.setTextValue(genAggregateGraphName());
    dlg.resize(500, 0);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString aggregateGraphName = dlg.textValue();
    if (aggregateGraphName.isEmpty()) {
        return;
    }

    QCustomPlot *plot = m_ui->customPlot;
    const QList<QCPAbstractLegendItem *> selectedLegendItems = plot->legend->selectedItems();

    QVector<QCPDataMap *> dataMaps;
    dataMaps.reserve(selectedLegendItems.size() > 1 ? selectedLegendItems.size() : plot->graphCount());

    if (selectedLegendItems.size() > 1) {
        for (QCPAbstractLegendItem *item : selectedLegendItems) {
            QCPPlottableLegendItem *legendItem = qobject_cast<QCPPlottableLegendItem *>(item);
            CounterGraph *graph = qobject_cast<CounterGraph *>(legendItem->plottable());
            dataMaps.append(graph->data());
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            dataMaps.append(plot->graph(i)->data());
        }
    }

    QCPDataMap *aggregateDataMap = m_stat.addDataMap(aggregateGraphName);
    if (aggregateDataMap == nullptr) {
        showErrorMsgBox(this, QStringLiteral("Graph name \"%1\" already exists!").arg(aggregateGraphName));
        return;
    }

    for (const QCPDataMap *dataMap : dataMaps) {
        for (const QCPData &data: *dataMap) {
            QCPData &dstData = (*aggregateDataMap)[data.key];
            dstData.key = data.key;
            dstData.value += data.value;
            if (data.valueErrorMinus > 0) {
                dstData.valueErrorMinus = data.valueErrorMinus;
            }
        }
    }

    CounterGraph *aggregateGraph = addCounterGraph(aggregateGraphName);
    aggregateGraph->setName(aggregateGraphName);
    aggregateGraph->setData(aggregateDataMap, true);
    if (m_ui->actionShowSuspectFlag->isChecked()) {
        aggregateGraph->enableSuspectFlag(true);
    }

    updateWindowTitle();
    updatePlotTitle();
    plot->yAxis->rescale();
    adjustYAxisRange(plot->yAxis);
    plot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::reverseSelection()
{
    for (int i = 0; i < m_ui->customPlot->graphCount(); ++i) {
        QCPGraph *graph = m_ui->customPlot->graph(i);
        graph->setSelected(!graph->selected());
    }
    selectionChanged();
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::removeSelectedGraphs()
{
    QVector<CounterGraph*> graphs;
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        if (graph->selected()) {
            graphs.append(graph);
        }
    }
    removeGraphs(graphs);
}

void PlotWindow::setGraphColor()
{
    QVector<QCPGraph *> graphs;
    if (m_valueText->visible() && m_tracer->graph()) {
        graphs.append(m_tracer->graph());
    } else {
        const auto selectedItems = m_ui->customPlot->legend->selectedItems();
        for (auto item : selectedItems) {
            QCPAbstractPlottable *plottable = qobject_cast<QCPPlottableLegendItem *>(item)->plottable();
            QCPGraph *graph = qobject_cast<QCPGraph *>(plottable);
            graphs.append(graph);
        }
    }
    if (graphs.isEmpty()) {
        return;
    }

    QColor color = QColorDialog::getColor(graphs.first()->pen().color(), this);
    if (!color.isValid()) {
        return;
    }
    for (QCPGraph *graph : graphs) {
        graph->setPen(color);
        graph->setSelectedPen(graph->pen());

        QCPScatterStyle scatterStyle = graph->scatterStyle();
        if (scatterStyle.shape() != QCPScatterStyle::ssNone) {
            scatterStyle.setPen(color);
            graph->setScatterStyle(scatterStyle);
        }
    }
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::copyGraphName()
{
    if (m_valueText->visible()) {
        QApplication::clipboard()->setText(m_valueText->graphName());
    } else {
        QStringList strList;
        const auto selectedItems = m_ui->customPlot->legend->selectedItems();
        for (auto item : selectedItems) {
            QCPAbstractPlottable *plottable = qobject_cast<QCPPlottableLegendItem *>(item)->plottable();
            CounterGraph *graph = qobject_cast<CounterGraph *>(plottable);
            strList << graph->displayName();
        }
        QApplication::clipboard()->setText(strList.join('\n'));
    }
}

void PlotWindow::copyGraphValue()
{
    QGuiApplication::clipboard()->setText(m_valueText->graphValue());
}

void PlotWindow::showLegendTriggered(bool checked)
{
    QCustomPlot *plot = m_ui->customPlot;
    plot->legend->setVisible(checked);
    plot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::showModuleNameTriggered(bool checked)
{
    m_showModule = checked;

    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        graph->setShowModule(checked);
    }

    if (m_valueText->visible() || plot->legend->visible()) {
        m_valueText->setVisible(false);
        plot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::displayUtcTimeTriggered(bool checked)
{
    if (m_stat.setUtcMode(checked)) {
        QCustomPlot *plot = m_ui->customPlot;
        updateDateTimeEdit(plot->xAxis->range());
        updatePlotTitle();
        plot->replot(QCustomPlot::rpQueued);
    }
}

void PlotWindow::updateDateTimeEdit(const QCPRange &newRange)
{
    uint dtFrom, dtTo;
    int lower = (int)newRange.lower;
    int upper = (int)newRange.upper;
    if (lower >= m_stat.dateTimeCount()) {
        dtFrom = m_stat.getLastDateTime();
        dtTo = dtFrom;
    } else if (upper < 0) {
        dtFrom = m_stat.getFirstDateTime();
        dtTo = dtFrom;
    } else {
        if (lower < 0) {
            dtFrom = m_stat.getFirstDateTime();
        } else {
            dtFrom = m_stat.getDateTime(lower);
        }
        if (upper >= m_stat.dateTimeCount()) {
            dtTo = m_stat.getLastDateTime();
        } else {
            dtTo = m_stat.getDateTime(upper);
        }
    }

    disconnectDateTimeEditChange();

    if (m_stat.utcMode()) {
        m_dtEditFrom->setTimeSpec(Qt::UTC);
        m_dtEditTo->setTimeSpec(Qt::UTC);

        QDateTime dt = QDateTime::fromTime_t(dtFrom);
        dt.setOffsetFromUtc(m_stat.offsetFromUtc());
        m_dtEditFrom->setDateTime(dt.toUTC());

        dt = QDateTime::fromTime_t(dtTo);
        dt.setOffsetFromUtc(m_stat.offsetFromUtc());
        m_dtEditTo->setDateTime(dt.toUTC());
    } else {
        m_dtEditFrom->setTimeSpec(Qt::LocalTime);
        m_dtEditTo->setTimeSpec(Qt::LocalTime);

        m_dtEditFrom->setDateTime(QDateTime::fromTime_t(dtFrom));
        m_dtEditTo->setDateTime(QDateTime::fromTime_t(dtTo));
    }

    connectDateTimeEditChange();
}

void PlotWindow::fromDateTimeChanged(const QDateTime &dateTime)
{
    uint dt = localTime_t(dateTime);
    int index = m_stat.firstIndexAfterTime_t(dt);
    if (index >= 0) {
        disconnectXAxisRangeChanged();
        m_ui->customPlot->xAxis->setRangeLower(std::max(0, index - 1));
        m_ui->customPlot->replot(QCustomPlot::rpQueued);
        connectXAxisRangeChanged();
    }
}

void PlotWindow::toDateTimeChanged(const QDateTime &dateTime)
{
    uint dt = localTime_t(dateTime);
    int index = m_stat.firstIndexAfterTime_t(dt);
    if (index >= 0) {
        disconnectXAxisRangeChanged();
        m_ui->customPlot->xAxis->setRangeUpper(std::max(0, index - 1));
        m_ui->customPlot->replot(QCustomPlot::rpQueued);
        connectXAxisRangeChanged();
    }
}

void PlotWindow::actionSaveAsImageTriggered()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save As Image"),
                                                    defaultSaveFileName(),
                                                    QStringLiteral("PNG File (*.png)"));
    if (!path.isEmpty()) {
        m_ui->customPlot->savePng(path);
    }
}

void PlotWindow::actionRestoreScaleTriggered()
{
    m_ui->customPlot->rescaleAxes();
    adjustYAxisRange(m_ui->customPlot->yAxis);
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::actionShowDeltaTriggered(bool checked)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (checked) {
        for (int i = 0; i < plot->graphCount(); ++i) {
            calcDelta(plot->graph(i));
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
            graph->setData(m_stat.getDataMap(graph->name()), true);
        }
    }

    updatePlotTitle();

    m_valueText->setVisible(false);

    // Only rescale Y axis
    plot->yAxis->rescale();
    adjustYAxisRange(plot->yAxis);
    plot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::actionShowSuspectFlagTriggered(bool checked)
{
    QCustomPlot *plot = m_ui->customPlot;

    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        graph->enableSuspectFlag(checked);
    }

    plot->replot(QCustomPlot::rpQueued);

    QSettings settings;
    settings.setValue(QStringLiteral("showSuspectFlag"), checked);
}

void PlotWindow::actionScriptTriggered()
{
    ScriptWindow *scriptWindow = findChild<ScriptWindow*>(QStringLiteral("ScriptWindow"), Qt::FindDirectChildrenOnly);
    if (scriptWindow) {
        if (scriptWindow->isMinimized()) {
            scriptWindow->showNormal();
        }
    } else {
        scriptWindow = new ScriptWindow(this);
        QString err;
        if (scriptWindow->initialize(err)) {
            scriptWindow->setAttribute(Qt::WA_DeleteOnClose);
            scriptWindow->showNormal();
        } else {
            showInfoMsgBox(this, QStringLiteral("Initialize script window failed."), err);
            delete scriptWindow;
        }
    }
}

void PlotWindow::actionRemoveZeroCountersTriggered()
{
    QVector<CounterGraph*> graphs;
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        const QCPDataMap *dataMap = graph->data();

        bool isZeroCounter = true;
        for (const QCPData &data : *dataMap) {
            // compare with 2 decimal places precision
            if (qint64(data.value * 100) != 0) {
                isZeroCounter = false;
                break;
            }
        }
        if (isZeroCounter) {
            graphs << graph;
        }
    }

    removeGraphs(graphs);
}

void PlotWindow::actionCopyToClipboardTriggered()
{
    QApplication::clipboard()->setPixmap(m_ui->customPlot->toPixmap());
}
