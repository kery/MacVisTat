#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "Version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog),
    m_resizeMan(this)
{
    m_ui->setupUi(this);
    m_ui->labelMailTo->setOpenExternalLinks(true);

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    m_ui->label->setText(QStringLiteral("Visual Statistics v%1").arg(VER_FILEVERSION_STR));
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}

bool AboutDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShowToParent && !m_resizeMan.showToParentHandled()) {
        m_resizeMan.resizeWidgetFromCharWidth(80, 0.45);
    }
    return QDialog::event(event);
}
