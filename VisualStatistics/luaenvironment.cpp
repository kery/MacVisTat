#include "luaenvironment.h"
#include "utils.h"
#include "scriptwindow.h"
#include "plotwindow.h"

static char kScriptWindow;

//static QCustomPlot* checkCustomPlot(lua_State *L)
//{
//    lua_pushlightuserdata(L, &kPlot);
//    lua_rawget(L, LUA_REGISTRYINDEX);
//    if (!lua_isuserdata(L, -1)) {
//        luaL_error(L, "invalid plot object in registry");
//    }
//    QCustomPlot *plot = (QCustomPlot*)lua_touserdata(L, -1);
//    lua_pop(L, 1);
//    return plot;
//}

//static QCustomPlot* checkCustomPlotAndGraphIndex(lua_State *L, int index, int arg)
//{
//    QCustomPlot *plot = checkCustomPlot(L);
//    luaL_argcheck(L, index >= 0 && index < plot->graphCount(), arg, "out of range");
//    return plot;
//}

//static int getDateTime(lua_State *L, int arg)
//{
//    lua_pushlightuserdata(L, &kDateTimeVector);
//    lua_rawget(L, LUA_REGISTRYINDEX);
//    if (!lua_isuserdata(L, -1)) {
//        luaL_error(L, "invalid date time vector in registry");
//    }
//    QVector<qint32> *vec = (QVector<qint32>*)lua_touserdata(L, -1);
//    lua_pop(L, 1);

//    int index = luaL_checkint(L, arg);
//    luaL_argcheck(L, index >=0 && index < vec->size(), arg, "out of range");

//    return vec->at(index);
//}

static ScriptWindow* scriptWindow(lua_State *L)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, &kScriptWindow);

    ScriptWindow *sw = (ScriptWindow*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return sw;
}

static PlotWindow* plotWindow(lua_State *L)
{
    return qobject_cast<PlotWindow*>(scriptWindow(L)->parent());
}

static int custom_print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);

    scriptWindow(L)->appendLog(str);
    return 0;
}

//static int value_cfunc(lua_State *L)
//{
//    int graph_index = luaL_checkint(L, 1);
//    int value_index = luaL_checkint(L, 2);

//    QCustomPlot *plot = checkCustomPlotAndGraphIndex(L, graph_index, 1);

//    QCPGraph *graph = plot->graph(graph_index);
//    if (graph->data()->contains(value_index)) {
//        lua_pushinteger(L, (int)graph->data()->value(value_index).value);
//    } else {
//        luaL_error(L, "no value at %d", value_index);
//    }

//    return 1;
//}

//static int dt_cfunc(lua_State *L)
//{
//    int dt_num = getDateTime(L, 1);
//    lua_pushinteger(L, dt_num);

//    return 1;
//}

//static int dt_str_cfunc(lua_State *L)
//{
//    int dt_num = getDateTime(L, 1);
//    QDateTime dt = QDateTime::fromTime_t(dt_num);
//    lua_pushstring(L, dt.toString(DT_FORMAT).toStdString().c_str());
//    return 1;
//}

//static int value_count_cfunc(lua_State *L)
//{
//    int graph_index = luaL_checkint(L, 1);

//    QCustomPlot *plot = checkCustomPlotAndGraphIndex(L, graph_index, 1);

//    QCPGraph *graph = plot->graph(graph_index);
//    lua_pushinteger(L, graph->data()->size());

//    return 1;
//}

//static int add_graph_cfunc(lua_State *L)
//{
//    const char *name = luaL_checkstring(L, 1);
//    luaL_checktype(L, 2, LUA_TTABLE);

//    QCustomPlot *plot = checkCustomPlot(L);

//    QCPGraph *graph = plot->addGraph();
//    if (graph) {
//        QCPData data;
//        QCPDataMap *dataMap = graph->data();
//        graph->setName(name);
//        graph->setPen(QPen(QColor(51, 102, 0)));

//        lua_pushnil(L);
//        while (lua_next(L, 2) != 0) {
//            if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
//                data.key = lua_tonumber(L, -2);
//                data.value = lua_tonumber(L, -1);
//                dataMap->insert(data.key, data);
//            }
//            lua_pop(L, 1);
//        }

//        lua_pushinteger(L, plot->graphCount() - 1);
//    } else {
//        luaL_error(L, "add graph failed");
//    }

//    return 1;
//}

