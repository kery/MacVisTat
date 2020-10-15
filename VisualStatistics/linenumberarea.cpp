#include "linenumberarea.h"

LineNumberArea::LineNumberArea(LuaCodeEdit *edit) :
    QWidget(edit),
    m_codeEdit(edit)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEdit->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_codeEdit->lineNumberAreaPaintEvent(event);
}
