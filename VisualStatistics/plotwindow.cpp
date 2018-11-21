#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "scriptwindow.h"
#include "utils.h"
#include "version.h"

static const char *SUFFIX_DELTA = " (DELTA)";

static const double SCATTER_SIZE = 6.0;
static const double TRACER_SIZE = SCATTER_SIZE + 1.0;

PlotWindow::PlotWindow(Statistics &stat) :
    QMainWindow(nullptr),
    m_sampleInterval(60),
    m_ui(new Ui::PlotWindow),
    m_userEditFlag(true),
    m_userDragFlag(true),
    m_hasScatter(false),
    m_stat(std::move(stat))
{
    m_ui->setupUi(this);
    setWindowTitle(m_stat.getNodesString());

    m_sampleInterval = m_stat.getSampleInterval();

    // Must be called after setupUi because member customPlot is initialized
    // in it. QCustomPlot takes ownership of tracer.
    m_tracer = new QCPItemTracer(m_ui->customPlot);
    m_tracer->setInterpolating(true);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setSize(TRACER_SIZE);
    m_tracer->setPen(QPen(Qt::red));
    m_tracer->setBrush(QColor(255, 0, 0, 100));
    m_tracer->setLayer(QStringLiteral("axes"));
    m_tracer->setVisible(false);

    QToolButton *saveButton = static_cast<QToolButton*>(
                m_ui->toolBar->widgetForAction(m_ui->actionSaveAsImage));
    saveButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *copyToClipboard = new QMenu(this);
    copyToClipboard->addAction(m_ui->actionCopyToClipboard);
    saveButton->setMenu(copyToClipboard);

    QToolButton *markRestartButton = static_cast<QToolButton*>(
                m_ui->toolBar->widgetForAction(m_ui->actionMarkRestartTime));
    markRestartButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *setSampleInterval = new QMenu(this);
    setSampleInterval->addAction(m_ui->actionSetSampleInterval);
    markRestartButton->setMenu(setSampleInterval);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_ui->toolBar->addWidget(spacer);
    m_dtEditFrom = new QDateTimeEdit();
    m_dtEditFrom->setDisplayFormat(DT_FORMAT);
    m_ui->toolBar->addWidget(m_dtEditFrom);
    spacer = new QWidget();
    spacer->setMinimumWidth(5);
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_ui->toolBar->addWidget(spacer);
    m_dtEditTo = new QDateTimeEdit();
    m_dtEditTo->setDisplayFormat(DT_FORMAT);
    m_ui->toolBar->addWidget(m_dtEditTo);

    connect(m_ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), SLOT(xAxisRangeChanged(QCPRange)));
    connect(m_dtEditFrom, SIGNAL(dateTimeChanged(QDateTime)), SLOT(fromDateTimeChanged(QDateTime)));
    connect(m_dtEditTo, SIGNAL(dateTimeChanged(QDateTime)), SLOT(toDateTimeChanged(QDateTime)));

    setFocus();
    initializePlot();
}

PlotWindow::~PlotWindow()
{
    delete m_ui;
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

    plot->xAxis2->setLabel(m_stat.getNodesString());

    plot->axisRect()->setupFullAxesBox();
    plot->xAxis->setAutoTicks(false);
    plot->xAxis->setAutoTickLabels(false);
    plot->xAxis->setAutoSubTicks(false);
    plot->xAxis->setSubTickCount(0);
    plot->xAxis->setTickLabelRotation(90);
    plot->xAxis2->setTicks(false);
    plot->yAxis2->setTicks(false);

    plot->legend->setSelectableParts(QCPLegend::spItems);
    plot->legend->setIconSize(15, 6);
    plot->legend->setVisible(true);

    plot->setNoAntialiasingOnDrag(true);

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect | QCP::iSelectLegend | QCP::iSelectPlottables);

    connect(plot->xAxis, &QCPAxis::ticksRequest, this, &PlotWindow::adjustTicks);
    connect(plot, &QCustomPlot::selectionChangedByUser, this, &PlotWindow::selectionChanged);
    connect(plot, &QCustomPlot::mousePress, this, &PlotWindow::mousePress);
    connect(plot, &QCustomPlot::mouseMove, this, &PlotWindow::mouseMove);
    connect(plot, &QCustomPlot::mouseWheel, this, &PlotWindow::mouseWheel);
    connect(plot, &QCustomPlot::customContextMenuRequested, this, &PlotWindow::contextMenuRequest);
    connect(plot, &QCustomPlot::legendDoubleClick, this, &PlotWindow::legendDoubleClick);

    for (const QString &node : m_stat.getNodes()) {
        for (const QString &name : m_stat.getNames(node)) {
            QCPGraph *graph = plot->addGraph();
            graph->setName(m_stat.formatName(node, name));
            graph->setPen(QPen(m_colorManager.getColor()));
            graph->setSelectedPen(graph->pen());
            // Set copy to true to avoid the data being deleted if show delta function is used
            graph->setData(m_stat.getDataMap(node, name), true);
        }
    }

    m_ui->actionMarkRestartTime->setChecked(true);
    markRestartTime();

    plot->rescaleAxes();
    adjustYAxisRange(plot->yAxis);
    plot->replot();
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

    if (result.isEmpty()) {
        result << 0; // At least show one tick
    }

    updateScatter(result, plotWidth);

    return result;
}

