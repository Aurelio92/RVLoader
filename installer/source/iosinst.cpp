#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <ogc/es.h>
#include <ogc/ipc.h>

#include "system.h"
#include "sha1.h"
#include "systitles.h"
#include "debug.h"
#include "haxx_certs.h"
#include "wd_bin.h"

extern "C" {
    extern void udelay(int us);
};

#define IOS36_WAD_PATH "/IOS36-64-v3608.wad"
#define IOS58_WAD_PATH "/IOS58-64-v6176.wad"
#define IOS80_WAD_PATH "/IOS80-64-v6944.wad"

static u8 commonkey[16] = {0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7};

static const sha1 IOS36Hashes[] = {
    {0x57, 0x66, 0x72, 0x79, 0x97, 0x22, 0x05, 0x46, 0x2D, 0xA4, 0x27, 0x53, 0x5A, 0x75, 0xA9, 0x13, 0x57, 0x4F, 0x27, 0x98},
    {0xE1, 0xA7, 0x17, 0x94, 0x6E, 0xF3, 0x90, 0xD9, 0xCA, 0x11, 0x82, 0xDA, 0xE6, 0xCB, 0xBB, 0x61, 0xF0, 0xAC, 0xB0, 0xCF},
    {0xAD, 0x64, 0x23, 0x8E, 0xD0, 0x18, 0x59, 0x73, 0x8E, 0xFB, 0x2C, 0xF0, 0xBA, 0xB2, 0x38, 0xED, 0x55, 0xA6, 0xD7, 0x0B},
    {0x7B, 0xC0, 0xB1, 0x55, 0x31, 0x9D, 0xF9, 0x2D, 0x31, 0xFA, 0x60, 0x63, 0xA3, 0xE9, 0x56, 0x61, 0x55, 0x2D, 0x0E, 0xFB},
    {0x61, 0x41, 0xC3, 0x6F, 0x82, 0x5A, 0x62, 0x09, 0x3B, 0x4B, 0xA2, 0x8B, 0x07, 0x05, 0x46, 0x80, 0xEC, 0xD4, 0x69, 0xA7},
    {0x80, 0xD7, 0x04, 0x96, 0xE2, 0x83, 0x4E, 0x11, 0x6C, 0x15, 0x44, 0x9E, 0xF1, 0xBE, 0x16, 0xDA, 0xA7, 0xCE, 0x62, 0x67},
    {0x41, 0xF6, 0x57, 0x42, 0x94, 0xB4, 0xB3, 0x5E, 0x6E, 0x40, 0x14, 0x35, 0xA2, 0x0C, 0xBD, 0xC0, 0x9E, 0x36, 0xF5, 0x3F}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
    {0x1D, 0x18, 0x53, 0x55, 0x81, 0x5B, 0xF6, 0x4C, 0x45, 0x71, 0x71, 0x35, 0xE6, 0x54, 0x03, 0x3B, 0xAB, 0x0C, 0x93, 0xBC},
    {0xAA, 0x6B, 0x00, 0x01, 0x47, 0xDD, 0x12, 0xC8, 0x38, 0xE4, 0x0E, 0x58, 0xB2, 0x5D, 0x3F, 0x5F, 0x51, 0x05, 0x85, 0xF8},
    {0xFF, 0x38, 0x0D, 0x01, 0x88, 0xFD, 0x07, 0x30, 0xBA, 0xF8, 0x37, 0xD4, 0x78, 0x3E, 0xA6, 0xA1, 0x8C, 0x84, 0x7B, 0x27},
    {0x9A, 0x8D, 0x1F, 0xCD, 0x38, 0xB7, 0x22, 0xCC, 0x6B, 0x6A, 0xBA, 0x3F, 0xF2, 0x12, 0x3C, 0x00, 0x7C, 0x82, 0xE5, 0x19},
    {0x43, 0x11, 0x19, 0x56, 0xA2, 0xA1, 0x27, 0x33, 0xA4, 0x9C, 0x88, 0x9E, 0x74, 0xE6, 0x85, 0xA7, 0x5E, 0xBD, 0xF1, 0x56},
    {0xD9, 0xDC, 0x9B, 0x47, 0x11, 0x13, 0xBE, 0x2E, 0x79, 0x74, 0x0F, 0x92, 0xC5, 0xCB, 0x37, 0x8B, 0x34, 0x4A, 0xBE, 0x3C},
};

