#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "main.h"
#include "luasupport.h"
#include "uamp.h"

static int lua_UAMP_isConnected(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, UAMP::isConnected());

    return 1;
}

static int lua_UAMP_setMute(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    UAMP::setMute(lua_toboolean(L, 1));

    return 0;
}

static int lua_UAMP_setHeadphonesVolume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    UAMP::setHeadphonesVolume(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_UAMP_setSpeakersVolume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    UAMP::setSpeakersVolume(luaL_checkinteger(L, 1));

    return 0;
}

static const luaL_Reg UAMP_functions[] = {
    {"isConnected", lua_UAMP_isConnected},
    {"setMute", lua_UAMP_setMute},
    {"setHeadphonesVolume", lua_UAMP_setHeadphonesVolume},
    {"setSpeakersVolume", lua_UAMP_setSpeakersVolume},
    {NULL, NULL}
};

void luaRegisterUAMPLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, UAMP_functions, 0);
    lua_setglobal(L, "UAMP");
}
