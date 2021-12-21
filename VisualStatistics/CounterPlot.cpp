#include "CounterPlot.h"
#include "CounterGraph.h"
#include "CounterLegendItem.h"
#include "CounterDescription.h"
#include "DateTimeTicker.h"
#include "CommentItem.h"
#include "QCustomPlot/src/selectionrect.h"
#include "QCustomPlot/src/layoutelements/layoutelement-axisrect.h"

#define MIME_TYPE_LEGEND_DRAGGING  "application/visualstat-legend"
#define MIME_TYPE_COMMENT_DRAGGING "application/visualstat-comment"

CounterPlot::CounterPlot(QWidget *parent) :
    QCustomPlot(parent),
    mCommentItem(nullptr)
{
    QColor color(70, 50, 200);
    QPen pen(Qt::DashLine);
    QSharedPointer<DateTimeTicker> ticker(new DateTimeTicker(xAxis));
    pen.setColor(color);
    selectionRect()->setPen(pen);
    color.setAlpha(30);
    selectionRect()->setBrush(color);
    axisRect()->setupFullAxesBox();
    xAxis2->setTicks(false);
    xAxis2->setTickLabels(false);
    yAxis2->setTicks(false);
    yAxis2->setTickLabels(false);
    xAxis->setTicker(ticker);
    color = legend->brush().color();
    color.setAlpha(200);
    legend->setIconSize(15, 8);
    legend->setBrush(QBrush(color));
    legend->setSelectableParts(QCPLegend::spItems);
    legend->setVisible(true);

    setNoAntialiasingOnDrag(true);
    setAutoAddPlottableToLegend(false);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect | QCP::iSelectPlottables |
                    QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectItems);

    setAcceptDrops(true);
    invalidateDragStartPos();
}

CounterPlot::~CounterPlot()
{
    clearGraphs();
}

CounterGraph * CounterPlot::addGraph()
{
    auto ticker = qSharedPointerDynamicCast<DateTimeTicker>(xAxis->ticker());
    CounterGraph *graph = new CounterGraph(xAxis, yAxis);
    CounterLegendItem *item = new CounterLegendItem(legend, graph);
    graph->setScatterVisible(ticker->skippedTicks() == 0);
    legend->addItem(item);
    connect(item, &QCPPlottableLegendItem::selectionChanged, graph, &CounterGraph::setSelected);
    connect(graph, QOverload<bool>::of(&CounterGraph::selectionChanged), item, &QCPPlottableLegendItem::setSelected);
    return graph;
}

CounterGraph * CounterPlot::graph(int index) const
{
    return qobject_cast<CounterGraph *>(QCustomPlot::graph(index));
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

int CounterPlot::selectedGraphCount() const
{
    int result = 0;
    for (int i = 0; i < graphCount(); ++i) {
        if (graph(i)->selected()) { ++result; }
    }
    return result;
}

QList<CounterGraph *> CounterPlot::selectedGraphs() const
{
    QList<CounterGraph*> result;
    const QList<QCPGraph*> graphs = QCustomPlot::selectedGraphs();
    for (QCPGraph *graph : graphs) {
        result.append(qobject_cast<CounterGraph *>(graph));
    }
    return result;
}

CommentItem * CounterPlot::commentItemAt(const QPointF &pos, bool onlyVisible) const
{
    for (int i = itemCount() - 1; i >= 0 ; --i) {
        CommentItem *ci = qobject_cast<CommentItem *>(item(i));
        if (!ci || (onlyVisible && !ci->visible())) {
            continue;
        }
        if (ci->selectTest(pos, false) > 0) {
            return ci;
        }
    }
    return nullptr;
}

bool CounterPlot::pointInVisibleLegend(const QPoint &pos)
{
    return legend->visible() && legend->selectTest(pos, false) > 0;
}

void CounterPlot::setCounterDescription(CounterDescription *desc)
{
    mCounterDesc = desc;
}

void CounterPlot::resizeEvent(QResizeEvent * /*event*/)
{
    setViewport(rect());
    // DateTimeTicker's createTickVector method depends on geometry of the axis rect, so call
    // updateLayout to update it.
    updateLayout();
    replot(rpQueuedRefresh);
}

void CounterPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Don't call QCustomPlot::mousePressEvent(event) when starting drag legend or comment
        // item, so that the QCustomPlot widget keeps in normal state, not in range dragging state.
        if (pointInVisibleLegend(event->pos())) {
            mDragStartPos = event->pos();

            // To make legend item selection work.
            mMouseHasMoved = false;
            mMousePressPos = event->pos();
            return;
        }
        if ((mCommentItem = commentItemAt(event->pos(), true)) != nullptr) {
            mDragStartPos = event->pos();
            return;
        }
    }
    invalidateDragStartPos();

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

void CounterPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || !hasValidDragStartPos()) {
        QCustomPlot::mouseMoveEvent(event);
        return;
    }

    QColor color(100, 100, 100);
    if (pointInVisibleLegend(event->pos())) {
        QPoint hotSpot;
        QRect rect = legend->outerRect();
        QPixmap pixmap(rect.width(), calcLegendPixmapHeight(hotSpot));
        pixmap.fill(color);

        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << mDragStartPos - rect.topLeft();

        QMimeData *mimeData = new QMimeData();
        mimeData->setData(MIME_TYPE_LEGEND_DRAGGING, itemData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(hotSpot);
        drag->exec();
    } else if (mCommentItem) {
        QPoint hotSpot(event->pos() - mCommentItem->topLeft->pixelPosition().toPoint());
        QPixmap pixmap(mCommentItem->size().toSize());
        pixmap.fill(color);

        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << mDragStartPos - mCommentItem->position->pixelPosition().toPoint();

        QMimeData *mimeData = new QMimeData();
        mimeData->setData(MIME_TYPE_COMMENT_DRAGGING, itemData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(hotSpot);
        drag->exec();
    }
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

void CounterPlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this &&
        (event->mimeData()->hasFormat(MIME_TYPE_LEGEND_DRAGGING) ||
         event->mimeData()->hasFormat(MIME_TYPE_COMMENT_DRAGGING)))
    {
        event->acceptProposedAction();
    }
}

void CounterPlot::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(MIME_TYPE_LEGEND_DRAGGING)) {
        QByteArray itemData = event->mimeData()->data(MIME_TYPE_LEGEND_DRAGGING);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPoint offset;
        dataStream >> offset;

        QCPLayoutInset *layout = axisRect()->insetLayout();
        layout->setInsetPlacement(0, QCPLayoutInset::ipFree);

        QPointF newPos = event->pos() - offset;
        QRect insetLayoutRect = layout->rect();
        layout->setInsetRect(0, QRectF((newPos.x() - insetLayoutRect.left())/insetLayoutRect.width(),
                                       (newPos.y() - insetLayoutRect.top())/insetLayoutRect.height(),
                                       0, 0));
        replot(rpQueuedReplot);
    } else if (event->mimeData()->hasFormat(MIME_TYPE_COMMENT_DRAGGING)) {
        QByteArray itemData = event->mimeData()->data(MIME_TYPE_COMMENT_DRAGGING);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPoint offset;
        dataStream >> offset;

        QPointF newPos = event->pos() - offset;
        mCommentItem->position->setPixelPosition(newPos);
        mCommentItem->updateLineStartAnchor();

        replot(rpQueuedReplot);
    }
}

bool CounterPlot::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (pointInVisibleLegend(helpEvent->pos())) {
            if (CounterLegendItem *item = legendItemAt(helpEvent->pos())) {
                QToolTip::showText(helpEvent->globalPos(),
                                   mCounterDesc->getDescription(item->plottable()->name()));
            }
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QCustomPlot::event(event);
}

void CounterPlot::invalidateDragStartPos()
{
    mDragStartPos.setX(-1);
    mDragStartPos.setY(-1);
}

bool CounterPlot::hasValidDragStartPos() const
{
    return mDragStartPos.x() >= 0 && mDragStartPos.y() >= 0;
}

int CounterPlot::calcLegendPixmapHeight(QPoint &hotSpot)
{
    const QRect legendRect = legend->outerRect();
    const int screenHeight = QApplication::desktop()->screenGeometry(this).height();
    const int hCursorToLegendTop = mDragStartPos.y() - legendRect.top();
    const int hCursorToLegendBottom = legendRect.bottom() - mDragStartPos.y();

    hotSpot = mDragStartPos - legendRect.topLeft();

    int h1, h2 = qMin(hCursorToLegendBottom, screenHeight);
    if (hCursorToLegendTop < screenHeight) {
        h1 = hCursorToLegendTop;
    } else {
        h1 = screenHeight;
        hotSpot.ry() -= hCursorToLegendTop - screenHeight;
    }

    return h1 + h2;
}

CounterLegendItem * CounterPlot::legendItemAt(const QPoint &pos) const
{
    for (int i = 0; i < legend->itemCount(); ++i) {
        CounterLegendItem *item = qobject_cast<CounterLegendItem *>(legend->item(i));
        if (item->selectTest(pos, false) > 0) {
            return item;
        }
    }
    return nullptr;
}
