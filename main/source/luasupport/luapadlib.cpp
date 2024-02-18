#include <gccore.h>
#include <lua.hpp>
#include "genpad.h"
#include "luasupport.h"
#include "main.h"

static void luaPushPadTable(lua_State* L, int padState) {
    lua_newtable(L);
    luaSetTableBoolField(L, "BUTTON_LEFT", (padState & PAD_BUTTON_LEFT));
    luaSetTableBoolField(L, "BUTTON_RIGHT", (padState & PAD_BUTTON_RIGHT));
    luaSetTableBoolField(L, "BUTTON_DOWN", (padState & PAD_BUTTON_DOWN));
    luaSetTableBoolField(L, "BUTTON_UP", (padState & PAD_BUTTON_UP));
    luaSetTableBoolField(L, "TRIGGER_Z", (padState & PAD_TRIGGER_Z));
    luaSetTableBoolField(L, "TRIGGER_R", (padState & PAD_TRIGGER_R));
    luaSetTableBoolField(L, "TRIGGER_L", (padState & PAD_TRIGGER_L));
    luaSetTableBoolField(L, "BUTTON_A", (padState & PAD_BUTTON_A));
    luaSetTableBoolField(L, "BUTTON_B", (padState & PAD_BUTTON_B));
    luaSetTableBoolField(L, "BUTTON_X", (padState & PAD_BUTTON_X));
    luaSetTableBoolField(L, "BUTTON_Y", (padState & PAD_BUTTON_Y));
    luaSetTableBoolField(L, "BUTTON_MENU", (padState & PAD_BUTTON_MENU));
    luaSetTableBoolField(L, "BUTTON_START", (padState & PAD_BUTTON_START));
}

static int lua_Pad_isConnected(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_pushboolean(L, connectedPads & (1 << channel));

    return 1;
}

static int lua_Pad_down(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, PAD_ButtonsDown(channel));

    return 1;
}

static int lua_Pad_up(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, PAD_ButtonsUp(channel));

    return 1;
}

static int lua_Pad_held(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, PAD_ButtonsHeld(channel));

    return 1;
}

static int lua_Pad_gendown(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, GenPad::down(channel));

    return 1;
}

static int lua_Pad_genup(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, GenPad::up(channel));

    return 1;
}

static int lua_Pad_genheld(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    luaPushPadTable(L, GenPad::held(channel));

    return 1;
}

static int lua_Pad_stick(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_newtable(L);
    luaSetTableIntField(L, "x", PAD_StickX(channel));
    luaSetTableIntField(L, "y", PAD_StickY(channel));

    return 1;
}

static int lua_Pad_subStick(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_newtable(L);
    luaSetTableIntField(L, "x", PAD_SubStickX(channel));
    luaSetTableIntField(L, "y", PAD_SubStickY(channel));

    return 1;
}

static int lua_Pad_genstick(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_newtable(L);
    luaSetTableIntField(L, "x", GenPad::stickX(channel));
    luaSetTableIntField(L, "y", GenPad::stickY(channel));

    return 1;
}

static int lua_Pad_gensubStick(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_newtable(L);
    luaSetTableIntField(L, "x", GenPad::subStickX(channel));
    luaSetTableIntField(L, "y", GenPad::subStickY(channel));

    return 1;
}

static int lua_Pad_triggers(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    lua_newtable(L);
    luaSetTableIntField(L, "l", PAD_TriggerL(channel));
    luaSetTableIntField(L, "r", PAD_TriggerR(channel));

    return 1;
}

static int lua_Pad_setRumble(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    u32 channel = luaL_checkinteger(L, 1);
    if (channel > 3) {
        return luaL_error(L, "pad channel must be 0-3");
    }

    bool rumbleState = lua_toboolean(L, 2);

    PAD_ControlMotor(channel, rumbleState ? 1 : 0);

    return 0;
}

static const luaL_Reg Pad_functions[] = {
    {"isConnected", lua_Pad_isConnected},
    {"held", lua_Pad_held},
    {"up", lua_Pad_up},
    {"down", lua_Pad_down},
    {"genheld", lua_Pad_genheld},
    {"genup", lua_Pad_genup},
    {"gendown", lua_Pad_gendown},
    {"stick", lua_Pad_stick},
    {"subStick", lua_Pad_subStick},
    {"genstick", lua_Pad_genstick},
    {"gensubStick", lua_Pad_gensubStick},
    {"triggers", lua_Pad_triggers},
    {"setRumble", lua_Pad_setRumble},
    {NULL, NULL}
};

void luaRegisterPadLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Pad_functions, 0);
    lua_setglobal(L, "Pad");
}
