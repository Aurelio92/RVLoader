#pragma once

#include <gccore.h>

#define HIIDRA_MAGIC        0x48445241

#define HIIDRA_CFG_VERSION  0x00000002

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
};

enum hiidraconfig {
    HIIDRA_CFG_VGA              =   (1 << HIIDRA_CFG_BIT_VGA),
    HIIDRA_CFG_BT               =   (1 << HIIDRA_CFG_BIT_BT),
    HIIDRA_CFG_GC2WIIMOTE       =   (1 << HIIDRA_CFG_BIT_GC2WIIMOTE),
    HIIDRA_CFG_WIFI             =   (1 << HIIDRA_CFG_BIT_WIFI),
    HIIDRA_CFG_CHEATS           =   (1 << HIIDRA_CFG_BIT_CHEATS),
    HIIDRA_CFG_USBSAVES         =   (1 << HIIDRA_CFG_BIT_USBSAVES),
};

void forgeKernel(char* kernel, u32 kernelSize, const uint8_t** extraModules, u32 nExtraModules, u32 keepES, u32 keepFS);
int getKernelSize(u32* kernelSize);
int loadKernel(char* kernel, u32* kernelSize, u32* FoundVersion);
int loadIOSModules(void);
int bootHiidra(HIIDRA_CFG hcfg, u32 gameIDU32);