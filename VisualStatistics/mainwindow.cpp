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
#include "changelogdialog.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QHostInfo>
#include <QSysInfo>
#include <QNetworkAccessManager>
#include <QNetworkProxyQuery>

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
    connect(m_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::adjustFilterHistoryOrder);
    connect(m_ui->lvStatName, &QListView::doubleClicked, this, &MainWindow::listViewDoubleClicked);

    connect(m_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(m_ui->lvStatName, &QListView::customContextMenuRequested, this, &MainWindow::listViewContextMenuRequest);

#ifdef INSTALLER
    startCheckNewVersionTask();
    startUserReportTask();
#endif

    loadFilterHistory();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::startCheckNewVersionTask()
{
    QString maintenanceToolPath = getMaintenanceToolPath();
    if (maintenanceToolPath.isEmpty()) {
        return;
    }
    QProcess *process = new QProcess();
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(checkNewVersionTaskFinished(int,QProcess::ExitStatus)));
    process->start(maintenanceToolPath, QStringList() << "--checkupdates" << "--proxy");
}

void MainWindow::startUserReportTask()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QUrl url("http://sdu.int.nokia-sbell.com:4099/report");
    QNetworkProxyQuery npq(url);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (proxies.size() > 0) {
        manager->setProxy(proxies[0]);
    }
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(userReportTaskFinished(QNetworkReply*)));

    QByteArray hostNameHash = QCryptographicHash::hash(QHostInfo::localHostName().toLatin1(),
                                                       QCryptographicHash::Md5);
    QString postData("host=");
    postData += hostNameHash.toHex();
    postData += QStringLiteral("&pt=");
    postData += QSysInfo::productType();
    postData += QStringLiteral("&ver=");
    postData += VER_FILEVERSION_STR;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, postData.toLatin1());
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
        appendLogWarn("ignored invalid file name " + fileName);
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

    m_ui->cbRegExpFilter->lineEdit()->setFocus();
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
                       QStringLiteral("Please check the statistics file(s) with which you want to plot!"),
                       QStringLiteral("No statistics file checked."));
        return;
    }

    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    Q_ASSERT(model != NULL);
    if (model->rowCount() == 0) {
        showInfoMsgBox(this,
                       QStringLiteral("Please specify at lease one statistics name which you want to plot!"),
                       QStringLiteral("No statistics name found."));
        return;
    }

    int statCountToPlot;
    QModelIndexList selectedIndexes = m_ui->lvStatName->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        statCountToPlot = model->rowCount();
    } else {
        statCountToPlot = selectedIndexes.size();
    }

    if (multipleWindows && statCountToPlot > 16) {
        int answer = showQuestionMsgBox(this,
                                        QStringLiteral("You clicked [%1]. There are %2 windows will be created. Do you want to continue?").
                                        arg(m_ui->actionPlotSeparately->text()).arg(statCountToPlot));
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
    Statistics::NodeNameDataMap nndm;
    QVector<QString> errors;
    if (fileParser.parseFileData(indexNameMap, checkedFiles, nndm, errors)) {
        for (const QString &error : errors) {
            appendLogError(error);
        }
        if (!nndm.isEmpty()) {
            handleParsedStat(nndm, multipleWindows);
        }
    }
}

bool MainWindow::allDataUnchanged(const Statistics::NodeNameDataMap &nndm)
{
    for (const Statistics::NameDataMap &ndm : nndm) {
        const QCPDataMap &dataMap = ndm.first();
        qint64 firstValue = (qint64)dataMap.first().value;
        for (const QCPData &data : dataMap) {
            if (firstValue != (qint64)data.value) {
                return false;
            }
        }
    }
    return true;
}

