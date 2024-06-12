#include <gccore.h>
#include <libgui.h>
#include <unistd.h>
#include <lua.hpp>
#include <stdexcept>
#include "titles.h"
#include "main.h"
#include "luasupport.h"
#include "system.h"
#include "guigamesview.h"
#include "nintendont.h"
#include "hiidra.h"
#include "gc2wiimote.h"
#include "rvldd.h"
#include "mx.h"

GuiGamesView::GuiGamesView(TitleType _titlesType) {
    //Set default cover size
    coverWidth = 160;
    coverHeight = 224;

    titlesType = _titlesType;

    lastGC2WiimoteConfigPath = "";
    loadDefaultGC2WiimoteConfig();

    initLUA();
}

GuiGamesView::~GuiGamesView() {
    lua_close(L);
}

void GuiGamesView::initLUA() {
    if (L != NULL) {
        lua_close(L);
    }

    //Init LUA
    L = luaL_newstate();
    luaL_openlibs(L);

    //Link some members to global LUA variables
    switch (titlesType) {
        case GC_GAME:
            lua_pushlightuserdata(L, &gcGames);
        break;

        case WII_GAME:
            lua_pushlightuserdata(L, &wiiGames);
        break;

        case WII_CHANNEL:
            lua_pushlightuserdata(L, &wiiChannels);
        break;

        case WII_VC:
            lua_pushlightuserdata(L, &vcGames);
        break;
    }
    lua_setglobal(L, "_gamesList");
    lua_pushlightuserdata(L, &coverWidth);
    lua_setglobal(L, "_coverWidth");
    lua_pushlightuserdata(L, &coverHeight);
    lua_setglobal(L, "_coverHeight");
    lua_pushlightuserdata(L, this);
    lua_setglobal(L, "_this");

    //Register custom libraries
    luaRegisterCustomLibs(L);

    //Register GuiGamesView library
    static const luaL_Reg GuiGamesView_functions[] = {
        {"setCoverSize", lua_setCoverSize},
        {"drawGameCover", lua_drawGameCover},
        {"drawGameSaveIcon", lua_drawGameSaveIcon},
        {"getGamesCount", lua_getGamesCount},
        {"getGameName", lua_getGameName},
        {"getGamesType", lua_getGamesType},
        {"bootGame", lua_bootGame},
        {"openGameConfig", lua_openGameConfig},
        {"saveGameConfig", lua_saveGameConfig},
        {"setGameConfigValue", lua_setGameConfigValue},
        {"getGameConfigValue", lua_getGameConfigValue},
        {"readGameCheats", lua_readGameCheats},
        {"getCheatNameHash", lua_getCheatNameHash},
        {"openGC2WiimoteGameConfig", lua_openGC2WiimoteGameConfig},
        {"saveGC2WiimoteGameConfig", lua_saveGC2WiimoteGameConfig},
        {"setGC2WiimoteGameConfigValue", lua_setGC2WiimoteGameConfigValue},
        {"getGC2WiimoteGameConfigValue", lua_getGC2WiimoteGameConfigValue},
        {"setGC2WiimoteGameConfigModifier", lua_setGC2WiimoteGameConfigModifier},
        {"getGC2WiimoteGameConfigModifier", lua_getGC2WiimoteGameConfigModifier},
        {"setGC2WiimoteGameConfigNegModifier", lua_setGC2WiimoteGameConfigNegModifier},
        {"getGC2WiimoteGameConfigNegModifier", lua_getGC2WiimoteGameConfigNegModifier},
        {"getGC2WiimoteMapString", lua_getGC2WiimoteMapString},
        {"getGC2WiimoteMapValue", lua_getGC2WiimoteMapValue},
        {"openGCPMapGameConfig", lua_openGCPMapGameConfig},
        {"saveGCPMapGameConfig", lua_saveGCPMapGameConfig},
        {"setGCPMapGameConfigValue", lua_setGCPMapGameConfigValue},
        {"getGCPMapGameConfigValue", lua_getGCPMapGameConfigValue},
        {"getGCPMapString", lua_getGCPMapString},
        {"getGCPMapValue", lua_getGCPMapValue},
        {NULL, NULL}
    };
    lua_newtable(L); //GamesView
    luaL_setfuncs(L, GuiGamesView_functions, 0);

    //Create GamesView.gameType table
    lua_pushstring(L, "gameType");
    lua_newtable(L);
    luaSetTableIntField(L, "GC_GAME", GC_GAME);
    luaSetTableIntField(L, "WII_GAME", WII_GAME);
    luaSetTableIntField(L, "WII_CHANNEL", WII_CHANNEL);
    luaSetTableIntField(L, "WII_VC", WII_VC);
    lua_settable(L, -3);

    //Create GamesView.config table
    lua_pushstring(L, "config");
    lua_newtable(L);
    luaSetTableIntField(L, "DISABLE", 0);
    luaSetTableIntField(L, "ENABLE", 1);
    luaSetTableIntField(L, "NO", 0);
    luaSetTableIntField(L, "YES", 1);
    lua_settable(L, -3);

    //Create GamesView.nintendont table
    lua_pushstring(L, "nintendont");
    lua_newtable(L);
    luaSetTableIntField(L, "VIDEO_AUTO", NIN_VID_AUTO);
    luaSetTableIntField(L, "VIDEO_NTSC", NIN_VID_FORCE | NIN_VID_FORCE_NTSC);
    luaSetTableIntField(L, "VIDEO_PAL50", NIN_VID_FORCE | NIN_VID_FORCE_PAL50);
    luaSetTableIntField(L, "VIDEO_PAL60", NIN_VID_FORCE | NIN_VID_FORCE_PAL60);
    luaSetTableIntField(L, "VIDEO_MPAL", NIN_VID_FORCE | NIN_VID_FORCE_MPAL);
    luaSetTableIntField(L, "LANG_AUTO", NIN_LAN_AUTO);
    luaSetTableIntField(L, "LANG_ENGLISH", NIN_LAN_ENGLISH);
    luaSetTableIntField(L, "LANG_GERMAN", NIN_LAN_GERMAN);
    luaSetTableIntField(L, "LANG_FRENCH", NIN_LAN_FRENCH);
    luaSetTableIntField(L, "LANG_SPANISH", NIN_LAN_SPANISH);
    luaSetTableIntField(L, "LANG_ITALIAN", NIN_LAN_ITALIAN);
    luaSetTableIntField(L, "LANG_DUTCH", NIN_LAN_DUTCH);
    lua_settable(L, -3);

    //Create GamesView.hiidra table
    lua_pushstring(L, "hiidra");
    lua_newtable(L);
    luaSetTableIntField(L, "PADREAD_AUTO", HIIDRA_PADREAD_AUTO);
    luaSetTableIntField(L, "PADREAD_BYPASS", HIIDRA_PADREAD_BYPASS);
    luaSetTableIntField(L, "PADREAD_REDIRECT", HIIDRA_PADREAD_REDIRECT);
    luaSetTableIntField(L, "HOOKTYPE_VBI", HIIDRA_HOOKTYPE_VBI);
    luaSetTableIntField(L, "HOOKTYPE_KPADRead", HIIDRA_HOOKTYPE_KPADRead);
    luaSetTableIntField(L, "HOOKTYPE_Joypad", HIIDRA_HOOKTYPE_Joypad);
    luaSetTableIntField(L, "HOOKTYPE_GXDraw", HIIDRA_HOOKTYPE_GXDraw);
    luaSetTableIntField(L, "HOOKTYPE_GXFlush", HIIDRA_HOOKTYPE_GXFlush);
    luaSetTableIntField(L, "HOOKTYPE_OSSleepThread", HIIDRA_HOOKTYPE_OSSleepThread);
    luaSetTableIntField(L, "HOOKTYPE_AXNextFrame", HIIDRA_HOOKTYPE_AXNextFrame);
    luaSetTableIntField(L, "DEFLICKER_AUTO", HIIDRA_DEFLICKER_AUTO);
    luaSetTableIntField(L, "DEFLICKER_OFF", HIIDRA_DEFLICKER_OFF);
    luaSetTableIntField(L, "DEFLICKER_OFF_EXTENDED", HIIDRA_DEFLICKER_OFF_EXTENDED);
    luaSetTableIntField(L, "DEFLICKER_ON_LOW", HIIDRA_DEFLICKER_ON_LOW);
    luaSetTableIntField(L, "DEFLICKER_ON_MEDIUM", HIIDRA_DEFLICKER_ON_MEDIUM);
    luaSetTableIntField(L, "DEFLICKER_ON_HIGH", HIIDRA_DEFLICKER_ON_HIGH);
    lua_settable(L, -3);

    //Create GamesView.GC2Wiimote
    lua_pushstring(L, "GC2Wiimote");
    lua_newtable(L);
    luaSetTableIntField(L, "EXT_NONE", EXT_NONE);
    luaSetTableIntField(L, "EXT_NUNCHUK", EXT_NUNCHUK);
    luaSetTableIntField(L, "EXT_CLASSIC", EXT_CLASSIC);
    luaSetTableIntField(L, "EXT_MAX", EXT_MAX);
    luaSetTableIntField(L, "ORIENT_STANDARD", ORIENT_STANDARD);
    luaSetTableIntField(L, "ORIENT_SIDEWAYS", ORIENT_SIDEWAYS);
    luaSetTableIntField(L, "ORIENT_MAX", ORIENT_MAX);
    luaSetTableIntField(L, "IRMODE_DIRECT", IRMODE_DIRECT);
    luaSetTableIntField(L, "IRMODE_SHIFT", IRMODE_SHIFT);
    luaSetTableIntField(L, "IRMODE_MAX", IRMODE_MAX);
    lua_settable(L, -3);

    lua_setglobal(L, "GamesView");
}

