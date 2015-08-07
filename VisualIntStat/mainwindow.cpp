#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include "statnamelistmodel.h"
#include "gzipfile.h"
#include <QStyleFactory>
#include <QLineEdit>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStringListModel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtConcurrent>
#include <functional>
// TODO: remove
#include <QDebug>

#define STAT_FILE_PATTERN "^([A-Z]+\\d+\\-\\d+)__intstat_(\\d{8}\\-\\d{6}|archive)\\.csv\\.gz$"

enum {
    KEY_FILE_NAME,
    KEY_START_TIME,
    KEY_END_TIME,
    KEY_LINE_COUNT
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _fusionStyle(QStyleFactory::create("Fusion")),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    // used to disable tooltip for tool button
    installEventFilterForAllToolButton();

    setAcceptDrops(true);

    _ui->mainToolBar->setStyle(_fusionStyle);

    _ui->splitterHor->setSizes(QList<int>() << 280 << width() - 280);
    _ui->splitterHor->setStretchFactor(0, 0);
    _ui->splitterHor->setStretchFactor(1, 1);
    _ui->splitteVer->setSizes(QList<int>() << height() - 80 << 80);
    _ui->splitteVer->setStretchFactor(0, 1);
    _ui->splitteVer->setStretchFactor(1, 0);

    _ui->lvStatName->setModel(new StatNameListModel(this));

    _ui->cbRegExpFilter->lineEdit()->setPlaceholderText("regular expression filter");
    connect(_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::updateFilterPattern);

    _ui->logTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::contextMenuRequest);
}

MainWindow::~MainWindow()
{
    delete _ui;
    delete _fusionStyle;
}

QString MainWindow::getStatFileNode() const
{
    QString node;
    if (_ui->lwStatFile->count() > 0) {
        QRegExp regExp(STAT_FILE_PATTERN);
        if (regExp.exactMatch(_ui->lwStatFile->item(0)->text())) {
            node = regExp.cap(1);
        }
    }
    return node;
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
        if (item->toolTip() == fileName)
            return true;
    }
    return false;
}

int MainWindow::addStatFiles(const QStringList &fileNames)
{
    QRegExp regExp(STAT_FILE_PATTERN);
    QIcon icon(":/resource/image/archive.png");
    QVector<QString> addedFileNames;
    for (const QString &fileName : fileNames) {
        QFileInfo fileInfo(fileName);
        if (!regExp.exactMatch(fileInfo.fileName())) {
            appendLogWarn(fileInfo.fileName() + " ignored. Invalid file name.");
            continue;
        }
        QString nativeName = QDir::toNativeSeparators(fileName);
        if (statFileAlreadyAdded(nativeName)) {
            continue;
        }
        if (!checkStatFileNode(regExp.cap(1))) {
            appendLogWarn(QString("%1 ignored. %2 is different from %3.").
                          arg(fileInfo.fileName()).arg(regExp.cap(1)).arg(_node));
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
        item->setCheckState(Qt::Checked);
        item->setToolTip(nativeName);
        _ui->lwStatFile->addItem(item);
        addedFileNames << nativeName;
    }

    typedef QMap<int, QString> ResultType;

    for (const QString &fileName : addedFileNames) {
        QFuture<ResultType> future = QtConcurrent::run([fileName] () {
            ResultType ret;
            const int TIME_STR_LEN = 19;
            int lineCount = 0; // Used to calculate progress range in other place

            ret.insert(KEY_FILE_NAME, fileName);

            GZipFile gzFile(fileName);
            QRegExp regExp("^\\d{2}\\.\\d{2}\\.\\d{4};\\d{2}:\\d{2}:\\d{2};");
            QString startLine, endLine;
            if (!gzFile.readLine(startLine)) {
                goto end;
            }

            // Ignore the first line because it is header
            // Read the second line
            if (!gzFile.readLine(startLine)) {
                goto end;
            }

            lineCount += 2;

            // Check format
            if (regExp.indexIn(startLine) != 0) {
                goto end;
            }

            // Get the last line
            while (gzFile.readLine(endLine)) {
                ++lineCount;
            }

            // Check format
            if (regExp.indexIn(endLine) != 0) {
                goto end;
            }

            ret.insert(KEY_START_TIME, startLine.left(TIME_STR_LEN).replace(';', ' '));
            ret.insert(KEY_END_TIME, endLine.left(TIME_STR_LEN).replace(';', ' '));
            ret.insert(KEY_LINE_COUNT, QString::number(lineCount)); // To simplity code store line count here
end:
            return ret;
        });

        QFutureWatcher<ResultType> *watcher = new QFutureWatcher<ResultType>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(handleFileInfoResult()));
        watcher->setFuture(future);
    }

    return addedFileNames.size();
}

