#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <gccore.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include "dolbooter_bin.h"
#include "stub_bin.h"
#include "usbstorage_ogc.h"
#include "debug.h"

typedef void (*entrypoint) (void);

extern "C" {
    extern void __exception_closeall();
    extern void VIDEO_SetFramebuffer(void *);
    extern void udelay(int us);
}

#define PRIILOADER_TOMENU       0x50756E65
#define PRIILOADER_SKIPAUTO     0x4461636F
#define PRIILOADER_BOOTAUTO     0x41627261
#define PRIILOADER_MAGICADDR    ((vu32*)0x8132FFFB)

#define EXECUTE_ADDR    ((u8*)0x92000000)
#define BOOTER_ADDR     ((u8*)0x93000000)
#define ARGS_ADDR       ((u8*)0x93200000)
#define CMDL_ADDR       ((u8*)0x93200000+sizeof(struct __argv))
#define HBMAGIC_ADDR    ((u8*)0x93200000-8)

#define PATHMAX 300
#define ARGSMAX 300

#define HAVE_AHBPROT    ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
#define MEM2_PROT       0x0D8B420A
#define ES_MODULE_START (u16*)0x939F0000
#define HW_DIFLAGS      0x0D800180

static const u16 ticket_check[] = {
    0x685B,          // ldr r3,[r3,#4] ; get TMD pointer
    0x22EC, 0x0052,  // movls r2, 0x1D8
    0x189B,          // adds r3, r3, r2; add offset of access rights field in TMD
    0x681B,          // ldr r3, [r3]   ; load access rights (haxxme!)
    0x4698,          // mov r8, r3     ; store it for the DVD video bitcheck later
    0x07DB           // lsls r3, r3, #31; check AHBPROT bit
};

static bool _fatInitialized = false;

static bool patchAHBPROT() {
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
        Debug("%u %u\n", __io_custom_usbstorage.startup(), __io_custom_usbstorage.isInserted());
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

    for (int i = 0; i < 50 && !_fatInitialized; i++) {
        if (fatMountSimple("usb", &__io_custom_usbstorage)) {
            chdir("usb:/");
            _fatInitialized = true;
            return true;
        }
        udelay(100000); //100ms
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

void shutdown() {
    // Clear potential homebrew channel stub
    memset((void*)0x80001800, 0, 0x1800);

    // Copy our own stub into memory
    memcpy((void*)0x80001800, stub_bin, stub_bin_size);

    DCFlushRange((void*)0x80001800, 0x1800);

    if (_fatInitialized) {
        _fatInitialized = false;
        fatUnmount("usb:/");
        __io_custom_usbstorage.shutdown();
    }

    USB_Deinitialize();
    VIDEO_Flush();
    VIDEO_WaitVSync();
    WPAD_Shutdown();
    PAD_Reset(0xf0000000);
}

void bootDOL(const char* path, const char* args) {
    u32 level;
    size_t size;
    struct __argv arg;

    FILE* fp = fopen(path, "rb");
    if (!fp)
        return;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    fread(EXECUTE_ADDR, 1, size, fp);
    fclose(fp);
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

    reloadIOS(-1, NULL);

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable(level);
    __exception_closeall();
    hbboot_ep();
    _CPU_ISR_Restore(level);
}

void bootPriiloader() {
    shutdown();
    *PRIILOADER_MAGICADDR = PRIILOADER_SKIPAUTO;
    DCFlushRange((void*)PRIILOADER_MAGICADDR, 4);
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
