#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include <QDialog>
#include <QNetworkReply>

#include "ResizeManager.h"

namespace Ui {
class ChangeLogDialog;
}

class ChangeLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeLogDialog(QWidget *parent = 0);
    ~ChangeLogDialog();

    void setShownAfterCheckingUpdates();

private:
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void fetchChangeLogFinished(QNetworkReply *reply);

private:
    Ui::ChangeLogDialog *ui;
    ResizeManager m_resizeMan;
};

#endif // CHANGELOGDIALOG_H
