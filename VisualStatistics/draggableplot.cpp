#include "draggableplot.h"

DraggablePlot::DraggablePlot(QWidget *parent) :
    QCustomPlot(parent)
{
    setAcceptDrops(true);
}

void DraggablePlot::mousePressEvent(QMouseEvent *event)
{
    if (legend->selectTest(event->pos(), false) >= 0 &&
            event->button() == Qt::LeftButton)
    {
        _dragStartPos = event->pos();
    } else {
        _dragStartPos.setX(-1);
        _dragStartPos.setY(-1);
    }

    QCustomPlot::mousePressEvent(event);
}

void DraggablePlot::mouseMoveEvent(QMouseEvent *event)
{
    if (legend->selectTest(event->pos(), false) >=0 &&
            event->buttons() & Qt::LeftButton &&
            _dragStartPos.x() >= 0 && _dragStartPos.y() >= 0)
    {
        QRect rect = legend->outerRect();
        QPixmap pixmap(rect.width(), calcLegendPixmapHeight());
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
        drag->setHotSpot(_dragStartPos - rect.topLeft());
        drag->exec();
    } else {
        QCustomPlot::mouseMoveEvent(event);
    }
}

void DraggablePlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/visualstat-legend")) &&
            event->source() == this)
    {
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
        replot();
    }
}

int DraggablePlot::calcLegendPixmapHeight()
{
    int screenHeight = QApplication::desktop()->screenGeometry(this).height();
    int offsetYGlobal = mapToGlobal(_dragStartPos).y();

    return qMin(legend->outerRect().height(), screenHeight + offsetYGlobal);
}
