#pragma once

#include <limits.h>
#include <lua.hpp>
#include <libgui.h>
#include <map>
#include <string>
#include "config.h"
#include "gc2wiimote.h"
#include "guiluaelement.h"

class GuiHBView : public GuiLuaElement {
    private:
        void initLUA();

        int coverWidth;
        int coverHeight;

    public:
        GuiHBView();
        ~GuiHBView();

        static int lua_setCoverSize(lua_State* L);
        static int lua_drawHBCover(lua_State* L);
        static int lua_getHBCount(lua_State* L);
        static int lua_getHBName(lua_State* L);
        static int lua_bootHB(lua_State* L);
};