static const sha1 IOS58Hashes[] = {
    {0xC6, 0x1B, 0xC1, 0x4A, 0xCF, 0xF2, 0x38, 0x7C, 0xCF, 0x1F, 0xCE, 0x7A, 0x0A, 0xAA, 0xC2, 0xC7, 0x0E, 0x91, 0xC4, 0x2B},
    {0x4E, 0x04, 0xE8, 0x8E, 0xC7, 0x25, 0x0D, 0xE8, 0x4A, 0x1E, 0x78, 0x8A, 0xE6, 0x9F, 0xDA, 0xD9, 0x35, 0x13, 0x30, 0xA8},
    {0x3B, 0xC9, 0xE4, 0x16, 0xD3, 0xFA, 0x86, 0xD0, 0xA0, 0x99, 0xB8, 0x2E, 0xCE, 0x0A, 0xA7, 0xEF, 0xF8, 0xF5, 0x9A, 0x68},
    {0x0B, 0xC6, 0x31, 0xBE, 0xD4, 0x20, 0x5F, 0x18, 0xA0, 0x05, 0x95, 0x0F, 0xC7, 0x46, 0xA5, 0x3C, 0xD4, 0xE7, 0x30, 0x7B},
    {0xEB, 0x3D, 0xBE, 0xA9, 0xDC, 0xBF, 0x57, 0xE4, 0x46, 0x35, 0x4A, 0xE2, 0xBD, 0xE1, 0xD8, 0x8C, 0x65, 0x6C, 0x90, 0xA7},
    {0x00, 0xEF, 0x2F, 0x8B, 0xBC, 0xD2, 0x08, 0xEB, 0x5E, 0x64, 0xCD, 0x91, 0x65, 0x54, 0x40, 0x5A, 0xD3, 0x4E, 0xCF, 0xD5},
    {0xE4, 0x73, 0xFD, 0xA0, 0x35, 0x15, 0x12, 0x40, 0xD6, 0xA6, 0xF5, 0x3C, 0x50, 0x36, 0x9D, 0x24, 0x14, 0xC3, 0x6F, 0x4D},
    {0x79, 0xA9, 0xF2, 0x15, 0xEA, 0x9B, 0x38, 0xFD, 0x08, 0x5D, 0xA3, 0x27, 0x20, 0xA7, 0xD3, 0x98, 0x18, 0xC8, 0x70, 0x1D}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
    {0x1D, 0x18, 0x53, 0x55, 0x81, 0x5B, 0xF6, 0x4C, 0x45, 0x71, 0x71, 0x35, 0xE6, 0x54, 0x03, 0x3B, 0xAB, 0x0C, 0x93, 0xBC},
    {0xFF, 0x38, 0x0D, 0x01, 0x88, 0xFD, 0x07, 0x30, 0xBA, 0xF8, 0x37, 0xD4, 0x78, 0x3E, 0xA6, 0xA1, 0x8C, 0x84, 0x7B, 0x27},
    {0x51, 0x37, 0xD0, 0x39, 0x3C, 0x98, 0x09, 0x7F, 0x0D, 0x59, 0x70, 0x6D, 0x1C, 0xEE, 0xC2, 0x2F, 0x75, 0x48, 0xD6, 0x0C},
    {0x7B, 0x17, 0x97, 0x6F, 0xC2, 0x0D, 0x77, 0xDA, 0x12, 0xCA, 0xE4, 0x36, 0xAD, 0x72, 0xFE, 0x8D, 0x12, 0x1A, 0xB4, 0xD2},
    {0x5D, 0x08, 0x25, 0xCA, 0xDA, 0xB0, 0x73, 0xA6, 0x59, 0x44, 0x49, 0xCB, 0x37, 0x0C, 0x66, 0xD1, 0x53, 0xCA, 0x11, 0xC2},
    {0x8A, 0x14, 0x37, 0x5F, 0x68, 0x90, 0x95, 0xC2, 0x79, 0xAE, 0xE9, 0x0E, 0xBC, 0xE8, 0xB3, 0x7D, 0x0C, 0x39, 0xE4, 0x67},
    {0xD3, 0xC2, 0x3A, 0x17, 0x2A, 0xFF, 0x83, 0x78, 0x05, 0xF4, 0xE6, 0xB2, 0xDB, 0x13, 0x72, 0xAE, 0x93, 0x6C, 0xE4, 0xED},
    {0xCC, 0x77, 0x66, 0x39, 0x56, 0x2B, 0xA2, 0x98, 0xD6, 0x65, 0x98, 0xEF, 0xE0, 0x34, 0x0B, 0x42, 0x55, 0xBF, 0x4B, 0x6C},
    {0x2D, 0x02, 0x39, 0xD0, 0x71, 0x80, 0xCA, 0x98, 0xBF, 0x51, 0xF5, 0xF2, 0xDE, 0x1A, 0xA9, 0xD1, 0x3E, 0x96, 0xE5, 0xA8},
};

