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
    toolButton->setStatusTip(QStringLiteral("Case sensitive/insensitive match"));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolButton->installEventFilter(this);

    QHBoxLayout *hLayout = new QHBoxLayout(m_ui->cbRegExpFilter);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(toolButton);
    hLayout->addStretch();

    connect(toolButton, SIGNAL(clicked(bool)), this, SLOT(caseSensitiveButtonClicked(bool)));

    m_ui->lvStatName->setModel(new StatisticsNameModel(this));
    m_lbStatNameInfo = new QLabel(this);
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
    connect(m_ui->lwModules, &QListWidget::itemSelectionChanged, this, &MainWindow::updateModuleTextColor);

    connect(m_ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logEditContextMenuRequest);
    connect(m_ui->lvStatName, &QListView::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);
    connect(m_ui->lwModules, &QListWidget::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);

    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpenTriggered);
    connect(m_ui->actionXmlToCSV, &QAction::triggered, this, &MainWindow::actionXmlToCSVTriggered);
    connect(m_ui->actionClose, &QAction::triggered, this, &MainWindow::actionCloseTriggered);
    connect(m_ui->actionPlot, &QAction::triggered, this, &MainWindow::actionPlotTriggered);
    connect(m_ui->actionPlotSeparately, &QAction::triggered, this, &MainWindow::actionPlotSeparatelyTriggered);
    connect(m_ui->actionViewHelp, &QAction::triggered, this, &MainWindow::actionViewHelpTriggered);
    connect(m_ui->actionChangeLog, &QAction::triggered, this, &MainWindow::actionChangeLogTriggered);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAboutTriggered);

    // Some editors' (e.g. VSCode) behavior is to save file first with 0 length and then with the actual content, so
    // in this case the fileChanged signal whill be emitted twice. The Windows build-in notepad.exe only save once.
    // In addition, sometimes the signal only emmitted once with 0 file length. So, seems a better way is to use a timer
    // for reloading.
    m_filterReloadTimer.setInterval(500);
    m_filterReloadTimer.setSingleShot(true);
    connect(&m_filterFileWatcher, &QFileSystemWatcher::fileChanged, &m_filterReloadTimer, QOverload<>::of(&QTimer::start));
    connect(&m_filterReloadTimer, &QTimer::timeout, this, &MainWindow::favoriteFilterFileChanged);

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
    loadFavoriteFilterMenu();
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
        if (m_statFilePath.compare(path, Qt::CaseInsensitive)) {
            actionCloseTriggered();
        } else {
            return;
        }
    }
    if (!parseStatFileHeader(path)) {
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

bool MainWindow::parseStatFileHeader(const QString &path)
{
    ProgressDialog dialog(this);
    dialog.setLabelText(QStringLiteral("Parsing statistics file header, please wait!"));
    dialog.busyIndicatorMode();
    dialog.setCancelButtonVisible(false);

    QString error;
    StatisticsFileParser fileParser(dialog);
    std::string header = fileParser.parseFileHeader(path, m_offsetFromUtc, error);
    if (!error.isEmpty()) {
        appendErrorLog(error);
        return false;
    }

    StatisticsNameModel *model = static_cast<StatisticsNameModel*>(m_ui->lvStatName->model());
    StatisticsNameModel::StatisticsNames statNames;

    const char *ptr = strchr(header.c_str(), ';');
    ptr = strchr(ptr + 1, ';');
    if (ptr == nullptr) {
        appendErrorLog(QStringLiteral("%1 is empty").arg(path));
        return false;
    }
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
    return true;
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

void MainWindow::handleParsedStat(Statistics::NameDataMap &ndm, bool multipleWindows)
{
    QSize screenSize = m_resizeMan.getScreenSize();

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
            plotWindow->resize(screenSize * 0.75);
            connect(this, SIGNAL(aboutToBeClosed()), plotWindow, SLOT(close()));
            plotWindow->showMaximized();
        }
    } else {
        Statistics stat(ndm, m_offsetFromUtc);
        PlotWindow *plotWindow = new PlotWindow(stat);
        plotWindow->setAttribute(Qt::WA_DeleteOnClose);
        plotWindow->resize(screenSize * 0.75);
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
        QDateTime::currentDateTime().toString(DT_FORMAT_IN_PLOT), text));
}

