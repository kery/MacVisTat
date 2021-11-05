#ifndef DRAGGABLEPLOT_H
#define DRAGGABLEPLOT_H

#include <qcustomplot.h>

class CommentText;

class DraggablePlot : public QCustomPlot
{
    Q_OBJECT

public:
    DraggablePlot(QWidget *parent);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    void invalidateDragStartPos();
    bool hasValidDragStartPos() const;
    bool pointInVisibleLegend(const QPoint &pt) const;
    CommentText * commentTextAt(const QPoint &pt) const;
    int calcLegendPixmapHeight(QPoint &hotSpot);

private:
    QPoint _dragStartPos;
    CommentText *_cmtText;
};

#endif // DRAGGABLEPLOT_H
