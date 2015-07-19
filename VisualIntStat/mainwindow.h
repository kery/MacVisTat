#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QStyle *fusionStyle;

private:
    void disableToolbarTooltip();

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_actionOpen_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();

    void on_actionListView_toggled(bool arg1);

    void on_actionTreeView_toggled(bool arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
