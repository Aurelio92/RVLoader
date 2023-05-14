#include "libgui.h"

GuiLabel::GuiLabel() {
    font = NULL;
}

GuiLabel::GuiLabel(Font* _font, std::string _text) {
    font = _font;
    text = _text;
    width = font->getTextWidth(text.c_str());
    height = font->getTextHeight(text.c_str());
}

void GuiLabel::draw(bool onFocus) {
    font->printf(0, 0, text.c_str());
}
