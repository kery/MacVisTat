#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QMainWindow>
#include <Qsci/qsciscintilla.h>

#include "LuaEnvironment.h"
#include "ResizeManager.h"

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

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

    void setupEditor(QsciScintilla *editor);
    void setupAutoCompletion(QsciScintilla *editor);
    bool maybeSave();
    void loadFile(const QString &path);
    bool saveFile(const QString &path);
    bool saveAs();
    void setCurrentFile(const QString &path);

private slots:
    void updateLineNumberMarginWidth();
    void updateModificationIndicator(bool m);

    void actionRunTriggered();
    void actionClearLogTriggered();
    void actionOpenTriggered();
    void actionSaveTriggered();

private:
    Ui::ScriptWindow *m_ui;
    LuaEnvironment m_luaEnv;
    QString m_scriptFile;
    ResizeManager m_resizeMan;
};

#endif // SCRIPTWINDOW_H
