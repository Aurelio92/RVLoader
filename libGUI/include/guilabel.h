#pragma once

#include "libgui.h"
#include <string>

class GuiLabel : public GuiElement {
    private:
        std::string text;
        Font* font;
    public:
        GuiLabel();
        GuiLabel(Font* _font, std::string _text);
        void draw(bool onFocus);

        //GuiImage& operator = (const GuiImage& img);
};
