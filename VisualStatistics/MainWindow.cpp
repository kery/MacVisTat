#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PlotWindow.h"
#include "CounterNameModel.h"
#include "CounterFileParser.h"
#include "Utils.h"
#include <QNetworkProxyQuery>

#define SETTING_KEY_RECENT_FILES   "recentFileList"
#define SETTING_KEY_CASE_SENSITIVE "caseSensitive"
#define SETTING_KEY_HIDE_TIME_GAP  "hideTimeGap"

MainWindow::MainWindow() :
    ui(new Ui::MainWindow),
    mOffsetFromUtc(0),
    mCntNameInfoLabel(new QLabel(this)),
    mLogDateTimeFmt(DTFMT_DISPLAY),
    mResizeMan(this)
{
    ui->setupUi(this);
    setupFilterComboBox();
    setupNetworkAccessManager();
    updateWindowTitle();

    ui->splitterHor->setStretchFactor(0, 0);
    ui->splitterHor->setStretchFactor(1, 1);
    ui->splitterVer->setStretchFactor(0, 1);
    ui->splitterVer->setStretchFactor(1, 0);

    CounterNameModel *model = new CounterNameModel(this);
    ui->counterNameView->setModel(model);
    ui->statusBar->addPermanentWidget(mCntNameInfoLabel);
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
    connect(ui->logTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::logTextEditCtxMenuRequest);
    connect(model, &CounterNameModel::rowsInserted, this, &MainWindow::updateCounterNameCountInfo);
    connect(model, &CounterNameModel::modelReset, this, &MainWindow::updateCounterNameCountInfo);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpenTriggered);
    connect(ui->actionXmlToCsv, &QAction::triggered, this, &MainWindow::actionXmlToCsvTriggered);
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::actionCloseTriggered);
    connect(ui->actionPlot, &QAction::triggered, this, &MainWindow::actionPlotTriggered);
    connect(ui->actionPlotSeparately, &QAction::triggered, this, &MainWindow::actionPlotSeparatelyTriggered);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::actionHelpTriggered);
    connect(ui->actionChangeLog, &QAction::triggered, this, &MainWindow::actionChangeLogTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAboutTriggered);
    connect(&mFilterFileWatcher, &QFileSystemWatcher::fileChanged, &mFilterMenuReloadTimer, QOverload<>::of(&QTimer::start));
    connect(&mFilterMenuReloadTimer, &QTimer::timeout, this, &MainWindow::filterFileChanged);

    initRecentFileActions();
    updateRecentFileActions(QStringList());
    loadFavoriteFilterMenu();
    loadFilterHistory();

#ifdef DEPLOY_VISUALSTAT
    startCheckUpdateTask();
    startUsageReport();
#endif
    startFetchCounterDescriptionTask();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actionOpenTriggered()
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilter(QStringLiteral("Counter File (*.csv *.csv.gz)"));

    if (dlg.exec() == QDialog::Accepted) {
        auto files = dlg.selectedFiles();
        openCounterFile(files.first());
    }
}

void MainWindow::actionXmlToCsvTriggered()
{
    // TODO
}

void MainWindow::actionCloseTriggered()
{
    mCounterFilePath.clear();
    updateWindowTitle();
    ui->moduleNameView->clear();
    CounterNameModel *model = qobject_cast<CounterNameModel*>(ui->counterNameView->model());
    model->clear();
}

void MainWindow::actionRecentFileTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString path = action->data().toString();
    openCounterFile(path);
}

void MainWindow::actionFilterTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
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
    int answer = showQuestionMsgBox(this, QStringLiteral("Do you want to clear filter history?"), QString(), false);
    if (answer == QMessageBox::Yes) {
        ui->filterComboBox->clear();
    }
}

