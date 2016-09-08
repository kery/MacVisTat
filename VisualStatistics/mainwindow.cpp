#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include "statisticsnamemodel.h"
#include "utils.h"
#include "version.h"
#include "aboutdialog.h"
#include "progressdialog.h"
#include "statisticsfileparser.h"
#include "gzipfile.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QtConcurrent>
#include <functional>
#include <utility>

#define STAT_FILE_PATTERN QStringLiteral("^(.+)__.+\\.csv\\.gz$")

MainWindow::MainWindow() :
    QMainWindow(nullptr),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    // used to disable tooltip for tool button
    installEventFilterForAllToolButton();

    m_ui->splitterHor->setSizes(QList<int>() << 280 << width() - 280);
    m_ui->splitterHor->setStretchFactor(0, 0);
    m_ui->splitterHor->setStretchFactor(1, 1);
    m_ui->splitteVer->setSizes(QList<int>() << height() - 80 << 80);
    m_ui->splitteVer->setStretchFactor(0, 1);
    m_ui->splitteVer->setStretchFactor(1, 0);

    m_ui->lvStatName->setModel(new StatisticsNameModel(this));

    m_lbStatNameInfo = new QLabel(this);
    m_lbStatNameInfo->setStyleSheet(QStringLiteral("QLabel{color:#888888}"));
    m_ui->statusBar->addPermanentWidget(m_lbStatNameInfo);
    updateStatNameInfo();
    connect(m_ui->lvStatName->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateStatNameInfo()));
    connect(m_ui->lvStatName->model(), SIGNAL(modelReset()), this, SLOT(updateStatNameInfo()));

    m_ui->cbRegExpFilter->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_ui->cbRegExpFilter->lineEdit()->setPlaceholderText(QStringLiteral("regular expression filter"));
    connect(m_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::updateFilterPattern);
    connect(m_ui->lvStatName, &QListView::doubleClicked, this, &MainWindow::listViewDoubleClicked);

    connect(m_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(m_ui->lvStatName, &QListView::customContextMenuRequested, this, &MainWindow::listViewContextMenuRequest);

#if defined(Q_OS_WIN)
    startCheckNewVersionTask();
#endif
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::startCheckNewVersionTask()
{
#if defined(Q_OS_WIN)
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>();
    connect(watcher, SIGNAL(finished()), SLOT(checkNewVersionTaskFinished()));
    watcher->setFuture(QtConcurrent::run([] {
        QDir dir(QStringLiteral("\\\\cdvasfile.china.nsn-net.net\\data\\sdu\\Tools\\VisualStatistics\\win"));
        QStringList nameFilter(QStringLiteral("*.exe"));
        QFileInfoList infoList = dir.entryInfoList(nameFilter, QDir::Files);
#if defined(Q_OS_WIN64)
        QRegExp regExp(QStringLiteral("VisualStatisticsSetup_win64_v(\\d+\\.\\d+\\.\\d+\\.\\d+)\\.exe"));
#else
        QRegExp regExp(QStringLiteral("VisualStatisticsSetup_win32_v(\\d+\\.\\d+\\.\\d+\\.\\d+)\\.exe"));
#endif
        for (const QFileInfo &fileInfo : infoList) {
            QString file = fileInfo.fileName();
            if (regExp.exactMatch(file)) {
                int verNum = versionStringToNumber(regExp.cap(1));
                if (verNum > VER_FILEVERSION_NUM) {
                    return file;
                }
            }
        }
        return QString();
    }));
#endif
}

void MainWindow::installEventFilterForAllToolButton()
{
    for (QObject *btn : m_ui->mainToolBar->findChildren<QObject*>()) {
        btn->installEventFilter(this);
    }
}

bool MainWindow::isToolTipEventOfToolButton(QObject *obj, QEvent *event)
{
    return event->type() == QEvent::ToolTip && obj->parent() == m_ui->mainToolBar;
}

bool MainWindow::statFileAlreadyAdded(const QString &filePath)
{
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
#if defined(Q_OS_WIN)
        Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
        Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
        if (item->toolTip().compare(filePath, cs) == 0) {
            return true;
        }
    }
    return false;
}

