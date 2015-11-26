#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "utils.h"

ScriptWindow::ScriptWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::ScriptWindow)
{
    m_ui->setupUi(this);

    m_ui->splitter->setSizes(QList<int>() << height() - 120 << 120);
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

void ScriptWindow::on_actionRun_triggered()
{
    QString err;
    QString scriptStr = m_ui->scriptTextEdit->toPlainText();
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
    fileDialog.setDirectory(getDocumentDir());

    if (fileDialog.exec() == QDialog::Accepted) {
        if (fileDialog.selectedFiles().size() > 0) {
            m_scriptFile = QDir::toNativeSeparators(fileDialog.selectedFiles().at(0));
            setWindowTitle(QStringLiteral("Script Window - ") + m_scriptFile);
        }
    }
}
