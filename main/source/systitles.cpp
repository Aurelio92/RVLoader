#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string>
#include <dirent.h>
#include "systitles.h"
#include "hiidra.h"

static u8 commonkey[16] = {0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7};

void get_title_key(signed_blob *s_tik, u8 *key) {
    static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);

    const tik *p_tik;
    p_tik = (tik*)SIGNATURE_PAYLOAD(s_tik);
    u8 *enc_key = (u8 *)&p_tik->cipher_title_key;
    memcpy(keyin, enc_key, sizeof keyin);
    memset(keyout, 0, sizeof keyout);
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    aes_set_key(commonkey);
    aes_decrypt(iv, keyin, keyout, sizeof keyin);

    memcpy(key, keyout, sizeof keyout);
}

void showWADTitleInstallProgress(u32 line, u32 percentage) {
    hiidraUpdateLogLine(line, "Progress: %u%%", percentage);
}

void decrypt_buffer(u16 index, u8 *source, u8 *dest, u32 len) {
    static u8 iv[16];
    memset(iv, 0, 16);
    memcpy(iv, &index, 2);
    aes_decrypt(iv, source, dest, len);
}

void decrypt_file(u16 index, FILE* infp, FILE* outfp, u32 inLen, u32 outLen) {
    static u8 iv[16];

    memset(iv, 0, 16);
    memcpy(iv, &index, 2);
    u32 logLine = hiidraAddLogLine(""); //Create empty line for logger
    aes_decrypt_file(iv, infp, outfp, inLen, outLen, showWADTitleInstallProgress, logLine);
}

bool openWAD(std::string filepath, WAD* wad) {
    FILE* fp = fopen(filepath.c_str(), "rb");
    if (fp == NULL)
        return false;

    fread(&wad->header, 1, sizeof(WAD_HEADER), fp);
    DCFlushRange(&wad->header, sizeof(WAD_HEADER));

    if  (wad->header.headerSize != 0x20 ||
        (wad->header.type != 0x49730000 && wad->header.type != 0x69620000 && wad->header.type != 0x426b0000)) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad->certs = NULL;
    fseek(fp, wad->header.certSize, SEEK_CUR); //Skip certs read
    wad->header.certSize = 0;
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad->header.crlSize, SEEK_CUR); //Skip crl read
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad->tik = (Ticket*)memalign(0x20, wad->header.tikSize);
    fread(wad->tik, 1, wad->header.tikSize, fp);
    DCFlushRange(wad->tik, wad->header.tikSize);
    if (wad->tik == NULL) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad->tmd = (TitleMetaData*)memalign(0x20, wad->header.tmdSize);
    fread(wad->tmd, 1, wad->header.tmdSize, fp);
    DCFlushRange(wad->tmd, wad->header.tmdSize);
    if (wad->tmd == NULL) {
        free(wad->tik);
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad->data = (u8**)memalign(0x20, wad->tmd->ContentCount * sizeof(u8*));
    if (wad->data == NULL) {
        free(wad->tik);
        free(wad->tmd);
        fclose(fp);
        return false;
    }

    u8 key[16];
    get_title_key((signed_blob*)wad->tik, key);
    aes_set_key(key);

    //Read and decrypt all titles
    for (int i = 0; i < wad->tmd->ContentCount; i++) {
        u32 bufSize = (wad->tmd->Contents[i].Size + 0x3F) & ~0x3F;
        u8* tempData = (u8*)memalign(0x20, bufSize);
        wad->data[i] = (u8*)memalign(0x20, bufSize);
        fread(tempData, 1, bufSize, fp);
        decrypt_buffer(i, tempData, wad->data[i], bufSize);
        DCFlushRange(wad->data[i], bufSize);
        free(tempData);
    }

    fclose(fp);

    return true;
}

void copyWAD(WAD* dst, WAD* src) {
    if (src == NULL || dst == NULL)
        return;

    dst->header = src->header;
    dst->certs = (signed_blob*)memalign(0x20, dst->header.certSize);
    memcpy(dst->certs, src->certs, dst->header.certSize);
    DCFlushRange(dst->certs, dst->header.certSize);
    dst->tik = (Ticket*)memalign(0x20, dst->header.tikSize);
    memcpy(dst->tik, src->tik, dst->header.tikSize);
    DCFlushRange(dst->tik, dst->header.tikSize);
    dst->tmd = (TitleMetaData*)memalign(0x20, dst->header.tmdSize);
    memcpy(dst->tmd, src->tmd, dst->header.tmdSize);
    DCFlushRange(dst->tmd, dst->header.tmdSize);

    dst->data = (u8**)memalign(0x20, dst->tmd->ContentCount * sizeof(u8*));

    //Read and decrypt all titles
    for (int i = 0; i < dst->tmd->ContentCount; i++) {
        u32 bufSize = (dst->tmd->Contents[i].Size + 0x3F) & ~0x3F;
        dst->data[i] = (u8*)memalign(0x20, bufSize);
        memcpy(dst->data[i], src->data[i], bufSize);
        DCFlushRange(dst->data[i], bufSize);
    }
}

