#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "third_party/qcustomplot/qcustomplot.h"

namespace Ui {
class MainWindow;
}

class ProgressBar : public QProgressBar
{
    Q_OBJECT

public slots:
    // In order to called by QMetaObject::invokeMethod, method must
    // be slots or decorate by Q_INVOKABLE
    void increaseValue(int value);
};

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
    QString getStatisticsFileNode() const;
    void installEventFilterForAllToolButton();
    bool isToolTipEventOfToolButton(QObject *obj, QEvent *event);

    bool statisticsFileAlreadyAdded(const QString &fileName);
    int addStatisticsFiles(const QStringList &fileNames);
    bool checkStatisticsFileNode(const QString &node);
    bool checkStatisticsFileType(const QString &type);
    void parseStatisticsFileHeader();
    // multipleWindows indicates that if the parsed data will be shown in
    // multiple windows when it is ready
    // When parsed result is ready the slot handleParsedResult will be called
    void parseStatisticsFileData(bool multipleWindows);

    void showInfoMsgBox(const QString &text, const QString &info);
    void showErrorMsgBox(const QString &text, const QString &info);

    void appendLogInfo(const QString &text);
    void appendLogWarn(const QString &text);
    void appendLogError(const QString &text);

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private:
    // Can't place this line in the slots section, otherwise no compile
    // error occurred but failed to generate executable file (due to MOC?)
    typedef QMap<int, QString> TimeDurationResult;

private slots:
    void updateFilterPattern();
    void listViewDoubleClicked(const QModelIndex &index);
    void handleStatisticsResult(const StatisticsResult &result, bool multipleWindows);
    void logEditContextMenuRequest(const QPoint &pos);
    void listViewContextMenuRequest(const QPoint &pos);
    void clearLogEdit();
    void handleTimeDurationResult(int index);
    void copyStatisticsNames();

    void on_actionAdd_triggered();

    void on_actionCloseAll_triggered();

    void on_actionDrawPlot_triggered();
    void on_actionDrawPlotInMultipleWindows_triggered();

    void on_actionSelectAll_triggered();

    void on_actionClearSelection_triggered();

    void on_actionInvertSelection_triggered();

    void on_actionViewHelp_triggered();

signals:
    void aboutToBeClosed();

private:
    Ui::MainWindow *_ui;
};

#endif // MAINWINDOW_H
