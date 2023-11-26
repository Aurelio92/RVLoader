#include <stdio.h>
#include <string.h>
#include <gccore.h>

namespace SYSCONF {
    static u8* __conf_buffer = NULL;
    static u8* __conf_txt_buffer = NULL;

    void setConfBuffer(u8* buffer) {
        __conf_buffer = buffer;
    }

    void setConfTxtBuffer(u8* buffer) {
        __conf_txt_buffer = buffer;
    }

    void decencTextBuffer() {
        u32 key = 0x73B5DBFA;
        int i;

        for(i=0; i<0x100; i++) {
            __conf_txt_buffer[i] ^= key & 0xff;
            key = (key<<1) | (key>>31);
        }
    }

    u8* __Find(const char *name) {
        u16 count;
        u16* offset;
        int nlen = strlen(name);
        count = *((u16*)(&__conf_buffer[4]));
        offset = (u16*)&__conf_buffer[6];

        while(count--) {
            if((nlen == ((__conf_buffer[*offset]&0x0F)+1)) && !memcmp(name, &__conf_buffer[*offset+1], nlen))
                return &__conf_buffer[*offset];
            offset++;
        }
        return NULL;
    }

    s32 GetLength(const char *name) {
        u8 *entry;

        entry = __Find(name);
        if(!entry) return CONF_ENOENT;

        switch(*entry>>5) {
            case 1:
                return *((u16*)&entry[strlen(name)+1]) + 1;
            case 2:
                return entry[strlen(name)+1] + 1;
            case 3:
                return 1;
            case 4:
                return 2;
            case 5:
                return 4;
            case 7:
                return 1;
            default:
                return CONF_ENOTIMPL;
        }
    }

    s32 GetType(const char *name) {
        u8 *entry;

        entry = __Find(name);
        if(!entry) return CONF_ENOENT;

        return *entry>>5;
    }

    s32 Get(const char *name, void *buffer, u32 length) {
        u8 *entry;
        s32 len;

        entry = __Find(name);
        if(!entry) return CONF_ENOENT;

        len = GetLength(name);
        if(len<0) return len;
        if(len>length) return CONF_ETOOBIG;

        switch(*entry>>5) {
            case CONF_BIGARRAY:
                memcpy(buffer, &entry[strlen(name)+3], len);
                break;
            case CONF_SMALLARRAY:
                memcpy(buffer, &entry[strlen(name)+2], len);
                break;
            case CONF_BYTE:
            case CONF_SHORT:
            case CONF_LONG:
            case CONF_BOOL:
                memset(buffer, 0, length);
                memcpy(buffer, &entry[strlen(name)+1], len);
                break;
            default:
                return CONF_ENOTIMPL;
        }
        return len;
    }

    s32 CONF_Set(const char *name, void *buffer, u32 length) {
        u8 *entry;
        s32 len;

        entry = __Find(name);
        if(!entry) return CONF_ENOENT;

        len = GetLength(name);
        if(len < 0) return len;
        if(len != length) return CONF_ETOOBIG;

        switch(*entry>>5) {
            case CONF_BIGARRAY:
                memcpy(&entry[strlen(name)+3], buffer, len);
                break;
            case CONF_SMALLARRAY:
                memcpy(&entry[strlen(name)+2], buffer, len);
                break;
            case CONF_BYTE:
            case CONF_SHORT:
            case CONF_LONG:
            case CONF_BOOL:
                memset(&entry[strlen(name)+1], 0, length);
                memcpy(&entry[strlen(name)+1], buffer, len);
                break;
            default:
                return CONF_ENOTIMPL;
        }
        return len;
    }

    s32 GetPadDevices(conf_pads* pads) {
        int res;

        res = Get("BT.DINF", pads, sizeof(conf_pads));
        if(res < 0) return res;
        if(res < sizeof(conf_pads)) return CONF_EBADVALUE;
        return 0;
    }

    s32 GetSensorBarPosition(void) {
        int res;
        u8 val = 0;
        res = Get("BT.BAR", &val, 1);
        if(res<0) return res;
        if(res!=1) return CONF_EBADVALUE;
        return val;
    }