QVector<QString> PlotWindow::calcTickLabelVector(const QVector<double> &ticks)
{
    Q_ASSERT(static_cast<int>(ticks.last()) < m_stat.dateTimeCount());

    QVector<QString> result;
    for (double index : ticks) {
        result << m_stat.getDateTimeString(index);
    }
    return result;
}

void PlotWindow::calcDelta(QCPGraph *graph)
{
    QCPDataMap *data = graph->data();
    if (data->size() > 0) {
        for (auto iter = data->end() - 1; iter != data->begin(); --iter) {
            if (m_stat.getDateTime(iter.key()) - m_stat.getDateTime((iter - 1).key()) > m_sampleInterval * 2) {
                // If timestamp difference is larger than 2 minutes then set delta to 0
                (*iter).value = 0;
            } else {
                (*iter).value = iter.value().value - (iter - 1).value().value;
            }
        }

        // The first element is always 0 in delta mode
        (*data->begin()).value = 0;
    }
}

QVector<double> PlotWindow::findRestartTimeIndex() const
{
    QVector<double> result;
    for (const QString &node : m_stat.getNodes()) {
        findRestartTimeIndexForNode(node, result);
    }

    // Remove duplicate index if any
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

void PlotWindow::findRestartTimeIndexForNode(const QString &node, QVector<double> &out) const
{
    QMap<int, qint32> map = m_stat.getIndexDateTimeMap(node);
    if (map.size() > 0) {
        for (auto iter = map.begin() + 1; iter != map.end(); ++iter) {
            if (iter.value() - (iter - 1).value() > m_sampleInterval * 2) {
                out << iter.key();
            }
        }
    }
}

void PlotWindow::markRestartTime()
{
    QVector<double> restartIndex = findRestartTimeIndex();
    if (restartIndex.size() > 0) {
        QCustomPlot *plot = m_ui->customPlot;
        QPen pen(Qt::red, 2);
        pen.setStyle(Qt::DotLine);
        for (double index : restartIndex) {
            QCPItemStraightLine *line = new QCPItemStraightLine(plot);
            line->point1->setCoords(index, std::numeric_limits<double>::min());
            line->point2->setCoords(index, std::numeric_limits<double>::max());
            line->setPen(pen);
            plot->addItem(line);
        }
    }
}

void PlotWindow::updateScatter(const QVector<double> &tickVector, int plotWidth)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (tickVector.size() > 1 && tickVector[1] - tickVector[0] == 1 && plotWidth / tickVector.size() > 20) {
        if (!m_hasScatter) {
            m_hasScatter = true;
            for (int i = 0; i < plot->graphCount(); ++i) {
                QCPGraph *graph = plot->graph(i);
                graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, graph->pen().color(), SCATTER_SIZE));
            }
        }
    } else {
        if (m_hasScatter) {
            m_hasScatter = false;
            for (int i = 0; i < plot->graphCount(); ++i) {
                QCPGraph *graph = plot->graph(i);
                graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
            }
        }
    }
}

QCPGraph * PlotWindow::findGraphValueToShow(int index, double yCoord, double &value)
{
    QCustomPlot *plot = m_ui->customPlot;
    QCPGraph *retGraph = NULL;
    double minDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph *graph = plot->graph(i);
        if (!graph->visible()) {
            continue;
        }

        QCPDataMap *data = graph->data();
        auto iter = data->find(index);
        if (iter == data->end() || !plot->yAxis->range().contains(iter->value)) {
            continue;
        }

        double yDistance = qAbs(iter->value - yCoord);
        if (yDistance < minDistance) {
            minDistance = yDistance;
            value = iter->value;
            retGraph = graph;
        }
    }
    return retGraph;
}