static const sha1 IOS80Hashes[] = {
    {0x4E, 0x04, 0xE8, 0x8E, 0xC7, 0x25, 0x0D, 0xE8, 0x4A, 0x1E, 0x78, 0x8A, 0xE6, 0x9F, 0xDA, 0xD9, 0x35, 0x13, 0x30, 0xA8},
    {0xE1, 0xA7, 0x17, 0x94, 0x6E, 0xF3, 0x90, 0xD9, 0xCA, 0x11, 0x82, 0xDA, 0xE6, 0xCB, 0xBB, 0x61, 0xF0, 0xAC, 0xB0, 0xCF},
    {0x3B, 0xC9, 0xE4, 0x16, 0xD3, 0xFA, 0x86, 0xD0, 0xA0, 0x99, 0xB8, 0x2E, 0xCE, 0x0A, 0xA7, 0xEF, 0xF8, 0xF5, 0x9A, 0x68},
    {0xEB, 0x3D, 0xBE, 0xA9, 0xDC, 0xBF, 0x57, 0xE4, 0x46, 0x35, 0x4A, 0xE2, 0xBD, 0xE1, 0xD8, 0x8C, 0x65, 0x6C, 0x90, 0xA7},
    {0x00, 0xEF, 0x2F, 0x8B, 0xBC, 0xD2, 0x08, 0xEB, 0x5E, 0x64, 0xCD, 0x91, 0x65, 0x54, 0x40, 0x5A, 0xD3, 0x4E, 0xCF, 0xD5},
    {0xE4, 0x73, 0xFD, 0xA0, 0x35, 0x15, 0x12, 0x40, 0xD6, 0xA6, 0xF5, 0x3C, 0x50, 0x36, 0x9D, 0x24, 0x14, 0xC3, 0x6F, 0x4D},
    {0x41, 0xF6, 0x57, 0x42, 0x94, 0xB4, 0xB3, 0x5E, 0x6E, 0x40, 0x14, 0x35, 0xA2, 0x0C, 0xBD, 0xC0, 0x9E, 0x36, 0xF5, 0x3F}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
    {0x1D, 0x18, 0x53, 0x55, 0x81, 0x5B, 0xF6, 0x4C, 0x45, 0x71, 0x71, 0x35, 0xE6, 0x54, 0x03, 0x3B, 0xAB, 0x0C, 0x93, 0xBC},
    {0xAA, 0x6B, 0x00, 0x01, 0x47, 0xDD, 0x12, 0xC8, 0x38, 0xE4, 0x0E, 0x58, 0xB2, 0x5D, 0x3F, 0x5F, 0x51, 0x05, 0x85, 0xF8},
    {0xFF, 0x38, 0x0D, 0x01, 0x88, 0xFD, 0x07, 0x30, 0xBA, 0xF8, 0x37, 0xD4, 0x78, 0x3E, 0xA6, 0xA1, 0x8C, 0x84, 0x7B, 0x27},
    {0x9A, 0x8D, 0x1F, 0xCD, 0x38, 0xB7, 0x22, 0xCC, 0x6B, 0x6A, 0xBA, 0x3F, 0xF2, 0x12, 0x3C, 0x00, 0x7C, 0x82, 0xE5, 0x19},
    {0x51, 0x37, 0xD0, 0x39, 0x3C, 0x98, 0x09, 0x7F, 0x0D, 0x59, 0x70, 0x6D, 0x1C, 0xEE, 0xC2, 0x2F, 0x75, 0x48, 0xD6, 0x0C},
    {0xA7, 0x99, 0x9A, 0x25, 0x99, 0x56, 0x06, 0xCC, 0x76, 0xEF, 0x8C, 0x1E, 0xC6, 0x5E, 0xF9, 0x13, 0x8A, 0x67, 0x2F, 0xCF},
};

