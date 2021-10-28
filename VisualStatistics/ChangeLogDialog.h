#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include <QDialog>
#include <QNetworkReply>

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

private slots:
    void fetchChangeLogFinished(QNetworkReply *reply);

private:
    Ui::ChangeLogDialog *ui;
};

#endif // CHANGELOGDIALOG_H
