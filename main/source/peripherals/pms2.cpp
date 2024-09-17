#include <string>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <math.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include "i2c.h"
#include "pms2.h"
#include "main.h"

#define PMS2_ADDR (0x20 << 1)
#define PMS2L_ADDR (0x21 << 1)

#define PGM_BLOCK_SIZE  0x80UL      //bytes
#define PMS2_PAYLOAD_ADDR 0x2000UL

#define CMD_GETVER              0x00
#define CMD_BOOTBL              0x01
#define CMD_FLASHCONFIG         0x02
#define CMD_NTC                 0x03
#define CMD_FAN                 0x04
#define CMD_PID_KP              0x05
#define CMD_PID_KI              0x06
#define CMD_PID_KD              0x07
#define CMD_PID_TARGET          0x08
#define CMD_PD_PROFILES         0x09
#define CMD_CONF0               0x0A
#define CMD_SHIPPINGMODE        0x0B
#define CMD_FANRANGE            0x0C
#define CMD_LEDINTENSITY        0x0D
#define CMD_POT                 0x0E

#define CMD_CHARGECURRENT       0x10
#define CMD_TERMCURRENT         0x11
#define CMD_PRECHARGECURRENT    0x12
#define CMD_CHARGEVOLTAGE       0x13
#define CMD_TREG                0x14
#define CMD_CHARGESTATUS        0x15

#define CMD_MAXRECONFIGURE      0x20
#define CMD_BATDESIGNCAP        0x21
#define CMD_MAXVEMPTY           0x22
#define CMD_MAXCYCLES           0x23
#define CMD_BATSOC              0x24
#define CMD_BATCURRENT          0x25
#define CMD_BATVOLTAGE          0x26
#define CMD_TTE                 0x27
#define CMD_TTF                 0x28

#define CMD_WRITEFLASH          0xF0
#define CMD_READFLASH           0xF1
#define CMD_BOOTPAYLOAD         0xF2

#define ERR_NONE                0x00
#define ERR_LOCKED              0xFF
#define ERR_WRONGARG            0xFE
#define ERR_WRONGMODE           0xFD
#define ERR_WRONGCRC            0xFC
#define ERR_WRONGADDR           0xFB

#define PMS_FLASH_RETRIES       20

#define PMS_POLLUPDATE_TIMEOUT  200

#define STACKSIZE 8192

extern "C" {
    extern void udelay(int us);
}

namespace PMS2 {
    static lwp_t updateThreadHandle;
    static u8* updateThreadStack = NULL;
    static mutex_t updateMutex = 0;

    static bool updating = false;
    static u32 updateProgress = 0;
    static bool updateSuccess = false;
    static std::string updatePath;

    static u8 curPMSAddress = PMS2_ADDR;

    bool isLite() {
        return (curPMSAddress == PMS2L_ADDR);
    }

