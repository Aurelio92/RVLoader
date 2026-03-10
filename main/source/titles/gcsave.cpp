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
    // printf("GCSave constructor\n");
    icons = nullptr;
    NIcons = 0;
}

GCSave::GCSave(std::string filename, u32 gameCodeMatch) {
    // printf("GCSave constructor with filename: %s\n", filename.c_str());
    loadSave(filename, gameCodeMatch);
    icons = nullptr;
    NIcons = 0;
}

GCSave::GCSave(const GCSave& src) :
    icons(nullptr),
    NIcons(src.NIcons), 
    iconAnimation(src.iconAnimation) {
    // printf("GCSave copy constructor\n");
    if (src.icons != nullptr) {
        this->icons = new GuiImage[src.NIcons];
        for (u8 i = 0; i < src.NIcons; ++i) {
            this->icons[i] = src.icons[i];
        }
    }
}

GCSave::GCSave(GCSave&& src) noexcept :
    icons(src.icons), 
    NIcons(src.NIcons), 
    iconAnimation(std::move(src.iconAnimation)) {
    // printf("GCSave move constructor\n");
    src.NIcons = 0;
    src.icons = nullptr;
    src.iconAnimation.clear();
}

GCSave::~GCSave() {
    // printf("GCSave destructor\n");
    if (icons != nullptr) {
        delete [] icons;
    }
}

GCSave& GCSave::operator = (const GCSave& src) {
    // printf("GCSave copy assignment operator\n");
    if (this == &src) { //Copying itself?
        return *this;
    }

    if (src.NIcons != 0 && src.icons != nullptr) {
        this->NIcons = src.NIcons;
        this->icons = new GuiImage[this->NIcons];
        for (u32 i = 0; i < this->NIcons; i++) {
            this->icons[i] = src.icons[i];
        }
        this->iconAnimation = src.iconAnimation;
    } else {
        this->NIcons = 0;
        this->icons = nullptr;
    }

    return *this;
}

GCSave& GCSave::operator=(GCSave&& src) noexcept {
    // printf("GCSave move assignment operator\n");
    if (this == &src) {
        return *this;
    }

    delete[] this->icons;
    this->icons = src.icons;
    this->NIcons = src.NIcons;
    this->iconAnimation = std::move(src.iconAnimation);

    src.icons = nullptr;
    src.NIcons = 0;

    return *this;
}

void GCSave::loadSave(std::string filename, u32 gameCodeMatch) {
    u8* _GCSAVE_iconData;
    u8* _GCSAVE_paletteData;
    DirEntry dirEntry;
    u32 dirIndex;
    u32 iconOffset;

    if (icons != nullptr) {
        delete [] icons;
    }
    icons = nullptr;
    NIcons = 0;

    std::ifstream is(filename, std::ifstream::binary);
    if (!is) {
        return;
    }

    // printf("GCSave loading %s\n", filename.c_str());

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

    // printf("firstBlockOffset: %u\n", dirEntry.firstBlockOffset);
    // printf("gfxDataOffset: %u\n", dirEntry.gfxDataOffset);
    // printf("bannerFormat: %02X\n", dirEntry.bannerFormat);
    // printf("iconFormat: %04X\n", dirEntry.iconFormat);
    // printf("iconAnimationSpeed: %04X\n", dirEntry.iconAnimationSpeed);
    // printf("iconOffset: %04X\n", iconOffset);

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

    u32 iconDataSize = 0;
    switch (dirEntry.iconFormat & SAVE_GFX_FORMAT_MASK) {
        case SAVE_GFX_FORMAT_SHARED_CI8:
            iconDataSize = NIcons * 32 * 32 + 0x200;
            break;
        case SAVE_GFX_FORMAT_UNIQUE_CI8:
            iconDataSize = NIcons * (32 * 32 + 0x200);
            break;
        case SAVE_GFX_FORMAT_RGB5A3:
            iconDataSize = NIcons * 32 * 32 * sizeof(u16);
            break;
    }

    u8* iconData = new u8[iconDataSize];
    is.seekg(iconOffset, is.beg);
    is.read((char*)iconData, iconDataSize);

    
    for (u8 i = 0; i < NIcons; i++) {
        switch (dirEntry.iconFormat & SAVE_GFX_FORMAT_MASK) {
            case SAVE_GFX_FORMAT_SHARED_CI8: {
                _GCSAVE_iconData = iconData + i * 32 * 32;
                _GCSAVE_paletteData = iconData + NIcons * 32 * 32;
                icons[i] = GuiImage(_GCSAVE_iconData, _GCSAVE_paletteData, 32, 32, 0x100U);
            } break;

            case SAVE_GFX_FORMAT_UNIQUE_CI8: {
                _GCSAVE_iconData = iconData + i * (32 * 32 + 0x200);
                _GCSAVE_paletteData = _GCSAVE_iconData + 32 * 32;
                icons[i] = GuiImage(_GCSAVE_iconData, _GCSAVE_paletteData, 32, 32, 0x100U);
            } break;

            case SAVE_GFX_FORMAT_RGB5A3: {
                _GCSAVE_iconData = iconData + i * 32 * 32 * sizeof(u16);
                icons[i] = GuiImage(_GCSAVE_iconData, 32, 32);
            } break;
        }
    }

    delete[] iconData;

    is.close();
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