static const sha1 IOS36WiFiHashes[] = {
    {0x41, 0xF6, 0x57, 0x42, 0x94, 0xB4, 0xB3, 0x5E, 0x6E, 0x40, 0x14, 0x35, 0xA2, 0x0C, 0xBD, 0xC0, 0x9E, 0x36, 0xF5, 0x3F}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
};

static const sha1 IOS58WiFiHashes[] = {
    {0x79, 0xA9, 0xF2, 0x15, 0xEA, 0x9B, 0x38, 0xFD, 0x08, 0x5D, 0xA3, 0x27, 0x20, 0xA7, 0xD3, 0x98, 0x18, 0xC8, 0x70, 0x1D}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
};

static const sha1 IOS80WiFiHashes[] = {
    {0x41, 0xF6, 0x57, 0x42, 0x94, 0xB4, 0xB3, 0x5E, 0x6E, 0x40, 0x14, 0x35, 0xA2, 0x0C, 0xBD, 0xC0, 0x9E, 0x36, 0xF5, 0x3F}, //WD
    {0xD1, 0xF6, 0x0D, 0x86, 0x43, 0xC8, 0xDC, 0xB4, 0x72, 0xD8, 0x41, 0x5A, 0x2E, 0x45, 0x8A, 0x64, 0xBC, 0x5D, 0x14, 0xDC}, //WL
};

static const u16 IOSBootIndex[3] = {0x0D, 0x11, 0x0D};
static const u32 IOSNModules[3] = {sizeof(IOS36Hashes) / 20, sizeof(IOS58Hashes) / 20, sizeof(IOS80Hashes) / 20};
static const sha1* IOSHashes[3] = {IOS36Hashes, IOS58Hashes, IOS80Hashes};
static const sha1* IOSWiFiHashes[3] = {IOS36WiFiHashes, IOS58WiFiHashes, IOS80WiFiHashes};
static const char* IOSWADPaths[3] = {IOS36_WAD_PATH, IOS58_WAD_PATH, IOS80_WAD_PATH};
static WAD IOSWads[3];



void zero_sig(signed_blob *sig) {
    u8 *sig_ptr = (u8 *)sig;
    memset(sig_ptr + 4, 0, SIGNATURE_SIZE(sig)-4);
}

void brute_tmd(tmd *p_tmd) {
    u16 fill;
    for(fill=0; fill<65535; fill++) {
        p_tmd->fill3=fill;
        sha1 hash;
        SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);;

        if (hash[0]==0) {
            return;
        }
    }
    printf("Unable to fix tmd :(\n");
    exit(4);
}

void brute_tik(tik *p_tik) {
    u16 fill;
    for(fill=0; fill<65535; fill++) {
        p_tik->padding=fill;
        sha1 hash;
        SHA1((u8 *)p_tik, sizeof(tik), hash);

        if (hash[0]==0) return;
    }
    printf("Unable to fix tik :(\n");
    exit(5);
}

