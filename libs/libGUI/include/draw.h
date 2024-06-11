#pragma once

#define RGBA8(r, g, b, a) ((r << 24) | (g << 16) | (b << 8) | a)

typedef struct {
    GXTexObj texObj;
    GXTlutObj tLut;
    u32 textureFormat;
    u8* data;
    u8* palette;
    u32 paletteNItems;
    u16 width;
    u16 height;
    u16 realWidth;
    u16 realHeight;
} Texture;

Texture createTextureARGB8(u8* buffer, u16 width, u16 height);
Texture createTextureRGB8(u8* buffer, u16 width, u16 height);
Texture createTextureA8(u8* buffer, u16 width, u16 height);
Texture createTextureFromTPL(TPLFile *tdf, s32 id);
Texture createTextureCI8(u8* textureBuffer, u8* paletteBuffer, u16 width, u16 height, u32 paletteNItems);
Texture createTextureRGB5A3(u8* textureBuffer, u16 width, u16 height);
Texture copyTexture(Texture src);

void drawTexture(int x, int y, Texture tex);
void drawTextureColor(int x, int y, u32 rgba, Texture tex);
void drawTextureResized(int x, int y, int width, int height, Texture tex);
void drawTextureAlphaResizeTexCoords(int x, int y, int width, int height, int alpha, f32 textCoords[], Texture tex);
void drawTextureResizedAlpha(int x, int y, int width, int height, int alpha, Texture tex);
void drawTextureFromCorners(f32* corners, Texture tex);
void drawRectangle(int x, int y, int width, int height, u32 rgba);
void drawEmptyRectangle(int x, int y, int width, int height, float lineWidth, u32 rgba);
void draw4ColorsRectangle(int x, int y, int width, int height, u32 rgba1, u32 rgba2, u32 rgba3, u32 rgba4);
void drawRectangleFromCorners(f32* corners, u32 rgba);
void drawLine(int x1, int y1, int x2, int y2, float width, u32 rgba);
void setTextureST(GXTexObj* texObj, u8 wrap_s, u8 wrap_t);