void MainWindow::addStatFiles(QStringList &filePaths)
{
    translateToLocalPath(filePaths);
    filterOutAlreadyAddedFiles(filePaths);
    QStringList invalidFileNames = filterOutInvalidFileNames(filePaths);
    for (const QString &fileName : invalidFileNames) {
        appendLogWarn(fileName + " ignored. Invalid file name.");
    }

    if (filePaths.isEmpty()) {
        return;
    }

    QStringList failInfo;
    if (m_ui->lwStatFiles->count() == 0) {
        parseStatFileHeader(filePaths, failInfo);
    } else {
        // Prepend a file to be compared to its header
        filePaths.prepend(m_ui->lwStatFiles->item(0)->toolTip());
        checkStatFileHeader(filePaths, failInfo);
        // Remove the prepended file
        filePaths.erase(filePaths.begin());
    }

    addStatFilesToListWidget(filePaths);

    for (const QString &info : failInfo) {
        appendLogError(info);
    }
}

void MainWindow::filterOutAlreadyAddedFiles(QStringList &filePaths)
{
    auto newEnd = std::remove_if(filePaths.begin(), filePaths.end(),
                                 std::bind(&MainWindow::statFileAlreadyAdded,
                                           this, std::placeholders::_1));
    filePaths.erase(newEnd, filePaths.end());
}

QStringList MainWindow::filterOutInvalidFileNames(QStringList &filePaths)
{
    QStringList invalidFileNames;
    auto newEnd = std::remove_if(filePaths.begin(), filePaths.end(),
                                 [&invalidFileNames] (const QString &filePath)
    {
        QRegExp regExp(STAT_FILE_PATTERN);
        const QString fileName = QFileInfo(filePath).fileName();
        if (regExp.exactMatch(fileName)) {
            return false;
        } else {
            invalidFileNames << fileName;
            return true;
        }
    });
    filePaths.erase(newEnd, filePaths.end());
    return invalidFileNames;
}

void MainWindow::parseStatFileHeader(QStringList &filePaths, QStringList &failInfo)
{
    ProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files' header..."));
    dialog.enableCancelButton(false);

    StatisticsFileParser fileParser(dialog);
    std::string result = fileParser.parseFileHeader(filePaths, failInfo);
    if (result.size() > 0) {
        StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
        StatisticsNameModel::StatisticsNames statNames;
        splitString(result.c_str() + sizeof("##date;time;") - 1, ';', statNames);
        statNames.back().erase(statNames.back().length() - 2);
        model->setStatisticsNames(statNames);

        QString filterText = m_ui->cbRegExpFilter->lineEdit()->text();
        if (!filterText.isEmpty()) {
            QStringList errList;
            model->setFilterPattern(filterText, errList);
            for (const QString &err : errList) {
                appendLogError(err);
            }
        }
    }
}

void MainWindow::checkStatFileHeader(QStringList &filePaths, QStringList &failInfo)
{
    ProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files' header..."));
    dialog.enableCancelButton(false);

    StatisticsFileParser fileParser(dialog);
    fileParser.checkFileHeader(filePaths, failInfo);
}

void MainWindow::addStatFilesToListWidget(const QStringList &filePaths)
{
    QIcon icon(QStringLiteral(":/resource/image/archive.png"));
    for (const QString &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
        item->setCheckState(Qt::Checked);

        item->setToolTip(filePath);
        m_ui->lwStatFiles->addItem(item);
    }
}

void MainWindow::translateToLocalPath(QStringList &filePaths)
{
    std::for_each(filePaths.begin(), filePaths.end(), [] (QString &filePath) {
        filePath = QDir::toNativeSeparators(filePath);
    });
}

