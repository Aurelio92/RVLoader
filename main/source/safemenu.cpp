#include <stdio.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>

#include "main.h"
#include "system.h"
#include "i2c.h"
#include "ave.h"
#include "gcplus.h"

#define PAD_DELAY 300 //ms

void safeMenu() {
    u8 i2c_error;
    static void *xfb = NULL;
    GXRModeObj *rmode = NULL;

    enum {
        BOOT_PRIILOADER = 0,
        RESET_STICKS,
        SYSMENU,
        INSTALLER,
        REBOOT,
        NMENUS
    } menus;

    int curMenu = 0;

    VIDEO_Init(); //Two in a row because reading from the AVE fucks it up
    //VIDEO_Init();

    u64 now = gettime();
    while (ticks_to_millisecs(gettime() - now) < PAD_DELAY) {
        if (PAD_ScanPads() & 1)
            break; //Exit loop if a pad has been found
        VIDEO_WaitVSync();
    }
    PAD_ScanPads();

    int held = PAD_ButtonsHeld(0);

    if (!((held & PAD_TRIGGER_L) && (held & PAD_TRIGGER_R) && (held & PAD_TRIGGER_Z)))
        return;

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    rmode = VIDEO_GetPreferredMode(NULL);

    // Allocate memory for the display in the uncached region
    void* tempXfb = SYS_AllocateFramebuffer(rmode);
    xfb = MEM_K0_TO_K1(tempXfb);

    // Initialise the console, required for printf
    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    // Set up the video registers with the chosen mode
    VIDEO_Configure(rmode);

    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);

    // Make the display visible
    VIDEO_SetBlack(FALSE);

    // Flush the video register changes to the hardware
    VIDEO_Flush();

    // Wait for Video setup to complete
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    //VIDEO_Init disables VGA, so we need to enable it again
    i2c_init();
    if (isVGAEnabled() || (held & PAD_BUTTON_B)) {
        setVGAEnabled(true);
        aveEnableVGA();
    }
    i2c_deinit();

    printf("\x1b[2;0HRVL safe menu\n\nSelect your option:\n");
    printf("> Boot priiloader\n");
    printf("- Reset GC+ sticks\n");
    printf("- Exit to System Menu\n");
    printf("- Run installer\n");
    printf("- Reboot\n");

    while (1) {
        PAD_ScanPads();
        int down = PAD_ButtonsDown(0);
        if (down & PAD_BUTTON_DOWN) {
            printf("\x1b[%d;0H-\n", 5 + curMenu);
            curMenu++;
        }
        if (down & PAD_BUTTON_UP) {
            printf("\x1b[%d;0H-\n", 5 + curMenu);
            curMenu--;
        }
        if (curMenu < 0)
            curMenu = 0;
        if (curMenu >= NMENUS)
            curMenu = NMENUS - 1;

        printf("\x1b[%d;0H>\n", 5 + curMenu);

        if (down & PAD_BUTTON_A) {
            switch (curMenu) {
                case BOOT_PRIILOADER:
                    bootPriiloader();
                break;

                case RESET_STICKS: {
                    u64 now;
                    GCPlus::unlock();
                    u8 tempConfig[13] = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x02, 0x03, 0x00, 0x01, 0x00};
                    GCPlus::writeEEPROM(0x06, tempConfig, 13);
                    GCPlus::reset();
                    now = gettime();
                    while (ticks_to_millisecs(gettime() - now) < 100);
                    printf("\x1b[%d;0H> Reset GC+ sticks  Done\n", 5 + curMenu);
                    now = gettime();
                    while (ticks_to_millisecs(gettime() - now) < 1000);
                    printf("\x1b[%d;0H> Reset GC+ sticks      \n", 5 + curMenu);
                } break;

                case SYSMENU:
                    bootSysMenu();
                break;

                case INSTALLER:
                    if (!initFAT())
                        systemError("Generic error", "Could not initialize FAT. Make sure USB is plugged in");
                    bootDOL("/apps/RVLoader/installer.dol", "", false);
                break;

                case REBOOT:
                    reboot();
                break;

                default:

                break;
            }
        }


        VIDEO_WaitVSync();
    }
}
