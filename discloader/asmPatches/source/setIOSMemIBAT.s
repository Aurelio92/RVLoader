#include <asm.h>

# This code sets IBAT7 registers to allow access to the memory region used by IOS
# Somehow some games don't have access to that region and we need it for our patches handled by gamepatcher IOS module
setIOSMemIBAT: 
    li r0, 0x0000
    mtspr 566, r0
    lis r0, 0x1300
    ori r0, r0, 0x0002
    mtspr 567, r0
    lis r0, 0x9300
    ori r0, r0, 0x00ff
    mtspr 566, r0
    isync
    mfmsr r3
    ori r3, r3, 48
    mtsrr1 r3
    mflr r3
    mtsrr0 r3
    rfi
