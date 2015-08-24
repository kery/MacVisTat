#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "mainwindow.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    static int predefinedColorCount();

public:
    explicit PlotWindow(const QString &node, const QMap<QString, QCPDataMap> &result, QWidget *parent = NULL);
    explicit PlotWindow(const QString &node, const QMap<QString, QCPDataMap> &result, const QVector<qint32> &dateTimes, qint32 legendAlignment, QWidget *parent = NULL);
    ~PlotWindow();

private:
    PlotWindow(const QString &node, QWidget *parent = NULL);

    void convertResultFirstData(const QMap<QString, QCPDataMap> &result);
    void convertResultRestData(const QMap<QString, QCPDataMap> &result);
    void initializePlot();

    QVector<double> calcTickVector(int plotWidth, int fontHeight, const QCPRange &range);
    QVector<QString> calcTickLabelVector(const QVector<double> &ticks);

    void calcDelta(QCPGraph *graph);

    QVector<int> findAbnormalTimeIndex() const;
    void markAbnormalTime();

    void adjustYAxisRange();

private slots:
    void adjustTicks();
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);
    void contextMenuRequest(const QPoint &pos);
    void moveLegend();
    void xAxisRangeChanged(const QCPRange &newRange);
    void fromDateTimeChanged(const QDateTime &dateTime);
    void toDateTimeChanged(const QDateTime &dateTime);

    void on_actionFullScreen_toggled(bool checked);

    void on_actionSaveAsImage_triggered();

    void on_actionRestoreScale_triggered();

    void on_actionShowDelta_toggled(bool checked);

    void on_actionFillPlot_toggled(bool checked);

    void on_actionMarkAbnormalTime_toggled(bool checked);

    void on_actionSaveToFile_triggered();

private:
    Ui::PlotWindow *_ui;
    const QString _node;
    QVector<qint32> _dateTimes;
    QMap<QString, QCPDataMap> _result;
    QDateTimeEdit *_dtEditFrom;
    QDateTimeEdit *_dtEditTo;
    bool _userEditFlag;
    bool _userDragFlag;
};

#endif // PLOTWINDOW_H
