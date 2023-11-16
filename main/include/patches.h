#pragma once

#include "DOL.h"
#include <vector>

typedef struct {
    u32 address;
    u32 offset;
    u32 checksum;
    u32 len;
} DOLFunction;

u32 computeCodeChecksum(u8* data, u32 len);
void listFunctionsInDOL(dolhdr* dol, std::vector<DOLFunction>& functions);
void injectMXPatch(u8* address);