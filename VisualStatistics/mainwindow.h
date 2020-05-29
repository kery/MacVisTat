#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QNetworkReply>
#include <array>
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
    void startUserReportTask();

    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);
    bool isRegexpCaseButtonResizeEvent(QObject *obj, QEvent *event);

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

    QString filterHistoryFilePath();
    void loadFilterHistory();
    void saveFilterHistory();

    void initializeRecentFileActions();
    void updateRecentFileActions();

    static bool allDataUnchanged(const Statistics::NodeNameDataMap &nndm);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private slots:
    void checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkNewVersionTaskError(QProcess::ProcessError error);
    void userReportTaskFinished(QNetworkReply *reply);

    void updateFilterPattern();
    void adjustFilterHistoryOrder();
    void listViewDoubleClicked(const QModelIndex &index);
    void logEditContextMenuRequest(const QPoint &pos);
    void lvStatNameCtxMenuRequest(const QPoint &pos);
    void lwModulesCtxMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void copyLvStatNameSelected();
    void updateStatNameInfo();
    void updateModulesInfo();
    void addRecentFile();
    void caseSensitiveButtonClicked(bool checked);

    void on_actionAdd_triggered();
    void on_actionXmlToCSV_triggered();
    void on_actionCloseAll_triggered();
    void on_actionPlot_triggered();
    void on_actionPlotSeparately_triggered();
    void on_actionClearFilterHistory_triggered();
    void on_actionSelectAll_triggered();
    void on_actionClearSelection_triggered();
    void on_actionInvertSelection_triggered();
    void on_actionViewHelp_triggered();
    void on_actionChangeLog_triggered();
    void on_actionAbout_triggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *m_ui;
    bool m_caseSensitive;
    QLabel *m_lbStatNameInfo;
    QLabel *m_lbModulesInfo;
    QAction *m_sepAction;
    std::array<QAction *, 10> m_recentFileActions;
};

#endif // MAINWINDOW_H