void installWAD(WAD* wad) {
    char path[256];
    FILE* fp;

    mkdir("/rvloader/Hiidra/emunand", 777);
    mkdir("/rvloader/Hiidra/emunand/ticket", 777);
    sprintf(path, "/rvloader/Hiidra/emunand/ticket/%08x", (u32)(wad->tmd->TitleID >> 32));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/ticket/%08x/%08x.tik", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF));
    fp = fopen(path, "wb");
    if (!fp)
        return;

    fwrite(wad->tik, 1, wad->header.tikSize, fp);
    fclose(fp);

    mkdir("/rvloader/Hiidra/emunand/title", 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x", (u32)(wad->tmd->TitleID >> 32));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/data", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);

    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content/title.tmd", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF));
    fp = fopen(path, "wb");
    if (!fp)
        return;

    fwrite(wad->tmd, 1, wad->header.tmdSize, fp);
    fclose(fp);

    for (int i = 0; i < wad->tmd->ContentCount; i++) {
        sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content/%08x.app", (u32)(wad->tmd->TitleID >> 32), (u32)(wad->tmd->TitleID & 0xFFFFFFFF), wad->tmd->Contents[i].ID);
        fp = fopen(path, "wb");
        if (!fp)
            return;
        u32 bufSize = (wad->tmd->Contents[i].Size + 0x3F) & ~0x3F;
        fwrite(wad->data[i], 1, bufSize, fp);
        fclose(fp);
    }
}

void freeWAD(WAD* wad) {
    if (wad == NULL)
        return;

    for (int i = 0; i < wad->tmd->ContentCount; i++) {
        free(wad->data[i]);
    }
    free(wad->certs);
    free(wad->tik);
    free(wad->tmd);
    free(wad->data);
}

/*void extractBannerFromWAD(std::string filepath, std::string iconPath) {
    WAD wad;
    FILE* fpOut;
    FILE* fp = fopen(filepath.c_str(), "rb");
    if (fp == NULL)
        return false;

    fread(&wad.header, 1, sizeof(WAD_HEADER), fp);
    DCFlushRange(&wad.header, sizeof(WAD_HEADER));

    if  (wad.header.headerSize != 0x20 ||
        (wad.header.type != 0x49730000 && wad.header.type != 0x69620000 && wad.header.type != 0x426b0000)) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad.header.certSize, SEEK_CUR); //Skip certs read
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad.header.crlSize, SEEK_CUR); //Skip crl read
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad.tik = (Ticket*)memalign(0x20, wad.header.tikSize);
    fread(wad.tik, 1, wad.header.tikSize, fp);
    DCFlushRange(wad.tik, wad.header.tikSize);
    if (wad.tik == NULL) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad.tmd = (TitleMetaData*)memalign(0x20, wad.header.tmdSize);
    fread(wad.tmd, 1, wad.header.tmdSize, fp);
    DCFlushRange(wad.tmd, wad.header.tmdSize);
    if (wad.tmd == NULL) {
        free(wad.tik);
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    u8 key[16];
    get_title_key((signed_blob*)wad.tik, key);
    aes_set_key(key);

    //Read and decrypt all titles
    for (int i = 0; i < wad.tmd->ContentCount; i++) {
        if (wad.tmd->Contents[i].ID == 0x00000000) {
            u32 bufSize = (wad.tmd->Contents[i].Size + 0x3F) & ~0x3F;
            u8* tempData = (u8*)memalign(0x20, bufSize);
            u8* tempDecryptedData = (u8*)memalign(0x20, bufSize);
            fread(tempData, 1, bufSize, fp);
            decrypt_buffer(i, tempData, tempDecryptedData, bufSize);
            DCFlushRange(wad.data[i], bufSize);
            free(tempData);
            free(tempDecryptedData);
            break;
        }
    }

    free(wad.tik);
    free(wad.tmd);

    fclose(fp);

    return true;
}*/

