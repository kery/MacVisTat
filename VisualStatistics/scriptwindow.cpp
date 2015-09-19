#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "utils.h"

ScriptWindow::ScriptWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::ScriptWindow)
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << height() - 120 << 120);
    _ui->splitter->setCollapsible(0, false);
}

ScriptWindow::~ScriptWindow()
{
    delete _ui;
}

bool ScriptWindow::initialize(QCustomPlot *plot, QString &err)
{
    if (_luaEnv.initialize(plot, err)) {
        _luaEnv.setPrintLogEdit(_ui->logTextEdit);
        return true;
    }
    return false;
}

void ScriptWindow::setDateTimeVec(void *vec)
{
    _luaEnv.setDateTimeVector(vec);
}

void ScriptWindow::on_actionRun_triggered()
{
    QString err;
    QString scriptStr = _ui->scriptTextEdit->toPlainText();
    if (scriptStr.isEmpty()) {
        if (!_scriptFile.isEmpty()) {
            if (!_luaEnv.doFile(_scriptFile, err)) {
                _ui->logTextEdit->appendPlainText(err);
            }
        }
    } else {
        if (!_luaEnv.doString(scriptStr, err)) {
            _ui->logTextEdit->appendPlainText(err);
        }
    }
}

void ScriptWindow::on_actionClearLog_triggered()
{
    _ui->logTextEdit->clear();
}

void ScriptWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(QStringLiteral("Lua File (*.lua)"));
    fileDialog.setDirectory(getDocumentDir());

    if (fileDialog.exec() == QDialog::Accepted) {
        if (fileDialog.selectedFiles().size() > 0) {
            _scriptFile = QDir::toNativeSeparators(fileDialog.selectedFiles().at(0));
            setWindowTitle(QStringLiteral("Script Window - ") + _scriptFile);
        }
    }
}
