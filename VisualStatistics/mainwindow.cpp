#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include "statisticsnamemodel.h"
#include "gzipfile.h"
#include "utils.h"
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

#define STAT_FILE_PATTERN "^([A-Z]+\\d+\\-\\d+)__intstat_(\\d{8}\\-\\d{6}|archive)\\.csv\\.gz$"

enum {
    KEY_FILE_NAME,
    KEY_START_TIME,
    KEY_END_TIME,
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    // used to disable tooltip for tool button
    installEventFilterForAllToolButton();

    setAcceptDrops(true);

    _ui->splitterHor->setSizes(QList<int>() << 280 << width() - 280);
    _ui->splitterHor->setStretchFactor(0, 0);
    _ui->splitterHor->setStretchFactor(1, 1);
    _ui->splitteVer->setSizes(QList<int>() << height() - 80 << 80);
    _ui->splitteVer->setStretchFactor(0, 1);
    _ui->splitteVer->setStretchFactor(1, 0);

    _ui->lvStatisticsName->setModel(new StatisticsNameModel(this));

    _ui->cbRegExpFilter->lineEdit()->setPlaceholderText(QStringLiteral("regular expression filter"));
    connect(_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::updateFilterPattern);

    _ui->logTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::contextMenuRequest);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

QString MainWindow::getStatisticsFileNode() const
{
    QString node;
    if (_ui->lwStatisticsFiles->count() > 0) {
        QRegExp regExp(STAT_FILE_PATTERN);
        if (regExp.exactMatch(_ui->lwStatisticsFiles->item(0)->text())) {
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

bool MainWindow::statisticsFileAlreadyAdded(const QString &fileName)
{
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        if (item->toolTip() == fileName)
            return true;
    }
    return false;
}

int MainWindow::addStatisticsFiles(const QStringList &fileNames)
{
    QRegExp regExp(STAT_FILE_PATTERN);
    QIcon icon(QStringLiteral(":/resource/image/archive.png"));
    QVector<QString> addedFileNames;
    for (const QString &fileName : fileNames) {
        QFileInfo fileInfo(fileName);
        if (!regExp.exactMatch(fileInfo.fileName())) {
            appendLogWarn(fileInfo.fileName() + " ignored. Invalid file name.");
            continue;
        }
        QString nativeName = QDir::toNativeSeparators(fileName);
        if (statisticsFileAlreadyAdded(nativeName)) {
            continue;
        }
        if (!checkStatisticsFileNode(regExp.cap(1))) {
            appendLogWarn(QString("%1 ignored. Please add only one node's statistics files!").arg(fileInfo.fileName()));
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
        item->setCheckState(Qt::Checked);
        item->setToolTip(nativeName);
        _ui->lwStatisticsFiles->addItem(item);
        addedFileNames << nativeName;
    }

    std::function<TimeDurationResult (const QString &)> mappedFunction = [] (const QString &fileName) {
        TimeDurationResult result;
        const int TIME_STR_LEN = 19;

        result.insert(KEY_FILE_NAME, fileName);

        GZipFile gzFile(fileName);
        QRegExp regExp(QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4};\\d{2}:\\d{2}:\\d{2};"));
        std::string startLine, endLine;
        if (!gzFile.readLine(startLine)) {
            goto end;
        }

        // Ignore the first line because it is header
        // Read the second line
        if (!gzFile.readLine(startLine)) {
            goto end;
        }

        // Check format
        if (regExp.indexIn(QString::fromStdString(startLine)) != 0) {
            goto end;
        }

        // Get the last line
        while (gzFile.readLine(endLine));

        // Check format
        if (regExp.indexIn(QString::fromStdString(endLine)) != 0) {
            goto end;
        }

        result.insert(KEY_START_TIME, QString::fromStdString(startLine).left(TIME_STR_LEN).replace(';', ' '));
        result.insert(KEY_END_TIME, QString::fromStdString(endLine).left(TIME_STR_LEN).replace(';', ' '));
end:
        return result;
    };

    QFutureWatcher<TimeDurationResult> *watcher = new QFutureWatcher<TimeDurationResult>(this);
    connect(watcher, SIGNAL(resultReadyAt(int)), SLOT(handleTimeDurationResult(int)));
    connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));

    watcher->setFuture(QtConcurrent::mapped(addedFileNames, mappedFunction));

    return addedFileNames.size();
}

bool MainWindow::checkStatisticsFileNode(const QString &node)
{
    if (_ui->lwStatisticsFiles->count() == 0) {
        return true;
    }

    QListWidgetItem *item = _ui->lwStatisticsFiles->item(0);
    QRegExp regExp(STAT_FILE_PATTERN);
    regExp.exactMatch(item->text());
    return regExp.cap(1) == node;
}

void MainWindow::parseStatisticsFileHeader()
{
    Q_ASSERT(_ui->lwStatisticsFiles->count() > 0);

    std::string header;
    GZipFile gzFile(_ui->lwStatisticsFiles->item(0)->toolTip());
    if (!gzFile.readLine(header)) {
        showErrorMsgBox(QStringLiteral("Read header of statistics file failed!"),
                        _ui->lwStatisticsFiles->item(0)->toolTip());
        return;
    }

    for (int i = 1; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);

        GZipFile gzFile(item->toolTip());
        std::string line;
        if (gzFile.readLine(line)) {
            if (line != header) {
                showErrorMsgBox(QStringLiteral("Headers of statistics file are not identical!"),
                                _ui->lwStatisticsFiles->item(0)->toolTip() + "\n" + item->toolTip());
                return;
            }
        } else {
            showErrorMsgBox(QStringLiteral("Read header of statistics file failed!"),
                            item->toolTip());
            return;
        }
    }

    if (header.find("##date;time;") != 0 || header.rfind("##") != header.length() - 2) {
        showErrorMsgBox(QStringLiteral("Invalid statistics file's header format!"),
                        _ui->lwStatisticsFiles->item(0)->toolTip());
        return;
    }

    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(_ui->lvStatisticsName->model());
    model->beforeDataContainerUpdate();
    std::vector<std::string> &container = model->getDataContainer();
    container.resize(0);
    splitString(header.c_str() + sizeof("##date;time;") - 1, ';', container);
    container.back().erase(container.back().length() - 2);
    model->endDataContainerUpdate();

    QString filterText = _ui->cbRegExpFilter->lineEdit()->text();
    if (!filterText.isEmpty()) {
        model->setFilterPattern(filterText);
    }
}

