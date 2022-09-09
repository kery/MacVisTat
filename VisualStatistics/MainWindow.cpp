#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Application.h"
#include "PlotWindow.h"
#include "CounterNameModel.h"
#include "CounterFileParser.h"
#include "KpiKciFileParser.h"
#include "ChangeLogDialog.h"
#include "AboutDialog.h"
#include "OptionsDialog.h"
#include "GlobalDefines.h"
#include "FileDialog.h"
#include "FilterValidator.h"
#include "Utils.h"
#include "Version.h"
#include <QHostInfo>
#include <QNetworkReply>

MainWindow::MainWindow() :
    ui(new Ui::MainWindow),
    mL(nullptr),
    mOffsetFromUtc(0),
    mStatusBarLabel(new QLabel(this)),
    mLogDateTimeFmt(DTFMT_DISPLAY),
    mResizeMan(this)
{
    ui->setupUi(this);

    QStackedLayout *stackedLayout = new QStackedLayout();
    stackedLayout->addWidget(ui->logTextEdit);
    stackedLayout->addWidget(ui->counterDescription);
    ui->stackedWidget->setLayout(stackedLayout);
    ui->counterNameViewParent->installEventFilter(this);

    setupFilterComboBox();
    updateWindowTitle();

    ui->splitterHor->setStretchFactor(0, 0);
    ui->splitterHor->setStretchFactor(1, 1);
    ui->splitterVer->setStretchFactor(0, 1);
    ui->splitterVer->setStretchFactor(1, 0);

    CounterNameModel *model = new CounterNameModel(this);
    model->setCounterDescription(&mCounterDesc);
    ui->counterNameView->setModel(model);
    ui->statusBar->addPermanentWidget(mStatusBarLabel);
    updateCounterNameCountInfo();

    // Some editors' (e.g. VSCode) behavior is to save file first with 0 length and then with the actual content, so
    // in this case the fileChanged signal whill be emitted twice. The Windows build-in notepad.exe only save once.
    // In addition, sometimes the signal only emmitted once with 0 file length. So, seems a better way is to use a timer
    // for reloading.
    mFilterMenuReloadTimer.setInterval(500);
    mFilterMenuReloadTimer.setSingleShot(true);

    connect(ui->counterNameView, &QListView::doubleClicked, this, &MainWindow::counterNameViewDoubleClicked);
    connect(ui->moduleNameView, &QListWidget::itemSelectionChanged, this, &MainWindow::updateFilterPattern);
    connect(ui->moduleNameView, &QListWidget::itemSelectionChanged, this, &MainWindow::updateModuleNameColor);
    connect(ui->counterNameView, &QListView::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);
    connect(ui->moduleNameView, &QListWidget::customContextMenuRequested, this, &MainWindow::listViewCtxMenuRequest);
    connect(model, &CounterNameModel::rowsInserted, this, &MainWindow::updateCounterNameCountInfo);
    connect(model, &CounterNameModel::modelReset, this, &MainWindow::updateCounterNameCountInfo);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpenTriggered);
    connect(ui->actionXmlToCsv, &QAction::triggered, this, &MainWindow::actionXmlToCsvTriggered);
    connect(ui->actionCloseFile, &QAction::triggered, this, &MainWindow::actionCloseFileTriggered);
    connect(ui->actionPlot, &QAction::triggered, this, &MainWindow::actionPlotTriggered);
    connect(ui->actionPlotSeparately, &QAction::triggered, this, &MainWindow::actionPlotSeparatelyTriggered);
    connect(ui->actionOpenPluginsFolder, &QAction::triggered, this, &MainWindow::actionOpenPluginsFolderTriggered);
    connect(ui->actionBrowseOnlinePlugins, &QAction::triggered, this, &MainWindow::actionBrowseOnlinePluginsTriggered);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::actionOptionsTriggered);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::actionHelpTriggered);
    connect(ui->actionChangeLog, &QAction::triggered, this, &MainWindow::actionChangeLogTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAboutTriggered);
    connect(&mFilterMenuFileWatcher, &QFileSystemWatcher::fileChanged, &mFilterMenuReloadTimer, QOverload<>::of(&QTimer::start));
    connect(&mFilterMenuReloadTimer, &QTimer::timeout, this, &MainWindow::filterMenuFileChanged);

    initRecentFileActions();
    updateRecentFileActions(QStringList());
    loadFilterMenu();
    loadFilterHistory();
    initLuaEnv();

#ifdef DEPLOY_VISUALSTAT
    checkUpdate();
    usageReport();
#endif
    listOnlinePlugins();
    downloadCounterDescription();
}

MainWindow::~MainWindow()
{
    if (mL != nullptr) {
        lua_close(mL);
    }
    delete ui;
}

void MainWindow::appendInfoLog(const QString &text)
{
    ui->logTextEdit->appendHtml(formatLog(text, llInfo));
}

void MainWindow::appendWarnLog(const QString &text)
{
    ui->logTextEdit->appendHtml(formatLog(text, llWarn));
}

void MainWindow::appendErrorLog(const QString &text)
{
    ui->logTextEdit->appendHtml(formatLog(text, llError));
}

