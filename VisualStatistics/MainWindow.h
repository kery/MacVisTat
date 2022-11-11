#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QProcess>
#include <array>
#include "ResizeManager.h"
#include "CounterDescription.h"

namespace Ui { class MainWindow; }

class QLabel;
class PlotWindow;
class PlotData;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void appendInfoLog(const QString &text);
    void appendWarnLog(const QString &text);
    void appendErrorLog(const QString &text);

    QAction * registerMenu(const QString &title, const QString &description);
    int parseCounterFileData();

signals:
    void aboutToBeClosed();

private slots:
    void actionOpenTriggered();
    void actionXmlToCsvTriggered();
    void actionCloseFileTriggered();
    void actionRecentFileTriggered();
    void actionFilterTriggered();
    void actionClearFilterHistoryTriggered();
    void actionEditFilterMenuTriggered();
    void actionPlotTriggered();
    void actionPlotSeparatelyTriggered();
    void actionOpenPluginsFolderTriggered();
    void actionBrowseOnlinePluginsTriggered();
    void actionOptionsTriggered();
    void actionHelpTriggered();
    void actionContactTriggered();
    void actionChangeLogTriggered();
    void actionAboutTriggered();
    void actionRegisteredMenuTriggered();

    void caseSensitiveButtonClicked(bool checked);
    void kpikciFileDlgDirEntered(const QString &dir);
    void updateCounterNameCountInfo();
    void updateFilterPattern();
    void filterEditReturnPressed();
    void counterNameViewDoubleClicked(const QModelIndex &index);
    void updateModuleNameColor();
    void listViewCtxMenuRequest(const QPoint &pos);
    void filterMenuFileChanged();
    void downloadCounterDescriptionFinished();
    void checkUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkUpdateFailed(QProcess::ProcessError error);
    void listOnlinePluginsFinished();
    void loadOnlinePluginsFinished();

private:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool event(QEvent *event) override;

    bool isCaseSensitiveButtonResizeEvent(QObject *obj, QEvent *event) const;
    void setupFilterComboBox();
    void connectClearButtonSignal();
    void updateWindowTitle();
    void updateCaseSensitiveButtonFont();
    void initRecentFileActions();
    void updateRecentFileActions(const QStringList &recentFiles);
    void loadFilterMenu();
    void addFilterAction(QMenu *menu, const QString &line);
    void loadFilterHistory();
    void saveFilterHistory();
    void adjustFilterHistoryOrder();
    void initLuaEnv();
    void listOnlinePlugins();
    void loadOnlinePlugins(const QStringList &hrefs);
    void checkUpdate();
    void downloadCounterDescription();
    void usageReport();
    void openCounterFile(const QString &path);
    bool parseCounterFileHeader(const QString &path);
    void parseCounterFileData(bool multiWnd);
    void processPlotData(PlotData &plotData, bool multiWnd);
    PlotWindow *createPlotWindow(PlotData &plotData);

    enum LogLevel {
        llInfo,
        llWarn,
        llError,
    };

    QString formatLog(const QString &text, LogLevel level);

    enum FilePath {
        fpFilterMenu,
        fpFilterHistory,
        fpCounterDescription,
        fpMaintenanceTool,
        fpPluginDir,
    };

    static QString filePath(FilePath fp);
    static int trimLeadingSpace(QString &str);

    Ui::MainWindow *ui;
    lua_State *mL;
    int mOffsetFromUtc;
    QLabel *mStatusBarLabel;
    bool mCaseSensitive;
    QString mCounterFilePath;
    QString mLogDateTimeFmt;
    std::array<QAction*, 10> mRecentFileActions;
    QTimer mFilterMenuReloadTimer;
    QFileSystemWatcher mFilterMenuFileWatcher;
    ResizeManager mResizeMan;
    CounterDescription mCounterDesc;
};

#endif // MAINWINDOW_H
