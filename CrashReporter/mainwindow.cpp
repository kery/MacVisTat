#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _dumpFilePath(path)
{
    _ui->setupUi(this);
    setFixedSize(size());

    _ui->textBrowser->setText("An unhandled exception occurred, which cause the program exit unexpectedly.\n\n"
                              "Do you want to upload the core dump file so that this bug can be fixed in future version?");
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::on_buttonBox_accepted()
{
}
