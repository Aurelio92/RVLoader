#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <aesndlib.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <ogc/usbgecko.h>
#include <ogc/exi.h>
#include <libgui.h>
#include <mxml.h>
#include <unistd.h>
#include <lua.hpp>
#include <lauxlib.h>

#include <vector>
#include <algorithm>
#include <clocale>
#include <string>
#include <dirent.h>
#include <malloc.h>
#include <cstring>
#include <sys/iosupport.h>
#include "titles.h"
#include "genpad.h"
#include "main.h"
#include "i2c.h"
#include "ave.h"
#include "system.h"
#include "pms2.h"
#include "gcplus.h"
#include "rvldd.h"
#include "mx.h"
#include "uamp.h"
#include "gpio.h"
#include "guielements.h"
#include "luasupport.h"
#include "theme.h"
#include "safemenu.h"
#include "hud.h"
#include "hiidra.h"

#include "poppins_ttf.h"

static Vector2 screenSize;
static bool VGAEnabled;

static GuiWindow mainWindow;
GuiImage* dummyCover;
GuiImage* dummyHBIcon;

Config mainConfig;

volatile u32 connectedPads;
static mutex_t SIMutex;
static bool controllersEnabled;
static bool controlledRedraw = false;
static bool hasToShutdown = false;
static lwpq_t redrawQueue;
static bool runningOnDolphin = false;

bool checkRVL();

void lockSIMutex() {
    LWP_MutexLock(SIMutex);
}

void unlockSIMutex() {
    LWP_MutexUnlock(SIMutex);
}

void enableControllers() {
    controllersEnabled = true;
}

void disableControllers() {
    controllersEnabled = false;
}

void enableControlledRedraw() {
    controlledRedraw = true;
}

void forceRedraw() {
    LWP_ThreadSignal(redrawQueue);
}

void mainWindowSwitchElement(const char* el) {
    mainWindow.switchToElement(el);
}

Vector2 getScreenSize() {
    return screenSize;
}

bool isVGAEnabled() {
    return VGAEnabled;
}

void setVGAEnabled(bool en) {
    VGAEnabled = en;
}

void shutdownCB() {
    hasToShutdown = true;
}

void WMPowerButtonCB(s32 chan) {
    shutdownCB();
}

bool isRunningOnDolphin() {
    return runningOnDolphin;
}


//rvloader.ld puts some libraries' code at the start of MEM2: MEM2's arena starts after it
extern "C" u8 __mem2_end[];
extern "C" void* __myArena2Lo = __mem2_end;

static devoptab_t timestampedStdout;
static ssize_t (*geckoWrite)(struct _reent *, void *, const char *, size_t);
static u64 bootTime;

// Prefixes every stdout line with ms since boot, to spot slow init steps.
// shortcut: lineStart is shared between threads, so concurrent prints may misplace a prefix; fine for debugging
static ssize_t timestampedWrite(struct _reent *r, void *fd, const char *ptr, size_t len) {
    static bool lineStart = true;
    for (size_t i = 0; i < len;) {
        if (lineStart) {
            char ts[16];
            int n = snprintf(ts, sizeof(ts), "[%6u] ", diff_msec(bootTime, gettime()));
            geckoWrite(r, fd, ts, n);
        }
        const char *nl = (const char *)memchr(ptr + i, '\n', len - i);
        size_t chunk = nl ? (size_t)(nl - (ptr + i)) + 1 : len - i;
        geckoWrite(r, fd, ptr + i, chunk);
        lineStart = nl != NULL;
        i += chunk;
    }
    return len;
}

// freetype only uses this for WOFF2 fonts, which RVLoader never loads.
// Stub function to avoid wasting ~170kB
extern "C" int BrotliDecoderDecompress(size_t encoded_size, const uint8_t* encoded, size_t* decoded_size, uint8_t* decoded) {
    return 0; // BROTLI_DECODER_RESULT_ERROR
}