void forge_tmd(signed_blob *s_tmd) {
    printf("forging tmd sig\n");
    zero_sig(s_tmd);
    tmd* myTMD = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
    if ((myTMD->title_id & 0xFF) == 58) {
        //Makes IOS 58 as recognizable as valid to be loaded from homebrews
        memset((u8*)s_tmd + 8, 0xFF, 4);
    }
    brute_tmd((tmd*)SIGNATURE_PAYLOAD(s_tmd));
}

void forge_tik(signed_blob *s_tik) {
    printf("forging tik sig\n");
    zero_sig(s_tik);
    brute_tik((tik*)SIGNATURE_PAYLOAD(s_tik));
}

static u8 encrypt_iv[16];
void set_encrypt_iv(u16 index) {
    memset(encrypt_iv, 0, 16);
    memcpy(encrypt_iv, &index, 2);
}

void encrypt_buffer(u8 *source, u8 *dest, u32 len) {
    aes_encrypt(encrypt_iv, source, dest, len);
}

int patch_version_check(u8 *buf, u32 size) {
    u32 match_count = 0;
    u8 version_check[] = { 0xD2, 0x01, 0x4E, 0x56 };
    u32 i;

    for(i = 0; i < size - 4; i++) {
        if(!memcmp(buf + i, version_check, sizeof version_check)) {
            buf[i] = 0xE0;
            buf[i+3] = 0;
            i += 4;
            match_count++;
            continue;
        }
    }
    return match_count;
}

int patch_hash_check(u8 *buf, u32 size) {
    u32 i;
    u32 match_count = 0;
    u8 new_hash_check[] = {0x20,0x07,0x4B,0x0B};
    u8 old_hash_check[] = {0x20,0x07,0x23,0xA2};

    for (i=0; i<size-4; i++) {
        if (!memcmp(buf + i, new_hash_check, sizeof new_hash_check)) {
            buf[i+1] = 0;
            i += 4;
            match_count++;
            continue;
        }

        if (!memcmp(buf + i, old_hash_check, sizeof old_hash_check)) {
            buf[i+1] = 0;
            i += 4;
            match_count++;
            continue;
        }
    }
    return match_count;
}

int patch_identify_check(u8 *buf, u32 size) {
    u32 match_count = 0;
    u8 identify_check[] = { 0x28, 0x03, 0xD1, 0x23 };
    u32 i;

    for(i = 0; i < size - 4; i++)
    {
        if(!memcmp(buf + i, identify_check, sizeof identify_check))
        {
            buf[i+2] = 0;
            buf[i+3] = 0;
            i += 4;
            match_count++;
            continue;
        }
    }
    return match_count;
}

int patch_patch_fsperms(u8 *buf, u32 size) {
    u32 i;
    u32 match_count = 0;
    u8 old_table[] = {0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66};
    u8 new_table[] = {0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66};

    for (i=0; i<size-sizeof old_table; i++) {
        if (!memcmp(buf + i, old_table, sizeof old_table)) {
            memcpy(buf + i, new_table, sizeof new_table);
            i += sizeof new_table;
            match_count++;
            continue;
        }

    }
    return match_count;
}

