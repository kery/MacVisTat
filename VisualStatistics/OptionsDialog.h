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

private:
    virtual bool event(QEvent *event) override;
    virtual void accept() override;

public:
    static bool sDefIgnoreConstant;
    static bool sDefAbortConvOnFailure;

    static QString sKeyIgnoreConstant;
    static QString sKeyAbortConvOnFailure;

private:
    Ui::OptionsDialog *ui;
    ResizeManager mResizeMan;
};

#endif // OPTIONSDIALOG_H