void GuiGamesView::openGameConfig(u32 idx) {
    int tempVal;
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    GameContainer& gc = gamesList->at(idx);
    //Load settings
    gameConfig.close(); //Make sure config lists are empty
    gameConfig.open(gc.confPath.c_str());

    //Set default values for missing configs
    switch (titlesType) {
        case GC_GAME:
            if (!gameConfig.getValue("Force progressive", &tempVal))
                gameConfig.setValue("Force progressive", 0);
            if (!gameConfig.getValue("Force widescreen", &tempVal))
                gameConfig.setValue("Force widescreen", 0);
            if (!gameConfig.getValue("Force RVL-DD stretching", &tempVal))
                gameConfig.setValue("Force RVL-DD stretching", 0);
            if (!gameConfig.getValue("Native SI", &tempVal))
                gameConfig.setValue("Native SI", 0);
            if (!gameConfig.getValue("Video mode", &tempVal))
                gameConfig.setValue("Video mode", NIN_VID_AUTO);
            if (!gameConfig.getValue("Language", &tempVal))
                gameConfig.setValue("Language", NIN_LAN_AUTO);
            if (!gameConfig.getValue("Enable Cheats", &tempVal))
                gameConfig.setValue("Enable Cheats", 0);
            if (!gameConfig.getValue("Memory Card Emulation", &tempVal))
                gameConfig.setValue("Memory Card Emulation", 1);
            if (!gameConfig.getValue("Max Pads", &tempVal))
                gameConfig.setValue("Max Pads", 4);
            if (!gameConfig.getValue("Video Width", &tempVal))
                gameConfig.setValue("Video Width", 0);

            //GC+2.0 mapping
            if (!gameConfig.getValue("GCPMap_A", &tempVal))
                gameConfig.setValue("GCPMap_A", PAD_BUTTON_A);
            if (!gameConfig.getValue("GCPMap_B", &tempVal))
                gameConfig.setValue("GCPMap_B", PAD_BUTTON_B);
            if (!gameConfig.getValue("GCPMap_X", &tempVal))
                gameConfig.setValue("GCPMap_X", PAD_BUTTON_X);
            if (!gameConfig.getValue("GCPMap_Y", &tempVal))
                gameConfig.setValue("GCPMap_Y", PAD_BUTTON_Y);
            if (!gameConfig.getValue("GCPMap_UP", &tempVal))
                gameConfig.setValue("GCPMap_UP", PAD_BUTTON_UP);
            if (!gameConfig.getValue("GCPMap_DOWN", &tempVal))
                gameConfig.setValue("GCPMap_DOWN", PAD_BUTTON_DOWN);
            if (!gameConfig.getValue("GCPMap_RIGHT", &tempVal))
                gameConfig.setValue("GCPMap_RIGHT", PAD_BUTTON_RIGHT);
            if (!gameConfig.getValue("GCPMap_LEFT", &tempVal))
                gameConfig.setValue("GCPMap_LEFT", PAD_BUTTON_LEFT);
            if (!gameConfig.getValue("GCPMap_Z", &tempVal))
                gameConfig.setValue("GCPMap_Z", PAD_TRIGGER_Z);
            if (!gameConfig.getValue("GCPMap_R", &tempVal))
                gameConfig.setValue("GCPMap_R", PAD_TRIGGER_R);
            if (!gameConfig.getValue("GCPMap_L", &tempVal))
                gameConfig.setValue("GCPMap_L", PAD_TRIGGER_L);
            if (!gameConfig.getValue("GCPMap_START", &tempVal))
                gameConfig.setValue("GCPMap_START", PAD_BUTTON_START);

            memset(gcpMap, 0, sizeof(int) * GCP_MAP_N_BUTTONS);
            gameConfig.getValue("GCPMap_A", &gcpMap[GCP_MAP_BUTTON_A_ID]);
            gameConfig.getValue("GCPMap_B", &gcpMap[GCP_MAP_BUTTON_B_ID]);
            gameConfig.getValue("GCPMap_X", &gcpMap[GCP_MAP_BUTTON_X_ID]);
            gameConfig.getValue("GCPMap_Y", &gcpMap[GCP_MAP_BUTTON_Y_ID]);
            gameConfig.getValue("GCPMap_UP", &gcpMap[GCP_MAP_BUTTON_DU_ID]);
            gameConfig.getValue("GCPMap_DOWN", &gcpMap[GCP_MAP_BUTTON_DD_ID]);
            gameConfig.getValue("GCPMap_RIGHT", &gcpMap[GCP_MAP_BUTTON_DR_ID]);
            gameConfig.getValue("GCPMap_LEFT", &gcpMap[GCP_MAP_BUTTON_DL_ID]);
            gameConfig.getValue("GCPMap_Z", &gcpMap[GCP_MAP_BUTTON_Z_ID]);
            gameConfig.getValue("GCPMap_R", &gcpMap[GCP_MAP_BUTTON_RD_ID]);
            gameConfig.getValue("GCPMap_L", &gcpMap[GCP_MAP_BUTTON_LD_ID]);
            gameConfig.getValue("GCPMap_START", &gcpMap[GCP_MAP_BUTTON_ST_ID]);
        break;

        case WII_GAME:
            if (!gameConfig.getValue("Enable WiFi", &tempVal))
                gameConfig.setValue("Enable WiFi", 0);
            if (!gameConfig.getValue("Enable Bluetooth", &tempVal))
                gameConfig.setValue("Enable Bluetooth", 0);
            if (!gameConfig.getValue("Enable USB saves", &tempVal))
                gameConfig.setValue("Enable USB saves", 1);
            if (!gameConfig.getValue("Enable GC2Wiimote", &tempVal))
                gameConfig.setValue("Enable GC2Wiimote", 0);
            if (!gameConfig.getValue("Patch MX chip", &tempVal))
                gameConfig.setValue("Patch MX chip", MX::isConnected() ? 0 : 1);
            if (!gameConfig.getValue("PADRead Mode", &tempVal))
                gameConfig.setValue("PADRead Mode", HIIDRA_PADREAD_AUTO);
            if (!gameConfig.getValue("Deflicker Mode", &tempVal))
                gameConfig.setValue("Deflicker Mode", HIIDRA_DEFLICKER_AUTO);
            if (!gameConfig.getValue("Enable Cheats", &tempVal))
                gameConfig.setValue("Enable Cheats", 0);
            if (!gameConfig.getValue("Cheats Hooktype", &tempVal))
                gameConfig.setValue("Cheats Hooktype", HIIDRA_HOOKTYPE_VBI);
        break;

        case WII_CHANNEL:
            if (!gameConfig.getValue("Enable WiFi", &tempVal))
                gameConfig.setValue("Enable WiFi", 0);
            if (!gameConfig.getValue("Enable Bluetooth", &tempVal))
                gameConfig.setValue("Enable Bluetooth", 0);
            if (!gameConfig.getValue("Enable GC2Wiimote", &tempVal))
                gameConfig.setValue("Enable GC2Wiimote", 0);
            if (!gameConfig.getValue("Patch MX chip", &tempVal))
                gameConfig.setValue("Patch MX chip", MX::isConnected() ? 0 : 1);
            if (!gameConfig.getValue("PADRead Mode", &tempVal))
                gameConfig.setValue("PADRead Mode", HIIDRA_PADREAD_AUTO);
            if (!gameConfig.getValue("Deflicker Mode", &tempVal))
                gameConfig.setValue("Deflicker Mode", HIIDRA_DEFLICKER_AUTO);
            if (!gameConfig.getValue("Enable Cheats", &tempVal))
                gameConfig.setValue("Enable Cheats", 0);
            if (!gameConfig.getValue("Cheats Hooktype", &tempVal))
                gameConfig.setValue("Cheats Hooktype", HIIDRA_HOOKTYPE_VBI);
        break;

        case WII_VC:
            if (!gameConfig.getValue("Enable WiFi", &tempVal))
                gameConfig.setValue("Enable WiFi", 0);
            if (!gameConfig.getValue("Enable Bluetooth", &tempVal))
                gameConfig.setValue("Enable Bluetooth", 0);
            if (!gameConfig.getValue("Enable GC2Wiimote", &tempVal))
                gameConfig.setValue("Enable GC2Wiimote", 0);
            if (!gameConfig.getValue("Patch MX chip", &tempVal))
                gameConfig.setValue("Patch MX chip", MX::isConnected() ? 0 : 1);
            if (!gameConfig.getValue("PADRead Mode", &tempVal))
                gameConfig.setValue("PADRead Mode", HIIDRA_PADREAD_AUTO);
            if (!gameConfig.getValue("Deflicker Mode", &tempVal))
                gameConfig.setValue("Deflicker Mode", HIIDRA_DEFLICKER_AUTO);
            if (!gameConfig.getValue("Enable Cheats", &tempVal))
                gameConfig.setValue("Enable Cheats", 0);
            if (!gameConfig.getValue("Cheats Hooktype", &tempVal))
                gameConfig.setValue("Cheats Hooktype", HIIDRA_HOOKTYPE_VBI);
        break;
    }
}