void MainWindow::parseStatisticsFileData(bool multipleWindows)
{
    QVector<QListWidgetItem*> checkedItems;
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedItems << item;
        }
    }

    if (checkedItems.isEmpty()) {
        showInfoMsgBox(QStringLiteral("Please check the statistics file(s) with which you want to draw plot!"),
                       QStringLiteral("No statistics file checked."));
        return;
    }

    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(_ui->lvStatisticsName->model());
    Q_ASSERT(model != NULL);
    if (model->rowCount() == 0) {
        showInfoMsgBox(QStringLiteral("Please specify at lease one statistics name which you want to draw!"),
                       QStringLiteral("No statistics name found."));
        return;
    }

    if (model->rowCount() > PlotWindow::predefinedColorCount()) {
        showInfoMsgBox(QStringLiteral("Too many statistics names specified, please change your filter text!"),
                       QString("At most %1 statistics names allowed at one time.").arg(PlotWindow::predefinedColorCount()));
        return;
    }

    QMap<QString, int> statisticsNames;
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex modelIndex = model->index(i);
        statisticsNames.insert(model->data(modelIndex).toString(), model->data(modelIndex, Qt::UserRole).toInt());
    }

    volatile bool working = true;
    std::function<StatisticsResult (const QString &)> mappedFunction = [statisticsNames, &working] (const QString &fileName) {
        StatisticsResult result;
        QString line;
        QCPData data;
        GZipFile gzFile(fileName);
        // Read the header
        if (!gzFile.readLine(line)) {
            result.failedFile << fileName;
            goto end;
        }
        while (working && gzFile.readLine(line)) {
            QDateTime dt = QDateTime::fromString(line.left(19), "dd.MM.yyyy;HH:mm:ss");
            if (dt.isValid()) {
                data.key = dt.toTime_t();
                QVector<QStringRef> refs = line.splitRef(';');
                for (auto iter = statisticsNames.begin(); iter != statisticsNames.end(); ++iter) {
                    if (iter.value() < refs.size()) {
                        data.value = refs.at(iter.value()).toInt();
                        result.statistics[iter.key()].insert(data.key, data);
                    } else {
                        result.failedFile << fileName;
                        goto end;
                    }
                }
            } else {
                result.failedFile << fileName;
                goto end;
            }
        }
end:
        return result;
    };

    std::function<void (StatisticsResult &, const StatisticsResult &)> reducedFunction =
        [] (StatisticsResult &result, const StatisticsResult &partial)
    {
        if (result.failedFile.isEmpty() && partial.failedFile.isEmpty()) {
            for (auto iter = partial.statistics.begin(); iter != partial.statistics.end(); ++iter) {
                const QCPDataMap &srcData = iter.value();
                QCPDataMap &destData = result.statistics[iter.key()];
                for (auto dataIter = srcData.begin(); dataIter != srcData.end(); ++dataIter) {
                    destData.insert(dataIter.key(), dataIter.value());
                }
            }
        } else {
            if (result.failedFile.isEmpty()) {
                for (const QString &file : partial.failedFile) {
                    result.failedFile << file;
                }
            }
        }
    };

    QProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files..."));
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFutureWatcher<StatisticsResult> watcher;
    connect(&watcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    connect(&watcher, SIGNAL(progressRangeChanged(int,int)), &dialog, SLOT(setRange(int,int)));
    connect(&watcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));
    connect(&dialog, &QProgressDialog::canceled, [&working, &watcher] () {watcher.cancel();working = false;});

    QVector<QString> checkedFiles;
    for (QListWidgetItem *item : checkedItems) {
        checkedFiles << item->toolTip();
    }

    watcher.setFuture(QtConcurrent::mappedReduced<StatisticsResult>(checkedFiles, mappedFunction, reducedFunction));
    dialog.exec();
    watcher.waitForFinished();

    if (watcher.isCanceled()) {
        return;
    }

    handleStatisticsResult(watcher.result(), multipleWindows);
}

