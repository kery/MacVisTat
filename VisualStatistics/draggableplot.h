#pragma once

#include "third_party/qcustomplot/qcustomplot.h"

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
    int calcLegendPixmapHeight();

private:
    QPoint _dragStartPos;
};
