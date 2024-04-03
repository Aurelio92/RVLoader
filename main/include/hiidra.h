#pragma once

#include <gccore.h>
#include <lua.hpp>
#include <stdint.h>
#include <vector>
#include "hiidratypes.h"

void forgeKernel(char* kernel, u32 kernelSize, const uint8_t** extraModules, u32 nExtraModules, u32 keepES, u32 keepFS);
int getKernelSize(u32* kernelSize);
int loadKernel(char* kernel, u32* kernelSize, u32* FoundVersion);
int loadIOSModules(void);
void initHiidra();
void lockHiidraLogMutex();
void unlockHiidraLogMutex();
void hiidraSignalRedraw();
void hiidraWaitForRedraw();
u32 hiidraAddLogLine(const char* line, ...);
void hiidraUpdateLogLine(u32 index, const char* line, ...);
void luaRegisterHiidraLib(lua_State* L);
int bootHiidra(HIIDRA_CFG hcfg, u32 gameIDU32, std::vector<uint32_t> cheats, bool forceReinstall);