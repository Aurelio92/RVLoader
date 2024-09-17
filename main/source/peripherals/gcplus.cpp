#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <unistd.h>
#include <malloc.h>
#include "gcplus.h"
#include "main.h"
#include "hex.h"

#define EEPROM_PACKET_SIZE 4
#define EEPROM_RETRIES  10
#define FLASH_PACKET_SIZE 32
#define FLASH_BLOCK_SIZE 64
#define FLASH_BASE_ADDRESS 0x2000

#define STACKSIZE 8192

namespace GCPlus {
    static lwp_t updateThreadHandle;
    static u8* updateThreadStack = NULL;
    static mutex_t updateMutex = 0;
    static mutex_t GCPMutex = 0;

    static bool updating = false;
    static u32 updateProgress = 0;
    static bool updateSuccess = false;
    static std::string updatePath;

    static bool gotPadAnswer = false;

    static void padCallback(s32 chan, u32 type) {
        gotPadAnswer = true;
    }

    static bool transferData(void *out, u32 out_len, void *in, u32 in_len) {
        #ifdef DEMOBUILD
        return true;
        #endif
        lockSIMutex();
        if (out_len > 8)
            SI_DisablePolling(0xf0000000);
        SI_Sync();
        while (SI_Busy());
        gotPadAnswer = false;
        SI_Transfer(0, out, out_len, in, in_len, padCallback, 0);
        u64 now = gettime();
        while ((ticks_to_millisecs(gettime() - now) < GCP_TRANSFER_DELAY) && !gotPadAnswer);
        now = gettime();
        if (out_len > 8)
            PAD_Reset(0xf0000000);
        while (ticks_to_microsecs(gettime() - now) < GCP_AFTERTRANSFER_DELAY);
        unlockSIMutex();
        return gotPadAnswer;
    }

    bool isV1() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = 0x31;
        u8 answer[2];

