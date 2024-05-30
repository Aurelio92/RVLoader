#include <stdio.h>
#include <ogc/machine/processor.h>
#include <ogc/cache.h>
#include <ogc/video.h>
#include <ogc/video_types.h>
#include <ogc/es.h>
#include <ogc/ipc.h>
#include <ogcsys.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <ogc/usbgecko.h>
#include <ogc/exi.h>
#include <lua.hpp>

#include "main.h"
#include "utils.h"
#include "systitles.h"
#include "system.h"
#include "hiidra.h"
#include "ELF.h"
#include "luasupport.h"
#include "ash.h"
#include "discAnim.h"

#define COPY_LIMIT (128*1024)

#define HIIDRA_LOG_LINES 32
#define HIIDRA_LOG_LINE_LENGTH 256

#define STACKSIZE (128*1024)

#define MEM_PROT       0x0D8B420A

#define ES_IOCTL_ENABLEUSBSAVES 0x61
#define ES_IOCTL_LAUNCHTITLEID 0x62
#define ES_IOCTL_ENABLEVGA 0x63
#define ES_IOCTL_ENABLEGC2WIIMOTE 0x64

#define DIP_IOCTL_DVDLoadGame 0xF0

static char moduleSHA[0x1C] ALIGNED(32);
static char modulePath[1024] ALIGNED(32);
#define MODULE_BUFFER_SIZE 0x1000
static char moduleBuffer[MODULE_BUFFER_SIZE] ALIGNED(32);

// Title ID for IOS58.
static const u64 TitleID_IOS58 = 0x000000010000003AULL;

static ioctlv IOCTL_Buf ALIGNED(32);

//Logger stuff
static mutex_t logMutex = 0;
static char logBuffer[HIIDRA_LOG_LINES][HIIDRA_LOG_LINE_LENGTH];
static u32 logLineIndex;
static u32 logNLines;
static lwpq_t logQueue = 0;

//Flag to manage HiidraThread log lines (needed for verbose hiidra loading)
volatile bool hideLogLines = false;

extern "C" {
    extern void* SYS_AllocArena2MemLo(u32 size,u32 align);
    extern void udelay(u32 us);
}

/* Full HW 0x0D8000000 Access */
static const unsigned char HWAccess_ES[] = {
    0x0D, 0x80, 0x00, 0x00, //virt address (0x0D800000)
    0x0D, 0x80, 0x00, 0x00, //phys address (0x0D800000)
    0x00, 0x0D, 0x00, 0x00, //length (up to 0x0D8D0000)
    0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x02, //permission (2=ro, patchme)
    0x00, 0x00, 0x00, 0x00,
};
static const unsigned char HWAccess_ESPatch[] = {
    0x0D, 0x80, 0x00, 0x00, //virt address (0x0D800000)
    0x0D, 0x80, 0x00, 0x00, //phys address (0x0D800000)
    0x00, 0x0D, 0x00, 0x00, //length (up to 0x0D8D0000)
    0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x03, //permission (3=rw)
    0x00, 0x00, 0x00, 0x00,
};

static const unsigned char KernelAccess[] = {
    0xFF, 0xF0, 0x00, 0x00,
    0xFF, 0xF0, 0x00, 0x00,
    0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01,
};

static const unsigned char KernelAccessPatch[] = {
    0xFF, 0xF0, 0x00, 0x00,
    0xFF, 0xF0, 0x00, 0x00,
    0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x01,
};

raw_irq_handler_t BeforeIOSReload() {
    __STM_Close();

    write32(0x80003140, 0);
    __MaskIrq(IRQ_PI_ACR);
    return IRQ_Free(IRQ_PI_ACR);
}

void AfterIOSReload(raw_irq_handler_t handle, u32 rev) {
    while (read32(0x80003140) != rev) {
        DCInvalidateRange((void*)0x80003140, 0x20);
        udelay(1000);
    }

    u32 counter;
    for (counter = 0; !(read32(0x8d000004) & 2); counter++)
    {
        udelay(1000);
        if (counter >= 40000)
            break;
    }
    IRQ_Request(IRQ_PI_ACR, handle, NULL);
    __UnmaskIrq(IRQ_PI_ACR);
    __IPC_Reinitialize();
    __STM_Init();
}

