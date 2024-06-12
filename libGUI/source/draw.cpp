#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "draw.h"

// texture header
struct _tplimgheader {
    u16 height;
    u16 width;
    u32 fmt;
    void *data;
    u32 wraps;
    u32 wrapt;
    u32 minfilter;
    u32 magfilter;
    f32 lodbias;
    u8 edgelod;
    u8 minlod;
    u8 maxlod;
    u8 unpacked;
} ATTRIBUTE_PACKED;

// texture palette header
struct _tplpalheader {
    u16 nitems;
    u8 unpacked;
    u8 pad;
    u32 fmt;
    void *data;
} ATTRIBUTE_PACKED;

typedef struct _tplpalheader TPLPalHeader;
typedef struct _tplimgheader TPLImgHeader;

struct _tpldesc {
    TPLImgHeader *imghead;
    TPLPalHeader *palhead;
} ATTRIBUTE_PACKED;

typedef struct _tpldesc TPLDescHeader;

Texture createTextureARGB8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.textureFormat = GX_TF_RGBA8;
    tex.width = (width + 3) & ~3;
    tex.height = (height + 3) & ~3;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u32));
    tex.palette = NULL;

    d = tex.data;
    s = tex.data;
    for (y = 0; y < tex.height; y += 4) {
        for (x = 0; x < tex.width; x += 4) {
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx * 4];
                    *d++ = s[0];  *d++ = s[1];
                    *d++ = s[4];  *d++ = s[5];
                    *d++ = s[8];  *d++ = s[9];
                    *d++ = s[12]; *d++ = s[13];
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx * 4];
                    *d++ = s[2];  *d++ = s[3];
                    *d++ = s[6];  *d++ = s[7];
                    *d++ = s[10]; *d++ = s[11];
                    *d++ = s[14]; *d++ = s[15];
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
        }
    }

    DCFlushRange(tex.data, tex.width * tex.height * sizeof(u32));
    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);

    return tex;
}

Texture createTextureRGB8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.textureFormat = GX_TF_RGBA8;
    tex.width = (width + 3) & ~3;
    tex.height = (height + 3) & ~3;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u32));
    tex.palette = NULL;

    d = tex.data;
    s = tex.data;
    for (y = 0; y < tex.height; y += 4) {
        for (x = 0; x < tex.width; x += 4) {
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx * 3];
                    *d++ = 0xFF;  *d++ = s[0];
                    *d++ = 0xFF;  *d++ = s[3];
                    *d++ = 0xFF;  *d++ = s[6];
                    *d++ = 0xFF; *d++ = s[9];
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx * 3];
                    *d++ = s[1];  *d++ = s[2];
                    *d++ = s[4];  *d++ = s[5];
                    *d++ = s[7]; *d++ = s[8];
                    *d++ = s[10]; *d++ = s[11];
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
        }
    }

    DCFlushRange(tex.data, tex.width * tex.height * sizeof(u32));
    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);

    return tex;
}

Texture createTextureA8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.textureFormat = GX_TF_RGBA8;
    tex.width = (width + 3) & ~3;
    tex.height = (height + 3) & ~3;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u32));
    tex.palette = NULL;

    d = tex.data;
    s = tex.data;
    for (y = 0; y < tex.height; y += 4) {
        for (x = 0; x < tex.width; x += 4) {
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx];
                    *d++ = s[0];  *d++ = 0xFF;
                    *d++ = s[1];  *d++ = 0xFF;
                    *d++ = s[2];  *d++ = 0xFF;
                    *d++ = s[3]; *d++ = 0xFF;
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
            for (r = 0; r < 4; ++r) {
                idx = x + (y + r) * width;
                if (idx < height * width) {
                    s = &buffer[idx];
                    *d++ = 0xFF;  *d++ = 0xFF;
                    *d++ = 0xFF;  *d++ = 0xFF;
                    *d++ = 0xFF; *d++ = 0xFF;
                    *d++ = 0xFF; *d++ = 0xFF;
                } else {
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                    *d++ = 0; *d++ = 0;
                }
            }
        }
    }

    DCFlushRange(tex.data, tex.width * tex.height * sizeof(u32));
    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);

    return tex;
}

