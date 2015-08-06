#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "parsedataworker.h"

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
    QString getStatFileNode() const;
    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);

    bool statFileAlreadyAdded(const QString &fileName);
    void addStatFiles(const QStringList &fileNames);
    bool checkStatFileNode(const QString &node);
    void parseStatFileHeader();
    // multipleWindows indicates that if the parsed data will be shown in
    // multiple windows when it is ready
    // When parsed result is ready the slot handleParsedResult will be called
    void parseStatFileData(bool multipleWindows);

    void showInfoMsgBox(const QString &text, const QString &info);
    void showErrorMsgBox(const QString &text, const QString &info);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private slots:
    void updateFilterPattern();
    void handleParsedResult(const ParsedResult &result, bool multipleWindows);

    void on_actionOpen_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();
    void on_actionDrawPlotInMultipleWindows_triggered();

    void on_actionSelectAll_triggered();

    void on_actionClearSelection_triggered();

    void on_actionInvertSelection_triggered();

    void on_actionTimeDuration_triggered();

signals:
    void parseDataParamReady(const ParseDataParam &param);

private:
    Ui::MainWindow *_ui;
    QString _node;
};

#endif // MAINWINDOW_H
