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

    void setDumpFilePath(const QString &path);

private:
    Ui::MainWindow *ui;
    QString _dumpFilePath;
};

#endif // MAINWINDOW_H
