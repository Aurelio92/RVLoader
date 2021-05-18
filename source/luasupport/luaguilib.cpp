#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "luasupport.h"
#include "main.h"

static GuiWindow* masterWindow = NULL;
static GuiWindow* parentWindow = NULL;

static int lua_Gui_loseFocus(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Vector2* direction = (Vector2*)luaL_checkinteger(L, 1);

    if (parentWindow != NULL)
        parentWindow->loseFocus(*direction);

    return 0;
}

static int lua_Gui_getScreenSize(lua_State* L) {
    lua_newtable(L);
    luaSetTableIntField(L, "x", getScreenSize().x);
    luaSetTableIntField(L, "y", getScreenSize().y);

    return 1;
}

static int lua_Gui_switchToElement(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    const char* el = luaL_checkstring(L, 1);

    if (masterWindow != NULL) {
        if (masterWindow->switchToElement(el)) {
            mainConfig.setValue("LastView", el);
            mainConfig.save(MAINCONFIG_PATH);
        }
    }

    return 0;
}

static const luaL_Reg Gui_functions[] = {
    {"loseFocus", lua_Gui_loseFocus},
    {"getScreenSize", lua_Gui_getScreenSize},
    {"switchToElement", lua_Gui_switchToElement},
    {NULL, NULL}
};

void luaSetGuiMasterWindow(GuiWindow* win) {
    masterWindow = win;
}

void luaSetGuiParentWindow(GuiWindow* win) {
    parentWindow = win;
}

void luaRegisterGuiLib(lua_State* L) {
    lua_newtable(L);
    luaSetTableIntField(L, "RIGHT", (u32)&Vector2::right);
    luaSetTableIntField(L, "LEFT", (u32)&Vector2::left);
    luaSetTableIntField(L, "UP", (u32)&Vector2::up);
    luaSetTableIntField(L, "DOWN", (u32)&Vector2::down);
    luaL_setfuncs(L, Gui_functions, 0);
    lua_setglobal(L, "Gui");
}