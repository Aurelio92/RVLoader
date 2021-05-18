#pragma once

#include <map>
#include <unordered_map>
#include "libgui.h"

class GuiWindow : public GuiElement {
    private:
        struct WinElement {
                GuiElement* element;
                int posX, posY;
                bool active;
                bool superFocus;
                bool switchable;

                WinElement(GuiElement* element, int posX, int posY, bool active) : element(element), posX(posX), posY(posY), active(active), superFocus(false), switchable(false) {}
                WinElement() {}
        };

        std::map<GuiElement*, WinElement> elements;
        std::map<u32, WinElement*> elementsOrder;
        std::unordered_map<std::string, WinElement*> elementsTable;
        WinElement* elementOnFocus;
        int width;
        int height;
        u32 color;

        Vector2 scroll;

    public:
        GuiWindow() {
            width = 0;
            height = 0;
            color = 0;
            scroll = Vector2(0, 0);
            elementOnFocus = NULL;
        }

        GuiWindow(int _width, int _height) {
            width = _width;
            height = _height;
            color = 0;
            scroll = Vector2(0, 0);
            elementOnFocus = NULL;
        }

        ~GuiWindow();
        void draw(bool onFocus);
        void handleInputs(bool onFocus);
        Vector2 getDimensions() {return Vector2(width, height);};
        void loseFocus(Vector2 dir);

        void setSize(int _width, int _height) {
            width = _width;
            height = _height;
        }

        void setColor(u32 _color) {
            color = _color;
        }

        void setScroll(Vector2 _scroll) {
            scroll = _scroll;
        }

        void setScroll(int sx, int sy) {
            scroll.x = sx;
            scroll.y = sy;
        }

        GuiElement* getElementOnFocus() {
            return elementOnFocus->element;
        }

        void addElement(GuiElement* el, int posX, int posY);
        void addElement(GuiElement* el, int posX, int posY, bool active);
        void addElement(GuiElement* el, int posX, int posY, bool active, std::string id);
        void setElementPosition(GuiElement* el, int posX, int posY);
        void setElementActive(GuiElement* el, bool active);
        void setElementSuperFocus(GuiElement* el, bool superFocus);
        void setElementSwitchable(GuiElement* el, bool switchable);
        bool switchToElement(GuiElement* el);
        bool switchToElement(std::string id);
        void focusOnElement(GuiElement* el);
        void bringElementForward(GuiElement* el);
        void bringElementToFront(GuiElement* el);
        void pushElementBackward(GuiElement* el);
        void pushElementToBack(GuiElement* el);

        GuiWindow& operator = (const GuiWindow& l);

        void onFocusEvent();
        void onDefocusEvent();
        void onActiveEvent();
        void onInactiveEvent();

        void sendSignal(std::string receipientId, u32 signal);
};
