#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <vector>

#include "apploader.h"
#include "wdvd.h"
#include "disc.h"

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

typedef struct _SPatchCfg
{
    bool cheat;
    u8 vidMode;
    GXRModeObj *vmode;
    bool vipatch;
    bool countryString;
    u8 patchVidModes;
    u8 patchDiscCheck;
} SPatchCfg;

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(app_init* report, app_main* main, app_final* final);

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

static void readCheats();
static bool maindolpatches(void *dst, int len);
static bool Remove_001_Protection(void *Address, int Size);
static void Anti_002_fix(void *Address, int Size);

static void __noprint(const char *fmt, ...)
{
}

s32 Apploader_Run(entry_point* entry) {

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

    while (appldr_main(&dst, &len, &offset)) {
        /* Read data from DVD */
        ret = WDVD_Read(dst, len, (u64)(offset << 2));

        maindolpatches(dst, len);
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

static void patch_SISetXY(void* buffer, u32 len) {
    u32 i;
    u32 SISetXYPattern[23] = {
        0x9421fff0, //stwu    r1,-16(r1)
        0x7c0802a6, //mflr    r0
        0x90010014, //stw     r0,20(r1)
        0x5480402e, //rlwinm  r0,r4,8,0,23
        0x93e1000c, //stw     r31,12(r1)
        0x547f801e, //rlwinm  r31,r3,16,0,15
        0x7fff0378, //or      r31,r31,r0
        0x00000000, //bl      <OSDisableInterrupts>


        0x3ca00000, //lis     r5,0          Patch with: addi    r5,0,0x00f0
        0x3c80cd00, //lis     r4,0xCD00     Patch with: lis     r4,0xCD00
        0x38a50000, //addi    r5,r5,0       Patch with: nop
        0x80050004, //lwz     r0,4(r5)      Patch with: nop
        0x5400060a, //rlwinm  r0,r0,0,24,5  Patch with: nop
        0x7c1ffb78, //or      r31,r0,r31    Patch with: or      r31,r5,r31
        0x93e50004, //stw     r31,4(r5)     Patch with: nop

        0x93e46430, //stw     r31,0x6430(r4)
        0x00000000, //bl      <OSRestoreInterrupts>
        0x7fe3fb78, //mr      r3,r31
        0x83e1000c, //lwz     r31,12(r1)
        0x80010014, //lwz     r0,20(r1)
        0x7c0803a6, //mtlr    r0
        0x38210010, //addi    r1,r1,16
        0x4e800020, //blr
    };

    u32 SISetXYMask[23] = {
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0x00000000,
        0xffff0000,
        0xffffffff,
        0xffff0000,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0x00000000,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
    };

    //Offset 32
    u32 SISetXYPatch[7] = {
        0x38a000f0, //addi    r5,0,0x00f0
        0x3c80cd00, //lis     r4,0xCD00
        0x60000000, //nop
        0x60000000, //nop
        0x60000000, //nop
        0x7cbffb78, //or      r31,r5,r31
        0x60000000, //nop
    };

    for(i=0; i < len; i+=4) {
        int j;
        int found = 1;
        for (j = 0; j < 23; j++) {
            if (i + j * 4 >= len) {
                found = 0;
                break;
            }
            if ((*(vu32*)(buffer+i+j*4) & SISetXYMask[j] ) != SISetXYPattern[j]) {
                found = 0;
                break;
            }
        }
        if (found) {
            for (j = 0; j < 7; j++) {
                *(vu32*)(buffer+i+32+j*4) = SISetXYPatch[j];
            }
        }
    }
}

static bool maindolpatches(void *dst, int len) {
        bool ret = false;
        u32 i;

        ICInvalidateRange(dst, len);

        //dogamehooks(1, dst, len);

        Remove_001_Protection(dst, len);
        Anti_002_fix(dst, len);
        PatchCountryStrings(dst, len);
        //patch_SISetXY(dst, len);

        DCFlushRange(dst, len);

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
