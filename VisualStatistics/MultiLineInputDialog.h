#ifndef MULTILINEINPUTDIALOG_H
#define MULTILINEINPUTDIALOG_H

#include <QInputDialog>

#include "ResizeManager.h"

class MultiLineInputDialog : public QInputDialog
{
public:
    MultiLineInputDialog(QWidget *widget);

protected:
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    ResizeManager m_resizeMan;
};

#endif // MULTILINEINPUTDIALOG_H
