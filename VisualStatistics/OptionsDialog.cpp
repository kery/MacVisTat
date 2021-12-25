#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "GlobalDefines.h"
#include <QSettings>

QString OptionsDialog::sKeyIgnoreConstant("ignoreConstant");
QString OptionsDialog::sKeyHideTimeGap("hideTimeGap");

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    QSettings setting;
    ui->ignoreConstCheckBox->setChecked(setting.value(sKeyIgnoreConstant, true).toBool());
    ui->hideTimeGapCheckBox->setChecked(setting.value(sKeyHideTimeGap, false).toBool());

    connect(ui->ignoreConstCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::ignoreConstChkBoxStateChanged);
    connect(ui->hideTimeGapCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::hideTimeGapChkBoxStateChanged);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

bool OptionsDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 100, 0.5);
    return QDialog::event(event);
}

void OptionsDialog::ignoreConstChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(sKeyIgnoreConstant, state == Qt::Checked);
}

void OptionsDialog::hideTimeGapChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(sKeyHideTimeGap, state == Qt::Checked);
}
