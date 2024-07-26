#include <stdio.h>
#include <mxml.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <libgui.h>
#include <map>
#include <string>
#include <lua.hpp>
#include "luasupport.h"
#include "system.h"
#include "guielements.h"
#include "utils.h"
#include "main.h"

std::map<std::string, GuiLuaElement*> themeElements;

static int lua_Theme_sendMessage(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    const char* destElement = luaL_checkstring(L, 1);
    auto it = themeElements.find(destElement);
    lua_remove(L, 1); //Remove "destElement" from stack to get to the table

    /*FILE* fp = fopen("/log.txt", "w");
    stackDump(fp, L);
    fclose(fp);*/

    if (it == themeElements.end())
        return luaL_error(L, "cannot find element %s", destElement);

    it->second->handleMessage(L);

    return 0;
}

static int lua_Theme_getThemes(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_newtable(L);

    struct dirent *dirp;
    DIR* dp = opendir("/rvloader/themes");
    int index = 1;

    if (dp != NULL) {
        while ((dirp = readdir(dp)) != NULL) {
            if (dirp->d_name == NULL)
            continue;

            if (dirp->d_name[0] == '.')
                continue;

            if (dirp->d_type != DT_DIR)
                continue;

            luaSetArrayStringField(L, index++, dirp->d_name);
        }
        closedir(dp);
    }

    return 1;
}

static int lua_Theme_getLoadedTheme(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    std::string curTheme;
    if (mainConfig.getValue("theme", &curTheme)) {
        lua_pushstring(L, curTheme.c_str());
    } else {
        lua_pushstring(L, "main");
    }

    return 1;
}

static int lua_Theme_getWiiLoadingScreen(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    int curLoadScreen;
    if(mainConfig.getValue("WiiLoadScreen", &curLoadScreen)){
        lua_pushnumber(L, (int) curLoadScreen);
    } else {
        lua_pushnumber(L, 0);
    }

    return 1;
}

static int lua_Theme_setWiiLoadingScreen(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }
    
    mainConfig.setValue("WiiLoadScreen", luaL_checkinteger(L, 1));
    mainConfig.save(MAINCONFIG_PATH);

    return 0;
}

static int lua_Theme_setTheme(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    mainConfig.setValue("theme", luaL_checkstring(L, 1));
    mainConfig.save(MAINCONFIG_PATH);

    return 0;
}

static int lua_Theme_getBackgrounds(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_newtable(L);

    struct dirent *dirp;
    DIR* dp = opendir("/rvloader/backgrounds");
    int index = 1;

    if (dp != NULL) {
        while ((dirp = readdir(dp)) != NULL) {
            if (dirp->d_name == NULL)
                continue;

            if (dirp->d_name[0] == '.')
                continue;

            //Only include .png files in the list
            if (!strncmp(&dirp->d_name[strlen(dirp->d_name) - 4], ".png", 4) && dirp->d_type == DT_REG)
                luaSetArrayStringField(L, index++, dirp->d_name);
        }
        closedir(dp);
    }

    return 1;
}

static int lua_Theme_getLoadedBackground(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    std::string curBackground;
    if (mainConfig.getValue("background", &curBackground)) {
        lua_pushstring(L, curBackground.c_str());
    } else {
        lua_pushstring(L, "main");
    }

    return 1;
}

static int lua_Theme_setBackground(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    mainConfig.setValue("background", luaL_checkstring(L, 1));
    mainConfig.save(MAINCONFIG_PATH);

    return 0;
}
static const luaL_Reg Theme_functions[] = {
    {"sendMessage", lua_Theme_sendMessage},
    {"getThemes", lua_Theme_getThemes},
    {"getLoadedTheme", lua_Theme_getLoadedTheme},
    {"getWiiLoadingScreen", lua_Theme_getWiiLoadingScreen},
    {"setWiiLoadingScreen", lua_Theme_setWiiLoadingScreen},
    {"setTheme", lua_Theme_setTheme},
    {"getBackgrounds", lua_Theme_getBackgrounds},
    {"getLoadedBackground", lua_Theme_getLoadedBackground},
    {"setBackground", lua_Theme_setBackground},
    {NULL, NULL}
};

void luaRegisterThemeLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Theme_functions, 0);
    lua_setglobal(L, "Theme");
}

void themeAddElement(std::string id, GuiLuaElement* element) {
    themeElements.insert({id, element});
}

