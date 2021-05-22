#include <gccore.h>
#include <ogc/machine/processor.h>
#include "i2c.h"

#define REG_BACKLIGHT       0x00
#define REG_STRETCH         0x01
#define REG_COLBRIGHTNESS   0x02
#define REG_COLCONTRAST     0x04
#define REG_COLTEMP         0x06

#define RVLDD_ADDR (0x52 << 1)

namespace RVLDD {
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
            i2cRet = i2c_sendByte(RVLDD_ADDR);
            if (!i2cRet) {
                ret = false;
            }
            i2c_stop();
            _CPU_ISR_Restore(level);
        }
        return ret;
    }

    u8 getBacklight() {
        u8 error;
        u8 bl;
        bl = i2c_read8(RVLDD_ADDR, REG_BACKLIGHT, &error);
        return bl;
    }

    u8 getStretch() {
        u8 error;
        u8 stretch;
        stretch = i2c_read8(RVLDD_ADDR, REG_STRETCH, &error);
        return stretch;
    }

    s16 getColorBrightness() {
        u8 error;
        s16 brightness;
        brightness = i2c_read16(RVLDD_ADDR, REG_COLBRIGHTNESS, &error);
        return brightness;
    }

    s16 getColorContrast() {
        u8 error;
        s16 contrast;
        contrast = i2c_read16(RVLDD_ADDR, REG_COLCONTRAST, &error);
        return contrast;
    }

    s16 getColorTemperature() {
        u8 error;
        s16 temperature;
        temperature = i2c_read16(RVLDD_ADDR, REG_COLTEMP, &error);
        return temperature;
    }

    void setBacklight(u8 bl) {
        u8 error;
        i2c_write8(RVLDD_ADDR, REG_BACKLIGHT, bl, &error);
    }

    void setStretch(u8 stretch) {
        u8 error;
        i2c_write8(RVLDD_ADDR, REG_STRETCH, stretch, &error);
    }

    void setColorBrightness(s16 brightness) {
        u8 error;
        i2c_write16(RVLDD_ADDR, REG_COLBRIGHTNESS, brightness, &error);
    }

    void setColorContrast(s16 contrast) {
        u8 error;
        i2c_write16(RVLDD_ADDR, REG_COLCONTRAST, contrast, &error);
    }

    void setColorTemperature(s16 temperature) {
        u8 error;
        i2c_write16(RVLDD_ADDR, REG_COLTEMP, temperature, &error);
    }
};