QAction *MainWindow::registerMenu(const QString &title, const QString &description)
{
    QStringList menuTitles = title.split('/', QString::SkipEmptyParts);
    if (menuTitles.isEmpty()) {
        luaL_error(mL, "menu title is empty");
    }

    QMenu *curMenu = ui->menuPlot;
    for (int i = 0; i < menuTitles.size() - 1; ++i) {
        QString simpleTitle = menuTitles[i].simplified();
        QMenu *tempMenu = curMenu->findChild<QMenu *>(simpleTitle, Qt::FindDirectChildrenOnly);
        if (tempMenu == nullptr) {
            tempMenu = curMenu->addMenu(simpleTitle);
            tempMenu->setObjectName(simpleTitle);
        }
        curMenu = tempMenu;
    }

    QAction *action = curMenu->addAction(menuTitles.last().simplified(), this, &MainWindow::actionRegisteredMenuTriggered);
    if (!description.isEmpty()) {
        action->setStatusTip(description);
    }
    return action;
}

int MainWindow::parseCounterFileData()
{
    const char *regexp = luaL_checkstring(mL, 1);
    double defValue = luaL_optnumber(mL, 2, NAN);

    if (mCounterFilePath.isEmpty()) {
        luaL_error(mL, "no counter file opened");
    }

    CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
    IndexNameMap inm = model->getIndexNameMap(regexp);
    if (inm.isEmpty()) {
        luaL_error(mL, "no counter matches filter '%s'", regexp);
    }

    bool canceled;
    CounterDataMap dataMap;
    CounterFileParser parser(this);
    QString error = parser.parseData(mCounterFilePath, inm, dataMap, canceled);

    if (canceled) {
        luaL_error(mL, "operation canceled");
    }

    if (!error.isEmpty()) {
        luaL_error(mL, "%s", error.toStdString().c_str());
    }

    const CounterData &cdata = dataMap.first();
    lua_createtable(mL, cdata.data.size(), 0); // first result table which contains timestamps
    for (int i = 1; i <= cdata.data.size(); ++i) {
        lua_pushnumber(mL, cdata.data.at(i - 1)->key);
        lua_rawseti(mL, -2, i);
    }

    int n = 0;
    lua_createtable(mL, dataMap.size(), 0); // second result table which contains counters
    for (auto iter = dataMap.begin(); iter != dataMap.end(); ++iter) {
        lua_createtable(mL, 0, 2);
        lua_pushstring(mL, "name");
        lua_pushstring(mL, iter.key().toStdString().c_str());
        lua_rawset(mL, -3);

        lua_pushstring(mL, "values");
        lua_createtable(mL, cdata.data.size(), 0);
        for (int i = 1; i <= cdata.data.size(); ++i) {
            double value = iter.value().data.at(i - 1)->value;
            lua_pushnumber(mL, qIsNaN(value) ? defValue : value);
            lua_rawseti(mL, -2, i);
        }
        lua_rawset(mL, -3);

        lua_rawseti(mL, -2, ++n);
    }

    return 2;
}

void MainWindow::actionOpenTriggered()
{
    FileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilter(QStringLiteral("Counter File (*.csv *.csv.gz)"));

    if (dlg.exec() == QDialog::Accepted) {
        auto files = dlg.selectedFiles();
        openCounterFile(files.first());
    }
}

void MainWindow::actionXmlToCsvTriggered()
{
    FileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters({QStringLiteral("Gzip KPI/KCI File (*.xml.gz)"),
                        QStringLiteral("XML KPI/KCI File (*.xml)"),
                        QStringLiteral("Any KPI/KCI File (*)")});
    connect(&dlg, &QFileDialog::directoryEntered, this, &MainWindow::kpikciFileDlgDirEntered);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    actionCloseFileTriggered();

    QVector<QString> errors, paths = dlg.selectedFiles().toVector();
    KpiKciFileParser parser(this);
    QString outPath = parser.convertToCsv(paths, errors);
#if defined(Q_OS_WIN)
    if (!errors.isEmpty()) { MessageBeep(MB_ICONWARNING); }
#endif
    for (const QString &err : qAsConst(errors)) {
        appendErrorLog(err);
    }
    if (outPath.isEmpty()) { return; }

    openCounterFile(outPath);
}

void MainWindow::actionCloseFileTriggered()
{
    mCounterFilePath.clear();
    updateWindowTitle();
    ui->moduleNameView->clear();
    CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
    model->clear();
}

void MainWindow::actionRecentFileTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString path = action->data().toString();
    openCounterFile(path);
}

void MainWindow::actionFilterTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString filterText = action->data().toString();
    if (filterText.isEmpty()) {
        ui->filterComboBox->lineEdit()->setText(action->text());
    } else {
        ui->filterComboBox->lineEdit()->setText(filterText);
    }
    updateFilterPattern();
}

void MainWindow::actionClearFilterHistoryTriggered()
{
    int answer = showQuestionMsgBox(this, QStringLiteral("Do you want to clear filter history?"));
    if (answer == QMessageBox::Yes) {
        ui->filterComboBox->clear();
    }
}

void MainWindow::actionEditFilterMenuTriggered()
{
    QString path = filePath(fpFilterMenu);
    if (!QFileInfo::exists(path)) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            appendErrorLog(file.errorString());
            return;
        }
        QTextStream ts(&file);
        ts << "# Lines starting with '#' will be ignored. Menus and submenus will be created according to their\n"
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
    if (mFilterMenuFileWatcher.files().isEmpty()) {
        mFilterMenuFileWatcher.addPath(path);
    }

    QDesktopServices::openUrl(path);
}

void MainWindow::actionPlotTriggered()
{
    parseCounterFileData(false);
}

