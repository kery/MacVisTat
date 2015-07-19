#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include <QStyleFactory>
#include <QLineEdit>
// TODO: remove
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    fusionStyle(QStyleFactory::create("Fusion")),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    disableToolbarTooltip();

    ui->mainToolBar->setStyle(fusionStyle);

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionListView);
    actionGroup->addAction(ui->actionTreeView);

    ui->splitter->setSizes(QList<int>() << 200 << 400);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->cbRegExpFilter->lineEdit()->setPlaceholderText("regular expression filter");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fusionStyle;
}

void MainWindow::disableToolbarTooltip()
{
    for (QObject *btn : ui->mainToolBar->findChildren<QObject*>()) {
        btn->installEventFilter(this);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_actionOpen_triggered()
{
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

void MainWindow::on_actionListView_toggled(bool arg1)
{
    qDebug() << "list view toggled: " << arg1;
}

void MainWindow::on_actionTreeView_toggled(bool arg1)
{
    qDebug() << "tree view toggled: " << arg1;
}
