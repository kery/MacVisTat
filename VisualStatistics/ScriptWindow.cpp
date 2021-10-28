#include "ScriptWindow.h"
#include "ui_ScriptWindow.h"
#include "Utils.h"
#include "AutoCompletionSrcPlotAPIs.h"
#include "SciLexerLua5_2.h"

ScriptWindow::ScriptWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::ScriptWindow)
{
    m_ui->setupUi(this);
    m_ui->splitter->setSizes(QList<int>() << height() - 140 << 140);
    m_ui->splitter->setCollapsible(0, false);

    setupEditor(m_ui->scriptTextEdit);

    setWindowTitle(QStringLiteral("Script Window - new.lua[*]"));
}

ScriptWindow::~ScriptWindow()
{
    delete m_ui;
}

bool ScriptWindow::initialize(QString &err)
{
    return m_luaEnv.initialize(this, err);
}

void ScriptWindow::appendLog(const QString &text)
{
    m_ui->logTextEdit->appendPlainText(text);
}

void ScriptWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
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
    if (m_ui->scriptTextEdit->isModified()) {
        int answer = QMessageBox::question(
                    this, QStringLiteral("VisualStatistics"),
                    QStringLiteral("The script has been modified. Do you want to save your changes?"),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape);
        if (answer == QMessageBox::Yes) {
            if (m_scriptFile.isEmpty()) {
                return saveAs();
            } else {
                return saveFile(m_scriptFile);
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
        showErrorMsgBox(this, QString("Cannot read file: %1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_ui->scriptTextEdit->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(path);
    m_ui->scriptTextEdit->setModified(false);
}

bool ScriptWindow::saveFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        showErrorMsgBox(this, QString("Cannot write file: %1").arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << m_ui->scriptTextEdit->text();
    QApplication::restoreOverrideCursor();
    m_ui->scriptTextEdit->setModified(false);

    return true;
}

bool ScriptWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(this);
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
    m_scriptFile = path;
    setWindowTitle(QString("Script Window - %1[*]").arg(QDir::toNativeSeparators(path)));
}

void ScriptWindow::updateLineNumberMarginWidth()
{
    int digits = 1;
    QsciScintilla *editor = m_ui->scriptTextEdit;
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

void ScriptWindow::on_actionRun_triggered()
{
    QString err;
    QString scriptStr = m_ui->scriptTextEdit->text();
    if (!scriptStr.isEmpty()) {
        if (!m_luaEnv.doString(scriptStr, err)) {
            m_ui->logTextEdit->appendPlainText(err);
        }
    }
}

void ScriptWindow::on_actionClearLog_triggered()
{
    m_ui->logTextEdit->clear();
}

void ScriptWindow::on_actionOpen_triggered()
{
    if (!maybeSave()) {
        return;
    }

    QString path = QFileDialog::getOpenFileName(
                this, QStringLiteral("Open File"), QString(), QStringLiteral("Lua File (*.lua)"));
    if (path.isEmpty()) {
        return;
    }

    loadFile(path);
}

void ScriptWindow::on_actionSave_triggered()
{
    if (!m_ui->scriptTextEdit->isModified()) {
        return;
    }

    if (m_scriptFile.isEmpty()) {
        saveAs();
    } else {
        saveFile(m_scriptFile);
    }
}
