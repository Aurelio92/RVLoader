#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <gccore.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include "dolbooter_bin.h"
#include "stub_bin.h"
#include "usbstorage_ogc.h"
#include "nintendont.h"
#include "hiidra.h"
#include "sha1.h"
#include "i2c.h"
#include "gpio.h"
#include "main.h"
#include "virtualsysconf.h"
#include "patches.h"
#include "DOL.h"

#include "discloader_dol.h"

#define __SYS_ReadROM_checksum  0x6e9de751
#define __SYS_ReadROM_len       0x00000158

typedef void (*entrypoint) (void);

extern "C" {
    extern void __exception_closeall();
    extern void VIDEO_SetFramebuffer(void *);
    extern void udelay(int us);
}

//Priiloader stuff
#define PRIILOADER_TOMENU       0x50756E65
#define PRIILOADER_SKIPAUTO     0x4461636F
#define PRIILOADER_BOOTAUTO     0x41627261
#define PRIILOADER_MAGICADDR    ((vu32*)0x8132FFFB)

#define EXECUTE_ADDR    ((u8*)0x92000000)
#define BOOTER_ADDR     ((u8*)0x93000000)
#define ARGS_ADDR       ((u8*)0x93200000)
#define CMDL_ADDR       ((u8*)0x93200000+sizeof(struct __argv))

#define HAVE_AHBPROT    ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
#define MEM2_PROT       0x0D8B420A
#define ES_MODULE_START (u16*)0x939F0000
#define IOS_PATCH_START (u16*)0x93400000
#define IOS_PATCH_END   (u16*)0x94000000
#define HW_DIFLAGS      0x0D800180

extern const DISC_INTERFACE __io_wiisd;

static bool _fatInitialized = false;

static bool patchAHBPROT() {
    const u16 ticket_check[] = {
        0x685B,          // ldr r3,[r3,#4] ; get TMD pointer
        0x22EC, 0x0052,  // movls r2, 0x1D8
        0x189B,          // adds r3, r3, r2; add offset of access rights field in TMD
        0x681B,          // ldr r3, [r3]   ; load access rights (haxxme!)
        0x4698,          // mov r8, r3     ; store it for the DVD video bitcheck later
        0x07DB           // lsls r3, r3, #31; check AHBPROT bit
    };

    if (HAVE_AHBPROT) {
        //Disable memory protection
        write16(MEM2_PROT, 2);

        for (u16* patchme = ES_MODULE_START; patchme < ES_MODULE_START + 0x4000; ++patchme) {
            if (!memcmp(patchme, ticket_check, sizeof(ticket_check))) {
                patchme[4] = 0x23FF; // li r3, 0xFF
                DCFlushRange(patchme + 4, 2);
                return true;
            }
        }
    }

    return false;
}

static bool patchISFS(bool patch) {
    const u8 isfs_old[] = {0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66};
    const u8 isfs_patch[] = {0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66};

    if (HAVE_AHBPROT) {
        //Disable memory protection
        write16(MEM2_PROT, 2);

        if (patch) {
            for (u16* patchme = IOS_PATCH_START; patchme < IOS_PATCH_END; ++patchme) {
                if (!memcmp(patchme, isfs_old, sizeof(isfs_old))) {
                    memcpy(patchme, isfs_patch, sizeof(isfs_patch));
                    DCFlushRange(patchme, sizeof(isfs_patch));
                    return true;
                }
            }
        } else {
            for (u16* patchme = IOS_PATCH_START; patchme < IOS_PATCH_END; ++patchme) {
                if (!memcmp(patchme, isfs_patch, sizeof(isfs_patch))) {
                    memcpy(patchme, isfs_old, sizeof(isfs_old));
                    DCFlushRange(patchme, sizeof(isfs_old));
                    return true;
                }
            }
        }
    }

    return false;
}

void patchFSAccess() {
    static const unsigned char FSAccessPattern[] = {0x9B, 0x05, 0x40, 0x03, 0x99, 0x05, 0x42, 0x8B};
    static const unsigned char FSAccessPatch[] = {0x9B, 0x05, 0x40, 0x03, 0x1C, 0x0B, 0x42, 0x8B};

    //Disables MEMPROT for patches
    write16(MEM2_PROT, 0);
    //Patches FS access
    for (u16* i = (u16*)0x93A00000; i < IOS_PATCH_END; i++) {
        if (memcmp((void*)i, FSAccessPattern, sizeof(FSAccessPattern)) == 0) {
            memcpy((void*)i, FSAccessPatch, sizeof(FSAccessPatch));
            DCFlushRange((void*)i, sizeof(FSAccessPatch));
            break;
        }
    }
}

