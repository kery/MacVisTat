#include "LuaEnvironment.h"
#include "ScriptWindow.h"
#include "PlotWindow.h"
#include "ui_PlotWindow.h"
#include "CounterGraph.h"
#include "DateTimeTicker.h"
#include "PlotData.h"
#include "GlobalDefines.h"
#include "Utils.h"

char LuaEnvironment::sKeyScriptWnd;

LuaEnvironment::LuaEnvironment() :
    mL(nullptr)
{
}

LuaEnvironment::~LuaEnvironment()
{
    if (mL) {
        lua_close(mL);
    }
}

QString LuaEnvironment::initialize(ScriptWindow *sw)
{
    mL = luaL_newstate();
    if (mL == nullptr) {
        return QString("create Lua state failed");
    }
    return protectedInit(sw);
}

QString LuaEnvironment::doString(const QString &str)
{
    if ((luaL_loadstring(mL, str.toStdString().c_str()) ||
         lua_pcall(mL, 0, 0, 0)) != LUA_OK)
    {
        return getLastLuaError(mL);
    }
    return QString();
}

QString LuaEnvironment::protectedInit(ScriptWindow *sw)
{
    lua_pushcfunction(mL, initLuaEnv);
    lua_pushlightuserdata(mL, sw);

    if (lua_pcall(mL, 1, 0, 0) != LUA_OK) {
        return getLastLuaError(mL);
    }
    return QString();
}

ScriptWindow * LuaEnvironment::scriptWindow(lua_State *L)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, &sKeyScriptWnd);

    ScriptWindow *sw = static_cast<ScriptWindow *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return sw;
}

PlotWindow * LuaEnvironment::plotWindow(lua_State *L)
{
    return qobject_cast<PlotWindow *>(scriptWindow(L)->parent());
}

int LuaEnvironment::print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    scriptWindow(L)->appendLog(str);
    return 0;
}

int LuaEnvironment::graphCount(lua_State *L)
{
    CounterPlot *plot = plotWindow(L)->ui->plot;
    lua_pushinteger(L, plot->graphCount());
    return 1;
}

int LuaEnvironment::graphName(lua_State *L)
{
    int graphIndex = luaL_checkint(L, 1);
    CounterPlot *plot = plotWindow(L)->ui->plot;
    luaL_argcheck(L, graphIndex >= 0 && graphIndex < plot->graphCount(), 1, "graph index out of range");

    QString name = plot->graph(graphIndex)->name();
    lua_pushstring(L, name.toStdString().c_str());
    return 1;
}

int LuaEnvironment::getLastKey(lua_State *L)
{
    PlotWindow *plotWnd = plotWindow(L);
    QSharedPointer<QCPGraphDataContainer> data = plotWnd->mPlotData.firstCounterData();
    if (data && data->size() > 0) {
        lua_pushinteger(L, data->size() - 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int LuaEnvironment::getValue(lua_State *L)
{
    int graphIndex = luaL_checkint(L, 1);
    CounterPlot *plot = plotWindow(L)->ui->plot;
    luaL_argcheck(L, graphIndex >= 0 && graphIndex < plot->graphCount(), 1, "graph index out of range");

    int valueIndex = luaL_checkint(L, 2);
    QSharedPointer<QCPGraphDataContainer> data = plot->graph(graphIndex)->data();
    luaL_argcheck(L, valueIndex >= 0 && valueIndex < data->size(), 2, "key out of range");

    double value = data->at(valueIndex)->value;
    if (qIsNaN(value) && lua_isnumber(L, 3)) {
        lua_pushvalue(L, 3);
    } else {
        lua_pushnumber(L, value);
    }
    return 1;
}

int LuaEnvironment::addGraph(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    PlotWindow *plotWnd = plotWindow(L);
    luaL_argcheck(L, !plotWnd->mPlotData.contains(name), 1, "graph name already exists");
    luaL_checktype(L, 2, LUA_TTABLE);

    if (plotWnd->mPlotData.counterCount() == 0) {
        luaL_error(L, "cannot add graph if currently plot is empty");
    }

    QColor color = plotWnd->mColorPool.getColor();
    // Check if color parameters are given
    if (lua_gettop(L) > 2) {
        int r = (luaL_optint(L, 3, 255) - 1) % 255 + 1;
        int g = (luaL_optint(L, 4, 0) - 1) % 255 + 1;
        int b = (luaL_optint(L, 5, 0) - 1) % 255 + 1;
        color.setRgb(r, g, b);
    }

    QSharedPointer<QCPGraphDataContainer> data = plotWnd->mPlotData.firstCounterData();
    QVector<QCPGraphData> dataVector(data->size());
    for (int i = 0; i < dataVector.size(); ++i) {
        dataVector[i].key = data->at(i)->key;
        dataVector[i].value = NAN;
    }

    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            int index = lua_tointeger(L, -2);
            if (index >= 0 && index < dataVector.size()) {
                dataVector[index].value = lua_tonumber(L, -1);
            }
        }
        lua_pop(L, 1);
    }

    QSharedPointer<QCPGraphDataContainer> newData = plotWnd->mPlotData.addCounterData(name);
    newData->set(dataVector, true);

    CounterGraph *newGraph = plotWnd->ui->plot->addGraph();
    plotWnd->ui->plot->updateYAxesTickVisible();
    newGraph->setName(name);
    newGraph->setData(newData);
    newGraph->setPen(QPen(color));
    newGraph->setSuspectKeys(plotWnd->mPlotData.suspectKeys(name));

    plotWnd->updateWindowTitle();
    plotWnd->updatePlotTitle();
    return 0;
}

int LuaEnvironment::updatePlot(lua_State *L)
{
    PlotWindow *plotWnd = plotWindow(L);
    plotWnd->ui->plot->rescaleYAxes();
    plotWnd->ui->plot->replot(QCustomPlot::rpQueuedReplot);
    return 0;
}

int LuaEnvironment::initLuaEnv(lua_State *L)
{
    luaL_openlibs(L);

    // replace default print function
    lua_pushcfunction(L, print);
    lua_setglobal(L, "print");

    // store ScriptWindow to registry
    lua_pushlightuserdata(L, &sKeyScriptWnd);
    lua_pushvalue(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);

    const struct luaL_Reg apis[] = {
        { "graph_count", graphCount },
        { "graph_name", graphName },
        { "get_lastkey", getLastKey },
        { "get_value", getValue },
        { "add_graph", addGraph },
        { "update", updatePlot },
        {NULL, NULL}
    };

    luaL_newlib(L, apis);
    lua_setglobal(L, "plot");
    return 0;
}
