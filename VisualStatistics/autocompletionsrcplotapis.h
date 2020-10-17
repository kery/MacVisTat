#ifndef AUTOCOMPLETIONSRCPLOTAPIS_H
#define AUTOCOMPLETIONSRCPLOTAPIS_H

#include <Qsci/qsciabstractapis.h>

class AutoCompletionSrcPlotAPIs : public QsciAbstractAPIs
{
public:
    AutoCompletionSrcPlotAPIs(QsciLexer *lexer);
    virtual ~AutoCompletionSrcPlotAPIs();

    virtual void updateAutoCompletionList(const QStringList &context, QStringList &list);
    virtual void autoCompletionSelected(const QString &selection);
    virtual QStringList callTips(const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList< int > &shifts);

private:
    QStringList m_apiInfos;
    QStringList m_apiNames;
};

#endif // AUTOCOMPLETIONSRCPLOTAPIS_H
