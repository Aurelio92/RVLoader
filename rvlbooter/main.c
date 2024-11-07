#include <string.h>
#include "global.h"
#include "elf.h"

#include "main_dol.h"

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

#define MAX_ADDRESS 0x817FFFFF

void DCFlushRange(void* startaddress,u32 len);
void ICInvalidateRange(void* startaddress,u32 len);

void _memcpy(void* dst, void* src, u32 len) {
    u8 *d = dst;
    const u8 *s = src;
    while (len--)
        *d++ = *s++;
}

void _memset(void* dst, u32 data, u32 len) {
    u8 *ptr = dst;
    while (len-- > 0)
        *ptr++ = data;
}

u32 relocateDol(const void* buffer) {
    int i;
    dolhdr* hdr = (dolhdr* )buffer;

    u8 set_bss = (hdr->addressBSS > 0x80003400 && hdr->addressBSS + hdr->sizeBSS < MAX_ADDRESS);

    //Load text sections
    for (i = 0; i < 7; i++) {
        if (!hdr->sizeText[i] || hdr->offsetText[i] < 0x100)
            continue;
        _memcpy((u32*)hdr->addressText[i], (u32*)(buffer + hdr->offsetText[i]), hdr->sizeText[i]);
        DCFlushRange((void*)hdr->addressText[i], hdr->sizeText[i]);
        ICInvalidateRange((void*)hdr->addressText[i], hdr->sizeText[i]);
    }

    //Load data sections
    for (i = 0; i < 11; i++) {
        if (!hdr->sizeData[i] || hdr->offsetData[i] < 0x100)
            continue;
        set_bss = set_bss &&
            (hdr->addressData[i] + hdr->sizeData[i] <= hdr->addressBSS ||
            hdr->addressData[i] >= hdr->addressBSS + hdr->sizeBSS);
        _memcpy((u32*)hdr->addressData[i], (u32*)(buffer + hdr->offsetData[i]), hdr->sizeData[i]);
        DCFlushRange((void*)hdr->addressData[i], hdr->sizeData[i]);
    }

    //Clear BSS
    if (set_bss) {
        _memset((void*)hdr->addressBSS, 0, hdr->sizeBSS);
        DCFlushRange((void*)hdr->addressBSS, hdr->sizeBSS);
    }

    return hdr->entrypoint | 0x80000000;
}

typedef void (*entry_t)(void);

void _main() {
    asm("isync");
    entry_t entry = (entry_t)relocateDol(main_dol);
    asm("isync");
    entry();
}
