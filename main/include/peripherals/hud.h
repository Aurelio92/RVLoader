#pragma once

#include <gccore.h>

#define HUD_CTRL_GCPAD      0
#define HUD_CTRL_POT        1
#define HUD_CTRL_BUTTONS    2

namespace HUD {
    void init();
    void setHeadphonesVolume(u8 vol);
    u8 getHeadphonesVolume();
    void setSpeakersVolume(u8 vol);
    u8 getSpeakersVolume();
    void setVolumeControlSystem(u8 ctrlSystem);
    u8 getVolumeControlSystem();
};