int reloadIOS(int ios, int* ahbprot) {
    if (ahbprot != NULL)
        *ahbprot = 0;

    if (ios < 0 || ios > 254) {
        ios = IOS_GetPreferredVersion();
        if (ios <= 9)
            ios = 58;
    }

    if (patchAHBPROT()) {
        IOS_ReloadIOS(ios);

        if (HAVE_AHBPROT) {
            if (ahbprot != NULL)
                *ahbprot = 1;

            //Disable memory protection
            write16(MEM2_PROT, 2);
            mask32(HW_DIFLAGS, 0x200000, 0);
        }

        return ios;
    }

    IOS_ReloadIOS(ios);
    return ios;
}

bool initFAT() {
    if (_fatInitialized)
        return true;

    bool usbInserted = false;

    /*for (int i = 0; i < 50 && !usbInserted; i++) {
        printf("%u %u\n", __io_custom_usbstorage.startup(), __io_custom_usbstorage.isInserted());
        if (__io_custom_usbstorage.startup() && __io_custom_usbstorage.isInserted()) {
            usbInserted = true;
        }
        udelay(100000); //100ms
    }

    if (!usbInserted)
        return false;

    if (fatMount("usb", &__io_custom_usbstorage, 0, 32, 64)) {
        chdir("usb:/");
        _fatInitialized = true;
        return true;
    }*/

    if (isRunningOnDolphin()) {
        //Try SD mounting
        if (!_fatInitialized) {
            if (fatMountSimple("sd", &__io_wiisd)) {
                printf("Falling back to SD\n");
                chdir("sd:/");
                _fatInitialized = true;
                return true;
            }
        }
    } else {
        for (int i = 0; i < 50 && !_fatInitialized; i++) {
            printf("Trying to mount FAT\n");
            if (fatMountSimple("usb", &__io_custom_usbstorage)) {
                printf("Success\n");
                chdir("usb:/");
                _fatInitialized = true;
                return true;
            }
            printf("Wait\n");
            udelay(100000); //100ms
            printf("Failed\n");
        }
    }

    //Wait up to 2 seconds
    /*for (int i = 0; i < 20 && !_fatInitialized; i++) {
        if (__io_custom_usbstorage.startup()) { //Try USB
            if (__io_custom_usbstorage.isInserted()) {
                if (fatMountSimple("usb", &__io_custom_usbstorage)) {
                    chdir("usb:/");
                    _fatInitialized = true;
                    return true;
                }
            }
        }
        udelay(100000); //100ms
    }*/
    return false;
}

void unmountFAT() {
    if (_fatInitialized) {
        _fatInitialized = false;
        fatUnmount("usb:/");
        __io_custom_usbstorage.shutdown();
    }

    USB_Deinitialize();
}

void shutdown() {
    // Clear potential homebrew channel stub
    memset((void*)0x80001800, 0, 0x1800);

    // Copy our own stub into memory
    memcpy((void*)0x80001800, stub_bin, stub_bin_size);

    DCFlushRange((void*)0x80001800, 0x1800);

    unmountFAT();

    VIDEO_Flush();
    VIDEO_WaitVSync();
    WPAD_Shutdown();
    PAD_Reset(0xf0000000);
}