void MainWindow::parseStatFileData(bool multipleWindows)
{
    QVector<QListWidgetItem*> checkedItems;
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedItems << item;
        }
    }

    if (checkedItems.isEmpty()) {
        showInfoMsgBox(this,
                       QStringLiteral("Please check the statistics file(s) with which you want to draw plot!"),
                       QStringLiteral("No statistics file checked."));
        return;
    }

    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    Q_ASSERT(model != NULL);
    if (model->rowCount() == 0) {
        showInfoMsgBox(this,
                       QStringLiteral("Please specify at lease one statistics name which you want to draw!"),
                       QStringLiteral("No statistics name found."));
        return;
    }

    int countToBeDrawn;
    QModelIndexList selectedIndexes = m_ui->lvStatName->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        countToBeDrawn = model->rowCount();
    } else {
        countToBeDrawn = selectedIndexes.size();
    }

    const int MAX_COUNT_TO_BE_DRAWN = 256;
    if (countToBeDrawn > MAX_COUNT_TO_BE_DRAWN)
    {
        showInfoMsgBox(this,
                       QStringLiteral("Too many statistics names specified, please change your filter text."),
                       QStringLiteral("At most %1 statistics names are allowed.").arg(MAX_COUNT_TO_BE_DRAWN));
        return;
    }

    if (multipleWindows && countToBeDrawn > 8) {
        int answer = showQuestionMsgBox(this,
                                        QStringLiteral("You clicked [%1]. There are %2 windows will be created. Do you want to continue?").
                                        arg(m_ui->actionDrawPlotInMultipleWindows->text()).arg(countToBeDrawn));
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    StatisticsFileParser::IndexNameMap indexNameMap;
    if (m_ui->lvStatName->selectionModel()->hasSelection()) {
        for (const QModelIndex &index : selectedIndexes) {
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

    ProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));
    dialog.setLabelText(QStringLiteral("Parsing statistics files..."));

    QVector<QString> checkedFiles;
    for (QListWidgetItem *item : checkedItems) {
        checkedFiles << item->toolTip();
    }

    StatisticsFileParser fileParser(dialog);
    StatisticsFileParser::Result result;
    if (fileParser.parseFileData(indexNameMap, checkedFiles, result)) {
        for (const QString &failedFile : result.failedFiles) {
            appendLogError(QStringLiteral("Parse file %1 failed.").arg(failedFile));
        }
        handleParsedStat(result.nndm, multipleWindows);
    }
}

void MainWindow::handleParsedStat(Statistics::NodeNameDataMap &nndm, bool multipleWindows)
{
    if (multipleWindows) {
        QMap<QString, Statistics::NodeNameDataMap> nndms(
                    Statistics::groupNodeNameDataMapByName(std::move(nndm)));
        for (Statistics::NodeNameDataMap &nndm : nndms) {
            Statistics stat(nndm);
            PlotWindow *plotWindow = new PlotWindow(stat);
            plotWindow->setAttribute(Qt::WA_DeleteOnClose);
            connect(this, SIGNAL(aboutToBeClosed()), plotWindow, SLOT(close()));
            plotWindow->showMaximized();
        }
    } else {
        Statistics stat(nndm);
        PlotWindow *plotWindow = new PlotWindow(stat);
        plotWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(this, SIGNAL(aboutToBeClosed()), plotWindow, SLOT(close()));
        plotWindow->showMaximized();
    }
}

void MainWindow::appendLogInfo(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='green'>INFO: %1</font>").arg(text));
}

void MainWindow::appendLogWarn(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#CC9900'>WARN: %1</font>").arg(text));
}

void MainWindow::appendLogError(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='red'>ERR: %1</font>").arg(text));
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
        addStatFiles(fileNames);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    emit aboutToBeClosed();
}

void MainWindow::checkNewVersionTaskFinished()
{
#if defined(Q_OS_WIN)
    QFutureWatcher<QString> *watcher = static_cast<QFutureWatcher<QString>*>(sender());
    QString newVersion = watcher->result();
    delete watcher;
    if (!newVersion.isEmpty()) {
        int answer = showQuestionMsgBox(this,
                                        QStringLiteral("A new version of this program has been found. Do you want to get it?"),
                                        newVersion);
        if (answer == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QStringLiteral("\\\\cdvasfile.china.nsn-net.net\\data\\sdu\\Tools")));
            QApplication::exit();
        }
    }
#endif
}

void MainWindow::updateFilterPattern()
{
    QStringList errList;
    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->setFilterPattern(
                m_ui->cbRegExpFilter->lineEdit()->text(), errList);
    for (const QString &err : errList) {
        appendLogError(err);
    }
}

