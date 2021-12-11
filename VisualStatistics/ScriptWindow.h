#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QMainWindow>
#include <Qsci/qsciscintilla.h>
#include "LuaEnvironment.h"
#include "ResizeManager.h"

namespace Ui { class ScriptWindow; }

class ScriptWindow : public QMainWindow
{
    Q_OBJECT

public:
    ScriptWindow(QWidget *parent);
    ~ScriptWindow();

    QString initialize();
    void appendLog(const QString &text);

private slots:
    void actionRunTriggered();
    void actionClearLogTriggered();
    void actionOpenTriggered();
    void actionSaveTriggered();

    void updateLineNumberMarginWidth();
    void updateModificationIndicator(bool m);

private:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool event(QEvent *event) override;

    void setupEditor(QsciScintilla *editor);
    void setupAutoCompletion(QsciScintilla *editor);
    bool maybeSave();
    void loadFile(const QString &path);
    bool saveFile(const QString &path);
    bool saveAs();
    void setCurrentFile(const QString &path);

    Ui::ScriptWindow *ui;
    LuaEnvironment mLuaEnv;
    QString mScriptFile;
    ResizeManager mResizeMan;
};

#endif // SCRIPTWINDOW_H
