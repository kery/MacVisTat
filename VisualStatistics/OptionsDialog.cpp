#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "GlobalDefines.h"
#include "MainWindow.h"
#include <QSettings>

bool OptionsDialog::sDefIgnoreConstant = true;
bool OptionsDialog::sDefAbortConvOnFailure = true;
bool OptionsDialog::sDefLoadOnlinePlugins = true;

QString OptionsDialog::sKeyIgnoreConstant("ignoreConstant");
QString OptionsDialog::sKeyAbortConvOnFailure("abortConvOnFailure");
QString OptionsDialog::sKeyLoadOnlinePlugins("loadOnlinePlugins");

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    mResizeMan(this)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    QSettings setting;
    ui->ignoreConstCheckBox->setChecked(setting.value(sKeyIgnoreConstant, sDefIgnoreConstant).toBool());
    ui->abortConvOnFailureCheckBox->setChecked(setting.value(sKeyAbortConvOnFailure, sDefAbortConvOnFailure).toBool());
    ui->loadOnlinePluginsCheckBox->setChecked(setting.value(sKeyLoadOnlinePlugins, sDefLoadOnlinePlugins).toBool());
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

void OptionsDialog::accept()
{
    QDialog::accept();

    QSettings setting;
    setting.setValue(sKeyIgnoreConstant, ui->ignoreConstCheckBox->isChecked());
    setting.setValue(sKeyAbortConvOnFailure, ui->abortConvOnFailureCheckBox->isChecked());
    setting.setValue(sKeyLoadOnlinePlugins, ui->loadOnlinePluginsCheckBox->isChecked());
}
