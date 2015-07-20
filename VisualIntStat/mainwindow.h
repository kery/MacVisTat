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
    QStyle *_fusionStyle;

private:
    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);

    bool statFileAlreadyAdded(const QString &fileName);
    void addStatFiles(const QStringList &fileNames);
    bool checkStatFileNode(const QString &node);

    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

private slots:
    void on_actionOpen_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();

    void on_actionListView_toggled(bool checked);

    void on_actionTreeView_toggled(bool checked);

    void on_actionSelectAll_triggered();

    void on_actionClearSelection_triggered();

    void on_actionInvertSelection_triggered();

private:
    Ui::MainWindow *_ui;
    QString _node;
};

#endif // MAINWINDOW_H
