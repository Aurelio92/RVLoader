#pragma once

#include <gccore.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "draw.h"
#include "sha1.h"

class Font {
    public:
        struct My_GlyphSlot {
                Texture tex;
                u16 width, height;
                FT_Glyph_Metrics metrics;
                FT_Vector advance;
                int bitmap_top, bitmap_left;
        };

        enum VerticalAlignment {
            TOP,
            CENTER,
            BOTTOM
        };

        struct My_Face {
            u8* memory;
            u32 size;
        };

    private:
        FT_Face face;
        u16 size;
        std::map<FT_ULong, My_GlyphSlot*> loadedGlyphs;
        u32 color;
        VerticalAlignment verticalAlignment;

        My_GlyphSlot* loadChar(FT_ULong charCode);

        char* charBuffer;

    public:
        static FT_Library library;
        static std::map<std::string, My_Face> faceCache;

        Font();
        Font(const char* filename, u16 size);
        Font(const u8* fontData, const u32 fontSize, u16 size);
        Font(const Font& font);
        ~Font();

        Font& operator = (const Font& font);

        u16 getSize() {
            return size;
        }

        void setColor(u32 _color) {
            color = _color;
        }

        u32 getColor() {
            return color;
        }

        void setVerticalAlignment(VerticalAlignment _verticalAlignment) {
            verticalAlignment = _verticalAlignment;
        }

        VerticalAlignment getVerticalAlignment() {
            return verticalAlignment;
        }

        static FT_ULong getCharUTF8(const char* buffer, u32* charLen);
        int drawChar(int x, int y, FT_ULong charCode);
        int getCharWidth(FT_ULong charCode);
        int getCharHeight(FT_ULong charCode);
        int getCharBearingX(FT_ULong charCode);
        int getCharBearingY(FT_ULong charCode);
        int printf(int x, int y, const char* format, ...);
        int print(int x, int y, const std::string str);
        int getTextWidth(const char* format, ...);
        int getTextHeight(const char* format, ...);

        /*My_GlyphSlot* loadChar(FT_ULong charCode);
        u32 getString16Width(const std::u16string& str);
        int drawChar(s16 x0, s16 y0, FT_ULong charCode);
        void drawCharBounded(s16* x0, s16* y0, Rect& rect, FT_ULong charCode);
        void drawTexture(s16 x0, s16 y0);
        int printf16(Rect rect, const std::u16string& format, ...);*/
        //int printf16(s16 x0, s16 y0, std::u16string s);
};
