#include "LogTextEdit.h"
#include <QMenu>

LogTextEdit::LogTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setFont(QFont(QStringLiteral("Consolas"), 10));
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::NoWrap);
}

void LogTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Clear"), this, &LogTextEdit::clear);
    menu->popup(event->globalPos());
}
