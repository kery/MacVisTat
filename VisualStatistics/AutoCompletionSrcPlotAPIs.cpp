#include "AutoCompletionSrcPlotAPIs.h"

AutoCompletionSrcPlotAPIs::AutoCompletionSrcPlotAPIs(QsciLexer *lexer) :
    QsciAbstractAPIs(lexer),
    mApiInfos{
        "graph_count()\n"
        "Returns the total number of graphs in a plot window.",

        "graph_name(graph)\n"
        "Returns the name of a graph. The first parameter 'graph' is the index of the\n"
        "graph in legend box, the index starts from 0.",

        "get_lastkey()\n"
        "Returns the last key of the plot, i.e. the last coordinate of the x axis.\n"
        "The first key of a plot is 0, the second key is 1, and so on.",

        "get_value(graph, key, default)\n"
        "Returns the value of a graph at 'key'. The first parameter 'graph' is the index\n"
        "of the graph in legend box, the index starts from 0.  If the value of 'key' does\n"
        "not exist, the third optional parameter 'default' will be returned. If the 'default'\n"
        "is not given, a nil will be returned.",

        "add_graph(name, data, r, g, b)\n"
        "Adds a new graph in plot window with 'name'. The parameter data is an array, the\n"
        "elements' index is represented as the key of the graph. The optional parameters\n"
        "'r', 'g', 'b' will be used as the color of the graph.",

        "update()\n"
        "Updates the plot. It is necessary to call this function after adding one or\n"
        "more new graphs."
    }
{
    for (const QString &apiInfo : qAsConst(mApiInfos)) {
        mApiNames.append(apiInfo.left(apiInfo.indexOf(QLatin1Char('('))));
    }
}

void AutoCompletionSrcPlotAPIs::updateAutoCompletionList(const QStringList &context, QStringList &list)
{
    if (context.size() != 2 || context.first() != "plot") {
        return;
    }
    for (const QString &apiName : qAsConst(mApiNames)) {
        if (apiName.startsWith(context.last())) {
            list.append(apiName);
        }
    }
}

void AutoCompletionSrcPlotAPIs::autoCompletionSelected(const QString &/*selection*/)
{
}

QStringList AutoCompletionSrcPlotAPIs::callTips(const QStringList &context, int /*commas*/, QsciScintilla::CallTipsStyle /*style*/, QList< int > &/*shifts*/)
{
    QStringList list;

    if (context.size() != 3 || context.first() != "plot") {
        return list;
    }
    for (int i = 0; i < mApiNames.size(); ++i) {
        if (mApiNames[i] == context[1]) {
            list.append(mApiInfos[i]);
            break;
        }
    }
    return list;
}
