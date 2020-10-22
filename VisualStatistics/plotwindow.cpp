#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "scriptwindow.h"
#include "utils.h"
#include "version.h"

const double PlotWindow::TracerSize = CounterGraph::ScatterSize + 4.0;
const int PlotWindow::AnimationMaxGraphs = 100;

PlotWindow::PlotWindow(Statistics &stat) :
    QMainWindow(nullptr),
    m_agggraph_idx(0),
    m_lastSelLegitemIndex(-1),
    m_ui(new Ui::PlotWindow),
    m_userEditFlag(true),
    m_userDragFlag(true),
    m_hasScatter(false),
    m_showModule(false),
    m_stat(std::move(stat))
{
    m_ui->setupUi(this);

    // Show tracer below legend layer, above other layers
    // Value tip will use the same layer as tracer
    m_ui->customPlot->addLayer(QStringLiteral("valuetip"), 0, QCustomPlot::limBelow);

    // Must be called after setupUi because member customPlot is initialized
    // in it. QCustomPlot takes ownership of tracer.
    m_tracer = new QCPItemTracer(m_ui->customPlot);
    m_tracer->setInterpolating(true);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setSelectable(false);
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
            m_tracer->setGraph(nullptr);
            this->m_ui->customPlot->replot(QCustomPlot::rpQueued);
        }
    });

    m_valueText = new ValueText(m_tracer);

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

    updateWindowTitle();
    updatePlotTitle();
}

PlotWindow::~PlotWindow()
{
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
    QString formatString;
    QCustomPlot *plot = m_ui->customPlot;
    if (m_ui->actionShowDelta->isChecked()) {
        formatString = "%1 (%2, DELTA)";
    } else {
        formatString = "%1 (%2)";
    }
    plot->xAxis2->setLabel(formatString.arg(windowTitle()).arg(plot->graphCount()));
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
    plot->legend->setIconSize(15, 6);
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
    connect(plot, &QCustomPlot::legendDoubleClick, this, &PlotWindow::legendDoubleClick);

    QSettings settings;
    bool showSuspectFlag = settings.value(QStringLiteral("showSuspectFlag"), true).toBool();
    if (showSuspectFlag) {
        m_ui->actionShowSuspectFlag->setChecked(true);
    }

    for (const QString &statName : m_stat.getNames()) {
        QString name;
        QString module = Statistics::splitStatNameToModuleAndName(statName, name);
        CounterGraph *graph = addCounterGraph(name, module);
        graph->setName(statName);
        // Set copy to true to avoid the data being deleted if show delta function is used
        graph->setData(m_stat.getDataMap(statName), true);
        if (showSuspectFlag) {
            graph->enableSuspectFlag(true);
        }
    }

    plot->rescaleAxes();
    adjustYAxisRange(plot->yAxis);
    plot->replot(QCustomPlot::rpQueued);
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

    updateScatter(result, plotWidth);

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

bool PlotWindow::shouldDrawScatter(const QVector<double> &tickVector, int plotWidth) const
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

    const int MIN_SPACE = 20;

    return (plotWidth / tickCount) > MIN_SPACE;
}

void PlotWindow::updateScatter(const QVector<double> &tickVector, int plotWidth)
{
    QCustomPlot *plot = m_ui->customPlot;

    if (shouldDrawScatter(tickVector, plotWidth)) {
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

QCPGraph * PlotWindow::findNearestGraphValue(int index, double yCoord, double &value)
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
        plot->removeGraph(graph);
    }

    updateWindowTitle();
    updatePlotTitle();

    selectionChanged();
    plot->replot(QCustomPlot::rpQueued);
}

QVector<CounterGraph *> PlotWindow::selectedGraphs(bool selected) const
{
    QVector<CounterGraph*> graphs;
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        if (graph->selected() == selected) {
            graphs.append(graph);
        }
    }
    return graphs;
}

