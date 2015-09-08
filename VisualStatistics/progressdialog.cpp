#include "progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QProgressDialog(parent)
{
#if defined(Q_OS_WIN)
    _taskbarButton.setWindow(parent->windowHandle());
    _taskbarProgress = _taskbarButton.progress();
    _taskbarProgress->setVisible(true);
#endif
}

ProgressDialog::~ProgressDialog()
{
#if defined(Q_OS_WIN)
    _taskbarProgress->setVisible(false);
#endif
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    QProgressDialog::setRange(minimum, maximum);
#if defined(Q_OS_WIN)
    _taskbarProgress->setRange(minimum, maximum);
#endif
}

void ProgressDialog::setValue(int progress)
{
    QProgressDialog::setValue(progress);
#if defined(Q_OS_WIN)
    _taskbarProgress->setValue(progress);
#endif
}

void ProgressDialog::increaseValue(int value)
{
    value += this->value();
    setValue(value);
#if defined(Q_OS_WIN)
    _taskbarProgress->setValue(value);
#endif
}
