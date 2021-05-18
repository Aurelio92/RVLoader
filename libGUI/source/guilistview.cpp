#include "libgui.h"
#include "gfx.h"

GuiListView::~GuiListView() {
    elements.clear();
}

void GuiListView::draw(bool onFocus) {
    Gfx::pushMatrix();
    for (auto& it : elements) {
        Vector2 dim = it->getDimensions();
        Gfx::pushScissorBox(dim.x, dim.y);
        it->draw(onFocus);
        Gfx::translate(0, dim.y);
        Gfx::popScissorBox();
    }
    Gfx::popMatrix();
}

void GuiListView::addElement(GuiElement* el) {
    elements.push_back(el);
}

GuiListView& GuiListView::operator = (const GuiListView& l) {
    if (this == &l) { //Copying itself?
        return *this;
    }

    this->elements = l.elements;

    return *this;
}
