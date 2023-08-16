#pragma once

#define PAD_BUTTON_DPAD (PAD_BUTTON_UP | PAD_BUTTON_DOWN | PAD_BUTTON_RIGHT | PAD_BUTTON_LEFT)

namespace GenPad {
    void update();
    int down(u32 channel);
    int held(u32 channel);
    int up(u32 channel);
    int stickX(u32 channel);
    int stickY(u32 channel);
    int subStickX(u32 channel);
    int subStickY(u32 channel);
};
