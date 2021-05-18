#pragma once

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include "rect.h"

class GuiElement {
    protected:
        int width;
        int height;
        GuiElement* parentElement;

    private:
        friend class GuiWindow;

    public:
        GuiElement() {
            width = 0;
            height = 0;
            parentElement = NULL;
        }
        virtual ~GuiElement() {}

        // Inherited classes override these
        virtual void draw(bool onFocus) {}
        virtual void handleInputs(bool onFocus) {}
        virtual Vector2 getDimensions() {return Vector2(width, height);}
        virtual void onFocusEvent() {};
        virtual void onDefocusEvent() {};
        virtual void onActiveEvent() {};
        virtual void onInactiveEvent() {};

        virtual void signalHandler(u32 signal) {};

        virtual void sendSignal(std::string receipientId, u32 signal) {
            if (parentElement != NULL)
                parentElement->sendSignal(receipientId, signal);
        }
};