void PlotWindow::removeGraphs(const QVector<QCPGraph *> &graphs)
{
    if (graphs.size() > 0) {
        QCustomPlot *plot = m_ui->customPlot;
        bool updateNameAndTitle = false;
        for (QCPGraph *graph : graphs) {
            if (!graph->property("add_by_script").isValid()) {
                updateNameAndTitle = true;
                m_stat.removeDataMap(graph->name());
            }
            plot->removeGraph(graph);
        }
        if (updateNameAndTitle) {
            int preNodeCount = m_stat.getNodeCount();
            m_stat.trimNodeNameDataMap();
            if (preNodeCount > 1 && m_stat.getNodeCount() == 1) {
                for (int i = 0; i < plot->graphCount(); ++i) {
                    QCPGraph *graph = plot->graph(i);
                    if (!graph->property("add_by_script").isValid())
                        graph->setName(m_stat.removeNodePrefix(graph->name()));
                }
            }

            setWindowTitle(evaluateWindowTitle());
            plot->xAxis2->setLabel(evaluatePlotTitle(m_ui->actionShowDelta->isChecked()));
        }

        selectionChanged();
        plot->replot();
    }
}

QString PlotWindow::evaluateWindowTitle() const
{
    if (m_customTitle.isEmpty()) {
        return m_stat.getNodesString();
    }
    return m_customTitle;
}

QString PlotWindow::evaluatePlotTitle(bool deltaMode) const
{
    QString title = evaluateWindowTitle();
    if (deltaMode) {
        title += SUFFIX_DELTA;
    }
    return title;
}

QString PlotWindow::defaultSaveFileName() const
{
    QString defaultFileName;
    for (const QString &node : m_stat.getNodes()) {
        for (const QString &name : m_stat.getNames(node)) {
            if (defaultFileName.isEmpty()) {
                defaultFileName = name;
            } else if (defaultFileName != name) {
                return QString();
            }
        }
    }
    return defaultFileName.splitRef('.').back().toString();
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

    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph *graph = plot->graph(i);
        QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            graph->setSelected(true);
        }
    }

    if (plot->legend->selectedItems().isEmpty()) {
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            graph->setVisible(true);
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
            graph->setVisible(item->selected());
        }

        if (QApplication::keyboardModifiers() & Qt::AltModifier) {
            plot->yAxis->rescale(true);
            adjustYAxisRange(plot->yAxis);
        }
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
    if (!(event->modifiers() & Qt::ControlModifier)) {
        if (m_tracer->graph()) {
            m_tracer->setGraph(NULL);
            m_tracer->setVisible(false);
            m_ui->customPlot->replot();

            m_valueTip.hide();
        }
        return;
    }

    QCustomPlot *plot = m_ui->customPlot;

    QCPRange xRange = plot->xAxis->range();
    int index = qRound(plot->xAxis->pixelToCoord(event->pos().x()));
    if (index >= 0 && xRange.contains(index)) {
        double value, yCoord = plot->yAxis->pixelToCoord(event->pos().y());
        QCPGraph *graph = findGraphValueToShow(index, yCoord, value);
        if (graph) {
            m_tracer->setGraph(graph);
            m_tracer->setGraphKey(index);
            m_tracer->setVisible(true);
            plot->replot();

            QPoint toolTipPos(
                static_cast<int>(plot->xAxis->coordToPixel(index)),
                static_cast<int>(plot->yAxis->coordToPixel(value)));
            QRect rect = plot->axisRect()->rect();
            QRect rectGlobal(
                plot->mapToGlobal(rect.topLeft()),
                plot->mapToGlobal(rect.bottomRight())
            );

            QString text = m_stat.getDateTimeString(index);
            text += '\n';
            text += QString::number(value, 'f', 2);
            m_valueTip.show(plot->mapToGlobal(toolTipPos), rectGlobal, text);
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
}

void PlotWindow::contextMenuRequest(const QPoint &pos)
{
    QCustomPlot *plot = m_ui->customPlot;
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction(plot->legend->visible() ? QStringLiteral("Hide Legend") : QStringLiteral("Show Legend"),
                    this, SLOT(toggleLegendVisibility()));
    menu->addAction(QStringLiteral("Set Custom Title..."), this, SLOT(setCustomTitle()));
    QMenu *subMenu = menu->addMenu(QStringLiteral("Move Legend to"));
    QMenu *subMenuTop = subMenu->addMenu(QStringLiteral("Top"));
    QMenu *subMenuBottom = subMenu->addMenu(QStringLiteral("Bottom"));
    subMenuTop->addAction(QStringLiteral("Left"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignLeft));
    subMenuTop->addAction(QStringLiteral("Center"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignCenter));
    subMenuTop->addAction(QStringLiteral("Right"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignTop | Qt::AlignRight));
    subMenuBottom->addAction(QStringLiteral("Left"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignLeft));
    subMenuBottom->addAction(QStringLiteral("Center"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignCenter));
    subMenuBottom->addAction(QStringLiteral("Right"), this, SLOT(moveLegend()))->setData(
        static_cast<int>(Qt::AlignBottom | Qt::AlignRight));

    QAction *actionCopy = menu->addAction(QStringLiteral("Copy Graph Name"), this, SLOT(copyGraphName()));
    QAction *actionRemove = menu->addAction(QStringLiteral("Remove Selected Graphs"), this, SLOT(removeSelectedGraph()));
    if (!plot->legend->selectedItems().size()) {
        actionCopy->setEnabled(false);
        actionRemove->setEnabled(false);
    }

    menu->popup(plot->mapToGlobal(pos));
}

void PlotWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    Q_UNUSED(legend)

    if (item) {
        QString node, name;
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        m_stat.parseFormattedName(plItem->plottable()->name(), node, name);

        bool ok;
        QString newName = QInputDialog::getText(this, QStringLiteral("Input graph name"),
                                                QStringLiteral("New graph name:"),
                                                QLineEdit::Normal,
                                                name, &ok);
        if (ok && !newName.isEmpty() && newName.indexOf(':') < 0 &&
                m_stat.renameDataMap(node, name, newName))
        {
            plItem->plottable()->setName(m_stat.formatName(node, newName));
            m_ui->customPlot->replot();
        }
    }
}