int main(int argc, char **argv) {
    // Waits for EXI probe debouncing. Timeout after 50ms
    for (int i = 0; i < 50 && EXI_ProbeEx(EXI_CHANNEL_1) == 0; i++) usleep(10000);
    if (usb_isgeckoalive(EXI_CHANNEL_1)) {
        CON_EnableGecko(EXI_CHANNEL_1, 1);
        bootTime = gettime();
        timestampedStdout = *devoptab_list[STD_OUT];
        geckoWrite = timestampedStdout.write_r;
        timestampedStdout.write_r = timestampedWrite;
        devoptab_list[STD_OUT] = &timestampedStdout;
    }
    TPLFile mainTDF;
    float mem1Max = (float)SYS_GetArena1Size() / 1048576.0f;
    float mem2Max = (float)SYS_GetArena2Size() / 1048576.0f;

    printf("Initializing RVLoader\n");

    //Detect if we are running under dolphin
    s32 dolphinFd = IOS_Open("/dev/dolphin", 0);
    if (dolphinFd >= 0) {
        IOS_Close(dolphinFd);
        runningOnDolphin = true;
    }

    LWP_InitQueue(&redrawQueue);
    LWP_MutexInit(&SIMutex, true);
    enableControllers();
    initHiidra();

    i2c_init();
    ISFS_Initialize();
    s32 vgaFd = ISFS_Open("/title/00000001/00000002/data/vga.bin", 1);
    VGAEnabled = ((vgaFd >= 0) && VIDEO_HaveComponentCable()) || aveIsVGAModeEnabled();
    ISFS_Close(vgaFd);
    ISFS_Deinitialize();

    printf("Checking if PMS2 is connected\n");

    //PMS2 includes on-board resistors on i2c lines, allowing full open-drain control
    if (PMS2::isConnected())
        i2c_setMode(I2C_MODE_OPENDRAIN);
    else
        i2c_setMode(I2C_MODE_PUSHPULL);

    //Set environment for lua interpreter (used by 'require')
    setenv("LUA_PATH", "./?.luac;./?.lua", 1);

    PAD_Init();
    SI_DisablePolling(0xf0000000);
    WPAD_Init();
    std::setlocale(LC_ALL, "en_US.UTF-8");
    safeMenu();
    Gfx::init();
    Gfx::setClearColor(0x2D, 0x2D, 0x2D);
    AESND_Init();

    SYS_SetPowerCallback(shutdownCB);
    WPAD_SetPowerButtonCallback(WMPowerButtonCB);

    //Set internal resolution for UI
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
        //Actually 15:9 for most widescreen portable Wiis
        screenSize.x = 800;
        screenSize.y = 480;
    } else {
        screenSize.x = 640;
        screenSize.y = 480;
    }
    mainWindow.setSize(screenSize.x, screenSize.y);

    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, screenSize.x, screenSize.y);

    //Dummy drawing to clear the framebuffer
    Gfx::startDrawing();
    Gfx::endDrawing();

    dummyCover = NULL;
    dummyHBIcon = NULL;

    Font font(poppins_ttf, poppins_ttf_size, 20);

    i2c_init();
    if (VGAEnabled) {
        aveEnableVGA();
    }
    //i2c_deinit();

    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
        RVLDD::setStretch(1);
    } else {
        RVLDD::setStretch(0);
    }

    printf("Mounting USB... ");
    if (!initFAT())
        systemError("Generic error", "Could not initialize FAT. Make sure USB is plugged in");
    printf("Done\n");

    //Check if the bootloader is installed
    if (!checkRVL()) {
        FILE* blFp = fopen("/apps/RVLoader/installer.dol", "rb");
        if (blFp) {
            fclose(blFp);

            while (1) {
                PAD_ScanPads();
                WPAD_ScanPads();

                int down = PAD_ButtonsDown(0);
                int wDown = WPAD_ButtonsDown(0);

                if (down & PAD_BUTTON_A || wDown & WPAD_BUTTON_A) {
                    bootDOL("/apps/RVLoader/installer.dol", "", false);
                }

                if (down & PAD_BUTTON_B || wDown & WPAD_BUTTON_B) {
                    break;
                }

                Gfx::startDrawing();
                font.printf(32, 32, "RVLoader is not installed. Do you want to run the installer?");
                font.printf(32, 64, "A - Yes        B - No");
                Gfx::endDrawing();
            }
        }
    }

    {
        GuiImage RVLlogo(LOGO_PATH);
        const int textHeight = 32;
        int textCenterOffsetY = (textHeight + font.getCharBearingY('O')) / 2 - font.getSize();
        Vector2 pos = (screenSize - RVLlogo.getDimensions()) / 2;
        //Repeated twice to avoid buffering issues
        Gfx::startDrawing();
        GFXWindow(pos.x, pos.y, RVLlogo.getDimensions().x, RVLlogo.getDimensions().y + 2 * textHeight) {
            RVLlogo.draw();
            font.printf(0, RVLlogo.getDimensions().y + textCenterOffsetY, "RVLoader v%u.%u %s", VER_MAJOR, VER_MINOR, VER_NAME);
            font.printf(0, RVLlogo.getDimensions().y + textCenterOffsetY + textHeight, "Loading...");
        }
        Gfx::endDrawing();
        dummyCover = new GuiImage(DUMMYCOVER_PATH);
        dummyHBIcon = new GuiImage(DUMMYHBICON_PATH);
    }

    printf("Initialization complete\n");

    HUD::init();

    //Make default dir structure if missing
    mkdir("/rvloader", 777);
    mkdir("/rvloader/covers", 777);
    mkdir("/rvloader/themes", 777);
    mkdir("/rvloader/configs", 777);
    mkdir("/wmemu", 777);

    wiiTDB::parse();
    addWiiGames();
    addGCGames();
    addVCGames();
    addWiiChannels();
    addWiiHomebrews();

    //This is somehow needed to get proper reading from PMS2
    if (PMS2::isConnected()) {
        volatile u16 dummyPMS2 = PMS2::getBatDesignCapacity();
    }

    //Reset GC+2.0 mapping
    if (GCPlus::isV2()) {
        GCPlus::unlock();
        GCPlus::setDefaultMapping();
        GCPlus::lock();
    }

    //Load main config
    mainConfig.open(MAINCONFIG_PATH);
    
    //Load anim for Wii game loading
    int wiiLoad;
    if (!mainConfig.getValue("WiiLoadScreen", &wiiLoad)) {
        wiiLoad = WII_LOAD_VERBOSE;
        mainConfig.setValue("WiiLoadScreen", wiiLoad);
        mainConfig.save(MAINCONFIG_PATH);
    }
    
    //Load theme
    std::string curTheme;
    std::string themePath;
    if (!mainConfig.getValue("theme", &curTheme)) {
        curTheme = "main";
        mainConfig.setValue("theme", curTheme);
        mainConfig.save(MAINCONFIG_PATH);
    }

    lockTitlesDBs();
    themePath = "/rvloader/themes/" + curTheme;
    if (!loadTheme(themePath.c_str(), &mainWindow)) {
        //Loading selected theme failed. Try to load default one
        curTheme = "main";
        mainConfig.setValue("theme", curTheme);
        mainConfig.save(MAINCONFIG_PATH);
        themePath = "/rvloader/themes/" + curTheme;

        if (!loadTheme(themePath.c_str(), &mainWindow))
            systemError("Theme error!", "Couldn't open theme file");
    }

    //Switch to last selected element
    std::string lastView;
    if (mainConfig.getValue("LastView", &lastView)) {
        mainWindow.switchToElement(lastView);
    }
    unlockTitlesDBs();

    while(1) {
        if (controlledRedraw) {
            LWP_ThreadSleep(redrawQueue);
        }

        if (controllersEnabled) {
            lockSIMutex();
            connectedPads = PAD_ScanPads();
            unlockSIMutex();
        }

        if (hasToShutdown) {
            powerOff();
        }

        WPAD_ScanPads();
        GenPad::update(); //Must be called after *PAD_ScanPads
        int down = PAD_ButtonsDown(0);

        //mainWindow always on focus
        lockTitlesDBs();
        mainWindow.handleInputs(true);

        Gfx::startDrawing();
        mainWindow.draw(true);
        //font.printf(300, 10, "MEM1: %.3f / %.3f", (float)SYS_GetArena1Size() / 1048576.0f, mem1Max);
        //font.printf(300, 30, "MEM2: %.3f / %.3f", (float)SYS_GetArena2Size() / 1048576.0f, mem2Max);
        //font.printf(300, 10, "PADS: %u", connectedPads);
        Gfx::waitDrawDone(); //Make sure GX is done rendering all covers
        unlockTitlesDBs(); //Needs to be before endDrawing
        Gfx::endDrawing();
        //VIDEO_WaitVSync();

        hiidraSignalRedraw();
    }

    return 0;
}

