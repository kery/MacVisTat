#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include "PlotData.h"
#include "ColorPool.h"

namespace Ui { class PlotWindow; }

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

    void selectionChanged();
    void skippedTicksChanged(int skipped);

private:
    void initGraphs();
    void adjustYAxisRange();
    int legendItemIndex(QCPAbstractLegendItem *item) const;
    int graphIndex(CounterGraph *graph) const;
    CounterGraph *prevGraph(CounterGraph *graph) const;
    CounterGraph *nextGraph(CounterGraph *graph) const;

    virtual void keyPressEvent(QKeyEvent *event);

    Ui::PlotWindow *ui;
    PlotData _plotData;
    ColorPool _colorPool;
    int _lastSelLegItemIndex;
};

#endif // PLOTWINDOW_H
