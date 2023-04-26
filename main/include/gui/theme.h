#pragma once

#include <string>
#include <lua.hpp>
#include "guiluaelement.h"

void luaRegisterThemeLib(lua_State* L);
void themeAddElement(std::string id, GuiLuaElement* element);
bool loadTheme(const char* themePath, GuiWindow* masterWindow);
