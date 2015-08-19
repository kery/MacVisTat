#include "plotwindow.h"
#include "ui_plotwindow.h"

struct DataColor
{
    DataColor(const QColor &p, const QColor &b) :
        pen(p),
        brush(b)
    {
    }

    const QColor pen;
    const QColor brush;
};

static DataColor dataColors[] = {
    DataColor(QColor(Qt::red), QColor(255, 0, 0, 50)),
    DataColor(QColor(Qt::green), QColor(0, 255, 0, 50)),
    DataColor(QColor(Qt::blue), QColor(0, 0, 255, 50)),
    DataColor(QColor(255, 255, 0), QColor(255, 255, 0, 50)),
    DataColor(QColor(255, 0, 255), QColor(255, 0, 255, 50)),
    DataColor(QColor(0, 255, 255), QColor(0, 255, 255, 50)),
};

int PlotWindow::predefinedColorCount()
{
    return sizeof(dataColors)/sizeof(dataColors[0]);
}

PlotWindow::PlotWindow(const QString &node, const QMap<QString, QCPDataMap> &result, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::PlotWindow),
    _node(node)
{
    _ui->setupUi(this);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _ui->toolBar->addWidget(spacer);
    _dtEditFrom = new QDateTimeEdit();
    _dtEditFrom->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
    _ui->toolBar->addWidget(_dtEditFrom);
    spacer = new QWidget();
    spacer->setMinimumWidth(5);
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    _ui->toolBar->addWidget(spacer);
    _dtEditTo = new QDateTimeEdit();
    _dtEditTo->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
    _ui->toolBar->addWidget(_dtEditTo);
    connect(_ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), SLOT(xAxisRangeChanged(QCPRange)));
    connect(_dtEditFrom, SIGNAL(dateTimeChanged(QDateTime)), SLOT(fromDateTimeChanged(QDateTime)));
    connect(_dtEditTo, SIGNAL(dateTimeChanged(QDateTime)), SLOT(toDateTimeChanged(QDateTime)));

    convertResultFirstData(result);
    convertResultRestData(result);

    initializePlot();
}

PlotWindow::~PlotWindow()
{
    delete _ui;
}

void PlotWindow::convertResultFirstData(const QMap<QString, QCPDataMap> &result)
{
    double i = 0;
    const QCPDataMap &srcDataMap = result.first();
    QCPDataMap destDataMap;
    QCPData data;
    for (auto iter = srcDataMap.begin(); iter != srcDataMap.end(); ++iter) {
        _dateTimes << iter.key();
        data.key = i++;
        data.value = iter.value().value;
        destDataMap.insert(data.key, data);
    }
    _result.insert(result.firstKey(), destDataMap);
}

void PlotWindow::convertResultRestData(const QMap<QString, QCPDataMap> &result)
{
    for (auto iter = result.begin() + 1; iter != result.end(); ++iter) {
        double i = 0;
        const QCPDataMap &srcDataMap = iter.value();
        QCPDataMap destDataMap;
        QCPData data;
        for (auto dataIter = srcDataMap.begin(); dataIter != srcDataMap.end(); ++dataIter) {
            data.key = i++;
            data.value = dataIter.value().value;
            destDataMap.insert(data.key, data);
        }
        _result.insert(iter.key(), destDataMap);
    }
}

void PlotWindow::initializePlot()
{
    QCustomPlot *plot = _ui->customPlot;

    plot->axisRect()->setupFullAxesBox();
    plot->xAxis->setAutoTicks(false);
    plot->xAxis->setAutoTickLabels(false);
    plot->xAxis->setAutoSubTicks(false);
    plot->xAxis->setSubTickCount(0);
    plot->xAxis->setTickLabelRotation(90);
    plot->xAxis2->setTicks(false);
    plot->yAxis2->setTicks(false);

    plot->legend->setSelectableParts(QCPLegend::spItems);
    plot->legend->setVisible(true);

    plot->setNoAntialiasingOnDrag(true);

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                          QCP::iSelectPlottables | QCP::iSelectLegend);

    connect(plot->xAxis, &QCPAxis::ticksRequest, this, &PlotWindow::adjustTicks);
    connect(plot, &QCustomPlot::selectionChangedByUser, this, &PlotWindow::selectionChanged);
    connect(plot, &QCustomPlot::mousePress, this, &PlotWindow::mousePress);
    connect(plot, &QCustomPlot::mouseWheel, this, &PlotWindow::mouseWheel);
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, &QCustomPlot::customContextMenuRequested, this, &PlotWindow::contextMenuRequest);

    Q_ASSERT(_result.size() <= static_cast<int>(sizeof(dataColors)/sizeof(dataColors[0])));

    int i = 0;
    QSettings settings;
    bool fillPlot = settings.value(QStringLiteral("PlotWindow/FillPlot"), true).toBool();
    _ui->actionFillPlot->setChecked(fillPlot);
    for (auto iter = _result.begin(); iter != _result.end(); ++iter, ++i) {
        plot->addGraph();
        plot->graph()->setName(iter.key());
        plot->graph()->setPen(QPen(dataColors[i].pen));
        if (fillPlot) {
            plot->graph()->setBrush(QBrush(dataColors[i].brush));
        }
        // Set copy to true to avoid the data being deleted if show delta function is used
        plot->graph()->setData(const_cast<QCPDataMap*>(&iter.value()), true);
    }

    bool markAbnormal = settings.value(QStringLiteral("PlotWindow/MarkAbnormal"), false).toBool();
    _ui->actionMarkAbnormalTime->setChecked(markAbnormal);
    if (markAbnormal) {
        markAbnormalTime();
    }

    plot->rescaleAxes(true);
    plot->replot();
}