void MainWindow::showInfoMsgBox(const QString &text, const QString &info)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("Information"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    msgBox.exec();
}

void MainWindow::showErrorMsgBox(const QString &text, const QString &info)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("Error"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    msgBox.exec();
}

void MainWindow::appendLogInfo(const QString &text)
{
    _ui->logTextEdit->appendHtml(QString("<font color='green'>INFO: %1</font>").arg(text));
}

void MainWindow::appendLogWarn(const QString &text)
{
    _ui->logTextEdit->appendHtml(QString("<font color='#CC9900'>WARN: %1</font>").arg(text));
}

void MainWindow::appendLogError(const QString &text)
{
    _ui->logTextEdit->appendHtml(QString("<font color='red'>ERR: %1</font>").arg(text));
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
        int preCount = _ui->lwStatisticsFiles->count();
        int added = addStatisticsFiles(fileNames);
        if (preCount == 0 && added > 0) {
            parseStatisticsFileHeader();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    emit aboutToBeClosed();
}

void MainWindow::updateFilterPattern()
{
    static_cast<StatisticsNameModel*>(_ui->lvStatisticsName->model())->setFilterPattern(
                _ui->cbRegExpFilter->lineEdit()->text());
}

void MainWindow::handleStatisticsResult(const StatisticsResult &result, bool multipleWindows)
{
    if (result.failedFile.isEmpty()) {
        if (multipleWindows) {
            for (auto iter = result.statistics.begin(); iter != result.statistics.end(); ++iter) {
                QMap<QString, QCPDataMap> tmpResult;
                tmpResult.insert(iter.key(), iter.value());

                PlotWindow *w = new PlotWindow(getStatisticsFileNode(), tmpResult);
                w->setAttribute(Qt::WA_DeleteOnClose);
                connect(this, SIGNAL(aboutToBeClosed()), w, SLOT(close()));
                w->showMaximized();
            }
        } else {
            PlotWindow *w = new PlotWindow(getStatisticsFileNode(), result.statistics);
            w->setAttribute(Qt::WA_DeleteOnClose);
            connect(this, SIGNAL(aboutToBeClosed()), w, SLOT(close()));
            w->showMaximized();
        }
    } else {
        for (const QString &failedFile : result.failedFile) {
            appendLogError(QString("Parse %1 failed.").arg(failedFile));
        }
    }
}

void MainWindow::contextMenuRequest(const QPoint &pos)
{
    QMenu *menu = _ui->logTextEdit->createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Clear"), this, SLOT(clearLogEdit()));

    menu->popup(_ui->logTextEdit->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    _ui->logTextEdit->clear();
}

void MainWindow::handleTimeDurationResult(int index)
{
    QFutureWatcher<TimeDurationResult> *watcher = static_cast<QFutureWatcher<TimeDurationResult>*>(sender());
    TimeDurationResult result = watcher->resultAt(index);
    QFileInfo fileInfo(result.value(KEY_FILE_NAME));
    if (result.size() > 1) {
        QList<QListWidgetItem*> items = _ui->lwStatisticsFiles->findItems(fileInfo.fileName(),
            Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if (items.size() > 0) {
            items.at(0)->setStatusTip(QString("Time duration: %1 - %2").arg(result.value(KEY_START_TIME)).arg(result.value(KEY_END_TIME)));
        }
    } else {
        appendLogError(QString("Parse file %1 failed.").arg(fileInfo.fileName()));
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter(QStringLiteral("Internal Statistics File (*.csv.gz)"));

    if (fileDialog.exec() == QDialog::Accepted) {
        int preCount = _ui->lwStatisticsFiles->count();
        int added = addStatisticsFiles(fileDialog.selectedFiles());
        if (preCount == 0 && added > 0) {
            parseStatisticsFileHeader();
        }
    }
}

void MainWindow::on_actionCloseAll_triggered()
{
    static_cast<StatisticsNameModel*>(_ui->lvStatisticsName->model())->clearStatisticsNames();
    for (int i = _ui->lwStatisticsFiles->count() - 1; i >= 0; --i) {
        delete _ui->lwStatisticsFiles->item(i);
    }
}

void MainWindow::on_actionDrawPlot_triggered()
{
    parseStatisticsFileData(false);
}

void MainWindow::on_actionDrawPlotInMultipleWindows_triggered()
{
    parseStatisticsFileData(true);
}

void MainWindow::on_actionSelectAll_triggered()
{
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        if (item->checkState() != Qt::Checked) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_actionClearSelection_triggered()
{
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        if (item->checkState() != Qt::Unchecked) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::on_actionInvertSelection_triggered()
{
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        switch (item->checkState()) {
        case Qt::Checked:
            item->setCheckState(Qt::Unchecked);
            break;
        case Qt::Unchecked:
            item->setCheckState(Qt::Checked);
            break;
        default:
            break;
        }
    }
}