void MainWindow::listViewDoubleClicked(const QModelIndex &index)
{
    QString text = m_ui->lvStatName->model()->data(index).toString();
    m_ui->cbRegExpFilter->lineEdit()->setText(QRegExp::escape(text));
    updateFilterPattern();
}

void MainWindow::logEditContextMenuRequest(const QPoint &pos)
{
    QMenu *menu = m_ui->logTextEdit->createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Clear"), this, SLOT(clearLogEdit()));

    menu->popup(m_ui->logTextEdit->mapToGlobal(pos));
}

void MainWindow::listViewContextMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(QStringLiteral("Copy"), this, SLOT(copyStatisticsNames()));
    menu->addAction(QStringLiteral("Clear selection"), m_ui->lvStatName->selectionModel(), SLOT(clearSelection()));

    menu->popup(m_ui->lvStatName->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    m_ui->logTextEdit->clear();
}

void MainWindow::handleTimeDurationResult(int index)
{
    QFutureWatcher<QString> *watcher = static_cast<QFutureWatcher<QString>*>(sender());
    if (!watcher->isCanceled()) {
        QString result = watcher->resultAt(index);
        QStringList stringList = result.split("##");
        Q_ASSERT(stringList.size() == 2);
        QList<QListWidgetItem*> items = m_ui->lwStatFiles->findItems(
                    QFileInfo(stringList.at(0)).fileName(),
                    Qt::MatchFixedString);
        if (items.size() > 0) {
            items.at(0)->setStatusTip(stringList.at(1));
        }
    }
}

void MainWindow::copyStatisticsNames()
{
    QModelIndexList indexList = m_ui->lvStatName->selectionModel()->selectedIndexes();
    if (indexList.size() > 0) {
        QStringList stringList;
        for (const QModelIndex &index : indexList) {
            stringList << m_ui->lvStatName->model()->data(index).toString();
        }
        QApplication::clipboard()->setText(stringList.join(QStringLiteral("\n")));
    }
}

void MainWindow::updateStatNameInfo()
{
    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    int displayed = model->rowCount();
    int filtered = model->filteredCount();
    int total = model->totalCount();

    m_lbStatNameInfo->setText(QStringLiteral("%1, %2, %3").arg(displayed).arg(filtered).arg(total));
}

void MainWindow::on_actionAdd_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter(QStringLiteral("Statistics File (*.csv.gz)"));

    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList strList = fileDialog.selectedFiles();
        addStatFiles(strList);
    }
}

void MainWindow::on_actionCloseAll_triggered()
{
    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->clearStatisticsNames();
    for (int i = m_ui->lwStatFiles->count() - 1; i >= 0; --i) {
        delete m_ui->lwStatFiles->item(i);
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
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
        if (item->checkState() != Qt::Checked) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_actionClearSelection_triggered()
{
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
        if (item->checkState() != Qt::Unchecked) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::on_actionInvertSelection_triggered()
{
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
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

void MainWindow::on_actionViewHelp_triggered()
{
    QUrl url(QStringLiteral("http://viini.dev.cic.nsn-rdnet.net/twiki/bin/view/SA/VisualStatistics"));
    QDesktopServices::openUrl(url);
}

void MainWindow::on_actionCalculateTimeDuration_triggered()
{
    QStringList filesToCalculate;
    for (int i = 0; i < m_ui->lwStatFiles->count(); ++i) {
        QListWidgetItem *item = m_ui->lwStatFiles->item(i);
        if (item->checkState() == Qt::Checked && item->statusTip().isEmpty()) {
            filesToCalculate << item->toolTip();
        }
    }

    if (filesToCalculate.size() > 0) {
        ProgressDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Please Wait"));
        dialog.setLabelText(QStringLiteral("Parsing statistics files' time duration..."));

        StatisticsFileParser fileParser(dialog);
        fileParser.parseTimeDuration(filesToCalculate, this, SLOT(handleTimeDurationResult(int)));
    }
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setLabelText(QStringLiteral("Visual Statistics v%1").arg(VER_FILEVERSION_STR));
    dialog.exec();
}
