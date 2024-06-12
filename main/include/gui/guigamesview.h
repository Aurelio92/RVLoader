#pragma once

#include <limits.h>
#include <lua.hpp>
#include <libgui.h>
#include <map>
#include <string>
#include "config.h"
#include "gc2wiimote.h"
#include "guiluaelement.h"
#include "gcplus.h"
#include "cheats.h"

typedef enum {
    GC_GAME = 0,
    WII_GAME,
    WII_CHANNEL,
    WII_VC
} TitleType;

class GuiGamesView : public GuiLuaElement {
    private:
        void initLUA();

        int coverWidth;
        int coverHeight;
        TitleType titlesType;

        Config gameConfig;

        Cheat cheatCodes;

        //GC2Wiimote
        WMEmuConfig_t emuConfig;
        std::string lastGC2WiimoteConfigPath;
        std::unordered_map<std::string, u32*> gc2wiimoteMapping = {
            {"Wiimote orientation", &emuConfig.orientation},
            {"Wiimote extension", &emuConfig.extension},
            {"Motion plus enabled", &emuConfig.mpEnabled},
            {"WM IR mode", &emuConfig.IRMode},
            {"WM IR", &emuConfig.mapping.WM_IR},
            {"WM IR timeout", &emuConfig.IRTimeout},
            {"WM DPAD-U", &emuConfig.mapping.WM_DU},
            {"WM DPAD-D", &emuConfig.mapping.WM_DD},
            {"WM DPAD-R", &emuConfig.mapping.WM_DR},
            {"WM DPAD-L", &emuConfig.mapping.WM_DL},
            {"WM A", &emuConfig.mapping.WM_A},
            {"WM B", &emuConfig.mapping.WM_B},
            {"WM 1", &emuConfig.mapping.WM_1},
            {"WM 2", &emuConfig.mapping.WM_2},
            {"WM Plus", &emuConfig.mapping.WM_Plus},
            {"WM Minus", &emuConfig.mapping.WM_Minus},
            {"WM Home", &emuConfig.mapping.WM_Home},
            {"WM Shake X", &emuConfig.mapping.WM_ShakeX},
            {"WM Shake Y", &emuConfig.mapping.WM_ShakeY},
            {"WM Shake Z", &emuConfig.mapping.WM_ShakeZ},
            {"NU Stick", &emuConfig.mapping.NU_Stick},
            {"NU C", &emuConfig.mapping.NU_C},
            {"NU Z", &emuConfig.mapping.NU_Z},
            {"NU Shake X", &emuConfig.mapping.NU_ShakeX},
            {"NU Shake Y", &emuConfig.mapping.NU_ShakeY},
            {"NU Shake Z", &emuConfig.mapping.NU_ShakeZ},
            {"CC DPAD-U", &emuConfig.mapping.CC_DU},
            {"CC DPAD-D", &emuConfig.mapping.CC_DD},
            {"CC DPAD-R", &emuConfig.mapping.CC_DR},
            {"CC DPAD-L", &emuConfig.mapping.CC_DL},
            {"CC A", &emuConfig.mapping.CC_A},
            {"CC B", &emuConfig.mapping.CC_B},
            {"CC X", &emuConfig.mapping.CC_X},
            {"CC Y", &emuConfig.mapping.CC_Y},
            {"CC R analog", &emuConfig.mapping.CC_RA},
            {"CC L analog", &emuConfig.mapping.CC_LA},
            {"CC R digital", &emuConfig.mapping.CC_RD},
            {"CC L digital", &emuConfig.mapping.CC_LD},
            {"CC Plus", &emuConfig.mapping.CC_Plus},
            {"CC Minus", &emuConfig.mapping.CC_Minus},
            {"CC Home", &emuConfig.mapping.CC_Home},
            {"CC L Stick", &emuConfig.mapping.CC_LStick},
            {"CC R Stick", &emuConfig.mapping.CC_RStick},
        };