bool MainWindow::checkStatFileNode(const QString &node)
{
    if (_node.isEmpty()) {
        _node = node;
        return true;
    } else {
        if (_node != node) {
            return false;
        }
        return true;
    }
}

void MainWindow::parseStatFileHeader()
{
    Q_ASSERT(_ui->lwStatFile->count() > 0);

    QString header;
    GZipFile gzFile(_ui->lwStatFile->item(0)->toolTip());
    if (!gzFile.readLine(header)) {
        showErrorMsgBox("Read header of statistics file failed!",
                        _ui->lwStatFile->item(0)->toolTip());
        return;
    }

    for (int i = 1; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);

        GZipFile gzFile(item->toolTip());
        QString line;
        if (gzFile.readLine(line)) {
            if (line != header) {
                showErrorMsgBox("Headers of statistics file are not identical!",
                                _ui->lwStatFile->item(0)->toolTip() + "\n" +
                                item->toolTip());
                return;
            }
        } else {
            showErrorMsgBox("Read header of statistics file failed!",
                            item->toolTip());
            return;
        }
    }

    if (!header.startsWith("##")) {
        showErrorMsgBox("Invalid statistics file's header format!",
                        _ui->lwStatFile->item(0)->toolTip());
        return;
    }

    header.remove('#');
    QStringList statNames = header.split(';');
    if (statNames.size() < 2 || statNames.at(0) != "date" || statNames.at(1) != "time") {
        showErrorMsgBox("Invalid statistics file's header format!",
                        _ui->lwStatFile->item(0)->toolTip());
        return;
    }
    statNames.removeFirst(); // remove "date"
    statNames.removeFirst(); // remove "time"

    StatNameListModel *model = static_cast<StatNameListModel*>(_ui->lvStatName->model());
    model->setStatNames(statNames);
    QString filterText = _ui->cbRegExpFilter->lineEdit()->text();
    if (!filterText.isEmpty()) {
        model->setFilterPattern(filterText);
    }
}

void MainWindow::parseStatFileData(bool multipleWindows)
{
    QVector<QListWidgetItem*> checkedItems;
    for (int i = 0; i < _ui->lwStatFile->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatFile->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedItems << item;
        }
    }

    if (checkedItems.isEmpty()) {
        showInfoMsgBox("Please check the statistics file(s) with which you want to draw plot!",
                       "No statistics file checked.");
        return;
    }

    StatNameListModel *model = static_cast<StatNameListModel*>(_ui->lvStatName->model());
    Q_ASSERT(model != NULL);
    if (model->rowCount() == 0) {
        showInfoMsgBox("Please specify at lease one statistics name which you want to draw!",
                       "No statistics name found.");
        return;
    }

    // TODO: limit the statistics count?

    QProgressDialog dialog(this);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QThread workerThread;
    ParseDataWorker worker;
    worker.moveToThread(&workerThread);
    connect(&worker, &ParseDataWorker::progressRangeWorkedOut, &dialog, &QProgressDialog::setRange);
    connect(&worker, &ParseDataWorker::progressValueUpdated, &dialog, &QProgressDialog::setValue);
    connect(&worker, &ParseDataWorker::progressLabelUpdated, &dialog, &QProgressDialog::setLabelText);
    connect(&worker, &ParseDataWorker::dataReady, this, &MainWindow::handleParsedResult);
    connect(&worker, &ParseDataWorker::dataReady, &dialog, &QProgressDialog::reset);
    connect(&dialog, &QProgressDialog::canceled, [&worker] {worker._canceled = true;});
    connect(this, &MainWindow::parseDataParamReady, &worker, &ParseDataWorker::parseData);

    workerThread.start();

    ParseDataParam param;
    param.multipleWindows = multipleWindows;
    for (QListWidgetItem *item : checkedItems) {
        // If the line count (stored in UserRole) is unknown 0 will be returned
        param.fileInfo.insert(item->toolTip(), item->data(Qt::UserRole).toInt());
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        param.statNames << model->data(model->index(i)).toString();
    }

    emit parseDataParamReady(param);
    dialog.exec();

    workerThread.quit();
    workerThread.wait();
}

