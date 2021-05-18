#pragma once

#include <string>
#include <gccore.h>

#define ALIGNED(x) __attribute__((aligned(x)))

enum ContentType
{
    CONTENT_REQUIRED    =    (1<< 0),    // not sure
    CONTENT_SHARED      =    (1<<15),
    CONTENT_OPTIONAL    =    (1<<14),
};

typedef struct
{
    u32 ID;                //    0    (0x1E4)
    u16 Index;            //    4    (0x1E8)
    u16 Type;            //    6    (0x1EA)
    u64 Size;            //    8    (0x1EC)
    u8    SHA1[20];        //  12    (0x1F4)
} __attribute__((packed)) Content;

typedef struct
{
    u32     SignatureType;        // 0x000
    u8      Signature[0x100];    // 0x004

    u8      Padding0[0x3C];        // 0x104
    u8      Issuer[0x40];        // 0x140

    u8      Version;            // 0x180
    u8      CACRLVersion;        // 0x181
    u8      SignerCRLVersion;    // 0x182
    u8      Padding1;            // 0x183

    u64     SystemVersion;        // 0x184
    u64     TitleID;            // 0x18C
    u32     TitleType;            // 0x194
    u16     GroupID;            // 0x198
    u8      Reserved[62];        // 0x19A
    u32     AccessRights;        // 0x1D8
    u16     TitleVersion;        // 0x1DC
    u16     ContentCount;        // 0x1DE
    u16     BootIndex;            // 0x1E0
    u8      Padding3[2];        // 0x1E2

    Content Contents[];        // 0x1E4

} __attribute__((packed)) TitleMetaData;

typedef struct {
    u32 SignatureType;        // 0x000
    u8 Signature[0x100];    // 0x004

    u8 Padding0[0x3C];        // 0x104
    u8 Issuer[0x40];        // 0x140
    u8 fill[63]; //TODO: not really fill
    aeskey cipher_title_key;
    u8 fill2;
    u64 ticketid;
    u32 devicetype;
    u64 titleid;
    u16 access_mask;
    u8 reserved[0x3c];
    u8 cidx_mask[0x40];
    u16 padding;
    tiklimit limits[8];
} __attribute__((packed)) Ticket;

typedef struct {
    u32 headerSize;
    u32 type;
    u32 certSize;
    u32 crlSize;
    u32 tikSize;
    u32 tmdSize;
    u32 dataSize;
    u32 footerSize;
} WAD_HEADER;

typedef struct {
    WAD_HEADER header;
    signed_blob* certs;
    Ticket* tik;
    TitleMetaData* tmd;
    u8** data;
} WAD;

#ifdef __cplusplus
extern "C" {
#endif
    void aes_set_key(u8 *key);
    void aes_decrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);
    void aes_encrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);
#ifdef __cplusplus
}
#endif

void get_title_key(signed_blob *s_tik, u8 *key);
bool openWAD(std::string filepath, WAD* wad);
u8* readNANDFile(const char* path, u32* size);
u8* readSharedContent(const sha1 hash, u32* size);
void copyWAD(WAD* dst, WAD* src);
void freeWAD(WAD* wad);
