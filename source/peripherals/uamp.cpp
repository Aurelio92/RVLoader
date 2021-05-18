#include <gccore.h>
#include <ogc/machine/processor.h>
#include "i2c.h"
#include "uamp.h"

namespace UAMP {
    bool isConnected() {
        u32 level;
        u8 i2cRet;
        static bool firstTime = true;
        static bool ret = false;

        if (firstTime) {
            firstTime = false;
            _CPU_ISR_Disable(level);
            ret = true;
            i2c_start();
            i2cRet = i2c_sendByte(LM49450_ADDR);
            if (!i2cRet) {
                ret = false;
            }
            i2c_stop();
            _CPU_ISR_Restore(level);
        }
        return ret;
    }

    void init() {
        u8 error;
        if (isConnected()) {
            i2c_write8(LM49450_ADDR, 0x00, 0x29, &error);
            i2c_write8(LM49450_ADDR, 0x01, 0x00, &error);
            i2c_write8(LM49450_ADDR, 0x02, 0x4B, &error);
            i2c_write8(LM49450_ADDR, 0x03, 0x01, &error);
            i2c_write8(LM49450_ADDR, 0x04, 0x00, &error);
            i2c_write8(LM49450_ADDR, 0x05, 0x00, &error);
            i2c_write8(LM49450_ADDR, 0x06, 0x00, &error);
        }
    }

    void setMute(bool mute) {
        u8 error;
        if (mute)
            i2c_write8(LM49450_ADDR, 0x00, 0x2D, &error);
        else
            i2c_write8(LM49450_ADDR, 0x00, 0x29, &error);
    }

    void setHeadphonesVolume(u8 vol) {
        u8 error;
        i2c_write8(LM49450_ADDR, 0x07, vol, &error);
    }

    void setSpeakersVolume(u8 vol) {
        u8 error;
        i2c_write8(LM49450_ADDR, 0x08, vol, &error);
    }
};