void bootDOL(const char* path, const char* args, bool patchMX) {
    u32 level;
    size_t size;
    struct __argv arg;
    std::vector<DOLFunction> DOLFunctions;

    i2c_deinit();

    FILE* fp = fopen(path, "rb");
    if (!fp)
        return;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    fread(EXECUTE_ADDR, 1, size, fp);
    fclose(fp);
    listFunctionsInDOL((dolhdr*)EXECUTE_ADDR, DOLFunctions);
    for (auto fun : DOLFunctions) {
        if (patchMX) {
            if (fun.checksum == __SYS_ReadROM_checksum &&
                fun.len == __SYS_ReadROM_len) {
                printf("Patching __SYS_ReadROM at %08X\n", fun.offset);
                injectMXPatch(&EXECUTE_ADDR[fun.offset]);
            }
        }
    }
    DCFlushRange(EXECUTE_ADDR, size);

    memset(&arg, 0, sizeof(arg));
    if (args != NULL) {
        arg.argvMagic = ARGV_MAGIC;
        arg.length = strlen(path) + strlen(args) + 2; //1 extra for the separator between the path and the other args + line ending
        arg.commandLine = (char*)CMDL_ADDR;
        sprintf(arg.commandLine, "%s;%s", path, args);
        int len = strlen(arg.commandLine);
        for (int i = 0; i < len; i++) {
            if (arg.commandLine[i] == ';')
                arg.commandLine[i] = '\0';
        }

        DCFlushRange(arg.commandLine, len + 1);
    }

    memmove(ARGS_ADDR, &arg, sizeof(arg));
    DCFlushRange(ARGS_ADDR, sizeof(arg));

    memcpy(BOOTER_ADDR, dolbooter_bin, dolbooter_bin_size);
    DCFlushRange(BOOTER_ADDR, dolbooter_bin_size);
    ICInvalidateRange(BOOTER_ADDR, dolbooter_bin_size);

    entrypoint hbboot_ep = (entrypoint)BOOTER_ADDR;

    shutdown();

    u32 prevOwners = read32(HW_GPIO_OWNER_ADDR);

    reloadIOS(-1, NULL);
    //Lock i2c to Starlet so homebrew's VIDEO_Init won't be able to disable VGA
    if (isVGAEnabled() || !(prevOwners & GPIO_I2C)) {
        u32 owners = read32(HW_GPIO_OWNER_ADDR);
        owners = owners & ~GPIO_I2C;
        write32(HW_GPIO_OWNER_ADDR, owners);
    }

    write32(HW_GPIO_ENABLE_ADDR, read32(HW_GPIO_ENABLE_ADDR) | GPIO_DEBUG);
    write32(HW_GPIO_DIR_ADDR, read32(HW_GPIO_DIR_ADDR) & ~GPIO_DEBUG); //Set all the GPIOs as input
    write32(HW_GPIO_OWNER_ADDR, read32(HW_GPIO_OWNER_ADDR) & ~GPIO_DEBUG);

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable(level);
    __exception_closeall();
    hbboot_ep();
    _CPU_ISR_Restore(level);
}

void bootGCGame(NIN_CFG cfg) {
    static char ninPath[256] = "/apps/nintendont/boot.dol";
    u32 level;
    size_t size;
    struct __argv arg;

    i2c_deinit();

    FILE* fp = fopen(ninPath, "rb");
    if (!fp)
        return;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    fread(EXECUTE_ADDR, 1, size, fp);
    fclose(fp);
    DCFlushRange(EXECUTE_ADDR, size);

    memset(&arg, 0, sizeof(arg));
    arg.argvMagic = ARGV_MAGIC;
    arg.argc = 2;
    arg.length = strlen(ninPath) + 1 + sizeof(NIN_CFG);
    arg.commandLine = (char*)CMDL_ADDR;
    sprintf(arg.commandLine, "%s", ninPath);
    memcpy(&arg.commandLine[strlen(ninPath) + 1], &cfg, sizeof(NIN_CFG));
    arg.argv = &arg.commandLine;
    arg.endARGV = arg.argv + 1;
    DCFlushRange(arg.commandLine, arg.length);

    memmove(ARGS_ADDR, &arg, sizeof(arg));
    DCFlushRange(ARGS_ADDR, sizeof(arg));

    memcpy(BOOTER_ADDR, dolbooter_bin, dolbooter_bin_size);
    DCFlushRange(BOOTER_ADDR, dolbooter_bin_size);
    ICInvalidateRange(BOOTER_ADDR, dolbooter_bin_size);

    entrypoint hbboot_ep = (entrypoint)BOOTER_ADDR;

    shutdown();

    reloadIOS(-1, NULL);
    //Lock i2c to Starlet so homebrew's VIDEO_Init won't be able to disable VGA
    if (isVGAEnabled()) {
        u32 owners = read32(HW_GPIO_OWNER_ADDR);
        owners = owners & ~GPIO_I2C;
        write32(HW_GPIO_OWNER_ADDR, owners);
    }

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable(level);
    __exception_closeall();
    hbboot_ep();
    _CPU_ISR_Restore(level);
}

