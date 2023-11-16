#include <gccore.h>
#include <libgui.h>
#include <unistd.h>
#include <lua.hpp>
#include <stdexcept>
#include "titles.h"
#include "main.h"
#include "luasupport.h"
#include "system.h"
#include "guihbview.h"
#include "mx.h"

GuiHBView::GuiHBView() {
    //Set default cover size
    coverWidth = 128;
    coverHeight = 48;

    initLUA();
}

GuiHBView::~GuiHBView() {
    lua_close(L);
}

void GuiHBView::initLUA() {
    if (L != NULL) {
        lua_close(L);
    }

    //Init LUA
    L = luaL_newstate();
    luaL_openlibs(L);

    //Link some members to global LUA variables
    lua_pushlightuserdata(L, &wiiHomebrews);
    lua_setglobal(L, "_hbList");
    lua_pushlightuserdata(L, &coverWidth);
    lua_setglobal(L, "_coverWidth");
    lua_pushlightuserdata(L, &coverHeight);
    lua_setglobal(L, "_coverHeight");
    lua_pushlightuserdata(L, this);
    lua_setglobal(L, "_this");

    //Register custom libraries
    luaRegisterCustomLibs(L);

    //Register GuiHBView library
    static const luaL_Reg GuiHBView_functions[] = {
        {"setCoverSize", lua_setCoverSize},
        {"drawHBCover", lua_drawHBCover},
        {"getHBCount", lua_getHBCount},
        {"getHBName", lua_getHBName},
        {"bootHB", lua_bootHB},
        {"openHBConfig", lua_openHBConfig},
        {"saveHBConfig", lua_saveHBConfig},
        {"setHBConfigValue", lua_setHBConfigValue},
        {"getHBConfigValue", lua_getHBConfigValue},
        {NULL, NULL}
    };
    lua_newtable(L); //HBView
    luaL_setfuncs(L, GuiHBView_functions, 0);

    //Create HBView.config table
    lua_pushstring(L, "config");
    lua_newtable(L);
    luaSetTableIntField(L, "DISABLE", 0);
    luaSetTableIntField(L, "ENABLE", 1);
    luaSetTableIntField(L, "NO", 0);
    luaSetTableIntField(L, "YES", 1);
    lua_settable(L, -3);

    lua_setglobal(L, "HBView");
}

void GuiHBView::openHBConfig(u32 idx) {
    int tempVal;
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    HBContainer& hbc = hbList->at(idx);
    //Load settings
    hbConfig.close(); //Make sure config lists are empty
    hbConfig.open(hbc.confPath.c_str());

    //Set default values for missing configs
    if (!hbConfig.getValue("Patch MX chip", &tempVal))
        hbConfig.setValue("Patch MX chip", MX::isConnected() ? 0 : 1);
}

int GuiHBView::lua_setCoverSize(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);

    //Get homebrews list and cover dimensions
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverWidth");
    int* coverWidth = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverHeight");
    int* coverHeight = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    *coverWidth = w;
    *coverHeight = h;

    //Set size for all homebrews already in the list
    for (auto& hb : *hbList) {
        if (hb.image != NULL)
           hb.image->setSize(*coverWidth, *coverHeight);
    }

    return 0;
}

int GuiHBView::lua_drawHBCover(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 3) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    u32 idx = luaL_checkinteger(L, 3);

    lua_getglobal(L, "_coverWidth");
    int* coverWidth = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverHeight");
    int* coverHeight = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Get homebrews list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Draw the cover
    try {
        Mtx tempMtx;
        HBContainer& hbc = hbList->at(idx);
        Gfx::pushMatrix();
        Gfx::translate(x, y);
        Gfx::getCurMatrix(tempMtx);

        //Load the cover if it's inside the screen view
        if ((tempMtx[0][3] >= -*coverWidth) && (tempMtx[0][3] < getScreenSize().x + *coverWidth)
            && (tempMtx[1][3] >= -*coverHeight) && (tempMtx[1][3] < getScreenSize().y + *coverHeight)) {
            if (hbc.image == NULL) {
                if (hbc.coverPath.size() > 0)
                    hbc.image = new GuiImage(hbc.coverPath.c_str());
                else
                    hbc.image = dummyHBIcon;
                hbc.image->setSize(*coverWidth, *coverHeight);
            }
        } else {
            if (hbc.image != NULL && hbc.image != dummyHBIcon)
                delete hbc.image;
            hbc.image = NULL;
        }
        if (hbc.image != NULL)
            hbc.image->draw(false);

        Gfx::popMatrix();
    } catch (std::out_of_range& e) {

    }

    return 0;
}

int GuiHBView::lua_getHBCount(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get homebrews list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushinteger(L, hbList->size());

    return 1;
}

int GuiHBView::lua_getHBName(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get homebrews list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Return result
    try {
        HBContainer& hbc = hbList->at(idx);
        lua_pushstring(L, hbc.name.c_str());
    } catch (std::out_of_range& e) {
        lua_pushstring(L, "");
    }
    return 1;
}

int GuiHBView::lua_bootHB(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get homebrews list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* hbList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_this");
    GuiHBView* thisView = (GuiHBView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    printf("lua_bootHB\n");

    //Return result
    try {
        int tempVal;
        HBContainer& hbc = hbList->at(idx);
        //Read the configuration file or set the default values if they are missing
        thisView->openHBConfig(idx);

        thisView->hbConfig.getValue("Patch MX chip", &tempVal);
        bootDOL(hbc.path.c_str(), "", tempVal);
    } catch (std::out_of_range& e) {

    }
    return 0;
}

int GuiHBView::lua_openHBConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiHBView* thisView = (GuiHBView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Open config
    try {
        thisView->openHBConfig(idx);
    } catch (std::out_of_range& e) {
        return luaL_error(L, "Can't find homebrew at index %u", idx);
    }

    return 0;
}

int GuiHBView::lua_saveHBConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiHBView* thisView = (GuiHBView*)lua_touserdata(L, -1);
    lua_pop(L, 1);


    //Save config
    thisView->hbConfig.save();

    return 0;
}

int GuiHBView::lua_setHBConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiHBView* thisView = (GuiHBView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->hbConfig.setValue(luaL_checkstring(L, 1), luaL_checkinteger(L, 2));

    return 0;
}

int GuiHBView::lua_getHBConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    int val = 0;

    lua_getglobal(L, "_this");
    GuiHBView* thisView = (GuiHBView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->hbConfig.getValue(luaL_checkstring(L, 1), &val);
    lua_pushinteger(L, val);

    return 1;
}
