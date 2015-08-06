#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "parsedataworker.h"
#include "mainwindow.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(const QString &node, const ParsedResult &result, QWidget *parent = 0);
    ~PlotWindow();

private:
    void initializePlot();

    MainWindow* mainWindow() const;

    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    QVector<int> findAbnormalTimeIndex() const;
    void markAbnormalTime();

private slots:
    void adjustTicks();
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);
    void contextMenuRequest(const QPoint &pos);
    void moveLegend();

    void on_actionFullScreen_toggled(bool checked);

    void on_actionSaveAsImage_triggered();

    void on_actionRestoreScale_triggered();

    void on_actionShowDelta_toggled(bool checked);

    void on_actionFillPlot_toggled(bool checked);

    void on_actionMarkAbnormalTime_toggled(bool checked);

private:
    Ui::PlotWindow *_ui;
    const QString _node;
    const ParsedResult _result;
};

#endif // PLOTWINDOW_H