void bootWiiGame(HIIDRA_CFG cfg, u32 gameIDU32, std::string gameIDString, std::vector<uint32_t> cheats, bool forceReinstall) {
    static const char __conf_file[] ATTRIBUTE_ALIGN(32) = "/shared2/sys/SYSCONF";
    static const char __conf_txt_file[] ATTRIBUTE_ALIGN(32) = "/title/00000001/00000002/data/setting.txt";
    static u8 __conf_buffer[0x4000] ATTRIBUTE_ALIGN(32);
    static u8 __conf_txt_buffer[0x101] ATTRIBUTE_ALIGN(32);

    //Copy SYSCONF file
    int fd = IOS_Open(__conf_file, 1);
    if (fd >= 0) {
        FILE* fp = fopen("/rvloader/Hiidra/SYSCONF", "wb");

        if (fp != NULL) {
            memset(__conf_buffer, 0, 0x4000);
            IOS_Read(fd, __conf_buffer, 0x4000);
            SYSCONF::setConfBuffer(__conf_buffer);
            SYSCONF::setRegion(gameIDU32);
            if ((cfg.Config & HIIDRA_CFG_GC2WIIMOTE) && !(cfg.Config & HIIDRA_CFG_BT)) {
                SYSCONF::injectGC2Wiimote();
            }
            fwrite(__conf_buffer, 1, 0x4000, fp);
            fclose(fp);
        }
        IOS_Close(fd);
    }

    //Copy settings.txt file
    fd = IOS_Open(__conf_txt_file, 1);
    if (fd >= 0) {
        FILE* fp = fopen("/rvloader/Hiidra/setting.txt", "wb");

        if (fp != NULL) {
            memset(__conf_txt_buffer, 0, 0x101);
            IOS_Read(fd, __conf_txt_buffer, 0x100);
            SYSCONF::setConfTxtBuffer(__conf_txt_buffer);
            SYSCONF::decencTextBuffer();
            SYSCONF::setRegionSetting(gameIDU32);
            SYSCONF::decencTextBuffer();
            fwrite(__conf_txt_buffer, 1, 0x100, fp);
            fclose(fp);
        }
        IOS_Close(fd);
    }

    i2c_deinit();

    bootHiidra(cfg, gameIDU32, gameIDString, cheats, forceReinstall);
}

void bootDiscLoader(bool hideLog) {
    u32 level;

    struct __argv arg;
    memset(&arg, 0, sizeof(arg));
    arg.argvMagic = ARGV_MAGIC;
    arg.length = 3; // \0 + '0' or '1' + \0
    arg.commandLine = (char*)CMDL_ADDR;
    arg.commandLine[0] = '\0';
    if(hideLog) {
        arg.commandLine[1] = '1';
    } else {
        arg.commandLine[1] = '0';
    }
    arg.commandLine[2] = '\0';
    arg.argv = &arg.commandLine;
    arg.endARGV = arg.argv + 1;
    DCFlushRange(arg.commandLine, arg.length);

    memmove(ARGS_ADDR, &arg, sizeof(arg));
    DCFlushRange(ARGS_ADDR, sizeof(arg));

    memcpy(EXECUTE_ADDR, discloader_dol, discloader_dol_size);
    DCFlushRange(EXECUTE_ADDR, discloader_dol_size);

    memcpy(BOOTER_ADDR, dolbooter_bin, dolbooter_bin_size);
    DCFlushRange(BOOTER_ADDR, dolbooter_bin_size);
    ICInvalidateRange(BOOTER_ADDR, dolbooter_bin_size);

    entrypoint hbboot_ep = (entrypoint)BOOTER_ADDR;

    //Removed due to Hiidra having control of the whole system already
    //SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable(level);
    __exception_closeall();
    hbboot_ep();
    _CPU_ISR_Restore(level);
}

void powerOff() {
    shutdown();
    SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

void bootPriiloader() {
    shutdown();
    *PRIILOADER_MAGICADDR = PRIILOADER_SKIPAUTO;
    DCFlushRange((void*)PRIILOADER_MAGICADDR, 4);
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void bootSysMenu() {
    shutdown();
    *PRIILOADER_MAGICADDR = PRIILOADER_TOMENU;
    DCFlushRange((void*)PRIILOADER_MAGICADDR, 4);
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void reboot() {
    shutdown();
    *PRIILOADER_MAGICADDR = PRIILOADER_BOOTAUTO;
    DCFlushRange((void*)PRIILOADER_MAGICADDR, 4);
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void systemError(const char* errorType, const char* error, ...) {
    char errorString[256];
    va_list args;

    va_start(args, error);
    vsprintf(errorString, error, args);
    va_end(args);

    //Taken from libogc
    static void *exception_xfb = (void*)0xC1700000;         //we use a static address above ArenaHi.
    GX_AbortFrame();
    VIDEO_SetFramebuffer(exception_xfb);
    CON_Init(exception_xfb,20,20,640,574,1280);
    CON_EnableGecko(1, true);

    kprintf("\n\n\n\tSystem error!\n\n");
    kprintf("\t%s\n", errorType);
    kprintf("\t%s\n", errorString);

    kprintf("\n\nReloading in 5 seconds\n");


    for (int i = 0; i < 250; i++)
        udelay(20000);

    reboot();
}