void MainWindow::actionEditFilterFileTriggered()
{
    QString path = filePath(fpFavoriteFilter);
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
    if (mFilterFileWatcher.files().isEmpty()) {
        mFilterFileWatcher.addPath(path);
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

void MainWindow::actionHelpTriggered()
{
    QDesktopServices::openUrl(url(upHelp));
}

void MainWindow::actionChangeLogTriggered()
{
    // TODO
}

void MainWindow::actionAboutTriggered()
{
    // TODO
}

void MainWindow::caseSensitiveButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    mCaseSensitive = !mCaseSensitive;
    updateCaseSensitiveButtonFont();
    updateFilterPattern();
}

void MainWindow::updateCounterNameCountInfo()
{
    CounterNameModel *model = qobject_cast<CounterNameModel*>(ui->counterNameView->model());
    int total = model->totalCount();
    int matched = model->matchedCount();
    int displayed = model->rowCount();
    mCntNameInfoLabel->setText(QStringLiteral("Counter:%1 Matched:%2 Displayed:%3").arg(total).arg(matched).arg(displayed));
}

void MainWindow::updateFilterPattern()
{
    QVector<QString> moduleNames;
    const auto moduleNameItems = ui->moduleNameView->selectedItems();
    moduleNames.reserve(moduleNameItems.size());
    for (const QListWidgetItem *item : moduleNameItems) {
        moduleNames.append(item->text());
    }

    CounterNameModel *model = qobject_cast<CounterNameModel*>(ui->counterNameView->model());
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

void MainWindow::logTextEditCtxMenuRequest(const QPoint &pos)
{
    QMenu *menu = ui->logTextEdit->createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Clear"), ui->logTextEdit, &QPlainTextEdit::clear);
    menu->popup(ui->logTextEdit->mapToGlobal(pos));
}

void MainWindow::listViewCtxMenuRequest(const QPoint &pos)
{
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QListView *view = qobject_cast<QListView*>(sender());
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

void MainWindow::filterFileChanged()
{
    ui->menuFilter->clear();
    loadFavoriteFilterMenu();
    appendInfoLog(QStringLiteral("finished reloading favorite filters"));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isCaseSensitiveButtonResizeEvent(obj, event)) {
        // The value is from qlineedit_p.cpp
        const int horizontalMargin = 2;

        QPoint pos = ui->filterComboBox->lineEdit()->pos();
        const QSize &size = static_cast<QResizeEvent*>(event)->size();
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
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
        QAction *action = qobject_cast<QAction*>(child);
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
        title += mCounterFilePath;
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

void MainWindow::loadFavoriteFilterMenu()
{
    int numSpaces;
    QStack<int> levelStack;
    QStack<QMenu*> menuStack;
    QString preLine, curLine;
    QTextStream ts;

    QFile file(filePath(fpFavoriteFilter));
    if (!file.open(QIODevice::ReadOnly)) {
        goto _return_;
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

_return_:
    if (!ui->menuFilter->isEmpty()) {
        ui->menuFilter->addSeparator();
    }
    ui->menuFilter->addAction(QStringLiteral("Clear Filter History"), this, &MainWindow::actionClearFilterHistoryTriggered);
    QAction *action = ui->menuFilter->addAction(QStringLiteral("Edit Favorite Filters"), this, &MainWindow::actionEditFilterFileTriggered);
    action->setStatusTip(QStringLiteral("Open favorite filter file for edit"));
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

void MainWindow::setupNetworkAccessManager()
{
    QNetworkProxyQuery npq(url(upRoot));
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (!proxies.isEmpty()) {
        mNetMan.setProxy(proxies[0]);
    }
}

void MainWindow::startCheckUpdateTask()
{
    // TODO
}

void MainWindow::startFetchCounterDescriptionTask()
{
    // TODO
}

void MainWindow::startUsageReport()
{
    // TODO
}

void MainWindow::openCounterFile(const QString &path)
{
    QString nativePath = QDir::toNativeSeparators(path);
    if (!mCounterFilePath.isEmpty()) {
        if (mCounterFilePath.compare(nativePath, Qt::CaseInsensitive)) {
            actionCloseTriggered();
        } else {
            return;
        }
    }
    if (parseCounterFileHeader(path)) {
        mCounterFilePath = nativePath;
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
    QString error = parser.parseHeader(path, counterNames, mOffsetFromUtc);
    if (error.isEmpty()) {
        CounterNameModel *model = qobject_cast<CounterNameModel*>(ui->counterNameView->model());
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
    appendErrorLog(error);
    return false;
}

void MainWindow::parseCounterFileData(bool multiWnd)
{
    if (mCounterFilePath.isEmpty()) {
        return;
    }

    CounterNameModel *model = qobject_cast<CounterNameModel*>(ui->counterNameView->model());
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

    CounterFileParser::IndexNameMap inm;
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

    CounterFileParser parser(this);
    CounterDataMap dataMap;
    QString error = parser.parseData(mCounterFilePath, inm, dataMap);
    if (error.isEmpty()) {
        QSettings setting;
        PlotData::KeyType keyType = PlotData::ktDateTime;
        if (setting.value(SETTING_KEY_HIDE_TIME_GAP, false).toBool()) {
            keyType = PlotData::ktIndex;
        }
        PlotData plotData(mOffsetFromUtc);
        plotData.setCounterDataMap(keyType, dataMap);
        processPlotData(plotData, multiWnd);
    } else {
        appendErrorLog(error);
    }
}

void MainWindow::processPlotData(PlotData &plotData, bool multiWnd)
{
    PlotWindow *plotWnd = createPlotWindow(plotData);
    plotWnd->showMaximized();
}

PlotWindow *MainWindow::createPlotWindow(PlotData &plotData) const
{
    PlotWindow *plotWnd = new PlotWindow(plotData);
    plotWnd->setAttribute(Qt::WA_DeleteOnClose);
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

QUrl MainWindow::url(UrlPath up)
{
    QUrl root(QStringLiteral("http://sdu.int.nokia-sbell.com:4099/"));

    switch (up) {
    case upHelp:
        return root.resolved(QStringLiteral("/help.html"));
    case upRoot:
        break;
    }
    return root;
}

QString MainWindow::filePath(FilePath fp)
{
    QDir home = QDir::home();

    switch (fp) {
    case fpFavoriteFilter:
        return home.filePath(QStringLiteral(".vstat_filter_favorite.txt"));
    case fpFilterHistory:
        return home.filePath(QStringLiteral(".vstat_filter_hist"));
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