Texture createTextureCI8(u8* textureBuffer, u8* paletteBuffer, u16 width, u16 height, u32 paletteNItems) {
    Texture tex;

    tex.textureFormat = GX_TF_CI8;
    tex.width = (width + 7) & ~7;
    tex.height = (height + 7) & ~7;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height);
    tex.palette = (u8*)memalign(32, paletteNItems * sizeof(u16));
    tex.paletteNItems = paletteNItems;

    memcpy(tex.data, textureBuffer, width * height);
    memcpy(tex.palette, paletteBuffer, paletteNItems * sizeof(u16));

    DCFlushRange(tex.data, tex.width * tex.height);
    DCFlushRange(tex.palette, paletteNItems * sizeof(u16));
    GX_InitTlutObj(&tex.tLut, tex.palette, GX_TL_RGB5A3, tex.paletteNItems);
    GX_InitTexObjCI(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE, 0);

    return tex;
}

Texture createTextureRGB5A3(u8* textureBuffer, u16 width, u16 height) {
    Texture tex;

    tex.textureFormat = GX_TF_RGB5A3;
    tex.width = (width + 3) & ~3;
    tex.height = (height + 3) & ~3;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u16));
    tex.palette = NULL;

    memcpy(tex.data, textureBuffer, width * height * sizeof(u16));

    DCFlushRange(tex.data, tex.width * tex.height * sizeof(u16));
    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);

    return tex;
}

Texture createTextureFromTPL(TPLFile *tdf, s32 id) {
    Texture tex;
    TPL_GetTextureInfo(tdf, id, NULL, &tex.realWidth, &tex.realHeight);
    tex.width = (tex.realWidth & 3) ? tex.realWidth + (4 - (tex.realWidth & 3)) : tex.realWidth;
    tex.height = (tex.realHeight & 3) ? tex.realHeight + (4 - (tex.realHeight & 3)) : tex.realHeight;
    if (!TPL_GetTexture(tdf, id, &tex.texObj)) {
        TPLDescHeader* deschead = (TPLDescHeader*)tdf->texdesc;
        TPLImgHeader* imghead = (TPLImgHeader*)deschead[id].imghead;
        tex.data = (u8*)imghead->data;
    }
    return tex;
}

Texture copyTexture(Texture src) {
    Texture tex;

    tex.width = src.width;
    tex.height = src.height;
    tex.realWidth = src.realWidth;
    tex.realHeight = src.realHeight;
    tex.textureFormat = src.textureFormat;

    if (tex.textureFormat >= GX_TF_CI4) {
        tex.data = (u8*)memalign(32, tex.width * tex.height);
        memcpy(tex.data, src.data, tex.width * tex.height);
        tex.palette = (u8*)memalign(32, src.paletteNItems * sizeof(u16));
        memcpy(tex.palette, src.palette, src.paletteNItems * sizeof(u16));
        tex.paletteNItems = src.paletteNItems;
    } else if (tex.textureFormat == GX_TF_RGB5A3) {
        tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u16));
        memcpy(tex.data, src.data, tex.width * tex.height * sizeof(u16));
        tex.palette = NULL;
    } else {
        tex.data = (u8*)memalign(32, tex.width * tex.height * sizeof(u32));
        memcpy(tex.data, src.data, tex.width * tex.height * sizeof(u32));
        tex.palette = NULL;
    }

    if (tex.textureFormat >= GX_TF_CI4) {
        DCFlushRange(tex.data, tex.width * tex.height);
        DCFlushRange(tex.palette, tex.paletteNItems * sizeof(u16));
        GX_InitTlutObj(&tex.tLut, tex.palette, GX_TL_RGB5A3, tex.paletteNItems);
        GX_InitTexObjCI(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE, 0);
    } else if (tex.textureFormat == GX_TF_RGB5A3) {
        DCFlushRange(tex.data, tex.width * tex.height * sizeof(u16));
        GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);
    } else {
        DCFlushRange(tex.data, tex.width * tex.height * sizeof(u32));
        GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, tex.textureFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);
    }

    return tex;
}

