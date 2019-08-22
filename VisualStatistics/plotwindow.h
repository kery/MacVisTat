#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "statistics.h"
#include "colormanager.h"
#include "valuetext.h"

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

    QCPGraph * addCounterGraph();

    Statistics& getStat();
    QCustomPlot* getPlot();

private:
    void initializePlot();

    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    QVector<double> findRestartTimeIndex() const;
    void findRestartTimeIndexForNode(const QString &node, QVector<double> &out) const;
    void markRestartTime();
    bool shouldDrawScatter(const QVector<double> &tickVector, int plotWidth) const;
    void updateScatter(const QVector<double> &tickVector, int plotWidth);

    QCPGraph * findGraphValueToShow(int index, double yCoord, double &value);

    QString genAggregateGraphName() const;
    void removeGraphs(const QVector<QCPGraph*> &graphs);

    QString evaluateWindowTitle() const;
    QString evaluatePlotTitle(bool deltaMode) const;

    QString defaultSaveFileName() const;

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
    void removeSelectedGraph();
    void copyGraphName();
    void setCustomTitle();
    void toggleLegendVisibility();
    void xAxisRangeChanged(const QCPRange &newRange);
    void fromDateTimeChanged(const QDateTime &dateTime);
    void toDateTimeChanged(const QDateTime &dateTime);

    void on_actionSaveAsImage_triggered();
    void on_actionRestoreScale_triggered();
    void on_actionShowDelta_toggled(bool checked);
    void on_actionScript_triggered();
    void on_actionRemoveZeroCounters_triggered();
    void on_actionCopyToClipboard_triggered();
    void on_actionMarkRestartTime_triggered(bool checked);
    void on_actionSetSampleInterval_triggered();

private:
    int m_sampleInterval;
    Ui::PlotWindow *m_ui;
    QString m_customTitle;
    QPropertyAnimation m_animation;
    QCPItemTracer *m_tracer;
    ValueText *m_valueText;
    QDateTimeEdit *m_dtEditFrom;
    QDateTimeEdit *m_dtEditTo;
    bool m_userEditFlag;
    bool m_userDragFlag;
    bool m_hasScatter;
    ColorManager m_colorManager;
    Statistics m_stat;
};

#endif // PLOTWINDOW_H
