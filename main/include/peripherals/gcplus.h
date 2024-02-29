#pragma once

#include <gccore.h>
#include <string>

//GCC standard commands
#define SI_CMD_ID       0x00
#define SI_CMD_POLL     0x40
#define SI_CMD_ORIGINS  0x41
#define SI_CMD_CALIB    0x42
#define SI_CMD_RESET    0xFF

//GC+2.0 custom commands
#define GCP_CMD_LOCKUNLOCK      0x10
#define GCP_CMD_GETVER          0x11
#define GCP_CMD_WRITEEEPROM     0x12
#define GCP_CMD_READEEPROM      0x13
#define GCP_CMD_RESET           0x14
#define GCP_CMD_BOOTBL          0x15
#define GCP_CMD_SETMAPBYTE0     0x16
#define GCP_CMD_SETMAPBYTE1     0x17
#define GCP_CMD_GETMAPBYTE0     0x18
#define GCP_CMD_GETMAPBYTE1     0x19
#define GCP_CMD_REBUILDLUT      0x20

//GC+2.0 bootloader specific commands
#define GCP_CMD_RESETIDX        0x70
#define GCP_CMD_FILLBUFFER      0x71
#define GCP_CMD_READBUFFER      0x72
#define GCP_CMD_WRITEFLASH      0x73
#define GCP_CMD_READFLASH       0x74
#define GCP_CMD_BOOTPAYLOAD     0x75

#define GCP_ERR_NONE        0x00
#define GCP_ERR_LOCKED      0xFF
#define GCP_ERR_WRONGARG    0xFE
#define GCP_ERR_WRONGMODE   0xFD

#define GCP_TRANSFER_DELAY 2000 //ms
#define GCP_AFTERTRANSFER_DELAY 5000 //us

#define DZ_MODE_RADIAL          0
#define DZ_MODE_SCALEDRADIAL    1

#define TRIG_MODE_DIGITAL   0
#define TRIG_MODE_ANALOG    1

typedef enum {
    GCP_MAP_BUTTON_A_ID = 0,
    GCP_MAP_BUTTON_B_ID,
    GCP_MAP_BUTTON_X_ID,
    GCP_MAP_BUTTON_Y_ID,
    GCP_MAP_BUTTON_ST_ID,
    GCP_MAP_BUTTON_DL_ID,
    GCP_MAP_BUTTON_DR_ID,
    GCP_MAP_BUTTON_DD_ID,
    GCP_MAP_BUTTON_DU_ID,
    GCP_MAP_BUTTON_Z_ID,
    GCP_MAP_BUTTON_RD_ID,
    GCP_MAP_BUTTON_LD_ID,
    GCP_MAP_BUTTON_LA_ID,
    GCP_MAP_BUTTON_RA_ID,
    GCP_MAP_BUTTON_Z2_ID,
    GCP_MAP_N_BUTTONS
} GCP_MAP_ButtonsID;

namespace GCPlus {
    bool isV1();
    bool isV2();
    bool unlock();
    bool lock();
    bool getFWVer(u16* version);
    bool getHWVer(u16* version);
    bool getMode(u8* mode);
    bool writeEEPROM(u8 addr, u8* dataToWrite, u8 len);
    bool readEEPROM(u8 addr, u8* output, u8 len);
    bool reset();
    bool bootBootloader();
    bool resetIDX();
    bool fillBuffer(u8* data, u8 len, u8* error);
    bool readBuffer(u8* output, u8 len);
    bool writeFlash(u16 addr);
    bool readFlash(u16 addr);
    bool bootPayload();
    bool flashPayload(u8* payload, u32 payloadSize);
    bool rebuildLUT();
    bool setMapping(int* mapping);
    bool setDefaultMapping();
    u32 getUpdateProgress();
    bool isUpdating();
    bool hasUpdateSucceeded();
    void startUpdate(std::string path);
};
