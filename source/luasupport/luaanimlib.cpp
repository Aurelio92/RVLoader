#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "luasupport.h"

static int lua_Anim_new(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = new Animation<Vector2>();
    lua_pushinteger(L, (u32)anim);

    return 1;
}

static int lua_Anim_delete(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    if (anim != NULL)
        delete anim;

    return 0;
}

static int lua_Anim_resume(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    if (anim != NULL)
        anim->resume();

    return 0;
}

static int lua_Anim_pause(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    if (anim != NULL)
        anim->pause();

    return 0;
}

static int lua_Anim_stop(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    if (anim != NULL)
        anim->stop();

    return 0;
}

static int lua_Anim_reset(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    if (anim != NULL)
        anim->reset();

    return 0;
}

static int lua_Anim_addStep(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 6) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    u32 time = luaL_checkinteger(L, 2);
    u32 x1 = luaL_checkinteger(L, 3);
    u32 y1 = luaL_checkinteger(L, 4);
    u32 x2 = luaL_checkinteger(L, 5);
    u32 y2 = luaL_checkinteger(L, 6);

    if (anim != NULL) {
        anim->addStep(millisecs_to_ticks(time), Vector2(x1, y1), Vector2(x2, y2));
    }

    return 0;
}

static int lua_Anim_addReturnToHomeStep(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    u32 time = luaL_checkinteger(L, 2);

    if (anim != NULL) {
        anim->addReturnToHomeStep(millisecs_to_ticks(time));
    }

    return 0;
}

static int lua_Anim_getPosition(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Animation<Vector2>* anim = (Animation<Vector2>*)luaL_checkinteger(L, 1);
    Vector2 pos(0, 0);
    if (anim != NULL) {
        anim->setOutput(&pos);
        anim->animate();
        anim->setOutput(NULL);
    }
    lua_newtable(L);
    luaSetTableIntField(L, "x", pos.x);
    luaSetTableIntField(L, "y", pos.y);

    return 1;
}

static const luaL_Reg Anim_functions[] = {
    {"new", lua_Anim_new},
    {"delete", lua_Anim_delete},
    {"resume", lua_Anim_resume},
    {"pause", lua_Anim_pause},
    {"stop", lua_Anim_stop},
    {"reset", lua_Anim_reset},
    {"addStep", lua_Anim_addStep},
    {"addReturnToHomeStep", lua_Anim_addReturnToHomeStep},
    {"getPosition", lua_Anim_getPosition},
    {NULL, NULL}
};

void luaRegisterAnimLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Anim_functions, 0);
    lua_setglobal(L, "Anim");
}
