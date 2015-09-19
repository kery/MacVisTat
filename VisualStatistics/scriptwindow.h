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
    explicit ScriptWindow(QWidget *parent = 0);
    ~ScriptWindow();

    bool initialize(QCustomPlot *plot, QString &err);
    void setDateTimeVec(void *vec);

private slots:
    void on_actionRun_triggered();

    void on_actionClearLog_triggered();

    void on_actionOpen_triggered();

private:
    Ui::ScriptWindow *_ui;
    LuaEnvironment _luaEnv;
    QString _scriptFile;
};

#endif // SCRIPTWINDOW_H
