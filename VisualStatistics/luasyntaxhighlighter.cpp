#include "luasyntaxhighlighter.h"

LuaSyntaxHighlighter::LuaSyntaxHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);

    // https://www.lua.org/manual/5.2/manual.html
    const QString keywordPatterns[] = {
        QStringLiteral("\\band\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\belse\\b"), QStringLiteral("\\belseif\\b"), QStringLiteral("\\bend\\b"),
        QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfunction\\b"),
        QStringLiteral("\\bgoto\\b"), QStringLiteral("\\bif\\b"), QStringLiteral("\\bin\\b"),
        QStringLiteral("\\blocal\\b"), QStringLiteral("\\bnil\\b"), QStringLiteral("\\bnot\\b"),
        QStringLiteral("\\bor\\b"), QStringLiteral("\\brepeat\\b"), QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bthen\\b"), QStringLiteral("\\btrue\\b"), QStringLiteral("\\buntil\\b"),
        QStringLiteral("\\bwhile\\b")
    };

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        m_highlightRules.append(rule);
    }
}

void LuaSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : m_highlightRules) {
        QRegularExpressionMatchIterator matchIter = rule.pattern.globalMatch(text);
        while (matchIter.hasNext()) {
            QRegularExpressionMatch match = matchIter.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
