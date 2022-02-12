#pragma once

#include <limits.h>
#include <lua.hpp>
#include <libgui.h>

class GuiLuaElement : public GuiElement {
    protected:
        lua_State* L;
        bool hasToLoadScript;
        std::string scriptPath;
        char basePath[PATH_MAX];

        virtual void initLUA();

        static int compilerWriter(lua_State* L, const void* p, size_t size, void* u);

    public:
        GuiLuaElement();
        ~GuiLuaElement();

        void onActiveEvent();

        virtual void setPath(const char* path);
        virtual void loadScript(const char* script);
        virtual void loadScriptFile(const char* script);
        virtual void draw(bool onFocus);
        virtual void handleInputs(bool onFocus);
        virtual Vector2 getDimensions();
        virtual void copyIncomingTable(lua_State* inState);
        virtual void handleMessage(lua_State* inState);
};
