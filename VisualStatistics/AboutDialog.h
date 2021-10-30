#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

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
    Ui::AboutDialog *m_ui;
};

#endif // ABOUTDIALOG_H