#include "luaenvironment.h"
#include "utils.h"
#include "scriptwindow.h"
#include "plotwindow.h"

static char kScriptWindow;

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

static QCPDataMap* dataMap(lua_State *L,
                           const char *node,
                           const char *name)
{
    QCPDataMap *dataMap = plotWindow(L)->getStat().getDataMap(node, name);
    if (dataMap == nullptr) {
        luaL_error(L, "invalid node or statistics name");
    }
    return dataMap;
}

static int custom_print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);

    scriptWindow(L)->appendLog(str);
    return 0;
}

static int get_keys(lua_State *L)
{
    int index = 0;
    lua_newtable(L);
    for (double key : plotWindow(L)->getStat().getDataKeys()) {
        lua_pushinteger(L, ++index);
        lua_pushinteger(L, (int)key);
        lua_rawset(L, 1);
    }
    return 1;
}

static int get_value(lua_State *L)
{
    int graphIndex = luaL_checkint(L, 1);
    QCustomPlot *plot = plotWindow(L)->getPlot();
    luaL_argcheck(L, graphIndex >= 0 && graphIndex < plot->graphCount(), 1, "graph index out of range");

    int key = luaL_checkint(L, 2);
    const QCPDataMap *dm = plot->graph(graphIndex)->data();

    if (dm->contains(key)) {
        lua_pushinteger(L, dm->value(key).value);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int get_dt(lua_State *L)
{
    int key = luaL_checkint(L, 1);

    Statistics &stat = plotWindow(L)->getStat();
    luaL_argcheck(L, key >= 0 && key < stat.dateTimeCount(), 1, "invalid key");

    lua_pushinteger(L, stat.getDateTime(key));
    return 1;
}

static int get_dt_str(lua_State *L)
{
    int key = luaL_checkint(L, 1);

    Statistics &stat = plotWindow(L)->getStat();
    luaL_argcheck(L, key >= 0 && key < stat.dateTimeCount(), 1, "invalid key");

    lua_pushstring(L, stat.getDateTimeString(key).toStdString().c_str());
    return 1;
}

static int add_graph(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    PlotWindow *plotWnd = plotWindow(L);
    CounterGraph *graph = plotWnd->addCounterGraph();
    if (graph == nullptr) {
        luaL_error(L, "add graph failed");
        return 1;
    }

    graph->setProperty("add_by_script", true);

    QCPData data;
    QCPDataMap *dataMap = graph->data();
    graph->setName(name);
    graph->setDisplayName(name);

    int r = (luaL_optint(L, 3, 255) - 1) % 255 + 1;
    int g = (luaL_optint(L, 4, 0) - 1) % 255 + 1;
    int b = (luaL_optint(L, 5, 0) - 1) % 255 + 1;
    graph->setPen(QPen(QColor(r, g, b)));
    graph->setSelectedPen(graph->pen());

    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            data.key = lua_tonumber(L, -2);
            data.value = lua_tonumber(L, -1);
            dataMap->insert(data.key, data);
        }
        lua_pop(L, 1);
    }

    QCustomPlot *plot = plotWnd->getPlot();
    lua_pushinteger(L, plot->graphCount() - 1);

    plotWnd->updateWindowTitle();
    plotWnd->updatePlotTitle();

    return 1;
}

static int update(lua_State *L)
{
    QCustomPlot *plot = plotWindow(L)->getPlot();
    int rescaleY = lua_toboolean(L, 1);
    if (rescaleY) {
        plot->yAxis->rescale();
        adjustYAxisRange(plot->yAxis);
    }
    plot->replot();
    return 0;
}

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
        { "get_keys", get_keys },
        { "get_value", get_value },
        { "get_dt", get_dt },
        { "get_dt_str", get_dt_str },
        { "add_graph", add_graph },
        { "update", update },
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
