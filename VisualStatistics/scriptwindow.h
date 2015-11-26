#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QMainWindow>
#include "luaenvironment.h"

namespace Ui {
class ScriptWindow;
}

class ScriptWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScriptWindow(QWidget *parent);
    ScriptWindow(const ScriptWindow &) = delete;
    ScriptWindow& operator=(const ScriptWindow &) = delete;
    ~ScriptWindow();

    bool initialize(QString &err);
    void appendLog(const QString &text);

private slots:
    void on_actionRun_triggered();

    void on_actionClearLog_triggered();

    void on_actionOpen_triggered();

private:
    Ui::ScriptWindow *m_ui;
    LuaEnvironment m_luaEnv;
    QString m_scriptFile;
};

#endif // SCRIPTWINDOW_H
