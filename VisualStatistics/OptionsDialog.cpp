#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "GlobalDefines.h"
#include "MainWindow.h"
#include <QSettings>

bool OptionsDialog::sDefIgnoreConstant = true;
bool OptionsDialog::sDefAbortConvOnFailure = true;

QString OptionsDialog::sKeyIgnoreConstant("ignoreConstant");
QString OptionsDialog::sKeyAbortConvOnFailure("abortConvOnFailure");

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
}
