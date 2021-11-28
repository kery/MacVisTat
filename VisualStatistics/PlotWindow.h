#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include "PlotData.h"

namespace Ui { class PlotWindow; }

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(PlotData &plotData);
    ~PlotWindow();

private slots:
    void actionRestoreTriggered();

private:
    void setupPlot();
    void adjustYAxisRange();

    Ui::PlotWindow *ui;
    PlotData _plotData;
};

#endif // PLOTWINDOW_H
