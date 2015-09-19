#include "luaenvironment.h"
#include "utils.h"

#define PLOT_METATB_NAME "plot"
#define GRAPH_METATB_NAME "graph"

static char kPlot;
static char kInitialGraphCount;
static char kPrintLogEdit;
static char kDateTimeVector;

static QCustomPlot* checkCustomPlot(lua_State *L)
{
    lua_pushlightuserdata(L, &kPlot);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) {
        luaL_error(L, "invalid plot object in registry");
    }
    QCustomPlot *plot = (QCustomPlot*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return plot;
}

static QCustomPlot* checkCustomPlotAndGraphIndex(lua_State *L, int index, int arg)
{
    QCustomPlot *plot = checkCustomPlot(L);
    luaL_argcheck(L, index >= 0 && index < plot->graphCount(), arg, "out of range");
    return plot;
}

static int getDateTime(lua_State *L, int arg)
{
    lua_pushlightuserdata(L, &kDateTimeVector);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) {
        luaL_error(L, "invalid date time vector in registry");
    }
    QVector<qint32> *vec = (QVector<qint32>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    int index = luaL_checkint(L, arg);
    luaL_argcheck(L, index >=0 && index < vec->size(), arg, "out of range");

    return vec->at(index);
}

static int custom_print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);

    lua_pushlightuserdata(L, &kPrintLogEdit);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_isuserdata(L, -1)) {
        QPlainTextEdit *logEdit = (QPlainTextEdit*)lua_touserdata(L, -1);
        logEdit->appendPlainText(str);
    } else {
        luaL_error(L, "call print failed: target not found");
    }

    return 0;
}

static int value_cfunc(lua_State *L)
{
    int graph_index = luaL_checkint(L, 1);
    int value_index = luaL_checkint(L, 2);

    QCustomPlot *plot = checkCustomPlotAndGraphIndex(L, graph_index, 1);

    QCPGraph *graph = plot->graph(graph_index);
    if (graph->data()->contains(value_index)) {
        lua_pushinteger(L, (int)graph->data()->value(value_index).value);
    } else {
        luaL_error(L, "no value at %d", value_index);
    }

    return 1;
}

static int dt_cfunc(lua_State *L)
{
    int dt_num = getDateTime(L, 1);
    lua_pushinteger(L, dt_num);

    return 1;
}

static int dt_str_cfunc(lua_State *L)
{
    int dt_num = getDateTime(L, 1);
    QDateTime dt = QDateTime::fromTime_t(dt_num);
    lua_pushstring(L, dt.toString(DT_FORMAT).toStdString().c_str());
    return 1;
}

static int count_cfunc(lua_State *L)
{
    int graph_index = luaL_checkint(L, 1);

    QCustomPlot *plot = checkCustomPlotAndGraphIndex(L, graph_index, 1);

    QCPGraph *graph = plot->graph(graph_index);
    lua_pushinteger(L, graph->data()->size());

    return 1;
}

static int add_graph_cfunc(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    QCustomPlot *plot = checkCustomPlot(L);

    QCPGraph *graph = plot->addGraph();
    if (graph) {
        QCPData data;
        QCPDataMap *dataMap = graph->data();
        graph->setName(name);

        lua_pushnil(L);
        while (lua_next(L, 2) != 0) {
            if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
                data.key = lua_tonumber(L, -2);
                data.value = lua_tonumber(L, -1);
                dataMap->insert(data.key, data);
            }
            lua_pop(L, 1);
        }

        lua_pushinteger(L, plot->graphCount() - 1);
    } else {
        luaL_error(L, "add graph failed");
    }

    return 1;
}

static int remove_graph_cfunc(lua_State *L)
{
    int index = luaL_checkint(L, 1);

    // get initial graph count from registry
    lua_pushlightuserdata(L, &kInitialGraphCount);
    lua_rawget(L, LUA_REGISTRYINDEX);
    int initialGraphCount = luaL_checkint(L, -1);
    lua_pop(L, 1);

    luaL_argcheck(L, index >= initialGraphCount, 1, "only custom graph can be removed");

    QCustomPlot *plot = checkCustomPlot(L);
    bool ret = plot->removeGraph(index);
    lua_pushboolean(L, ret ? 1 : 0);

    return 1;
}

static int update_cfunc(lua_State *L)
{
    QCustomPlot *plot = checkCustomPlot(L);
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

    // store plot to registry
    lua_pushlightuserdata(L, &kPlot);
    lua_pushvalue(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);

    // store initial graph count to registry
    lua_pushlightuserdata(L, &kInitialGraphCount);
    lua_pushvalue(L, 2);
    lua_rawset(L, LUA_REGISTRYINDEX);

    const struct luaL_Reg methods[] = {
        {"value", value_cfunc},
        {"dt", dt_cfunc},
        {"dt_str", dt_str_cfunc},
        {"count", count_cfunc},
        {"add_graph", add_graph_cfunc},
        {"remove_graph", remove_graph_cfunc},
        {"update", update_cfunc},
        {NULL, NULL}
    };

    luaL_newlib(L, methods);
    lua_setglobal(L, "plot");

    return 0;
}

LuaEnvironment::LuaEnvironment() :
    L(NULL)
{
}

LuaEnvironment::~LuaEnvironment()
{
    if (L) {
        lua_close(L);
    }
}

bool LuaEnvironment::initialize(QCustomPlot *plot, QString &err)
{
    Q_ASSERT(L == NULL);

    L = luaL_newstate();
    if (!L) {
        err = "create Lua state failed";
        return false;
    }

    if (!protectedInit(plot, err)) {
        return false;
    }

    return true;
}

void LuaEnvironment::setPrintLogEdit(QPlainTextEdit *logEdit)
{
    Q_ASSERT(L != NULL);

    lua_pushlightuserdata(L, &kPrintLogEdit);
    lua_pushlightuserdata(L, logEdit);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void LuaEnvironment::setDateTimeVector(void *vec)
{
    Q_ASSERT(L != NULL);

    lua_pushlightuserdata(L, &kDateTimeVector);
    lua_pushlightuserdata(L, vec);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

bool LuaEnvironment::doString(const QString &str, QString &err)
{
    Q_ASSERT(L != NULL);

    if ((luaL_loadstring(L, str.toStdString().c_str()) ||
         lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        getLuaError(err);
        return false;
    }

    return true;
}

bool LuaEnvironment::protectedInit(QCustomPlot *plot, QString &err)
{
    Q_ASSERT(L != NULL);

    lua_pushcfunction(L, init_cfunc);
    lua_pushlightuserdata(L, plot);
    lua_pushinteger(L, plot->graphCount());

    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        getLuaError(err);
        return false;
    }

    return true;
}

void LuaEnvironment::getLuaError(QString &err)
{
    Q_ASSERT(L != NULL);

    const char *msg = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1)
                                                     : NULL;
    if (!msg) {
        msg = "(error object is not a string)";
    }
    err = msg;
    lua_pop(L, 1);
}
