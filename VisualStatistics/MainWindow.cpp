#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PlotWindow.h"
#include "StatisticsNameModel.h"
#include "Utils.h"
#include "Version.h"
#include "AboutDialog.h"
#include "ProgressDialog.h"
#include "StatisticsFileParser.h"
#include "GzipFile.h"
#include "ChangeLogDialog.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QHostInfo>
#include <QSysInfo>
#include <QNetworkProxyQuery>

MainWindow::MainWindow() :
    QMainWindow(nullptr),
    m_ui(new Ui::MainWindow),
    m_lbStatNameInfo(nullptr),
    m_lbModulesInfo(nullptr),
    m_sepAction(nullptr),
    m_resizeMan(this)
{
    m_ui->setupUi(this);
    disableToolTipOfToolButton();

    m_ui->splitterHor->setStretchFactor(0, 0);
    m_ui->splitterHor->setStretchFactor(1, 1);
    m_ui->splitterVer->setStretchFactor(0, 1);
    m_ui->splitterVer->setStretchFactor(1, 0);

    QSettings settings;
    m_caseSensitive = settings.value(QStringLiteral("caseSensitive"), true).toBool();

    QToolButton *toolButton = new QToolButton();
    QFont font = toolButton->font();
    font.setBold(true);
    font.setStrikeOut(!m_caseSensitive);

    toolButton->setFont(font);
    toolButton->setStyleSheet(QStringLiteral("color:#999;"));
    toolButton->setAutoRaise(true);
    toolButton->setText(QStringLiteral("Aa"));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
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
    m_ui->cbRegExpFilter->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_ui->cbRegExpFilter->lineEdit()->setPlaceholderText(QStringLiteral("regular expression filter"));
    connect(m_ui->cbRegExpFilter, SIGNAL(activated(int)), this, SLOT(updateFilterPattern()));
    connect(m_ui->cbRegExpFilter->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::cbRegExpFilterEditReturnPressed);
    connect(m_ui->lvStatName, &QListView::doubleClicked, this, &MainWindow::listViewDoubleClicked);
    connect(m_ui->lwModules, &QListWidget::itemSelectionChanged, this, &MainWindow::updateFilterPattern);
    connect(m_ui->lwModules, &QListWidget::itemSelectionChanged, this, &MainWindow::updateModulesInfo);

    connect(m_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(m_ui->lvStatName, &QListView::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);
    connect(m_ui->lwModules, &QListWidget::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);

    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpenTriggered);
    connect(m_ui->actionXmlToCSV, &QAction::triggered, this, &MainWindow::actionXmlToCSVTriggered);
    connect(m_ui->actionClose, &QAction::triggered, this, &MainWindow::actionCloseTriggered);
    connect(m_ui->actionPlot, &QAction::triggered, this, &MainWindow::actionPlotTriggered);
    connect(m_ui->actionPlotSeparately, &QAction::triggered, this, &MainWindow::actionPlotSeparatelyTriggered);
    connect(m_ui->actionClearFilterHistory, &QAction::triggered, this, &MainWindow::actionClearFilterHistoryTriggered);
    connect(m_ui->actionViewHelp, &QAction::triggered, this, &MainWindow::actionViewHelpTriggered);
    connect(m_ui->actionChangeLog, &QAction::triggered, this, &MainWindow::actionChangeLogTriggered);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAboutTriggered);

    QUrl url("http://sdu.int.nokia-sbell.com:4099/");
    QNetworkProxyQuery npq(url);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (proxies.size() > 0) {
        m_netMan.setProxy(proxies[0]);
    }

#ifdef DEPLOY_VISUALSTAT
    startCheckNewVersionTask();
    startUserReportTask();
#endif
    startFetchStatDescriptionTask();

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
        appendWarnLog(QStringLiteral("unable to find updater"));
        return;
    }
    QProcess *process = new QProcess();
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(checkNewVersionTaskFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), SLOT(checkNewVersionTaskError(QProcess::ProcessError)));
    process->start(maintenanceToolPath, QStringList() << "--checkupdates" << "--proxy");
}

void MainWindow::startUserReportTask()
{
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

    QUrl url("http://sdu.int.nokia-sbell.com:4099/report");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = m_netMan.post(request, postData.toLatin1());
    connect(reply, &QNetworkReply::finished, this, &MainWindow::userReportTaskFinished);
}

void MainWindow::startFetchStatDescriptionTask()
{
    QUrl url("http://sdu.int.nokia-sbell.com:4099/counters.desc");
    QNetworkRequest request(url);
    QNetworkReply *reply = m_netMan.get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::fetchStatDescriptionFinished);
}

void MainWindow::disableToolTipOfToolButton()
{
    const auto toolButtons = m_ui->mainToolBar->findChildren<QToolButton*>();
    for (QToolButton *btn : toolButtons) {
        btn->setToolTip(QString());
    }
}

