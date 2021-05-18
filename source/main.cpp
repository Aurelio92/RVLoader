#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <libgui.h>
#include <mxml.h>
#include <lua.hpp>
#include <vector>
#include <algorithm>
#include <clocale>
#include <string>
#include <dirent.h>
#include <malloc.h>
#include "titles.h"
#include "genpad.h"
#include "main.h"
#include "i2c.h"
#include "debug.h"
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

#include "textures.h"
#include "textures_tpl.h"
#include "notosans_ttf.h"

#define HUD_CONFIG_PATH "/rvloader/hud.conf"

static Vector2 screenSize;
static bool VGAEnabled;

GuiImage* AIcon;
GuiImage* BIcon;
GuiImage* XIcon;
GuiImage* YIcon;
GuiImage* SIcon;
GuiImage* dummyCover;
GuiImage* dummyHBIcon;

Config mainConfig;

volatile u32 connectedPads;
static mutex_t SIMutex;

extern "C" {
    extern void udelay(int us);
};

u64 bootTime;

extern "C" {
    void __SYS_PreInit() {
        bootTime = gettime();
    }
}

bool checkRVL();

void lockSIMutex() {
    LWP_MutexLock(SIMutex);
}

void unlockSIMutex() {
    LWP_MutexUnlock(SIMutex);
}

Vector2 getScreenSize() {
    return screenSize;
}

bool isVGAEnabled() {
    return VGAEnabled;
}

void shutdownCB() {
    powerOff();
}

void WMPowerButtonCB(s32 chan) {
    shutdownCB();
}

int main(int argc, char **argv) {
    if (!MX::isConnected())
        settime(bootTime);

    TPLFile mainTDF;
    GuiWindow mainWindow;
    float mem1Max = (float)SYS_GetArena1Size() / 1048576.0f;
    float mem2Max = (float)SYS_GetArena2Size() / 1048576.0f;

    LWP_MutexInit(&SIMutex, true);

    i2c_init();
    ISFS_Initialize();
    s32 vgaFd = ISFS_Open("/title/00000001/00000002/data/vga.bin", 1);
    VGAEnabled = ((vgaFd >= 0) && VIDEO_HaveComponentCable()) || aveIsVGAModeEnabled();
    ISFS_Close(vgaFd);
    ISFS_Deinitialize();

    //PMS2 includes on-board resistors on i2c lines, allowing full open-drain control
    if (PMS2::isConnected())
        i2c_setMode(I2C_MODE_OPENDRAIN);
    else
        i2c_setMode(I2C_MODE_PUSHPULL);

    DebugStart();

    PAD_Init();
    SI_DisablePolling(0xf0000000);
    WPAD_Init();
    std::setlocale(LC_ALL, "en_US.UTF-8");
    safeMenu();
    Gfx::init();
    Gfx::setClearColor(0x2D, 0x2D, 0x2D);

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

    TPL_OpenTPLFromMemory(&mainTDF, (void*)textures_tpl, textures_tpl_size);
    AIcon = new GuiImage(&mainTDF, TEXID_button_A);
    BIcon = new GuiImage(&mainTDF, TEXID_button_B);
    XIcon = new GuiImage(&mainTDF, TEXID_button_X);
    YIcon = new GuiImage(&mainTDF, TEXID_button_Y);
    SIcon = new GuiImage(&mainTDF, TEXID_button_S);
    dummyCover = new GuiImage(&mainTDF, TEXID_dummyCover);
    dummyHBIcon = new GuiImage(&mainTDF, TEXID_dummyHBIcon);
    GuiImage RVLlogo(&mainTDF, TEXID_logo);

    Font font(notosans_ttf, notosans_ttf_size, 20);

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

    Debug("Mounting USB... ");
    if (!initFAT())
        systemError("Generic error", "Could not initialize FAT. Make sure USB is plugged in");
    Debug("Done\n");

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
                    bootDOL("/apps/RVLoader/installer.dol", "");
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
        const int textHeight = 32;
        int textCenterOffsetY = (textHeight + font.getCharBearingY('O')) / 2 - font.getSize();
        Vector2 pos = (screenSize - RVLlogo.getDimensions()) / 2;
        //Repeated twice to avoid buffering issues
        Gfx::startDrawing();
        GFXWindow(pos.x, pos.y, RVLlogo.getDimensions().x, RVLlogo.getDimensions().y + 2 * textHeight) {
            RVLlogo.draw();
            font.printf(0, RVLlogo.getDimensions().y + textCenterOffsetY, "RVLoader v%u.%u", VER_MAJOR, VER_MINOR);
            font.printf(0, RVLlogo.getDimensions().y + textCenterOffsetY + textHeight, "Loading...");
        }
        Gfx::endDrawing();
    }
    Debug("Initialization complete\n");

    //Set volume
    u8 hudVolume[2] = {20, 10};
    FILE* hudFp = fopen(HUD_CONFIG_PATH, "rb");
    if (hudFp) {
        fread(hudVolume, 1, 2, hudFp);
        fclose(hudFp);
    } else {
        hudFp = fopen(HUD_CONFIG_PATH, "wb");
        if (hudFp) {
            fwrite(hudVolume, 1, 2, hudFp);
            fclose(hudFp);
        }
    }

    if (UAMP::isConnected()) {
        write32(HW_GPIO_DIR_ADDR, read32(HW_GPIO_DIR_ADDR) & (~GPIO_DEBUG2));
        int headphonesIn = read32(HW_GPIOB_IN_ADDR) & GPIO_DEBUG2;

        UAMP::init();
        UAMP::setMute((headphonesIn && !hudVolume[0]) || (!headphonesIn && !hudVolume[1]));
        if (hudVolume[0] > 0)
            UAMP::setHeadphonesVolume(hudVolume[0] - 1);
        else
            UAMP::setHeadphonesVolume(0);
        if (hudVolume[1] > 0)
            UAMP::setSpeakersVolume(hudVolume[1] - 1);
        else
            UAMP::setSpeakersVolume(0);
    }

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
    //Load theme
    std::string curTheme;
    std::string themePath;
    if (!mainConfig.getValue("theme", &curTheme)) {
        curTheme = "main";
        mainConfig.setValue("theme", curTheme);
        mainConfig.save(MAINCONFIG_PATH);
    }

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

    while(1) {
        lockSIMutex();
        connectedPads = PAD_ScanPads();
        unlockSIMutex();

        WPAD_ScanPads();
        GenPad::update(); //Must be called after *PAD_ScanPads
        int down = PAD_ButtonsDown(0);

        //mainWindow always on focus
        mainWindow.handleInputs(true);

        Gfx::startDrawing();
        mainWindow.draw(true);
        //font.printf(300, 10, "MEM1: %.3f / %.3f", (float)SYS_GetArena1Size() / 1048576.0f, mem1Max);
        //font.printf(300, 30, "MEM2: %.3f / %.3f", (float)SYS_GetArena2Size() / 1048576.0f, mem2Max);
        //font.printf(300, 10, "PADS: %u", connectedPads);
        Gfx::endDrawing();
        //VIDEO_WaitVSync();
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
