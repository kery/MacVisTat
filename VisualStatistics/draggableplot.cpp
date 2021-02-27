#include "draggableplot.h"

DraggablePlot::DraggablePlot(QWidget *parent) :
    QCustomPlot(parent)
{
    setAcceptDrops(true);
}

QCPGraph * DraggablePlot::graphAtPosInLegend(const QPoint &pos)
{
    QCPLayerable *clickedLayerable = layerableAt(pos, false, nullptr);
    if (QCPAbstractLegendItem *li = qobject_cast<QCPAbstractLegendItem*>(clickedLayerable)) {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(li);
        return qobject_cast<QCPGraph *>(plItem->plottable());
    }
    return nullptr;
}

void DraggablePlot::mousePressEvent(QMouseEvent *event)
{
    if (legend->visible() && legend->selectTest(event->pos(), false) >= 0 &&
            event->button() == Qt::LeftButton)
    {
        _dragStartPos = event->pos();
    } else {
        _dragStartPos.setX(-1);
        _dragStartPos.setY(-1);

        if (event->button() == Qt::RightButton && QGuiApplication::keyboardModifiers() & Qt::ControlModifier) {
            QPixmap bgImg = toBackgroundPixmap();
            QByteArray bgData;
            QDataStream stream(&bgData, QIODevice::WriteOnly);
            stream << bgImg << window()->size() << axisRect()->margins();

            QMimeData *mimeData = new QMimeData();
            mimeData->setData(QStringLiteral("application/visualstat-bg"), bgData);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(bgImg.scaledToWidth(160, Qt::SmoothTransformation));

            double scaleRatio = double(160)/bgImg.width();
            QPoint pos = event->pos();
            drag->setHotSpot(QPoint(pos.x()*scaleRatio, pos.y()*scaleRatio));
            drag->exec(Qt::CopyAction);
        }
    }

    QCustomPlot::mousePressEvent(event);
}

void DraggablePlot::mouseMoveEvent(QMouseEvent *event)
{
    if (legend->selectTest(event->pos(), false) >=0 &&
            event->buttons() & Qt::LeftButton &&
            _dragStartPos.x() >= 0 && _dragStartPos.y() >= 0)
    {
        QPoint hotSpot;
        QRect rect = legend->outerRect();
        QPixmap pixmap(rect.width(), calcLegendPixmapHeight(hotSpot));
        QCPPainter painter(&pixmap);
        painter.fillRect(pixmap.rect(), QColor(96, 96, 96));

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
    } else {
        QCustomPlot::mouseMoveEvent(event);
    }
}

void DraggablePlot::resizeEvent(QResizeEvent *event)
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

void DraggablePlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-legend")) &&
            event->source() == this)
    {
        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-bg")) &&
               event->source() != this) {
        event->acceptProposedAction();
    }
}

void DraggablePlot::dropEvent(QDropEvent *event)
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
    } else if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-bg"))) {
        QByteArray bgData = event->mimeData()->data(QStringLiteral("application/visualstat-bg"));
        QDataStream stream(&bgData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QSize wndSize;
        QMargins margins;
        stream >> pixmap >> wndSize >> margins;

        setBackground(pixmap, false);
        axisRect()->setAutoMargins(QCP::msTop|QCP::msRight|QCP::msBottom);
        axisRect()->setMargins(margins);
        window()->resize(wndSize);
        replot(rpQueued);
    }
}

QPixmap DraggablePlot::toBackgroundPixmap()
{
    QString label = xAxis2->label();
    QColor labelColor = xAxis->tickLabelColor();

    xAxis2->setLabel(" ");
    xAxis->setTickLabelColor(mBackgroundBrush.color());
    yAxis->setTickLabelColor(Qt::darkGray);
    QPixmap result = toPixmap();
    yAxis->setTickLabelColor(labelColor);
    xAxis->setTickLabelColor(labelColor);
    xAxis2->setLabel(label);

    return result;
}

int DraggablePlot::calcLegendPixmapHeight(QPoint &hotSpot)
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
