#pragma once

namespace SYSCONF {
    void setConfBuffer(u8* buffer);
    void setConfTxtBuffer(u8* buffer);
    void decencTextBuffer();
    s32 injectGC2Wiimote();
    void setRegion(u32 gameID);
    void setRegionSetting(u32 gameID);
};
