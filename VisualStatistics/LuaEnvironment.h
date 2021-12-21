#ifndef LUAENVIRONMENT_H
#define LUAENVIRONMENT_H

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}
#include <QString>

class ScriptWindow;
class PlotWindow;

class LuaEnvironment
{
public:
    LuaEnvironment();
    ~LuaEnvironment();

    QString initialize(ScriptWindow *sw);
    QString doString(const QString &str);

private:
    QString protectedInit(ScriptWindow *sw);
    QString getLastError();

    static ScriptWindow * scriptWindow(lua_State *L);
    static PlotWindow * plotWindow(lua_State *L);
    static int print(lua_State *L);
    static int graphCount(lua_State *L);
    static int graphName(lua_State *L);
    static int getLastKey(lua_State *L);
    static int getValue(lua_State *L);
    static int addGraph(lua_State *L);
    static int updatePlot(lua_State *L);
    static int initLuaEnv(lua_State *L);

    static char sKeyScriptWnd;

    lua_State *mL;
};

#endif // LUAENVIRONMENT_H