void GuiGamesView::loadDefaultGC2WiimoteConfig() {
    memset(&emuConfig, 0, sizeof(WMEmuConfig_t));
    emuConfig.magic = 0x574D4549;
    emuConfig.version = GC2WIIMOTE_VER; //0x01.0x00
    emuConfig.orientation = ORIENT_STANDARD;
    emuConfig.extension = EXT_NONE;
    emuConfig.mpEnabled = 0;
    memset(&(emuConfig.mapping), 0, sizeof(WMMapping_t));
    memset(&(emuConfig.modifiers), 0, sizeof(WMMapping_t));
    memset(&(emuConfig.negModifiers), 0, sizeof(WMMapping_t));
}

void GuiGamesView::openGameGC2WiimoteConfig(u32 idx) {
    u32 magic;
    u32 version;

    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    GameContainer& gc = gamesList->at(idx);

    loadDefaultGC2WiimoteConfig();
    lastGC2WiimoteConfigPath = "/wmemu/" + gc.gameIDString + "_map.conf";

    FILE* fp = fopen(lastGC2WiimoteConfigPath.c_str(), "rb");
    if (fp == NULL) {
        printf("Couldn't load file\n");
        return;
    }

    if (fread(&magic, 1, sizeof(u32), fp) != sizeof(u32)) {
        printf("Couldn't read magic\n");
        fclose(fp);
        return;
    }
    if (magic != 0x574D4549) {
        printf("Magic didn't match\n");
        fclose(fp);
        return;
    }
    if (fread(&version, 1, sizeof(u32), fp) != sizeof(u32)) {
        printf("Couldn't read version\n");
        fclose(fp);
        return;
    }
    if (version != GC2WIIMOTE_VER) {
        printf("Version didn't match\n");
        fclose(fp);
        return;
    }
    rewind(fp);
    if (fread(&emuConfig, 1, sizeof(WMEmuConfig_t), fp) != sizeof(WMEmuConfig_t)) {
        printf("Couldn't read config\n");
        fclose(fp);
        loadDefaultGC2WiimoteConfig();
        return;
    }
    fclose(fp);
}

