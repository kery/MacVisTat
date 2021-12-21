#ifndef COUNTERPLOT_H
#define COUNTERPLOT_H

#include "QCustomPlot/src/core.h"

class CounterGraph;
class CommentItem;
class CounterLegendItem;
class CounterDescription;

class CounterPlot : public QCustomPlot
{
    Q_OBJECT

public:
    CounterPlot(QWidget *parent = nullptr);
    ~CounterPlot();

    CounterGraph * addGraph();
    CounterGraph * graph(int index) const;
    bool removeGraph(CounterGraph *graph);
    bool removeGraph(int index);
    int clearGraphs();
    bool hasSelectedGraphs() const;
    int selectedGraphCount() const;
    QList<CounterGraph *> selectedGraphs() const;
    CommentItem * commentItemAt(const QPointF &pos, bool onlyVisible) const;
    bool pointInVisibleLegend(const QPoint &pos);
    void setCounterDescription(CounterDescription *desc);

private:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual bool event(QEvent *event) override;

    void invalidateDragStartPos();
    bool hasValidDragStartPos() const;
    int calcLegendPixmapHeight(QPoint &hotSpot);
    CounterLegendItem * legendItemAt(const QPoint &pos) const;

    QPoint mDragStartPos;
    CommentItem *mCommentItem;
    CounterDescription *mCounterDesc;
};

#endif // COUNTERPLOT_H
