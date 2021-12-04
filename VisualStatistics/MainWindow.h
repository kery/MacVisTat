#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <array>
#include "ResizeManager.h"

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

signals:
    void aboutToBeClosed();

private slots:
    void actionOpenTriggered();
    void actionXmlToCsvTriggered();
    void actionCloseTriggered();
    void actionRecentFileTriggered();
    void actionFilterTriggered();
    void actionClearFilterHistoryTriggered();
    void actionEditFilterFileTriggered();
    void actionPlotTriggered();
    void actionPlotSeparatelyTriggered();
    void actionOptionsTriggered();
    void actionHelpTriggered();
    void actionChangeLogTriggered();
    void actionAboutTriggered();

    void caseSensitiveButtonClicked(bool checked);
    void updateCounterNameCountInfo();
    void updateFilterPattern();
    void filterEditReturnPressed();
    void counterNameViewDoubleClicked(const QModelIndex &index);
    void updateModuleNameColor();
    void logTextEditCtxMenuRequest(const QPoint &pos);
    void listViewCtxMenuRequest(const QPoint &pos);
    void filterFileChanged();

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
    void loadFavoriteFilterMenu();
    void addFilterAction(QMenu *menu, const QString &line);
    void loadFilterHistory();
    void saveFilterHistory();
    void adjustFilterHistoryOrder();
    void setupNetworkAccessManager();
    void startCheckUpdateTask();
    void startFetchCounterDescriptionTask();
    void startUsageReport();
    void openCounterFile(const QString &path);
    bool parseCounterFileHeader(const QString &path);
    void parseCounterFileData(bool multiWnd);
    void processPlotData(PlotData &plotData, bool multiWnd);
    PlotWindow *createPlotWindow(PlotData &plotData) const;

    enum LogLevel {
        llInfo,
        llWarn,
        llError,
    };

    QString formatLog(const QString &text, LogLevel level);
    void appendInfoLog(const QString &text);
    void appendWarnLog(const QString &text);
    void appendErrorLog(const QString &text);

    enum UrlPath {
        upHelp,
        upRoot,
    };

    enum FilePath {
        fpFavoriteFilter,
        fpFilterHistory,
    };

    static QUrl url(UrlPath up);
    static QString filePath(FilePath fp);
    static int trimLeadingSpace(QString &str);

    Ui::MainWindow *ui;
    int mOffsetFromUtc;
    QLabel *mCntNameInfoLabel;
    bool mCaseSensitive;
    QString mCounterFilePath;
    QString mLogDateTimeFmt;
    std::array<QAction*, 10> mRecentFileActions;
    QTimer mFilterMenuReloadTimer;
    QFileSystemWatcher mFilterFileWatcher;
    ResizeManager mResizeMan;
    QNetworkAccessManager mNetMan;
};

#endif // MAINWINDOW_H
