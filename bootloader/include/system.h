#pragma once

int reloadIOS(int ios, int* ahbprot);
bool initFAT();
void bootDOL(const char* path, const char* args);
void bootPriiloader();
void bootSysMenu();
void systemError(const char* errorType, const char* error, ...);
