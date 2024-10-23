#include <gccore.h>
#include <lua.hpp>
#include <audiogc.hpp>
#include <unordered_set>
#include "luasupport.h"

namespace LUALibSfx {
    static std::unordered_set<audiogc::player*> players;

    static int open(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = new audiogc::player(audiogc::type::mp3, luaL_checkstring(L, 1),  audiogc::mode::store);

        if (player) {
            players.insert(player);
            lua_pushinteger(L, (int)player);
        } else {
            lua_pushinteger(L, 0);
        }

        return 1;
    }
    
    static int close(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            players.erase(player);
            delete player;
        }

        return 0;
    }

    static int play(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->play();
        }

        return 0;
    }

    static int pause(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->pause();
        }

        return 0;
    }

    static int stop(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->stop();
        }

        return 0;
    }

    static int getChannelCount(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushinteger(L, player->get_channel_count());
        } else {
            lua_pushinteger(L, 0);
        }

        return 1;
    }

    static int getPitch(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushnumber(L, player->get_pitch());
        } else {
            lua_pushnumber(L, 0);
        }

        return 1;
    }

    static int getVolume(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushinteger(L, player->get_volume());
        } else {
            lua_pushinteger(L, 0);
        }

        return 1;
    }

    static int isLooping(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushboolean(L, player->is_looping());
        } else {
            lua_pushboolean(L, false);
        }

        return 1;
    }

    static int isPlaying(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushboolean(L, player->is_playing());
        } else {
            lua_pushboolean(L, false);
        }

        return 1;
    }

    static int seek(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 2) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->seek(luaL_checkinteger(L, 2));
        }

        return 0;
    }

    static int setLooping(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 2) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->set_looping(lua_toboolean(L, 2));
        }

        return 0;
    }

    static int setPitch(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 2) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->set_pitch(luaL_checknumber(L, 2));
        }

        return 0;
    }

    static int setVolume(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 2) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            player->set_volume(luaL_checkinteger(L, 2));
        }

        return 0;
    }

    static int tell(lua_State* L) {
        int argc = lua_gettop(L);
        if (argc != 1) {
            return luaL_error(L, "wrong number of arguments");
        }

        audiogc::player* player = (audiogc::player*)luaL_checkinteger(L, 1);

        if (players.contains(player)) {
            lua_pushnumber(L, player->tell());
        } else {
            lua_pushnumber(L, 0);
        }

        return 1;
    }

    static const luaL_Reg functions[] = {
        {"open", open},
        {"close", close},
        {"play", play},
        {"pause", pause},
        {"stop", stop},
        {"getChannelCount", getChannelCount},
        {"getPitch", getPitch},
        {"getVolume", getVolume},
        {"isLooping", isLooping},
        {"isPlaying", isPlaying},
        {"seek", seek},
        {"setLooping", setLooping},
        {"setPitch", setPitch},
        {"setVolume", setVolume},
        {"tell", tell},
        {NULL, NULL}
    };

    void registerLibrary(lua_State* L) {
        lua_newtable(L);
        luaL_setfuncs(L, functions, 0);
        lua_setglobal(L, "Sfx");
    }
};