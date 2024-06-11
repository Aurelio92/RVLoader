#pragma once

#include "libgui.h"
#include "draw.h"

class GuiImage : public GuiElement {
    private:
        Texture tex;
        int width;
        int height;
        bool tplMode;
    public:
        GuiImage();
        GuiImage(const char* filename);
        GuiImage(u8* textureBuffer, u8* paletteBuffer, u16 width, u16 height, u32 paletteNItems);
        GuiImage(u8* textureBuffer, u16 width, u16 height);
        GuiImage(TPLFile* tdf, s32 id);
        GuiImage(const GuiImage& img);
        ~GuiImage();
        //GuiImage(const char* filename, int _width, int _height);
        void loadImage(const char* filename);
        void loadImage(TPLFile* tdf, s32 id);
        void setSize(int _width, int _height);
        void draw();
        void draw(bool onFocus);
        void draw(bool onFocus, bool xMirror, bool yMirror);
        void drawAlpha(int alpha);
        void drawTextureAlphaTexCoords(int alpha, f32 textCoords[]);
        void drawFromCorners(f32* corners);
        Vector2 getDimensions() {return Vector2(width, height);};

        GuiImage& operator = (const GuiImage& img);
        void setTextureWrap(int wrap_s, int wrap_t);
};