void forgeKernel(char* kernel, u32 kernelSize, const uint8_t** extraModules, u32 nExtraModules, u32 keepES, u32 keepFS) {
    u32 i, j;
    u32 noteSize;
    unsigned int loadersize = *(vu32*)(kernel) + *(vu32*)(kernel+4);
    //Copy old kernel in a buffer so we can overwrite the kernel memory region
    char* oldKernel = (char*)SYS_AllocArena2MemLo(kernelSize, 0x20);
    memcpy(oldKernel, kernel, kernelSize);
    for (i = loadersize; i < kernelSize; i+=4) {
        if (memcmp(oldKernel+i, HWAccess_ES, sizeof(HWAccess_ES)) == 0) {
            printf("Found HWAccess_ES at %08X\r\n", i);
            hiidraAddLogLine("Found HWAccess_ES at %08X", i);
            memcpy(oldKernel+i, HWAccess_ESPatch, sizeof(HWAccess_ESPatch));
        }

        if (memcmp(oldKernel+i, KernelAccess, sizeof(KernelAccess)) == 0) {
            printf("Found KernelAccess at %08X\r\n", i);
            hiidraAddLogLine("Found KernelAccess at %08X", i);
            memcpy(oldKernel+i, KernelAccessPatch, sizeof(KernelAccessPatch));
        }
    }
    char* oldElf = oldKernel + loadersize;

    loadersize = *(vu32*)(kernel) + *(vu32*)(kernel+4);
    char* outElf = kernel + loadersize;

    Elf32_Ehdr* old_hdr = (Elf32_Ehdr*)oldElf;
    Elf32_Ehdr* out_hdr = (Elf32_Ehdr*)outElf;
    Elf32_Phdr* old_Phdr = (Elf32_Phdr*)(oldElf + old_hdr->e_phoff);
    char* old_note = (char*)(oldElf + old_Phdr[1].p_offset);
    memcpy(out_hdr, old_hdr, sizeof(Elf32_Ehdr));
    Elf32_Phdr* out_Phdr = (Elf32_Phdr*)(outElf + out_hdr->e_phoff);

    char* outputKernel = (char*)out_Phdr;
    u32 elfSize = sizeof(Elf32_Ehdr);
    u32 offset = out_hdr->e_phoff;

    //Compute total number of PHDR entries
    out_hdr->e_phnum = 0;
    for (i = 0; i < nExtraModules; i++) { //New modules
        Elf32_Ehdr* mod_hdr = (Elf32_Ehdr*)extraModules[i];
        out_hdr->e_phnum += mod_hdr->e_phnum - 3; //Skipping first 3 each
    }
    for (i = 0; i < old_hdr->e_phnum; i++) { //Old entries
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20100000) && !keepES)
            continue;
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20000000) && !keepFS)
            continue;

        out_hdr->e_phnum++;
    }

    //Build PHDR header
    out_Phdr = (Elf32_Phdr*)outputKernel;
    out_Phdr->p_type = PT_PHDR;
    out_Phdr->p_offset = offset;
    out_Phdr->p_vaddr = old_Phdr[0].p_vaddr;
    out_Phdr->p_paddr = old_Phdr[0].p_paddr;
    out_Phdr->p_filesz = out_hdr->e_phnum * sizeof(Elf32_Phdr);
    out_Phdr->p_memsz = out_hdr->e_phnum * sizeof(Elf32_Phdr);
    out_Phdr->p_flags = old_Phdr[0].p_flags;
    out_Phdr->p_align = 0x4;
    offset += out_hdr->e_phnum * sizeof(Elf32_Phdr);
    elfSize += sizeof(Elf32_Phdr);
    outputKernel += sizeof(Elf32_Phdr);

    //Build NOTE header
    out_Phdr = (Elf32_Phdr*)outputKernel;
    out_Phdr->p_type = PT_NOTE;
    out_Phdr->p_offset = offset;
    out_Phdr->p_vaddr = old_Phdr[0].p_vaddr + out_hdr->e_phnum * sizeof(Elf32_Phdr);
    out_Phdr->p_paddr = old_Phdr[0].p_paddr + out_hdr->e_phnum * sizeof(Elf32_Phdr);
    out_Phdr->p_filesz = 12;
    for (i = 0; i < nExtraModules; i++) { //New modules
        Elf32_Ehdr* mod_hdr = (Elf32_Ehdr*)extraModules[i];
        Elf32_Phdr* mod_Phdr = (Elf32_Phdr*)(extraModules[i] + mod_hdr->e_phoff);
        char* mod_note = (char*)(extraModules[i] + mod_Phdr[1].p_offset);
        out_Phdr->p_filesz += *((u32*)(mod_note + 4));
    }
    //Add old modules notes size
    if (keepFS)
        out_Phdr->p_filesz += 0x28;
    if (keepES)
        out_Phdr->p_filesz += 0x28;
    noteSize = out_Phdr->p_filesz;
    out_Phdr->p_memsz = out_Phdr->p_filesz;
    out_Phdr->p_flags = old_Phdr[1].p_flags;
    out_Phdr->p_align = 0x4;
    offset += out_Phdr->p_filesz;
    elfSize += sizeof(Elf32_Phdr);
    outputKernel += sizeof(Elf32_Phdr);

    //PHDR+NOTE LOAD header
    out_Phdr = (Elf32_Phdr*)outputKernel;
    out_Phdr->p_type = PT_LOAD;
    out_Phdr->p_offset = sizeof(Elf32_Ehdr);
    out_Phdr->p_vaddr = old_Phdr[2].p_vaddr;
    out_Phdr->p_paddr = old_Phdr[2].p_paddr;
    out_Phdr->p_filesz = offset - sizeof(Elf32_Phdr);
    out_Phdr->p_memsz = old_Phdr[2].p_memsz;
    out_Phdr->p_flags = old_Phdr[2].p_flags;
    out_Phdr->p_align = 0x4000;
    elfSize += sizeof(Elf32_Phdr);
    outputKernel += sizeof(Elf32_Phdr);

    //Copy new modules PHDRs
    for (i = 0; i < nExtraModules; i++) {
        Elf32_Ehdr* mod_hdr = (Elf32_Ehdr*)extraModules[i];
        Elf32_Phdr* mod_Phdr = (Elf32_Phdr*)(extraModules[i] + mod_hdr->e_phoff);
        for (j = 3; j < mod_hdr->e_phnum; j++) {
            out_Phdr = (Elf32_Phdr*)outputKernel;
            out_Phdr->p_type = PT_LOAD;
            out_Phdr->p_offset = offset;
            out_Phdr->p_vaddr = mod_Phdr[j].p_vaddr;
            out_Phdr->p_paddr = mod_Phdr[j].p_paddr;
            out_Phdr->p_filesz = mod_Phdr[j].p_filesz;
            out_Phdr->p_memsz = mod_Phdr[j].p_memsz;
            out_Phdr->p_flags = mod_Phdr[j].p_flags;
            out_Phdr->p_align = mod_Phdr[j].p_align;
            offset += mod_Phdr[j].p_filesz;
            if (out_Phdr->p_filesz & 3) {
                offset += 4 - (out_Phdr->p_filesz & 3);
            }
            elfSize += sizeof(Elf32_Phdr);
            outputKernel += sizeof(Elf32_Phdr);
        }
    }

    //Copy old kernel PHDRs
    for (i = 3; i < old_hdr->e_phnum; i++) {
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20100000) && !keepES)
            continue;
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20000000) && !keepFS)
            continue;
        out_Phdr = (Elf32_Phdr*)outputKernel;
        out_Phdr->p_type = PT_LOAD;
        out_Phdr->p_offset = offset;
        out_Phdr->p_flags = old_Phdr[i].p_flags;
        out_Phdr->p_align = old_Phdr[i].p_align;
        out_Phdr->p_vaddr = old_Phdr[i].p_vaddr;
        out_Phdr->p_paddr = old_Phdr[i].p_paddr;
        out_Phdr->p_filesz = old_Phdr[i].p_filesz;
        out_Phdr->p_memsz = old_Phdr[i].p_memsz;
        offset += out_Phdr->p_filesz;
        if (out_Phdr->p_filesz & 3) {
            offset += 4 - (out_Phdr->p_filesz & 3);
        }
        elfSize += sizeof(Elf32_Phdr);
        outputKernel += sizeof(Elf32_Phdr);
    }

    //Build NOTE
    *(volatile unsigned int*)(outputKernel) = 0x0;
    *(volatile unsigned int*)(outputKernel + 4) = noteSize - 0xC;
    *(volatile unsigned int*)(outputKernel + 8) = 0x6;
    outputKernel += 12;
    elfSize += 12;

    for (i = 0; i < nExtraModules; i++) { //New modules
        Elf32_Ehdr* mod_hdr = (Elf32_Ehdr*)extraModules[i];
        Elf32_Phdr* mod_Phdr = (Elf32_Phdr*)(extraModules[i] + mod_hdr->e_phoff);
        char* mod_note = (char*)(extraModules[i] + mod_Phdr[1].p_offset);
        memcpy(outputKernel, mod_note + 12, *((u32*)(mod_note + 4)));
        outputKernel += *((u32*)(mod_note + 4));
        elfSize += *((u32*)(mod_note + 4));
    }

    for (i = 0; i < *((u32*)(old_note + 4)) / 0x28; i++) { //Old modules
        char* mod_note = ((char*)(old_note + 0xC + i * 0x28));
        if ((*((u32*)(mod_note + 0xC)) == 0x20000000) && !keepFS)
            continue;
        if ((*((u32*)(mod_note + 0xC)) == 0x20100000) && !keepES)
            continue;
        memcpy(outputKernel, mod_note, 0x28);
        outputKernel += 0x28;
        elfSize += 0x28;
    }

    //Build LOAD entries
    for (i = 0; i < nExtraModules; i++) { //New modules
        Elf32_Ehdr* mod_hdr = (Elf32_Ehdr*)extraModules[i];
        Elf32_Phdr* mod_Phdr = (Elf32_Phdr*)(extraModules[i] + mod_hdr->e_phoff);
        for (j = 3; j < mod_hdr->e_phnum; j++) {
            if (mod_Phdr[j].p_filesz > 0) {
                u32 alignment = 4 - (mod_Phdr[j].p_filesz & 3);
                if (alignment == 4) {
                    alignment = 0;
                }
                memcpy(outputKernel, (void*)(extraModules[i] + mod_Phdr[j].p_offset), mod_Phdr[j].p_filesz + alignment);
                elfSize += mod_Phdr[j].p_filesz + alignment;
                outputKernel += mod_Phdr[j].p_filesz + alignment;
            }
        }
    }

    for (i = 3; i < old_hdr->e_phnum; i++) {
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20100000) && !keepES)
            continue;
        if (((old_Phdr[i].p_vaddr & 0xFFFF0000) == 0x20000000) && !keepFS)
            continue;

        if (old_Phdr[i].p_filesz > 0) {
            u32 alignment = 4 - (old_Phdr[i].p_filesz & 3);
            if (alignment == 4) {
                alignment = 0;
            }
            memcpy(outputKernel, (void*)(oldElf + old_Phdr[i].p_offset), old_Phdr[i].p_filesz + alignment);
            elfSize += old_Phdr[i].p_filesz + alignment;
            outputKernel += old_Phdr[i].p_filesz + alignment;
        }
    }

    *(volatile unsigned int*)(kernel+8) = elfSize;

    DCFlushRange(kernel, loadersize + elfSize);

    printf("Saving hiidra.bin\n");
    hiidraAddLogLine("Saving hiidra.bin");
    FILE* fp = fopen("/hiidra.bin", "wb");
    if (fp == NULL) {
        printf("Couldn't\n");
    } else {
        u8* bufferToSave = (u8*)kernel;
        u32 toSave = loadersize + elfSize;
        printf("Saving %u\n", toSave);
        while (toSave > 0) {
            u32 chunkSize = (toSave > COPY_LIMIT) ? COPY_LIMIT : toSave;
            fwrite(bufferToSave, chunkSize, 1, fp);
            bufferToSave += chunkSize;
            toSave -= chunkSize;
        }
        fclose(fp);
        printf("Success\n");
        hiidraAddLogLine("Success");
    } 
}