void MainWindow::actionPlotSeparatelyTriggered()
{
    parseCounterFileData(true);
}

void MainWindow::actionOpenPluginsFolderTriggered()
{
    QDir dir(filePath(fpPluginDir));
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}

void MainWindow::actionBrowseOnlinePluginsTriggered()
{
    Application *app = Application::instance();
    QUrl url = app->getUrl(Application::upPlugins);
    QDesktopServices::openUrl(url);
}

void MainWindow::actionOptionsTriggered()
{
    OptionsDialog dlg(this);
    dlg.exec();
}

void MainWindow::actionHelpTriggered()
{
    QUrl url = Application::instance()->getUrl(Application::upHelp);
    QDesktopServices::openUrl(url);
}

void MainWindow::actionChangeLogTriggered()
{
    ChangeLogDialog dlg(this, false);
    dlg.exec();
}

void MainWindow::actionAboutTriggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::actionRegisteredMenuTriggered()
{
    QObject *source = sender();
    if (source == nullptr) {
        return;
    }

    lua_rawgetp(mL, LUA_REGISTRYINDEX, source);
    if (lua_pcall(mL, 0, 2, 0) != LUA_OK) {
        QString err = getLastLuaError(mL);
        appendWarnLog(err);
        return;
    }

    if (!lua_istable(mL, 1) || !lua_istable(mL, 2)) {
        lua_pop(mL, 2);
        appendWarnLog(QStringLiteral("the registered function must return two tables"));
        return;
    }

    QVector<double> timestamps;
    lua_pushnil(mL);
    while (lua_next(mL, 1) != 0) {
        int isNum;
        double ts = lua_tonumberx(mL, -1, &isNum);
        if (isNum) {
            timestamps.append(ts);
            lua_pop(mL, 1);
        } else {
            lua_pop(mL, 4);
            appendWarnLog(QStringLiteral("elements in first result table must be Unix time"));
            return;
        }
    }

    CounterDataMap dataMap;
    lua_pushnil(mL);
    while (lua_next(mL, 2) != 0) {
        if (!lua_istable(mL, -1)) {
            lua_pop(mL, 4);
            appendWarnLog(QStringLiteral("elements in second result table must be a table"));
            return;
        }
        lua_pushstring(mL, "name");
        lua_rawget(mL, -2);
        if (!lua_isstring(mL, -1)) {
            lua_pop(mL, 5);
            appendWarnLog(QStringLiteral("value of 'name' must be a string"));
            return;
        }
        QString name(lua_tostring(mL, -1));
        if (dataMap.contains(name)) {
            lua_pop(mL, 5);
            appendWarnLog(QStringLiteral("counter name '%1' is duplicate").arg(name));
            return;
        }
        lua_pop(mL, 1);

        lua_pushstring(mL, "values");
        lua_rawget(mL, -2);
        if (!lua_istable(mL, -1)) {
            lua_pop(mL, 5);
            appendWarnLog(QStringLiteral("value of 'values' must be a table"));
            return;
        }

        QCPGraphData gdata;
        CounterData &cdata = dataMap[name];

        for (int i = 0; i < timestamps.size(); ++i) {
            int isNum;
            lua_rawgeti(mL, -1, i + 1);
            double value = lua_tonumberx(mL, -1, &isNum);
            lua_pop(mL, 1);

            gdata.key = timestamps.at(i);
            gdata.value = isNum ? value : NAN;
            cdata.data.add(gdata);
        }

        lua_pop(mL, 2);
    }

    lua_pop(mL, 2);
    if (dataMap.isEmpty()) {
        appendWarnLog(QStringLiteral("the second result table has no counters"));
        return;
    }

    PlotData plotData(mOffsetFromUtc, CounterFileParser::getNodeName(mCounterFilePath));
    plotData.setCounterDataMap(dataMap);
    processPlotData(plotData, false);
}

void MainWindow::caseSensitiveButtonClicked(bool /*checked*/)
{
    mCaseSensitive = !mCaseSensitive;
    updateCaseSensitiveButtonFont();
    updateFilterPattern();
}

void MainWindow::kpikciFileDlgDirEntered(const QString &dir)
{
    FileDialog *dlg = qobject_cast<FileDialog *>(sender());
    int cntXmlGz = 0, cntXml = 0;
    QStringList fileNames = QDir(dir).entryList(QDir::Files);
    for (const QString &fn : qAsConst(fileNames)) {
        if (fn.endsWith(QLatin1String(".xml.gz"), Qt::CaseInsensitive)) {
            ++cntXmlGz;
        } else if (fn.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
            ++cntXml;
        }
    }

    if (cntXmlGz == 0 && cntXml == 0) {
        dlg->selectNameFilter(2);
    } else if (cntXml > cntXmlGz) {
        dlg->selectNameFilter(1);
    } else {
        dlg->selectNameFilter(0);
    }
}

void MainWindow::updateCounterNameCountInfo()
{
    CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
    int total = model->totalCount();
    int matched = model->matchedCount();
    int displayed = model->rowCount();
    mStatusBarLabel->setText(QStringLiteral("Total:%1, Matched:%2, Displayed:%3").arg(total).arg(matched).arg(displayed));
}

