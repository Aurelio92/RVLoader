#pragma once

#include <gccore.h>

#define __VIRetraceHandler0_checksum    0x3abd8500
#define __VIRetraceHandler0_len         0x00000274
#define __VIRetraceHandler1_checksum    0xae3b3ce7
#define __VIRetraceHandler1_len         0x0000080c
#define __VIRetraceHandler2_checksum    0x4d1bad5c
#define __VIRetraceHandler2_len         0x0000074c
#define __VIRetraceHandler3_checksum    0xda9ea4ed
#define __VIRetraceHandler3_len         0x00000810
#define SISetXY0_checksum               0x046c4697
#define SISetXY0_len                    0x0000005c
#define SISetXY1_checksum               0x6fea1b4d
#define SISetXY1_len                    0x0000006c
#define ReadFont0_checksum              0xc077f38d
#define ReadFont0_len                   0x00000300
#define ReadFont1_checksum              0xf1d1a20e
#define ReadFont1_len                   0x00000310
#define PADRead0_checksum               0xffc5f9b2
#define PADRead0_len                    0x000004c0
#define PADRead1_checksum               0x9a51849a
#define PADRead1_len                    0x000004bc
#define VISetNextFrameBuffer_checksum   0xd273a2b5
#define VISetNextFrameBuffer_len        0x0000006c

typedef struct {
    u32 address;
    u32 checksum;
    u32 len;
} GamePatch;

void findPatches(void* dst, u32 len, std::vector<GamePatch>& patches);
