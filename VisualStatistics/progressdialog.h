#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>
#if defined(Q_OS_WIN)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent);
    ~ProgressDialog();

public slots:
    void setRange(int minimum, int maximum);
    void setValue(int progress);
    // In order to called by QMetaObject::invokeMethod, method must
    // be slots or decorate by Q_INVOKABLE
    void increaseValue(int value);

private:
#if defined(Q_OS_WIN)
    QWinTaskbarButton _taskbarButton;
    QWinTaskbarProgress *_taskbarProgress;
#endif
};

#endif // PROGRESSDIALOG_H