void MainWindow::handleParsedStat(Statistics::NodeNameDataMap &nndm, bool multipleWindows)
{
    if (multipleWindows) {
        QMap<QString, Statistics::NodeNameDataMap> nndms(
                    Statistics::groupNodeNameDataMapByName(std::move(nndm)));
        for (Statistics::NodeNameDataMap &nndm : nndms) {
            if ((QApplication::keyboardModifiers() & Qt::ControlModifier) && allDataUnchanged(nndm)) {
                continue;
            }
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

QString MainWindow::getMaintenanceToolPath()
{
    QDir appDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
    QString fileName = QStringLiteral("maintenancetool.exe");
#else
    QString fileName = QStringLiteral("maintenancetool");
#endif
    if (appDir.exists(fileName))
        return appDir.absoluteFilePath(fileName);
    return QString();
}

void MainWindow::appendLogInfo(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='green'>[%1] INFO: %2</font>").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendLogWarn(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#CC9900'>[%1] WARN: %2</font>").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendLogError(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='red'>[%1] ERR: %2</font>").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

QString MainWindow::filterHistoryFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_filter_hist"));
}

void MainWindow::loadFilterHistory()
{
    QFile histFile(filterHistoryFilePath());
    if (histFile.open(QIODevice::ReadOnly)) {
        QTextStream ts(&histFile);
        while (!ts.atEnd()) {
            QString line = ts.readLine();
            m_ui->cbRegExpFilter->addItem(line);
        }
        histFile.close();

        m_ui->cbRegExpFilter->setCurrentIndex(-1);
    }
}

void MainWindow::saveFilterHistory()
{
    const int MAX_FILTER_HISTORY = 100;

    QFile histFile(filterHistoryFilePath());
    if (histFile.open(QIODevice::WriteOnly)) {
        QTextStream ts(&histFile);
        for (int i = 0; i < MAX_FILTER_HISTORY && i < m_ui->cbRegExpFilter->count(); ++i) {
            QString text = m_ui->cbRegExpFilter->itemText(i);
            ts << text << endl;
        }
        histFile.close();
    }
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

    saveFilterHistory();
}

void MainWindow::checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    delete sender();

    // exitCode != 0 indicates that there is no update available
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        return;
    }

    ChangeLogDialog dlg(this);
    dlg.setShownAfterCheckingUpdates();
    dlg.exec();

    QString maintenanceToolPath = getMaintenanceToolPath();
    QProcess::startDetached(maintenanceToolPath, QStringList() << ("--updater") << "--proxy");
    QApplication::exit();
}

void MainWindow::userReportTaskFinished(QNetworkReply *reply)
{
    sender()->deleteLater();
    reply->deleteLater();
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

void MainWindow::adjustFilterHistoryOrder()
{
    QString currentText = m_ui->cbRegExpFilter->lineEdit()->text();
    int index = m_ui->cbRegExpFilter->findText(currentText);
    if (index > 0) {
        m_ui->cbRegExpFilter->removeItem(index);
        m_ui->cbRegExpFilter->insertItem(0, currentText);
        m_ui->cbRegExpFilter->setCurrentIndex(0);
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
    int loaded = model->rowCount();
    int filtered = model->filteredCount();
    int total = model->totalCount();

    m_lbStatNameInfo->setText(QStringLiteral("loaded:%1, filtered:%2, total:%3").arg(loaded).arg(filtered).arg(total));
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

void MainWindow::on_actionXmlToCSV_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter(QStringLiteral("KPI-KCI File (*.xml *xml.gz)"));

    if (fileDialog.exec() != QDialog::Accepted) {
        return;
    }

    ProgressDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Please Wait"));

    StatisticsFileParser fileParser(dialog);
    QString error;
    QStringList selectedFiles = fileDialog.selectedFiles();
    QString convertedPath = fileParser.kciKpiToCsvFormat(selectedFiles, error);
    if (error.isEmpty()) {
        if (!convertedPath.isEmpty()) {
            QString info = "KPI-KCI files have been converted to " + convertedPath;
            appendLogInfo(info);

            int answer = showQuestionMsgBox(this, info, QStringLiteral("Do you want to open it?"));
            if (answer == QMessageBox::Yes) {
                on_actionCloseAll_triggered();

                QStringList filePaths(convertedPath);
                addStatFiles(filePaths);
            }
        }
    } else {
        showErrorMsgBox(this, error);
    }
}

void MainWindow::on_actionCloseAll_triggered()
{
    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->clearStatisticsNames();
    for (int i = m_ui->lwStatFiles->count() - 1; i >= 0; --i) {
        delete m_ui->lwStatFiles->item(i);
    }
}

void MainWindow::on_actionPlot_triggered()
{
    parseStatFileData(false);
}

void MainWindow::on_actionPlotSeparately_triggered()
{
    parseStatFileData(true);
}

void MainWindow::on_actionClearFilterHistory_triggered()
{
    m_ui->cbRegExpFilter->clear();
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

void MainWindow::on_actionChangeLog_triggered()
{
    ChangeLogDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}
