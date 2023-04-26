#include <libgui.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <lua.hpp>
#include "main.h"
#include "mx.h"

static int lua_Time_getms(lua_State* L) {
    lua_pushinteger(L, ticks_to_millisecs(gettime()));

    return 1;
}

static int lua_Time_available(lua_State* L) {
    lua_pushboolean(L, MX::isConnected());

    return 1;
}

static const luaL_Reg Time_functions[] = {
    {"getms", lua_Time_getms},
    {"available", lua_Time_available},
    {NULL, NULL}
};

void luaRegisterTimeLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Time_functions, 0);
    lua_setglobal(L, "Time");
}