bool MainWindow::isRegexpCaseButtonResizeEvent(QObject *obj, QEvent *event)
{
    return event->type() == QEvent::Resize && m_ui->cbRegExpFilter->findChild<QToolButton *>() == obj;
}

void MainWindow::openStatFile(QString &path)
{
    path = QDir::toNativeSeparators(path);

    if (!m_statFilePath.isEmpty()) {
#if defined(Q_OS_WIN)
        Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
        Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
        if (m_statFilePath.compare(path, cs)) {
            actionCloseTriggered();
        } else {
            return;
        }
    }

    QString error;
    parseStatFileHeader(path, error);
    if (!error.isEmpty()) {
        appendErrorLog(error);
        return;
    }

    m_statFilePath = path;
    setWindowTitle("Visual Statistics - " + path);

    m_ui->cbRegExpFilter->lineEdit()->setFocus();

    QSettings settings;
    QStringList files = settings.value(QStringLiteral("recentFileList")).toStringList();
    files.removeOne(path);
    files.prepend(path);

    while (files.size() > (int)m_recentFileActions.size()) {
        files.removeLast();
    }

    settings.setValue(QStringLiteral("recentFileList"), files);

    updateRecentFileActions();
}

void MainWindow::parseStatFileHeader(const QString &path, QString &error)
{
    ProgressDialog dialog(this);
    dialog.setLabelText(QStringLiteral("Parsing statistics file header, please wait!"));
    dialog.busyIndicatorMode();
    dialog.setCancelButtonVisible(false);

    StatisticsFileParser fileParser(dialog);
    std::string header = fileParser.parseFileHeader(path, m_offsetFromUtc, error);
    if (error.isEmpty() && header.length() > 0) {
        StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
        StatisticsNameModel::StatisticsNames statNames;

        const char *ptr = strchr(header.c_str(), ';');
        ptr = strchr(ptr + 1, ';');
        splitString(ptr + 1, ';', statNames);
        statNames.back().erase(statNames.back().length() - 2);
        model->setStatisticsNames(statNames);

        m_ui->lwModules->addItems(model->getModules());

        QString filterText = m_ui->cbRegExpFilter->lineEdit()->text();
        if (!filterText.isEmpty()) {
            QString error;
            model->setFilterPattern(QStringList(), filterText, m_caseSensitive, error);
            if (!error.isEmpty()) {
                appendErrorLog(error);
            }
        }
    }
}

void MainWindow::parseStatFileData(bool multipleWindows)
{   
    if (m_statFilePath.isEmpty()) {
        return;
    }

    int statCountToPlot;
    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    const QModelIndexList selectedIndexes = m_ui->lvStatName->selectionModel()->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        statCountToPlot = model->rowCount();
    } else {
        statCountToPlot = selectedIndexes.size();
    }

    if (statCountToPlot <= 0) {
        return;
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
    dialog.setLabelText(QStringLiteral("Parsing statistics file data, please wait!"));

    StatisticsFileParser fileParser(dialog);
    Statistics::NameDataMap ndm;
    QString error;
    if (fileParser.parseFileData(indexNameMap, m_statFilePath, ndm, error)) {
        if (!error.isEmpty()) {
            appendErrorLog(error);
        } else if (!ndm.isEmpty()) {
            handleParsedStat(ndm, multipleWindows);
        }
    }
}

bool MainWindow::checkFileName(const QString &path)
{
    return path.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive) ||
            path.endsWith(QLatin1String(".csv.gz"), Qt::CaseInsensitive);
}

