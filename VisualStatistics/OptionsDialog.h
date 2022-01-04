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

    Q_SLOT void ignoreConstChkBoxStateChanged(int state);
    Q_SLOT void yAxis2DraggableZoomableChkBoxStateChanged(int state);
    Q_SLOT void abortConvOnFailureChkBoxStateChanged(int state);

public:
    static bool sDefIgnoreConstant;
    static bool sDefYAxis2DraggableZoomable;
    static bool sDefAbortConvOnFailure;

    static QString sKeyIgnoreConstant;
    static QString sKeyYAxis2DraggableZoomable;
    static QString sKeyAbortConvOnFailure;

private:
    Ui::OptionsDialog *ui;
    ResizeManager mResizeMan;
};

#endif // OPTIONSDIALOG_H
