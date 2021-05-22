#include <gccore.h>
#include "i2c.h"

#define AVE_ADDR (0x70 << 1)

bool aveIsVGAModeEnabled() {
    u8 i2c_error;
    if ((i2c_read8(AVE_ADDR, 0x01, &i2c_error) == 0x23) && !i2c_error) {
        return true;
    }

    return false;
}

bool aveEnableVGA() {
    u8 i2c_error;
    i2c_write8(AVE_ADDR, 0x01, 0x23, &i2c_error);
    if (i2c_error)
        return false;

    i2c_write8(AVE_ADDR, 0x0A, 0x00, &i2c_error);
    if (i2c_error)
        return false;

    i2c_write8(AVE_ADDR, 0x62, 0x01, &i2c_error);
    if (i2c_error)
        return false;

    i2c_write8(AVE_ADDR, 0x6E, 0x01, &i2c_error);
    if (i2c_error)
        return false;

    i2c_write8(AVE_ADDR, 0x65, 0x03, &i2c_error);
    if (i2c_error)
        return false;

    return true;
}