QVector<double> PlotWindow::calcTickVector(int plotWidth, int fontHeight, const QCPRange &range)
{
    QVector<double> result;
    int count = plotWidth / (fontHeight * 1.2);
    if (count > 0) {
        int step = qMax(1, static_cast<int>(range.size() / count));
        int upper = qMin(static_cast<int>(range.upper), _dateTimes.size());
        for (int i = qMax(static_cast<int>(range.lower), 0); i < upper; i += step) {
            result << i;
        }
    }

    if (result.isEmpty()) {
        result << 0; // At least show one tick
    }

    return result;
}

QVector<QString> PlotWindow::calcTickLabelVector(const QVector<double> &ticks)
{
    Q_ASSERT(static_cast<int>(ticks.last()) < _dateTimes.size());

    QVector<QString> result;
    for (double d : ticks) {
        result << QDateTime::fromTime_t(
            _dateTimes.at(static_cast<int>(d))).toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
    }
    return result;
}

void PlotWindow::calcDelta(QCPGraph *graph)
{
    QCPDataMap *data = graph->data();
    if (data->size() > 0) {
        int i = data->size() - 1;
        for (auto iter = data->end() - 1; iter != data->begin(); --iter, --i) {
            if (_dateTimes.at(i) - _dateTimes.at(i - 1) > 60 * 2) {
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

QVector<int> PlotWindow::findAbnormalTimeIndex() const
{
    QVector<int> result;
    for (int i = 1; i < _dateTimes.size(); ++i) {
        if (_dateTimes.at(i) - _dateTimes.at(i - 1) > 60 * 2) {
            result << i;
        }
    }
    return result;
}

void PlotWindow::markAbnormalTime()
{
    QVector<int> abnormalIndex = findAbnormalTimeIndex();
    if (abnormalIndex.size() > 0) {
        QCustomPlot *plot = _ui->customPlot;
        QPen pen(Qt::red);
        pen.setStyle(Qt::DotLine);
        for (int index : abnormalIndex) {
            QCPItemStraightLine *line = new QCPItemStraightLine(plot);
            line->point1->setCoords(index, 0);
            line->point2->setCoords(index, 100);
            line->setPen(pen);
            plot->addItem(line);
        }
    }
}

void PlotWindow::adjustTicks()
{
    QCustomPlot *plot = _ui->customPlot;
    const QCPRange range = plot->xAxis->range();
    int fontHeight = QFontMetrics(plot->xAxis->tickLabelFont()).height();

    QVector<double> ticks = calcTickVector(plot->size().width(), fontHeight, range);
    plot->xAxis->setTickVector(ticks);
    plot->xAxis->setTickVectorLabels(calcTickLabelVector(ticks));
}

void PlotWindow::selectionChanged()
{
    QCustomPlot *plot = _ui->customPlot;

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis))
    {
        plot->xAxis->setSelectedParts(QCPAxis::spAxis);
        plot->xAxis2->setSelectedParts(QCPAxis::spAxis);
    }

    if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis))
    {
        plot->yAxis->setSelectedParts(QCPAxis::spAxis);
        plot->yAxis2->setSelectedParts(QCPAxis::spAxis);
    }

    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph *graph = plot->graph(i);
        QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            graph->setSelected(true);
        }
    }
}

void PlotWindow::mousePress(QMouseEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    // also support key modifiers
    QCustomPlot *plot = _ui->customPlot;

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || event->modifiers() & Qt::ControlModifier) {
        plot->axisRect()->setRangeDrag(plot->xAxis->orientation());
    } else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || event->modifiers() & Qt::ShiftModifier) {
        plot->axisRect()->setRangeDrag(plot->yAxis->orientation());
    } else {
        plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
}

void PlotWindow::mouseWheel(QWheelEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed
    // also support key modifiers
    QCustomPlot *plot = _ui->customPlot;

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || event->modifiers() & Qt::ControlModifier) {
        plot->axisRect()->setRangeZoom(plot->xAxis->orientation());
    } else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || event->modifiers() & Qt::ShiftModifier) {
        plot->axisRect()->setRangeZoom(plot->yAxis->orientation());
    } else {
        plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }
}

