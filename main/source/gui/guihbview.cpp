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
        {"setCoverSize", GuiHBView::lua_setCoverSize},
        {"drawHBCover", GuiHBView::lua_drawHBCover},
        {"getHBCount", GuiHBView::lua_getHBCount},
        {"getHBName", GuiHBView::lua_getHBName},
        {"bootHB", GuiHBView::lua_bootHB},
        {NULL, NULL}
    };
    lua_newtable(L); //GamesView
    luaL_setfuncs(L, GuiHBView_functions, 0);
    lua_setglobal(L, "HBView");
}

int GuiHBView::lua_setCoverSize(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);

    //Get games list and cover dimensions
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* gamesList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverWidth");
    int* coverWidth = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverHeight");
    int* coverHeight = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    *coverWidth = w;
    *coverHeight = h;

    //Set size for all games already in the list
    for (auto& game : *gamesList) {
        if (game.image != NULL)
           game.image->setSize(*coverWidth, *coverHeight);
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

    //Get games list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* gamesList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Draw the cover
    try {
        Mtx tempMtx;
        HBContainer& hbc = gamesList->at(idx);
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

    //Get games list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* gamesList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushinteger(L, gamesList->size());

    return 1;
}

int GuiHBView::lua_getHBName(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get games list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* gamesList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Return result
    try {
        HBContainer& hbc = gamesList->at(idx);
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

    //Get games list
    lua_getglobal(L, "_hbList");
    std::vector<HBContainer>* gamesList = (std::vector<HBContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    printf("lua_bootHB\n");

    //Return result
    try {
        HBContainer& hbc = gamesList->at(idx);
        bootDOL(hbc.path.c_str(), "");
    } catch (std::out_of_range& e) {

    }
    return 0;
}
