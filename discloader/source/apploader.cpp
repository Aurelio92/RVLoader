#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <vector>

#include "apploader.h"
#include "wdvd.h"
#include "disc.h"
#include "hiidratypes.h"

/* Constants */
#define APPLDR_OFFSET 0x2440
#ifndef APPLOADER_START
#define APPLOADER_START (void *)0x81200000
#endif
#ifndef APPLOADER_END
#define APPLOADER_END (void *)0x81700000
#endif

#define IOCTL_GAMEPATCHER_ALLOC_GCT     0x00
#define ISFS_IOCTL_SETDOLHEADER 101

static u8 dolHeaderBuffer[0x100] ATTRIBUTE_ALIGN(32);

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(app_init* report, app_main* main, app_final* final);

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

static void readCheats();
static bool maindolpatches(void *dst, int len, u8 deflicker);
static bool Remove_001_Protection(void *Address, int Size);
static void Anti_002_fix(void *Address, int Size);

static void __noprint(const char *fmt, ...)
{
}

s32 Apploader_Run(entry_point* entry) {
    HIIDRA_CFG hiidraCfg ALIGNED(32);
    void *dst = NULL;
    int len = 0;
    int offset = 0;
    s32 ret;
    app_init  appldr_init;
    app_main  appldr_main;
    app_final appldr_final;

    /* Read apploader header */
    ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
    if (ret < 0) return ret;

    /* Calculate apploader length */
    u32 appldr_len = buffer[5] + buffer[6];

    SYS_SetArena1Hi(APPLOADER_END);

    /* Read apploader code */
    // Either you limit memory usage or you don't touch the heap after that, because this is writing at 0x1200000
    ret = WDVD_Read(APPLOADER_START, appldr_len, APPLDR_OFFSET + 0x20);
    if (ret < 0) return ret;

    DCFlushRange(APPLOADER_START, appldr_len);

    /* Set apploader entry function */
    app_entry appldr_entry = (app_entry)buffer[4];

    /* Call apploader entry */
    printf("appldr_entry\n");
    appldr_entry(&appldr_init, &appldr_main, &appldr_final);

    /* Initialize apploader */
    printf("appldr_init\n");
    appldr_init(__noprint);

    //Read hiidraCfg
    s32 hiidraCfgFD = IOS_Open("$rvloader/Hiidra/boot.cfg", 1);
    if (hiidraCfgFD >= 0) {
        IOS_Read(hiidraCfgFD, (void*)&hiidraCfg, sizeof(HIIDRA_CFG));
        IOS_Close(hiidraCfgFD);
    } else {
        hiidraCfg.DeflickerMode = HIIDRA_DEFLICKER_AUTO;
    }

    printf("Deflicker mode: %u\n", hiidraCfg.DeflickerMode);

    while (appldr_main(&dst, &len, &offset)) {
        /* Read data from DVD */
        ret = WDVD_Read(dst, len, (u64)(offset << 2));

        maindolpatches(dst, len, hiidraCfg.DeflickerMode);
    }
    printf("appldr_main (done)\n");

    //Read mainDOL disc
    WDVD_GetDOLHeader(dolHeaderBuffer);
    s32 fdFS = IOS_Open("/dev/fs", 0);
    if (fdFS >= 0) {
        IOS_Ioctl(fdFS, ISFS_IOCTL_SETDOLHEADER, dolHeaderBuffer, 0x100, NULL, 0);
        IOS_Close(fdFS);
    }
    
    printf("Patching game\n");
    IOS_Open("/dev/patch_game", 0); //This internally returns IPC_ENOENT
    printf("Patched\n");

    /* Set entry point from apploader */
    *entry = (entry_point)appldr_final();

    readCheats();

    // IOSReloadBlock(IOS_GetVersion());
    *(vu32 *)0x80003140 = *(vu32 *)0x80003188; // IOS Version Check
    *(vu32 *)0x80003180 = *(vu32 *)0x80000000; // Game ID Online Check
    *(vu32 *)0x80003184 = 0x80000000;

    DCFlushRange((void*)0x80000000, 0x3f00);

    return 0;
}

static void readCheats() {
    s32 ret;
    s32 hId = iosCreateHeap(1024);
    u32 codeHandlerSize = 0;
    u32 cheatsSize = 0;
    u8* fileBuffer = (u8*)iosAlloc(hId, sizeof(1024));

    if (fileBuffer == NULL) {
        return;
    }
    
    s32 codeHandlerFD = IOS_Open("$rvloader/Hiidra/codehandleronly.bin", ISFS_OPEN_READ);
    if (codeHandlerFD < 0) {
        iosFree(hId, fileBuffer);
        return;
    }
    while ((ret = IOS_Read(codeHandlerFD, fileBuffer, 1024)) > 0) {
        memcpy((void*)(0x80001900 + codeHandlerSize), fileBuffer, ret);
        codeHandlerSize += ret;
    }
    DCFlushRange((void*)0x80001900, codeHandlerSize);
    IOS_Close(codeHandlerFD);

    s32 cheatsFD = IOS_Open("$rvloader/Hiidra/cheats.gct", ISFS_OPEN_READ);
    if (cheatsFD < 0) {
        iosFree(hId, fileBuffer);
        return;
    }
    while ((ret = IOS_Read(cheatsFD, fileBuffer, 1024)) > 0) {
        memcpy((void*)(0x80001900 + codeHandlerSize - 8 + cheatsSize), fileBuffer, ret);
        cheatsSize += ret;
    }
    DCFlushRange((void*)(0x80001900 + codeHandlerSize - 8), cheatsSize);
    IOS_Close(cheatsFD);
    
    iosFree(hId, fileBuffer);
}