void MainWindow::updateFilterPattern()
{
    QVector<QString> moduleNames;
    const auto moduleNameItems = ui->moduleNameView->selectedItems();
    moduleNames.reserve(moduleNameItems.size());
    for (const QListWidgetItem *item : moduleNameItems) {
        moduleNames.append(item->text());
    }

    CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
    QString error = model->setFilterPattern(moduleNames, ui->filterComboBox->lineEdit()->text(), mCaseSensitive);
    if (!error.isEmpty()) {
        appendErrorLog(error);
    }
}

void MainWindow::filterEditReturnPressed()
{
    // If current text of combo box is empty and return key pressed, the "activated" signal
    // will not be sent. So, we have to call updateFilterPattern manually.
    QString curText = ui->filterComboBox->currentText();
    if (curText.isEmpty()) {
        updateFilterPattern();
    } else {
        adjustFilterHistoryOrder();
    }
}

void MainWindow::counterNameViewDoubleClicked(const QModelIndex &index)
{
    QVariant var = ui->counterNameView->model()->data(index);
    ui->filterComboBox->lineEdit()->setText(QRegularExpression::escape(var.toString()));
    updateFilterPattern();
}

void MainWindow::updateModuleNameColor()
{
    bool hasSelection = ui->moduleNameView->selectionModel()->hasSelection();
    for (int i = 0; i < ui->moduleNameView->count(); ++i) {
        ui->moduleNameView->item(i)->setForeground(hasSelection ? Qt::gray : qvariant_cast<QBrush>(QVariant()));
    }
}

void MainWindow::listViewCtxMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QListView *view = qobject_cast<QListView *>(sender());
    menu->addAction(QStringLiteral("Copy Selected"), this, [view](){
        const QModelIndexList indexes = view->selectionModel()->selectedIndexes();
        if (!indexes.isEmpty()) {
            QStringList stringList;
            for (const QModelIndex &index : indexes) {
                QVariant var = view->model()->data(index);
                stringList << var.toString();
            }
            QApplication::clipboard()->setText(stringList.join('\n'));
        }
    });
    menu->addAction(QStringLiteral("Clear Selection"), view, &QListView::clearSelection);
    menu->popup(view->mapToGlobal(pos));
}

void MainWindow::filterMenuFileChanged()
{
    ui->menuFilter->clear();
    loadFilterMenu();
    appendInfoLog(QStringLiteral("finished reloading filter menu"));
}

void MainWindow::downloadCounterDescriptionFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    QFile file(filePath(fpCounterDescription));
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray netData = reply->readAll();
        if (file.open(QIODevice::ReadWrite)) {
            QByteArray fileData = file.readAll();
            if (fileData != netData && file.resize(0)) {
                file.write(netData);
            }
            file.close();

            QSettings settings;
            settings.setValue(SETTING_KEY_CD_UPDATE_TIME, QDateTime::currentDateTime());
        }
    } else {
        appendWarnLog("failed to download counter description file: " + reply->errorString());
    }

    mCounterDesc.load(file.fileName());
}

void MainWindow::checkUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    process->deleteLater();

    if (exitStatus != QProcess::NormalExit) {
        appendWarnLog(QStringLiteral("updater crashed"));
        return;
    }

    appendInfoLog(QStringLiteral("checking for updates finished"));

    // exitCode != 0 indicates that there is no update available
    // or failed to check update
    if (exitCode != 0) {
        return;
    }

    bool showUpdateDlg = QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier;

    if (!showUpdateDlg) {
        int numOfUpdate = 0;
        QByteArray baUpdate = process->readAllStandardOutput();
        QXmlStreamReader xmlReader(baUpdate);
        while (!xmlReader.atEnd()) {
            QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
            if (tokenType != QXmlStreamReader::StartElement) {
                continue;
            }
            if (xmlReader.name() != "update") {
                continue;
            }
            if (++numOfUpdate > 1) {
                showUpdateDlg = true;
                break;
            }
            QXmlStreamAttributes attrs = xmlReader.attributes();
            QStringRef name = attrs.value(QLatin1String("name"));
            if (name != "VisualStatistics") {
                continue;
            }
            QStringRef version = attrs.value(QLatin1String("version"));
            if (version.endsWith(QLatin1String(".0"))) {
                showUpdateDlg = true;
                break;
            }
        }
    }

    if (showUpdateDlg) {
        ChangeLogDialog dlg(this, true);
        if (dlg.exec() == QDialog::Accepted) {
            QProcess::startDetached(filePath(fpMaintenanceTool), QStringList() << "--updater" << "--proxy");
            QApplication::exit();
        }
    }
}

void MainWindow::checkUpdateFailed(QProcess::ProcessError error)
{
    // Don't worry about the deleteLater of QProcess will be called twice in case both finished and
    // errorOccurred signals are emitted. Because deleteLater has below note:
    // It is safe to call this function more than once; when the first deferred deletion event is
    // delivered, any pending events for the object are removed from the event queue.
    sender()->deleteLater();

    appendWarnLog(QStringLiteral("updater failed with error code %1").arg(error));
}

void MainWindow::listOnlinePluginsFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        appendWarnLog("failed to get online plugins: " + reply->errorString());
        return;
    }

    QStringList hrefList;
    QByteArray netData = reply->readAll();
    QXmlStreamReader xmlReader(netData);

    while (!xmlReader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
        if (tokenType != QXmlStreamReader::StartElement) {
            continue;
        }
        if (xmlReader.name() != "a") {
            continue;
        }
        QXmlStreamAttributes attrs = xmlReader.attributes();
        QStringRef href = attrs.value(QLatin1String("href"));
        if (href.isEmpty() || !href.endsWith(QLatin1String(".lua"))) {
            continue;
        }
        hrefList.append(href.toString());
    }

    loadOnlinePlugins(hrefList);
}

