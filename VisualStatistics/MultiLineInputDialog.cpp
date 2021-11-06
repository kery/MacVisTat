#include "MultiLineInputDialog.h"

#include <QEvent>

MultiLineInputDialog::MultiLineInputDialog(QWidget *parent) :
    QInputDialog(parent, Qt::Window | Qt::WindowCloseButtonHint),
    m_resizeMan(this)
{
    setOptions(QInputDialog::UsePlainTextEditForTextInput);
    setWindowTitle(QStringLiteral("Visual Statistics"));
    setInputMethodHints(Qt::ImhNone);
    setSizeGripEnabled(true);
}

bool MultiLineInputDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShowToParent && !m_resizeMan.showToParentHandled()) {
        m_resizeMan.resizeWidgetOnShowToParent();
    }
    return QInputDialog::event(event);
}
