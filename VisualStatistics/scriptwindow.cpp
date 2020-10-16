#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "utils.h"

#include <Qsci/qscilexerlua.h>

ScriptWindow::ScriptWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::ScriptWindow)
{
    m_ui->setupUi(this);

    setupEditor(m_ui->scriptTextEdit);

    m_ui->splitter->setSizes(QList<int>() << height() - 140 << 140);
    m_ui->splitter->setCollapsible(0, false);
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

void ScriptWindow::setupEditor(QsciScintilla *editor)
{
    QsciLexerLua *lexer = new QsciLexerLua(editor);
    lexer->setColor(Qt::blue, QsciLexerLua::Keyword);

    editor->setLexer(lexer);
    editor->setMargins(1);
    editor->setMarginsForegroundColor(Qt::gray);
    editor->setMarginsBackgroundColor(editor->palette().color(QPalette::Base));
    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginLineNumbers(0, true);
    editor->setTabWidth(4);
    editor->setWrapMode(QsciScintilla::WrapWord);
    // Set the left margin of editor's text.
    editor->SendScintilla(QsciScintilla::SCI_SETMARGINLEFT, 0, 16);

    connect(editor, &QsciScintilla::linesChanged, this, &ScriptWindow::updateLineNumberMarginWidth);

    updateLineNumberMarginWidth();
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

void ScriptWindow::on_actionRun_triggered()
{
    QString err;
    QString scriptStr = m_ui->scriptTextEdit->text();
    if (scriptStr.isEmpty()) {
        if (!m_scriptFile.isEmpty()) {
            if (!m_luaEnv.doFile(m_scriptFile, err)) {
                m_ui->logTextEdit->appendPlainText(err);
            }
        }
    } else {
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
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(QStringLiteral("Lua File (*.lua)"));

    if (fileDialog.exec() == QDialog::Accepted) {
        if (fileDialog.selectedFiles().size() > 0) {
            m_scriptFile = QDir::toNativeSeparators(fileDialog.selectedFiles().at(0));
            setWindowTitle(QStringLiteral("Script Window - ") + m_scriptFile);
        }
    }
}
