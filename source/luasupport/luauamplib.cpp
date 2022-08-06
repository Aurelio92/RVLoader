#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "main.h"
#include "luasupport.h"
#include "uamp.h"
#include "hud.h"

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

    HUD::setHeadphonesVolume(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_UAMP_getHeadphonesVolume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, HUD::getHeadphonesVolume());

    return 1;
}

static int lua_UAMP_setSpeakersVolume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    HUD::setSpeakersVolume(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_UAMP_getSpeakersVolume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, HUD::getSpeakersVolume());

    return 1;
}

static int lua_UAMP_setVolumeControlSystem(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    HUD::setVolumeControlSystem(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_UAMP_getVolumeControlSystem(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, HUD::getVolumeControlSystem());

    return 1;
}


static const luaL_Reg UAMP_functions[] = {
    {"isConnected", lua_UAMP_isConnected},
    {"setMute", lua_UAMP_setMute},
    {"setHeadphonesVolume", lua_UAMP_setHeadphonesVolume},
    {"getHeadphonesVolume", lua_UAMP_getHeadphonesVolume},
    {"setSpeakersVolume", lua_UAMP_setSpeakersVolume},
    {"getSpeakersVolume", lua_UAMP_getSpeakersVolume},
    {"setVolumeControlSystem", lua_UAMP_setVolumeControlSystem},
    {"getVolumeControlSystem", lua_UAMP_getVolumeControlSystem},
    {NULL, NULL}
};

void luaRegisterUAMPLib(lua_State* L) {
    lua_newtable(L);
    luaSetTableIntField(L, "HUD_CTRL_GCPAD", HUD_CTRL_GCPAD);
    luaSetTableIntField(L, "HUD_CTRL_POT", HUD_CTRL_POT);
    luaSetTableIntField(L, "HUD_CTRL_BUTTONS", HUD_CTRL_BUTTONS);
    luaL_setfuncs(L, UAMP_functions, 0);
    lua_setglobal(L, "UAMP");
}
