#include "JobIdSelectionDialog.h"
#include "ui_JobIdSelectionDialog.h"
#include <QCloseEvent>

JobIdSelectionDialog::JobIdSelectionDialog(QWidget *parent, const QSet<QString> jobIds) :
    QDialog(parent, Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::JobIdSelectionDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &JobIdSelectionDialog::accept);

    for (const QString &jobId : jobIds) {
        ui->jobIdComboBox->addItem(jobId);
    }
    ui->jobIdComboBox->model()->sort(0);
}

JobIdSelectionDialog::~JobIdSelectionDialog()
{
    delete ui;
}

QString JobIdSelectionDialog::selectedJobId() const
{
    return ui->jobIdComboBox->currentText();
}

void JobIdSelectionDialog::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void JobIdSelectionDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    }
}

bool JobIdSelectionDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 80, 0.32);
    return QDialog::event(event);
}