void MainWindow::handleParsedStat(Statistics::NameDataMap &ndm, bool multipleWindows)
{
    if (multipleWindows && ndm.size() > 1) {
        QVector<Statistics::NameDataMap> ndms = Statistics::divideNameDataMap(ndm);
        for (Statistics::NameDataMap &tempNdm : ndms) {
            if ((QApplication::keyboardModifiers() & Qt::ControlModifier) &&
                    Statistics::isConstantDataMap(tempNdm.first()))
            {
                continue;
            }
            Statistics stat(tempNdm, m_offsetFromUtc);
            PlotWindow *plotWindow = new PlotWindow(stat);
            plotWindow->setAttribute(Qt::WA_DeleteOnClose);
            m_resizeMan.resizeWidget(plotWindow);
            connect(this, SIGNAL(aboutToBeClosed()), plotWindow, SLOT(close()));
            plotWindow->showMaximized();
        }
    } else {
        Statistics stat(ndm, m_offsetFromUtc);
        PlotWindow *plotWindow = new PlotWindow(stat);
        plotWindow->setAttribute(Qt::WA_DeleteOnClose);
        m_resizeMan.resizeWidget(plotWindow);
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

void MainWindow::appendInfoLog(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#808080'>[%1]</font>  <font color='#13a10e'>INFO</font>: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendWarnLog(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#808080'>[%1]</font>  <font color='#c19c00'>WARN</font>: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

void MainWindow::appendErrorLog(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#808080'>[%1]</font> <font color='#c50f1f'>ERROR</font>: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT), text));
}

QString MainWindow::filterHistoryFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_filter_hist"));
}

QString MainWindow::statDescriptionFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_stat_desc"));
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

void MainWindow::updateCaseSensitiveButtonFont()
{
    QToolButton *toolButton = m_ui->cbRegExpFilter->findChild<QToolButton *>();
    QFont font = toolButton->font();
    font.setStrikeOut(!m_caseSensitive);
    toolButton->setFont(font);
}

void MainWindow::initializeRecentFileActions()
{
    m_sepAction = m_ui->menu_File->insertSeparator(m_ui->actionExit);

    for (int i = 0; i < (int)m_recentFileActions.size(); ++i) {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);

        m_ui->menu_File->insertAction(m_sepAction, m_recentFileActions[i]);
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value(QStringLiteral("recentFileList")).toStringList();

    files.erase(std::remove_if(files.begin(), files.end(),
                               [](const QString &path) {
        return !QFileInfo::exists(path);
    }), files.end());

    int numRecentFiles = qMin(files.size(), (int)m_recentFileActions.size());

    for (int i = 0; i < numRecentFiles; ++i) {
        QString path = files[i];
        QString text = QString("%1: %2").arg(i + 1).arg(QFileInfo(path).fileName());

        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(path);
        m_recentFileActions[i]->setStatusTip(path);
        m_recentFileActions[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < (int)m_recentFileActions.size(); ++j) {
        m_recentFileActions[j]->setVisible(false);
    }

    m_sepAction->setVisible(numRecentFiles > 0);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
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
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() != 1) {
        return;
    }

    if (checkFileName(urls.first().toLocalFile())) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        QString path = url.toLocalFile();
        openStatFile(path);
        return;
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    emit aboutToBeClosed();

    saveFilterHistory();

    QSettings settings;
    settings.setValue(QStringLiteral("caseSensitive"), m_caseSensitive);
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    double scale = m_resizeMan.currentScreenScale();
    if (qFuzzyCompare(scale, m_resizeMan.scale())) {
        return;
    }
    if (!qFuzzyIsNull(m_resizeMan.scale())) {
        double factor = scale/m_resizeMan.scale();
        auto sizesHor = m_ui->splitterHor->sizes();
        auto sizesVer = m_ui->splitterVer->sizes();

        int leftWidth = qRound(sizesHor[0] * factor);
        int rightWidth = sizesHor[0] + sizesHor[1] - leftWidth;
        m_ui->splitterHor->setSizes(QList<int>() << leftWidth << rightWidth);

        int bottomHeight = qRound(sizesVer[1] * factor);
        int topHeight = sizesVer[0] + sizesVer[1] - bottomHeight;
        m_ui->splitterVer->setSizes(QList<int>() << topHeight << bottomHeight);
    }
    m_resizeMan.setScale(scale);
}

bool MainWindow::event(QEvent *event)
{
    // Get the screen scale value in which this window is shown initially.
    // Please note that we cannot do this in showEvent because at that point
    // this window's position is on primary screen which may be different
    // from the position after it is shown. It seems that the position is
    // changed in show_sys() call.
    //
    // QShowEvent showEvent;
    // QCoreApplication::sendEvent(q, &showEvent);
    //
    // show_sys();
    //
    // This event is sent by QCoreApplication::sendEvent(q, &showToParentEvent)
    // in QWidgetPrivate::setVisible(bool visible).
    if (event->type() == QEvent::ShowToParent && !m_resizeMan.showToParentHandled()) {
        int leftWidth = 320, bottomHeight = 100;
        if (m_resizeMan.resizeWidgetOnShowToParent()) {
            leftWidth *= m_resizeMan.scale();
            bottomHeight *= m_resizeMan.scale();
        }
        m_ui->splitterHor->setSizes(QList<int>() << leftWidth << width() - leftWidth);
        m_ui->splitterVer->setSizes(QList<int>() << height() - bottomHeight << bottomHeight);
    }
    return QMainWindow::event(event);
}

void MainWindow::checkNewVersionTaskFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    sender()->deleteLater();

    if (exitStatus != QProcess::NormalExit) {
        appendWarnLog(QStringLiteral("updater crashed"));
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

    appendWarnLog(QStringLiteral("updater failed with error %1").arg(error));
}

void MainWindow::userReportTaskFinished()
{
    sender()->deleteLater();
}

void MainWindow::fetchStatDescriptionFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    QFile file(statDescriptionFilePath());
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray netData = reply->readAll();
        if (file.open(QIODevice::ReadWrite)) {
            QByteArray fileData = file.readAll();
            if (fileData != netData && file.resize(0)) {
                file.write(netData);
            }
            file.close();
        }
    }

    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->parseStatDescription(file.fileName());
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
    QStringList modules;
    const auto selectedModules = m_ui->lwModules->selectedItems();
    for (const QListWidgetItem *item : selectedModules) {
        modules.append(item->text());
    }

    QString error;
    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->setFilterPattern(
                modules, m_ui->cbRegExpFilter->lineEdit()->text(), m_caseSensitive, error);
    if (!error.isEmpty()) {
        appendErrorLog(error);
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

void MainWindow::listViewCtxMenuRequest(const QPoint &pos)
{
    QListView *view = qobject_cast<QListView *>(sender());
    QMenu *menu = new QMenu();

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(QStringLiteral("Copy Selected"), [view](){
        const QModelIndexList indexList = view->selectionModel()->selectedIndexes();
        if (indexList.size() > 0) {
            QStringList stringList;
            for (const QModelIndex &index : indexList) {
                stringList << view->model()->data(index).toString();
            }
            QApplication::clipboard()->setText(stringList.join(QStringLiteral("\n")));
        }
    });
    menu->addAction(QStringLiteral("Clear Selection"), view, SLOT(clearSelection()));
    menu->popup(view->mapToGlobal(pos));
}

void MainWindow::clearLogEdit()
{
    m_ui->logTextEdit->clear();
}

void MainWindow::updateStatNameInfo()
{
    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    int displayed = model->rowCount();
    int matched = model->matchedCount();
    int total = model->totalCount();

    m_lbStatNameInfo->setText(QStringLiteral("Stat:%1, Matched:%2, Displayed:%3; ").arg(total).arg(matched).arg(displayed));
}

void MainWindow::updateModulesInfo()
{
    int modules = m_ui->lwModules->count();
    int selected = m_ui->lwModules->selectedItems().size();

    m_lbModulesInfo->setText(QStringLiteral("Module:%1, Selected:%2; ").arg(modules).arg(selected));
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString path = action->data().toString();
    openStatFile(path);
}

void MainWindow::caseSensitiveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    m_caseSensitive = !m_caseSensitive;
    updateCaseSensitiveButtonFont();
    updateFilterPattern();
}

void MainWindow::actionOpenTriggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(QStringLiteral("Statistics File (*.csv *.csv.gz)"));

    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList strList = fileDialog.selectedFiles();
        openStatFile(strList.first());
    }
}

