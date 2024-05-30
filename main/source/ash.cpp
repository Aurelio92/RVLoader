// decompiled and slightly modified ASH v0.1 by crediar - taken from https://github.com/PMArkive/ASH_Extractor/blob/master/original%20source%20from%20crediar/main.cpp

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <gccore.h>
#include <ogc/machine/processor.h>

u8* ashExtract()
{    
    u32 size = 0xBD60; // Size of the ASH file to be decoded: real size is 0xBD48 but needs to be 32-byte aligned thus 0xBD60
    s32 in = -1;
    u32 address;
    u8* ashData = (u8*)memalign(32, size);
    
    ISFS_Initialize();
    
    switch(CONF_GetRegion())
    {
        case CONF_REGION_JP: // System Menu Japan (has to be 4.3J)
            printf("Region: CONF_REGION_JP\n");
            in = ISFS_Open("/title/00000001/00000002/content/00000094.app", 1);
            address = 0x18D920;
            break;
        case CONF_REGION_US: // System Menu US (has to be 4.3U)
            printf("Region: CONF_REGION_US\n");
            in = ISFS_Open("/title/00000001/00000002/content/00000097.app", 1);
            address = 0x19F140;
            break;
        case CONF_REGION_EU: // System Menu EU (has to be 4.3E)
            printf("Region: CONF_REGION_EU\n");
            in = ISFS_Open("/title/00000001/00000002/content/0000009a.app", 1);
            address = 0x2BFD20;
            break;
        case CONF_REGION_KR: // System Menu KOR (has to be 4.3K)
            printf("Region: CONF_REGION_KR\n");
            in = ISFS_Open("/title/00000001/00000002/content/0000009d.app", 1);
            address = 0x12AAC0;
            break;
    }
            
    if (in < 0){
        ISFS_Deinitialize();
        return 0;
    }
    
    ISFS_Seek(in, address, SEEK_SET); // Offset to diskBann.ash file
    int ret = ISFS_Read(in, ashData, size);
    if ((u32)ret != size || ret < 0) {
        printf("Couldn't read %u bytes, read %d\n", size, ret);
        ISFS_Close(in);
        ISFS_Deinitialize();
        free(ashData);
        return NULL;
    }
    DCInvalidateRange(ashData, size);

    ISFS_Close(in);
    ISFS_Deinitialize();

    unsigned int r[32];
    unsigned int count=0;
    unsigned int t;

    r[4] = (unsigned int)ashData;

    r[5] = 0x415348;
    r[6] = 0x415348;
    r[5] = *(unsigned int *)(r[4]+4);
    r[5] = r[5] & 0x00FFFFFF;

    size = r[5];
    unsigned int o = r[3] = (unsigned int)memalign(32, size);    //out
    memset( (void*)(r[3]), 0, size );

    r[24] = 0x10;
    r[28] = *(unsigned int *)(r[4]+8);
    r[25] = 0;
    r[29] = 0;
    r[26] = *(unsigned int *)(r[4]+0xC);
    r[30] = *(unsigned int *)(r[4]+r[28]);
    r[28] = r[28] + 4;
    //HACK, pointer to RAM
    r[8]  = (unsigned int)malloc(0x100000);
    memset( (void*)(r[8]), 0, 0x100000);

    r[8]  = r[8];
    r[9]  = r[8]  + 0x07FE;
    r[10] = r[9]  + 0x07FE;
    r[11] = r[10] + 0x1FFE;
    r[31] = r[11] + 0x1FFE;
    r[23] = 0x200;
    r[22] = 0x200;
    r[27] = 0;

loc_81332124:
    
    if( r[25] != 0x1F )
        goto loc_81332140;

    r[0] = r[26] >> 31;
    r[26]= *(unsigned int *)(r[4] + r[24]); 
    r[25]= 0;
    r[24]= r[24] + 4;
    goto loc_8133214C;

loc_81332140:

    r[0] = r[26] >> 31;
    r[25]= r[25] + 1;
    r[26]= r[26] << 1;

loc_8133214C:

    if( r[0] == 0 )
        goto loc_81332174;

    r[0] = r[23] | 0x8000;
    *(unsigned short *)(r[31]) = r[0];
    r[0] = r[23] | 0x4000;
    *(unsigned short *)(r[31]+2) = r[0];

    r[31] = r[31] + 4;
    r[27] = r[27] + 2;
    r[23] = r[23] + 1;
    r[22] = r[22] + 1;

    goto loc_81332124;

loc_81332174:

    r[12] = 9;
    r[21] = r[25] + r[12];
    t = r[21];
    if( r[21] > 0x20 )
        goto loc_813321AC;

    r[21] = (~(r[12] - 0x20))+1;
    r[6]  = r[26] >> r[21];
    if( t == 0x20 )
        goto loc_8133219C;

    r[26] = r[26] << r[12];
    r[25] = r[25] +  r[12];
    goto loc_813321D0;

loc_8133219C:

    r[26]= *(unsigned int *)(r[4] + r[24]);
    r[25]= 0;
    r[24]= r[24] + 4;
    goto loc_813321D0;

loc_813321AC:

    r[0] = (~(r[12] - 0x20))+1;
    r[6] = r[26] >> r[0];
    r[26]= *(unsigned int *)(r[4] + r[24]);
    r[0] = (~(r[21] - 0x40))+1;
    r[24]= r[24] + 4;
    r[0] = r[26] >> r[0];
    r[6] = r[6] | r[0];
    r[25] = r[21] - 0x20;
    r[26] = r[26] << r[25];

loc_813321D0:

    r[12]= *(unsigned short *)(r[31] - 2);
        r[31] -= 2;
    r[27]= r[27] - 1;
    r[0] = r[12] & 0x8000;
    r[12]= (r[12] & 0x1FFF) << 1;
    if( r[0] == 0 )
        goto loc_813321F8;

    *(unsigned short *)(r[9]+r[12]) = r[6];
    r[6] = (r[12] & 0x3FFF)>>1;                            //    extrwi  %r6, %r12, 14,17
    if( r[27] != 0 )
        goto loc_813321D0;

    goto loc_81332204;

loc_813321F8:

    *(unsigned short *)(r[8]+r[12]) = r[6];
    r[23] = r[22];
    goto loc_81332124;

loc_81332204:

    r[23] = 0x800;
    r[22] = 0x800;

loc_8133220C:

    if( r[29] != 0x1F )
        goto loc_81332228;

    r[0] = r[30] >> 31;
    r[30]= *(unsigned int *)(r[4] + r[28]);
    r[29]= 0;
    r[28]= r[28] + 4;
    goto loc_81332234;

loc_81332228:

    r[0] = r[30] >> 31;
    r[29]= r[29] +  1;
    r[30]= r[30] << 1;

loc_81332234:

    if( r[0] == 0 )
        goto loc_8133225C;

    r[0] = r[23] | 0x8000;
    *(unsigned short *)(r[31]) = r[0];
    r[0] = r[23] | 0x4000;
    *(unsigned short *)(r[31]+2) = r[0];

    r[31] = r[31] + 4;
    r[27] = r[27] + 2;
    r[23] = r[23] + 1;
    r[22] = r[22] + 1;

    goto loc_8133220C;

loc_8133225C:

    r[12] = 0xB;
    r[21] = r[29] + r[12];
    t = r[21];
    if( r[21] > 0x20 )
        goto loc_81332294;

    r[21] = (~(r[12] - 0x20))+1;
    r[7]  = r[30] >> r[21];
    if( t == 0x20 )
        goto loc_81332284;

    r[30] = r[30] << r[12];
    r[29] = r[29] +  r[12];
    goto loc_813322B8;

loc_81332284:

    r[30]= *(unsigned int *)(r[4] + r[28]);
    r[29]= 0;
    r[28]= r[28] + 4;
    goto loc_813322B8;

loc_81332294:

    r[0] = (~(r[12] - 0x20))+1;
    r[7] = r[30] >> r[0];
    r[30]= *(unsigned int *)(r[4] + r[28]);
    r[0] = (~(r[21] - 0x40))+1;
    r[28]= r[28] + 4;
    r[0] = r[30] >> r[0];
    r[7] = r[7] | r[0];
    r[29]= r[21] - 0x20;
    r[30]= r[30] << r[29];

loc_813322B8:

    r[12]= *(unsigned short *)(r[31] - 2);
        r[31] -= 2;
    r[27]= r[27] - 1;
    r[0] = r[12] & 0x8000;
    r[12]= (r[12] & 0x1FFF) << 1;
    if( r[0] == 0 )
        goto loc_813322E0;

    *(unsigned short *)(r[11]+r[12]) = r[7];
    r[7] = (r[12] & 0x3FFF)>>1;                            // extrwi  %r7, %r12, 14,17
    if( r[27] != 0 )
        goto loc_813322B8;

    goto loc_813322EC;

loc_813322E0:
    
    *(unsigned short *)(r[10]+r[12]) = r[7];
    r[23] = r[22];
    goto loc_8133220C;

loc_813322EC:

    r[0] = r[5];

loc_813322F0:

    r[12]= r[6];

loc_813322F4:

    if( r[12] < 0x200 )
        goto loc_8133233C;

    if( r[25] != 0x1F )
        goto loc_81332318;

    r[31] = r[26] >> 31;
    r[26] = *(unsigned int *)(r[4] + r[24]);
    r[24] = r[24] + 4;
    r[25] = 0;
    goto loc_81332324;

loc_81332318:

    r[31] = r[26] >> 31;
    r[25] = r[25] +  1;
    r[26] = r[26] << 1;

loc_81332324:

    r[27] = r[12] << 1;
    if( r[31] != 0 )
        goto loc_81332334;

    r[12] = *(unsigned short *)(r[8] + r[27]);
    goto loc_813322F4;

loc_81332334:

    r[12] = *(unsigned short *)(r[9] + r[27]);
    goto loc_813322F4;

loc_8133233C:

    if( r[12] >= 0x100 )
        goto loc_8133235C;

    *(unsigned char *)(r[3]) = r[12];
    r[3] = r[3] + 1;
    r[5] = r[5] - 1;
    if( r[5] != 0 )
        goto loc_813322F0;

    goto loc_81332434;

loc_8133235C:

    r[23] = r[7];

loc_81332360:

    if( r[23] < 0x800 )
        goto loc_813323A8;

    if( r[29] != 0x1F )
        goto loc_81332384;

    r[31] = r[30] >> 31;
    r[30] = *(unsigned int *)(r[4] + r[28]);
    r[28] = r[28] + 4;
    r[29] = 0;
    goto loc_81332390;

loc_81332384:

    r[31] = r[30] >> 31;
    r[29] = r[29] +  1;
    r[30] = r[30] << 1;

loc_81332390:

    r[27] = r[23] << 1;
    if( r[31] != 0 )
        goto loc_813323A0;

    r[23] = *(unsigned short *)(r[10] + r[27]);
    goto loc_81332360;

loc_813323A0:

    r[23] = *(unsigned short *)(r[11] + r[27]);
    goto loc_81332360;

loc_813323A8:

    r[12] = r[12] - 0xFD;
    r[23] = ~r[23] + r[3] + 1;
    r[5]  = ~r[12] + r[5] + 1;
    r[31] = r[12] >> 3;

    if( r[31] == 0 )
        goto loc_81332414;

    count = r[31];

loc_813323C0:

    r[31] = *(unsigned char *)(r[23] - 1);
    *(unsigned char *)(r[3]) = r[31];

    r[31] = *(unsigned char *)(r[23]);
    *(unsigned char *)(r[3]+1) = r[31];

    r[31] = *(unsigned char *)(r[23] + 1);
    *(unsigned char *)(r[3]+2) = r[31];

    r[31] = *(unsigned char *)(r[23] + 2);
    *(unsigned char *)(r[3]+3) = r[31];

    r[31] = *(unsigned char *)(r[23] + 3);
    *(unsigned char *)(r[3]+4) = r[31];

    r[31] = *(unsigned char *)(r[23] + 4);
    *(unsigned char *)(r[3]+5) = r[31];

    r[31] = *(unsigned char *)(r[23] + 5);
    *(unsigned char *)(r[3]+6) = r[31];

    r[31] = *(unsigned char *)(r[23] + 6);
    *(unsigned char *)(r[3]+7) = r[31];

    r[23] = r[23] + 8;
    r[3]  = r[3]  + 8;

    if( --count )
        goto loc_813323C0;

    r[12] = r[12] & 7;
    if( r[12] == 0 )
        goto loc_8133242C;

loc_81332414:

    count = r[12];

loc_81332418:

    r[31] = *(unsigned char *)(r[23] - 1);
    r[23] = r[23] + 1;
    *(unsigned char *)(r[3]) = r[31];
    r[3]  = r[3] + 1;

    if( --count )
        goto loc_81332418;

loc_8133242C:

    if( r[5] != 0 )
        goto loc_813322F0;

loc_81332434:

    r[3] = r[0];
    
    free(ashData);
    free((void*)r[8]);

    return (u8*)(o);
}