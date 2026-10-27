#pragma once

#include <gccore.h>

#define WIFI_OFF  0
#define WIFI_ON   1
#define WIFI_AUTO 2

#define WIFI_MANUFACTURER_BROADCOM 0x02d0

namespace Wifi {
    u16 getManufacturer();
    bool isEnabled(int setting);
};
