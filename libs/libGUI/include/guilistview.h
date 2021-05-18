#pragma once

#include <list>
#include "libgui.h"

class GuiListView : public GuiElement {
    private:
        std::list<GuiElement*> elements;
    public:
        GuiListView() {}
        ~GuiListView();
        void draw(bool onFocus);

        void addElement(GuiElement* el);

        GuiListView& operator = (const GuiListView& l);
};
