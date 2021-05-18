#include <string.h>
#include "global.h"
#include "elf.h"

#define EXECUTE_ADDR   ((u8*)0x92000000)
#define BOOTER_ADDR    ((u8*)0x93000000)
#define ARGS_ADDR      ((u8*)0x93200000)
#define CMDL_ADDR      ((u8*)0x93200000+sizeof(struct __argv))
#define LOADER_SIZE    0x400000
#define LOADER_ADDR    ((u8*)ARGS_ADDR - LOADER_SIZE)

struct __argv {
    int argvMagic;      //!< argv magic number, set to 0x5f617267 ('_arg') if valid
    char *commandLine;  //!< base address of command line, set of null terminated strings
    int length;//!< total length of command line
    int argc;
    char **argv;
    char **endARGV;
};

#define ARGV_MAGIC 0x5f617267

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
    return;
}

void _memset(void* dst, u32 data, u32 len) {
    u8 *ptr = dst;
    while (len-- > 0)
        *ptr++ = data;
    return;
}

asm(R"(.globl CacheSyncRange
CacheSyncRange:
    addi       4,4,0x1f
    add        4,4,3
    rlwinm     3,3,0x0,0x0,0x1a
    rlwinm     4,4,0x0,0x0,0x1a
    b          _checkLoop
_sync:
    dcbst      0,3
    sync       0x0
    icbi       0,3
    addi       3,3,0x20
_checkLoop:
    cmplw      cr7,3,4
    blt        cr7,_sync
    sync       0x0
    isync
    blr)");

void CacheSyncRange(void* startaddress,u32 len);

u32 relocateDol(u8* buffer, struct __argv* argv) {
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
        //CacheSyncRange((void*)hdr->addressText[i], hdr->sizeText[i]);
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
        //CacheSyncRange((void*)hdr->addressData[i], hdr->sizeData[i]);
    }

    //Clear BSS
    if (set_bss) {
        _memset((void*)hdr->addressBSS, 0, hdr->sizeBSS);
        DCFlushRange((void*)hdr->addressBSS, hdr->sizeBSS);
        //CacheSyncRange((void*)hdr->addressBSS, hdr->sizeBSS);
    }

    if (argv && argv->argvMagic == ARGV_MAGIC) {
        _memcpy((void*)(hdr->entrypoint + 8), (void*)argv, sizeof(*argv));
        DCFlushRange((void*)(hdr->entrypoint + 8), sizeof(*argv));
        //CacheSyncRange((void*)(hdr->entrypoint + 8), sizeof(*argv));
    }
    return hdr->entrypoint | 0x80000000;
}

u32 relocateExecutable(u8* buffer, struct __argv* argv) {
    int i;

    //Check if ELF
    Elf32_Ehdr *ElfHdr = (Elf32_Ehdr *)buffer;

    if (ElfHdr->e_ident[EI_MAG0] == 0x7F &&
        ElfHdr->e_ident[EI_MAG1] == 'E' &&
        ElfHdr->e_ident[EI_MAG2] == 'L' &&
        ElfHdr->e_ident[EI_MAG3] == 'F') {

        if ((ElfHdr->e_entry | 0x80000000) < 0x80003400 && (ElfHdr->e_entry | 0x80000000) >= MAX_ADDRESS) {
            return 0;
        }

        //Load sections
        for (i = 0; i < ElfHdr->e_phnum; ++i) {
            Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + (ElfHdr->e_phoff + sizeof(Elf32_Phdr) * i));
            DCFlushRange((void*)(phdr->p_vaddr | 0x80000000),phdr->p_filesz);
            ICInvalidateRange((void*)(phdr->p_vaddr | 0x80000000),phdr->p_filesz);
            if (phdr->p_type != PT_LOAD)
                continue;

            //PT_LOAD Segment, aka static program data
            _memcpy((void*)(phdr->p_vaddr | 0x80000000), buffer + phdr->p_offset , phdr->p_filesz);
        }

        for (i = 0; i < ElfHdr->e_shnum; ++i) {
            Elf32_Shdr *shdr = (Elf32_Shdr*)(buffer + (ElfHdr->e_shoff + sizeof(Elf32_Shdr) * i));

            if (shdr->sh_type != SHT_NOBITS)
                continue;

            _memset((void*)(shdr->sh_addr | 0x80000000), 0, shdr->sh_size);
            DCFlushRange((void*)(shdr->sh_addr | 0x80000000),shdr->sh_size);
        }

        if (argv && argv->argvMagic == ARGV_MAGIC) {
            _memcpy((void*)(ElfHdr->e_entry + 8), (void*)argv, sizeof(*argv));
            DCFlushRange((void*)(ElfHdr->e_entry + 8), sizeof(*argv));
            //CacheSyncRange((void*)(ElfHdr->e_entry + 8), sizeof(*argv));
        }

        return (ElfHdr->e_entry | 0x80000000);
    }

    return relocateDol(buffer, argv);
}

typedef void (*entry_t)(void);

void _main() {
    asm("isync");
    entry_t entry = (entry_t)relocateExecutable(EXECUTE_ADDR, (struct __argv*)ARGS_ADDR);
    asm("isync");
    entry();
}
