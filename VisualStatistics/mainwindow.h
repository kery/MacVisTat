#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "statistics.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    MainWindow(const MainWindow &) = delete;
    MainWindow& operator=(const MainWindow &) = delete;
    ~MainWindow();

private:
    void startCheckNewVersionTask();

    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);

    bool statFileAlreadyAdded(const QString &filePath);
    void addStatFiles(QStringList &filePaths);
    void filterOutAlreadyAddedFiles(QStringList &filePaths);
    QStringList filterOutInvalidFileNames(QStringList &filePaths);
    void parseStatFileHeader(QStringList &filePaths, QStringList &failInfo);
    void checkStatFileHeader(QStringList &filePaths, QStringList &failInfo);
    void addStatFilesToListWidget(const QStringList &filePaths);
    void translateToLocalPath(QStringList &filePaths);
    // multipleWindows indicates that if the parsed data will be shown in
    // multiple windows when it is ready
    // When parsed result is ready the slot handleParsedResult will be called
    void parseStatFileData(bool multipleWindows);
    void handleParsedStat(Statistics::NodeNameDataMap &nndm, bool multipleWindows);
    QString getMaintenanceToolPath();

    void appendLogInfo(const QString &text);
    void appendLogWarn(const QString &text);
    void appendLogError(const QString &text);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private slots:
    void checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void updateFilterPattern();
    void listViewDoubleClicked(const QModelIndex &index);
    void logEditContextMenuRequest(const QPoint &pos);
    void listViewContextMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void handleTimeDurationResult(int index);
    void copyStatisticsNames();
    void updateStatNameInfo();

    void on_actionAdd_triggered();
    void on_actionCloseAll_triggered();
    void on_actionDrawPlot_triggered();
    void on_actionDrawPlotInMultipleWindows_triggered();
    void on_actionSelectAll_triggered();
    void on_actionClearSelection_triggered();
    void on_actionInvertSelection_triggered();
    void on_actionViewHelp_triggered();
    void on_actionCalculateTimeDuration_triggered();
    void on_actionAbout_triggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *m_ui;
    QLabel *m_lbStatNameInfo;
};

#endif // MAINWINDOW_H