void MainWindow::loadOnlinePluginsFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QStringList hrefs = reply->property("hrefs").toStringList();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        appendWarnLog(reply->errorString());
        return;
    }

    QByteArray baPlugin = reply->readAll();
    if (luaL_dostring(mL, baPlugin.constData()) != LUA_OK) {
        QString err = getLastLuaError(mL);
        err += " in ";
        err += hrefs.first();
        appendWarnLog(err);
    } else {
        QString logText("load online plugin ");
        logText += hrefs.first();
        logText += " successfully";
        appendInfoLog(logText);
    }

    hrefs.removeFirst();
    loadOnlinePlugins(hrefs);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->counterNameViewParent && event->type() == QEvent::StatusTip) {
        QStatusTipEvent *statusTipEvent = dynamic_cast<QStatusTipEvent *>(event);
        QString counterDesc = statusTipEvent->tip();
        QStackedLayout *stackedLayout = qobject_cast<QStackedLayout *>(ui->stackedWidget->layout());
        if (counterDesc.isEmpty()) {
            stackedLayout->setCurrentWidget(ui->logTextEdit);
        } else {
            ui->counterDescription->setPlainText(counterDesc);
            stackedLayout->setCurrentWidget(ui->counterDescription);
        }
        return true;
    }
    if (isCaseSensitiveButtonResizeEvent(obj, event)) {
        // The value is from qlineedit_p.cpp
        const int horizontalMargin = 2;

        QPoint pos = ui->filterComboBox->lineEdit()->pos();
        const QSize &size = static_cast<QResizeEvent *>(event)->size();
        const int leftMargin = size.width() - pos.x() - horizontalMargin;
        ui->filterComboBox->lineEdit()->setTextMargins(leftMargin, 0, 0, 0);
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
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() == 1) {
        openCounterFile(urls.first().toLocalFile());
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    emit aboutToBeClosed();

    saveFilterHistory();

    QSettings setting;
    setting.setValue(SETTING_KEY_CASE_SENSITIVE, mCaseSensitive);
}

bool MainWindow::event(QEvent *event)
{
    if (mResizeMan.resizeWidgetFromScreenSize(event, 0.75, 0.75)) {

        int leftWidth = ui->splitterHor->width() * 0.23334;
        int bottomHeight = ui->splitterVer->height() * 0.13141;
        ui->splitterHor->setSizes(QList<int>() << leftWidth << ui->splitterHor->width() - leftWidth - ui->splitterHor->handleWidth());
        ui->splitterVer->setSizes(QList<int>() << ui->splitterVer->height() - ui->splitterVer->handleWidth() - bottomHeight << bottomHeight);
    }
    return QMainWindow::event(event);
}

bool MainWindow::isCaseSensitiveButtonResizeEvent(QObject *obj, QEvent *event) const
{
    return event->type() == QEvent::Resize && ui->filterComboBox->findChild<QToolButton *>() == obj;
}

void MainWindow::setupFilterComboBox()
{
    QSettings settings;
    mCaseSensitive = settings.value(SETTING_KEY_CASE_SENSITIVE, true).toBool();

    QToolButton *toolButton = new QToolButton();
    QFont font = toolButton->font();
    font.setBold(true);
    font.setStrikeOut(!mCaseSensitive);
    toolButton->setFont(font);
    toolButton->setAutoRaise(true);
    toolButton->setText(QStringLiteral("Aa"));
    toolButton->setToolTip(QStringLiteral("Case sensitivity"));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolButton->installEventFilter(this);

    QHBoxLayout *hLayout = new QHBoxLayout(ui->filterComboBox);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(toolButton);
    hLayout->addStretch();

    ui->filterComboBox->lineEdit()->setClearButtonEnabled(true);
    ui->filterComboBox->lineEdit()->setValidator(new FilterValidator(this));
    ui->filterComboBox->lineEdit()->setPlaceholderText(QStringLiteral("regular expression filter"));
    ui->filterComboBox->completer()->setCaseSensitivity(Qt::CaseSensitive);
    ui->filterComboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);

    connect(toolButton, &QToolButton::clicked, this, &MainWindow::caseSensitiveButtonClicked);
    connect(ui->filterComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateFilterPattern);
    connect(ui->filterComboBox->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::filterEditReturnPressed);
    connectClearButtonSignal();
}

void MainWindow::connectClearButtonSignal()
{
    QLineEdit *lineEdit = ui->filterComboBox->lineEdit();
    for (QObject *child : lineEdit->children()) {
        QAction *action = qobject_cast<QAction *>(child);
        // "_q_qlineeditclearaction" is defined in qlineedit.cpp
        if (action && action->objectName() == "_q_qlineeditclearaction") {
            connect(action, &QAction::triggered, this, &MainWindow::updateFilterPattern, Qt::QueuedConnection);
            return;
        }
    }
}

void MainWindow::updateWindowTitle()
{
    QString title(APP_NAME);
    if (!mCounterFilePath.isEmpty()) {
        title += " - ";
        title += QDir::toNativeSeparators(mCounterFilePath);
    }
    setWindowTitle(title);
}