int GuiGamesView::lua_setCoverSize(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);

    //Get games list and cover dimensions
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
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

int GuiGamesView::lua_drawGameCover(lua_State* L) {
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
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Draw the cover
    try {
        Mtx tempMtx;
        GameContainer& gc = gamesList->at(idx);
        Gfx::pushMatrix();
        Gfx::translate(x, y);
        Gfx::getCurMatrix(tempMtx);

        switch (thisView->titlesType) {
            case GC_GAME:
                LWP_MutexLock(gcCoversMutex);
            break;

            case WII_GAME:
                LWP_MutexLock(wiiCoversMutex);
            break;

            case WII_CHANNEL:
                LWP_MutexLock(wiiChanCoversMutex);
            break;

            case WII_VC:
                LWP_MutexLock(vcCoversMutex);
            break;
        }

        //Load the cover if it's inside the screen view
        if ((tempMtx[0][3] >= -*coverWidth) && (tempMtx[0][3] < getScreenSize().x + *coverWidth)
            && (tempMtx[1][3] >= -*coverHeight) && (tempMtx[1][3] < getScreenSize().y + *coverHeight)) {
            if (gc.image == NULL) {
                if (gc.coverPath.size() > 0)
                    gc.image = new GuiImage(gc.coverPath.c_str());
                else
                    gc.image = dummyCover;
                gc.image->setSize(*coverWidth, *coverHeight);
            }
        } else {
            if (gc.image != NULL && gc.image != dummyCover)
                delete gc.image;
            gc.image = NULL;
        }
        if (gc.image != NULL)
            gc.image->draw(false);

        switch (thisView->titlesType) {
            case GC_GAME:
                LWP_MutexUnlock(gcCoversMutex);
            break;

            case WII_GAME:
                LWP_MutexUnlock(wiiCoversMutex);
            break;

            case WII_CHANNEL:
                LWP_MutexUnlock(wiiChanCoversMutex);
            break;

            case WII_VC:
                LWP_MutexUnlock(vcCoversMutex);
            break;
        }

        Gfx::popMatrix();
    } catch (std::out_of_range& e) {

    }

    return 0;
}

int GuiGamesView::lua_drawGameSaveIcon(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2 && argc != 3) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Game index is always the last argument
    u32 idx = luaL_checkinteger(L, argc);

    lua_getglobal(L, "_coverWidth");
    int* coverWidth = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_coverHeight");
    int* coverHeight = (int*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Get games list
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (argc == 2) {
        f32 coordinates[8];
        luaL_checktype(L, 1, LUA_TTABLE);
        if (lua_rawlen(L, 1) != 8) {
            return luaL_error(L, "drawRectangleFromCorners first argument must have 8 elements");
        }

        for (int i = 1; i <= 8; i++) {
            lua_pushnumber(L, i);
            lua_gettable(L, 1);

            if (!lua_isnumber(L, -1)) // optional check
                return luaL_error(L, "item %d invalid (number required, got %s)",
                                i, luaL_typename(L, -1));

            coordinates[i-1] = lua_tonumber(L, -1);
            lua_pop(L, 1);
        }

        try {
            GameContainer& gc = gamesList->at(idx);
            gc.save.drawIcon(coordinates);
        } catch (std::out_of_range& e) {

        }
    } else {
        int x = luaL_checkinteger(L, 1);
        int y = luaL_checkinteger(L, 2);

        try {
            GameContainer& gc = gamesList->at(idx);
            Gfx::pushMatrix();
            Gfx::translate(x, y);

            Gfx::popMatrix();
        } catch (std::out_of_range& e) {

        }
    }

    return 0;
}

int GuiGamesView::lua_getGamesCount(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get games list
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushinteger(L, gamesList->size());

    return 1;
}

int GuiGamesView::lua_getGameName(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get games list
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Return result
    try {
        GameContainer& gc = gamesList->at(idx);
        lua_pushstring(L, gc.name.c_str());
    } catch (std::out_of_range& e) {
        lua_pushstring(L, "");
    }
    return 1;
}

int GuiGamesView::lua_getGamesType(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Return result
    lua_pushinteger(L, thisView->titlesType);
    return 1;
}

