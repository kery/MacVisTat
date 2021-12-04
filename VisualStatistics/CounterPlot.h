#ifndef COUNTERPLOT_H
#define COUNTERPLOT_H

#include "qcustomplot/qcustomplot.h"

class CounterGraph;

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
    QList<CounterGraph*> selectedGraphs() const;

private:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // COUNTERPLOT_H