void PlotWindow::moveLegend()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        bool ok;
        int align = action->data().toInt(&ok);
        if (ok) {
            QCustomPlot *plot = m_ui->customPlot;
            plot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)align);
            plot->replot();
        }
    }
}

void PlotWindow::removeSelectedGraph()
{
    QVector<QCPGraph*> graphsToBeRemoved;
    QCustomPlot *plot = m_ui->customPlot;
    auto selectedItems = plot->legend->selectedItems();
    for (int i = 0; i < plot->legend->itemCount(); ++i) {
        auto item = plot->legend->item(i);
        if (selectedItems.contains(item)) {
            graphsToBeRemoved << static_cast<QCPGraph*>(static_cast<QCPPlottableLegendItem*>(item)->plottable());
        }
    }

    removeGraphs(graphsToBeRemoved);
}

void PlotWindow::copyGraphName()
{
    QStringList strList;
    for (auto item : m_ui->customPlot->legend->selectedItems()) {
        strList << qobject_cast<QCPPlottableLegendItem *>(item)->plottable()->name();
    }
    QApplication::clipboard()->setText(strList.join('\n'));
}

void PlotWindow::setCustomTitle()
{
    bool ok;
    QString title = QInputDialog::getText(this, QStringLiteral("Set Custom Title"),
        QStringLiteral("Custom Title:"),
        QLineEdit::Normal,
        QString(), &ok);
    if (ok) {
        m_customTitle = title;
        setWindowTitle(evaluateWindowTitle());
        m_ui->customPlot->xAxis2->setLabel(evaluatePlotTitle(m_ui->actionShowDelta->isChecked()));
        m_ui->customPlot->replot();
    }
}

void PlotWindow::toggleLegendVisibility()
{
    QCustomPlot *plot = m_ui->customPlot;
    plot->legend->setVisible(!plot->legend->visible());
    plot->replot();
}

void PlotWindow::xAxisRangeChanged(const QCPRange &newRange)
{
    if (m_userDragFlag) {
        int dtFrom, dtTo;
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

        m_userEditFlag = false;
        m_dtEditFrom->setDateTime(QDateTime::fromTime_t(dtFrom));
        m_dtEditTo->setDateTime(QDateTime::fromTime_t(dtTo));
        m_userEditFlag = true;
    }
}

