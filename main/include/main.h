#pragma once
#include <libgui.h>
#include "config.h"

#define MAINCONFIG_PATH "/rvloader/config.cfg"
#define LOGO_PATH "/rvloader/assets/logo.png"
#define DUMMYCOVER_PATH "/rvloader/assets/dummyCover.png"
#define DUMMYHBICON_PATH "/rvloader/assets/dummyHBIcon.png"

extern GuiImage* dummyCover;
extern GuiImage* dummyHBIcon;
extern Config mainConfig;
extern volatile u32 connectedPads;

void lockSIMutex();
void unlockSIMutex();
void enableControllers();
void disableControllers();
void enableControlledRedraw();
void forceRedraw();
void mainWindowSwitchElement(const char* el);
Vector2 getScreenSize();
bool isVGAEnabled();
void setVGAEnabled(bool en);
bool isRunningOnDolphin();
