#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <array>
#include "Statistics.h"
#include "ResizeManager.h"

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
    void startFetchStatDescriptionTask();

    void disableToolTipOfToolButton();
    bool isRegexpCaseButtonResizeEvent(QObject *obj, QEvent *event);

    void openStatFile(QString &path);
    bool parseStatFileHeader(const QString &path);
    void parseStatFileData(bool multipleWindows);
    void handleParsedStat(Statistics::NameDataMap &ndm, bool multipleWindows);
    QString getMaintenanceToolPath();

    void appendInfoLog(const QString &text);
    void appendWarnLog(const QString &text);
    void appendErrorLog(const QString &text);

    QString filterHistoryFilePath();
    QString statDescriptionFilePath();
    QString favoriteFilterFilePath();
    void loadFilterHistory();
    void saveFilterHistory();
    void adjustFilterHistoryOrder();
    void connectClearButtonSignal();
    void updateCaseSensitiveButtonFont();

    void initializeRecentFileActions();
    void updateRecentFileActions();
    void loadFavoriteFilterMenu();
    void addFavoriteFilterAction(QMenu *menu, const QString &line);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkNewVersionTaskError(QProcess::ProcessError error);
    void userReportTaskFinished();
    void fetchStatDescriptionFinished();

    void cbRegExpFilterEditReturnPressed();
    void updateFilterPattern();
    void listViewDoubleClicked(const QModelIndex &index);
    void logEditContextMenuRequest(const QPoint &pos);
    void listViewCtxMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void updateStatNameInfo();
    void updateModuleTextColor();
    void openRecentFile();
    void caseSensitiveButtonClicked(bool checked);
    void favoriteFilterFileChanged(const QString &path);

    void actionOpenTriggered();
    void actionXmlToCSVTriggered();
    void actionCloseTriggered();
    void actionPlotTriggered();
    void actionPlotSeparatelyTriggered();
    void actionClearFilterHistoryTriggered();
    void actionViewHelpTriggered();
    void actionChangeLogTriggered();
    void actionAboutTriggered();
    void actionEditFavoriteFilters();
    void actionFilterTriggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *m_ui;
    int m_offsetFromUtc;
    bool m_caseSensitive;
    QString m_statFilePath;
    QLabel *m_lbStatNameInfo;
    QAction *m_sepAction;
    std::array<QAction *, 10> m_recentFileActions;
    QNetworkAccessManager m_netMan;
    ResizeManager m_resizeMan;
    QFileSystemWatcher m_filterFileWatcher;
};

#endif // MAINWINDOW_H