void PlotWindow::contextMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QCustomPlot *plot = _ui->customPlot;
    if (plot->legend->selectTest(pos, false) >= 0) {
        menu->addAction(QStringLiteral("Move to top left"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignTop | Qt::AlignLeft));
        menu->addAction(QStringLiteral("Move to top center"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignTop | Qt::AlignCenter));
        menu->addAction(QStringLiteral("Move to top right"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignTop | Qt::AlignRight));
        menu->addAction(QStringLiteral("Move to bottom left"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignBottom | Qt::AlignLeft));
        menu->addAction(QStringLiteral("Move to bottom center"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignBottom | Qt::AlignCenter));
        menu->addAction(QStringLiteral("Move to bottom right"), this, SLOT(moveLegend()))->setData(
            static_cast<int>(Qt::AlignBottom | Qt::AlignRight));
    }

    menu->popup(plot->mapToGlobal(pos));
}

void PlotWindow::moveLegend()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        bool ok;
        int align = action->data().toInt(&ok);
        if (ok) {
            QCustomPlot *plot = _ui->customPlot;
            plot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)align);
            plot->replot();
        }
    }
}

void PlotWindow::xAxisRangeChanged(const QCPRange &newRange)
{
    int indexFrom = (int)newRange.lower;
    int indexTo = (int)newRange.upper;
    if (indexFrom < 0) {
        indexFrom = 0;
    }
    if (indexTo >= _dateTimes.size()) {
        indexTo = _dateTimes.size() - 1;
    }
    _dtEditFrom->setDateTime(QDateTime::fromTime_t(_dateTimes.at(indexFrom)));
    _dtEditTo->setDateTime(QDateTime::fromTime_t(_dateTimes.at(indexTo)));
}

void PlotWindow::fromDateTimeChanged(const QDateTime &dateTime)
{
    int time = (int)dateTime.toTime_t();
    int i;
    for (i = 0; i < _dateTimes.size(); ++i) {
        if (_dateTimes.at(i) >= time) {
            break;
        }
    }
    if (i < _dateTimes.size()) {
        _ui->customPlot->xAxis->setRangeLower(i);
        _ui->customPlot->replot();
    }
}

void PlotWindow::toDateTimeChanged(const QDateTime &dateTime)
{
    int time = (int)dateTime.toTime_t();
    int i;
    for (i = _dateTimes.size() - 1; i >= 0; --i) {
        if (_dateTimes.at(i) <= time) {
            break;
        }
    }

    if (i >= 0) {
        _ui->customPlot->xAxis->setRangeUpper(i);
        _ui->customPlot->replot();
    }
}

void PlotWindow::on_actionFullScreen_toggled(bool checked)
{
    if (checked) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void PlotWindow::on_actionSaveAsImage_triggered()
{
    QString dir = QDir::cleanPath(qApp->applicationDirPath() + QDir::separator() + _node);

    if (_result.size() == 1) {
        dir += QString("-%1").arg(_result.firstKey());
    }

    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Save As Image"),
                                                    dir,
                                                    QStringLiteral("PNG File (*.png)"));
    _ui->customPlot->savePng(fileName);
}

void PlotWindow::on_actionRestoreScale_triggered()
{
    _ui->customPlot->rescaleAxes(true);
    _ui->customPlot->replot();
}

void PlotWindow::on_actionShowDelta_toggled(bool checked)
{
    const char *SUFFIX = "(DELTA)";
    const int SUFFIX_LEN = 7;
    QCustomPlot *plot = _ui->customPlot;
    if (checked) {
        for (int i = 0; i < plot->graphCount(); ++i) {
            QCPGraph *graph = plot->graph(i);
            graph->setName(graph->name() + SUFFIX);
            calcDelta(graph);
        }
    } else {
        int i = 0;
        for (auto iter = _result.begin(); iter != _result.end(); ++iter, ++i) {
            QCPGraph *graph = plot->graph(i);
            QString name = graph->name();
            graph->setName(name.left(name.length() - SUFFIX_LEN));
            // Set copy to true to avoid the data being deleted if show delta function is used
            graph->setData(const_cast<QCPDataMap*>(&iter.value()), true);
        }
    }
    plot->rescaleAxes(true);
    plot->replot();
}

void PlotWindow::on_actionFillPlot_toggled(bool checked)
{
    QCustomPlot *plot = _ui->customPlot;
    if (checked) {
        for (int i = 0; i < plot->graphCount(); ++i) {
            plot->graph(i)->setBrush(dataColors[i].brush);
        }
    } else {
        for (int i = 0; i < plot->graphCount(); ++i) {
            plot->graph(i)->setBrush(Qt::NoBrush);
        }
    }
    plot->replot();
    QSettings().setValue(QStringLiteral("PlotWindow/FillPlot"), checked);
}

void PlotWindow::on_actionMarkAbnormalTime_toggled(bool checked)
{
    if (checked) {
        markAbnormalTime();
    } else {
        _ui->customPlot->clearItems();
    }
    _ui->customPlot->replot();
    QSettings().setValue(QStringLiteral("PlotWindow/MarkAbnormal"), checked);
}