int GuiGamesView::lua_bootGame(lua_State* L) {
    char oldPath[PATH_MAX];
    int argc = lua_gettop(L);
    bool forceReinstall = false;
    if (argc != 1 && argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get games list
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);
    if (argc == 2) {
        forceReinstall = lua_toboolean(L, 2);
    }

    printf("lua_bootGame\n");

    //Return result
    try {
        std::vector<uint32_t> cheats;

        GameContainer& gc = gamesList->at(idx);
        //Read the configuration file or set the default values if they are missing
        thisView->openGameConfig(idx);

        //Boot game
        if (thisView->titlesType == WII_GAME) {
            int tempVal;
            HIIDRA_CFG cfg;
            memset(&cfg, 0, sizeof(HIIDRA_CFG));

            cfg.Magicbytes = HIIDRA_MAGIC;
            cfg.Version = HIIDRA_CFG_VERSION;

            if (isVGAEnabled())
                cfg.Config |= HIIDRA_CFG_VGA;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable WiFi", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_WIFI;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable Bluetooth", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_BT;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable USB saves", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_USBSAVES;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable GC2Wiimote", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_GC2WIIMOTE;
            
            tempVal = 0;
            thisView->gameConfig.getValue("Patch MX chip", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_PATCHMX;

            tempVal = HIIDRA_PADREAD_AUTO;
            thisView->gameConfig.getValue("PADRead Mode", &tempVal);
            cfg.PADReadMode = tempVal;

            tempVal = HIIDRA_DEFLICKER_AUTO;
            thisView->gameConfig.getValue("Deflicker Mode", &tempVal);
            cfg.DeflickerMode = tempVal;
            
            //Handle cheats
            tempVal = 0;
            thisView->gameConfig.getValue("Enable Cheats", &tempVal);
            if (tempVal) {
                cfg.Config |= HIIDRA_CFG_CHEATS;

                //Read cheats
                getcwd(oldPath, PATH_MAX);
                chdir("/");
                thisView->cheatCodes.parseFile(gc.cheatPath);
                chdir(oldPath);

                for (auto& cheat : thisView->cheatCodes) {
                    std::string cheatConfName = "Cheat_" + Cheat::getCheatNameHash(cheat.first);
                    thisView->gameConfig.getValue(cheatConfName.c_str(), &tempVal);
                    thisView->cheatCodes.setCheatActive(cheat.first, tempVal);
                }

                cheats = thisView->cheatCodes.generateGCT();
            }

            tempVal = HIIDRA_HOOKTYPE_VBI;
            thisView->gameConfig.getValue("Cheats Hooktype", &tempVal);
            cfg.Hooktype = tempVal;

            strcpy(cfg.GamePath, gc.path.c_str());

            bootWiiGame(cfg, gc.gameID, gc.gameIDString, cheats, forceReinstall);
        } else if (thisView->titlesType == GC_GAME) {
            int tempVal;
            NIN_CFG cfg;
            memset(&cfg, 0, sizeof(NIN_CFG));

            cfg.Magicbytes = NIN_MAGIC;
            cfg.Version = NIN_CFG_VERSION;
            cfg.Config = NIN_CFG_AUTO_BOOT;
            cfg.Config |= NIN_CFG_USB;
            cfg.Config |= NIN_CFG_HID;

            tempVal = 0;
            thisView->gameConfig.getValue("Force progressive", &tempVal);
            if (tempVal)
                cfg.Config |= NIN_CFG_FORCE_PROG;

            tempVal = 0;
            thisView->gameConfig.getValue("Force widescreen", &tempVal);
            if (tempVal)
                cfg.Config |= NIN_CFG_FORCE_WIDE;

            tempVal = 0;
            thisView->gameConfig.getValue("Force RVL-DD stretching", &tempVal);
            if (tempVal || (cfg.Config & NIN_CFG_FORCE_WIDE))
                RVLDD::setStretch(1);
            else
                RVLDD::setStretch(0);

            tempVal = 0;
            thisView->gameConfig.getValue("Native SI", &tempVal);
            if (tempVal)
                cfg.Config |= NIN_CFG_NATIVE_SI;

            tempVal = NIN_VID_AUTO;
            thisView->gameConfig.getValue("Video mode", &tempVal);
            cfg.VideoMode = tempVal;

            tempVal = NIN_LAN_AUTO;
            thisView->gameConfig.getValue("Language", &tempVal);
            cfg.Language = tempVal;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable Cheats", &tempVal);
            if (tempVal)
                cfg.Config |= NIN_CFG_CHEATS;

            tempVal = 1;
            thisView->gameConfig.getValue("Memory Card Emulation", &tempVal);
            if (tempVal)
                cfg.Config |= NIN_CFG_MEMCARDEMU;

            tempVal = 4;
            thisView->gameConfig.getValue("Max Pads", &tempVal);
            cfg.MaxPads = tempVal;

            tempVal = 0;
            thisView->gameConfig.getValue("Video Width", &tempVal);
            cfg.VideoScale = tempVal - 600;

            //GC+2.0 map
            thisView->gameConfig.getValue("GCPMap_A", &thisView->gcpMap[GCP_MAP_BUTTON_A_ID]);
            thisView->gameConfig.getValue("GCPMap_B", &thisView->gcpMap[GCP_MAP_BUTTON_B_ID]);
            thisView->gameConfig.getValue("GCPMap_X", &thisView->gcpMap[GCP_MAP_BUTTON_X_ID]);
            thisView->gameConfig.getValue("GCPMap_Y", &thisView->gcpMap[GCP_MAP_BUTTON_Y_ID]);
            thisView->gameConfig.getValue("GCPMap_UP", &thisView->gcpMap[GCP_MAP_BUTTON_DU_ID]);
            thisView->gameConfig.getValue("GCPMap_DOWN", &thisView->gcpMap[GCP_MAP_BUTTON_DD_ID]);
            thisView->gameConfig.getValue("GCPMap_RIGHT", &thisView->gcpMap[GCP_MAP_BUTTON_DR_ID]);
            thisView->gameConfig.getValue("GCPMap_LEFT", &thisView->gcpMap[GCP_MAP_BUTTON_DL_ID]);
            thisView->gameConfig.getValue("GCPMap_Z", &thisView->gcpMap[GCP_MAP_BUTTON_Z_ID]);
            thisView->gameConfig.getValue("GCPMap_R", &thisView->gcpMap[GCP_MAP_BUTTON_RD_ID]);
            thisView->gameConfig.getValue("GCPMap_L", &thisView->gcpMap[GCP_MAP_BUTTON_LD_ID]);
            thisView->gameConfig.getValue("GCPMap_START", &thisView->gcpMap[GCP_MAP_BUTTON_ST_ID]);

            //Set mapping if GC+2.0 is connected
            if (GCPlus::isV2()) {
                GCPlus::unlock();
                GCPlus::setMapping(thisView->gcpMap);
                GCPlus::lock();
            }

            strcpy(cfg.GamePath, gc.path.c_str());
            cfg.GameID = gc.gameID;
            bootGCGame(cfg);
        } else if (thisView->titlesType == WII_VC) {
            int tempVal;
            HIIDRA_CFG cfg;
            memset(&cfg, 0, sizeof(HIIDRA_CFG));

            cfg.Magicbytes = HIIDRA_MAGIC;
            cfg.Version = HIIDRA_CFG_VERSION;

            if (isVGAEnabled())
                cfg.Config |= HIIDRA_CFG_VGA;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable WiFi", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_WIFI;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable Bluetooth", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_BT;

            cfg.Config |= HIIDRA_CFG_USBSAVES;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable GC2Wiimote", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_GC2WIIMOTE;

            tempVal = 0;
            thisView->gameConfig.getValue("Patch MX chip", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_PATCHMX;

            tempVal = HIIDRA_PADREAD_AUTO;
            thisView->gameConfig.getValue("PADRead Mode", &tempVal);
            cfg.PADReadMode = tempVal;

            tempVal = HIIDRA_DEFLICKER_AUTO;
            thisView->gameConfig.getValue("Deflicker Mode", &tempVal);
            cfg.DeflickerMode = tempVal;
            
            //Handle cheats
            tempVal = 0;
            thisView->gameConfig.getValue("Enable Cheats", &tempVal);
            if (tempVal) {
                cfg.Config |= HIIDRA_CFG_CHEATS;

                //Read cheats
                getcwd(oldPath, PATH_MAX);
                chdir("/");
                thisView->cheatCodes.parseFile(gc.cheatPath);
                chdir(oldPath);

                for (auto& cheat : thisView->cheatCodes) {
                    std::string cheatConfName = "Cheat_" + Cheat::getCheatNameHash(cheat.first);
                    thisView->gameConfig.getValue(cheatConfName.c_str(), &tempVal);
                    thisView->cheatCodes.setCheatActive(cheat.first, tempVal);
                }

                cheats = thisView->cheatCodes.generateGCT();
            }

            tempVal = HIIDRA_HOOKTYPE_VBI;
            thisView->gameConfig.getValue("Cheats Hooktype", &tempVal);
            cfg.Hooktype = tempVal;

            strcpy(cfg.GamePath, gc.path.c_str());

            bootWiiGame(cfg, gc.gameID, gc.gameIDString, cheats, forceReinstall);
        } else if (thisView->titlesType == WII_CHANNEL) {
            int tempVal;
            HIIDRA_CFG cfg;
            memset(&cfg, 0, sizeof(HIIDRA_CFG));

            cfg.Magicbytes = HIIDRA_MAGIC;
            cfg.Version = HIIDRA_CFG_VERSION;

            if (isVGAEnabled())
                cfg.Config |= HIIDRA_CFG_VGA;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable WiFi", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_WIFI;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable Bluetooth", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_BT;

            cfg.Config |= HIIDRA_CFG_USBSAVES;

            tempVal = 0;
            thisView->gameConfig.getValue("Enable GC2Wiimote", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_GC2WIIMOTE;

            tempVal = 0;
            thisView->gameConfig.getValue("Patch MX chip", &tempVal);
            if (tempVal)
                cfg.Config |= HIIDRA_CFG_PATCHMX;

            tempVal = HIIDRA_PADREAD_AUTO;
            thisView->gameConfig.getValue("PADRead Mode", &tempVal);
            cfg.PADReadMode = tempVal;

            tempVal = HIIDRA_DEFLICKER_AUTO;
            thisView->gameConfig.getValue("Deflicker Mode", &tempVal);
            cfg.DeflickerMode = tempVal;
            
            //Handle cheats
            tempVal = 0;
            thisView->gameConfig.getValue("Enable Cheats", &tempVal);
            if (tempVal) {
                cfg.Config |= HIIDRA_CFG_CHEATS;

                //Read cheats
                getcwd(oldPath, PATH_MAX);
                chdir("/");
                thisView->cheatCodes.parseFile(gc.cheatPath);
                chdir(oldPath);

                for (auto& cheat : thisView->cheatCodes) {
                    std::string cheatConfName = "Cheat_" + Cheat::getCheatNameHash(cheat.first);
                    thisView->gameConfig.getValue(cheatConfName.c_str(), &tempVal);
                    thisView->cheatCodes.setCheatActive(cheat.first, tempVal);
                }

                cheats = thisView->cheatCodes.generateGCT();
            }

            tempVal = HIIDRA_HOOKTYPE_VBI;
            thisView->gameConfig.getValue("Cheats Hooktype", &tempVal);
            cfg.Hooktype = tempVal;

            strcpy(cfg.GamePath, gc.path.c_str());

            bootWiiGame(cfg, gc.gameID, gc.gameIDString, cheats, forceReinstall);
        }
    } catch (std::out_of_range& e) {

    }
    return 0;
}

int GuiGamesView::lua_openGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Open config
    try {
        thisView->openGameConfig(idx);
    } catch (std::out_of_range& e) {
        return luaL_error(L, "Can't find game at index %u", idx);
    }

    return 0;
}

int GuiGamesView::lua_saveGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);


    //Save config
    thisView->gameConfig.save();

    return 0;
}