        std::unordered_map<std::string, u32*> gc2wiimoteModifiers = {
            {"WM IR", &emuConfig.modifiers.WM_IR},
            {"WM DPAD-U", &emuConfig.modifiers.WM_DU},
            {"WM DPAD-D", &emuConfig.modifiers.WM_DD},
            {"WM DPAD-R", &emuConfig.modifiers.WM_DR},
            {"WM DPAD-L", &emuConfig.modifiers.WM_DL},
            {"WM A", &emuConfig.modifiers.WM_A},
            {"WM B", &emuConfig.modifiers.WM_B},
            {"WM 1", &emuConfig.modifiers.WM_1},
            {"WM 2", &emuConfig.modifiers.WM_2},
            {"WM Plus", &emuConfig.modifiers.WM_Plus},
            {"WM Minus", &emuConfig.modifiers.WM_Minus},
            {"WM Home", &emuConfig.modifiers.WM_Home},
            {"WM Shake X", &emuConfig.modifiers.WM_ShakeX},
            {"WM Shake Y", &emuConfig.modifiers.WM_ShakeY},
            {"WM Shake Z", &emuConfig.modifiers.WM_ShakeZ},
            {"NU Stick", &emuConfig.modifiers.NU_Stick},
            {"NU C", &emuConfig.modifiers.NU_C},
            {"NU Z", &emuConfig.modifiers.NU_Z},
            {"NU Shake X", &emuConfig.modifiers.NU_ShakeX},
            {"NU Shake Y", &emuConfig.modifiers.NU_ShakeY},
            {"NU Shake Z", &emuConfig.modifiers.NU_ShakeZ},
            {"CC DPAD-U", &emuConfig.modifiers.CC_DU},
            {"CC DPAD-D", &emuConfig.modifiers.CC_DD},
            {"CC DPAD-R", &emuConfig.modifiers.CC_DR},
            {"CC DPAD-L", &emuConfig.modifiers.CC_DL},
            {"CC A", &emuConfig.modifiers.CC_A},
            {"CC B", &emuConfig.modifiers.CC_B},
            {"CC X", &emuConfig.modifiers.CC_X},
            {"CC Y", &emuConfig.modifiers.CC_Y},
            {"CC R analog", &emuConfig.modifiers.CC_RA},
            {"CC L analog", &emuConfig.modifiers.CC_LA},
            {"CC R digital", &emuConfig.modifiers.CC_RD},
            {"CC L digital", &emuConfig.modifiers.CC_LD},
            {"CC Plus", &emuConfig.modifiers.CC_Plus},
            {"CC Minus", &emuConfig.modifiers.CC_Minus},
            {"CC Home", &emuConfig.modifiers.CC_Home},
            {"CC L Stick", &emuConfig.modifiers.CC_LStick},
            {"CC R Stick", &emuConfig.modifiers.CC_RStick},
        };

        std::unordered_map<std::string, u32*> gc2wiimoteNegModifiers = {
            {"WM IR", &emuConfig.negModifiers.WM_IR},
            {"WM DPAD-U", &emuConfig.negModifiers.WM_DU},
            {"WM DPAD-D", &emuConfig.negModifiers.WM_DD},
            {"WM DPAD-R", &emuConfig.negModifiers.WM_DR},
            {"WM DPAD-L", &emuConfig.negModifiers.WM_DL},
            {"WM A", &emuConfig.negModifiers.WM_A},
            {"WM B", &emuConfig.negModifiers.WM_B},
            {"WM 1", &emuConfig.negModifiers.WM_1},
            {"WM 2", &emuConfig.negModifiers.WM_2},
            {"WM Plus", &emuConfig.negModifiers.WM_Plus},
            {"WM Minus", &emuConfig.negModifiers.WM_Minus},
            {"WM Home", &emuConfig.negModifiers.WM_Home},
            {"WM Shake X", &emuConfig.negModifiers.WM_ShakeX},
            {"WM Shake Y", &emuConfig.negModifiers.WM_ShakeY},
            {"WM Shake Z", &emuConfig.negModifiers.WM_ShakeZ},
            {"NU Stick", &emuConfig.negModifiers.NU_Stick},
            {"NU C", &emuConfig.negModifiers.NU_C},
            {"NU Z", &emuConfig.negModifiers.NU_Z},
            {"NU Shake X", &emuConfig.negModifiers.NU_ShakeX},
            {"NU Shake Y", &emuConfig.negModifiers.NU_ShakeY},
            {"NU Shake Z", &emuConfig.negModifiers.NU_ShakeZ},
            {"CC DPAD-U", &emuConfig.negModifiers.CC_DU},
            {"CC DPAD-D", &emuConfig.negModifiers.CC_DD},
            {"CC DPAD-R", &emuConfig.negModifiers.CC_DR},
            {"CC DPAD-L", &emuConfig.negModifiers.CC_DL},
            {"CC A", &emuConfig.negModifiers.CC_A},
            {"CC B", &emuConfig.negModifiers.CC_B},
            {"CC X", &emuConfig.negModifiers.CC_X},
            {"CC Y", &emuConfig.negModifiers.CC_Y},
            {"CC R analog", &emuConfig.negModifiers.CC_RA},
            {"CC L analog", &emuConfig.negModifiers.CC_LA},
            {"CC R digital", &emuConfig.negModifiers.CC_RD},
            {"CC L digital", &emuConfig.negModifiers.CC_LD},
            {"CC Plus", &emuConfig.negModifiers.CC_Plus},
            {"CC Minus", &emuConfig.negModifiers.CC_Minus},
            {"CC Home", &emuConfig.negModifiers.CC_Home},
            {"CC L Stick", &emuConfig.negModifiers.CC_LStick},
            {"CC R Stick", &emuConfig.negModifiers.CC_RStick},
        };

