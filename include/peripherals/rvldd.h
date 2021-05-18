#pragma once

#include <gccore.h>

namespace RVLDD {
    bool isConnected();
    u8 getBacklight();
    u8 getStretch();
    s16 getColorBrightness();
    s16 getColorContrast();
    s16 getColorTemperature();
    void setBacklight(u8 bl);
    void setStretch(u8 stretch);
    void setColorBrightness(s16 brightness);
    void setColorContrast(s16 contrast);
    void setColorTemperature(s16 temperature);
};