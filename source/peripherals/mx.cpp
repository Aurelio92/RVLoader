#include <gccore.h>

namespace MX {
    bool isConnected() {
        u32 retId = 0;
        EXI_GetID(EXI_CHANNEL_0,EXI_DEVICE_1, &retId);
        if (retId == 0xFFFFF308)
            return true;
        return false;
    }
}