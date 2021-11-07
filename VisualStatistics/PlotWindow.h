#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "Statistics.h"
#include "ColorManager.h"
#include "ValueText.h"
#include "CommentText.h"
#include "CounterGraph.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(Statistics &stat);
    PlotWindow(const PlotWindow &) = delete;
    PlotWindow& operator=(const PlotWindow &) = delete;
    ~PlotWindow();

    CounterGraph * addCounterGraph(const QString &name, const QString &module=QString());
    void updateWindowTitle();
    void updatePlotTitle();

    Statistics& getStat();
    QCustomPlot* getPlot();

private:
    void initializePlot();
    void markDiscontinuousTime();

    void connectXAxisRangeChanged();
    void disconnectXAxisRangeChanged();
    void connectDateTimeEditChange();
    void disconnectDateTimeEditChange();

    uint localTime_t(const QDateTime &dt) const;
    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    bool shouldDrawScatter(const QVector<double> &tickVector, int plotWidth, int fontHeight) const;
    void updateScatter(const QVector<double> &tickVector, int plotWidth, int fontHeight);

    QCPGraph * findNearestGraphValue(int index, double yCoord, double &value, bool &suspect);

    QString genAggregateGraphName();
    void removeGraphs(const QVector<CounterGraph *> &graphs);
    QVector<CounterGraph *> selectedGraphs(bool selected) const;
    QVector<CommentText *> commentsOfGraph(const QCPGraph *graph) const;

    QString defaultSaveFileName() const;

    int getLegendItemIndex(QCPAbstractLegendItem *item) const;

    void setTracerGraph(QCPGraph *graph);
    QString getInputComment(const QString &text);

    int graphIndex(QCPGraph *graph) const;
    QCPGraph * prevGraph(QCPGraph *graph) const;
    QCPGraph * nextGraph(QCPGraph *graph) const;

    virtual void keyPressEvent(QKeyEvent *event);

private slots:
    void adjustTicks();
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);
    void timerTimeout();
    void contextMenuRequest(const QPoint &pos);
    void moveLegend();
    void addComment();
    void editComment();
    void removeComment();
    void addAggregateGraph();
    void removeSelectedGraphs();
    void removeUnselectedGraphs();
    void setGraphColor();
    void copyGraphName();
    void copyGraphValue();
    void showLegendTriggered(bool checked);
    void showModuleNameTriggered(bool checked);
    void displayUtcTimeTriggered(bool checked);
    void updateDateTimeEdit(const QCPRange &newRange);
    void fromDateTimeChanged(const QDateTime &dateTime);
    void toDateTimeChanged(const QDateTime &dateTime);

    void actionSaveAsImageTriggered();
    void actionRestoreScaleTriggered();
    void actionShowDeltaTriggered(bool checked);
    void actionShowSuspectFlagTriggered(bool checked);
    void actionScriptTriggered();
    void actionRemoveZeroCountersTriggered();
    void actionCopyToClipboardTriggered();

public:
    static const double TracerSize;

private:
    int m_agggraph_idx;
    int m_lastSelLegitemIndex;
    Ui::PlotWindow *m_ui;
    QTimer m_timer;
    QPropertyAnimation m_animation;
    QCPItemTracer *m_tracer;
    ValueText *m_valueText;
    QDateTimeEdit *m_dtEditFrom;
    QDateTimeEdit *m_dtEditTo;
    bool m_hasScatter;
    bool m_showModule;
    ColorManager m_colorManager;
    Statistics m_stat;

    static const int AnimationMaxGraphs;
};

#endif // PLOTWINDOW_H
