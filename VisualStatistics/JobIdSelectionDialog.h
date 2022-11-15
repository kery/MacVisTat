#ifndef JOBIDSELECTIONDIALOG_H
#define JOBIDSELECTIONDIALOG_H

#include <QDialog>
#include "ResizeManager.h"

namespace Ui {
class JobIdSelectionDialog;
}

class JobIdSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JobIdSelectionDialog(QWidget *parent, const QSet<QString> jobIds);
    ~JobIdSelectionDialog();

    QString selectedJobId() const;

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual bool event(QEvent *event) override;

private:
    Ui::JobIdSelectionDialog *ui;
    ResizeManager mResizeMan;
};

#endif // JOBIDSELECTIONDIALOG_H
