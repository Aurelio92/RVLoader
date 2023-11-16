#include <gccore.h>
#include <lua.hpp>
#include "system.h"

static int lua_Sys_debug(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    const char* str = luaL_checkstring(L, 1);

    printf(str);

    return 0;
}

static int lua_Sys_bootSysMenu(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootSysMenu();

    return 0;
}

static int lua_Sys_bootPriiloader(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootPriiloader();

    return 0;
}

static int lua_Sys_bootInstaller(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootDOL("/apps/RVLoader/installer.dol", "", false);

    return 0;
}

static int lua_Sys_reboot(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    reboot();

    return 0;
}

static int lua_Sys_getVersion(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, (float)VER_MAJOR + VER_MINOR / 10.0f);

    return 1;
}

static const luaL_Reg Sys_functions[] = {
    {"debug", lua_Sys_debug},
    {"bootSysMenu", lua_Sys_bootSysMenu},
    {"bootPriiloader", lua_Sys_bootPriiloader},
    {"bootInstaller", lua_Sys_bootInstaller},
    {"reboot", lua_Sys_reboot},
    {"getVersion", lua_Sys_getVersion},
    {NULL, NULL}
};

void luaRegisterSysLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Sys_functions, 0);
    lua_setglobal(L, "Sys");
}
