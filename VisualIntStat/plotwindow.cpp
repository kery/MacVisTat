#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "mainwindow.h"

PlotWindow::PlotWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::PlotWindow)
{
    _ui->setupUi(this);

    MainWindow *mainWindow = dynamic_cast<MainWindow*>(parent);
    if (mainWindow) {
        _ui->toolBar->setStyle(mainWindow->_fusionStyle);
    }

    _ui->customPlot->axisRect()->setupFullAxesBox();
    _ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectLegend);
}

PlotWindow::~PlotWindow()
{
    delete _ui;
}

void PlotWindow::on_actionFullScreen_toggled(bool checked)
{
    if (checked) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void PlotWindow::on_actionSaveToFile_triggered()
{
    // TODO: set default name as statistics name or node name
    QString dir = QDir::cleanPath(qApp->applicationDirPath() + QDir::separator() + "test.png");
    QString fileName = QFileDialog::getSaveFileName(this, "Save Plot to Image", dir, "PNG File (*.png)");
    _ui->customPlot->savePng(fileName);
}

void PlotWindow::on_actionRestoreSize_triggered()
{
    // TODO: restore to the original size
}