int GuiGamesView::lua_setGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->gameConfig.setValue(luaL_checkstring(L, 1), luaL_checkinteger(L, 2));

    return 0;
}

int GuiGamesView::lua_getGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    int val = 0;

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->gameConfig.getValue(luaL_checkstring(L, 1), &val);
    lua_pushinteger(L, val);

    return 1;
}

int GuiGamesView::lua_readGameCheats(lua_State* L) {
    int index = 1;
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_getglobal(L, "_gamesList");
    std::vector<GameContainer>* gamesList = (std::vector<GameContainer>*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Open cheat
    try {
        GameContainer& gc = gamesList->at(idx);
        char oldPath[PATH_MAX];
        getcwd(oldPath, PATH_MAX);
        chdir("/");
        thisView->cheatCodes.parseFile(gc.cheatPath);
        chdir(oldPath);
    } catch (std::out_of_range& e) {
        return luaL_error(L, "Can't find game at index %u", idx);
    }

    lua_newtable(L);

    for (auto& cheat : thisView->cheatCodes) {
        luaSetArrayStringField(L, index++, cheat.first.c_str());
    }

    return 1;
}

int GuiGamesView::lua_getCheatNameHash(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushstring(L, Cheat::getCheatNameHash(luaL_checkstring(L, 1)).c_str());

    return 1;
}

int GuiGamesView::lua_openGC2WiimoteGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    u32 idx = luaL_checkinteger(L, 1);

    //Open config
    try {
        thisView->openGameGC2WiimoteConfig(idx);
    } catch (std::out_of_range& e) {
        return luaL_error(L, "Can't find game at index %u", idx);
    }

    return 0;
}

