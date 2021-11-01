#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QNetworkReply>
#include <array>
#include "Statistics.h"

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

    void disableToolTipOfToolButton();
    bool isRegexpCaseButtonResizeEvent(QObject *obj, QEvent *event);

    void openStatFile(QString &path);
    void parseStatFileHeader(const QString &path, QString &error);
    void parseStatFileData(bool multipleWindows);
    void handleParsedStat(Statistics::NameDataMap &ndm, bool multipleWindows);
    QString getMaintenanceToolPath();

    void appendInfoLog(const QString &text);
    void appendWarnLog(const QString &text);
    void appendErrorLog(const QString &text);

    QString filterHistoryFilePath();
    void loadFilterHistory();
    void saveFilterHistory();
    void adjustFilterHistoryOrder();
    void connectClearButtonSignal();
    void updateCaseSensitiveButtonFont();

    void initializeRecentFileActions();
    void updateRecentFileActions();
    qreal getCurrentScreenScale();

    static bool checkFileName(const QString &path);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkNewVersionTaskError(QProcess::ProcessError error);
    void userReportTaskFinished(QNetworkReply *reply);

    void cbRegExpFilterEditReturnPressed();
    void updateFilterPattern();
    void listViewDoubleClicked(const QModelIndex &index);
    void logEditContextMenuRequest(const QPoint &pos);
    void listViewCtxMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void updateStatNameInfo();
    void updateModulesInfo();
    void openRecentFile();
    void caseSensitiveButtonClicked(bool checked);

    void actionOpenTriggered();
    void actionXmlToCSVTriggered();
    void actionCloseTriggered();
    void actionPlotTriggered();
    void actionPlotSeparatelyTriggered();
    void actionClearFilterHistoryTriggered();
    void actionViewHelpTriggered();
    void actionChangeLogTriggered();
    void actionAboutTriggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *m_ui;
    int m_offsetFromUtc;
    bool m_caseSensitive;
    qreal m_lastScale;
    QString m_statFilePath;
    QLabel *m_lbStatNameInfo;
    QLabel *m_lbModulesInfo;
    QAction *m_sepAction;
    std::array<QAction *, 10> m_recentFileActions;
};

#endif // MAINWINDOW_H
