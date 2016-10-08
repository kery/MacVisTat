#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "statistics.h"
#include "colorgenerator.h"

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

    Statistics& getStat();
    QCustomPlot* getPlot();

private:
    void initializePlot();

    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    QVector<double> findAbnormalTimeIndex() const;
    void findAbnormalTimeIndexForNode(const QString &node, QVector<double> &out) const;
    void markAbnormalTime();

    void removeGraphs(const QVector<QCPGraph*> &graphs);

private slots:
    void adjustTicks();
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);
    void contextMenuRequest(const QPoint &pos);
    void moveLegend();
    void removeSelectedGraph();
    void copyGraphName();
    void toggleLegendVisibility();
    void xAxisRangeChanged(const QCPRange &newRange);
    void fromDateTimeChanged(const QDateTime &dateTime);
    void toDateTimeChanged(const QDateTime &dateTime);

    void on_actionFullScreen_toggled(bool checked);

    void on_actionSaveAsImage_triggered();

    void on_actionRestoreScale_triggered();

    void on_actionShowDelta_toggled(bool checked);

    void on_actionScript_triggered();

    void on_actionRemoveZeroCounters_triggered();

    void on_actionCopyToClipboard_triggered();

private:
    Ui::PlotWindow *m_ui;
    QDateTimeEdit *m_dtEditFrom;
    QDateTimeEdit *m_dtEditTo;
    bool m_userEditFlag;
    bool m_userDragFlag;
    ColorGenerator m_colorGenerator;
    Statistics m_stat;
};

#endif // PLOTWINDOW_H
