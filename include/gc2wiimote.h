#pragma once

#define GC2WIIMOTE_VER 0x0100

typedef struct {
    char name[0x44];
    char id[8];
} titleInfo;

typedef enum {
    ORIENT_STANDARD = 0,
    ORIENT_SIDEWAYS,
    ORIENT_MAX
} WMOrientation_t;

const char WMOrientation_Name[][32] = {
    "Standard",
    "Sideways"
};

typedef enum {
    EXT_NONE = 0,
    EXT_NUNCHUK,
    EXT_CLASSIC,
    EXT_MAX
} WMExt_t;

const char WMExt_Name[][32] = {
    "None",
    "Nunchuk",
    "Classic controller"
};

typedef enum {
    IRMODE_DIRECT = 0,
    IRMODE_SHIFT,
    IRMODE_MAX
} WMIRMode_t;

const char WMIRMode_Name[][32] = {
    "Direct",
    "Shift"
};

typedef struct {
    //Wiimote
    u32 WM_IR;
    u32 WM_DU;
    u32 WM_DD;
    u32 WM_DR;
    u32 WM_DL;
    u32 WM_A;
    u32 WM_B;
    u32 WM_1;
    u32 WM_2;
    u32 WM_Plus;
    u32 WM_Minus;
    u32 WM_Home;
    u32 WM_ShakeX;
    u32 WM_ShakeY;
    u32 WM_ShakeZ;
    //Nunchuk
    u32 NU_Stick;
    u32 NU_C;
    u32 NU_Z;
    u32 NU_ShakeX;
    u32 NU_ShakeY;
    u32 NU_ShakeZ;
    //Classic controller
    u32 CC_DU;
    u32 CC_DD;
    u32 CC_DR;
    u32 CC_DL;
    u32 CC_A;
    u32 CC_B;
    u32 CC_X;
    u32 CC_Y;
    u32 CC_RA;
    u32 CC_LA;
    u32 CC_RD;
    u32 CC_LD;
    u32 CC_Plus;
    u32 CC_Minus;
    u32 CC_Home;
    u32 CC_LStick;
    u32 CC_RStick;
} WMMapping_t_v1_00;

typedef struct {
    //Wiimote
    u32 WM_IR;
    u32 WM_DU;
    u32 WM_DD;
    u32 WM_DR;
    u32 WM_DL;
    u32 WM_A;
    u32 WM_B;
    u32 WM_1;
    u32 WM_2;
    u32 WM_Plus;
    u32 WM_Minus;
    u32 WM_Home;
    u32 WM_ShakeX;
    u32 WM_ShakeY;
    u32 WM_ShakeZ;
    //Nunchuk
    u32 NU_Stick;
    u32 NU_C;
    u32 NU_Z;
    u32 NU_ShakeX;
    u32 NU_ShakeY;
    u32 NU_ShakeZ;
    //Classic controller
    u32 CC_DU;
    u32 CC_DD;
    u32 CC_DR;
    u32 CC_DL;
    u32 CC_A;
    u32 CC_B;
    u32 CC_X;
    u32 CC_Y;
    u32 CC_RA;
    u32 CC_LA;
    u32 CC_RD;
    u32 CC_LD;
    u32 CC_Plus;
    u32 CC_Minus;
    u32 CC_Home;
    u32 CC_LStick;
    u32 CC_RStick;
} WMMapping_t;

typedef struct {
    u32 magic;
    u32 version;
    u32 size;
    u32 orientation;
    u32 extension;
    u32 mpEnabled;
    u32 IRMode;
    u32 IRTimeout;
    WMMapping_t mapping;
    WMMapping_t modifiers;
    WMMapping_t negModifiers;
} WMEmuConfig_t_v1_00;

typedef struct {
    u32 magic;
    u32 version;
    u32 size;
    u32 orientation;
    u32 extension;
    u32 mpEnabled;
    u32 IRMode;
    u32 IRTimeout;
    WMMapping_t mapping;
    WMMapping_t modifiers;
    WMMapping_t negModifiers;
} WMEmuConfig_t;

const std::string PADNames[] = {
    "DL", "DR", "DD", "DU", "Z", "R", "L", "", "A", "B", "X", "Y", "Start", "", "", "", "Stick", "C-Stick", "", "", "StickRight", "StickLeft", "StickUp", "StickDown", "C-StickRight", "C-StickLeft", "C-StickUp", "C-StickDown"
};

#define NPADNAMES 28

#define SSTICK      0x00010000
#define CSTICK      0x00020000
#define SSTICKRIGHT 0x00100000
#define SSTICKLEFT  0x00200000
#define SSTICKUP    0x00400000
#define SSTICKDOWN  0x00800000
#define CSTICKRIGHT 0x01000000
#define CSTICKLEFT  0x02000000
#define CSTICKUP    0x04000000
#define CSTICKDOWN  0x08000000
