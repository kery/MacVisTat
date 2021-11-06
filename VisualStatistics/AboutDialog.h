#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include "ResizeManager.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent);
    AboutDialog(const AboutDialog &) = delete;
    AboutDialog& operator=(const AboutDialog &) = delete;
    ~AboutDialog();

private:
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::AboutDialog *m_ui;
    ResizeManager m_resizeMan;
};

#endif // ABOUTDIALOG_H
