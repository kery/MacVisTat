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

void ScriptWindow::on_actionExecute_triggered()
{
    QString err;
    if (!_luaEnv.doString(_ui->scriptTextEdit->toPlainText(), err)) {
        _ui->logTextEdit->appendPlainText(err);
    }
}

void ScriptWindow::on_actionClearLog_triggered()
{
    _ui->logTextEdit->clear();
}

void ScriptWindow::on_actionLoadScript_triggered()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(QStringLiteral("Lua File (*.lua)"));
    fileDialog.setDirectory(getDocumentDir());

    if (fileDialog.exec() == QDialog::Accepted) {
        if (fileDialog.selectedFiles().size() > 0) {
            QFile file(fileDialog.selectedFiles().at(0));
            if (file.open(QFile::ReadOnly)) {
                QTextStream in(&file);
                _ui->scriptTextEdit->clear();
                while (!in.atEnd()) {
                    _ui->scriptTextEdit->appendPlainText(in.readLine());
                }
                file.close();
            } else {
                showErrorMsgBox(this, QStringLiteral("Open file failed."));
            }
        }
    }
}
