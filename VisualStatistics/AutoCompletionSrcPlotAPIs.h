#ifndef AUTOCOMPLETIONSRCPLOTAPIS_H
#define AUTOCOMPLETIONSRCPLOTAPIS_H

#include <Qsci/qsciabstractapis.h>

class AutoCompletionSrcPlotAPIs : public QsciAbstractAPIs
{
public:
    AutoCompletionSrcPlotAPIs(QsciLexer *lexer);

    virtual void updateAutoCompletionList(const QStringList &context, QStringList &list) override;
    virtual void autoCompletionSelected(const QString &selection) override;
    virtual QStringList callTips(const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList<int> &shifts) override;

private:
    QStringList mApiInfos;
    QStringList mApiNames;
};

#endif // AUTOCOMPLETIONSRCPLOTAPIS_H