int getKernelSize(u32* kernelSize) {
    unsigned int TMDSize;
    int i, u;

    if (kernelSize == NULL) {
        return -1;
    }

    int r = ES_GetStoredTMDSize(TitleID_IOS58, (u32*)&TMDSize);
    if (r < 0) {
        return r;
    }

    //gprintf("TMDSize:%u\r\n", TMDSize);

    TitleMetaData *TMD = (TitleMetaData*)memalign(32, TMDSize);
    if (!TMD) {
        return -1;
    }

    r = ES_GetStoredTMD(TitleID_IOS58, (signed_blob*)TMD, TMDSize);
    if (r < 0) {
        // Unable to load IOS58.
        //printf("ES_GetStoredTMD(0x%llX) failed: %d\r\n", TitleID_IOS58, r);
        free(TMD);
        return r;
    }

    //Look for boot index
    for (i = 0; i < TMD->ContentCount; ++i) {
        if (TMD->BootIndex == TMD->Contents[i].Index)
            break;
    }

    int cfd = IOS_Open("/shared1/content.map", 1);
    if (cfd < 0) {
        //printf("IOS_Open(\"/shared1/content.map\") failed: %d\r\n", cfd);
        free(TMD);
        return cfd;
    }

    for (u = 0; ; u += 0x1C) {
        if (IOS_Read(cfd, moduleSHA, 0x1C) != 0x1C) {
            //printf("Hash not found in content.map\r\n");
            IOS_Close(cfd);
            free(TMD);
            return -2;
        }

        if (memcmp((char*)(moduleSHA+8), TMD->Contents[i].SHA1, 0x14) == 0)
            break;
    }

    free(TMD);

    IOS_Close(cfd);

    memset(modulePath, 0, sizeof(modulePath));
    snprintf(modulePath, sizeof(modulePath), "/shared1/%.8s.app", moduleSHA);
    DCFlushRange(modulePath, sizeof(modulePath));

    //Open the actual IOS58 kernel file.
    int kfd = IOS_Open(modulePath, 1);
    if (kfd < 0) {
        return kfd;
    }

    *kernelSize = IOS_Seek(kfd, 0, SEEK_END);

    IOS_Close(kfd);
    return 0;
}

