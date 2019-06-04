#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);
    m_ui->labelMailTo->setOpenExternalLinks(true);

    setFixedSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_ui->label->setText(QStringLiteral("Visual Statistics v%1").arg(VER_FILEVERSION_STR));
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}
