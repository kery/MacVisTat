#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "Version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    ui->verLabel->setText(ui->verLabel->text().arg(VER_FILEVERSION_STR));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

bool AboutDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 80, 0.45);
    return QDialog::event(event);
}