/**
 * Load and patch IOS.
 * @return 0 on success; negative on error.
 */
int loadKernel(char* kernel, u32* kernelSize, u32* FoundVersion) {
    unsigned int TMDSize;
    u32 size;
    int i, u;

    if (kernel == NULL) {
        return -1;
    }

    int r = ES_GetStoredTMDSize(TitleID_IOS58, (u32*)&TMDSize);
    if (r < 0) {
        return r;
    }

    //gprintf("TMDSize:%u\r\n", TMDSize);

    TitleMetaData *TMD = (TitleMetaData*)memalign(32, TMDSize);
    if (!TMD) {
        return -1;
    }

    r = ES_GetStoredTMD(TitleID_IOS58, (signed_blob*)TMD, TMDSize);
    if (r < 0) {
        // Unable to load IOS58.
        //printf("ES_GetStoredTMD(0x%llX) failed: %d\r\n", TitleID_IOS58, r);
        free(TMD);
        return r;
    }

    //Look for boot index
    for (i = 0; i < TMD->ContentCount; ++i) {
        if (TMD->BootIndex == TMD->Contents[i].Index)
            break;
    }

    int cfd = IOS_Open("/shared1/content.map", 1);
    if (cfd < 0) {
        //printf("IOS_Open(\"/shared1/content.map\") failed: %d\r\n", cfd);
        free(TMD);
        return cfd;
    }

    for (u = 0; ; u += 0x1C) {
        if (IOS_Read(cfd, moduleSHA, 0x1C) != 0x1C) {
            //printf("Hash not found in content.map\r\n");
            IOS_Close(cfd);
            free(TMD);
            return -2;
        }

        if (memcmp((char*)(moduleSHA+8), TMD->Contents[i].SHA1, 0x14) == 0)
            break;
    }

    *FoundVersion = ((TMD->TitleID & 0xFFFF) << 16) | (TMD->TitleVersion);
    free(TMD);

    IOS_Close(cfd);

    memset(modulePath, 0, sizeof(modulePath));
    snprintf(modulePath, sizeof(modulePath), "/shared1/%.8s.app", moduleSHA);
    DCFlushRange(modulePath, sizeof(modulePath));

    //Open the actual IOS58 kernel file.
    int kfd = IOS_Open(modulePath, 1);
    if (kfd < 0) {
        return kfd;
    }

    size = IOS_Seek(kfd, 0, SEEK_END);
    IOS_Seek(kfd, 0, 0);

    if (IOS_Read(kfd, kernel, size) != size) {
        IOS_Close(kfd);
        return -1;
    }

    if (kernelSize != NULL) {
        *kernelSize = size;
    }

    IOS_Close(kfd);
    return 0;
}

