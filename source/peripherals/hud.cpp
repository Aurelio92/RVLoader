#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include "uamp.h"
#include "hud.h"
#include "gpio.h"

#define HUD_CONFIG_PATH "/rvloader/hud.conf"

#define CUR_HUD_VERSION 0
#define HUD_VOLUME_LIMIT 32
#define HUD_HEADPHONES_INIT_VOLUME  20
#define HUD_SPEAKERS_INIT_VOLUME    10

namespace HUD {
    typedef struct {
        u16 version;
        u8 headphonesVolume;
        u8 speakersVolume;
        u8 volumeControlSystem;
    } __attribute__((packed)) hudSettings_t;

    static hudSettings_t hudSettings;

    void init() {
        bool hasToRewriteConfig = false;
        hudSettings.version = CUR_HUD_VERSION;
        hudSettings.headphonesVolume = HUD_HEADPHONES_INIT_VOLUME;
        hudSettings.speakersVolume = HUD_SPEAKERS_INIT_VOLUME;
        hudSettings.volumeControlSystem = HUD_CTRL_GCPAD;

        FILE* hudFp = fopen(HUD_CONFIG_PATH, "rb");
        if (hudFp) {
            fseek(hudFp, 0, SEEK_END);
            size_t hudSize = ftell(hudFp);
            rewind(hudFp);
            if (hudSize == 2) { //Retrocompatibility with old simple format
                u8 hudVolume[2];
                fread(hudVolume, 1, 2, hudFp);
                hudSettings.headphonesVolume = hudVolume[0];
                hudSettings.speakersVolume = hudVolume[1];
                hasToRewriteConfig = true;
            } else if (hudSize < sizeof(hudSettings_t)) { //Possibly older config file version
                fread(&hudSettings, 1, hudSize, hudFp);
                //Check for possibly corrupted file
                if (hudSettings.version >= CUR_HUD_VERSION ||
                    hudSettings.volumeControlSystem > HUD_CTRL_BUTTONS) {
                    hudSettings.version = CUR_HUD_VERSION;
                    hudSettings.headphonesVolume = HUD_HEADPHONES_INIT_VOLUME;
                    hudSettings.speakersVolume = HUD_SPEAKERS_INIT_VOLUME;
                    hudSettings.volumeControlSystem = HUD_CTRL_GCPAD;
                } else {
                    //Implement here config file version updates
                }
                hasToRewriteConfig = true;
            } else if (hudSize > sizeof(hudSettings_t)) { //Assume corrupted file in this case
                hudSettings.version = CUR_HUD_VERSION;
                hudSettings.headphonesVolume = HUD_HEADPHONES_INIT_VOLUME;
                hudSettings.speakersVolume = HUD_SPEAKERS_INIT_VOLUME;
                hudSettings.volumeControlSystem = HUD_CTRL_GCPAD;
                hasToRewriteConfig = true;
            } else{
                fread(&hudSettings, 1, sizeof(hudSettings_t), hudFp);
                //Check for possibly corrupted file
                if (hudSettings.version != CUR_HUD_VERSION ||
                    hudSettings.volumeControlSystem > HUD_CTRL_BUTTONS) {
                    hudSettings.version = CUR_HUD_VERSION;
                    hudSettings.headphonesVolume = HUD_HEADPHONES_INIT_VOLUME;
                    hudSettings.speakersVolume = HUD_SPEAKERS_INIT_VOLUME;
                    hudSettings.volumeControlSystem = HUD_CTRL_GCPAD;
                    hasToRewriteConfig = true;
                }
            }

            fclose(hudFp);
        } else { //File doesn't exist
            hasToRewriteConfig = true;
        }

        if (hudSettings.headphonesVolume > HUD_VOLUME_LIMIT) {
            hudSettings.headphonesVolume = HUD_HEADPHONES_INIT_VOLUME;
            hasToRewriteConfig = true;
        }
        if (hudSettings.speakersVolume > HUD_VOLUME_LIMIT) {
            hudSettings.speakersVolume = HUD_SPEAKERS_INIT_VOLUME;
            hasToRewriteConfig = true;
        }

        if (hasToRewriteConfig) {
            hudFp = fopen(HUD_CONFIG_PATH, "wb");
            if (hudFp) {
                fwrite(&hudSettings, 1, sizeof(hudSettings_t), hudFp);
                fclose(hudFp);
            }
        }

        if (UAMP::isConnected()) {
            write32(HW_GPIO_DIR_ADDR, read32(HW_GPIO_DIR_ADDR) & (~GPIO_DEBUG2));
            int headphonesIn = read32(HW_GPIOB_IN_ADDR) & GPIO_DEBUG2;

            UAMP::init();
            UAMP::setMute((headphonesIn && !hudSettings.headphonesVolume) || (!headphonesIn && !hudSettings.speakersVolume));
            if (hudSettings.headphonesVolume > 0)
                UAMP::setHeadphonesVolume(hudSettings.headphonesVolume - 1);
            else
                UAMP::setHeadphonesVolume(0);
            if (hudSettings.speakersVolume > 0)
                UAMP::setSpeakersVolume(hudSettings.speakersVolume - 1);
            else
                UAMP::setSpeakersVolume(0);
        }
    }

    void setHeadphonesVolume(u8 vol) {
        if (vol > 32) {
            vol = 32;
        }
        hudSettings.headphonesVolume = vol;

        FILE* hudFp = fopen(HUD_CONFIG_PATH, "wb");
        if (hudFp) {
            fwrite(&hudSettings, 1, sizeof(hudSettings_t), hudFp);
            fclose(hudFp);
        }

        int headphonesIn = read32(HW_GPIOB_IN_ADDR) & GPIO_DEBUG2;
        UAMP::setMute(headphonesIn && !hudSettings.headphonesVolume);
        if (hudSettings.headphonesVolume > 0)
            UAMP::setHeadphonesVolume(hudSettings.headphonesVolume - 1);
        else
            UAMP::setHeadphonesVolume(0);
    }

    u8 getHeadphonesVolume() {
        return hudSettings.headphonesVolume;
    }

    void setSpeakersVolume(u8 vol) {
        if (vol > 32) {
            vol = 32;
        }
        hudSettings.speakersVolume = vol;

        FILE* hudFp = fopen(HUD_CONFIG_PATH, "wb");
        if (hudFp) {
            fwrite(&hudSettings, 1, sizeof(hudSettings_t), hudFp);
            fclose(hudFp);
        }

        int headphonesIn = read32(HW_GPIOB_IN_ADDR) & GPIO_DEBUG2;
        UAMP::setMute(!headphonesIn && !hudSettings.speakersVolume);
        if (hudSettings.speakersVolume > 0)
            UAMP::setSpeakersVolume(hudSettings.speakersVolume - 1);
        else
            UAMP::setSpeakersVolume(0);
    }

    u8 getSpeakersVolume() {
        return hudSettings.speakersVolume;
    }

    void setVolumeControlSystem(u8 ctrlSystem) {
        if (ctrlSystem <= HUD_CTRL_BUTTONS) {
            hudSettings.volumeControlSystem = ctrlSystem;
            FILE* hudFp = fopen(HUD_CONFIG_PATH, "wb");
            if (hudFp) {
                fwrite(&hudSettings, 1, sizeof(hudSettings_t), hudFp);
                fclose(hudFp);
            }
        }
    }

    u8 getVolumeControlSystem() {
        return hudSettings.volumeControlSystem;
    }
};