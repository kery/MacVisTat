#ifndef LUAENVIRONMENT_H
#define LUAENVIRONMENT_H

#include "third_party/lua/lua.h"
#include "third_party/lua/lauxlib.h"
#include "third_party/lua/lualib.h"
#include "third_party/qcustomplot/qcustomplot.h"

class LuaEnvironment
{
public:
    LuaEnvironment();
    ~LuaEnvironment();

    bool initialize(QCustomPlot *plot, QString &err);

    void setPrintLogEdit(QPlainTextEdit *logEdit);
    void setDateTimeVector(void *vec);

    bool doString(const QString &str, QString &err);

private:
    bool protectedInit(QCustomPlot *plot, QString &err);

    void getLuaError(QString &err);

private:
    lua_State *L;
};

#endif // LUAENVIRONMENT_H