int loadIOSModules(void) {
    unsigned int TMDSize;

    int r = ES_GetStoredTMDSize(TitleID_IOS58, (u32*)&TMDSize);
    if (r < 0) {
        // IOS58 not found.
        //printf("ES_GetStoredTMDSize(0x%llX) failed: %d\r\n", TitleID_IOS58, r);
        return r;
    }

    //gprintf("TMDSize:%u\r\n", TMDSize);

    TitleMetaData *TMD = (TitleMetaData*)memalign(32, TMDSize);
    if (!TMD) {
        // Memory allocation failure.
        // NOTE: Not an IOS error, so we'll have to fake
        // a negative error code.
        //printf("Failed to alloc: %u\r\n", TMDSize);
        return -1;
    }

    r = ES_GetStoredTMD(TitleID_IOS58, (signed_blob*)TMD, TMDSize);
    if (r < 0) {
        // Unable to load IOS58.
        //printf("ES_GetStoredTMD(0x%llX) failed: %d\r\n", TitleID_IOS58, r);
        free(TMD);
        return r;
    }

    u8 EHCI_SHA1[] = {0xc6, 0x1b, 0xc1, 0x4a, 0xcf, 0xf2, 0x38, 0x7c, 0xcf, 0x1f, 0xce, 0x7a, 0x0a, 0xaa, 0xc2, 0xc7, 0x0e, 0x91, 0xc4, 0x2b};
    u8 KD_SHA1[] = {0xe4, 0x73, 0xfd, 0xa0, 0x35, 0x15, 0x12, 0x40, 0xd6, 0xa6, 0xf5, 0x3c, 0x50, 0x36, 0x9d, 0x24, 0x14, 0xc3, 0x6f, 0x4d};
    u8 KERNEL_SHA1[] = {0xa6, 0x52, 0x1c, 0x6b, 0xe9, 0x13, 0x65, 0xdc, 0xe1, 0x78, 0x4d, 0xc6, 0xf2, 0x95, 0xb7, 0xa2, 0xaa, 0x45, 0x5e, 0xf0};
    u8 NCD_SHA1[] = {0x1d, 0x18, 0x53, 0x55, 0x81, 0x5b, 0xf6, 0x4c, 0x45, 0x71, 0x71, 0x35, 0xe6, 0x54, 0x03, 0x3b, 0xab, 0x0c, 0x93, 0xbc};
    u8 OH1_SHA1[] = {0x3b, 0xc9, 0xe4, 0x16, 0xd3, 0xfa, 0x86, 0xd0, 0xa0, 0x99, 0xb8, 0x2e, 0xce, 0x0a, 0xa7, 0xef, 0xf8, 0xf5, 0x9a, 0x68};
    u8 OHCI0_SHA1[] = {0x0b, 0xc6, 0x31, 0xbe, 0xd4, 0x20, 0x5f, 0x18, 0xa0, 0x05, 0x95, 0x0f, 0xc7, 0x46, 0xa5, 0x3c, 0xd4, 0xe7, 0x30, 0x7b};
    u8 SDI_SHA1[] = {0xeb, 0x3d, 0xbe, 0xa9, 0xdc, 0xbf, 0x57, 0xe4, 0x46, 0x35, 0x4a, 0xe2, 0xbd, 0xe1, 0xd8, 0x8c, 0x65, 0x6c, 0x90, 0xa7};
    u8 SO_SHA1[] = {0x00, 0xef, 0x2f, 0x8b, 0xbc, 0xd2, 0x08, 0xeb, 0x5e, 0x64, 0xcd, 0x91, 0x65, 0x54, 0x40, 0x5a, 0xd3, 0x4e, 0xcf, 0xd5};
    u8 SSL_SHA1[] = {0x51, 0x37, 0xd0, 0x39, 0x3c, 0x98, 0x09, 0x7f, 0x0d, 0x59, 0x70, 0x6d, 0x1c, 0xee, 0xc2, 0x2f, 0x75, 0x48, 0xd6, 0x0c};
    u8 STM_SHA1[] = {0xff, 0x38, 0x0d, 0x01, 0x88, 0xfd, 0x07, 0x30, 0xba, 0xf8, 0x37, 0xd4, 0x78, 0x3e, 0xa6, 0xa1, 0x8c, 0x84, 0x7b, 0x27};
    u8 USB_SHA1[] = {0x7b, 0x17, 0x97, 0x6f, 0xc2, 0x0d, 0x77, 0xda, 0x12, 0xca, 0xe4, 0x36, 0xad, 0x72, 0xfe, 0x8d, 0x12, 0x1a, 0xb4, 0xd2};
    u8 USB_HID_SHA1[] = {0x5d, 0x08, 0x25, 0xca, 0xda, 0xb0, 0x73, 0xa6, 0x59, 0x44, 0x49, 0xcb, 0x37, 0x0c, 0x66, 0xd1, 0x53, 0xca, 0x11, 0xc2};
    u8 USB_HUB_SHA1[] = {0x8a, 0x14, 0x37, 0x5f, 0x68, 0x90, 0x95, 0xc2, 0x79, 0xae, 0xe9, 0x0e, 0xbc, 0xe8, 0xb3, 0x7d, 0x0c, 0x39, 0xe4, 0x67};
    u8 USB_VEN_SHA1[] = {0xd3, 0xc2, 0x3a, 0x17, 0x2a, 0xff, 0x83, 0x78, 0x05, 0xf4, 0xe6, 0xb2, 0xdb, 0x13, 0x72, 0xae, 0x93, 0x6c, 0xe4, 0xed};
    u8 ETH_SHA1[] = {0xcc, 0x77, 0x66, 0x39, 0x56, 0x2b, 0xa2, 0x98, 0xd6, 0x65, 0x98, 0xef, 0xe0, 0x34, 0x0b, 0x42, 0x55, 0xbf, 0x4b, 0x6c};
    u8 WD_SHA1[] = {0x79, 0xa9, 0xf2, 0x15, 0xea, 0x9b, 0x38, 0xfd, 0x08, 0x5d, 0xa3, 0x27, 0x20, 0xa7, 0xd3, 0x98, 0x18, 0xc8, 0x70, 0x1d};
    u8 WL_SHA1[] = {0xd1, 0xf6, 0x0d, 0x86, 0x43, 0xc8, 0xdc, 0xb4, 0x72, 0xd8, 0x41, 0x5a, 0x2e, 0x45, 0x8a, 0x64, 0xbc, 0x5d, 0x14, 0xdc};

    int cfd = IOS_Open("/shared1/content.map", 1);
    if (cfd < 0) {
        //printf("IOS_Open(\"/shared1/content.map\") failed: %d\r\n", cfd);
        free(TMD);
        return cfd;
    }

    while (1) {
        if (IOS_Read(cfd, moduleSHA, 0x1C) != 0x1C) //EOF
            break;
        memset(modulePath, 0, sizeof(modulePath));
        snprintf(modulePath, sizeof(modulePath), "/shared1/%.8s.app", moduleSHA);
        DCFlushRange(modulePath, sizeof(modulePath));

        FILE* fp = NULL;
        
        if (!memcmp((char*)(moduleSHA+8), EHCI_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/EHCI.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/EHCI.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), KD_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/KD.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/KD.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), KERNEL_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/KERNEL.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/KERNEL.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), NCD_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/NCD.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/NCD.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), OH1_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/OH1.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/OH1.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), OHCI0_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/OHCI0.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/OHCI0.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), SDI_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/SDI.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/SDI.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), SO_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/SO.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/SO.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), SSL_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/SSL.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/SSL.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), STM_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/STM.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/STM.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), USB_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/USB.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/USB.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), USB_HID_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/USB_HID.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/USB_HID.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), USB_HUB_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/USB_HUB.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/USB_HUB.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), USB_VEN_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/USB_VEN.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/USB_VEN.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), ETH_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/ETH.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/ETH.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), WD_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/WD.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/WD.app", "wb");
        }
        if (!memcmp((char*)(moduleSHA+8), WL_SHA1, 0x14)) {
            fp = fopen("/rvloader/Hiidra/IOS58/WL.app", "rb");
            if (fp) {
                fclose(fp);
                continue; //No need to write multiple times
            }
            fp = fopen("/rvloader/Hiidra/IOS58/WL.app", "wb");
        }

        if (fp!= NULL) {
            int fd = IOS_Open(modulePath, 1);
            if (fd < 0) {
                fclose(fp);
                continue;
            }

            u32 moduleSize = IOS_Seek(fd, 0, SEEK_END);
            IOS_Seek(fd, 0, 0);

            while (moduleSize) {
                u32 packetSize = (moduleSize > MODULE_BUFFER_SIZE) ? MODULE_BUFFER_SIZE : moduleSize;
                printf("Packet size: %08X\n", packetSize);
                printf("IOS_Read ret: %d\n", IOS_Read(fd, moduleBuffer, packetSize));
                fwrite(moduleBuffer, 1, packetSize, fp);
                moduleSize -= packetSize;
            }
            fclose(fp);
            IOS_Close(fd);
        }
    }

    IOS_Close(cfd);

    free(TMD);
    return 0;
}

