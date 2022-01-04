#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "GlobalDefines.h"
#include "MainWindow.h"
#include <QSettings>

bool OptionsDialog::sDefIgnoreConstant = true;
bool OptionsDialog::sDefYAxis2DraggableZoomable = false;
bool OptionsDialog::sDefAbortConvOnFailure = true;

QString OptionsDialog::sKeyIgnoreConstant("ignoreConstant");
QString OptionsDialog::sKeyYAxis2DraggableZoomable("yAxis2DraggableZoomable");
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
    ui->yAxis2DraggableZoomableCheckBox->setChecked(setting.value(sKeyYAxis2DraggableZoomable, sDefYAxis2DraggableZoomable).toBool());
    ui->abortConvOnFailureCheckBox->setChecked(setting.value(sKeyAbortConvOnFailure, sDefAbortConvOnFailure).toBool());

    connect(ui->ignoreConstCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::ignoreConstChkBoxStateChanged);
    connect(ui->yAxis2DraggableZoomableCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::yAxis2DraggableZoomableChkBoxStateChanged);
    connect(ui->yAxis2DraggableZoomableCheckBox, &QCheckBox::stateChanged, qobject_cast<MainWindow *>(parent), &MainWindow::yAxis2DraggableZoomableStateChanged);
    connect(ui->abortConvOnFailureCheckBox, &QCheckBox::stateChanged, this, &OptionsDialog::abortConvOnFailureChkBoxStateChanged);
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

void OptionsDialog::yAxis2DraggableZoomableChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(sKeyYAxis2DraggableZoomable, state == Qt::Checked);
}

void OptionsDialog::abortConvOnFailureChkBoxStateChanged(int state)
{
    QSettings setting;
    setting.setValue(sKeyAbortConvOnFailure, state == Qt::Checked);
}
