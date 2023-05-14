#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "main.h"
#include "luasupport.h"
#include "gcplus.h"

static int lua_Gcp_isV1(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, GCPlus::isV1());
    return 1;
}

static int lua_Gcp_isV2(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, GCPlus::isV2());
    return 1;
}

static int lua_Gcp_getFWVer(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }
    u16 version;
    GCPlus::unlock();
    GCPlus::getFWVer(&version);
    GCPlus::lock();
    lua_pushnumber(L, (float)((version >> 3) & 0x1FFF) + (version & 0x0007) / 10.0f);
    return 1;
}

static int lua_Gcp_getSticksRange(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 config[13];

    GCPlus::unlock();
    GCPlus::readEEPROM(0x06, config, 13);
    GCPlus::lock();
    lua_newtable(L);
    luaSetTableIntField(L, "leftMinX", config[0]);
    luaSetTableIntField(L, "leftMaxX", config[1]);
    luaSetTableIntField(L, "leftMinY", config[2]);
    luaSetTableIntField(L, "leftMaxY", config[3]);
    luaSetTableIntField(L, "rightMinX", config[4]);
    luaSetTableIntField(L, "rightMaxX", config[5]);
    luaSetTableIntField(L, "rightMinY", config[6]);
    luaSetTableIntField(L, "rightMaxY", config[7]);
    return 1;
}

