#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::AboutDialog)
{
    _ui->setupUi(this);
    _ui->labelMailTo->setOpenExternalLinks(true);
}

AboutDialog::~AboutDialog()
{
    delete _ui;
}

void AboutDialog::setLabelText(const QString &text)
{
    _ui->label->setText(text);
}
