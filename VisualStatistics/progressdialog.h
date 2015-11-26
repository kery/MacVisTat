#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#if defined(Q_OS_WIN)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

namespace Ui {
class ProgressDialog;
}

// Implement progress dialog class ourselves
//
// QProgressDialog was not well implemented since it may
// lead to stack overflow due to re-entrance of setValue.
// The detail is that if dialog is modal, processEvents
// will be called inside setValue method, which may also
// call setValue again if there are many messages in
// message queue.

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent);
    ProgressDialog(const ProgressDialog &) = delete;
    ProgressDialog& operator=(const ProgressDialog &) = delete;
    ~ProgressDialog();

    void setLabelText(const QString &text);
    void enableCancelButton(bool enabled);

public slots:
    void setRange(int minimum, int maximum);
    void setValue(int progress);
    // In order to called by QMetaObject::invokeMethod, method must
    // be slots or decorate by Q_INVOKABLE
    void increaseValue(int val);

signals:
    void canceled();

private slots:
    void on_pushButton_clicked();

private:
#if defined(Q_OS_WIN)
    QWinTaskbarButton m_taskbarButton;
    QWinTaskbarProgress *m_taskbarProgress;
#endif
    Ui::ProgressDialog *m_ui;
};

#endif // PROGRESSDIALOG_H