        if (!transferData(&cmd, 1, answer, 2)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer[0] == 1 && answer[1] == 0) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool isV2() {
        if (isRunningOnDolphin()) {
            return false;
        }

        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u16 version;

        if (!unlock()) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (!getHWVer(&version)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (!lock()) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if ((version >> 3) == 2) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool unlock() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd[5] = {GCP_CMD_LOCKUNLOCK, 0x47, 0x43, 0x2B, 0x32};
        u8 answer;
        if (!transferData(cmd, 5, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool lock() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd[5] = {GCP_CMD_LOCKUNLOCK, 0x00, 0x00, 0x00, 0x00};
        u8 answer;
        if (!transferData(cmd, 5, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool getFWVer(u16* version) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_GETVER;
        u8 answer[5];
        if (!transferData(&cmd, 1, answer, 5)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer[0] == GCP_ERR_LOCKED) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        *version = ((answer[2] << 8) | answer[1]);
        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool getHWVer(u16* version) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_GETVER;
        u8 answer[5];
        if (!transferData(&cmd, 1, answer, 5)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer[0] == GCP_ERR_LOCKED) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        *version = ((answer[4] << 8) | answer[3]);
        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool getMode(u8* mode) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_GETVER;
        u8 answer[5];
        *mode = 0;
        if (!transferData(&cmd, 1, answer, 5)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer[0] == GCP_ERR_LOCKED) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        *mode = answer[0];
        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool writeEEPROM(u8 addr, u8* data, u8 len) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8* cmd = (u8*)malloc(3 + EEPROM_PACKET_SIZE);
        u8* test = (u8*)malloc(EEPROM_PACKET_SIZE);
        u8 answer;

        while (len) {
            bool success = false;
            u8 pLen = len > EEPROM_PACKET_SIZE ? EEPROM_PACKET_SIZE : len;

            cmd[0] = GCP_CMD_WRITEEEPROM;
            cmd[1] = addr;
            cmd[2] = 0x00;
            for (int i = 0; i < pLen; i++) {
                cmd[3 + i] = data[i];
            }
            for (int i = 0; i < EEPROM_RETRIES && !success; i++) {
                if (!transferData(cmd, 3 + pLen, &answer, 1)) {
                    free(cmd);
                    free(test);
                    LWP_MutexUnlock(GCPMutex);
                    return false;
                }
                if (answer != GCP_ERR_NONE) {
                    printf("writeEEPROM error: %02X\n", answer);
                }
                readEEPROM(addr, test, pLen);
                if (!memcmp(test, data, pLen) && answer == GCP_ERR_NONE)
                    success = true;
            }

            if (answer != GCP_ERR_NONE || !success) {
                free(cmd);
                free(test);
                LWP_MutexUnlock(GCPMutex);
                return false;
            }

            len -= pLen;
            addr += pLen;
            data += pLen;
        }

        free(cmd);
        free(test);

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool readEEPROM(u8 addr, u8* output, u8 len) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 answer[EEPROM_PACKET_SIZE + 1];
        u8 cmd[4] = {GCP_CMD_READEEPROM, addr, 0x00, len};

        while (len) {
            u8 pLen = len > EEPROM_PACKET_SIZE ? EEPROM_PACKET_SIZE : len;
            cmd[3] = pLen;
            if (!transferData(cmd, 4, answer, pLen + 1)) {
                LWP_MutexUnlock(GCPMutex);
                return false;
            }

            if (answer[0] == GCP_ERR_NONE) {
                memcpy(output, &answer[1], pLen);
                output += pLen;
                len -= pLen;
                addr += pLen;
                cmd[1] = addr;
            } else {
                printf("readEEPROM error: %02X\n", answer[0]);
                LWP_MutexUnlock(GCPMutex);
                return false;
            }
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool reset() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_RESET;

        if (!transferData(&cmd, 1, NULL, 0)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool bootBootloader() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_BOOTBL;
        u8 answer;

        if (!transferData(&cmd, 1, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool resetIDX() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_RESETIDX;
        u8 answer;

        if (!transferData(&cmd, 1, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool fillBuffer(u8* data, u8 len, u8* error) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8* cmd = (u8*)malloc(1 + len);
        u8 answer;

        cmd[0] = GCP_CMD_FILLBUFFER;
        for (int i = 0; i < len; i++) {
            cmd[1 + i] = data[i];
        }

        if (!transferData(cmd, 1 + len, &answer, 1)) {
            *error = 1;
            free(cmd);
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        free(cmd);

        *error = answer;
        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool readBuffer(u8* output, u8 len) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd[2] = {GCP_CMD_READBUFFER, len};
        u8* answer = (u8*)malloc(len + 1);

        if (!transferData(&cmd, 1, answer, len + 1)) {
            free(answer);
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer[0] == GCP_ERR_NONE) {
            memcpy(output, &answer[1], len);
            free(answer);
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        free(answer);
        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool writeFlash(u16 addr) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd[3] = {GCP_CMD_WRITEFLASH, (u8)(addr & 0xFF), (u8)((addr >> 8) & 0xFF)};
        u8 answer;

        if (!transferData(cmd, 3, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool readFlash(u16 addr) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd[3] = {GCP_CMD_READFLASH, (u8)(addr & 0xFF), (u8)((addr >> 8) & 0xFF)};
        u8 answer;

        if (!transferData(cmd, 3, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer == GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return true;
        }

        LWP_MutexUnlock(GCPMutex);
        return false;
    }

    bool bootPayload() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_BOOTPAYLOAD;
        u8 answer;

        if (!transferData(&cmd, 1, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool flashPayload(u8* payload, u32 payloadSize) {
        u8 buffer[FLASH_PACKET_SIZE];
        u32 packetIndex = 0;
        u16 address = FLASH_BASE_ADDRESS;
        u32 totalSize = payloadSize;
        u8 error;

        //Make sure flash buffer index is reset
        if (!resetIDX()) {
            return false;
        }

        while (payloadSize) {
            //Fill packet
            for (int i = 0; (i < FLASH_PACKET_SIZE) && payloadSize; i++) {
               buffer[i] = *payload++;
               payloadSize--;
            }
            packetIndex++;
            if (!fillBuffer(buffer, FLASH_PACKET_SIZE, &error)) {
                return false;
            }

            //Flash the internal buffer if a whole block has been sent
            //Also force the flashing if we got to the end of the payload
            if ((packetIndex == (FLASH_BLOCK_SIZE / FLASH_PACKET_SIZE)) || !payloadSize) {
                packetIndex = 0;
                if (!writeFlash(address)) {
                    return false;
                }
                if (!resetIDX()) {
                    return false;
                }

                address += FLASH_BLOCK_SIZE;
            }

            #ifdef DEMOBUILD
            {
                u64 now = gettime();
                while (ticks_to_millisecs(gettime() - now) < 10);
            }
            #endif

            LWP_MutexLock(updateMutex);
            updateProgress = 100 * (totalSize - payloadSize) / totalSize;
            LWP_MutexUnlock(updateMutex);
            usleep(100);
        }

        return true;
    }

    bool rebuildLUT() {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 cmd = GCP_CMD_REBUILDLUT;

        if (!transferData(&cmd, 1, NULL, 0)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool setMapping(int* mapping) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);
        u8 answer;
        u8 buffer[GCP_MAP_N_BUTTONS + 1];

        buffer[0] = GCP_CMD_SETMAPBYTE0;
        for (int i = 0; i < GCP_MAP_N_BUTTONS; i++)
            buffer[i + 1] = (mapping[i] >> 8) & 0xFF;


        if (!transferData(buffer, GCP_MAP_N_BUTTONS + 1, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer != GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        buffer[0] = GCP_CMD_SETMAPBYTE1;
        for (int i = 0; i < GCP_MAP_N_BUTTONS; i++)
            buffer[i + 1] = mapping[i] & 0xFF;


        if (!transferData(buffer, GCP_MAP_N_BUTTONS + 1, &answer, 1)) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        if (answer != GCP_ERR_NONE) {
            LWP_MutexUnlock(GCPMutex);
            return false;
        }

        LWP_MutexUnlock(GCPMutex);
        return true;
    }

    bool setDefaultMapping() {
        int mapping[GCP_MAP_N_BUTTONS];
        memset(mapping, 0, sizeof(int) * GCP_MAP_N_BUTTONS);
        mapping[GCP_MAP_BUTTON_A_ID] = PAD_BUTTON_A;
        mapping[GCP_MAP_BUTTON_B_ID] = PAD_BUTTON_B;
        mapping[GCP_MAP_BUTTON_X_ID] = PAD_BUTTON_X;
        mapping[GCP_MAP_BUTTON_Y_ID] = PAD_BUTTON_Y;
        mapping[GCP_MAP_BUTTON_ST_ID] = PAD_BUTTON_START;
        mapping[GCP_MAP_BUTTON_DL_ID] = PAD_BUTTON_LEFT;
        mapping[GCP_MAP_BUTTON_DR_ID] = PAD_BUTTON_RIGHT;
        mapping[GCP_MAP_BUTTON_DD_ID] = PAD_BUTTON_DOWN;
        mapping[GCP_MAP_BUTTON_DU_ID] = PAD_BUTTON_UP;
        mapping[GCP_MAP_BUTTON_Z_ID] = PAD_TRIGGER_Z;
        mapping[GCP_MAP_BUTTON_RD_ID] = PAD_TRIGGER_R;
        mapping[GCP_MAP_BUTTON_LD_ID] = PAD_TRIGGER_L;

        return setMapping(mapping);
    }

    static void* updateThread(void* arg) {
        if (!GCPMutex)
            LWP_MutexInit(&GCPMutex, true);
        LWP_MutexLock(GCPMutex);

        IntelHex hex(updatePath.c_str());
        if (!hex.binary)
            goto fail;

        if (!GCPlus::unlock())
            goto fail;

        if (!GCPlus::bootBootloader())
            goto fail;
        usleep(100000);

        if (!GCPlus::unlock())
            goto fail;

        //Read update file size
        if (!flashPayload(hex.binary, hex.binarySize))
            goto fail;

        usleep(1000000);

        if (!GCPlus::bootPayload())
            goto fail;
        usleep(1000000);

        GCPlus::lock();
        LWP_MutexLock(updateMutex);
        updateSuccess = true;
        updating = false;
        LWP_MutexUnlock(updateMutex);

        LWP_MutexUnlock(GCPMutex);
        return NULL;

    fail:
        GCPlus::lock();
        LWP_MutexLock(updateMutex);
        updateSuccess = false;
        updating = false;
        LWP_MutexUnlock(updateMutex);

        LWP_MutexUnlock(GCPMutex);
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

        LWP_CreateThread(&updateThreadHandle, updateThread, NULL, updateThreadStack, STACKSIZE, 30);
    }
};
