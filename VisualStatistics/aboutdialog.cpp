#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);
    m_ui->labelMailTo->setOpenExternalLinks(true);
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}

void AboutDialog::setLabelText(const QString &text)
{
    m_ui->label->setText(text);
}