void PlotWindow::fromDateTimeChanged(const QDateTime &dateTime)
{
    if (m_userEditFlag) {
        int dt = (int)dateTime.toTime_t();
        int index = m_stat.firstGreaterDateTimeIndex(dt);
        if (index >= 0) {
            m_userDragFlag = false;
            if (index != 0) {
                m_ui->customPlot->xAxis->setRangeLower(index - 1);
            } else {
                m_ui->customPlot->xAxis->setRangeLower(0);
            }
            m_userDragFlag = true;
            m_ui->customPlot->replot();
        }
    }
}

void PlotWindow::toDateTimeChanged(const QDateTime &dateTime)
{
    if (m_userEditFlag) {
        int dt = (int)dateTime.toTime_t();
        int index = m_stat.firstGreaterDateTimeIndex(dt);
        if (index >= 0) {
            m_userDragFlag = false;
            m_ui->customPlot->xAxis->setRangeUpper(index);
            m_userDragFlag = true;
            m_ui->customPlot->replot();
        }
    }
}

void PlotWindow::on_actionSaveAsImage_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save As Image"),
                                                    defaultSaveFileName(),
                                                    QStringLiteral("PNG File (*.png)"));
    if (!path.isEmpty()) {
        m_ui->customPlot->savePng(path);
    }
}

void PlotWindow::on_actionRestoreScale_triggered()
{
    m_ui->customPlot->rescaleAxes();
    adjustYAxisRange(m_ui->customPlot->yAxis);
    m_ui->customPlot->replot();
}

void PlotWindow::on_actionShowDelta_toggled(bool checked)
{
    QCustomPlot *plot = m_ui->customPlot;
    plot->xAxis2->setLabel(evaluatePlotTitle(checked));

    if (checked) {
        // The script added graphs are not show in delta mode
        for (int i = 0; i < m_stat.totalNameCount(); ++i) {
            calcDelta(plot->graph(i));
        }
    } else {
        for (int i = 0; i < m_stat.totalNameCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            // Set copy to true to avoid the data being deleted if show delta function is used
            graph->setData(m_stat.getDataMap(graph->name()), true);
        }
    }
    // Only rescale Y axis
    plot->yAxis->rescale();
    adjustYAxisRange(plot->yAxis);
    plot->replot();
}

void PlotWindow::on_actionScript_triggered()
{
    ScriptWindow *scriptWindow = findChild<ScriptWindow*>(QStringLiteral("ScriptWindow"), Qt::FindDirectChildrenOnly);
    if (scriptWindow) {
        if (scriptWindow->isMinimized()) {
            scriptWindow->showNormal();
        }
    } else {
        scriptWindow = new ScriptWindow(this);
        QString err;
        if (!scriptWindow->initialize(err)) {
            showInfoMsgBox(this, QStringLiteral("Initialize script window failed."), err);
            delete scriptWindow;
        }
        scriptWindow->setAttribute(Qt::WA_DeleteOnClose);
        scriptWindow->showNormal();
    }
}

void PlotWindow::on_actionRemoveZeroCounters_triggered()
{
    QVector<QCPGraph*> graphsToBeRemoved;
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph *graph = plot->graph(i);
        QCPDataMap *dataMap;
        if (graph->property("add_by_script").isValid()) {
            dataMap = graph->data();
        } else {
            // Also remove the counters that delta is zero
            dataMap = m_ui->actionShowDelta->isChecked() ? graph->data() : m_stat.getDataMap(graph->name());
        }
        bool isZeroCounter = true;
        for (const QCPData &data : *dataMap) {
            if (static_cast<int>(data.value) != 0) {
                isZeroCounter = false;
                break;
            }
        }
        if (isZeroCounter) {
            graphsToBeRemoved << graph;
        }
    }

    removeGraphs(graphsToBeRemoved);
}

void PlotWindow::on_actionCopyToClipboard_triggered()
{
    QApplication::clipboard()->setPixmap(m_ui->customPlot->toPixmap());
}

void PlotWindow::on_actionMarkRestartTime_triggered(bool checked)
{
    if (checked) {
        markRestartTime();
    } else {
        m_ui->customPlot->clearItems();
    }
    m_ui->customPlot->replot();
}

void PlotWindow::on_actionSetSampleInterval_triggered()
{
    bool ok;
    int interval = QInputDialog::getInt(this, QStringLiteral("Set Sample Interval"),
                                        QStringLiteral("Sample Interval (secs):"),
                                        m_sampleInterval, 60, 3600, 60, &ok);
    if (ok) {
        m_sampleInterval = interval;
        m_ui->customPlot->clearItems();
        markRestartTime();
        m_ui->customPlot->replot();
    }
}
