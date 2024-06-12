#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "main.h"
#include "luasupport.h"

static int lua_Gfx_RGBA8(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 3 && argc != 4) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    u32 r = luaL_checkinteger(L, 1);
    u32 g = luaL_checkinteger(L, 2);
    u32 b = luaL_checkinteger(L, 3);
    u32 a = 0xFF;
    if (argc == 4) {
        a = luaL_checkinteger(L, 4);
    }
    lua_pushinteger(L, (u32)RGBA8(r, g, b, a));
    return 1;
}

static int lua_Gfx_drawRectangle(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 5) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int w = (int)luaL_checknumber(L, 3);
    int h = (int)luaL_checknumber(L, 4);
    u32 rgba = luaL_checkinteger(L, 5);

    drawRectangle(x, y, w, h, rgba);
    return 0;
}

static int lua_Gfx_draw4ColorsRectangle(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 8) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int w = (int)luaL_checknumber(L, 3);
    int h = (int)luaL_checknumber(L, 4);
    u32 rgba1 = luaL_checkinteger(L, 5);
    u32 rgba2 = luaL_checkinteger(L, 6);
    u32 rgba3 = luaL_checkinteger(L, 7);
    u32 rgba4 = luaL_checkinteger(L, 8);

    draw4ColorsRectangle(x, y, w, h, rgba1, rgba2, rgba3, rgba4);
    return 0;
}

static int lua_Gfx_drawRectangleFromCorners(lua_State* L) {
    f32 coordinates[8];
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }
    
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

    u32 rgba = luaL_checkinteger(L, 2);

    drawRectangleFromCorners(coordinates, RGBA8(0xFF, 0xFF, 0xFF, 0xFF));

    return 0;
}

static int lua_Gfx_drawLine(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 6) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    int x1 = (int)luaL_checknumber(L, 1);
    int y1 = (int)luaL_checknumber(L, 2);
    int x2 = (int)luaL_checknumber(L, 3);
    int y2 = (int)luaL_checknumber(L, 4);
    int w = (int)luaL_checknumber(L, 5);
    u32 rgba = luaL_checkinteger(L, 6);

    drawLine(x1, y1, x2, y2, w, rgba);
    return 0;
}

static int lua_Gfx_translate(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    f32 x = luaL_checknumber(L, 1);
    f32 y = luaL_checknumber(L, 2);
    Gfx::translate(x, y);
    return 0;
}

static int lua_Gfx_rotate(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 3) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    f32 px = luaL_checknumber(L, 1);
    f32 py = luaL_checknumber(L, 2);
    f32 deg = luaL_checknumber(L, 3);
    Gfx::rotate(px, py, deg);
    return 0;
}

static int lua_Gfx_pushMatrix(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Gfx::pushMatrix();
    return 0;
}

static int lua_Gfx_identity(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Gfx::identity();
    return 0;
}

static int lua_Gfx_popMatrix(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Gfx::popMatrix();
    return 0;
}

static int lua_Gfx_pushScissorBox(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    //Get arguments
    u32 w = (int)luaL_checknumber(L, 1);
    u32 h = (int)luaL_checknumber(L, 2);
    Gfx::pushScissorBox(w, h);
    return 0;
}

static int lua_Gfx_pushIdentityScissorBox(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Gfx::pushIdentityScissorBox();
    return 0;
}

static int lua_Gfx_popScissorBox(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Gfx::popScissorBox();
    return 0;
}

static int lua_Gfx_getCurScissorBox(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    Rect r = Gfx::getCurScissorBox();
    lua_newtable(L);
    luaSetTableIntField(L, "x", r.x);
    luaSetTableIntField(L, "y", r.y);
    luaSetTableIntField(L, "width", r.width);
    luaSetTableIntField(L, "height", r.height);

    return 1;
}

static int lua_Gfx_getCurMatrix(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }
    Mtx tempMtx;
    Gfx::getCurMatrix(tempMtx);
    lua_newtable(L);
    for (int i = 0; i < 4; i++) {
        lua_pushinteger(L, i + 1);
        lua_newtable(L);
        for (int j = 0; j < 4; j++) {
            lua_pushinteger(L, j + 1);
            lua_pushnumber(L, tempMtx[i][j]);
            lua_settable(L, -3);
        }
        lua_settable(L, -3);
    }

    return 1;
}

static int lua_Gfx_loadFont(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = new Font(luaL_checkstring(L, 1), luaL_checkinteger(L, 2));
    lua_pushinteger(L, (lua_Integer)font);
    return 1;
}

static int lua_Gfx_print(lua_State* L) {
    int argc = lua_gettop(L);
    int width = 0;
    if (argc != 4) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = (Font*)luaL_checkinteger(L, 1);
    if (font) {
        width = font->print(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4));
    }

    lua_pushinteger(L, width);
    return 1;
}

static int lua_Gfx_setFontColor(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = (Font*)(luaL_checkinteger(L, 1));
    if (font) {
        font->setColor(luaL_checkinteger(L, 2));
    }
    return 0;
}

static int lua_Gfx_getFontColor(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = (Font*)(luaL_checkinteger(L, 1));
    if (font)
        lua_pushinteger(L, font->getColor());
    else
        lua_pushinteger(L, 0);

    return 1;
}

