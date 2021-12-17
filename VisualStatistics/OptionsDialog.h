#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "ResizeManager.h"

namespace Ui { class OptionsDialog; }

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QWidget *parent);
    ~OptionsDialog();

private slots:
    void ignoreConstChkBoxStateChanged(int state);
    void hideTimeGapChkBoxStateChanged(int state);

private:
    virtual bool event(QEvent *event) override;

    Ui::OptionsDialog *ui;
    ResizeManager mResizeMan;
};

#endif // OPTIONSDIALOG_H