static void loadThemeWindow(const char* themePath, GuiWindow* win, mxml_node_t* winNode) {
    mxml_node_t *node = mxmlGetFirstChild(winNode);
    while (node != NULL) {
        const char* xmlElementChar = mxmlGetElement(node);
        if (xmlElementChar != NULL) {
            std::string xmlElement = xmlElementChar;
            GuiLuaElement* luaElement = NULL;

            if (xmlElement == "gcview") {
                GuiGamesView* gView = new GuiGamesView(GC_GAME);
                luaElement = gView;
                gView->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    gView->loadScriptFile(mxmlElementGetAttr(node, "script"));

            } else if (xmlElement == "wiiview") {
                GuiGamesView* gView = new GuiGamesView(WII_GAME);
                luaElement = gView;
                gView->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    gView->loadScriptFile(mxmlElementGetAttr(node, "script"));

            } else if (xmlElement == "vcview") {
                GuiGamesView* gView = new GuiGamesView(WII_VC);
                luaElement = gView;
                gView->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    gView->loadScriptFile(mxmlElementGetAttr(node, "script"));

            } else if (xmlElement == "chanview") {
                GuiGamesView* gView = new GuiGamesView(WII_CHANNEL);
                luaElement = gView;
                gView->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    gView->loadScriptFile(mxmlElementGetAttr(node, "script"));

            } else if (xmlElement == "hbview") {
                GuiHBView* hbView = new GuiHBView();
                luaElement = hbView;
                hbView->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    hbView->loadScriptFile(mxmlElementGetAttr(node, "script"));

            } else if (xmlElement == "luaelement") {
                luaElement = new GuiLuaElement();
                luaElement->setPath(themePath);
                if (mxmlElementGetAttr(node, "script") != NULL)
                    luaElement->loadScriptFile(mxmlElementGetAttr(node, "script"));
            }

            if (luaElement == NULL) {
                node = mxmlGetNextSibling(node);
                continue;
            }

            //Check basic attributes
            int x, y;
            char* position = (char*)mxmlElementGetAttr(node, "position");
            if (position == NULL)
                systemError("Theme error!", "Element %s does not have position attribute", xmlElement.c_str());
            if (sscanf(position, "%d %d", &x, &y) != 2)
                systemError("Theme error!", "Element %s's position attribute is not valid", xmlElement.c_str());

            bool active = true;
            if (matchStr(mxmlElementGetAttr(node, "active"), "false")) {
                active = false;
            }

            const char* elID = mxmlElementGetAttr(node, "id");

            //Add element to window
            if (elID != NULL)
                win->addElement(luaElement, x, y, false, elID);
            else
                win->addElement(luaElement, x, y, false);

            //Force the call of onActiveEvent() if element is active
            if (active)
                win->setElementActive(luaElement, active);

            //Check additional attributes
            if (matchStr(mxmlElementGetAttr(node, "switchable"), "true")) {
                win->setElementSwitchable(luaElement, true);
            }
            if (matchStr(mxmlElementGetAttr(node, "focus"), "true")) {
                win->focusOnElement(luaElement);
            }

            if (elID != NULL)
                themeAddElement(elID, luaElement);
        }
        node = mxmlGetNextSibling(node);
    }
}

bool loadTheme(const char* themePath, GuiWindow* masterWindow) {
    char tempPath[PATH_MAX];
    size_t len = strlen(themePath);

    if (!len)
        systemError("Theme error!", "Wrong theme path");

    if (themePath[len - 1] != '/') {
        sprintf(tempPath, "%s/theme.xml", themePath);
    } else {
        sprintf(tempPath, "%stheme.xml", themePath);
    }

    FILE* fp = fopen(tempPath, "r");
    if (fp == NULL)
        return false;

    mxml_node_t* xmlTree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
    mxml_node_t* themeNode = mxmlFindElement(xmlTree, xmlTree, "theme", NULL, NULL, MXML_DESCEND);
    if (themeNode == NULL)
        systemError("Theme error!", "Missing theme node");

    //Append '/' if missing
    if (themePath[len - 1] != '/')
        sprintf(tempPath, "%s/", themePath);
    else
        strcpy(tempPath, themePath);

    loadThemeWindow(tempPath, masterWindow, themeNode);
    mxmlDelete(xmlTree);

    fclose(fp);

    luaSetGuiMasterWindow(masterWindow);

    return true;
}
