#include <lua.hpp>
#include "luasupport.h"
#include "theme.h"
#include "hiidra.h"

void luaSetTableIntField(lua_State* L, const char* index, int value) {
    lua_pushstring(L, index);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
}

void luaSetTableBoolField(lua_State* L, const char* index, int value) {
    lua_pushstring(L, index);
    lua_pushboolean(L, value);
    lua_settable(L, -3);
}

void luaSetArrayStringField(lua_State* L, int index, const char* value) {
    lua_pushinteger(L, index);
    lua_pushstring(L, value);
    lua_settable(L, -3);
}

void luaRegisterCustomLibs(lua_State* L) {
    luaRegisterGfxLib(L);
    LUALibSfx::registerLibrary(L);
    luaRegisterTimeLib(L);
    luaRegisterPadLib(L);
    luaRegisterGuiLib(L);
    luaRegisterSysLib(L);
    luaRegisterAnimLib(L);
    luaRegisterGcpLib(L);
    luaRegisterPMS2Lib(L);
    luaRegisterUAMPLib(L);
    luaRegisterThemeLib(L);
    luaRegisterHiidraLib(L);
}