void MainWindow::appendWarnLog(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#808080'>[%1]</font>  <font color='#c19c00'>WARN</font>: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT_IN_PLOT), text));
}

void MainWindow::appendErrorLog(const QString &text)
{
    m_ui->logTextEdit->appendHtml(QStringLiteral("<font color='#808080'>[%1]</font> <font color='#c50f1f'>ERROR</font>: %2").arg(
        QDateTime::currentDateTime().toString(DT_FORMAT_IN_PLOT), text));
}

QString MainWindow::filterHistoryFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_filter_hist"));
}

QString MainWindow::statDescriptionFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_stat_desc"));
}

QString MainWindow::favoriteFilterFilePath()
{
    return QDir::home().filePath(QStringLiteral(".vstat_filter_favorite.txt"));
}

void MainWindow::loadFilterHistory()
{
    QFile histFile(filterHistoryFilePath());
    if (histFile.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream ts(&histFile);
        while (ts.readLineInto(&line)) {
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
    m_sepAction = m_ui->menuFile->insertSeparator(m_ui->actionExit);

    for (int i = 0; i < (int)m_recentFileActions.size(); ++i) {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);

        m_ui->menuFile->insertAction(m_sepAction, m_recentFileActions[i]);
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

static int trimLeadingSpace(QString &str)
{
    int i = 0;
    for (; i < str.size(); ++i) {
        if (str.at(i) != ' ') {
            break;
        }
    }
    if (i < str.size()) {
        str.remove(0, i);
        return i;
    }
    return -1;
}

void MainWindow::loadFavoriteFilterMenu()
{
    int numSpaces;
    QStack<int> levelStack;
    QStack<QMenu*> menuStack;
    QString preLine, curLine;
    QTextStream ts;

    QFile file(favoriteFilterFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        goto _lbReturn;
    }
    ts.setDevice(&file);

    while (ts.readLineInto(&curLine)) {
        numSpaces = trimLeadingSpace(curLine);
        if (numSpaces < 0) {
            continue;
        }
        if (curLine.startsWith('#')) {
            continue;
        }
        if (preLine.isEmpty()) {
            preLine = curLine;
            levelStack.push(numSpaces);
            menuStack.push(m_ui->menuFilter);
            continue;
        }
        if (numSpaces > levelStack.top()) {
            QMenu *menu = menuStack.top()->addMenu(preLine);
            menuStack.push(menu);
            levelStack.push(numSpaces);
        } else {
            addFavoriteFilterAction(menuStack.top(), preLine);
            while (levelStack.size() > 1 && numSpaces < levelStack.top()) {
                levelStack.pop();
                menuStack.pop();
            }
        }
        preLine = curLine;
    }
    numSpaces = trimLeadingSpace(preLine);
    if (numSpaces >= 0) {
        addFavoriteFilterAction(menuStack.top(), preLine);
    }

_lbReturn:
    if (!m_ui->menuFilter->isEmpty()) {
        m_ui->menuFilter->addSeparator();
    }
    m_ui->menuFilter->addAction(QStringLiteral("Clear Filter History"), this, &MainWindow::actionClearFilterHistoryTriggered);
    QAction *action = m_ui->menuFilter->addAction(QStringLiteral("Edit Favorite Filters"), this, &MainWindow::actionEditFavoriteFilters);
    action->setStatusTip(QStringLiteral("Open file of favorite filters for edit"));
}

void MainWindow::addFavoriteFilterAction(QMenu *menu, const QString &line)
{
    QAction *action = menu->addAction(line.section(',', 0, 0), this, &MainWindow::actionFilterTriggered);
    QString tempStr = line.section(',', 1, 1);
    if (!tempStr.isEmpty()) {
        action->setData(tempStr);
    }
    tempStr = line.section(',', 2, 2);
    if (!tempStr.isEmpty()) {
        action->setStatusTip(tempStr);
    }
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

    QString path = urls.first().toLocalFile();
    if (path.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive) ||
            path.endsWith(QLatin1String(".csv.gz"), Qt::CaseInsensitive))
    {
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

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::ShowToParent && !m_resizeMan.showToParentHandled()) {
        m_resizeMan.resizeWidgetFromScreenSize(0.75, 0.75);

        int leftWidth = m_ui->splitterHor->width() * 0.23334;
        int bottomHeight = m_ui->splitterVer->height() * 0.13140604;
        m_ui->splitterHor->setSizes(QList<int>() << leftWidth << m_ui->splitterHor->width() - leftWidth - m_ui->splitterHor->handleWidth());
        m_ui->splitterVer->setSizes(QList<int>() << m_ui->splitterVer->height() - m_ui->splitterVer->handleWidth() - bottomHeight << bottomHeight);
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

    ChangeLogDialog dlg(this, true);
    if (dlg.exec() == QDialog::Accepted) {
        QString maintenanceToolPath = getMaintenanceToolPath();
        QProcess::startDetached(maintenanceToolPath, QStringList() << ("--updater") << "--proxy");
        QApplication::exit();
    }
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

    m_lbStatNameInfo->setText(QStringLiteral("Counter:%1, Matched:%2, Displayed:%3; ").arg(total).arg(matched).arg(displayed));
}

void MainWindow::updateModuleTextColor()
{
    bool hasSelection = m_ui->lwModules->selectionModel()->hasSelection();
    for (int i = 0; i < m_ui->lwModules->count(); ++i) {
        m_ui->lwModules->item(i)->setForeground(hasSelection ? Qt::gray : qvariant_cast<QBrush>(QVariant()));
    }
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

void MainWindow::favoriteFilterFileChanged()
{
    m_ui->menuFilter->clear();
    loadFavoriteFilterMenu();
    appendInfoLog(QStringLiteral("finished reloading favorite filters"));
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
    int answer = showQuestionMsgBox(this, QStringLiteral("Do you want to clear filter history?"), QString(), false);
    if (QMessageBox::Yes == answer) {
        m_ui->cbRegExpFilter->clear();
    }
}

void MainWindow::actionViewHelpTriggered()
{
    QUrl url(QStringLiteral("http://sdu.int.nokia-sbell.com:4099/help.html"));
    QDesktopServices::openUrl(url);
}

void MainWindow::actionChangeLogTriggered()
{
    ChangeLogDialog dlg(this, false);
    dlg.exec();
}

void MainWindow::actionAboutTriggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::actionEditFavoriteFilters()
{
    QString path = favoriteFilterFilePath();
    if (!QFileInfo::exists(path)) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            appendErrorLog(file.errorString());
            return;
        }
        QTextStream ts(&file);
        ts <<
"# Lines starting with '#' will be ignored. Menus and submenus will be created according to their\n"
"# indentation. Filter text and description are optional.\n"
"#\n"
"# Submenu1\n"
"#     Submenu2\n"
"#         Filter1\n"
"#         Filter2,filterText,description\n"
"#     Filter3\n"
"# Submenu3\n"
"#     Filter4\n"
"# Filter5";
    }

    // If the file is deleted or removed, the watching will stop. So, it's better to check if the watching
    // is still working, add it again if not.
    if (m_filterFileWatcher.files().isEmpty()) {
        m_filterFileWatcher.addPath(path);
    }

    QUrl url(path);
    QDesktopServices::openUrl(url);
}

void MainWindow::actionFilterTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString filterText = action->data().toString();
    if (filterText.isEmpty()) {
        m_ui->cbRegExpFilter->lineEdit()->setText(action->text());
    } else {
        m_ui->cbRegExpFilter->lineEdit()->setText(filterText);
    }
    updateFilterPattern();
}
