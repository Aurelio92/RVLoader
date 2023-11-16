#pragma once

#include <gccore.h>

typedef struct {
    u32 offsetText[7];
    u32 offsetData[11];
    u32 addressText[7];
    u32 addressData[11];
    u32 sizeText[7];
    u32 sizeData[11];
    u32 addressBSS;
    u32 sizeBSS;
    u32 entrypoint;
} dolhdr;