static void PatchCountryStrings(void *Address, int Size)
{
    u8 SearchPattern[4] = {0x00, 0x00, 0x00, 0x00};
    u8 PatchData[4] = {0x00, 0x00, 0x00, 0x00};
    u8 *Addr = (u8*)Address;
    int wiiregion = CONF_GetRegion();

    switch (wiiregion)
    {
        case CONF_REGION_JP:
            SearchPattern[0] = 0x00;
            SearchPattern[1] = 'J';
            SearchPattern[2] = 'P';
            break;
        case CONF_REGION_EU:
            SearchPattern[0] = 0x02;
            SearchPattern[1] = 'E';
            SearchPattern[2] = 'U';
            break;
        case CONF_REGION_KR:
            SearchPattern[0] = 0x04;
            SearchPattern[1] = 'K';
            SearchPattern[2] = 'R';
            break;
        case CONF_REGION_CN:
            SearchPattern[0] = 0x05;
            SearchPattern[1] = 'C';
            SearchPattern[2] = 'N';
            break;
        case CONF_REGION_US:
        default:
            SearchPattern[0] = 0x01;
            SearchPattern[1] = 'U';
            SearchPattern[2] = 'S';
    }
    switch (((const u8 *)0x80000000)[3])
    {
        case 'J':
            PatchData[1] = 'J';
            PatchData[2] = 'P';
            break;
        case 'D':
        case 'F':
        case 'P':
        case 'X':
        case 'Y':
            PatchData[1] = 'E';
            PatchData[2] = 'U';
            break;

        case 'E':
        default:
            PatchData[1] = 'U';
            PatchData[2] = 'S';
    }
    while (Size >= 4)
        if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
        {
            //*Addr = PatchData[0];
            Addr += 1;
            *Addr = PatchData[1];
            Addr += 1;
            *Addr = PatchData[2];
            Addr += 1;
            //*Addr = PatchData[3];
            Addr += 1;
            Size -= 4;
        }
        else
        {
            Addr += 4;
            Size -= 4;
        }
}

static void patch_NoDiscinDrive(void *buffer, u32 len)
{
    static const u8 oldcode[] = {0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x41, 0x82, 0x00, 0x0C};
    static const u8 newcode[] = {0x54, 0x60, 0xF7, 0xFF, 0x40, 0x82, 0x00, 0x0C, 0x54, 0x60, 0x07, 0xFF, 0x48, 0x00, 0x00, 0x0C};
    int n;

   /* Patch cover register */
    for (n = 0; n < len - sizeof oldcode; n += 4) // n is not 4 aligned here, so you can get an out of buffer thing
    {
        if (memcmp(buffer + n, (void *)oldcode, sizeof oldcode) == 0)
            memcpy(buffer + n, (void *)newcode, sizeof newcode);
    }
}

void deflicker_patch(u8 *addr, u32 len) {
    u32 SearchPattern[18] = {
        0x3D20CC01, 0x39400061, 0x99498000,
        0x2C050000, 0x38800053, 0x39600000,
        0x90098000, 0x38000054, 0x39800000,
        0x508BC00E, 0x99498000, 0x500CC00E,
        0x90698000, 0x99498000, 0x90E98000,
        0x99498000, 0x91098000, 0x41820040};
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - sizeof(SearchPattern);
    while (addr_start <= addr_end) {
        if (memcmp(addr_start, SearchPattern, sizeof(SearchPattern)) == 0) {
            *((u32 *)addr_start + 17) = 0x48000040; // Change beq to b
            printf("Patched GXSetCopyFilter @ %p\n", addr_start);
            return;
        }
        addr_start += 4;
    }
}

