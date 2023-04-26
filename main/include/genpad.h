#pragma once

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
