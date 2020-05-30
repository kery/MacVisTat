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
    m_ui(new Ui::MainWindow),
    m_caseSensitive(true),
    m_lbStatNameInfo(nullptr),
    m_lbModulesInfo(nullptr),
    m_sepAction(nullptr)
{
    m_ui->setupUi(this);

    // used to disable tooltip for tool button
    installEventFilterForAllToolButton();

    m_ui->splitterHor->setSizes(QList<int>() << 330 << width() - 330);
    m_ui->splitterHor->setStretchFactor(0, 0);
    m_ui->splitterHor->setStretchFactor(1, 1);
    m_ui->splitterVer->setSizes(QList<int>() << height() - 80 << 80);
    m_ui->splitterVer->setStretchFactor(0, 1);
    m_ui->splitterVer->setStretchFactor(1, 0);

    QToolButton *toolButton = new QToolButton();
    QFont font = toolButton->font();
    font.setBold(true);

    toolButton->setFont(font);
    toolButton->setStyleSheet(QStringLiteral("color:#999;"));
    toolButton->setAutoRaise(true);
    toolButton->setText(QStringLiteral("Aa"));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolButton->setToolTip(QStringLiteral("Case sensitive"));
    toolButton->installEventFilter(this);

    QHBoxLayout *hLayout = new QHBoxLayout(m_ui->cbRegExpFilter);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(toolButton);
    hLayout->addStretch();

    connect(toolButton, SIGNAL(clicked(bool)), this, SLOT(caseSensitiveButtonClicked(bool)));

    m_ui->lvStatName->setModel(new StatisticsNameModel(this));

    m_lbModulesInfo = new QLabel(this);
    m_lbModulesInfo->setStyleSheet(QStringLiteral("QLabel{color:#888888}"));
    m_ui->statusBar->addPermanentWidget(m_lbModulesInfo);
    updateModulesInfo();
    connect(m_ui->lwModules->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateModulesInfo()));
    connect(m_ui->lwModules->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateModulesInfo()));

    m_lbStatNameInfo = new QLabel(this);
    m_lbStatNameInfo->setStyleSheet(QStringLiteral("QLabel{color:#888888}"));
    m_ui->statusBar->addPermanentWidget(m_lbStatNameInfo);
    updateStatNameInfo();
    connect(m_ui->lvStatName->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateStatNameInfo()));
    connect(m_ui->lvStatName->model(), SIGNAL(modelReset()), this, SLOT(updateStatNameInfo()));

    m_ui->cbRegExpFilter->lineEdit()->setClearButtonEnabled(true);
    connectClearButtonSignal();
    m_ui->cbRegExpFilter->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_ui->cbRegExpFilter->lineEdit()->setPlaceholderText(QStringLiteral("regular expression filter"));
    connect(m_ui->cbRegExpFilter, SIGNAL(activated(int)), this, SLOT(updateFilterPattern()));
    connect(m_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::cbRegExpFilterEditReturnPressed);
    connect(m_ui->lvStatName, &QListView::doubleClicked, this, &MainWindow::listViewDoubleClicked);
    connect(m_ui->lwModules, &QListWidget::itemSelectionChanged, this, &MainWindow::updateFilterPattern);
    connect(m_ui->lwModules, &QListWidget::itemSelectionChanged, this, &MainWindow::updateModulesInfo);

    connect(m_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(m_ui->lvStatName, &QListView::customContextMenuRequested, this, &MainWindow::lvStatNameCtxMenuRequest);
    connect(m_ui->lwModules, &QListWidget::customContextMenuRequested, this, &MainWindow::lwModulesCtxMenuRequest);

#ifdef INSTALLER
    startCheckNewVersionTask();
    startUserReportTask();
#endif

    loadFilterHistory();

    initializeRecentFileActions();
    updateRecentFileActions();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::startCheckNewVersionTask()
{
    QString maintenanceToolPath = getMaintenanceToolPath();
    if (maintenanceToolPath.isEmpty()) {
        appendLogWarn(QStringLiteral("unable to find updater"));
        return;
    }
    QProcess *process = new QProcess();
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(checkNewVersionTaskFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), SLOT(checkNewVersionTaskError(QProcess::ProcessError)));
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
    postData += "&pt=";
    postData += QSysInfo::productType();

    QString userName = getUserName();
    if (!userName.isEmpty()) {
        postData += "-";
        postData += userName;
    }

    postData += "&ver=";
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

bool MainWindow::isRegexpCaseButtonResizeEvent(QObject *obj, QEvent *event)
{
    return event->type() == QEvent::Resize && m_ui->cbRegExpFilter->findChild<QToolButton *>() == obj;
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

    QSettings settings;
    QStringList files = settings.value(QStringLiteral("recentFileList")).toStringList();
    for (const QString &path : filePaths) {
        files.removeOne(path);
        files.prepend(path);
    }

    while (files.size() > (int)m_recentFileActions.size()) {
        files.removeLast();
    }

    settings.setValue(QStringLiteral("recentFileList"), files);

    updateRecentFileActions();
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

        m_ui->lwModules->addItems(model->getModules());

        QString filterText = m_ui->cbRegExpFilter->lineEdit()->text();
        if (!filterText.isEmpty()) {
            QStringList errList;
            model->setFilterPattern(QStringList(), filterText, m_caseSensitive, errList);
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
    m_ui->logTextEdit->appendHtml(QStringLiteral("[%1] INFO: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendLogWarn(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("[%1] WARN: <font color='#CC9900'>%2</font>").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendLogError(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("[%1] ERR: <font color='red'>%2</font>").arg(
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

void MainWindow::connectClearButtonSignal()
{
    QLineEdit *lineEdit = m_ui->cbRegExpFilter->lineEdit();
    for (QObject *child : lineEdit->children()) {
        QAction *action = qobject_cast<QAction *>(child);
        // "_q_qlineeditclearaction" is defined in qlineedit.cpp
        if (action && action->objectName() == QLatin1String("_q_qlineeditclearaction")) {
            connect(action, &QAction::triggered, this, &MainWindow::updateFilterPattern, Qt::QueuedConnection);
            return;
        }
    }
}

void MainWindow::initializeRecentFileActions()
{
    m_sepAction = m_ui->menu_File->insertSeparator(m_ui->actionExit);

    for (int i = 0; i < (int)m_recentFileActions.size(); ++i) {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], SIGNAL(triggered()), this, SLOT(addRecentFile()));

        m_ui->menu_File->insertAction(m_sepAction, m_recentFileActions[i]);
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value(QStringLiteral("recentFileList")).toStringList();

    files.erase(std::remove_if(files.begin(), files.end(),
                               [](const QString &path) {
        return !QFileInfo(path).exists();
    }), files.end());

    int numRecentFiles = qMin(files.size(), (int)m_recentFileActions.size());

    for (int i = 0; i < numRecentFiles; ++i) {
        QString path = files[i];
        QString text = QString("%1: %2").arg(i + 1).arg(QFileInfo(path).fileName());

        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(path);
        m_recentFileActions[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < (int)m_recentFileActions.size(); ++j) {
        m_recentFileActions[j]->setVisible(false);
    }

    m_sepAction->setVisible(numRecentFiles > 0);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isToolTipEventOfToolButton(obj, event)) {
        return true;
    }

    if (isRegexpCaseButtonResizeEvent(obj, event)) {
        // The value is from qlineedit_p.cpp
        const int horizontalMargin = 2;

        QPoint pos = m_ui->cbRegExpFilter->lineEdit()->pos();
        const QSize &size = static_cast<QResizeEvent *>(event)->size();
        const int leftMargin = size.width() - pos.x() - horizontalMargin;
        m_ui->cbRegExpFilter->lineEdit()->setTextMargins(leftMargin, 0, 0, 0);
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
    sender()->deleteLater();

    if (exitStatus != QProcess::NormalExit) {
        appendLogWarn(QStringLiteral("updater crashed"));
        return;
    }

    // exitCode != 0 indicates that there is no update available
    // or failed to check update
    if (exitCode != 0) {
        return;
    }

    ChangeLogDialog dlg(this);
    dlg.setShownAfterCheckingUpdates();
    dlg.exec();

    QString maintenanceToolPath = getMaintenanceToolPath();
    QProcess::startDetached(maintenanceToolPath, QStringList() << ("--updater") << "--proxy");
    QApplication::exit();
}

void MainWindow::checkNewVersionTaskError(QProcess::ProcessError error)
{
    // Don't worry about the deleteLater of QProcess will be called twice in case both finished and
    // errorOccurred signals are emitted. Because deleteLater has below note:
    // It is safe to call this function more than once; when the first deferred deletion event is
    // delivered, any pending events for the object are removed from the event queue.
    sender()->deleteLater();

    appendLogWarn(QStringLiteral("updater failed with error %1").arg(error));
}

void MainWindow::userReportTaskFinished(QNetworkReply *reply)
{
    sender()->deleteLater();
    reply->deleteLater();
}

void MainWindow::cbRegExpFilterEditReturnPressed()
{
    // If current text of combo box is empty and return key pressed, the "activated" signal
    // will not be sent. So, we have to call updateFilterPattern manually.
    if (m_ui->cbRegExpFilter->currentText().isEmpty()) {
        updateFilterPattern();
    } else {
        adjustFilterHistoryOrder();
    }
}

void MainWindow::updateFilterPattern()
{
//    QByteArray baSig;
//    if (sender()) {
//        QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
//        baSig = metaMethod.methodSignature();
//    }

//    static int c = 0;
//    qDebug() << "updateFilterPattern" << ++c << m_ui->cbRegExpFilter->currentIndex() << baSig;

    QStringList modules, errList;
    for (const QListWidgetItem *item : m_ui->lwModules->selectedItems()) {
        modules.append(item->text());
    }

    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->setFilterPattern(
                modules, m_ui->cbRegExpFilter->lineEdit()->text(), m_caseSensitive, errList);
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

void MainWindow::lvStatNameCtxMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(QStringLiteral("Copy Selected"), this, SLOT(copyLvStatNameSelected()));
    menu->addAction(QStringLiteral("Clear Selection"), m_ui->lvStatName, SLOT(clearSelection()));

    menu->popup(m_ui->lvStatName->mapToGlobal(pos));
}

void MainWindow::lwModulesCtxMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(QStringLiteral("Clear Selection"), m_ui->lwModules, SLOT(clearSelection()));

    menu->popup(m_ui->lwModules->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    m_ui->logTextEdit->clear();
}

void MainWindow::copyLvStatNameSelected()
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

void MainWindow::updateModulesInfo()
{
    int modules = m_ui->lwModules->count();
    int selected = m_ui->lwModules->selectedItems().size();

    m_lbModulesInfo->setText(QStringLiteral("modules:%1, selected:%2 ").arg(modules).arg(selected));
}

void MainWindow::addRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QStringList recentFile = action->data().toStringList();
    addStatFiles(recentFile);
}

void MainWindow::caseSensitiveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    m_caseSensitive = !m_caseSensitive;

    QToolButton *toolButton = m_ui->cbRegExpFilter->findChild<QToolButton *>();
    QFont font = toolButton->font();
    font.setStrikeOut(!m_caseSensitive);

    toolButton->setFont(font);
    toolButton->setToolTip(m_caseSensitive ? QStringLiteral("Case sensitive") : QStringLiteral("Case insensitive"));
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
    QString convertedPath = fileParser.kpiKciToCsvFormat(selectedFiles, error);
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

    for (int i = m_ui->lwModules->count() - 1; i >= 0; --i) {
        delete m_ui->lwModules->item(i);
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
