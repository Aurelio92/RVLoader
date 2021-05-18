#pragma once

#include "libgui.h"
#include "draw.h"

class GuiImage : public GuiElement {
    private:
        Texture tex;
        int width;
        int height;
    public:
        GuiImage();
        GuiImage(const char* filename);
        GuiImage(TPLFile* tdf, s32 id);
        GuiImage(const GuiImage& img);
        ~GuiImage();
        //GuiImage(const char* filename, int _width, int _height);
        void loadImage(const char* filename);
        void loadImage(TPLFile* tdf, s32 id);
        void setSize(int _width, int _height);
        void draw();
        void draw(bool onFocus);
        Vector2 getDimensions() {return Vector2(width, height);};

        GuiImage& operator = (const GuiImage& img);
};
