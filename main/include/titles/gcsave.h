#pragma once

#include <gccore.h>
#include <string>
#include <libgui.h>

class GCSave {
    typedef struct {
        u32 gameCode;
        u16 makerCode;
        u8 padding1;
        u8 bannerFormat;
        char filename[0x20];
        u32 lastSaveTimestamp;
        u32 gfxDataOffset;
        u16 iconFormat;
        u16 iconAnimationSpeed;
        u8 filePermission;
        u8 copyCounter;
        u16 firstBlockOffset;
        u16 fileLength;
        u16 padding2;
        u32 commentsOffset;
    } DirEntry;

    GuiImage* icons;
    u8 NIcons;
    Animation<int> iconAnimation;

public:
    GCSave();
    GCSave(std::string filename, u32 gameCodeMatch);
    GCSave(std::string filename) : GCSave(filename, 0) {}
    GCSave(const GCSave& src);
    ~GCSave();
    GCSave& operator = (const GCSave& src);

    void drawIcon(f32* corners);
};