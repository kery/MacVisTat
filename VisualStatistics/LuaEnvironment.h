#ifndef LUAENVIRONMENT_H
#define LUAENVIRONMENT_H

#include "third_party/lua/lua.h"
#include "third_party/lua/lauxlib.h"
#include "third_party/lua/lualib.h"
#include <QString>

class ScriptWindow;

class LuaEnvironment
{
public:
    LuaEnvironment();
    LuaEnvironment(const LuaEnvironment &) = delete;
    LuaEnvironment& operator=(const LuaEnvironment &) = delete;
    ~LuaEnvironment();

    bool initialize(ScriptWindow *sw, QString &err);

    bool doString(const QString &str, QString &err);
    bool doFile(const QString &file, QString &err);

private:
    bool protectedInit(ScriptWindow *sw, QString &err);

    void getLuaError(QString &err);

private:
    lua_State *m_L;
};

#endif // LUAENVIRONMENT_H