bool openAndInstallWAD(const char* filepath, u64* titleID, bool forceReinstall) {
    int i;
    WAD wad;
    char path[256];
    FILE* fpOut;
    FILE* fpTest;
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL)
        return false;

    fread(&wad.header, 1, sizeof(WAD_HEADER), fp);
    DCFlushRange(&wad.header, sizeof(WAD_HEADER));

    if  (wad.header.headerSize != 0x20 ||
        (wad.header.type != 0x49730000 && wad.header.type != 0x69620000 && wad.header.type != 0x426b0000)) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad.header.certSize, SEEK_CUR); //Skip certs read
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad.header.crlSize, SEEK_CUR); //Skip crl read
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad.tik = (Ticket*)memalign(0x20, wad.header.tikSize);
    fread(wad.tik, 1, wad.header.tikSize, fp);
    DCFlushRange(wad.tik, wad.header.tikSize);
    if (wad.tik == NULL) {
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    wad.tmd = (TitleMetaData*)memalign(0x20, wad.header.tmdSize);
    fread(wad.tmd, 1, wad.header.tmdSize, fp);
    DCFlushRange(wad.tmd, wad.header.tmdSize);
    if (wad.tmd == NULL) {
        free(wad.tik);
        fclose(fp);
        return false;
    }
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    *titleID = wad.tmd->TitleID;

    //Check if wad is already installed by reading the tmd
    if (!forceReinstall) {
        sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content/title.tmd", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
        fpTest = fopen(path, "rb");
        if (fpTest) {
            hiidraAddLogLine("WAD already installed");
            free(wad.tik);
            free(wad.tmd);
            fclose(fp);
            fclose(fpTest);
            return true;
        }
    }

    mkdir("/rvloader/Hiidra/emunand", 777);
    mkdir("/rvloader/Hiidra/emunand/ticket", 777);
    sprintf(path, "/rvloader/Hiidra/emunand/ticket/%08x", (u32)(wad.tmd->TitleID >> 32));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/ticket/%08x/%08x.tik", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
    printf("Saving ticket\n");
    hiidraAddLogLine("Saving ticket");
    fpOut = fopen(path, "wb");
    if (!fpOut) {
        hiidraAddLogLine("Could not save ticket");
        free(wad.tik);
        free(wad.tmd);
        fclose(fp);
        return false;
    }

    if (fwrite(wad.tik, 1, wad.header.tikSize, fpOut) != wad.header.tikSize) {
        hiidraAddLogLine("Could not save ticket");
    }
    fclose(fpOut);

    mkdir("/rvloader/Hiidra/emunand/title", 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x", (u32)(wad.tmd->TitleID >> 32));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);
    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/data", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
    mkdir(path, 777);

    u8 key[16];
    get_title_key((signed_blob*)wad.tik, key);
    aes_set_key(key);

    //Read and decrypt all contents
    for (i = 0; i < wad.tmd->ContentCount; i++) {
        hiidraAddLogLine("Saving title %u/%u", i + 1, wad.tmd->ContentCount);
        printf("Saving title %u/%u\n", i + 1, wad.tmd->ContentCount);
        u32 bufSize = (wad.tmd->Contents[i].Size + 0x3F) & ~0x3F;
        printf("Content size %u\n", (u32)wad.tmd->Contents[i].Size);
        printf("bufSize %u\n", bufSize);
        printf("Cur file position: %ld\n", ftell(fp));

        sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content/%08x.app", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF), wad.tmd->Contents[i].ID);
        printf("Opening for write: %s\n", path);
        fpOut = fopen(path, "wb");
        if (!fpOut) {
            printf("Failed to open\n");
            hiidraAddLogLine("Failed to save title");
            free(wad.tik);
            free(wad.tmd);
            fclose(fp);
            return false;
        }
        printf("Opened\n");
        //decrypt_file(i, fp, fpOut, wad.tmd->Contents[i].Size);
        size_t prevPos = ftell(fp);
        printf("About to decrypt %llu bytes\n", wad.tmd->Contents[i].Size);
        decrypt_file(i, fp, fpOut, bufSize, wad.tmd->Contents[i].Size);
        fclose(fpOut);
        printf("Saved\nCur file position: %ld\n", ftell(fp));
        fseek(fp, prevPos + bufSize, SEEK_SET);
        //fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment
        printf("Seeked\nCur file position: %ld\n", ftell(fp));
    }

    sprintf(path, "/rvloader/Hiidra/emunand/title/%08x/%08x/content/title.tmd", (u32)(wad.tmd->TitleID >> 32), (u32)(wad.tmd->TitleID & 0xFFFFFFFF));
    printf("Saving tmd\n");
    fpOut = fopen(path, "wb");
    if (!fpOut) {
        free(wad.tik);
        free(wad.tmd);
        fclose(fp);
        return false;
    }

    fwrite(wad.tmd, 1, wad.header.tmdSize, fpOut);
    fclose(fpOut);

    fclose(fp);
    free(wad.tmd);
    free(wad.tik);
    printf("Wad install complete\n");

    return true;
}