void MainWindow::updateCaseSensitiveButtonFont()
{
    QToolButton *toolButton = ui->filterComboBox->findChild<QToolButton*>();
    QFont font = toolButton->font();
    font.setStrikeOut(!mCaseSensitive);
    toolButton->setFont(font);
}

void MainWindow::initRecentFileActions()
{
    auto fileActions = ui->menuFile->actions();
    QAction *actionSep = fileActions.at(fileActions.size() - 2);

    for (size_t i = 0; i < mRecentFileActions.size(); ++i) {
        mRecentFileActions[i] = new QAction(this);
        mRecentFileActions[i]->setVisible(false);
        connect(mRecentFileActions[i], &QAction::triggered, this, &MainWindow::actionRecentFileTriggered);
        ui->menuFile->insertAction(actionSep, mRecentFileActions[i]);
    }
}

void MainWindow::updateRecentFileActions(const QStringList &recentFiles)
{
    QStringList files;
    if (recentFiles.isEmpty()) {
        QSettings settings;
        files = settings.value(SETTING_KEY_RECENT_FILES).toStringList();
    } else {
        files = recentFiles;
    }

    files.erase(std::remove_if(files.begin(), files.end(), [](const QString &path) {
        return !QFileInfo::exists(path);
    }), files.end());

    int numRecentFiles = qMin(files.size(), static_cast<int>(mRecentFileActions.size()));
    for (int i = 0; i < numRecentFiles; ++i) {
        QString path = files[i];
        QString text = QString::number(i + 1);
        text += ": ";
        text += QFileInfo(path).fileName();

        mRecentFileActions[i]->setText(text);
        mRecentFileActions[i]->setData(path);
        mRecentFileActions[i]->setStatusTip(QDir::toNativeSeparators(path));
        mRecentFileActions[i]->setVisible(true);
    }

    for (int i = numRecentFiles; i < mRecentFileActions.size(); ++i) {
        mRecentFileActions[i]->setVisible(false);
    }
}

void MainWindow::loadFilterMenu()
{
    int numSpaces;
    QStack<int> levelStack;
    QStack<QMenu*> menuStack;
    QString preLine, curLine;
    QTextStream ts;

    QFile file(filePath(fpFilterMenu));
    if (!file.open(QIODevice::ReadOnly)) {
        goto lbReturn;
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
            menuStack.push(ui->menuFilter);
            continue;
        }
        if (numSpaces > levelStack.top()) {
            QMenu *menu = menuStack.top()->addMenu(preLine);
            menuStack.push(menu);
            levelStack.push(numSpaces);
        } else {
            addFilterAction(menuStack.top(), preLine);
            while (levelStack.size() > 1 && numSpaces < levelStack.top()) {
                levelStack.pop();
                menuStack.pop();
            }
        }
        preLine = curLine;
    }
    numSpaces = trimLeadingSpace(preLine);
    if (numSpaces >= 0) {
        addFilterAction(menuStack.top(), preLine);
    }

lbReturn:
    if (!ui->menuFilter->isEmpty()) {
        ui->menuFilter->addSeparator();
    }
    QAction *actionClear = ui->menuFilter->addAction(QStringLiteral("Clear Filter History"), this, &MainWindow::actionClearFilterHistoryTriggered);
    actionClear->setStatusTip(actionClear->text());
    QAction *actionEdit = ui->menuFilter->addAction(QStringLiteral("Edit Filter Menu"), this, &MainWindow::actionEditFilterMenuTriggered);
    actionEdit->setStatusTip(actionEdit->text());
}

void MainWindow::addFilterAction(QMenu *menu, const QString &line)
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

void MainWindow::loadFilterHistory()
{
    QFile histFile(filePath(fpFilterHistory));
    if (histFile.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream ts(&histFile);
        while (ts.readLineInto(&line)) {
            ui->filterComboBox->addItem(line);
        }
        ui->filterComboBox->setCurrentIndex(-1);
    }
}

void MainWindow::saveFilterHistory()
{
    const int maxFilterHistory = 100;

    QFile histFile(filePath(fpFilterHistory));
    if (histFile.open(QIODevice::WriteOnly)) {
        QTextStream ts(&histFile);
        for (int i = 0; i < maxFilterHistory && i < ui->filterComboBox->count(); ++i) {
            QString text = ui->filterComboBox->itemText(i);
            ts << text << endl;
        }
    }
}

void MainWindow::adjustFilterHistoryOrder()
{
    QString currentText = ui->filterComboBox->lineEdit()->text();
    int index = ui->filterComboBox->findText(currentText);
    if (index > 0) {
        ui->filterComboBox->removeItem(index);
        ui->filterComboBox->insertItem(0, currentText);
        ui->filterComboBox->setCurrentIndex(0);
    }
}

static char sKeyMainWindow;

static MainWindow * mainWindow(lua_State *L)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, &sKeyMainWindow);

    MainWindow *mw = static_cast<MainWindow *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return mw;
}

static int print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    mainWindow(L)->appendInfoLog(str);
    return 0;
}

static int registerMenu(lua_State *L)
{
    const char *menuTitle = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    const char *description = luaL_optstring(L, 3, nullptr);

    QAction *action = mainWindow(L)->registerMenu(menuTitle, description);
    lua_pushlightuserdata(L, action);
    lua_pushvalue(L, 2);
    lua_rawset(L, LUA_REGISTRYINDEX);

    return 0;
}

static int parse(lua_State *L)
{
    return mainWindow(L)->parseCounterFileData();
}

