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

#define STAT_FILE_PATTERN "^([A-Z]+\\d+\\-\\d+)__(int|ext)stat_(\\d{8}\\-\\d{6}|archive)\\.csv\\.gz$"

enum {
    KEY_FILE_NAME,
    KEY_START_TIME,
    KEY_END_TIME,
};

void ProgressBar::increaseValue(int value)
{
    setValue(this->value() + value);
}

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
    connect(_ui->lvStatisticsName, &QListView::doubleClicked, this, &MainWindow::listViewDoubleClicked);

    connect(_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(_ui->lvStatisticsName, &QListView::customContextMenuRequested, this, &MainWindow::listViewContextMenuRequest);
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

QVector<QString> MainWindow::addStatisticsFiles(const QStringList &fileNames)
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
            appendLogWarn(QStringLiteral("%1 ignored. Please add only one node's statistics files!").arg(fileInfo.fileName()));
            continue;
        }
        if (!checkStatisticsFileType(regExp.cap(2))) {
            appendLogWarn(QStringLiteral("%1 ignored. Please add only one type of statistics files!").arg(fileInfo.fileName()));
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
        item->setCheckState(Qt::Checked);
        item->setToolTip(nativeName);
        _ui->lwStatisticsFiles->addItem(item);
        addedFileNames << nativeName;
    }

    return addedFileNames;
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

bool MainWindow::checkStatisticsFileType(const QString &type)
{
    if (_ui->lwStatisticsFiles->count() == 0) {
        return true;
    }

    QListWidgetItem *item = _ui->lwStatisticsFiles->item(0);
    QRegExp regExp(STAT_FILE_PATTERN);
    regExp.exactMatch(item->text());
    return regExp.cap(2) == type;
}

