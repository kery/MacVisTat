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
    void actionRestoreTriggered();

    void skippedTicksChanged(int skipped);

private:
    void setupPlot();
    void adjustYAxisRange();

    Ui::PlotWindow *ui;
    PlotData _plotData;
    ColorPool _colorPool;
};

#endif // PLOTWINDOW_H
