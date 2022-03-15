#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"
#include <QKeyEvent>
#if defined(Q_OS_WIN)
#include <QWinTaskbarProgress>
#endif

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgressDialog::cancelButtonClicked);

#if defined(Q_OS_WIN)
    mTaskbarButton.setWindow(parent->windowHandle());
    mTaskbarProgress = mTaskbarButton.progress();
    mTaskbarProgress->setVisible(true);
#endif
}

ProgressDialog::~ProgressDialog()
{
#if defined(Q_OS_WIN)
    mTaskbarProgress->setVisible(false);
#endif
    delete ui;
}

void ProgressDialog::setDescription(const QString &text)
{
    ui->descLabel->setText(text);
}

void ProgressDialog::setCancelButtonVisible(bool visible)
{
    ui->cancelButton->setVisible(visible);
}

void ProgressDialog::setUndeterminable()
{
    ui->progressBar->setRange(0, 0);
#if defined(Q_OS_WIN)
    mTaskbarProgress->setRange(0, 0);
#endif
}

void ProgressDialog::setRange(int min, int max)
{
    ui->progressBar->setRange(min, max);
    ui->progressBar->setValue(min);
#if defined(Q_OS_WIN)
    mTaskbarProgress->setRange(min, max);
    mTaskbarProgress->setValue(min);
#endif
}

void ProgressDialog::setValue(int value)
{
    ui->progressBar->setValue(value);
#if defined(Q_OS_WIN)
    mTaskbarProgress->setValue(value);
#endif
}

void ProgressDialog::cancelButtonClicked()
{
    setDescription(QStringLiteral("Canceling..."));
    ui->cancelButton->setEnabled(false);
    emit canceling();
}

// Ignore keyboard event, especially the Esc, to prevent it from
// being closed. When Esc pressed, closeEvent will not be called.
// This is documented in QDialog page.
void ProgressDialog::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

// Ignore close event, e.g. Alt+F4
void ProgressDialog::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

bool ProgressDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 80, 0.33333);
    return QDialog::event(event);
}
