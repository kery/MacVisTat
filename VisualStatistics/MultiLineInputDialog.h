#ifndef MULTILINEINPUTDIALOG_H
#define MULTILINEINPUTDIALOG_H

#include <QInputDialog>
#include "ResizeManager.h"

class MultiLineInputDialog : public QInputDialog
{
public:
    MultiLineInputDialog(QWidget *widget);

private:
    virtual bool event(QEvent *event) override;

    ResizeManager mResizeMan;
};

#endif // MULTILINEINPUTDIALOG_H