void patch_vfilters(u8 *addr, u32 len, u8 *vfilter) {
    static u8 PATTERN[12][2] = {
        {6, 6}, {6, 6}, {6, 6},
        {6, 6}, {6, 6}, {6, 6},
        {6, 6}, {6, 6}, {6, 6},
        {6, 6}, {6, 6}, {6, 6}
    };

    static u8 PATTERN_AA[12][2] = {
        {3, 2}, {9, 6}, {3, 10},
        {3, 2}, {9, 6}, {3, 10},
        {9, 2}, {3, 6}, {9, 10},
        {9, 2}, {3, 6}, {9, 10}
    };
    u8 *addr_start = addr;
    while (len >= sizeof(GXRModeObj)) {
        GXRModeObj* vidmode = (GXRModeObj*)addr_start;
        if ((memcmp(vidmode->sample_pattern, PATTERN, 24) == 0 || memcmp(vidmode->sample_pattern, PATTERN_AA, 24) == 0) &&
            (vidmode->fbWidth == 640 || vidmode->fbWidth == 608 || vidmode->fbWidth == 512) &&
            (vidmode->field_rendering == 0 || vidmode->field_rendering == 1) &&
            (vidmode->aa == 0 || vidmode->aa == 1)) {
            printf("Replaced vfilter %02x%02x%02x%02x%02x%02x%02x @ %p (GXRModeObj)\n",
                    vidmode->vfilter[0], vidmode->vfilter[1], vidmode->vfilter[2], vidmode->vfilter[3],
                    vidmode->vfilter[4], vidmode->vfilter[5], vidmode->vfilter[6], addr_start);
            memcpy(vidmode->vfilter, vfilter, 7);
            addr_start += (sizeof(GXRModeObj) - 4);
            len -= (sizeof(GXRModeObj) - 4);
        }
        addr_start += 4;
        len -= 4;
    }
}

// Patch rogue vfilters found in some games
void patch_vfilters_rogue(u8 *addr, u32 len, u8 *vfilter) {
    u8 known_vfilters[7][7] = {
        {8, 8, 10, 12, 10, 8, 8},
        {4, 8, 12, 16, 12, 8, 4},
        {7, 7, 12, 12, 12, 7, 7},
        {5, 5, 15, 14, 15, 5, 5},
        {4, 4, 15, 18, 15, 4, 4},
        {4, 4, 16, 16, 16, 4, 4},
        {2, 2, 17, 22, 17, 2, 2}
    };
    u8 *addr_start = addr;
    u8 *addr_end = addr + len - 8;
    while (addr_start <= addr_end) {
        u8 known_vfilter[7];
        for (int i = 0; i < 7; i++) {
            for (int x = 0; x < 7; x++)
                known_vfilter[x] = known_vfilters[i][x];
            if (!addr_start[7] && memcmp(addr_start, known_vfilter, 7) == 0) {
                printf("Replaced vfilter %02x%02x%02x%02x%02x%02x%02x @ %p\n", addr_start[0], addr_start[1],
                        addr_start[2], addr_start[3], addr_start[4], addr_start[5], addr_start[6], addr_start);
                memcpy(addr_start, vfilter, 7);
                addr_start += 7;
                break;
            }
        }
        addr_start += 1;
    }
}

static bool maindolpatches(void *dst, int len, u8 deflicker) {
        u8 vfilter_off[7] = {0, 0, 21, 22, 21, 0, 0};
        u8 vfilter_low[7] = {4, 4, 16, 16, 16, 4, 4};
        u8 vfilter_medium[7] = {4, 8, 12, 16, 12, 8, 4};
        u8 vfilter_high[7] = {8, 8, 10, 12, 10, 8, 8};
        bool ret = false;
        u32 i;

        ICInvalidateRange(dst, len);

        Remove_001_Protection(dst, len);
        Anti_002_fix(dst, len);
        PatchCountryStrings(dst, len);

        if (deflicker == HIIDRA_DEFLICKER_ON_LOW) {
            patch_vfilters((u8*)dst, len, vfilter_low);
            patch_vfilters_rogue((u8*)dst, len, vfilter_low);
        } else if (deflicker == HIIDRA_DEFLICKER_ON_MEDIUM) {
            patch_vfilters((u8*)dst, len, vfilter_medium);
            patch_vfilters_rogue((u8*)dst, len, vfilter_medium);
        } else if (deflicker == HIIDRA_DEFLICKER_ON_HIGH) {
            patch_vfilters((u8*)dst, len, vfilter_high);
            patch_vfilters_rogue((u8*)dst, len, vfilter_high);
        } else if (deflicker != HIIDRA_DEFLICKER_AUTO) {
            patch_vfilters((u8*)dst, len, vfilter_off);
            patch_vfilters_rogue((u8*)dst, len, vfilter_off);
            // This might break fade and brightness effects
            if (deflicker == HIIDRA_DEFLICKER_OFF_EXTENDED)
                deflicker_patch((u8*)dst, len);
        }

        DCFlushRange((u8*)dst, len);

        return ret;
}

static bool Remove_001_Protection(void *Address, int Size)
{
    static const u8 SearchPattern[] = {0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
    static const u8 PatchData[] = {0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18};
    u8 *Addr_end = (u8*)(Address + Size);
    u8 *Addr;

    for (Addr = (u8*)Address; Addr <= Addr_end - sizeof SearchPattern; Addr += 4)
        if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0)
        {
            memcpy(Addr, PatchData, sizeof PatchData);
            return true;
        }
    return false;
}

static void Anti_002_fix(void *Address, int Size)
{
    static const u8 SearchPattern[] = {0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00};
    static const u8 PatchData[] = {0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00};
    void *Addr = Address;
    void *Addr_end = Address + Size;

    while (Addr <= Addr_end - sizeof SearchPattern)
    {
        if (memcmp(Addr, SearchPattern, sizeof SearchPattern) == 0)
            memcpy(Addr, PatchData, sizeof PatchData);
        Addr += 4;
    }
}
