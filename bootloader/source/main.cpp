#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_watchdog.h>
#include <fat.h>
#include <unistd.h>
#include "i2c.h"
#include "gpio.h"
#include "system.h"

#define AVE_ADDR (0x70 << 1) //0xE0
#define LM49450_ADDR (0x7D << 1) //0xFA


static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void failedBoot();

int main(int argc, char **argv) {
    u8 i2c_error;
    u8 VGAEnabled;

    i2c_init();
    ISFS_Initialize();
    s32 vgaFd = ISFS_Open("/title/00000001/00000002/data/vga.bin", 1);
    VGAEnabled = ((vgaFd >= 0) && VIDEO_HaveComponentCable()) || ((i2c_read8(AVE_ADDR, 0x01, &i2c_error) == 0x23));
    ISFS_Close(vgaFd);
    ISFS_Deinitialize();
    //i2c_deinit();

    // Initialise the video system
    VIDEO_Init(); //Two in a row because reading from the AVE fucks it up
    VIDEO_Init();
    // This function initialises the attached controllers
    PAD_Init();
    WPAD_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    rmode = VIDEO_GetPreferredMode(NULL);

    // Allocate memory for the display in the uncached region
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

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

    printf("\x1b[2;0H");

    VIDEO_WaitVSync();
    VIDEO_WaitVSync();

    i2c_init();
    //VIDEO_Init disables VGA, so we need to enable it again
    if (VGAEnabled) {
        i2c_write8(AVE_ADDR, 0x01, 0x23, &i2c_error);
        i2c_write8(AVE_ADDR, 0x0A, 0x00, &i2c_error);
        i2c_write8(AVE_ADDR, 0x62, 0x01, &i2c_error);
        i2c_write8(AVE_ADDR, 0x6E, 0x01, &i2c_error);
        i2c_write8(AVE_ADDR, 0x65, 0x03, &i2c_error);
    }
    //i2c_write16(AVE_ADDR, 0x71, 0x8e8e, &i2c_error);
    //i2c_deinit();

    printf("RVLoader bootloader %02X\n\n", VGAEnabled);

    if (!initFAT()) {
        printf("Failed to initialize FAT FS! Is a USB drive plugged in?\n");
        failedBoot();
    }

    //Try to boot RVLoader
    printf("Loading...\n");
    bootDOL("/apps/RVLoader/boot.dol", NULL);
    printf("Error while booting /apps/RVLoader/boot.dol\n");
    failedBoot();

    return 0;
}

void failedBoot() {
    printf("\n");
    printf("Press A to boot into priiloader\n");
    while (1) {
        PAD_ScanPads();
        WPAD_ScanPads();

        int down = PAD_ButtonsDown(0);
        if (down & PAD_BUTTON_A)
            bootPriiloader();

        VIDEO_WaitVSync();
    }
}
