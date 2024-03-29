#pragma once

#include "nintendont.h"
#include "hiidra.h"

void patchFSAccess();
int reloadIOS(int ios, int* ahbprot);
bool initFAT();
void unmountFAT();
void shutdown();
void bootDOL(const char* path, const char* args, bool patchMX);
void bootGCGame(NIN_CFG cfg);
void bootWiiGame(HIIDRA_CFG cfg, u32 gameIDU32, std::vector<uint32_t> cheats, bool forceReinstall);
void bootDiscLoader();
void bootPriiloader();
void bootSysMenu();
void powerOff();
void reboot();
void systemError(const char* errorType, const char* error, ...);
