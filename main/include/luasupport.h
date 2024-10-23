#pragma once

#include <libgui.h>
#include <lua.hpp>

void luaSetTableIntField(lua_State* L, const char* index, int value);
void luaSetTableBoolField(lua_State* L, const char* index, int value);
void luaSetArrayStringField(lua_State* L, int index, const char* value);
void luaSetGuiMasterWindow(GuiWindow* win);
void luaSetGuiParentWindow(GuiWindow* win);

void luaRegisterCustomLibs(lua_State* L);

void luaRegisterGfxLib(lua_State* L);
namespace LUALibSfx {
    void registerLibrary(lua_State* L);
};
void luaRegisterTimeLib(lua_State* L);
void luaRegisterPadLib(lua_State* L);
void luaRegisterGuiLib(lua_State* L);
void luaRegisterSysLib(lua_State* L);
void luaRegisterAnimLib(lua_State* L);
void luaRegisterGcpLib(lua_State* L);
void luaRegisterPMS2Lib(lua_State* L);
void luaRegisterUAMPLib(lua_State* L);
