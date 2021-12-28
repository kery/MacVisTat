#include "ScriptWindow.h"
#include "ui_ScriptWindow.h"
#include "FileDialog.h"
#include "AutoCompletionSrcPlotAPIs.h"
#include "SciLexerLua5_2.h"
#include "GlobalDefines.h"
#include "Utils.h"
#include <QTextStream>
#include <QFileDialog>

ScriptWindow::ScriptWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ScriptWindow),
    mResizeMan(this)
{
    ui->setupUi(this);
    setupEditor(ui->scriptTextEdit);
    ui->splitter->setCollapsible(0, false);

    connect(ui->actionRun, &QAction::triggered, this, &ScriptWindow::actionRunTriggered);
    connect(ui->actionOpen, &QAction::triggered, this, &ScriptWindow::actionOpenTriggered);
    connect(ui->actionSave, &QAction::triggered, this, &ScriptWindow::actionSaveTriggered);

    setWindowTitle(QStringLiteral("Script - new.lua[*]"));
}

ScriptWindow::~ScriptWindow()
{
    delete ui;
}

QString ScriptWindow::initialize()
{
    return mLuaEnv.initialize(this);
}

void ScriptWindow::appendLog(const QString &text)
{
    ui->logTextEdit->appendPlainText(text);
}

void ScriptWindow::actionRunTriggered()
{
    QString scriptStr = ui->scriptTextEdit->text();
    if (scriptStr.isEmpty()) { return; }

    QString err = mLuaEnv.doString(scriptStr);
    if (!err.isEmpty()) {
        ui->logTextEdit->appendPlainText(err);
    }
}

void ScriptWindow::actionOpenTriggered()
{
    if (!maybeSave()) {
        return;
    }

    QString path = FileDialog::getOpenFileName(this, QStringLiteral("Lua File (*.lua)"));
    if (path.isEmpty()) {
        return;
    }

    loadFile(path);
}

void ScriptWindow::actionSaveTriggered()
{
    if (!ui->scriptTextEdit->isModified()) {
        return;
    }

    if (mScriptFile.isEmpty()) {
        saveAs();
    } else {
        saveFile(mScriptFile);
    }
}

void ScriptWindow::updateLineNumberMarginWidth()
{
    int digits = 1;
    QsciScintilla *editor = ui->scriptTextEdit;
    int numLines = editor->lines();

    while (numLines >= 10) {
        numLines /= 10;
        ++digits;
    }

    editor->setMarginWidth(0, QString(digits + 1, QLatin1Char('9')));
}

void ScriptWindow::updateModificationIndicator(bool m)
{
    setWindowModified(m);
}

void ScriptWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool ScriptWindow::event(QEvent *event)
{
    if (mResizeMan.resizeWidgetFromScreenHeight(event, 0.7, 1)) {
        int topHeight = ui->splitter->height() * 0.85;
        ui->splitter->setSizes(QList<int>() << topHeight << ui->splitter->height() - topHeight - ui->splitter->handleWidth());
    }
    return QMainWindow::event(event);
}

void ScriptWindow::setupEditor(QsciScintilla *editor)
{
    QsciLexer *lexer = new SciLexerLua5_2(editor);
    lexer->setColor(Qt::blue, SciLexerLua5_2::Keyword);
    lexer->setColor(QColor(170, 85, 255), SciLexerLua5_2::BasicFunctions);
    lexer->setColor(lexer->color(SciLexerLua5_2::BasicFunctions), SciLexerLua5_2::StringTableMathsFunctions);
    lexer->setColor(lexer->color(SciLexerLua5_2::BasicFunctions), SciLexerLua5_2::CoroutinesIOSystemFacilities);
    lexer->setColor(lexer->color(SciLexerLua5_2::BasicFunctions), SciLexerLua5_2::Bit32DebugPackageFunctions);
    lexer->setColor(Qt::darkMagenta, SciLexerLua5_2::PlotFunctions);

    editor->setLexer(lexer);
    editor->setMargins(1);
    editor->setMarginsForegroundColor(Qt::gray);
    editor->setMarginsBackgroundColor(editor->palette().color(QPalette::Base));
    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginLineNumbers(0, true);
    editor->setWrapMode(QsciScintilla::WrapWord);
    // When inserting a new line, automatic indentation pushes the cursor to the same indentation level as the previous one.
    editor->setAutoIndent(true);
    editor->setTabWidth(4);
    editor->setIndentationsUseTabs(false);
    editor->setBackspaceUnindents(true);
    editor->setBraceMatching(QsciScintilla::StrictBraceMatch);
    // Set the matching braces' font weight to bold.
    editor->SendScintilla(QsciScintilla::SCI_STYLESETBOLD, QsciScintilla::STYLE_BRACELIGHT, true);
    // Set the left margin of editor's text.
    editor->SendScintilla(QsciScintilla::SCI_SETMARGINLEFT, 0, 16);

    setupAutoCompletion(editor);

    connect(editor, &QsciScintilla::linesChanged, this, &ScriptWindow::updateLineNumberMarginWidth);
    connect(editor, &QsciScintilla::modificationChanged, this, &ScriptWindow::updateModificationIndicator);

    updateLineNumberMarginWidth();
}

void ScriptWindow::setupAutoCompletion(QsciScintilla *editor)
{
    new AutoCompletionSrcPlotAPIs(editor->lexer());

    editor->setAutoCompletionSource(QsciScintilla::AcsAll);
    editor->setAutoCompletionThreshold(1);
}

bool ScriptWindow::maybeSave()
{
    if (ui->scriptTextEdit->isModified()) {
        int answer = showQuestionMsgBox(this, QStringLiteral("The script has been modified. Do you want to save your changes?"),
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        if (answer == QMessageBox::Yes) {
            if (mScriptFile.isEmpty()) {
                return saveAs();
            } else {
                return saveFile(mScriptFile);
            }
        } else if (answer == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void ScriptWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        showErrorMsgBox(this, QStringLiteral("Failed to read file!"), file.errorString());
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->scriptTextEdit->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(path);
    ui->scriptTextEdit->setModified(false);
}

bool ScriptWindow::saveFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        showErrorMsgBox(this, QString("Failed to write file!"), file.errorString());
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << ui->scriptTextEdit->text();
    QApplication::restoreOverrideCursor();
    ui->scriptTextEdit->setModified(false);

    return true;
}

bool ScriptWindow::saveAs()
{
    QString path = FileDialog::getSaveFileName(this, QString(), QStringLiteral("Lua File (*.lua)"));
    if (path.isEmpty()) {
        return false;
    }

    bool ret = saveFile(path);
    if (ret) {
        setCurrentFile(path);
    }
    return ret;
}

void ScriptWindow::setCurrentFile(const QString &path)
{
    mScriptFile = path;
    setWindowTitle(QStringLiteral("Script - %1[*]").arg(QDir::toNativeSeparators(path)));
}
