#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include <QStyleFactory>
#include <QLineEdit>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QErrorMessage>
// TODO: remove
#include <QDebug>

#define STAT_FILE_PATTERN "^([A-Z]+\\d+\\-\\d+)__intstat_(\\d{8}\\-\\d{6}|archive)\\.csv\\.gz$"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _fusionStyle(QStyleFactory::create("Fusion")),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    installEventFilterForAllToolButton();

    setAcceptDrops(true);

    _ui->mainToolBar->setStyle(_fusionStyle);

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(_ui->actionListView);
    actionGroup->addAction(_ui->actionTreeView);

    _ui->splitter->setSizes(QList<int>() << 300 << 500);
    _ui->splitter->setStretchFactor(0, 0);
    _ui->splitter->setStretchFactor(1, 1);
    _ui->cbRegExpFilter->lineEdit()->setPlaceholderText("regular expression filter");
}

MainWindow::~MainWindow()
{
    delete _ui;
    delete _fusionStyle;
}

void MainWindow::installEventFilterForAllToolButton()
{
    for (QObject *btn : _ui->mainToolBar->findChildren<QObject*>()) {
        btn->installEventFilter(this);
    }
}

bool MainWindow::isToolTipEventOfToolButton(QObject *obj, QEvent *event)
{
    return event->type() == QEvent::ToolTip && obj->parent() == _ui->mainToolBar;
}

bool MainWindow::statFileAlreadyAdded(const QString &fileName)
{
    for (int i = 0; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);
        if (item->statusTip() == fileName)
            return true;
    }
    return false;
}

void MainWindow::addStatFiles(const QStringList &fileNames)
{
    QRegExp regExp(STAT_FILE_PATTERN);
    QIcon icon(":/resource/image/archive.png");
    for (const QString &fileName : fileNames) {
        QFileInfo fileInfo(fileName);
        if (!regExp.exactMatch(fileInfo.fileName()))
            continue;
        QString nativeName = QDir::toNativeSeparators(fileName);
        if (statFileAlreadyAdded(nativeName))
            continue;
        if (!checkStatFileNode(regExp.cap(1)))
            continue;
        QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
        item->setCheckState(Qt::Checked);
        item->setStatusTip(nativeName);
        _ui->lwStatFile->addItem(item);
    }
}

bool MainWindow::checkStatFileNode(const QString &node)
{
    if (_node.isEmpty()) {
        _node = node;
        return true;
    } else {
        if (_node != node) {
            // TODO: show error message box
            return false;
        }
        return true;
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isToolTipEventOfToolButton(obj, event)) {
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    QRegExp regExp(STAT_FILE_PATTERN);
    for (const QUrl &url : event->mimeData()->urls()) {
        QFileInfo fileInfo(url.toLocalFile());
        if (regExp.exactMatch(fileInfo.fileName())) {
            event->acceptProposedAction();
            return;
        }
    }
}

void MainWindow::dropEvent(QDropEvent * event)
{
    QStringList fileNames;
    for (const QUrl &url : event->mimeData()->urls()) {
        fileNames.append(url.toLocalFile());
    }
    if (fileNames.size() > 0) {
        addStatFiles(fileNames);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter("Internal Statistics File (*.csv.gz)");

    if (fileDialog.exec() == QDialog::Accepted) {
        addStatFiles(fileDialog.selectedFiles());
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
    for (int i = 0; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);
        if (item->checkState() != Qt::Checked) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_actionClearSelection_triggered()
{
    for (int i = 0; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);
        if (item->checkState() != Qt::Unchecked) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::on_actionInvertSelection_triggered()
{
    for (int i = 0; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);
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
