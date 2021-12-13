#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include "PlotData.h"
#include "ColorPool.h"

namespace Ui { class PlotWindow; }

class CounterGraph;
class ValueTipItem;
class CommentItem;
class CounterDescription;

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(PlotData &plotData);
    ~PlotWindow();

    void setCounterDescription(CounterDescription *desc);

private slots:
    void actionSaveTriggered();
    void actionCopyTriggered();
    void actionExportToCsvTriggered();
    void actionRestoreTriggered();
    void actionShowDeltaTriggered(bool checked);
    void actionDisplayUtcTriggered(bool checked);
    void actionRemoveZeroCountersTriggered();
    void actionScriptTriggered();
    void actionShowLegendTriggered(bool checked);
    void actionMoveLegendTriggered();
    void actionAddCommentTriggered();
    void actionEditCommentTriggered();
    void actionRemoveCommentTriggered();
    void actionAddAggregateGraphTriggered();
    void actionSetGraphColorTriggered();
    void actionCopyGraphNameTriggered();
    void actionCopyGraphValueTriggered();
    void actionReverseSelectionTriggered();
    void actionRemoveSelectedGraphsTriggered();

    void selectionChanged();
    void skippedTicksChanged(int skipped);
    void contextMenuRequested(const QPoint &pos);
    void plotMouseMove(QMouseEvent *event);
    void axisBeginDateTimeChanged(const QDateTime &dateTime);
    void axisEndDateTimeChanged(const QDateTime &dateTime);
    void editBeginDateTimeChanged(const QDateTime &dateTime);
    void editEndDateTimeChanged(const QDateTime &dateTime);

private:
    void setupPlot();
    void setupDateTimeEdits();
    void initGraphs();
    void adjustYAxisRange();
    void highlightTimeGap();
    void updateWindowTitle();
    void updatePlotTitle();
    QString defaultSaveFileName() const;
    QString getInputComment(const QString &text);
    QVector<CommentItem *> commentItemsOfGraph(CounterGraph *graph) const;
    void removeGraphs(const QVector<CounterGraph*> &graphs);
    int legendItemIndex(QCPAbstractLegendItem *item) const;
    int graphIndex(CounterGraph *graph) const;
    CounterGraph * prevGraph(CounterGraph *graph) const;
    CounterGraph * nextGraph(CounterGraph *graph) const;
    CounterGraph * findNearestGraphData(const QPoint &pos, QCPGraphData &data) const;

    virtual void keyPressEvent(QKeyEvent *event);

    Ui::PlotWindow *ui;
    ValueTipItem *mValueTip;
    QDateTimeEdit *mDtEditBegin, *mDtEditEnd;
    PlotData mPlotData;
    ColorPool mColorPool;
    int mLastSelLegItemIndex;

    friend class LuaEnvironment;
};

#endif // PLOTWINDOW_H
