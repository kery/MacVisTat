#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private slots:
    void filterTextReady();

    void on_actionOpen_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();

    void on_actionSelectAll_triggered();

    void on_actionClearSelection_triggered();

    void on_actionInvertSelection_triggered();

    void on_actionTimeDuration_triggered();

private:
    Ui::MainWindow *_ui;
    QString _node;
    QTimer _timer;
};

#endif // MAINWINDOW_H
