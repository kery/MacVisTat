#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include <QStyleFactory>
#include <QLineEdit>
#include <QFileDialog>
#include <QSortFilterProxyModel>
// TODO: remove
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    fusionStyle(QStyleFactory::create("Fusion")),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    installEventFilterForAllToolButton();

    ui->mainToolBar->setStyle(fusionStyle);

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionListView);
    actionGroup->addAction(ui->actionTreeView);

    ui->splitter->setSizes(QList<int>() << 250 << 400);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->cbRegExpFilter->lineEdit()->setPlaceholderText("regular expression filter");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fusionStyle;
}

void MainWindow::installEventFilterForAllToolButton()
{
    for (QObject *btn : ui->mainToolBar->findChildren<QObject*>()) {
        btn->installEventFilter(this);
    }
}

bool MainWindow::isToolTipEventOfToolButton(QObject *obj, QEvent *event)
{
    return event->type() == QEvent::ToolTip && obj->parent() == ui->mainToolBar;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isToolTipEventOfToolButton(obj, event)) {
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter("Internal Statistics File (*.csv.gz)");

    QSortFilterProxyModel proxyModel;
    proxyModel.setFilterRegExp("test");

    fileDialog.setProxyModel(&proxyModel);
    if (fileDialog.exec() == QDialog::Accepted) {
    }
}

void MainWindow::on_actionCloseAll_triggered()
{
}

void MainWindow::on_actionDrawPlot_triggered()
{
    // TODO: check if there is data to draw

    // TEST
    PlotWindow *plotWindow = new PlotWindow(this);
    plotWindow->setAttribute(Qt::WA_DeleteOnClose);
    plotWindow->showMaximized();
}

void MainWindow::on_actionListView_toggled(bool checked)
{
}

void MainWindow::on_actionTreeView_toggled(bool checked)
{
}

void MainWindow::on_actionSelectAll_triggered()
{
    for (int i = 0; i < ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = ui->lwStatFile->item(i);
        if (item->checkState() != Qt::Checked) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_actionClearSelection_triggered()
{
    for (int i = 0; i < ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = ui->lwStatFile->item(i);
        if (item->checkState() != Qt::Unchecked) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::on_actionInvertSelection_triggered()
{
    for (int i = 0; i < ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = ui->lwStatFile->item(i);
        switch (item->checkState()) {
        case Qt::Checked:
            item->setCheckState(Qt::Unchecked);
            break;
        case Qt::Unchecked:
            item->setCheckState(Qt::Checked);
            break;
        }
    }
}