void setESBootpoint(u32 IOSVersion, u32 bootPoint) {
    unsigned char ESBootPatch[] = {
        0x48, 0x03, 0x49, 0x04, 0x47, 0x78, 0x46, 0xC0, 0xE6, 0x00, 0x08, 0x70, 0xE1, 0x2F, 0xFF, 0x1E,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    *((vu32*)&ESBootPatch[0x10]) = MEM_VIRTUAL_TO_PHYSICAL(bootPoint);
    *((vu32*)&ESBootPatch[0x14]) = IOSVersion;

    DCInvalidateRange((void*)0x939F02F0, sizeof(ESBootPatch));
    memcpy((void*)0x939F02F0, ESBootPatch, sizeof(ESBootPatch));
    DCFlushRange((void*)0x939F02F0, sizeof(ESBootPatch));
}

void copyNANDDirToUSB(const char* src, const char* dst) {
    printf("copyNANDDirToUSB(%s, %s)\n", src, dst);
    char srcPath[256];
    char dstPath[256];
    fstats fileStats ALIGNED(32);
    u32 num, i;
    u32 usage1, usage2;
    mkdir(dst, 777);
    s32 ret = ISFS_ReadDir(src, NULL, &num);
    void* buffer = (void*)memalign(32, COPY_LIMIT);
    if (ret >= 0) {
        char* filelist = (char*)memalign(32, ISFS_MAXPATH * num);
        char* tempfile = filelist;
        ret = ISFS_ReadDir(src, filelist, &num);
        if (ret >= 0) {
            for (i = 0; i < num; i++) {
                sprintf(srcPath, "%s/%s", src, tempfile);
                sprintf(dstPath, "%s/%s", dst, tempfile);

                printf("%s is ", tempfile);

                //Check if file
                s32 fd = IOS_Open(srcPath, 1);
                if (fd >= 0) {
                    printf("file\n");
                    FILE* fp = fopen(dstPath, "w");
                    ISFS_GetFileStats(fd, &fileStats);
                    if (fileStats.file_length <= COPY_LIMIT) {
                        ISFS_Read(fd, buffer, fileStats.file_length);
                        fwrite(buffer, 1, fileStats.file_length, fp);
                    } else {
                        while (fileStats.file_length) {
                            u32 tempLen = (fileStats.file_length > COPY_LIMIT) ? COPY_LIMIT : fileStats.file_length;
                            ISFS_Read(fd, buffer, tempLen);
                            fwrite(buffer, 1, tempLen, fp);
                            fileStats.file_length -= tempLen;
                        }
                    }
                    fclose(fp);
                    IOS_Close(fd);
                    tempfile += strlen(tempfile) + 1; //Go to next file
                    continue;
                }

                //Check if dir
                if (ISFS_GetUsage(srcPath, &usage1, &usage2) >= 0) {
                    printf("dir\n");
                    copyNANDDirToUSB(srcPath, dstPath);
                    tempfile += strlen(tempfile) + 1; //Go to next file
                    continue;
                }

                printf("unk\n");
                tempfile += strlen(tempfile) + 1; //Go to next file

            }
        }
        free(filelist);
    }
    free(buffer);
}

void copyWBFSData(const char* wbfsPath) {
    u32 gameID;
    u64 majorTitleID;
    char path[256];
    char path2[256];
    s32 ret;

    ret = ISFS_Initialize();
    if (ret != ISFS_OK)
        return;

    FILE* fp = fopen(wbfsPath, "r");
    if (!fp) {
        ISFS_Deinitialize();
        return;
    }

    //Read ID
    fseek(fp, 0x200, SEEK_SET);
    fread(&gameID, 1, sizeof(u32), fp);
    fclose(fp);

    //Check complete titleID.
    //Must be 00010000-gameID or 00010004-gameID for disc games
    sprintf(path, "/title/00010000/%08x/content/title.tmd", gameID);
    ret = ISFS_Open(path, 1);
    if (ret >= 0) {
        ISFS_Close(ret);
        //Check if save already exists
        sprintf(path, "/rvloader/Hiidra/emunand/title/00010000/%08x/content/title.tmd", gameID);
        FILE* fp = fopen(path, "r");
        if (fp) {
            fclose(fp);
            ISFS_Deinitialize();
            return;
        }
        mkdir("/rvloader/Hiidra/emunand/title", 777);
        mkdir("/rvloader/Hiidra/emunand/title/00010000", 777);
        sprintf(path, "/title/00010000/%08x", gameID);
        sprintf(path2, "/rvloader/Hiidra/emunand/title/00010000/%08x", gameID);
        copyNANDDirToUSB(path, path2);
        ISFS_Deinitialize();
        return;
    }

    sprintf(path, "/title/00010004/%08x/content/title.tmd", gameID);
    ret = ISFS_Open(path, 1);
    if (ret >= 0) {
        //Check if save already exists
        sprintf(path, "/rvloader/Hiidra/emunand/title/00010004/%08x/content/title.tmd", gameID);
        FILE* fp = fopen(path, "r");
        if (fp) {
            fclose(fp);
            ISFS_Deinitialize();
            return;
        }
        mkdir("/rvloader/Hiidra/emunand/title", 777);
        mkdir("/rvloader/Hiidra/emunand/title/00010004", 777);
        sprintf(path, "/title/00010004/%08x", gameID);
        sprintf(path2, "/rvloader/Hiidra/emunand/title/00010004/%08x", gameID);
        copyNANDDirToUSB(path, path2);
        ISFS_Deinitialize();
        return;
    }
}

static void* bootHiidraThread(void* arg) {
    HIIDRA_CFG hcfg = *(HIIDRA_CFG*)arg;
    u32 gameIDU32 = *(u32*)((u32)arg + sizeof(HIIDRA_CFG));
    bool forceReinstall = *(bool*)((u32)arg + sizeof(HIIDRA_CFG) + sizeof(u32));

    static char __di[] ATTRIBUTE_ALIGN(32) = "/dev/di";
    static char __bt[] ATTRIBUTE_ALIGN(32) = "/dev/usb/oh1/57e/305";

    u32 IOSVersion;
    u32 kernelSize;
    uint8_t* kernelModule;
    uint8_t* usbfsModule;
    uint8_t* fspluginModule;
    uint8_t* modulesUSB[4];
    u32 modulesUSBSize[4];
    char* kernelAddress;
    u32 totalUSBSize = 0;

    s32 fd;
    u64 wadTitleID = 0;

    mkdir("/rvloader", 777);
    mkdir("/rvloader/Hiidra", 777);
    mkdir("/rvloader/Hiidra/IOS58", 777);
    mkdir("/rvloader/Hiidra/emunand", 777);

    //Delete lastdisc.sys before booting
    remove("/lastdisc.sys");
    
    remove("/rvloader/Hiidra/modules.txt");
    if (hcfg.Config & HIIDRA_CFG_BT) {
        FILE* fp = fopen("/rvloader/Hiidra/modules.txt", "a");
        fprintf(fp, "/dev/usbfs/rvloader/Hiidra/IOS58/OH1.app\n");
        fclose(fp);
    } else {
        FILE* fp = fopen("/rvloader/Hiidra/modules.txt", "a");
        fprintf(fp, "/dev/usbfs/rvloader/Hiidra/modules/bt.bin\n");
        fclose(fp);
    }
    if (hcfg.Config & HIIDRA_CFG_WIFI) {
        FILE* fp = fopen("/rvloader/Hiidra/modules.txt", "a");
        fprintf(fp, "/dev/usbfs/rvloader/Hiidra/IOS58/WD.app\n");
        fprintf(fp, "/dev/usbfs/rvloader/Hiidra/IOS58/WL.app\n");
        fclose(fp);
    } else {
        FILE* fp = fopen("/rvloader/Hiidra/modules.txt", "a");
        fprintf(fp, "/dev/usbfs/rvloader/Hiidra/modules/wd.bin\n");
        fclose(fp);
    }

    if (!strncmp(&hcfg.GamePath[strlen(hcfg.GamePath) - 4], ".wad", 4)) {
        printf("Installing WAD\n");
        hiidraAddLogLine("Starting WAD installation");
        if (!openAndInstallWAD(hcfg.GamePath, &wadTitleID, forceReinstall)) {
            hiidraAddLogLine("Error while installing WAD");
            while(1);
        }
        hcfg.TitleID = wadTitleID;
    } else {
        hcfg.TitleID = 0;
    }

    if (!strncmp(&hcfg.GamePath[strlen(hcfg.GamePath) - 5], ".wbfs", 5) && (hcfg.Config & HIIDRA_CFG_USBSAVES)) {
        hiidraAddLogLine("Copying game data");
        copyWBFSData(hcfg.GamePath);
    }

    writeFile("/rvloader/Hiidra/boot.cfg", &hcfg, sizeof(HIIDRA_CFG));

    if (getKernelSize(&kernelSize)  < 0) {
        return NULL;
    }

    printf("IOS58 kernel size: %u\n", kernelSize);

    kernelAddress = (char*)SYS_AllocArena2MemLo(512*1024, 0x20);
    printf("Kernel address: %08X\n", MEM_VIRTUAL_TO_PHYSICAL((u32)kernelAddress));

    loadIOSModules();
    hiidraAddLogLine("Loading kernel from NAND");
    if (loadKernel(kernelAddress, NULL, &IOSVersion) < 0) {
        printf("Couldn't load kernel\n");
        exit(0);
    }
    printf("Kernel loaded. Kernel version: %08X\n", IOSVersion);

    readFile("/rvloader/Hiidra/IOS58/EHCI.app", &modulesUSB[0], &modulesUSBSize[0]);
    readFile("/rvloader/Hiidra/IOS58/OHCI0.app", &modulesUSB[1], &modulesUSBSize[1]);
    readFile("/rvloader/Hiidra/IOS58/USB.app", &modulesUSB[2], &modulesUSBSize[2]);
    readFile("/rvloader/Hiidra/IOS58/USB_VEN.app", &modulesUSB[3], &modulesUSBSize[3]);

    readFile("/rvloader/Hiidra/modules/kernel_es.elf", &kernelModule, NULL);
    readFile("/rvloader/Hiidra/modules/usbfs.elf", &usbfsModule, NULL);
    readFile("/rvloader/Hiidra/modules/fsplugin.elf", &fspluginModule, NULL);

    for (int i = 0; i < sizeof(modulesUSBSize) / sizeof(u32); i++) {
        totalUSBSize += modulesUSBSize[i];
    }

    printf("Total USB modules size: %u\n", totalUSBSize);

    printf("Forging custom kernel\n");
    hiidraAddLogLine("Forging custom kernel");
    const uint8_t* customModules[] = {kernelModule, modulesUSB[0], modulesUSB[1], modulesUSB[2], modulesUSB[3], usbfsModule, fspluginModule};
    forgeKernel(kernelAddress, kernelSize, customModules, sizeof(customModules) / sizeof(uint8_t*), 0, 1);
    
    unmountFAT();
    printf("Injecting Hiidra bootpoint\n");
    setESBootpoint(IOSVersion, (u32)kernelAddress);

    __ES_Close();
    fd = IOS_Open("/dev/es", 0);

    *(vu32*)0xD3003420 = 0; //make sure kernel doesnt reload

    printf("Triggering Hiidra boot\n");
    hiidraAddLogLine("Triggering Hiidra boot");
    raw_irq_handler_t irq_handler = BeforeIOSReload();
    IOS_IoctlvAsync(fd, 0x1F, 0, 0, &IOCTL_Buf, NULL, NULL);
    AfterIOSReload(irq_handler, IOSVersion);

    printf("Hiidra is running\n");
    hiidraAddLogLine("Hiidra is running");

    udelay(100000);

    write16(MEM_PROT, 0);

    s32 hId = iosCreateHeap(1024);
    printf("Waiting for ES\n");
    hiidraAddLogLine("Waiting for ES");
    do {
        udelay(1000);
        fd = IOS_Open("/dev/es", 0);
    } while (fd < 0);

    if (!(hcfg.Config & HIIDRA_CFG_BT) && (hcfg.Config & HIIDRA_CFG_GC2WIIMOTE)) {
        printf("Enabling GC2Wiimote\n");
        IOS_Ioctl(fd, ES_IOCTL_ENABLEGC2WIIMOTE, NULL, 0, NULL, 0);
    }

    if (hcfg.Config & HIIDRA_CFG_USBSAVES) {
        printf("Enabling USB saves\n");
        IOS_Ioctl(fd, ES_IOCTL_ENABLEUSBSAVES, NULL, 0, NULL, 0);
    }
    //Enable VGA if requested to
    if (hcfg.Config & HIIDRA_CFG_VGA) {
        printf("Enabling VGA\n");
        IOS_Ioctl(fd, ES_IOCTL_ENABLEVGA, NULL, 0, NULL, 0);
    }
    char* gamePath = (char*)iosAlloc(hId, 0x80);
    sprintf(gamePath, "/dev/usbfs%s", hcfg.GamePath);

    printf("Launching %s\n", gamePath);
    hiidraAddLogLine("Launching %s\n", gamePath);
    if (wadTitleID) {
        u64* titleID = (u64*)iosAlloc(hId, sizeof(u64));
        *titleID = wadTitleID;
        IOS_Ioctl(fd, ES_IOCTL_LAUNCHTITLEID, titleID, sizeof(u64), NULL, 0);
        IOS_Close(fd);
        while(1);
    }

    IOS_Close(fd);

    fd = IOS_Open(__di, 0);
    if (fd >= 0) {
        s32 ret;
        printf("Loading disc game\n");
        hiidraAddLogLine("Loading disc game");
        ret = IOS_Ioctl(fd, DIP_IOCTL_DVDLoadGame, gamePath, 0x80, NULL, 0);
        if (ret == 0) {
            printf("Success\n");
        } else {
            printf("Error: %d\n", ret);
        }
        IOS_Close(fd);
    } else {
        printf("Couldn't open DI\n");
        hiidraAddLogLine("Couldn't open DI");
        while(1);
    }

    printf("Booting discloader\n");
    hiidraAddLogLine("Booting discloader");
    insertDisc = 1; // This flag tells discAnim() to enter the last part of the animation where the disc is inserted in the tray
    udelay(1000000); // 1 second delay to allow for the disc to enter the tray before loading bootDiscLoader which changes the scene
    bootDiscLoader(hideLogLines);
    
    return NULL;
}

void initHiidra() {
    LWP_MutexInit(&logMutex, false);
    LWP_InitQueue(&logQueue);
    logLineIndex = 0;
    logNLines = 0;
}

void lockHiidraLogMutex() {
    if (logMutex) {
        LWP_MutexLock(logMutex);
    }
}

void unlockHiidraLogMutex() {
    if (logMutex) {
        LWP_MutexUnlock(logMutex);
    }
}

void hiidraSignalRedraw() {
    LWP_ThreadSignal(logQueue);
}

void hiidraWaitForRedraw() {
    LWP_ThreadSleep(logQueue);
}

u32 hiidraAddLogLine(const char* line, ...) {
    if(hideLogLines) return 0;
    va_list args;
    u32 prevIndex;

    lockHiidraLogMutex();

    prevIndex = logLineIndex;

    va_start(args, line);
    vsnprintf(logBuffer[logLineIndex], HIIDRA_LOG_LINE_LENGTH, line, args);
    va_end(args);

    logLineIndex++;
    if (logLineIndex == HIIDRA_LOG_LINES) {
        logLineIndex = 0;
    }
    if (logNLines < HIIDRA_LOG_LINES) {
        logNLines++;
    }

    unlockHiidraLogMutex();

    forceRedraw();
    hiidraWaitForRedraw();

    return prevIndex;
}

void hiidraUpdateLogLine(u32 index, const char* line, ...) {
    if(hideLogLines) return;
    va_list args;
    lockHiidraLogMutex();
    va_start(args, line);
    vsnprintf(logBuffer[index], HIIDRA_LOG_LINE_LENGTH, line, args);
    va_end(args);
    unlockHiidraLogMutex();

    forceRedraw();
    hiidraWaitForRedraw();
}

static int lua_getLogLines(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_newtable(L);

    int index = 1;

    lockHiidraLogMutex();

    for (u32 i = 0; i < logNLines; i++) {
        //This will copy the data as if logBuffer was handled as a circular buffer
        luaSetArrayStringField(L, index++, logBuffer[(i + HIIDRA_LOG_LINES - logNLines + logLineIndex) % HIIDRA_LOG_LINES]);
    }

    unlockHiidraLogMutex();

    return 1;
}

static const luaL_Reg Hiidra_functions[] = {
    {"getLogLines", lua_getLogLines},
    {NULL, NULL}
};

void luaRegisterHiidraLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Hiidra_functions, 0);
    lua_setglobal(L, "Hiidra");
}

