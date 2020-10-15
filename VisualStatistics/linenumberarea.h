#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

#include "luacodeedit.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(LuaCodeEdit *edit);

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    LuaCodeEdit *m_codeEdit;
};

#endif // LINENUMBERAREA_H
