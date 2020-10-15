#ifndef LUACODEEDIT_H
#define LUACODEEDIT_H

#include <QPlainTextEdit>

class LuaCodeEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    LuaCodeEdit(QWidget *parent);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *m_lineNumberArea;
};

#endif // LUACODEEDIT_H
