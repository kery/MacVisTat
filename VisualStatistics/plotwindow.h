#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "statistics.h"
#include "colormanager.h"
#include "valuetext.h"
#include "countergraph.h"

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

    void connectXAxisRangeChanged();
    void disconnectXAxisRangeChanged();
    void connectDateTimeEditChange();
    void disconnectDateTimeEditChange();

    uint localTime_t(const QDateTime &dt) const;
    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    bool shouldDrawScatter(const QVector<double> &tickVector, int plotWidth) const;
    void updateScatter(const QVector<double> &tickVector, int plotWidth);

    QCPGraph * findNearestGraphValue(int index, double yCoord, double &value);

    QString genAggregateGraphName();
    void removeGraphs(const QVector<CounterGraph *> &graphs);
    QVector<CounterGraph *> selectedGraphs(bool selected) const;

    QString defaultSaveFileName() const;

    int getLegendItemIndex(QCPAbstractLegendItem *item) const;

    void setTracerGraph(QCPGraph *graph);

private slots:
    void adjustTicks();
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);
    void contextMenuRequest(const QPoint &pos);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void moveLegend();
    void addAggregateGraph();
    void removeSelectedGraphs();
    void removeUnselectedGraphs();
    void copyGraphName();
    void copyGraphValue();
    void showLegendTriggered(bool checked);
    void showModuleNameTriggered(bool checked);
    void displayUtcTimeTriggered(bool checked);
    void updateDateTimeEdit(const QCPRange &newRange);
    void fromDateTimeChanged(const QDateTime &dateTime);
    void toDateTimeChanged(const QDateTime &dateTime);

    void on_actionSaveAsImage_triggered();
    void on_actionRestoreScale_triggered();
    void on_actionShowDelta_triggered(bool checked);
    void on_actionShowSuspectFlag_triggered(bool checked);
    void on_actionScript_triggered();
    void on_actionRemoveZeroCounters_triggered();
    void on_actionCopyToClipboard_triggered();

public:
    static const double TracerSize;

private:
    int m_agggraph_idx;
    int m_lastSelLegitemIndex;
    Ui::PlotWindow *m_ui;
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
