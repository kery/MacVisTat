#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"
#include <QKeyEvent>
#include <QWinTaskbarProgress>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgressDialog::cancelButtonClicked);

    mTaskbarButton.setWindow(parent->windowHandle());
    mTaskbarProgress = mTaskbarButton.progress();
    mTaskbarProgress->setVisible(true);
}

ProgressDialog::~ProgressDialog()
{
    mTaskbarProgress->setVisible(false);
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
    mTaskbarProgress->setRange(0, 0);
}

void ProgressDialog::setRange(int min, int max)
{
    ui->progressBar->setRange(min, max);
    ui->progressBar->setValue(min);
    mTaskbarProgress->setRange(min, max);
    mTaskbarProgress->setValue(min);
}

void ProgressDialog::setValue(int value)
{
    ui->progressBar->setValue(value);
    mTaskbarProgress->setValue(value);
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