    static void* updateThread(void* arg) {
        u8 error;
        u8 tempBuffer[5 + PGM_BLOCK_SIZE];
        u8 unlockBuffer[4] = {0x50, 0x4D , 0x53, 0x32};
        const u8 PMS2Magic[8] = {'P', 'M', 'S', '2', ' ', '4', 'L', 'T'};
        const u8 PMS2LMagic[8] = {'P', 'M', 'S', '2', 'L', '4', 'L', 'T'};
        u8 magic[8];
        u8 writeFlashError;

        printf("PMS2 update thread started\n");
        u8 verBuffer[3];
        u16 version;
        u16 minVer;
        u16 majVer;

        FILE* fp = fopen(updatePath.c_str(), "rb");
        if (!fp) {
            LWP_MutexLock(updateMutex);
            updateSuccess = false;
            updating = false;
            LWP_MutexUnlock(updateMutex);
            return NULL;
        }

        //Check that the update file is for the right kind of PMS2
        fread(magic, 1, sizeof(magic), fp);
        if ((isLite() && memcmp(magic, PMS2LMagic, sizeof(magic))) || (!isLite() && memcmp(magic, PMS2Magic, sizeof(magic)))) {
            fclose(fp);
            LWP_MutexLock(updateMutex);
            updateSuccess = false;
            updating = false;
            LWP_MutexUnlock(updateMutex);
            return NULL;
        }

        printf("Reading version\n");
        i2c_readBuffer(curPMSAddress, CMD_GETVER, &error, verBuffer, 3);
        version = verBuffer[1] | (verBuffer[2] << 8);
        minVer = (version & 0x0007);
        majVer = ((version >> 13) & 0x1FFF);
        printf("Currently running: %s (v%u.%u)\n", verBuffer[0] ? "bootloader" : "payload", majVer, minVer);
        printf("Attempting to boot bootloader\n");
        printf("PMS2 booting bootloader\n");
        i2c_writeBuffer(curPMSAddress, CMD_BOOTBL, unlockBuffer, 4, &error);
        printf("Waiting...\n");
        udelay(100000);
        printf("Reading version\n");
        i2c_readBuffer(curPMSAddress, CMD_GETVER, &error, verBuffer, 3);
        version = verBuffer[1] | (verBuffer[2] << 8);
        minVer = (version & 0x0007);
        majVer = ((version >> 13) & 0x1FFF);
        printf("Currently running: %s (v%u.%u)\n", verBuffer[0] ? "bootloader" : "payload", majVer, minVer);

        //Read update file size
        fseek(fp, 0, SEEK_END);
        u32 size = ftell(fp) - sizeof(magic); //Skip header
        fseek(fp, sizeof(magic), SEEK_SET); //Skip header

        u32 tempSize = size;
        u32 flashedSize = 0;
        u32 address = PMS2_PAYLOAD_ADDR;

        while (tempSize) {
            u8 retries = PMS_FLASH_RETRIES;
            bool flashed = false;

            tempBuffer[0] = address & 0xFF;
            tempBuffer[1] = (address >> 8) & 0xFF;
            tempBuffer[2] = (address >> 16) & 0xFF;
            fread(&tempBuffer[3], 1, PGM_BLOCK_SIZE + 2, fp);
            //i2c_writeBuffer(curPMSAddress, CMD_WRITEFLASH, tempBuffer, 5 + PGM_BLOCK_SIZE, &error);
            while (retries-- && !flashed) {
                i2c_writeReadBuffer(curPMSAddress, CMD_WRITEFLASH, tempBuffer, 5 + PGM_BLOCK_SIZE, &writeFlashError, 1, &error);

                if (writeFlashError == ERR_NONE)
                    flashed = true;
                else
                    printf("%06X: writeFlashError: %d\n", address, writeFlashError);
            }

            if (!flashed) {
                fclose(fp);
                printf("PMS2 booting payload\n");
                i2c_writeBuffer(curPMSAddress, CMD_BOOTPAYLOAD, unlockBuffer, 4, &error);
                udelay(100000);
                LWP_MutexLock(updateMutex);
                updateSuccess = false;
                updating = false;
                LWP_MutexUnlock(updateMutex);
                return NULL;
            }

            flashedSize += PGM_BLOCK_SIZE + 2;
            tempSize -= PGM_BLOCK_SIZE + 2;
            address += PGM_BLOCK_SIZE;

            LWP_MutexLock(updateMutex);
            updateProgress = 100 * flashedSize / size;
            LWP_MutexUnlock(updateMutex);
        }
        fclose(fp);

        printf("PMS2 booting payload\n");
        i2c_writeBuffer(curPMSAddress, CMD_BOOTPAYLOAD, unlockBuffer, 4, &error);
        udelay(100000);

        LWP_MutexLock(updateMutex);
        updating = false;
        updateSuccess = true;
        LWP_MutexUnlock(updateMutex);
        return NULL;
    }