static int lua_Gcp_setSticksRange(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    //Grab fields
    lua_getfield(L, 1, "leftMinX");
    lua_getfield(L, 1, "leftMaxX");
    lua_getfield(L, 1, "leftMinY");
    lua_getfield(L, 1, "leftMaxY");
    lua_getfield(L, 1, "rightMinX");
    lua_getfield(L, 1, "rightMaxX");
    lua_getfield(L, 1, "rightMinY");
    lua_getfield(L, 1, "rightMaxY");

    u8 config[8];
    //Read values from stack in reverse
    for (int i = 0; i < 8; i++) {
        config[7 - i] = luaL_checkinteger(L, -1 - i);
    }
    lua_pop(L, 9); //Pop table and config data

    GCPlus::unlock();
    GCPlus::writeEEPROM(0x06, config, sizeof(config));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getSticksInvert(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 invert;

    GCPlus::unlock();
    GCPlus::readEEPROM(0x12, &invert, sizeof(invert));
    GCPlus::lock();
    lua_newtable(L);
    luaSetTableBoolField(L, "leftX", invert & 1);
    luaSetTableBoolField(L, "leftY", invert & 2);
    luaSetTableBoolField(L, "rightX", invert & 4);
    luaSetTableBoolField(L, "rightY", invert & 8);
    return 1;
}

static int lua_Gcp_setSticksInvert(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    //Grab fields
    lua_getfield(L, 1, "leftX");
    lua_getfield(L, 1, "leftY");
    lua_getfield(L, 1, "rightX");
    lua_getfield(L, 1, "rightY");

    u8 invert = 0;
    //Read values from stack in reverse
    for (int i = 0; i < 4; i++) {
        invert |= lua_toboolean(L, -1 - i) ? (1 << (3 - i)) : 0;
    }
    lua_pop(L, 5); //Pop table and config data

    GCPlus::unlock();
    GCPlus::writeEEPROM(0x12, &invert, sizeof(invert));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getSticksChannel(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 channels[4];
    const u8 mapTable[4] = {0x02, 0x03, 0x00, 0x01};

    GCPlus::unlock();
    GCPlus::readEEPROM(0x0E, channels, sizeof(channels));
    GCPlus::lock();
    lua_newtable(L);
    luaSetTableIntField(L, "leftX", mapTable[channels[0]] + 1);
    luaSetTableIntField(L, "leftY", mapTable[channels[1]] + 1);
    luaSetTableIntField(L, "rightX", mapTable[channels[2]] + 1);
    luaSetTableIntField(L, "rightY", mapTable[channels[3]] + 1);
    return 1;
}

static int lua_Gcp_setSticksChannel(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    //Grab fields
    lua_getfield(L, 1, "leftX");
    lua_getfield(L, 1, "leftY");
    lua_getfield(L, 1, "rightX");
    lua_getfield(L, 1, "rightY");

    u8 channels[4];
    const u8 mapTable[4] = {0x02, 0x03, 0x00, 0x01};
    //Read values from stack in reverse
    for (int i = 0; i < 4; i++) {
        channels[3 - i] = mapTable[luaL_checkinteger(L, -1 - i) - 1];
    }
    lua_pop(L, 5); //Pop table and config data

    GCPlus::unlock();
    GCPlus::writeEEPROM(0x0E, channels, sizeof(channels));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_loadDefaultSticksConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    GCPlus::unlock();
    u8 tempConfig[13] = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x02, 0x03, 0x00, 0x01, 0x00};
    GCPlus::writeEEPROM(0x06, tempConfig, 13);
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_rebuildLUT(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    GCPlus::unlock();
    GCPlus::reset();
    u64 now = gettime();
    while (ticks_to_millisecs(gettime() - now) < 100);

    return 0;
}

static int lua_Gcp_getSticksDeadzoneRadius(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val;
    GCPlus::unlock();
    GCPlus::readEEPROM(0x13, &val, sizeof(val));
    GCPlus::lock();
    lua_pushinteger(L, val);

    return 1;
}

static int lua_Gcp_setSticksDeadzoneRadius(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val = luaL_checkinteger(L, 1);
    GCPlus::unlock();
    //As of now left stick and right stick have same deadzone
    GCPlus::writeEEPROM(0x13, &val, sizeof(val));
    GCPlus::writeEEPROM(0x14, &val, sizeof(val));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getSticksDeadzoneMode(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val;
    GCPlus::unlock();
    GCPlus::readEEPROM(0x15, &val, sizeof(val));
    GCPlus::lock();
    lua_pushinteger(L, val);

    return 1;
}

static int lua_Gcp_setSticksDeadzoneMode(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val = luaL_checkinteger(L, 1);
    GCPlus::unlock();
    GCPlus::writeEEPROM(0x15, &val, sizeof(val));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getRumbleIntensity(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val;
    GCPlus::unlock();
    GCPlus::readEEPROM(0x16, &val, sizeof(val));
    GCPlus::lock();
    lua_pushinteger(L, val - 128);

    return 1;
}

static int lua_Gcp_setRumbleIntensity(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val = luaL_checkinteger(L, 1) + 128;
    GCPlus::unlock();
    GCPlus::writeEEPROM(0x16, &val, sizeof(val));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getTriggerMode(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val;
    GCPlus::unlock();
    GCPlus::readEEPROM(0x17, &val, sizeof(val));
    GCPlus::lock();
    lua_pushinteger(L, val);

    return 1;
}

static int lua_Gcp_setTriggerMode(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u8 val = luaL_checkinteger(L, 1);
    GCPlus::unlock();
    GCPlus::writeEEPROM(0x17, &val, sizeof(val));
    GCPlus::lock();

    return 0;
}

static int lua_Gcp_getUpdateProgress(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, GCPlus::getUpdateProgress());

    return 1;
}

static int lua_Gcp_isUpdating(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, GCPlus::isUpdating());

    return 1;
}

static int lua_Gcp_hasUpdateSucceeded(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, GCPlus::hasUpdateSucceeded());

    return 1;
}

static int lua_Gcp_startUpdate(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    GCPlus::startUpdate(luaL_checkstring(L, 1));

    return 0;
}

static const luaL_Reg Gcp_functions[] = {
    {"isV1", lua_Gcp_isV1},
    {"isV2", lua_Gcp_isV2},
    {"getFWVer", lua_Gcp_getFWVer},
    {"getSticksRange", lua_Gcp_getSticksRange},
    {"setSticksRange", lua_Gcp_setSticksRange},
    {"getSticksInvert", lua_Gcp_getSticksInvert},
    {"setSticksInvert", lua_Gcp_setSticksInvert},
    {"getSticksChannel", lua_Gcp_getSticksChannel},
    {"setSticksChannel", lua_Gcp_setSticksChannel},
    {"loadDefaultSticksConfig", lua_Gcp_loadDefaultSticksConfig},
    {"rebuildLUT", lua_Gcp_rebuildLUT},
    {"getSticksDeadzoneRadius", lua_Gcp_getSticksDeadzoneRadius},
    {"setSticksDeadzoneRadius", lua_Gcp_setSticksDeadzoneRadius},
    {"getSticksDeadzoneMode", lua_Gcp_getSticksDeadzoneMode},
    {"setSticksDeadzoneMode", lua_Gcp_setSticksDeadzoneMode},
    {"getRumbleIntensity", lua_Gcp_getRumbleIntensity},
    {"setRumbleIntensity", lua_Gcp_setRumbleIntensity},
    {"getTriggerMode", lua_Gcp_getTriggerMode},
    {"setTriggerMode", lua_Gcp_setTriggerMode},
    {"getUpdateProgress", lua_Gcp_getUpdateProgress},
    {"isUpdating", lua_Gcp_isUpdating},
    {"startUpdate", lua_Gcp_startUpdate},
    {"hasUpdateSucceeded", lua_Gcp_hasUpdateSucceeded},
    {NULL, NULL}
};

void luaRegisterGcpLib(lua_State* L) {
    lua_newtable(L);
    luaSetTableIntField(L, "DEADZONE_RADIAL", 0);
    luaSetTableIntField(L, "DEADZONE_SCALEDRADIAL", 1);
    luaSetTableIntField(L, "TRIGGER_DIGITAL", 0);
    luaSetTableIntField(L, "TRIGGER_ANALOG", 1);
    luaL_setfuncs(L, Gcp_functions, 0);
    lua_setglobal(L, "Gcp");
}