void drawTexture(int x, int y, Texture tex) {
    //GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    //GX_SetCopyFilter(GX_FALSE, screenMode.sample_pattern, GX_FALSE, screenMode.vfilter);

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(x + tex.realWidth, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, 0);

        GX_Position3f32(x + tex.realWidth, y + tex.realHeight, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, (f32)tex.realHeight / tex.height);

        GX_Position3f32(x, y + tex.realHeight, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(0, (f32)tex.realHeight / tex.height);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawTextureColor(int x, int y, u32 rgba, Texture tex) {
    //GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    //GX_SetCopyFilter(GX_FALSE, screenMode.sample_pattern, GX_FALSE, screenMode.vfilter);

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color1u32(rgba);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(x + tex.realWidth, y, 0);
        GX_Color1u32(rgba);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, 0);

        GX_Position3f32(x + tex.realWidth, y + tex.realHeight, 0);
        GX_Color1u32(rgba);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, (f32)tex.realHeight / tex.height);

        GX_Position3f32(x, y + tex.realHeight, 0);
        GX_Color1u32(rgba);
        GX_TexCoord2f32(0, (f32)tex.realHeight / tex.height);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawTextureResized(int x, int y, int width, int height, Texture tex) {
    f32 left, right, top, bottom;

    if (width < 0) {
        width = -width;
        left = (f32)tex.realWidth / tex.width;
        right = 0;
    } else {
        left = 0;
        right = (f32)tex.realWidth / tex.width;
    }

    if (height < 0) {
        height = -height;
        top = (f32)tex.realHeight / tex.height;
        bottom = 0;
    } else {
        top = 0;
        bottom = (f32)tex.realHeight / tex.height;
    }

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(left, top);

        GX_Position3f32(x + width, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(right, top);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(right, bottom);

        GX_Position3f32(x, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(left, bottom);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawTextureResizedAlpha(int x, int y, int width, int height, int alpha, Texture tex) {
    f32 left, right, top, bottom;

    if (width < 0) {
        width = -width;
        left = (f32)tex.realWidth / tex.width;
        right = 0;
    } else {
        left = 0;
        right = (f32)tex.realWidth / tex.width;
    }

    if (height < 0) {
        height = -height;
        top = (f32)tex.realHeight / tex.height;
        bottom = 0;
    } else {
        top = 0;
        bottom = (f32)tex.realHeight / tex.height;
    }

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(left, top);

        GX_Position3f32(x + width, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(right, top);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(right, bottom);

        GX_Position3f32(x, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(left, bottom);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawTextureAlphaResizeTexCoords(int x, int y, int width, int height, int alpha, f32 texCoords[], Texture tex){
    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(texCoords[0], texCoords[1]);

        GX_Position3f32(x + width, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(texCoords[2], texCoords[3]);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(texCoords[4], texCoords[5]);

        GX_Position3f32(x, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(texCoords[6], texCoords[7]);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawTextureFromCorners(f32* corners, Texture tex) {
    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);
    if (tex.textureFormat >= GX_TF_CI4) {
        GX_LoadTlut(&tex.tLut, 0);
    }

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(corners[0], corners[1], 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(corners[2], corners[3], 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, 0);

        GX_Position3f32(corners[4], corners[5], 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, (f32)tex.realHeight / tex.height);

        GX_Position3f32(corners[6], corners[7], 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(0, (f32)tex.realHeight / tex.height);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

void drawRectangle(int x, int y, int width, int height, u32 rgba) {
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x + width, y, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x, y + height, 0);
        GX_Color1u32(rgba);
    GX_End();
}

void drawEmptyRectangle(int x, int y, int width, int height, float lineWidth, u32 rgba) {
    drawLine(x, y, x + width, y, lineWidth, rgba);
    drawLine(x + width, y, x + width, y + height, lineWidth, rgba);
    drawLine(x + width, y + height, x, y + height, lineWidth, rgba);
    drawLine(x, y + height, x, y, lineWidth, rgba);
}

void draw4ColorsRectangle(int x, int y, int width, int height, u32 rgba1, u32 rgba2, u32 rgba3, u32 rgba4) {
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color1u32(rgba1);

        GX_Position3f32(x + width, y, 0);
        GX_Color1u32(rgba2);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color1u32(rgba3);

        GX_Position3f32(x, y + height, 0);
        GX_Color1u32(rgba4);
    GX_End();
}

void drawRectangleFromCorners(f32* corners, u32 rgba) {
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    for (int i = 0; i < 4; i++) {
        GX_Position3f32(corners[2*i], corners[2*i+1], 0);
        GX_Color1u32(rgba);
    }
    GX_End();
}

void drawLine(int x1, int y1, int x2, int y2, float width, u32 rgba) {
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    float angle = atan2(y1 - y2, x2 - x1);
    float halfwidth = width / 2.0f;
    float hwsin = halfwidth * sin(angle);
    float hwcos = halfwidth * cos(angle);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x1 - hwsin, y1 - hwcos, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x1 + hwsin, y1 + hwcos, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x2 + hwsin, y2 + hwcos, 0);
        GX_Color1u32(rgba);

        GX_Position3f32(x2 - hwsin, y2 - hwcos, 0);
        GX_Color1u32(rgba);
    GX_End();
}

// Sets the texture coordinate wrapping mode for a given texture: GX_CLAMP, GX_REPEAT or GX_MIRROR
void setTextureST(GXTexObj* texObj, u8 wrap_s, u8 wrap_t) {
    GX_InitTexObjWrapMode(texObj, wrap_s, wrap_t);
}