int bootHiidra(HIIDRA_CFG hcfg, u32 gameIDU32, std::string gameIDString, std::vector<uint32_t> cheats, bool forceReinstall) {
    static lwp_t bootHiidraThreadHandle;
    static void* bootHiidraThreadArg = NULL;
    static u8* bootHiidraThreadStack = NULL;
    int wiiLoad;
    bool isWiiGame = false;
    char wbfsPath[6] = "/wbfs";

    patchFSAccess();
    
    mainConfig.getValue("WiiLoadScreen", &wiiLoad);
    if(strncmp(hcfg.GamePath, wbfsPath, 5) == 0) isWiiGame = true;

    if (bootHiidraThreadArg != NULL) {
        return -1;
    }

    if (hcfg.Config & HIIDRA_CFG_CHEATS) {
        writeFile("/rvloader/Hiidra/cheats.gct", cheats.data(), cheats.size() * sizeof(uint32_t));
    }

    bootHiidraThreadArg = (void*)malloc(sizeof(HIIDRA_CFG) + sizeof(u32) + sizeof(bool));
    memcpy(bootHiidraThreadArg, &hcfg, sizeof(HIIDRA_CFG));
    memcpy((void*)((u32)bootHiidraThreadArg + sizeof(HIIDRA_CFG)), &gameIDU32, sizeof(u32));
    memcpy((void*)((u32)bootHiidraThreadArg + sizeof(HIIDRA_CFG) + sizeof(u32)), &forceReinstall, sizeof(bool));
    bootHiidraThreadStack = (u8*)memalign(32, STACKSIZE);

    //Disable any controller input
    disableControllers();

    //Switch to Hiidra's loading screen
    if(!wiiLoad || !isWiiGame){
        hideLogLines = false;
        mainWindowSwitchElement("HiidraBootScreen");
        enableControlledRedraw();
        LWP_CreateThread(&bootHiidraThreadHandle, bootHiidraThread, bootHiidraThreadArg, bootHiidraThreadStack, STACKSIZE, 50);
    } else{
        hideLogLines = true;
        u8* arcData = ashExtract();
        enableControlledRedraw();
        LWP_CreateThread(&bootHiidraThreadHandle, bootHiidraThread, bootHiidraThreadArg, bootHiidraThreadStack, STACKSIZE, 50);
        discAnim(arcData, gameIDString);
    }
    
    return 0;
}