bool checkRVL() {
    const char blStr[] = "RVLoader bootloader";
    u32 blStrLen = strlen(blStr);
    ISFS_Initialize();
    s32 fd = ISFS_Open("/title/00000001/00000002/data/main.bin", 1);
    if (fd < 0) {
        ISFS_Deinitialize();
        return false;
    }

    s32 size = ISFS_Seek(fd, 0, SEEK_END);
    ISFS_Seek(fd, 0, SEEK_SET);
    if (size < 0) {
        ISFS_Close(fd);
        ISFS_Deinitialize();
        return false;
    }

    if (size > 0x100000) { //Bootloader must be smaller than 1MB
        ISFS_Close(fd);
        ISFS_Deinitialize();
        return false;
    }

    char* blBuffer = (char*)memalign(0x20, (u32)size);
    if (blBuffer == NULL) {
        ISFS_Close(fd);
        ISFS_Deinitialize();
        return false;
    }

    if (ISFS_Read(fd, blBuffer, (u32)size) != size) {
        ISFS_Close(fd);
        ISFS_Deinitialize();
        free(blBuffer);
        return false;
    }

    for (u32 i = 0; i < (u32)size - blStrLen; i++) {
        if (!memcmp(&blBuffer[i], blStr, blStrLen)) {
            ISFS_Close(fd);
            ISFS_Deinitialize();
            free(blBuffer);
            return true;
        }
    }

    ISFS_Close(fd);
    ISFS_Deinitialize();
    free(blBuffer);
    return false;
}
