#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "draw.h"

Texture createTextureARGB8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.width = (width & 3) ? width + (4 - (width & 3)) : width;
    tex.height = (height & 3) ? height + (4 - (height & 3)) : height;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * 4);

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

    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    DCFlushRange(tex.data, tex.width * tex.height * 4);

    return tex;
}

Texture createTextureRGB8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.width = (width & 3) ? width + (4 - (width & 3)) : width;
    tex.height = (height & 3) ? height + (4 - (height & 3)) : height;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * 4);

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

    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    DCFlushRange(tex.data, tex.width * tex.height * 4);

    return tex;
}

Texture createTextureA8(u8* buffer, u16 width, u16 height) {
    u16 x, y;
    u8* d;
    u8* s;
    int r;
    int idx;
    Texture tex;

    tex.width = (width & 3) ? width + (4 - (width & 3)) : width;
    tex.height = (height & 3) ? height + (4 - (height & 3)) : height;
    tex.realWidth = width;
    tex.realHeight = height;
    tex.data = (u8*)memalign(32, tex.width * tex.height * 4);

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

    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    DCFlushRange(tex.data, tex.width * tex.height * 4);

    return tex;
}

Texture createTextureFromTPL(TPLFile *tdf, s32 id) {
    Texture tex;
    TPL_GetTextureInfo(tdf, id, NULL, &tex.realWidth, &tex.realHeight);
    tex.width = (tex.realWidth & 3) ? tex.realWidth + (4 - (tex.realWidth & 3)) : tex.realWidth;
    tex.height = (tex.realHeight & 3) ? tex.realHeight + (4 - (tex.realHeight & 3)) : tex.realHeight;
    TPL_GetTexture(tdf, id, &tex.texObj);
    return tex;
}

Texture copyTexture(Texture src) {
    Texture tex;

    tex.width = src.width;
    tex.height = src.height;
    tex.realWidth = src.realWidth;
    tex.realHeight = src.realHeight;
    tex.data = (u8*)memalign(32, tex.width * tex.height * 4);
    memcpy(tex.data, src.data, tex.width * tex.height * 4);

    GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    DCFlushRange(tex.data, tex.width * tex.height * 4);

    return tex;
}

void drawTexture(int x, int y, Texture tex) {
    //GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    //GX_SetCopyFilter(GX_FALSE, screenMode.sample_pattern, GX_FALSE, screenMode.vfilter);

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);

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
    //GX_InitTexObj(&tex.texObj, tex.data, tex.width, tex.height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    //GX_SetCopyFilter(GX_FALSE, screenMode.sample_pattern, GX_FALSE, screenMode.vfilter);

    GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(x + width, y, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, 0);

        GX_Position3f32(x + width, y + height, 0);
        GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
        GX_TexCoord2f32((f32)tex.realWidth / tex.width, (f32)tex.realHeight / tex.height);

        GX_Position3f32(x, y + height, 0);
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
