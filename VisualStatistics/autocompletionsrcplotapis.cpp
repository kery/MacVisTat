#include "autocompletionsrcplotapis.h"

#include <qdebug.h>

AutoCompletionSrcPlotAPIs::AutoCompletionSrcPlotAPIs(QsciLexer *lexer) :
    QsciAbstractAPIs(lexer),
    m_apiInfos{
        "graph_count()\n"
        "Returns the total number of graphs in a plot window.",

        "graph_name(graph)\n"
        "Returns the name of a graph. The first parameter graph is the index of the\n"
        "graph in legend box, the index starts from 0.",

        "get_lastkey()\n"
        "Returns the last key of the plot, i.e. the last coordinate of the x axis.\n"
        "The first key of a plot is 0, the second key is 1, and so on.",

        "get_value(graph, key, default)\n"
        "Returns the value of a graph at key. The first parameter graph is the index\n"
        "of the graph in legend box, the index starts from 0.  If the key of graph\n"
        "does not exist, the third optional parameter default will be returned. If the\n"
        "default is not given, a nil will be returned.",

        "get_dt(key)\n"
        "Returns the date time at key as the number of seconds that have passed since\n"
        "1970-01-01T00:00:00.",

        "get_dtstr(key)\n"
        "Returns the date time at key as a string, e.g. 2020-08-22 20:07:12.",

        "set_selected(graph, sel)\n"
        "Set the selection state of a graph. The first parameter graph is the index of the\n"
        "graph in legend box, the index starts from 0. The second parameter is the selection\n"
        "state, true indicates selected.",

        "add_graph(name, data, r, g, b)\n"
        "Adds a new graph in plot window with name. The parameter data is an array, the\n"
        "elements' index is represented as the key of the graph. The optional parameters\n"
        "r, g, b will be used as the color of the graph.",

        "update(rescaleY)\n"
        "Refreshes the plot area. It is necessary to call this function after adding one or\n"
        "more new graphs. If the optional parameter rescaleY is set to true then the y axis\n"
        "will be rescaled if necessary."
    }
{
    for (const QString &apiInfo : m_apiInfos) {
        m_apiNames.append(apiInfo.left(apiInfo.indexOf(QLatin1Char('('))));
    }
}

AutoCompletionSrcPlotAPIs::~AutoCompletionSrcPlotAPIs()
{
}

void AutoCompletionSrcPlotAPIs::updateAutoCompletionList(const QStringList &context, QStringList &list)
{
    if (context.size() != 2 || context.first() != "plot") {
        return;
    }

    for (const QString &apiName : m_apiNames) {
        if (apiName.startsWith(context.last())) {
            list.append(apiName);
        }
    }
}

void AutoCompletionSrcPlotAPIs::autoCompletionSelected(const QString &selection)
{
    Q_UNUSED(selection)
}

QStringList AutoCompletionSrcPlotAPIs::callTips(const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList< int > &shifts)
{
    Q_UNUSED(commas)
    Q_UNUSED(style)
    Q_UNUSED(shifts)

    QStringList list;

    if (context.size() != 3 || context.first() != "plot") {
        return list;
    }

    for (int i = 0; i < m_apiNames.size(); ++i) {
        if (m_apiNames[i] == context[1]) {
            list.append(m_apiInfos[i]);
            break;
        }
    }
    return list;
}
