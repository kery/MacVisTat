#include "CustomPlot.h"
#include "CommentText.h"

CustomPlot::CustomPlot(QWidget *parent) :
    QCustomPlot(parent),
    _cmtText(nullptr)
{
    setAcceptDrops(true);
    invalidateDragStartPos();
}

CommentText * CustomPlot::commentTextAt(const QPoint &pt, bool onlyVisible) const
{
    for (int i = itemCount() - 1; i >= 0 ; --i) {
        CommentText *textItem = qobject_cast<CommentText *>(item(i));
        if (!textItem || (onlyVisible && !textItem->visible())) {
            continue;
        }
        if (textItem->selectTest(pt, false) > 0) {
            return textItem;
        }
    }
    return nullptr;
}

void CustomPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (pointInVisibleLegend(event->pos())) {
            _dragStartPos = event->pos();
            QCustomPlot::mousePressEvent(event);
            return;
        }
        if ((_cmtText = commentTextAt(event->pos(), true)) != nullptr) {
            // Don't call QCustomPlot::mousePressEvent(event), so that the QCustomPlot
            // widget keeps in normal state, not in range dragging state, even though the mouse
            // pointer is in a QCPAbstractItem rather than a QCPLayoutElement.
            _dragStartPos = event->pos();
            return;
        }
    }
    invalidateDragStartPos();
    QCustomPlot::mousePressEvent(event);
}

void CustomPlot::mouseMoveEvent(QMouseEvent *event)
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
        dataStream << _dragStartPos - rect.topLeft();

        QMimeData *mimeData = new QMimeData();
        mimeData->setData(QStringLiteral("application/visualstat-legend"), itemData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(hotSpot);
        drag->exec();
    } else if (_cmtText) {
        QPoint hotSpot(event->pos() - _cmtText->topLeft->pixelPoint().toPoint());
        QPixmap pixmap(_cmtText->size());
        pixmap.fill(color);

        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << _dragStartPos - _cmtText->position->pixelPoint().toPoint();

        QMimeData *mimeData = new QMimeData();
        mimeData->setData(QStringLiteral("application/visualstat-comment"), itemData);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(hotSpot);
        drag->exec();
    }
}

void CustomPlot::resizeEvent(QResizeEvent *event)
{
    QCPLayoutInset *layout = axisRect()->insetLayout();
    QRect oldRect = layout->rect();

    QCustomPlot::resizeEvent(event);

    if (layout->insetPlacement(0) == QCPLayoutInset::ipFree) {
        QRect newRect = layout->rect();
        QRectF insetRect = layout->insetRect(0);
        bool insetRectChanged = false;

        if (insetRect.left() < 0) {
            qreal leftPixels = oldRect.width() * insetRect.left();
            insetRect.setLeft(leftPixels / newRect.width());
            insetRectChanged = true;
        }

        if (insetRect.top() < 0) {
            qreal topPixels = oldRect.height() * insetRect.top();
            insetRect.setTop(topPixels / newRect.height());
            insetRectChanged = true;
        }

        if (insetRectChanged) {
            layout->setInsetRect(0, insetRect);
            replot(rpQueued);
        }
    }
}

void CustomPlot::dragEnterEvent(QDragEnterEvent *event)
{
    if ((event->mimeData()->hasFormat(QStringLiteral("application/visualstat-legend")) ||
         event->mimeData()->hasFormat(QStringLiteral("application/visualstat-comment"))) &&
            event->source() == this)
    {
        event->acceptProposedAction();
    }
}

void CustomPlot::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-legend"))) {
        QByteArray itemData = event->mimeData()->data(QStringLiteral("application/visualstat-legend"));
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
        replot(rpQueued);
    } else if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-comment"))) {
        QByteArray itemData = event->mimeData()->data(QStringLiteral("application/visualstat-comment"));
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPoint offset;
        dataStream >> offset;

        QPointF newPos = event->pos() - offset;
        _cmtText->position->setCoords(xAxis->pixelToCoord(newPos.x()), yAxis->pixelToCoord(newPos.y()));
        _cmtText->updateTracerAndLine();

        replot(rpQueued);
    }
}

void CustomPlot::invalidateDragStartPos()
{
    _dragStartPos.setX(-1);
    _dragStartPos.setY(-1);
}

bool CustomPlot::hasValidDragStartPos() const
{
    return _dragStartPos.x() >= 0 && _dragStartPos.y() >= 0;
}

bool CustomPlot::pointInVisibleLegend(const QPoint &pt) const
{
    return legend->visible() && legend->selectTest(pt, false) > 0;
}

int CustomPlot::calcLegendPixmapHeight(QPoint &hotSpot)
{
    const QRect legendRect = legend->outerRect();
    const int screenHeight = QApplication::desktop()->screenGeometry(this).height();
    const int hCursorToLegendTop = _dragStartPos.y() - legendRect.top();
    const int hCursorToLegendBottom = legendRect.bottom() - _dragStartPos.y();

    hotSpot = _dragStartPos - legendRect.topLeft();

    int h1, h2 = qMin(hCursorToLegendBottom, screenHeight);
    if (hCursorToLegendTop < screenHeight) {
        h1 = hCursorToLegendTop;
    } else {
        h1 = screenHeight;
        hotSpot.ry() -= hCursorToLegendTop - screenHeight;
    }

    return h1 + h2;
}