void MainWindow::showInfoMsgBox(const QString &text, const QString &info)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Information");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    msgBox.exec();
}

void MainWindow::showErrorMsgBox(const QString &text, const QString &info)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Error");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    msgBox.exec();
}

void MainWindow::appendLogInfo(const QString &text)
{
    _ui->logTextEdit->appendPlainText("INFO: " + text);
}

void MainWindow::appendLogWarn(const QString &text)
{
    _ui->logTextEdit->appendHtml(QString("WARN: <font color='#CC9900'>%1</font>").arg(text));
}

void MainWindow::appendLogError(const QString &text)
{
    _ui->logTextEdit->appendHtml(QString("ERR: <font color='red'>%1</font>").arg(text));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isToolTipEventOfToolButton(obj, event)) {
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
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

void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList fileNames;
    for (const QUrl &url : event->mimeData()->urls()) {
        fileNames.append(url.toLocalFile());
    }
    if (fileNames.size() > 0) {
        int preCount = _ui->lwStatFile->count();
        int added = addStatFiles(fileNames);
        if (preCount == 0 && added > 0) {
            parseStatFileHeader();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    emit aboutToBeClosed();
}

void MainWindow::updateFilterPattern()
{
    static_cast<StatNameListModel*>(_ui->lvStatName->model())->setFilterPattern(
                _ui->cbRegExpFilter->lineEdit()->text());
}

void MainWindow::handleParsedResult(const ParsedResult &result, bool multipleWindows)
{
    if (multipleWindows) {
        for (auto iter = result.data.begin(); iter != result.data.end(); ++iter) {
            ParsedResult tmpResult;
            tmpResult.dateTimes = result.dateTimes;

            QMap<QString, QCPDataMap*> data;
            data.insert(iter.key(), iter.value());
            tmpResult.data = data;
            PlotWindow *w = new PlotWindow(getStatFileNode(), tmpResult);
            w->setAttribute(Qt::WA_DeleteOnClose);
            connect(this, SIGNAL(aboutToBeClosed()), w, SLOT(close()));
            w->showMaximized();
        }
    } else {
        PlotWindow *w = new PlotWindow(getStatFileNode(), result);
        w->setAttribute(Qt::WA_DeleteOnClose);
        connect(this, SIGNAL(aboutToBeClosed()), w, SLOT(close()));
        w->showMaximized();
    }
}

void MainWindow::contextMenuRequest(const QPoint &pos)
{
    QMenu *menu = _ui->logTextEdit->createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction("Clear", this, SLOT(clearLogEdit()));

    menu->popup(_ui->logTextEdit->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    _ui->logTextEdit->clear();
}

void MainWindow::handleFileInfoResult()
{
    typedef QMap<int, QString> ResultType;

    QFutureWatcher<ResultType> *watcher = static_cast<QFutureWatcher<ResultType>*>(sender());
    ResultType result = watcher->result();
    QFileInfo fileInfo(result.value(KEY_FILE_NAME));
    if (result.size() > 1) {
        QList<QListWidgetItem*> items = _ui->lwStatFile->findItems(fileInfo.fileName(),
            Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if (items.size() > 0) {
            items.at(0)->setStatusTip(QString("Time duration: %1 - %2").arg(result.value(KEY_START_TIME)).arg(result.value(KEY_END_TIME)));
            int lineCount = result.value(KEY_LINE_COUNT).toInt();
            if (lineCount) {
                items.at(0)->setData(Qt::UserRole, lineCount);
            }
        }
    } else {
        appendLogError(QString("Parse file %1 failed.").arg(fileInfo.fileName()));
    }

    watcher->deleteLater();
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter("Internal Statistics File (*.csv.gz)");

    if (fileDialog.exec() == QDialog::Accepted) {
        int preCount = _ui->lwStatFile->count();
        int added = addStatFiles(fileDialog.selectedFiles());
        if (preCount == 0 && added > 0) {
            parseStatFileHeader();
        }
    }
}

void MainWindow::on_actionCloseAll_triggered()
{
    _node.clear();
    static_cast<StatNameListModel*>(_ui->lvStatName->model())->clearStatNames();
    for (int i = _ui->lwStatFile->count() - 1; i >= 0; --i) {
        delete _ui->lwStatFile->item(i);
    }
}

void MainWindow::on_actionDrawPlot_triggered()
{
    parseStatFileData(false);
}

void MainWindow::on_actionDrawPlotInMultipleWindows_triggered()
{
    parseStatFileData(true);
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