        std::unordered_map<int, int> gcpMapPadToID = {
            {PAD_BUTTON_A, GCP_MAP_BUTTON_A_ID},
            {PAD_BUTTON_B, GCP_MAP_BUTTON_B_ID},
            {PAD_BUTTON_X, GCP_MAP_BUTTON_X_ID},
            {PAD_BUTTON_Y, GCP_MAP_BUTTON_Y_ID},
            {PAD_BUTTON_START, GCP_MAP_BUTTON_ST_ID},
            {PAD_BUTTON_LEFT, GCP_MAP_BUTTON_DL_ID},
            {PAD_BUTTON_RIGHT, GCP_MAP_BUTTON_DR_ID},
            {PAD_BUTTON_DOWN, GCP_MAP_BUTTON_DD_ID},
            {PAD_BUTTON_UP, GCP_MAP_BUTTON_DU_ID},
            {PAD_TRIGGER_Z, GCP_MAP_BUTTON_Z_ID},
            {PAD_TRIGGER_R, GCP_MAP_BUTTON_RD_ID},
            {PAD_TRIGGER_L, GCP_MAP_BUTTON_LD_ID},
        };

        std::unordered_map<std::string, int> gcpMapStringToPad = {
            {"A", PAD_BUTTON_A},
            {"B", PAD_BUTTON_B},
            {"X", PAD_BUTTON_X},
            {"Y", PAD_BUTTON_Y},
            {"Start", PAD_BUTTON_START},
            {"Left", PAD_BUTTON_LEFT},
            {"Right", PAD_BUTTON_RIGHT},
            {"Down", PAD_BUTTON_DOWN},
            {"Up", PAD_BUTTON_UP},
            {"Z", PAD_TRIGGER_Z},
            {"R", PAD_TRIGGER_R},
            {"L", PAD_TRIGGER_L},
        };

        std::unordered_map<int, std::string> gcpMapIDToString = {
            {GCP_MAP_BUTTON_A_ID, "A"},
            {GCP_MAP_BUTTON_B_ID, "B"},
            {GCP_MAP_BUTTON_X_ID, "X"},
            {GCP_MAP_BUTTON_Y_ID, "Y"},
            {GCP_MAP_BUTTON_ST_ID, "Start"},
            {GCP_MAP_BUTTON_DL_ID, "Left"},
            {GCP_MAP_BUTTON_DR_ID, "Right"},
            {GCP_MAP_BUTTON_DD_ID, "Down"},
            {GCP_MAP_BUTTON_DU_ID, "Up"},
            {GCP_MAP_BUTTON_Z_ID, "Z"},
            {GCP_MAP_BUTTON_RD_ID, "R"},
            {GCP_MAP_BUTTON_LD_ID, "L"},
        };

        int gcpMap[GCP_MAP_N_BUTTONS];

    public:
        GuiGamesView(TitleType _titlesType);
        ~GuiGamesView();

        void openGameConfig(u32 idx);
        void openGameGC2WiimoteConfig(u32 idx);
        void loadDefaultGC2WiimoteConfig();

        static int lua_setCoverSize(lua_State* L);
        static int lua_drawGameCover(lua_State* L);
        static int lua_drawGameSaveIcon(lua_State* L);
        static int lua_getGamesCount(lua_State* L);
        static int lua_getGameName(lua_State* L);
        static int lua_getGamesType(lua_State* L);
        static int lua_bootGame(lua_State* L);
        static int lua_openGameConfig(lua_State* L);
        static int lua_saveGameConfig(lua_State* L);
        static int lua_setGameConfigValue(lua_State* L);
        static int lua_getGameConfigValue(lua_State* L);
        static int lua_readGameCheats(lua_State* L);
        static int lua_getCheatNameHash(lua_State* L);
        static int lua_openGC2WiimoteGameConfig(lua_State* L);
        static int lua_saveGC2WiimoteGameConfig(lua_State* L);
        static int lua_setGC2WiimoteGameConfigValue(lua_State* L);
        static int lua_getGC2WiimoteGameConfigValue(lua_State* L);
        static int lua_setGC2WiimoteGameConfigModifier(lua_State* L);
        static int lua_getGC2WiimoteGameConfigModifier(lua_State* L);
        static int lua_setGC2WiimoteGameConfigNegModifier(lua_State* L);
        static int lua_getGC2WiimoteGameConfigNegModifier(lua_State* L);
        static int lua_getGC2WiimoteMapString(lua_State* L);
        static int lua_getGC2WiimoteMapValue(lua_State* L);
        static int lua_openGCPMapGameConfig(lua_State* L);
        static int lua_saveGCPMapGameConfig(lua_State* L);
        static int lua_setGCPMapGameConfigValue(lua_State* L);
        static int lua_getGCPMapGameConfigValue(lua_State* L);
        static int lua_getGCPMapString(lua_State* L);
        static int lua_getGCPMapValue(lua_State* L);
};
