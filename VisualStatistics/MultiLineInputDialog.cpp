#include "MultiLineInputDialog.h"
#include "Utils.h"
#include <QEvent>

MultiLineInputDialog::MultiLineInputDialog(QWidget *parent) :
    QInputDialog(parent, Qt::Window | Qt::WindowCloseButtonHint),
    mResizeMan(this)
{
    setOptions(QInputDialog::UsePlainTextEditForTextInput);
    setWindowTitle(APP_NAME);
    setInputMethodHints(Qt::ImhNone);
    setSizeGripEnabled(true);
}

bool MultiLineInputDialog::event(QEvent *event)
{
    mResizeMan.resizeWidgetFromCharWidth(event, 145, 0.31256);
    return QInputDialog::event(event);
}
