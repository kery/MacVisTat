#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include <QDialog>
#include "ResizeManager.h"

namespace Ui { class ChangeLogDialog; }

class QNetworkReply;

class ChangeLogDialog : public QDialog
{
    Q_OBJECT

public:
    ChangeLogDialog(QWidget *parent, bool update);
    ~ChangeLogDialog();

private:
    virtual bool event(QEvent *event) override;

    Q_SLOT void fetchChangeLogFinished();

    Ui::ChangeLogDialog *ui;
    ResizeManager mResizeMan;
};

#endif // CHANGELOGDIALOG_H
