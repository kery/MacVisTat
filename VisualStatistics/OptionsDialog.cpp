#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "GlobalDefines.h"
#include <QSettings>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    QSettings setting;
    ui->ignoreConstCheckBox->setChecked(setting.value(SETTING_KEY_IGNORE_CONSTANT, true).toBool());
    ui->hideTimeGapCheckBox->setChecked(setting.value(SETTING_KEY_HIDE_TIME_GAP, false).toBool());

    connect(ui->ignoreConstCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::ignoreConstChkBoxStateChanged);
    connect(ui->hideTimeGapCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::hideTimeGapChkBoxStateChanged);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::ignoreConstChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(SETTING_KEY_IGNORE_CONSTANT, state == Qt::Checked);
}

void OptionsDialog::hideTimeGapChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(SETTING_KEY_HIDE_TIME_GAP, state == Qt::Checked);
}

bool OptionsDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 100, 0.5);
    return QDialog::event(event);
}
