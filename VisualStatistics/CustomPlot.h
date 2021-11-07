#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <qcustomplot.h>

class CommentText;

class CustomPlot : public QCustomPlot
{
    Q_OBJECT

public:
    CustomPlot(QWidget *parent);

    CommentText * commentTextAt(const QPoint &pt, bool onlyVisible) const;

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
    int calcLegendPixmapHeight(QPoint &hotSpot);

private:
    QPoint _dragStartPos;
    CommentText *_cmtText;
};

#endif // CUSTOMPLOT_H
