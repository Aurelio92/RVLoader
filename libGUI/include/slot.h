#pragma once

#include "libgui.h"

typedef void (*slotcb)(void* arg, u32 signal);

class Slot : public GuiElement {
    void* arg;
    slotcb cb;

    public:
        Slot() {
            arg = NULL;
            cb = NULL;
        }

        Slot(void* _arg, slotcb _cb) {
            arg = _arg;
            cb = _cb;
        }

        void signalHandler(u32 signal) {
            if (cb)
                cb(arg, signal);
        }
};
