#include "progressdialog.h"
#include "ui_progressdialog.h"

#include <QKeyEvent>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProgressDialog)
{
    m_ui->setupUi(this);

    setFixedSize(size());
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

#if defined(Q_OS_WIN)
    m_taskbarButton.setWindow(parent->windowHandle());
    m_taskbarProgress = m_taskbarButton.progress();
    m_taskbarProgress->setVisible(true);
#endif
}

ProgressDialog::~ProgressDialog()
{
#if defined(Q_OS_WIN)
    m_taskbarProgress->setVisible(false);
#endif
    delete m_ui;
}

void ProgressDialog::setLabelText(const QString &text)
{
    m_ui->label->setText(text);
}

void ProgressDialog::enableCancelButton(bool enabled)
{
    m_ui->pushButton->setEnabled(enabled);
}

void ProgressDialog::busyIndicatorMode()
{
    m_ui->progressBar->setRange(0, 0);
#if defined(Q_OS_WIN)
    m_taskbarProgress->setRange(0, 0);
#endif
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    m_ui->progressBar->setRange(minimum, maximum);
    m_ui->progressBar->setValue(minimum);
#if defined(Q_OS_WIN)
    m_taskbarProgress->setRange(minimum, maximum);
    m_taskbarProgress->setValue(minimum);
#endif
}

void ProgressDialog::setValue(int progress)
{
    m_ui->progressBar->setValue(progress);
#if defined(Q_OS_WIN)
    m_taskbarProgress->setValue(progress);
#endif
}

void ProgressDialog::on_pushButton_clicked()
{
    setLabelText(QStringLiteral("Canceling..."));
    enableCancelButton(false);
    emit canceling();
}

void ProgressDialog::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}