int GuiGamesView::lua_saveGC2WiimoteGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    //Save config
    if (thisView->lastGC2WiimoteConfigPath != "") {
        FILE* fp = fopen(thisView->lastGC2WiimoteConfigPath.c_str(), "wb");
        if (fp != NULL) {
            fwrite(&(thisView->emuConfig), 1, sizeof(WMEmuConfig_t), fp);
            fclose(fp);
        }
    }

    return 0;
}

int GuiGamesView::lua_setGC2WiimoteGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteMapping.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteMapping.end()) {
        if (it->second)
            *(it->second) = luaL_checkinteger(L, 2);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 0;
}

int GuiGamesView::lua_getGC2WiimoteGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteMapping.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteMapping.end()) {
        if (it->second)
            lua_pushinteger(L, *(it->second));
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 1;
}

int GuiGamesView::lua_setGC2WiimoteGameConfigModifier(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteModifiers.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteModifiers.end()) {
        if (it->second)
            *(it->second) = luaL_checkinteger(L, 2);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 0;
}

int GuiGamesView::lua_getGC2WiimoteGameConfigModifier(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteModifiers.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteModifiers.end()) {
        if (it->second)
            lua_pushinteger(L, *(it->second));
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 1;
}

int GuiGamesView::lua_setGC2WiimoteGameConfigNegModifier(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteNegModifiers.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteNegModifiers.end()) {
        if (it->second)
            *(it->second) = luaL_checkinteger(L, 2);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 0;
}

int GuiGamesView::lua_getGC2WiimoteGameConfigNegModifier(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gc2wiimoteNegModifiers.find(luaL_checkstring(L, 1));

    if (it != thisView->gc2wiimoteNegModifiers.end()) {
        if (it->second)
            lua_pushinteger(L, *(it->second));
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 1;
}

int GuiGamesView::lua_getGC2WiimoteMapString(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    std::string tempString;
    u32 value = 0, modifier = 0, negModifier = 0;

    auto itValue = thisView->gc2wiimoteMapping.find(luaL_checkstring(L, 1));

    if (itValue != thisView->gc2wiimoteMapping.end()) {
        if (itValue->second)
            value = *(itValue->second);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    auto itModifier = thisView->gc2wiimoteModifiers.find(luaL_checkstring(L, 1));

    if (itModifier != thisView->gc2wiimoteModifiers.end()) {
        if (itModifier->second)
            modifier = *(itModifier->second);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    auto itNegModifier = thisView->gc2wiimoteNegModifiers.find(luaL_checkstring(L, 1));

    if (itNegModifier != thisView->gc2wiimoteNegModifiers.end()) {
        if (itNegModifier->second)
            negModifier = *(itNegModifier->second);
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    if (value == 0) {
        tempString += "None";
    } else {
        if (modifier != 0 || negModifier != 0) {
            for (int i = 0; i < NPADNAMES; i++) {
                if (modifier & (1 << i)) {
                    tempString += PADNames[i] + "&";
                    break;
                }
            }
            for (int i = 0; i < NPADNAMES; i++) {
                if (negModifier & (1 << i)) {
                    tempString += "!" + PADNames[i] + "&";
                    break;
                }
            }
            tempString += "(";
        }
        for (int i = 0; i < NPADNAMES; i++) {
            if (value & (1 << i)) {
                //Add the separator if there are more buttons to print
                if ((value & ((1 << i) - 1)) && (i > 0)) {
                    tempString += "|";
                }
                tempString += PADNames[i];

            }
        }
        if (modifier != 0 || negModifier != 0) {
            tempString += ")";
        }
    }

    lua_pushstring(L, tempString.c_str());
    return 1;
}

int GuiGamesView::lua_getGC2WiimoteMapValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    u32 held = 0;

    lua_getfield(L, 1, "BUTTON_LEFT");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_LEFT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_RIGHT");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_RIGHT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_DOWN");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_DOWN;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_UP");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_UP;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_Z");
    if (lua_toboolean(L, -1))
        held |= PAD_TRIGGER_Z;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_R");
    if (lua_toboolean(L, -1))
        held |= PAD_TRIGGER_R;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_L");
    if (lua_toboolean(L, -1))
        held |= PAD_TRIGGER_L;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_A");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_A;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_B");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_B;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_X");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_X;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_Y");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_Y;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_START");
    if (lua_toboolean(L, -1))
        held |= PAD_BUTTON_START;
    lua_remove(L, -1);
    lua_getfield(L, 1, "STICK");
    if (lua_toboolean(L, -1))
        held |= SSTICK;
    lua_remove(L, -1);
    lua_getfield(L, 1, "SUBSTICK");
    if (lua_toboolean(L, -1))
        held |= CSTICK;
    lua_remove(L, -1);
    lua_getfield(L, 1, "STICKRIGHT");
    if (lua_toboolean(L, -1))
        held |= SSTICKRIGHT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "STICKLEFT");
    if (lua_toboolean(L, -1))
        held |= SSTICKLEFT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "STICKUP");
    if (lua_toboolean(L, -1))
        held |= SSTICKUP;
    lua_remove(L, -1);
    lua_getfield(L, 1, "STICKDOWN");
    if (lua_toboolean(L, -1))
        held |= SSTICKDOWN;
    lua_remove(L, -1);
    lua_getfield(L, 1, "SUBSTICKRIGHT");
    if (lua_toboolean(L, -1))
        held |= CSTICKRIGHT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "SUBSTICKLEFT");
    if (lua_toboolean(L, -1))
        held |= CSTICKLEFT;
    lua_remove(L, -1);
    lua_getfield(L, 1, "SUBSTICKUP");
    if (lua_toboolean(L, -1))
        held |= CSTICKUP;
    lua_remove(L, -1);
    lua_getfield(L, 1, "SUBSTICKDOWN");
    if (lua_toboolean(L, -1))
        held |= CSTICKDOWN;
    lua_remove(L, -1);

    lua_pushinteger(L, held);
    return 1;
}

