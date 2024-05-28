#include <stdio.h>
#include <gccore.h>
#include <malloc.h>

#include "disc.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
bool hideLines = false;

int main(int argc, char **argv) {
    struct discHdr *header;

    // Initialise the video system
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
    if (usb_isgeckoalive(EXI_CHANNEL_1)) {
        CON_EnableGecko(EXI_CHANNEL_1, 1);
    }
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    printf("\x1b[2;0H");
    if(argv[1][0] == '1'){
        hideLines = true;
        Disc_Init();
        header = (struct discHdr *)memalign(32, sizeof(struct discHdr));
        while (Disc_Open() < 0);
        Disc_IsWii();
        Disc_ReadHeader(header);
        Disc_WiiBoot(0);
    }
    else{
        printf("Disc_Init() returned: %d\n", Disc_Init());
        header = (struct discHdr *)memalign(32, sizeof(struct discHdr));
        while (Disc_Open() < 0);
        //printf("Disc_Open() returned: %d\n", Disc_Open());
        printf("Disc_IsWii() returned: %d\n", Disc_IsWii());
        printf("Disc_ReadHeader() returned: %d\n", Disc_ReadHeader(header));
        printf("Disc_WiiBoot() returned: %d\n", Disc_WiiBoot(0));
    }
    
    while(1) {
        VIDEO_WaitVSync();
    }

    return 0;
}
