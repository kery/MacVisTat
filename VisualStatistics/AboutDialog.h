#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "ResizeManager.h"

namespace Ui { class AboutDialog; }

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent);
    ~AboutDialog();

private:
    virtual bool event(QEvent *event) override;

    Ui::AboutDialog *ui;
    ResizeManager mResizeMan;
};

#endif // ABOUTDIALOG_H