void MainWindow::parseStatisticsFileHeader(const QVector<QString> &fileNames, bool updateModel)
{
    QProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files' header..."));
    dialog.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    dialog.setCancelButton(NULL);
    dialog.setRange(0, fileNames.size());

    QFuture<std::string> future = QtConcurrent::run([&fileNames, &dialog] () {
        std::string result;
        GZipFile gzFile(fileNames.at(0));
        int progress = 0;
        if (gzFile.readLine(result)) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
            if (strncmp(result.c_str(), "##date;time;", 12) == 0 &&
                strcmp(result.c_str() + result.length() - 2, "##") == 0)
            {
                for (int i = 1; i < fileNames.size(); ++i) {
                    std::string header;
                    GZipFile tmpGzFile(fileNames.at(i));
                    if (tmpGzFile.readLine(header)) {
                        QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
                        if (header != result) {
                            result = (QFileInfo(fileNames.at(0)).fileName() + '\n' +
                                      QFileInfo(fileNames.at(i)).fileName() + QDir::separator()).toStdString();
                            break;
                        }
                    } else {
                        result = QFileInfo(fileNames.at(i)).fileName().toStdString();
                    }
                }
            } else {
                // Append a flag to indicate that the header in file is invalid
                result = (QFileInfo(fileNames.at(0)).fileName() + QDir::separator()).toStdString();
            }
        } else {
            result = QFileInfo(fileNames.at(0)).fileName().toStdString();
        }
        QMetaObject::invokeMethod(&dialog, "reset", Qt::QueuedConnection);
        return result;
    });

    dialog.exec();
    future.waitForFinished();

    std::string result = future.result();
    if (result.front() == '#') {
        if (updateModel) {
            StatisticsNameModel *model = static_cast<StatisticsNameModel*>(_ui->lvStatisticsName->model());
            model->beforeDataContainerUpdate();
            std::vector<std::string> &container = model->getDataContainer();
            container.resize(0);
            splitString(result.c_str() + sizeof("##date;time;") - 1, ';', container);
            container.back().erase(container.back().length() - 2);
            model->endDataContainerUpdate();

            QString filterText = _ui->cbRegExpFilter->lineEdit()->text();
            if (!filterText.isEmpty()) {
                model->setFilterPattern(filterText);
            }
        }
    } else if (result.back() == QDir::separator().toLatin1()) {
        result.erase(result.end() - 1);
        showErrorMsgBox(QStringLiteral("Invalid statistics file header format. The reason may be the header itself is invalid or two files' header aren't the same."),
                        QString::fromStdString(result));
    } else {
        showErrorMsgBox(QStringLiteral("Read statistics file header failed."),
                        QString::fromStdString(result));
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

    if (_ui->lvStatisticsName->selectionModel()->selectedIndexes().size() > PlotWindow::predefinedColorCount() ||
        (model->rowCount() > PlotWindow::predefinedColorCount() && !_ui->lvStatisticsName->selectionModel()->hasSelection()))
    {
        showInfoMsgBox(QStringLiteral("Too many statistics names specified, please change your filter text."),
                       QStringLiteral("At most %1 statistics names allowed at one time.").arg(PlotWindow::predefinedColorCount()));
        return;
    }

    QMap<int, QString> indexNameMap;
    if (_ui->lvStatisticsName->selectionModel()->hasSelection()) {
        QModelIndexList list = _ui->lvStatisticsName->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : list) {
            indexNameMap.insert(model->data(index, Qt::UserRole).toInt(),
                                model->data(index).toString());
        }
    } else {
        for (int i = 0; i < model->rowCount(); ++i) {
            QModelIndex modelIndex = model->index(i);
            indexNameMap.insert(model->data(modelIndex, Qt::UserRole).toInt(),
                                model->data(modelIndex).toString());
        }
    }

    volatile bool working = true;
    ProgressBar *customBar = new ProgressBar();
    std::function<StatisticsResult (const QString &)> mappedFunction = [indexNameMap, customBar, &working] (const QString &fileName) {
        StatisticsResult result;
        std::string line;
        QCPData data;
        int preCompletionRate = 0;
        GZipFile gzFile(fileName);
        QList<int> indexes = indexNameMap.keys();
        // Read the header
        if (!gzFile.readLine(line)) {
            result.failedFile << QFileInfo(fileName).fileName();
            goto end;
        }
        while (working && gzFile.readLine(line)) {
            QDateTime dt = QDateTime::fromString(QString::fromLatin1(line.c_str(), 19), "dd.MM.yyyy;HH:mm:ss");
            if (dt.isValid()) {
                data.key = dt.toTime_t();
                int index = 0;
                int parsedStatCount = 0;
                const char *cstr = line.c_str();
                const char *ptr;
                while ((ptr = strchr(cstr, ';')) != NULL) {
                    int tmpIndex = indexes.at(parsedStatCount);
                    if (index == tmpIndex) {
                        data.value = atoi(cstr);
                        result.statistics[indexNameMap.value(tmpIndex)].insert(data.key, data);
                        if (++parsedStatCount == indexes.size()) {
                            break;
                        }
                    }
                    cstr = ptr + 1;
                    ++index;
                }
                // Last occurence
                if (parsedStatCount < indexes.size() && *cstr) {
                    int tmpIndex = indexes.at(parsedStatCount);
                    if (index == tmpIndex) {
                        data.value = atoi(cstr);
                        result.statistics[indexNameMap.value(tmpIndex)].insert(data.key, data);
                        ++parsedStatCount;
                    }
                }
                if (parsedStatCount != indexes.size()) {
                    result.failedFile << QFileInfo(fileName).fileName();
                    goto end;
                }
                int completionRate = gzFile.completionRate();
                if (completionRate > preCompletionRate) {
                    QMetaObject::invokeMethod(customBar, "increaseValue", Qt::QueuedConnection,
                                              Q_ARG(int, completionRate - preCompletionRate));
                    preCompletionRate = completionRate;
                }
            } else {
                result.failedFile << QFileInfo(fileName).fileName();
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
            for (const QString &file : partial.failedFile) {
                result.failedFile << file;
            }
        }
    };

    QProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files..."));
    dialog.setBar(customBar); // dialog takes ownership of customBar
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // We don't use wathcer to monitor progress because it base on item count in the container, this
    // is not accurate. Instead, we calculate the progress ourselves
    QFutureWatcher<StatisticsResult> watcher;
    connect(&watcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    connect(&dialog, &QProgressDialog::canceled, [&working, &watcher] () {watcher.cancel();working = false;});

    QVector<QString> checkedFiles;
    for (QListWidgetItem *item : checkedItems) {
        checkedFiles << item->toolTip();
    }

    dialog.setRange(0, checkedFiles.size() * 100);
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
    _ui->logTextEdit->appendHtml(QStringLiteral("<font color='green'>INFO: %1</font>").arg(text));
}

void MainWindow::appendLogWarn(const QString &text)
{
    _ui->logTextEdit->appendHtml(QStringLiteral("<font color='#CC9900'>WARN: %1</font>").arg(text));
}

void MainWindow::appendLogError(const QString &text)
{
    _ui->logTextEdit->appendHtml(QStringLiteral("<font color='red'>ERR: %1</font>").arg(text));
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
        int beforeAdd = _ui->lwStatisticsFiles->count();
        QVector<QString> addedFiles = addStatisticsFiles(fileNames);
        if (addedFiles.size() > 0) {
            parseStatisticsFileHeader(addedFiles, beforeAdd == 0);
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

void MainWindow::listViewDoubleClicked(const QModelIndex &index)
{
    QString text = _ui->lvStatisticsName->model()->data(index).toString();
    _ui->cbRegExpFilter->lineEdit()->setText(QRegExp::escape(text));
    updateFilterPattern();
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
            appendLogError(QStringLiteral("Parse fle %1 failed.").arg(failedFile));
        }
    }
}

void MainWindow::logEditContextMenuRequest(const QPoint &pos)
{
    QMenu *menu = _ui->logTextEdit->createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Clear"), this, SLOT(clearLogEdit()));

    menu->popup(_ui->logTextEdit->mapToGlobal(pos));
}

void MainWindow::listViewContextMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(QStringLiteral("Copy"), this, SLOT(copyStatisticsNames()));
    menu->addAction(QStringLiteral("Clear selection"), _ui->lvStatisticsName->selectionModel(), SLOT(clearSelection()));

    menu->popup(_ui->lvStatisticsName->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    _ui->logTextEdit->clear();
}

void MainWindow::handleTimeDurationResult(int index)
{
    QFutureWatcher<QString> *watcher = static_cast<QFutureWatcher<QString>*>(sender());
    if (!watcher->isCanceled()) {
        QString result = watcher->resultAt(index);
        QStringList stringList = result.split(QDir::separator());
        Q_ASSERT(stringList.size() == 2);
        QList<QListWidgetItem*> items = _ui->lwStatisticsFiles->findItems(stringList.at(1),
            Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if (items.size() > 0) {
            items.at(0)->setStatusTip(stringList.at(0));
        }
        if (stringList.at(0).contains(QStringLiteral("parse failed!"))) {
            appendLogError(QStringLiteral("Parse file %1's time duration failed.").arg(stringList.at(1)));
        }
    }
}

void MainWindow::copyStatisticsNames()
{
    QModelIndexList indexList = _ui->lvStatisticsName->selectionModel()->selectedIndexes();
    if (indexList.size() > 0) {
        QStringList stringList;
        for (const QModelIndex &index : indexList) {
            stringList << _ui->lvStatisticsName->model()->data(index).toString();
        }
        QApplication::clipboard()->setText(stringList.join(QStringLiteral("\n")));
    }
}

void MainWindow::on_actionAdd_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter(QStringLiteral("Statistics File (*.csv.gz)"));

    if (fileDialog.exec() == QDialog::Accepted) {
        int beforeAdd = _ui->lwStatisticsFiles->count();
        QVector<QString> addedFiles = addStatisticsFiles(fileDialog.selectedFiles());
        if (addedFiles.size() > 0) {
            parseStatisticsFileHeader(addedFiles, beforeAdd == 0);
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

QDataStream& operator>> (QDataStream &in, QCPData &data)
{
    in >> data.key >> data.value;
    return in;
}

void MainWindow::on_actionViewHelp_triggered()
{
}

void MainWindow::on_actionCalculateTimeDuration_triggered()
{
    QVector<QString> filesToCalculate;
    for (int i = 0; i < _ui->lwStatisticsFiles->count(); ++i) {
        QListWidgetItem *item = _ui->lwStatisticsFiles->item(i);
        if (item->statusTip().isEmpty()) {
            filesToCalculate << item->toolTip();
        }
    }

    if (filesToCalculate.size() > 0) {
        volatile bool working = true;
        ProgressBar *customBar = new ProgressBar();

        std::function<QString (const QString &)> mappedFunction = [&working, customBar] (const QString &fileName) -> QString {
            const int TIME_STR_LEN = 19;

            GZipFile gzFile(fileName);
            QRegExp regExp(QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4};\\d{2}:\\d{2}:\\d{2};"));
            std::string startLine, endLine;
            if (!gzFile.readLine(startLine)) {
                return QStringLiteral("Time duration: parse failed!%1%2").
                        arg(QDir::separator()).arg(QFileInfo(fileName).fileName());
            }

            // Ignore the first line because it is header
            // Read the second line
            if (!gzFile.readLine(startLine)) {
                return QStringLiteral("Time duration: parse failed!%1%2").
                        arg(QDir::separator()).arg(QFileInfo(fileName).fileName());
            }

            // Check format
            if (regExp.indexIn(QString::fromStdString(startLine)) != 0) {
                return QStringLiteral("Time duration: parse failed!%1%2").
                        arg(QDir::separator()).arg(QFileInfo(fileName).fileName());
            }

            // Get the last line
            int preCompletionRate = 0;
            while (working && gzFile.readLine(endLine)) {
                int completionRate = gzFile.completionRate();
                if (completionRate > preCompletionRate) {
                    QMetaObject::invokeMethod(customBar, "increaseValue", Qt::QueuedConnection,
                                              Q_ARG(int, completionRate - preCompletionRate));
                    preCompletionRate = completionRate;
                }
            }

            // Check format
            if (regExp.indexIn(QString::fromStdString(endLine)) != 0) {
                return QStringLiteral("Time duration: parse failed!%1%2").
                        arg(QDir::separator()).arg(QFileInfo(fileName).fileName());
            }

            return QStringLiteral("Time duration: %1 - %2%3%4").
                    arg(QString::fromStdString(startLine).left(TIME_STR_LEN).replace(';', ' ')).
                    arg(QString::fromStdString(endLine).left(TIME_STR_LEN).replace(';', ' ')).
                    arg(QDir::separator()).
                    arg(QFileInfo(fileName).fileName());
        };

        QProgressDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Please Wait"));
        dialog.setLabelText(QStringLiteral("Parsing statistics files' time duration..."));
        dialog.setBar(customBar); // dialog takes ownership of customBar
        dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dialog.setRange(0, filesToCalculate.size() * 100);

        // We don't use wathcer to monitor progress because it base on item count in the container, this
        // is not accurate. Instead, we calculate the progress ourselves
        QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
        connect(watcher, SIGNAL(resultReadyAt(int)), SLOT(handleTimeDurationResult(int)));
        connect(watcher, SIGNAL(finished()), &dialog, SLOT(reset()));
        connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
        connect(&dialog, &QProgressDialog::canceled, [&working, watcher] () {watcher->cancel();working = false;});

        watcher->setFuture(QtConcurrent::mapped(filesToCalculate, mappedFunction));

        dialog.exec();
        watcher->waitForFinished();
        if (!watcher->isCanceled()) {
            showInfoMsgBox(QStringLiteral("Calculate statistics files' time duration finished."),
                           QStringLiteral("The time duration will be shown in status bar when you hover mouse on file name."));
        }
    }
}
