#pragma once
#include <libgui.h>
#include "config.h"

#define MAINCONFIG_PATH "/rvloader/config.cfg"

extern GuiImage* AIcon;
extern GuiImage* BIcon;
extern GuiImage* XIcon;
extern GuiImage* YIcon;
extern GuiImage* SIcon;
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
