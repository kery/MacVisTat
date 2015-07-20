#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = 0);
    ~PlotWindow();

private slots:
    void on_actionFullScreen_toggled(bool checked);

    void on_actionSaveToFile_triggered();

    void on_actionRestoreSize_triggered();

private:
    Ui::PlotWindow *_ui;
};

#endif // PLOTWINDOW_H