void MainWindow::actionXmlToCSVTriggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter(QStringLiteral("KPI-KCI File (*.xml *xml.gz)"));

    if (fileDialog.exec() != QDialog::Accepted) {
        return;
    }

    ProgressDialog dialog(this);

    StatisticsFileParser fileParser(dialog);
    QString error;
    QStringList selectedFiles = fileDialog.selectedFiles();
    QString convertedPath = fileParser.kpiKciToCsvFormat(selectedFiles, error);
    if (error.isEmpty()) {
        if (!convertedPath.isEmpty()) {
            QString info = "KPI-KCI files have been converted to " + QDir::toNativeSeparators(convertedPath);
            appendInfoLog(info);
            actionCloseTriggered();
            openStatFile(convertedPath);

            int answer = showQuestionMsgBox(this, info, QStringLiteral("Do you want to delete the original XML files?"), false);
            if (answer == QMessageBox::Yes) {
                for (const QString &path : qAsConst(selectedFiles)) {
                    if (QFile::remove(path)) {
                        appendInfoLog(QStringLiteral("delete %1 successfully").arg(QDir::toNativeSeparators(path)));
                    } else {
                        appendWarnLog(QStringLiteral("delete %1 unsuccessfully").arg(QDir::toNativeSeparators(path)));
                    }
                }
            }
        }
    } else {
        showErrorMsgBox(this, error);
    }
}

void MainWindow::actionCloseTriggered()
{
    static_cast<StatisticsNameModel*>(m_ui->lvStatName->model())->clearStatisticsNames();
    m_ui->lwModules->clear();
    m_statFilePath.clear();
    setWindowTitle(QStringLiteral("Visual Statistics"));
}

void MainWindow::actionPlotTriggered()
{
    parseStatFileData(false);
}

void MainWindow::actionPlotSeparatelyTriggered()
{
    parseStatFileData(true);
}

void MainWindow::actionClearFilterHistoryTriggered()
{
    m_ui->cbRegExpFilter->clear();
}

void MainWindow::actionViewHelpTriggered()
{
    QUrl url(QStringLiteral("http://sdu.int.nokia-sbell.com:4099/help.html"));
    QDesktopServices::openUrl(url);
}

void MainWindow::actionChangeLogTriggered()
{
    ChangeLogDialog dlg(this);
    dlg.exec();
}

void MainWindow::actionAboutTriggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}
