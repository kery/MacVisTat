#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    LogTextEdit(QWidget *parent);

private:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // LOGTEXTEDIT_H
