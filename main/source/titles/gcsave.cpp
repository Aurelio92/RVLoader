#include <fstream>
#include <malloc.h>

#include "gcsave.h"

enum SAVE_GFX_FORMAT {
    SAVE_GFX_FORMAT_NO_DATA     =   0x00,
    SAVE_GFX_FORMAT_SHARED_CI8  =   0x01,
    SAVE_GFX_FORMAT_RGB5A3      =   0x02,
    SAVE_GFX_FORMAT_UNIQUE_CI8  =   0x03,
};

#define SAVE_GFX_FORMAT_MASK    0x03

GCSave::GCSave() {
    icons = NULL;
    NIcons = 0;
}

GCSave::GCSave(std::string filename, u32 gameCodeMatch) {
    DirEntry dirEntry;
    u32 dirIndex;
    u32 iconOffset;
    icons = NULL;
    NIcons = 0;

    std::ifstream is(filename, std::ifstream::binary);
    if (!is) {
        return;
    }

    printf("GCSave loading %s\n", filename.c_str());

    //Offset to directory listing
    for (u32 dirOffset = 0x2000; dirOffset <= 0x4000; dirOffset += 0x2000) {
        is.seekg(dirOffset, is.beg);
        for (dirIndex = 0; dirIndex < 127; dirIndex++) {
            is.read((char*)&dirEntry, sizeof(DirEntry));
            if (gameCodeMatch && dirEntry.gameCode == gameCodeMatch) {
                break;
            } else if (!gameCodeMatch && dirEntry.gameCode != 0xFFFFFFFF) { //If gameCodeMatch is zero, stop on first save found
                break;
            }
        }

        //Sanity checks. Otherwise try directory backup
        if (dirEntry.lastSaveTimestamp != 0xFFFFFFFF &&
            dirEntry.gfxDataOffset != 0xFFFFFFFF) {
            break;
        }
    }

    if (dirEntry.gameCode == 0xFFFFFFFF || (gameCodeMatch && dirEntry.gameCode != gameCodeMatch)) {
        is.close();
        return;
    }

    switch (dirEntry.bannerFormat & SAVE_GFX_FORMAT_MASK) {
        case SAVE_GFX_FORMAT_NO_DATA:
            iconOffset = (dirEntry.firstBlockOffset << 13) + dirEntry.gfxDataOffset;
        break;

        case SAVE_GFX_FORMAT_SHARED_CI8:
        case SAVE_GFX_FORMAT_UNIQUE_CI8:
            iconOffset = (dirEntry.firstBlockOffset << 13) + dirEntry.gfxDataOffset + 0xc00 + 0x200;
        break;

        case SAVE_GFX_FORMAT_RGB5A3:
            iconOffset = (dirEntry.firstBlockOffset << 13) + dirEntry.gfxDataOffset + 0x1800;
        break;
    }

    printf("firstBlockOffset: %u\n", dirEntry.firstBlockOffset);
    printf("gfxDataOffset: %u\n", dirEntry.gfxDataOffset);
    printf("bannerFormat: %02X\n", dirEntry.bannerFormat);
    printf("iconFormat: %04X\n", dirEntry.iconFormat);
    printf("iconAnimationSpeed: %04X\n", dirEntry.iconAnimationSpeed);
    printf("iconOffset: %04X\n", iconOffset);

    u16 tempIconFormat = dirEntry.iconFormat;
    u16 tempIconSpeed = dirEntry.iconAnimationSpeed;
    while (tempIconFormat & SAVE_GFX_FORMAT_MASK) {
        iconAnimation.addStep(millisecs_to_ticks((tempIconSpeed & 3) * 4 * 1000 / 60), NIcons, NIcons + 1);
        //iconAnimation.addStep(1000, NIcons << 8, (NIcons + 1) << 8);
        NIcons++;
        tempIconFormat >>= 2;
        tempIconSpeed >>= 2;
    }
    iconAnimation.addReturnToHomeStep(0);
    iconAnimation.resume();

    icons = new GuiImage[NIcons];
    tempIconFormat = dirEntry.iconFormat;
    for (u8 i = 0; i < NIcons; i++) {
        switch (dirEntry.iconFormat & SAVE_GFX_FORMAT_MASK) {
            case SAVE_GFX_FORMAT_SHARED_CI8: {
                u8* iconData = (u8*)memalign(32, 32 * 32);
                u8* paletteData = (u8*)memalign(32, 0x200);
                is.seekg(iconOffset + i * 32 * 32, is.beg);
                is.read((char*)iconData, 32 * 32);
                is.seekg(iconOffset + NIcons * 32 * 32, is.beg);
                is.read((char*)paletteData, 0x200);
                icons[i] = GuiImage(iconData, paletteData, 32, 32, 0x100U);
                free(iconData);
                free(paletteData);
            } break;

            case SAVE_GFX_FORMAT_UNIQUE_CI8: {
                u8* iconData = (u8*)memalign(32, 32 * 32);
                u8* paletteData = (u8*)memalign(32, 0x200);
                is.seekg(iconOffset + i * (32 * 32 + 0x200), is.beg);
                is.read((char*)iconData, 32 * 32);
                is.read((char*)paletteData, 0x200);
                icons[i] = GuiImage(iconData, paletteData, 32, 32, 0x100U);
                free(iconData);
                free(paletteData);
            } break;

            case SAVE_GFX_FORMAT_RGB5A3: {
                u8* iconData = (u8*)memalign(32, 32 * 32 * sizeof(u16));
                is.seekg(iconOffset + i * 32 * 32 * sizeof(u16), is.beg);
                is.read((char*)iconData, 32 * 32 * sizeof(u16));
                icons[i] = GuiImage(iconData, 32, 32);
                free(iconData);
            } break;
        }
    }

    is.close();
}

GCSave::GCSave(const GCSave& src) {
    if (src.NIcons != 0 && src.icons != NULL) {
        this->NIcons = src.NIcons;
        this->icons = new GuiImage[this->NIcons];
        for (u32 i = 0; i < this->NIcons; i++) {
            this->icons[i] = src.icons[i];
        }
        this->iconAnimation = src.iconAnimation;
    } else {
        this->NIcons = 0;
        this->icons = NULL;
    }
}

GCSave::~GCSave() {
    if (icons != NULL) {
        delete [] icons;
    }
}

GCSave& GCSave::operator = (const GCSave& src) {
    if (this == &src) { //Copying itself?
        return *this;
    }

    if (src.NIcons != 0 && src.icons != NULL) {
        this->NIcons = src.NIcons;
        this->icons = new GuiImage[this->NIcons];
        for (u32 i = 0; i < this->NIcons; i++) {
            this->icons[i] = src.icons[i];
        }
        this->iconAnimation = src.iconAnimation;
    } else {
        this->NIcons = 0;
        this->icons = NULL;
    }

    return *this;
}

void GCSave::drawIcon(f32* corners) {
    int curFrame = 0;
    iconAnimation.setOutput(&curFrame);
    iconAnimation.animate();

    if (curFrame < this->NIcons)
        this->icons[curFrame].drawFromCorners(corners);
    else
        drawRectangleFromCorners(corners, RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
}
