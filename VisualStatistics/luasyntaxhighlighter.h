#ifndef LUASYNTAXHIGHLIGHTER_H
#define LUASYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class LuaSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LuaSyntaxHighlighter(QTextDocument *parent);

protected:
    virtual void highlightBlock(const QString &text);

private:
    struct HighlightRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> m_highlightRules;
};

#endif // LUASYNTAXHIGHLIGHTER_H
