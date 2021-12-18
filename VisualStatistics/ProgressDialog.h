#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QWinTaskbarButton>
#include "ResizeManager.h"

namespace Ui { class ProgressDialog; }

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
    ProgressDialog(QWidget *parent);
    ~ProgressDialog();

    void setDescription(const QString &text);
    void setCancelButtonVisible(bool visible);
    void setUndeterminable();

public slots:
    void setRange(int min, int max);
    void setValue(int value);

signals:
    void canceling();

private slots:
    void cancelButtonClicked();

private:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool event(QEvent *event) override;

    Ui::ProgressDialog *ui;
    QWinTaskbarButton mTaskbarButton;
    QWinTaskbarProgress *mTaskbarProgress;
    ResizeManager mResizeMan;
};

#endif // PROGRESSDIALOG_H