s32 installIOSWAD(WAD* IOSWad) {
    s32 ret;
    s32 cfd;

    printf("Installing ticket...\n");
    if (((u8*)(IOSWad->tik))[0x1F1] > 1)
        ((u8*)(IOSWad->tik))[0x1F1] = 0x00; /* -1029 fix */
    ret = ES_AddTicket((signed_blob*)IOSWad->tik, STD_SIGNED_TIK_SIZE, (signed_blob*)IOSWad->certs, IOSWad->header.certSize, NULL, 0);

    if(ret < 0) {
        printf("ES_AddTicket failed: %d\n",ret);
        ES_AddTitleCancel();
        return ret;
    }

    ret = ES_AddTitleStart((signed_blob*)IOSWad->tmd, IOSWad->header.tmdSize, IOSWad->certs, IOSWad->header.certSize, NULL, 0);
    if (ret < 0) {
        printf("\nES_AddTitleStart returned: %d\n", ret);
        ES_AddTitleCancel();
        return ret;
    }

    for(u16 i = 0; i < IOSWad->tmd->ContentCount; i++) {
        printf("Adding content ID %08x", IOSWad->tmd->Contents[i].ID);
        Debug("Adding content ID %08x\n", IOSWad->tmd->Contents[i].ID);
        u32 bufSize = (IOSWad->tmd->Contents[i].Size + 0x3F) & ~0x3F;
        u8* tempBuffer = (u8*)memalign(0x20, bufSize);
        set_encrypt_iv(IOSWad->tmd->Contents[i].Index);
        encrypt_buffer(IOSWad->data[i], tempBuffer, IOSWad->tmd->Contents[i].Size);
        cfd = ES_AddContentStart(IOSWad->tmd->TitleID, IOSWad->tmd->Contents[i].ID);
        if (cfd < 0) {
            free(tempBuffer);
            printf("\nES_AddContentStart for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, cfd);
            Debug("\nES_AddContentStart for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, cfd);
            ES_AddTitleCancel();
            return cfd;
        }

        ret = ES_AddContentData(cfd, tempBuffer, bufSize);
        if (ret < 0) {
            free(tempBuffer);
            printf("\nES_AddContentData for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, ret);
            Debug("\nES_AddContentData for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, ret);
            ES_AddTitleCancel();
            return ret;
        }

        ret = ES_AddContentFinish(cfd);
        if (ret < 0) {
            free(tempBuffer);
            printf("\nES_AddContentFinish for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, ret);
            Debug("\nES_AddContentFinish for content #%u cid %u returned: %d\n", i, IOSWad->tmd->Contents[i].ID, ret);
            ES_AddTitleCancel();
            return ret;
        }
        free(tempBuffer);
        printf("\n");
    }

    ret = ES_AddTitleFinish();
    if(ret < 0) {
        printf("ES_AddTitleFinish failed: %d\n",ret);
        Debug("ES_AddTitleFinish failed: %d\n",ret);
        ES_AddTitleCancel();
        return ret;
    }

    printf("Installation complete!\n");
    Debug("Installation complete!\n");
    return 0;

}

void change_ticket_title_id(signed_blob *s_tik, u32 titleid1, u32 titleid2) {
    static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);

    tik *p_tik;
    p_tik = (tik*)SIGNATURE_PAYLOAD(s_tik);
    u8 *enc_key = (u8 *)&p_tik->cipher_title_key;
    memcpy(keyin, enc_key, sizeof keyin);
    memset(keyout, 0, sizeof keyout);
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    aes_set_key(commonkey);
    aes_decrypt(iv, keyin, keyout, sizeof keyin);
    p_tik->titleid = (u64)titleid1 << 32 | (u64)titleid2;
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    aes_encrypt(iv, keyout, keyin, sizeof keyout);
    memcpy(enc_key, keyin, sizeof keyin);
}

void change_tmd_title_id(signed_blob *s_tmd, u32 titleid1, u32 titleid2) {
    tmd *p_tmd;
    u64 title_id = titleid1;
    title_id <<= 32;
    title_id |= titleid2;
    p_tmd = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
    p_tmd->title_id = title_id;
}

void loadIOSModules() {
    char path[256] ALIGNED(32);
    char Entry[0x1C] ALIGNED(32);

    //Sanity check
    int cfd = IOS_Open("/shared1/content.map", 1);
    if (cfd < 0) {
        return;
    }

    u32 match[3] = {0, 0, 0};
    int iosSlotList[3] = {36, 58, 80};
    while (1) {
        if (IOS_Read(cfd, Entry, 0x1C) != 0x1C) //EOF
            break;

        for (u32 slot = 0; slot < 3; slot++) {
            for (u32 i = 0; i < IOSNModules[slot]; i++) {
                if (!memcmp((char*)(Entry+8), IOSHashes[slot][i], 0x14))
                    match[slot]++;
            }
        }
    }
    IOS_Close(cfd);

    //Look for WADs if needed or build them from installed modules
    for (u32 slot = 0; slot < 3; slot++) {
        if (match[slot] != IOSNModules[slot]) {
            printf("Couldn't find all modules for IOS%u. Checking for WAD...\n", iosSlotList[slot]);
            FILE* fp = fopen(IOSWADPaths[slot], "rb");
            if (!fp) {
                printf("Couldn't find IOS%u WAD at %s\n", iosSlotList[slot], IOSWADPaths[slot]);
                udelay(3000000);
                exit(0);
            }
            fclose(fp);
            if (openWAD(IOSWADPaths[slot], &IOSWads[slot])) {
                printf("Successfully loaded IOS%u WAD\n", iosSlotList[slot]);
            } else {
                printf("Failed to load IOS%u WAD at %s\n", iosSlotList[slot], IOSWADPaths[slot]);
                udelay(3000000);
                exit(0);
            }
        } else {
            printf("Found all the needed IOS%u modules in the NAND\n", iosSlotList[slot]);
            Debug("Found all the needed IOS%u modules in the NAND\n", iosSlotList[slot]);
            //Build a virtual IOS WAD from NAND
            IOSWads[slot].header.certSize = haxx_certs_size;
            IOSWads[slot].certs = (signed_blob*)haxx_certs;
            sprintf(path, "/ticket/00000001/%08x.tik", iosSlotList[slot]);
            IOSWads[slot].tik = (Ticket*)readNANDFile(path, &IOSWads[slot].header.tikSize);
            sprintf(path, "/title/00000001/%08x/content/title.tmd", iosSlotList[slot]);
            TitleMetaData* temp = (TitleMetaData*)readNANDFile(path, &IOSWads[slot].header.tmdSize);

            //Forge tmd
            u32 tmdSize = sizeof(TitleMetaData) + sizeof(Content) * (IOSNModules[slot]);
            IOSWads[slot].header.tmdSize = tmdSize;
            IOSWads[slot].tmd = (TitleMetaData*)memalign(0x20, tmdSize);
            memcpy(IOSWads[slot].tmd, temp, sizeof(TitleMetaData));
            free(temp);
            IOSWads[slot].tmd->BootIndex = IOSBootIndex[slot];
            IOSWads[slot].tmd->ContentCount = IOSNModules[slot];
            IOSWads[slot].data = (u8**)memalign(0x20, (IOSNModules[slot]) * sizeof(u8*));

            //Read the shared modules
            IOSWads[slot].header.dataSize = 0;
            for (u32 i = 0; i < IOSNModules[slot]; i++) {
                u32 tempSize;
                IOSWads[slot].data[i] = readSharedContent(IOSHashes[slot][i], &tempSize);
                IOSWads[slot].tmd->Contents[i].ID = i;
                IOSWads[slot].tmd->Contents[i].Index = i;
                IOSWads[slot].tmd->Contents[i].Type = CONTENT_REQUIRED | CONTENT_SHARED;
                IOSWads[slot].tmd->Contents[i].Size = tempSize;
                memcpy(IOSWads[slot].tmd->Contents[i].SHA1, IOSHashes[slot][i], 0x14);
                IOSWads[slot].header.dataSize += tempSize;
            }
        }
    }
}

void injectNoWiFi(WAD* wad) {
    if (wad == NULL)
        return;

    u8 slot = wad->tmd->TitleID;
    switch (slot) {
        case 36:
            slot = 0;
        break;

        case 58:
            slot = 1;
        break;

        case 80:
            slot = 2;
        break;

        default:
            slot = 0;
        break;
    }

    //Forge tmd
    u32 tmdSize = sizeof(TitleMetaData) + sizeof(Content) * (wad->tmd->ContentCount - 1);
    wad->header.tmdSize = tmdSize;
    TitleMetaData* tempTmd = (TitleMetaData*)memalign(0x20, tmdSize);
    memcpy(tempTmd, wad->tmd, sizeof(TitleMetaData));
    tempTmd->ContentCount--; //Remove stock WD and WL, add custom WD
    u8** tempData = (u8**)memalign(0x20, tempTmd->ContentCount * sizeof(u8*));
    u32 contentIdx = 0;
    u32 maxCid = 0;

    //Copy all modules but WD and WL
    for (u32 i = 0; i < wad->tmd->ContentCount; i++) {
        if (!memcmp(wad->tmd->Contents[i].SHA1, IOSWiFiHashes[slot][0], 0x14) ||
            !memcmp(wad->tmd->Contents[i].SHA1, IOSWiFiHashes[slot][1], 0x14)) {
            if (i < wad->tmd->BootIndex)
                tempTmd->BootIndex--;
            free(wad->data[i]);
            continue;
        }

        if (wad->tmd->Contents[i].ID > maxCid)
            maxCid = wad->tmd->Contents[i].ID;

        tempData[contentIdx] = wad->data[i];
        tempTmd->Contents[contentIdx].ID = wad->tmd->Contents[i].ID;
        tempTmd->Contents[contentIdx].Index = contentIdx;
        tempTmd->Contents[contentIdx].Type = wad->tmd->Contents[i].Type;
        tempTmd->Contents[contentIdx].Size = wad->tmd->Contents[i].Size;
        memcpy(tempTmd->Contents[contentIdx].SHA1, wad->tmd->Contents[i].SHA1, 0x14);
        contentIdx++;
    }

    //Add custom WD
    tempData[contentIdx] = (u8*)memalign(0x20, (wd_bin_size + 0x3F) & ~0x3F);
    memcpy(tempData[contentIdx], wd_bin, wd_bin_size);
    tempTmd->Contents[contentIdx].ID = maxCid + 1;
    tempTmd->Contents[contentIdx].Index = contentIdx;
    tempTmd->Contents[contentIdx].Type = CONTENT_REQUIRED | CONTENT_SHARED;
    tempTmd->Contents[contentIdx].Size = wd_bin_size;
    SHA1((u8*)wd_bin, wd_bin_size, tempTmd->Contents[contentIdx].SHA1);

    free(wad->tmd);
    free(wad->data);
    wad->tmd = tempTmd;
    wad->data = tempData;
}

void installIOS(u8 slot, u8 newSlot, bool patches, bool nowifi) {
    printf("Installing IOS%u as IOS%u\n", slot, newSlot);
    switch (slot) {
        case 36:
            slot = 0;
        break;

        case 58:
            slot = 1;
        break;

        case 80:
            slot = 2;
        break;

        default:
            slot = 0;
        break;
    }

    WAD wad;
    copyWAD(&wad, &IOSWads[slot]);

    if (patches) {
        printf("Injecting patches\n");
        patch_hash_check(wad.data[wad.tmd->BootIndex], wad.tmd->Contents[wad.tmd->BootIndex].Size);
        patch_identify_check(wad.data[wad.tmd->BootIndex], wad.tmd->Contents[wad.tmd->BootIndex].Size);
        patch_patch_fsperms(wad.data[wad.tmd->BootIndex], wad.tmd->Contents[wad.tmd->BootIndex].Size);
        patch_version_check(wad.data[wad.tmd->BootIndex], wad.tmd->Contents[wad.tmd->BootIndex].Size);
        SHA1(wad.data[wad.tmd->BootIndex], wad.tmd->Contents[wad.tmd->BootIndex].Size, wad.tmd->Contents[wad.tmd->BootIndex].SHA1);
    }

    if (nowifi) {
        printf("Injecting NoWiFi\n");
        injectNoWiFi(&wad);
    }

    change_ticket_title_id((signed_blob*)wad.tik, 0x00000001, newSlot);
    change_tmd_title_id((signed_blob*)wad.tmd, 0x00000001, newSlot);

    //Force max version to avoid error -1035
    wad.tmd->TitleVersion = 0xFFFF;

    forge_tik((signed_blob*)wad.tik);
    forge_tmd((signed_blob*)wad.tmd);

    u8 key[16];
    get_title_key((signed_blob*)wad.tik, key);
    aes_set_key(key);

    installIOSWAD(&wad);

    freeWAD(&wad);
}