QString PlotWindow::defaultSaveFileName() const
{
    if (m_ui->customPlot->graphCount() > 0) {
         CounterGraph *graph = qobject_cast<CounterGraph *>(m_ui->customPlot->graph(0));
         return graph->displayName().splitRef('.').back().toString();
    }
    return QString();
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
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            graph->setVisible(graph->selected());
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

    const double MAX_DIST = 20;

    QCPGraph *graph = nullptr;
    double value, dist = MAX_DIST;

    int index = qRound(plot->xAxis->pixelToCoord(event->pos().x()));
    QCPRange xRange = plot->xAxis->range();

    if (xRange.contains(index)) {
        double yCoord = plot->yAxis->pixelToCoord(event->pos().y());
        graph = findNearestGraphValue(index, yCoord, value);
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
                plot->replot(QCustomPlot::rpQueued);
            }
        }
    } else if (graph && (graph != m_tracer->graph() || (int)m_tracer->graphKey() != index)) {
        if (graph != m_tracer->graph()) {
            QColor color = graph->pen().color();
            QPen pen(color.darker(128));
            pen.setWidth(2);

            m_tracer->setPen(pen);
            m_tracer->setBrush(color);
        }

        m_tracer->setGraph(graph);
        m_tracer->setGraphKey(index);
        m_tracer->setVisible(true);

        m_valueText->setGraphName(qobject_cast<CounterGraph *>(graph)->displayName());
        m_valueText->setDateTime(m_stat.getDateTimeString(index));
        m_valueText->setGraphValue(QString::number(value, 'f', 2));
        m_valueText->updateText();
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

void PlotWindow::contextMenuRequest(const QPoint &pos)
{
    QCustomPlot *plot = m_ui->customPlot;
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *actionShowLegend = menu->addAction(QStringLiteral("Show Legend"), this, &PlotWindow::showLegendTriggered);
    actionShowLegend->setCheckable(true);
    actionShowLegend->setChecked(plot->legend->visible());

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

    QAction *actionShowModuleName = menu->addAction(QStringLiteral("Show Module Name"), this, &PlotWindow::showModuleNameTriggered);
    actionShowModuleName->setCheckable(true);
    actionShowModuleName->setChecked(m_showModule);

    menu->addSeparator();

    QAction *actionAddGraph = menu->addAction(QStringLiteral("Add Aggregate Graph"), this, SLOT(addAggregateGraph()));
    QAction *actionCopyName = menu->addAction(QStringLiteral("Copy Graph Name"), this, SLOT(copyGraphName()));
    QAction *actionCopyValue = menu->addAction(QStringLiteral("Copy Graph Value"), this, SLOT(copyGraphValue()));

    menu->addSeparator();

    QAction *actionRemove = menu->addAction(QStringLiteral("Remove Selected Graphs"), this, SLOT(removeSelectedGraphs()));
    QAction *actionRemoveUnsel = menu->addAction(QStringLiteral("Remove Unselected Graphs"), this, SLOT(removeUnselectedGraphs()));

    auto selectedLegendItems = plot->legend->selectedItems();
    if (selectedLegendItems.isEmpty()) {
        if (!m_valueText->visible()) {
            actionCopyName->setEnabled(false);
        }
        actionRemove->setEnabled(false);
        actionRemoveUnsel->setEnabled(false);
    }

    if (plot->graphCount() < 2) {
        actionAddGraph->setEnabled(false);
    }

    if (!m_valueText->visible()) {
        actionCopyValue->setEnabled(false);
    }

    menu->popup(plot->mapToGlobal(pos));
}

void PlotWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    Q_UNUSED(legend)

    if (item) {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        CounterGraph *graph = qobject_cast<CounterGraph *>(plItem->plottable());

        QColor color = QColorDialog::getColor(graph->pen().color(), this);
        if (color.isValid()) {
            graph->setPen(color);
            graph->setSelectedPen(graph->pen());

            QCPScatterStyle scatterStyle = graph->scatterStyle();
            if (scatterStyle.shape() != QCPScatterStyle::ssNone) {
                scatterStyle.setPen(color);
                graph->setScatterStyle(scatterStyle);
            }

            m_ui->customPlot->replot(QCustomPlot::rpQueued);
        }
    }
}

QRectF translateToInsetRect(QCPLayoutInset *layout, const QRectF &rect)
{
    return QRectF(rect.x() / (layout->rect().width()),
                  rect.y() / (layout->rect().height()),
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

    if (plot->graphCount() <= AnimationMaxGraphs) {
        QRect originRect = plot->legend->outerRect();
        originRect.translate(-layout->rect().x(), -layout->rect().y());

        layout->updateLayout();

        QRect newRect = plot->legend->outerRect();
        newRect.translate(-layout->rect().x(), -layout->rect().y());

        QVariantAnimation *anim = new QVariantAnimation();
        anim->setDuration(250);
        anim->setStartValue(originRect);
        anim->setEndValue(newRect);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        connect(anim, &QVariantAnimation::valueChanged, [layout, plot] (const QVariant &value) {
            QRectF rect = translateToInsetRect(layout, value.toRect());
            layout->setInsetRect(0, rect);
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
    QList<QCPAbstractLegendItem *> selectedLegendItems = plot->legend->selectedItems();

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

void PlotWindow::removeSelectedGraphs()
{
    removeGraphs(selectedGraphs(true));
}

void PlotWindow::removeUnselectedGraphs()
{
    removeGraphs(selectedGraphs(false));
}

void PlotWindow::copyGraphName()
{
    if (m_valueText->visible()) {
        QApplication::clipboard()->setText(m_valueText->graphName());
    } else {
        QStringList strList;
        for (auto item : m_ui->customPlot->legend->selectedItems()) {
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
            m_ui->customPlot->replot(QCustomPlot::rpQueued);
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
            m_ui->customPlot->replot(QCustomPlot::rpQueued);
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
    m_ui->customPlot->replot(QCustomPlot::rpQueued);
}

void PlotWindow::on_actionShowDelta_triggered(bool checked)
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

void PlotWindow::on_actionShowSuspectFlag_triggered(bool checked)
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
    QVector<CounterGraph*> graphs;
    QCustomPlot *plot = m_ui->customPlot;
    for (int i = 0; i < plot->graphCount(); ++i) {
        CounterGraph *graph = qobject_cast<CounterGraph *>(plot->graph(i));
        QCPDataMap *dataMap = graph->data();

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

void PlotWindow::on_actionCopyToClipboard_triggered()
{
    QApplication::clipboard()->setPixmap(m_ui->customPlot->toPixmap());
}
