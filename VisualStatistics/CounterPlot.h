#ifndef COUNTERPLOT_H
#define COUNTERPLOT_H

#include "qcustomplot/qcustomplot.h"

class CounterGraph;
class CommentItem;

class CounterPlot : public QCustomPlot
{
    Q_OBJECT

public:
    CounterPlot(QWidget *parent = nullptr);
    ~CounterPlot();

    CounterGraph *addGraph();
    CounterGraph *graph(int index) const;
    bool removeGraph(CounterGraph *graph);
    bool removeGraph(int index);
    int clearGraphs();
    bool hasSelectedGraphs() const;
    int selectedGraphCount() const;
    QList<CounterGraph*> selectedGraphs() const;
    CommentItem *commentItemAt(const QPointF &pos, bool onlyVisible) const;
    bool pointInVisibleLegend(const QPoint &pos);

private:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    void invalidateDragStartPos();
    bool hasValidDragStartPos() const;
    int calcLegendPixmapHeight(QPoint &hotSpot);

    QPoint mDragStartPos;
    CommentItem *mCommentItem;
};

#endif // COUNTERPLOT_H
