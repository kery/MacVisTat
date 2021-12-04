#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include "PlotData.h"
#include "ColorPool.h"

namespace Ui { class PlotWindow; }

class ValueTipItem;

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(PlotData &plotData);
    ~PlotWindow();

private slots:
    void actionSaveTriggered();
    void actionCopyTriggered();
    void actionRestoreTriggered();
    void actionShowDeltaTriggered(bool checked);
    void actionDisplayUtcTriggered(bool checked);
    void actionRemoveZeroCountersTriggered();
    void actionScriptTriggered();
    void actionShowLegendTriggered(bool checked);
    void actionMoveLegend();
    void actionReverseSelection();
    void actionRemoveSelectedGraphs();

    void selectionChanged();
    void skippedTicksChanged(int skipped);
    void contextMenuRequested(const QPoint &pos);
    void plotMouseMove(QMouseEvent *event);

private:
    void setupPlot();
    void initGraphs();
    void adjustYAxisRange();
    void highlightTimeGap();
    void updateWindowTitle();
    void updatePlotTitle();
    void removeGraphs(const QVector<CounterGraph*> &graphs);
    int legendItemIndex(QCPAbstractLegendItem *item) const;
    int graphIndex(CounterGraph *graph) const;
    CounterGraph *prevGraph(CounterGraph *graph) const;
    CounterGraph *nextGraph(CounterGraph *graph) const;
    CounterGraph *findNearestGraphData(const QPoint &pos, QCPGraphData &data) const;

    virtual void keyPressEvent(QKeyEvent *event);

    Ui::PlotWindow *ui;
    ValueTipItem *mValueTip;
    PlotData mPlotData;
    ColorPool mColorPool;
    int mLastSelLegItemIndex;
};

#endif // PLOTWINDOW_H
