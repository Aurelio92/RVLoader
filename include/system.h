#pragma once

#include "nintendont.h"
#include "hiidra.h"

int reloadIOS(int ios, int* ahbprot);
bool initFAT();
void bootDOL(const char* path, const char* args);
void bootGCGame(NIN_CFG cfg);
void bootWiiGame(HIIDRA_CFG cfg, u32 gameIDU32);
void bootPriiloader();
void bootSysMenu();
void powerOff();
void reboot();
void systemError(const char* errorType, const char* error, ...);