//static int remove_graph_cfunc(lua_State *L)
//{
//    int index = luaL_checkint(L, 1);

//    // get initial graph count from registry
//    lua_pushlightuserdata(L, &kInitialGraphCount);
//    lua_rawget(L, LUA_REGISTRYINDEX);
//    int initialGraphCount = luaL_checkint(L, -1);
//    lua_pop(L, 1);

//    luaL_argcheck(L, index >= initialGraphCount, 1, "only custom graph can be removed");

//    QCustomPlot *plot = checkCustomPlot(L);
//    bool ret = plot->removeGraph(index);
//    lua_pushboolean(L, ret ? 1 : 0);

//    return 1;
//}

//static int graph_count_cfunc(lua_State *L)
//{
//    QCustomPlot *plot = checkCustomPlot(L);
//    lua_pushinteger(L, plot->graphCount());

//    return 1;
//}

//static int graph_name_cfunc(lua_State *L)
//{
//    int graph_index = luaL_checkint(L, 1);

//    QCustomPlot *plot = checkCustomPlotAndGraphIndex(L, graph_index, 1);

//    lua_pushstring(L, plot->graph(graph_index)->name().toStdString().c_str());

//    return 1;
//}

//static int update_cfunc(lua_State *L)
//{
//    QCustomPlot *plot = checkCustomPlot(L);
//    int rescaleY = lua_toboolean(L, 1);
//    if (rescaleY) {
//        plot->yAxis->rescale();
//        adjustYAxisRange(plot->yAxis);
//    }
//    plot->replot();
//    return 0;
//}

static int init_cfunc(lua_State *L)
{
    luaL_openlibs(L);

    // replace default print function
    lua_pushcfunction(L, custom_print);
    lua_setglobal(L, "print");

    // store ScriptWindow to registry
    lua_pushlightuserdata(L, &kScriptWindow);
    lua_pushvalue(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);

    const struct luaL_Reg methods[] = {
//        {"value", value_cfunc},
//        {"dt", dt_cfunc},
//        {"dt_str", dt_str_cfunc},
//        {"value_count", value_count_cfunc},
//        {"add_graph", add_graph_cfunc},
//        {"remove_graph", remove_graph_cfunc},
//        {"graph_count", graph_count_cfunc},
//        {"graph_name", graph_name_cfunc},
//        {"update", update_cfunc},
        {NULL, NULL}
    };

    luaL_newlib(L, methods);
    lua_setglobal(L, "plot");

    return 0;
}

LuaEnvironment::LuaEnvironment() :
    m_L(NULL)
{
}

LuaEnvironment::~LuaEnvironment()
{
    if (m_L) {
        lua_close(m_L);
    }
}

bool LuaEnvironment::initialize(ScriptWindow *sw, QString &err)
{
    Q_ASSERT(m_L == NULL);

    m_L = luaL_newstate();
    if (!m_L) {
        err = "create Lua state failed";
        return false;
    }

    return protectedInit(sw, err);
}

bool LuaEnvironment::doString(const QString &str, QString &err)
{
    Q_ASSERT(m_L != NULL);

    if ((luaL_loadstring(m_L, str.toStdString().c_str()) ||
         lua_pcall(m_L, 0, 0, 0)) != LUA_OK)
    {
        getLuaError(err);
        return false;
    }

    return true;
}

bool LuaEnvironment::doFile(const QString &file, QString &err)
{
    Q_ASSERT(m_L != NULL);

    if ((luaL_loadfile(m_L, file.toStdString().c_str()) ||
         lua_pcall(m_L, 0, 0, 0)) != LUA_OK)
    {
        getLuaError(err);
        return false;
    }

    return true;
}

bool LuaEnvironment::protectedInit(ScriptWindow *sw, QString &err)
{
    Q_ASSERT(m_L != NULL);

    lua_pushcfunction(m_L, init_cfunc);
    lua_pushlightuserdata(m_L, sw);

    if (lua_pcall(m_L, 1, 0, 0) != LUA_OK) {
        getLuaError(err);
        return false;
    }

    return true;
}

void LuaEnvironment::getLuaError(QString &err)
{
    Q_ASSERT(m_L != NULL);

    const char *msg = lua_type(m_L, -1) == LUA_TSTRING ? lua_tostring(m_L, -1)
                                                     : NULL;
    if (!msg) {
        msg = "(error object is not a string)";
    }
    err = msg;
    lua_pop(m_L, 1);
}