static int lua_Gfx_setFontVerticalAlignment(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = (Font*)(luaL_checkinteger(L, 1));
    if (font) {
        font->setVerticalAlignment((Font::VerticalAlignment)luaL_checkinteger(L, 2));
    }
    return 0;
}

static int lua_Gfx_getFontVerticalAlignment(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    Font* font = (Font*)(luaL_checkinteger(L, 1));
    if (font)
        lua_pushinteger(L, (int)font->getVerticalAlignment());
    else
        lua_pushinteger(L, 0);

    return 1;
}

static int lua_Gfx_getTextWidth(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "wrong number of arguments");
    }
    int width = 0;

    Font* font = (Font*)(luaL_checkinteger(L, 1));
    if (font) {
        width = font->getTextWidth(luaL_checkstring(L, 2));
    }
    lua_pushinteger(L, width);
    return 1;
}


static int lua_Gfx_loadImage(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    GuiImage* img = new GuiImage(luaL_checkstring(L, 1));
    lua_pushinteger(L, (lua_Integer)img);
    return 1;
}

static int lua_Gfx_drawImage(lua_State* L) {
    bool xMirror = false;
    bool yMirror = false;
    int argc = lua_gettop(L);
    if (argc < 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    GuiImage* img = (GuiImage*)luaL_checkinteger(L, 1);
    if (img) {
        Gfx::pushMatrix();
        if (argc == 3 || argc == 5) {
            Gfx::translate(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
        }
        if (argc == 5) {
            xMirror = lua_toboolean(L, 4);
            yMirror = lua_toboolean(L, 5);
        }
        img->draw(false, xMirror, yMirror);
        Gfx::popMatrix();
    }

    return 0;
}

static int lua_Gfx_resizeImage(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 3) {
        return luaL_error(L, "wrong number of arguments");
    }

    GuiImage* img = (GuiImage*)luaL_checkinteger(L, 1);
    if (img) {
        img->setSize((int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3));
    } else {
        luaL_error(L, "wrong pointer to image");
    }

    return 0;
}

static int lua_Gfx_getImageSize(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    GuiImage* img = (GuiImage*)luaL_checkinteger(L, 1);
    if (img) {
        Vector2 dim = img->getDimensions();
        lua_pushinteger(L, dim.x);
        lua_pushinteger(L, dim.y);
    } else {
        luaL_error(L, "wrong pointer to image");
        return 0;
    }

    return 2;
}

static const luaL_Reg Gfx_functions[] = {
    {"RGBA8", lua_Gfx_RGBA8},
    {"drawRectangle", lua_Gfx_drawRectangle},
    {"draw4ColorsRectangle", lua_Gfx_draw4ColorsRectangle},
    {"drawRectangleFromCorners", lua_Gfx_drawRectangleFromCorners},
    {"drawLine", lua_Gfx_drawLine},
    {"translate", lua_Gfx_translate},
    {"rotate", lua_Gfx_rotate},
    {"pushMatrix", lua_Gfx_pushMatrix},
    {"identity", lua_Gfx_identity},
    {"popMatrix", lua_Gfx_popMatrix},
    {"pushScissorBox", lua_Gfx_pushScissorBox},
    {"pushIdentityScissorBox", lua_Gfx_pushIdentityScissorBox},
    {"popScissorBox", lua_Gfx_popScissorBox},
    {"getCurScissorBox", lua_Gfx_getCurScissorBox},
    {"getCurMatrix", lua_Gfx_getCurMatrix},
    {"loadFont", lua_Gfx_loadFont},
    {"print", lua_Gfx_print},
    {"setFontColor", lua_Gfx_setFontColor},
    {"getFontColor", lua_Gfx_getFontColor},
    {"setFontVerticalAlignment", lua_Gfx_setFontVerticalAlignment},
    {"getFontVerticalAlignment", lua_Gfx_getFontVerticalAlignment},
    {"getTextWidth", lua_Gfx_getTextWidth},
    {"drawImage", lua_Gfx_drawImage},
    {"loadImage", lua_Gfx_loadImage},
    {"resizeImage", lua_Gfx_resizeImage},
    {"getImageSize", lua_Gfx_getImageSize},
    {NULL, NULL}
};

void luaRegisterGfxLib(lua_State* L) {
    lua_newtable(L);
    luaSetTableIntField(L, "TOP_ALIGN", (u32)Font::TOP);
    luaSetTableIntField(L, "CENTER_ALIGN", (u32)Font::CENTER);
    luaSetTableIntField(L, "BOTTOM_ALIGN", (u32)Font::BOTTOM);
    luaSetTableIntField(L, "BLACK", (u32)RGBA8(0x00, 0x00, 0x00, 0xFF));
    luaSetTableIntField(L, "WHITE", (u32)RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
    luaSetTableIntField(L, "RED", (u32)RGBA8(0xFF, 0x00, 0x00, 0xFF));
    luaSetTableIntField(L, "GREEN", (u32)RGBA8(0x00, 0xFF, 0x00, 0xFF));
    luaSetTableIntField(L, "BLUE", (u32)RGBA8(0x00, 0x00, 0xFF, 0xFF));
    luaL_setfuncs(L, Gfx_functions, 0);
    lua_setglobal(L, "Gfx");
}
