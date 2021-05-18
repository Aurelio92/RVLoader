#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string>
#include "systitles.h"
#include "debug.h"
#include "haxx_certs.h"

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

void decrypt_buffer(u16 index, u8 *source, u8 *dest, u32 len) {
    static u8 iv[16];
    memset(iv, 0, 16);
    memcpy(iv, &index, 2);
    aes_decrypt(iv, source, dest, len);
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

    wad->certs = (signed_blob*)haxx_certs;
    fseek(fp, wad->header.certSize, SEEK_CUR); //Fake certs read
    wad->header.certSize = haxx_certs_size;
    fseek(fp, (ftell(fp) + 0x3F) & ~0x3F, SEEK_SET); //Handle 0x40 bytes alignment

    fseek(fp, wad->header.crlSize, SEEK_CUR); //Fake crl read
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

u8* readNANDFile(const char* path, u32* size) {
    s32 ret;
    u8* buffer = NULL;
    Debug("Opening %s\n", path);
    s32 fd = IOS_Open(path, IPC_OPEN_READ);
    if (fd < 0)
        return NULL;

    *size = IOS_Seek(fd, 0, SEEK_END);
    Debug("Size: %d\n", *size);
    IOS_Seek(fd, 0, SEEK_SET);
    buffer = (u8*)memalign(0x20, (*size + 0x3F) & ~0x3F);
    Debug("Buffer: %08X\n", (u32)buffer);
    if (buffer == NULL) {
        IOS_Close(fd);
        return NULL;
    }

    IOS_Read(fd, buffer, *size);
    IOS_Close(fd);
    Debug("Done\n");

    return buffer;
}

u8* readSharedContent(const sha1 hash, u32* size) {
    static char path[256] ALIGNED(32);
    static char entry[0x1C] ALIGNED(32);
    s32 ret;
    u8* buffer = NULL;

    Debug("readSharedContent\n");

    s32 cfd = IOS_Open("/shared1/content.map", IPC_OPEN_READ);
    if (cfd < 0) {
        return NULL;
    }

    Debug("Seeking shared content\n");

    //Seek hash
    while (1) {
        if (IOS_Read(cfd, entry, 0x1C) != 0x1C) { //EOF
            IOS_Close(cfd);
            Debug("Hash not found\n");
            return NULL;
        }

        if (!memcmp(hash, entry + 8, 0x14))
            break;
    }

    Debug("Found\n");

    sprintf(path, "/shared1/%.8s.app", entry);
    IOS_Close(cfd);
    buffer = readNANDFile(path, size);
    return buffer;
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
