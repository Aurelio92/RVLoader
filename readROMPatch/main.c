
typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;
typedef volatile signed long long vs64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef float f32;

#define NULL ((void*)0)

#include "font_western_bin.h"

void _main(void *buf, u32 len, u32 offset) {
    u8* bufU8 = (u8*)buf;
    if (len != 12288 || offset != 2084608) {
        return;
    }

    for (u32 i = 0; i < font_western_bin_size; i++) {
        bufU8[i] = font_western_bin[i];
    }
}
