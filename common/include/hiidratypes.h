#pragma once

#include <gccore.h>

#define HIIDRA_MAGIC        0x48445241

#define HIIDRA_CFG_VERSION  0x00000005

typedef struct HIIDRA_CFG {
    u32     Magicbytes;
    u32     Version;
    u32     Config;
    u64     TitleID;
    u32     PADReadMode;
    u32     Hooktype;
    u32     DeflickerMode;
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

enum hiidrapadreadmode {
    HIIDRA_PADREAD_AUTO = 0,
    HIIDRA_PADREAD_BYPASS,
    HIIDRA_PADREAD_REDIRECT
};

enum hiidrahooktype {
    HIIDRA_HOOKTYPE_VBI = 1,
    HIIDRA_HOOKTYPE_KPADRead,
    HIIDRA_HOOKTYPE_Joypad,
    HIIDRA_HOOKTYPE_GXDraw,
    HIIDRA_HOOKTYPE_GXFlush,
    HIIDRA_HOOKTYPE_OSSleepThread,
    HIIDRA_HOOKTYPE_AXNextFrame,
};

enum hiidradeflickermode {
    HIIDRA_DEFLICKER_AUTO,
    HIIDRA_DEFLICKER_OFF,
    HIIDRA_DEFLICKER_OFF_EXTENDED,
    HIIDRA_DEFLICKER_ON_LOW,
    HIIDRA_DEFLICKER_ON_MEDIUM,
    HIIDRA_DEFLICKER_ON_HIGH
};