static int initLuaEnv(lua_State *L)
{
    luaL_openlibs(L);

    // replace default print function
    lua_pushcfunction(L, print);
    lua_setglobal(L, "print");

    // store MainWindow to registry
    lua_pushlightuserdata(L, &sKeyMainWindow);
    lua_pushvalue(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);

    const struct luaL_Reg apis[] = {
        { "register_menu", registerMenu },
        { "parse", parse },
        {NULL, NULL}
    };

    luaL_newlib(L, apis);
    lua_setglobal(L, "vs");

    return 0;
}

void MainWindow::initLuaEnv()
{
    mL = luaL_newstate();
    if (mL == nullptr) {
        appendWarnLog(QStringLiteral("can't create Lua environment"));
        return;
    }

    lua_pushcfunction(mL, ::initLuaEnv);
    lua_pushlightuserdata(mL, this);

    if (lua_pcall(mL, 1, 0, 0) != LUA_OK) {
        QString err = getLastLuaError(mL);
        appendWarnLog(err);
        return;
    }

    QDir dir(filePath(fpPluginDir));
    dir.setNameFilters({ "*.lua" });

    QStringList luaFiles = dir.entryList(QDir::Files, QDir::Name);
    for (const QString &fn : qAsConst(luaFiles)) {
        std::string path = QDir::toNativeSeparators(dir.absoluteFilePath(fn)).toStdString();
        if (luaL_dofile(mL, path.c_str()) != LUA_OK) {
            QString err = getLastLuaError(mL);
            appendWarnLog(err);
        } else {
            QString logText("load offline plugin ");
            logText += fn;
            logText += " successfully";
            appendInfoLog(logText);
        }
    }
}

void MainWindow::listOnlinePlugins()
{
    QSettings settings;
    bool loadPlugins = settings.value(OptionsDialog::sKeyLoadOnlinePlugins, OptionsDialog::sDefLoadOnlinePlugins).toBool();
    if (!loadPlugins) {
        return;
    }

    Application *app = Application::instance();
    QUrl url = app->getUrl(Application::upPlugins);
    QNetworkRequest request(url);
    QNetworkReply *reply = app->networkAccessManager().get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::listOnlinePluginsFinished);
}

void MainWindow::loadOnlinePlugins(const QStringList &hrefs)
{
    if (hrefs.isEmpty()) {
        return;
    }

    Application *app = Application::instance();
    QUrl url = app->getUrl(Application::upPlugins).resolved(hrefs.first());
    QNetworkRequest request(url);
    QNetworkReply *reply = app->networkAccessManager().get(request);
    reply->setProperty("hrefs", hrefs);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::loadOnlinePluginsFinished);
}

void MainWindow::checkUpdate()
{
    QProcess *process = new QProcess();
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::checkUpdateFinished);
    connect(process, &QProcess::errorOccurred, this, &MainWindow::checkUpdateFailed);
    process->start(filePath(fpMaintenanceTool), QStringList() << "--checkupdates" << "--proxy");
}

void MainWindow::downloadCounterDescription()
{
    QSettings settings;
    QDateTime lastUpdate = settings.value(SETTING_KEY_CD_UPDATE_TIME).toDateTime();
    QDateTime now = QDateTime::currentDateTime();
    if (lastUpdate.isValid() && lastUpdate.daysTo(now) < 30) {
        mCounterDesc.load(filePath(fpCounterDescription));
        return;
    }

    Application *app = Application::instance();
    QUrl url = app->getUrl(Application::upCounterDescription);
    QNetworkRequest request(url);
    QNetworkReply *reply = app->networkAccessManager().get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::downloadCounterDescriptionFinished);
}

