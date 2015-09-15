#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "third_party/qcustomplot/qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct StatisticsResult {
        QVector<QString> failedFile;
        QMap<QString, QCPDataMap> statistics;
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
#if defined(Q_OS_WIN)
    void startCheckNewVersionTask();
#endif
    QString getStatisticsFileNode() const;
    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);

    bool statisticsFileAlreadyAdded(const QString &fileName);
    QVector<QString> addStatisticsFiles(const QStringList &fileNames);
    bool checkStatisticsFileNode(const QString &node);
    bool checkStatisticsFileType(const QString &type);
    void parseStatisticsFileHeader(const QVector<QString> &fileNames, bool updateModel);
    // multipleWindows indicates that if the parsed data will be shown in
    // multiple windows when it is ready
    // When parsed result is ready the slot handleParsedResult will be called
    void parseStatisticsFileData(bool multipleWindows);

    void showInfoMsgBox(const QString &text, const QString &info);
    void showErrorMsgBox(const QString &text, const QString &info);
    int showQuestionMsgBox(const QString &text, const QString &info = "");

    void appendLogInfo(const QString &text);
    void appendLogWarn(const QString &text);
    void appendLogError(const QString &text);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private slots:
#if defined(Q_OS_WIN)
    void checkNewVersionTaskFinished();
#endif
    void updateFilterPattern();
    void listViewDoubleClicked(const QModelIndex &index);
    void handleStatisticsResult(const StatisticsResult &result, bool multipleWindows);
    void logEditContextMenuRequest(const QPoint &pos);
    void listViewContextMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void handleTimeDurationResult(int index);
    void copyStatisticsNames();
    void updateStatNameStatus();

    void on_actionAdd_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();
    void on_actionDrawPlotInMultipleWindows_triggered();

    void on_actionSelectAll_triggered();

    void on_actionClearSelection_triggered();

    void on_actionInvertSelection_triggered();

    void on_actionViewHelp_triggered();

    void on_actionCalculateTimeDuration_triggered();

    void on_actionOpenPlotFile_triggered();

    void on_actionAbout_triggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *_ui;
    QLabel *_lbStatNameStatus;
};

#endif // MAINWINDOW_H
