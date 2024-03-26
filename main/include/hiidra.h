#pragma once

#include <gccore.h>
#include <lua.hpp>
#include <stdint.h>
#include <vector>

#define HIIDRA_MAGIC        0x48445241

#define HIIDRA_CFG_VERSION  0x00000003

typedef struct HIIDRA_CFG {
    u32     Magicbytes;
    u32     Version;
    u32     Config;
    u64     TitleID;
    char    GamePath[256];
    char    CheatPath[256];
} HIIDRA_CFG;

enum hiidraconfigbitpos {
    HIIDRA_CFG_BIT_VGA          =   (0),
    HIIDRA_CFG_BIT_BT           =   (1),
    HIIDRA_CFG_BIT_GC2WIIMOTE   =   (2),
    HIIDRA_CFG_BIT_WIFI         =   (3),
    HIIDRA_CFG_BIT_CHEATS       =   (4),
    HIIDRA_CFG_BIT_USBSAVES     =   (5),
    HIIDRA_CFG_BIT_PATCHMX      =   (6),
};

enum hiidraconfig {
    HIIDRA_CFG_VGA              =   (1 << HIIDRA_CFG_BIT_VGA),
    HIIDRA_CFG_BT               =   (1 << HIIDRA_CFG_BIT_BT),
    HIIDRA_CFG_GC2WIIMOTE       =   (1 << HIIDRA_CFG_BIT_GC2WIIMOTE),
    HIIDRA_CFG_WIFI             =   (1 << HIIDRA_CFG_BIT_WIFI),
    HIIDRA_CFG_CHEATS           =   (1 << HIIDRA_CFG_BIT_CHEATS),
    HIIDRA_CFG_USBSAVES         =   (1 << HIIDRA_CFG_BIT_USBSAVES),
    HIIDRA_CFG_PATCHMX          =   (1 << HIIDRA_CFG_BIT_PATCHMX),
};

void forgeKernel(char* kernel, u32 kernelSize, const uint8_t** extraModules, u32 nExtraModules, u32 keepES, u32 keepFS);
int getKernelSize(u32* kernelSize);
int loadKernel(char* kernel, u32* kernelSize, u32* FoundVersion);
int loadIOSModules(void);
void initHiidra();
void lockHiidraLogMutex();
void unlockHiidraLogMutex();
u32 hiidraAddLogLine(const char* line, ...);
void hiidraUpdateLogLine(u32 index, const char* line, ...);
void luaRegisterHiidraLib(lua_State* L);
int bootHiidra(HIIDRA_CFG hcfg, u32 gameIDU32, std::vector<uint32_t> cheats);