void MainWindow::usageReport()
{
    QByteArray hostNameHash = QCryptographicHash::hash(QHostInfo::localHostName().toLatin1(),
                                                       QCryptographicHash::Md5);
    QByteArray postData("host=");
    postData += hostNameHash.toHex();
    postData += "&pt=";
    postData += QSysInfo::productType();

    QByteArray userName = qgetenv("USERNAME");
    if (!userName.isEmpty()) {
        postData += "-";
        postData += userName;
    }

    postData += "&ver=";
    postData += VER_FILEVERSION_STR;

    Application *app = Application::instance();
    QNetworkRequest request(app->getUrl(Application::upUsageReport));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = app->networkAccessManager().post(request, postData);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void MainWindow::openCounterFile(const QString &path)
{
    if (!mCounterFilePath.isEmpty()) {
#if defined(Q_OS_WIN)
        if (mCounterFilePath.compare(path, Qt::CaseInsensitive)) {
#else
        if (mCounterFilePath.compare(path, Qt::CaseSensitive)) {
#endif
            actionCloseFileTriggered();
        } else {
            return;
        }
    }
    if (parseCounterFileHeader(path)) {
        mCounterFilePath = path;
        updateWindowTitle();
        ui->filterComboBox->setFocus();

        QSettings settings;
        QStringList files = settings.value(SETTING_KEY_RECENT_FILES).toStringList();
        files.removeOne(path);
        files.prepend(path);

        while (files.size() > static_cast<int>(mRecentFileActions.size())) {
            files.removeLast();
        }

        settings.setValue(SETTING_KEY_RECENT_FILES, files);
        updateRecentFileActions(files);
    }
}

bool MainWindow::parseCounterFileHeader(const QString &path)
{
    QVector<QString> counterNames;
    CounterFileParser parser(this);
    QString error = parser.parseHeader(path, counterNames);
    if (error.isEmpty()) {
        CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
        mOffsetFromUtc = parser.offsetFromUtc();
        model->setCounterNames(counterNames);
        ui->moduleNameView->addItems(model->moduleNames());

        QString filterText = ui->filterComboBox->lineEdit()->text();
        if (!filterText.isEmpty()) {
            QString error = model->setFilterPattern(QVector<QString>(), filterText, mCaseSensitive);
            if (!error.isEmpty()) {
                appendErrorLog(error);
            }
        }
        return true;
    }
#if defined(Q_OS_WIN)
    MessageBeep(MB_ICONERROR);
#endif
    appendErrorLog(error);
    return false;
}

void MainWindow::parseCounterFileData(bool multiWnd)
{
    if (mCounterFilePath.isEmpty()) {
        return;
    }

    CounterNameModel *model = qobject_cast<CounterNameModel *>(ui->counterNameView->model());
    const QModelIndexList selectedIndexes = ui->counterNameView->selectionModel()->selectedIndexes();
    int numGraphs = selectedIndexes.isEmpty() ? model->rowCount() : selectedIndexes.size();
    if (numGraphs <= 0) {
        return;
    }
    if (multiWnd && numGraphs > 16) {
        int answer = showQuestionMsgBox(this, QStringLiteral("There are %1 windows will be created. Do you want to continue?").arg(numGraphs));
        if (QMessageBox::Yes != answer) {
            return;
        }
    }

    IndexNameMap inm;
    if (selectedIndexes.isEmpty()) {
        for (int i = 0; i < model->rowCount(); ++i) {
            QModelIndex index = model->index(i);
            inm.insert(model->data(index, CounterNameModel::IndexRole).toInt(),
                       model->data(index).toString());
        }
    } else {
        for (const QModelIndex &index : selectedIndexes) {
            inm.insert(model->data(index, CounterNameModel::IndexRole).toInt(),
                       model->data(index).toString());
        }
    }

    bool canceled;
    CounterDataMap dataMap;
    CounterFileParser parser(this);
    QString error = parser.parseData(mCounterFilePath, inm, dataMap, canceled);
    if (!error.isEmpty()) {
#if defined(Q_OS_WIN)
        MessageBeep(MB_ICONERROR);
#endif
        appendErrorLog(error);
    } else if (!canceled) {
        PlotData plotData(mOffsetFromUtc, CounterFileParser::getNodeName(mCounterFilePath));
        plotData.setCounterDataMap(dataMap);
        processPlotData(plotData, multiWnd);
    }
}

void MainWindow::processPlotData(PlotData &plotData, bool multiWnd)
{
    if (multiWnd && plotData.counterCount() > 1) {
        QSettings setting;
        bool ignoreConstant = setting.value(OptionsDialog::sKeyIgnoreConstant, OptionsDialog::sDefIgnoreConstant).toBool();
        std::unique_ptr<PlotData[]> plotDataPtr = plotData.split();
        for (int i = 0; i < plotData.counterCount(); ++i) {
            if (ignoreConstant && CounterData::isConstant(plotDataPtr[i].firstCounterData())) {
                appendInfoLog("ignored constant counter " + plotDataPtr[i].firstCounterName());
                continue;
            }
            processPlotData(plotDataPtr[i], false);
        }
    } else {
        PlotWindow *plotWnd = createPlotWindow(plotData);
        plotWnd->showMaximized();
    }
}

PlotWindow *MainWindow::createPlotWindow(PlotData &plotData)
{
    PlotWindow *plotWnd = new PlotWindow(plotData);
    plotWnd->setAttribute(Qt::WA_DeleteOnClose);
    plotWnd->setCounterDescription(&mCounterDesc);
    plotWnd->resize((mResizeMan.screenSize() * 0.75).toSize());
    connect(this, &MainWindow::aboutToBeClosed, plotWnd, &PlotWindow::close);
    return plotWnd;
}

QString MainWindow::formatLog(const QString &text, LogLevel level)
{
    QString result("<font color='#808080'>");
    result += QDateTime::currentDateTime().toString(mLogDateTimeFmt);
    switch (level) {
    case llInfo:
        result += "</font>  <font color='#13a10e'>INFO</font>: ";
        break;
    case llWarn:
        result += "</font>  <font color='#c19c00'>WARN</font>: ";
        break;
    case llError:
        result += "</font> <font color='#c50f1f'>ERROR</font>: ";
        break;
    }
    result += text;
    return result;
}

QString MainWindow::filePath(FilePath fp)
{
    QDir dir = QDir::home();

    switch (fp) {
    case fpFilterMenu:
        return dir.filePath(QStringLiteral(".vstat_filter_menu.txt"));
    case fpFilterHistory:
        return dir.filePath(QStringLiteral(".vstat_filter_hist"));
    case fpCounterDescription:
        return dir.filePath(QStringLiteral(".vstat_counter_desc"));
    case fpMaintenanceTool:
        dir.setPath(QApplication::applicationDirPath());
#if defined(Q_OS_WIN)
        return dir.absoluteFilePath(QStringLiteral("maintenancetool.exe"));
#else
        return dir.absoluteFilePath(QStringLiteral("maintenancetool"));
#endif
    case fpPluginDir:
        return dir.filePath(QStringLiteral("VisualStatisticsPlugin"));
    }

    return QString();
}

int MainWindow::trimLeadingSpace(QString &str)
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