    s32 injectGC2Wiimote() {
        int res;
        u8 bar = 0;
        conf_pads pads;
        //res = GetPadDevices(&pads);
        //Debug_hexdump(&pads, sizeof(conf_pads));
        memset(&pads, 0, sizeof(conf_pads));
        pads.num_registered = 1;
        sprintf(pads.registered[0].name, "Nintendo RVL-CNT-01");
        pads.registered[0].bdaddr[0] = 0xD8;
        pads.registered[0].bdaddr[1] = 0x6B;
        pads.registered[0].bdaddr[2] = 0xF7;
        pads.registered[0].bdaddr[3] = 0xED;
        pads.registered[0].bdaddr[4] = 0x95;
        pads.registered[0].bdaddr[5] = 0x25;
        pads.active[0].bdaddr[0] = 0xD8;
        pads.active[0].bdaddr[1] = 0x6B;
        pads.active[0].bdaddr[2] = 0xF7;
        pads.active[0].bdaddr[3] = 0xED;
        pads.active[0].bdaddr[4] = 0x95;
        pads.active[0].bdaddr[5] = 0x25;
        CONF_Set("BT.DINF", &pads, sizeof(conf_pads));

        //Force top position for bar
        CONF_Set("BT.BAR", &bar, 1);

        return 0;
    }

    void setRegion(u32 gameID) {
        u8 valU8;
        u8 CCode[0x100A];
        Get("IPL.SADR", CCode, 0x100A);
        switch (gameID & 0xFF) {
            case 'J':
                valU8 = 0;
                CONF_Set("IPL.LNG", &valU8, 1);
                CCode[0] = 1;
                Get("IPL.SADR", CCode, 0x100A);
            break;
            case 'E':
                valU8 = 1;
                CONF_Set("IPL.LNG", &valU8, 1);
                CCode[0] = 31;
                Get("IPL.SADR", CCode, 0x100A);
            break;
            case 'D':
            case 'F':
            case 'I':
            case 'M':
            case 'P':
            case 'S':
            case 'U':
                Get("IPL.LNG", &valU8, 1);
                if (valU8 == 0 || valU8 >= 9)
                    valU8 = 1;
                CONF_Set("IPL.LNG", &valU8, 1);
                CCode[0] = 110;
                Get("IPL.SADR", CCode, 0x100A);
            break;
            case 'k':
                valU8 = 9;
                CONF_Set("IPL.LNG", &valU8, 1);
                CCode[0] = 136;
                Get("IPL.SADR", CCode, 0x100A);
            break;
        }
    }

    void clearSettings() {
        memset(__conf_txt_buffer, 0, 0x100);
    }

    bool getSetting(const char item[128], char val[128]) {
        char *curstrt, *curend;
        char *curitem = strstr((char*)__conf_txt_buffer, item);
        val[0] = '\0';

        curstrt = strchr(curitem, '=');
        curend = strchr(curitem, 0x0d);

        if (curstrt && curend) {
            curstrt++;
            u32 len = curend - curstrt;
            strncpy(val, curstrt, len);
            val[len] = '\0';

            return true;
        }

        return false;
    }
    
    void setSetting(const char item[128], const char val[128]) {
        char* settingEnd = strchr((char*)__conf_txt_buffer, '\0');
        sprintf(settingEnd, "%s=%s\r\n", item, val);
    }

    void setRegionSetting(u32 gameID) {
        char dvd[128];
        char mpch[128];
        char serno[128];
        getSetting("DVD", dvd);
        getSetting("MPCH", mpch);
        getSetting("SERNO", serno);
        switch (gameID & 0xFF) {
            case 'J':
                clearSettings();
                setSetting("DVD", dvd);
                setSetting("MPCH", mpch);
                setSetting("SERNO", serno);
                setSetting("AREA", "JPN");
                setSetting("MODEL", "RVL-001(JPN)");
                setSetting("CODE", "LJM");
                setSetting("VIDEO", "NTSC");
                setSetting("GAME", "JP");
            break;
            case 'E':
                clearSettings();
                setSetting("DVD", dvd);
                setSetting("MPCH", mpch);
                setSetting("SERNO", serno);
                setSetting("AREA", "USA");
                setSetting("MODEL", "RVL-001(USA)");
                setSetting("CODE", "LU");
                setSetting("VIDEO", "NTSC");
                setSetting("GAME", "US");
            break;
            case 'D':
            case 'F':
            case 'I':
            case 'M':
            case 'P':
            case 'S':
            case 'U':
                clearSettings();
                setSetting("DVD", dvd);
                setSetting("MPCH", mpch);
                setSetting("SERNO", serno);
                setSetting("AREA", "EUR");
                setSetting("MODEL", "RVL-001(EUR)");
                setSetting("CODE", "LEH");
                setSetting("VIDEO", "PAL");
                setSetting("GAME", "EU");
            break;
            case 'k':
                clearSettings();
                setSetting("DVD", dvd);
                setSetting("MPCH", mpch);
                setSetting("SERNO", serno);
                setSetting("AREA", "KOR");
                setSetting("MODEL", "RVL-001(KOR)");
                setSetting("CODE", "LKM");
                setSetting("VIDEO", "NTSC");
                setSetting("GAME", "KR");
            break;
        }
    }
};
