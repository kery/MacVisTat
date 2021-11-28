#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"
#include <QKeyEvent>
#include <QWinTaskbarProgress>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    _resizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgressDialog::cancelButtonClicked);

    _taskbarButton.setWindow(parent->windowHandle());
    _taskbarProgress = _taskbarButton.progress();
    _taskbarProgress->setVisible(true);
}

ProgressDialog::~ProgressDialog()
{
    _taskbarProgress->setVisible(false);
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
    _taskbarProgress->setRange(0, 0);
}

void ProgressDialog::setRange(int min, int max)
{
    ui->progressBar->setRange(min, max);
    ui->progressBar->setValue(min);
    _taskbarProgress->setRange(min, max);
    _taskbarProgress->setValue(min);
}

void ProgressDialog::setValue(int value)
{
    ui->progressBar->setValue(value);
    _taskbarProgress->setValue(value);
}

void ProgressDialog::cancelButtonClicked()
{
    setDescription(QStringLiteral("Canceling..."));
    ui->cancelButton->setEnabled(false);
    emit canceling();
}

void ProgressDialog::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

bool ProgressDialog::event(QEvent *event)
{
    _resizeMan.resizeWidgetFromCharWidth(event, 80, 0.33333);
    return QDialog::event(event);
}
