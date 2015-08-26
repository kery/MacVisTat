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
    explicit MainWindow(const QString &path, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::MainWindow *_ui;
    QString _dumpFilePath;
};

#endif // MAINWINDOW_H