int GuiGamesView::lua_openGCPMapGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->gameConfig.getValue("GCPMap_A", &thisView->gcpMap[GCP_MAP_BUTTON_A_ID]);
    thisView->gameConfig.getValue("GCPMap_B", &thisView->gcpMap[GCP_MAP_BUTTON_B_ID]);
    thisView->gameConfig.getValue("GCPMap_X", &thisView->gcpMap[GCP_MAP_BUTTON_X_ID]);
    thisView->gameConfig.getValue("GCPMap_Y", &thisView->gcpMap[GCP_MAP_BUTTON_Y_ID]);
    thisView->gameConfig.getValue("GCPMap_UP", &thisView->gcpMap[GCP_MAP_BUTTON_DU_ID]);
    thisView->gameConfig.getValue("GCPMap_DOWN", &thisView->gcpMap[GCP_MAP_BUTTON_DD_ID]);
    thisView->gameConfig.getValue("GCPMap_RIGHT", &thisView->gcpMap[GCP_MAP_BUTTON_DR_ID]);
    thisView->gameConfig.getValue("GCPMap_LEFT", &thisView->gcpMap[GCP_MAP_BUTTON_DL_ID]);
    thisView->gameConfig.getValue("GCPMap_Z", &thisView->gcpMap[GCP_MAP_BUTTON_Z_ID]);
    thisView->gameConfig.getValue("GCPMap_R", &thisView->gcpMap[GCP_MAP_BUTTON_RD_ID]);
    thisView->gameConfig.getValue("GCPMap_L", &thisView->gcpMap[GCP_MAP_BUTTON_LD_ID]);
    thisView->gameConfig.getValue("GCPMap_START", &thisView->gcpMap[GCP_MAP_BUTTON_ST_ID]);

    return 0;
}

int GuiGamesView::lua_saveGCPMapGameConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    thisView->gameConfig.setValue("GCPMap_A", thisView->gcpMap[GCP_MAP_BUTTON_A_ID]);
    thisView->gameConfig.setValue("GCPMap_B", thisView->gcpMap[GCP_MAP_BUTTON_B_ID]);
    thisView->gameConfig.setValue("GCPMap_X", thisView->gcpMap[GCP_MAP_BUTTON_X_ID]);
    thisView->gameConfig.setValue("GCPMap_Y", thisView->gcpMap[GCP_MAP_BUTTON_Y_ID]);
    thisView->gameConfig.setValue("GCPMap_UP", thisView->gcpMap[GCP_MAP_BUTTON_DU_ID]);
    thisView->gameConfig.setValue("GCPMap_DOWN", thisView->gcpMap[GCP_MAP_BUTTON_DD_ID]);
    thisView->gameConfig.setValue("GCPMap_RIGHT", thisView->gcpMap[GCP_MAP_BUTTON_DR_ID]);
    thisView->gameConfig.setValue("GCPMap_LEFT", thisView->gcpMap[GCP_MAP_BUTTON_DL_ID]);
    thisView->gameConfig.setValue("GCPMap_Z", thisView->gcpMap[GCP_MAP_BUTTON_Z_ID]);
    thisView->gameConfig.setValue("GCPMap_R", thisView->gcpMap[GCP_MAP_BUTTON_RD_ID]);
    thisView->gameConfig.setValue("GCPMap_L", thisView->gcpMap[GCP_MAP_BUTTON_LD_ID]);
    thisView->gameConfig.setValue("GCPMap_START", thisView->gcpMap[GCP_MAP_BUTTON_ST_ID]);

    return 0;
}

int GuiGamesView::lua_setGCPMapGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gcpMapStringToPad.find(luaL_checkstring(L, 1));

    printf("setGCPMapGameConfigValue(%s, %u)\n", luaL_checkstring(L, 1), luaL_checkinteger(L, 2));

    if (it != thisView->gcpMapStringToPad.end()) {
        //Swap other mapped buttons
        for (int i = 0; i < GCP_MAP_N_BUTTONS; i++) {
            if (thisView->gcpMap[i] == it->second) {
                printf("gcpMap[%d] = 0;\n", i);
                thisView->gcpMap[i] = thisView->gcpMap[luaL_checkinteger(L, 2)];
            }
        }

        //Map button
        printf("gcpMap[%d] = %04X;\n", luaL_checkinteger(L, 2), it->second);
        thisView->gcpMap[luaL_checkinteger(L, 2)] = it->second;
    } else {
        return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
    }

    return 0;
}

int GuiGamesView::lua_getGCPMapGameConfigValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    auto it = thisView->gcpMapStringToPad.find(luaL_checkstring(L, 1));

    if (it != thisView->gcpMapStringToPad.end()) {
        for (int i = 0; i < GCP_MAP_N_BUTTONS; i++) {
            if (thisView->gcpMap[i] == it->second) {
                lua_pushinteger(L, i);
                return 1;
            }
        }
        lua_pushinteger(L, -1);
        return 1;
    }

    return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
}

int GuiGamesView::lua_getGCPMapString(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_getglobal(L, "_this");
    GuiGamesView* thisView = (GuiGamesView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    std::string tempString = "None";

    auto itValue = thisView->gcpMapStringToPad.find(luaL_checkstring(L, 1));

    if (itValue != thisView->gcpMapStringToPad.end()) {
        for (int i = 0; i < GCP_MAP_N_BUTTONS; i++) {
            if (thisView->gcpMap[i] == itValue->second) {
                auto temp = thisView->gcpMapIDToString.find(i);
                if (temp != thisView->gcpMapIDToString.end()) {
                    tempString = temp->second;
                    break;
                }
            }
        }

        lua_pushstring(L, tempString.c_str());
        return 1;
    }

    return luaL_error(L, "unknown key %s", luaL_checkstring(L, 1));
}

int GuiGamesView::lua_getGCPMapValue(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    int ID = -1;

    lua_getfield(L, 1, "BUTTON_LEFT");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_DL_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_RIGHT");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_DR_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_DOWN");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_DD_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_UP");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_DU_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_Z");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_Z_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_R");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_RD_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "TRIGGER_L");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_LD_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_A");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_A_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_B");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_B_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_X");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_X_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_Y");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_Y_ID;
    lua_remove(L, -1);
    lua_getfield(L, 1, "BUTTON_START");
    if (ID < 0 && lua_toboolean(L, -1))
        ID = GCP_MAP_BUTTON_ST_ID;
    lua_remove(L, -1);

    lua_pushinteger(L, ID);
    return 1;
}