    u32 getUpdateProgress() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        u32 temp = updateProgress;
        LWP_MutexUnlock(updateMutex);
        return temp;
    }

    bool isUpdating() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        bool temp = updating;
        LWP_MutexUnlock(updateMutex);
        return temp;
    }

    bool hasUpdateSucceeded() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        bool temp = updateSuccess;
        LWP_MutexUnlock(updateMutex);
        return temp;
    }

    void startUpdate(std::string path) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        updating = true;
        updateSuccess = false;
        updatePath = path;
        updateProgress = 0;
        LWP_MutexUnlock(updateMutex);

        if (updateThreadStack == NULL)
            updateThreadStack = (u8 *)memalign(32, STACKSIZE);

        printf("Starting PMS2 update thread\n");
        LWP_CreateThread(&updateThreadHandle, updateThread, NULL, updateThreadStack, STACKSIZE, 30);
    }

    static inline void swap32(u8* data) {
        u8 temp;
        temp = data[0];
        data[0] = data[3];
        data[3] = temp;
        temp = data[1];
        data[1] = data[2];
        data[2] = temp;
    }

    bool isConnected() {
        if (isRunningOnDolphin()) {
            return false;
        }

        u32 level;
        u8 i2cRet;
        static bool firstTime = true;
        static bool ret = false;

        if (firstTime) {
            firstTime = false;
            _CPU_ISR_Disable(level);
            ret = true;
            i2c_start();
            i2cRet = i2c_sendByte(PMS2_ADDR);
            i2c_stop();
            if (!i2cRet) {
                //Try PMS2 lite
                i2c_start();
                i2cRet = i2c_sendByte(PMS2L_ADDR);
                i2c_stop();
                if (!i2cRet) {
                    ret = false;
                } else {
                    curPMSAddress = PMS2L_ADDR;
                }
            } else {
                curPMSAddress = PMS2_ADDR;
            }
            _CPU_ISR_Restore(level);
        }
        return ret;
    }

    float getVer() {
        u8 error;
        u8 verBuffer[3];
        i2c_readBuffer(curPMSAddress, CMD_GETVER, &error, verBuffer, 3);
        u16 version = verBuffer[1] | (verBuffer[2] << 8);
        float ret = ((version >> 3) & 0x1FFF) + (version & 0x0007) / 10.0f;
        return ret;
    }

    void getPDProfiles(u32* profiles) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_readBuffer(curPMSAddress, CMD_PD_PROFILES, &error, (u8*)profiles, 11 * sizeof(u32));
        for (int i = 0; i < 11; i++)
            swap32((u8*)&profiles[i]);
    }

    u8 getConf0() {
        u8 error;
        static u64 lastTime = 0;
        static u8 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read8(curPMSAddress, CMD_CONF0, &error);
        return ret;
    }

    u16 getPot() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_POT, &error);
        return ret;
    }

    u16 getChargeCurrent() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_CHARGECURRENT, &error);
        return ret;
    }

    u16 getTermCurrent() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_TERMCURRENT, &error);
        return ret;
    }

    u16 getPreChargeCurrent() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_PRECHARGECURRENT, &error);
        return ret;
    }

    u16 getChargeVoltage() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_CHARGEVOLTAGE, &error);
        return ret;
    }

    u8 getTREG() {
        u8 error;
        static u64 lastTime = 0;
        static u8 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read8(curPMSAddress, CMD_TREG, &error);
        return ret;
    }

    u8 getChargeStatus() {
        u8 error;
        static u64 lastTime = 0;
        static u8 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read8(curPMSAddress, CMD_CHARGESTATUS, &error);
        return ret;
    }

    u16 getBatDesignCapacity() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return ret;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_BATDESIGNCAP, &error);
        return ret;
    }

    float getSOC() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0.0f;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        u16 soc = i2c_read16(curPMSAddress, CMD_BATSOC, &error);
        ret = (float)soc / 256.0f;
        return ret;
    }

    u16 getSOCRaw() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_BATSOC, &error);
        return ret;
    }

    float getVCell() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0.0f;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        u16 vcell = i2c_read16(curPMSAddress, CMD_BATVOLTAGE, &error);
        ret = 78.125f * (float)vcell / 1000.0f;
        return ret;
    }

    float getCurrent() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0.0f;

        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return ret;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        s16 current = (s16)i2c_read16(curPMSAddress, CMD_BATCURRENT, &error);
        ret = 312.5f * (float)current / 1000.0f;
        return ret;
    }

    u16 getVCellRaw() {
        u8 error;
        static u64 lastTime = 0;
        static u16 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read16(curPMSAddress, CMD_BATVOLTAGE, &error);
        return ret;
    }

    s16 getCurrentRaw() {
        u8 error;
        static u64 lastTime = 0;
        static s16 ret = 0;

        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return ret;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = (s16)i2c_read16(curPMSAddress, CMD_BATCURRENT, &error);
        return ret;
    }

    u32 getTTE() {
        u8 error;
        static u64 lastTime = 0;
        static u32 ret = 0;

        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return ret;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        u16 tte  = i2c_read16(curPMSAddress, CMD_TTE, &error);
        ret = (u32)round(5.625f * (float)tte / 60.0f);
        return ret;
    }

    u32 getTTF() {
        u8 error;
        static u64 lastTime = 0;
        static u32 ret = 0;

        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return ret;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        u16 ttf  = i2c_read16(curPMSAddress, CMD_TTF, &error);
        ret = (u32)round(5.625f * (float)ttf / 60.0f);
        return ret;
    }

    void setConf0(u8 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write8(curPMSAddress, CMD_CONF0, v, &error);
    }

    void setChargeCurrent(u16 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write16(curPMSAddress, CMD_CHARGECURRENT, v, &error);
    }

    void setTermCurrent(u16 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write16(curPMSAddress, CMD_TERMCURRENT, v, &error);
    }

    void setPreChargeCurrent(u16 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write16(curPMSAddress, CMD_PRECHARGECURRENT, v, &error);
    }

    void setChargeVoltage(u16 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write16(curPMSAddress, CMD_CHARGEVOLTAGE, v, &error);
    }

    void setTREG(u8 v) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write8(curPMSAddress, CMD_TREG, v, &error);
    }

    void setBatDesignCapacity(u16 v) {
        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_write16(curPMSAddress, CMD_BATDESIGNCAP, v, &error);
    }

    void enableShippingMode() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        u8 tempBuffer[4] = {0x50, 0x4D , 0x53, 0x32};
        i2c_writeBuffer(curPMSAddress, CMD_SHIPPINGMODE, tempBuffer, 4, &error);
    }

    void flashConfig() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        u8 tempBuffer[4] = {0x50, 0x4D , 0x53, 0x32};
        i2c_writeBuffer(curPMSAddress, CMD_FLASHCONFIG, tempBuffer, 4, &error);
    }

    void reconfigureMAX() {
        //This command is not available on PMS2 lite
        if (curPMSAddress == PMS2L_ADDR)
            return;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        u8 tempBuffer[4] = {0x50, 0x4D , 0x53, 0x32};
        i2c_writeBuffer(curPMSAddress, CMD_MAXRECONFIGURE, tempBuffer, 4, &error);
    }

    #define NTC_RBOT 2200.0f
    #define NTC_R25 10000.0f
    #define NTC_B 3988.0f
    #define NTC_T0 298.15f

    float NTCToCelsius(u16 ntc) {
        float VNTC = 2.048f * (float)ntc / 4095.0f;
        //VNTC = 1.8 * NTC_RBOT / (RNTC + NTC_RBOT)
        float RNTC = 1.8f * NTC_RBOT / VNTC - NTC_RBOT;
        //float RNTC  = NTC_RBOT * (4095.0f / (float)ntc - 1.0f);
        float Rinf = NTC_R25 * exp(-NTC_B / NTC_T0);
        float ret = NTC_B / log(RNTC / Rinf) - 273.15;
        return ret;
    }

    u16 CelsiusToNTC(float temperature) {
        float RNTC = NTC_R25 * exp(NTC_B * (1 / (temperature + 273.15) - 1 / NTC_T0));
        float VNTC = 1.8f * NTC_RBOT / (RNTC + NTC_RBOT);
        u16 ntc = (u16)(VNTC * 4095.0f / 2.048);
        return ntc;
    }

    float getNTC() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0.0f;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        u16 ntc = i2c_read16(curPMSAddress, CMD_NTC, &error);
        ret = NTCToCelsius(ntc);
        return ret;
    }

    u8 getFanSpeed() {
        u8 error;
        static u64 lastTime = 0;
        static u8 ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        ret = i2c_read8(curPMSAddress, CMD_FAN, &error);

        return ret;
    }

    void setFanSpeed(u8 speed) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        u8 tempBuffer[5] = {0x50, 0x4D , 0x53, 0x32, speed};
        i2c_writeBuffer(curPMSAddress, CMD_FAN, tempBuffer, 5, &error);
    }

    void freeFan() {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        u8 tempBuffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
        i2c_writeBuffer(curPMSAddress, CMD_FAN, tempBuffer, 5, &error);
    }

    float getFanPIDkP() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_PID_KP, &error, (u8*)&ret, 4);
        swap32((u8*)&ret);

        return ret;
    }

    void setFanPIDkP(float kP) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        swap32((u8*)&kP);
        i2c_writeBuffer(curPMSAddress, CMD_PID_KP, (u8*)&kP, 4, &error);
    }

    float getFanPIDkI() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_PID_KI, &error, (u8*)&ret, 4);
        swap32((u8*)&ret);

        return ret;
    }

    void setFanPIDkI(float kI) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        swap32((u8*)&kI);
        i2c_writeBuffer(curPMSAddress, CMD_PID_KI, (u8*)&kI, 4, &error);
    }

    float getFanPIDkD() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_PID_KD, &error, (u8*)&ret, 4);
        swap32((u8*)&ret);

        return ret;
    }

    void setFanPIDkD(float kD) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        swap32((u8*)&kD);
        i2c_writeBuffer(curPMSAddress, CMD_PID_KD, (u8*)&kD, 4, &error);
    }

    float getFanPIDTarget() {
        u8 error;
        static u64 lastTime = 0;
        static float ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_PID_TARGET, &error, (u8*)&ret, 4);
        swap32((u8*)&ret);

        return ret;
    }

    void setFanPIDTarget(float target) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        swap32((u8*)&target);
        i2c_writeBuffer(curPMSAddress, CMD_PID_TARGET, (u8*)&target, 4, &error);
    }

    fanRange_t getFanRange() {
        u8 error;
        static u64 lastTime = 0;
        static fanRange_t ret = {0, 255};

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_FANRANGE, &error, (u8*)&ret, sizeof(fanRange_t));

        return ret;
    }

    void setFanRange(fanRange_t range) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_writeBuffer(curPMSAddress, CMD_FANRANGE, (u8*)&range, sizeof(fanRange_t), &error);
    }

    uint8_t getLEDIntensity() {
        u8 error;
        static u64 lastTime = 0;
        static uint8_t ret = 0;

        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return ret;
        }
        LWP_MutexUnlock(updateMutex);

        if ((diff_msec(gettime(), lastTime) < PMS_POLLUPDATE_TIMEOUT) && lastTime) {
            return ret;
        }
        lastTime = gettime();
        i2c_readBuffer(curPMSAddress, CMD_LEDINTENSITY, &error, (u8*)&ret, 1);

        return ret;
    }

    void setLEDIntensity(uint8_t intensity) {
        if (!updateMutex)
            LWP_MutexInit(&updateMutex, false);
        LWP_MutexLock(updateMutex);
        if (updating) {
            LWP_MutexUnlock(updateMutex);
            return;
        }
        LWP_MutexUnlock(updateMutex);

        u8 error;
        i2c_writeBuffer(curPMSAddress, CMD_LEDINTENSITY, &intensity, 1, &error);
    }
};
