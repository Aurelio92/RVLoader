#include <bitset>
#include <sstream>
#include "font.h"

FT_Library Font::library = NULL;
std::map<std::string, Font::My_Face> Font::faceCache;

Font::Font() {
    this->face = NULL;
    charBuffer = (char*)malloc(256 * (sizeof(char)));

    //TODO: Error handler
    FT_Error error;

    if (Font::library == NULL) { //Initialize freetype library only if it isn't already
        error = FT_Init_FreeType(&(Font::library));
    }

    color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
    verticalAlignment = TOP;
}

Font::Font(const char* filename, u16 size) {
    //TODO: Error handler
    FT_Error error;
    this->face = NULL;
    charBuffer = (char*)malloc(256 * (sizeof(char)));

    if (Font::library == NULL) { //Initialize freetype library only if it isn't already
        error = FT_Init_FreeType(&(Font::library));
    }

    //Load font
    //error = FT_New_Face(Font::library, filename, 0, &face);
    FILE* fp = fopen(filename, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        u32 faceMemorySize = ftell(fp);
        rewind(fp);
        u8* faceMemory = (u8*)malloc(faceMemorySize);
        fread(faceMemory, 1, faceMemorySize, fp);
        fclose(fp);

        sha1 hash;
        char tempString[64] = "";
        SHA1(faceMemory, faceMemorySize, hash);
        for (u32 i = 0; i < sizeof(hash); i++)
            sprintf(&tempString[2 * i], "%02X", hash[i]);
        auto it = faceCache.find(tempString);
        if (it == faceCache.end()) {
            //Font not cached. Add it
            My_Face f = {faceMemory, faceMemorySize};
            faceCache.insert({tempString, f});
            error = FT_New_Memory_Face(Font::library, faceMemory, faceMemorySize, 0, &face);
        } else {
            //Font already cached. Grab it
            error = FT_New_Memory_Face(Font::library, it->second.memory, it->second.size, 0, &face);
            free(faceMemory);
        }

        //Set font size
        error = FT_Set_Pixel_Sizes(this->face, 0, size);
        this->size = size;

        color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
        verticalAlignment = TOP;
    }
}

Font::Font(const u8* fontData, const u32 fontSize, u16 size) {
    //TODO: Error handler
    FT_Error error;
    this->face = NULL;
    charBuffer = (char*)malloc(256 * (sizeof(char)));

    if (Font::library == NULL) { //Initialize freetype library only if it isn't already
        error = FT_Init_FreeType(&(Font::library));
    }

    //Load font
    sha1 hash;
    char tempString[64] = "";
    u8* faceMemory = (u8*)malloc(fontSize);
    u32 faceMemorySize = fontSize;
    memcpy(faceMemory, fontData, faceMemorySize);
    SHA1(faceMemory, faceMemorySize, hash);
    for (u32 i = 0; i < sizeof(hash); i++)
        sprintf(&tempString[2 * i], "%02X", hash[i]);
    auto it = faceCache.find(tempString);
    if (it == faceCache.end()) {
        //Font not cached. Add it
        My_Face f = {faceMemory, faceMemorySize};
        faceCache.insert({tempString, f});
        error = FT_New_Memory_Face(Font::library, faceMemory, faceMemorySize, 0, &face);
    } else {
        //Font already cached. Grab it
        error = FT_New_Memory_Face(Font::library, it->second.memory, it->second.size, 0, &face);
        free(faceMemory);
    }

    //Set font size
    error = FT_Set_Pixel_Sizes(this->face, 0, size);
    this->size = size;

    color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
    verticalAlignment = TOP;
}

Font::Font(const Font& font) {
    /*FT_Error error;

    //Deallocate memory for face
    if (face != NULL) FT_Done_Face(face);
    face = NULL;
    if (faceMemory != NULL) free(faceMemory);
    faceMemory = NULL;
    if (charBuffer != NULL) free(charBuffer);
    charBuffer = (char*)malloc(256 * (sizeof(char)));

    //Delete all the loaded glyphs
    for (auto it : loadedGlyphs) {
        //Second contains the actual glyph
        delete it.second;
    }

    faceMemory = (u8*)malloc(font.faceMemorySize);
    memcpy(faceMemory, font.faceMemory, font.faceMemorySize);
    faceMemorySize = font.faceMemorySize;
    error = FT_New_Memory_Face(Font::library, faceMemory, faceMemorySize, 0, &face);

    //Set font size
    error = FT_Set_Pixel_Sizes(this->face, 0, font.size);
    this->size = font.size;

    color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
    verticalAlignment = TOP;*/
}

Font::~Font() {
    //Deallocate memory for face
    /*if (face != NULL) FT_Done_Face(face);
    face = NULL;
    if (faceMemory != NULL) free(faceMemory);
    faceMemory = NULL;
    if (charBuffer != NULL) free(charBuffer);

    //Delete all the loaded glyphs
    for (auto it : loadedGlyphs) {
        //Second contains the actual glyph
        delete it.second;
    }*/
}

Font& Font::operator = (const Font& font) {
    if (this == &font) {
        return *this;
    }

    /*FT_Error error;

    //Deallocate memory for face
    if (face != NULL) FT_Done_Face(face);
    face = NULL;
    if (faceMemory != NULL) free(faceMemory);
    faceMemory = NULL;

    //Delete all the loaded glyphs
    for (auto it : loadedGlyphs) {
        //Second contains the actual glyph
        delete it.second;
    }

    faceMemory = (u8*)malloc(font.faceMemorySize);
    memcpy(faceMemory, font.faceMemory, font.faceMemorySize);
    faceMemorySize = font.faceMemorySize;
    error = FT_New_Memory_Face(Font::library, faceMemory, faceMemorySize, 0, &face);

    //Set font size
    error = FT_Set_Pixel_Sizes(this->face, 0, font.size);
    this->size = font.size;

    color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);*/

    return *this;
}

Font::My_GlyphSlot* Font::loadChar(FT_ULong charCode) {
    //TODO: Error handler
    FT_Error error;

    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return NULL;

    My_GlyphSlot* glyph;

    //Check if the needed glyph is already loaded
    auto searchGlyph = loadedGlyphs.find(charCode);
    if (searchGlyph != loadedGlyphs.end()) { //A glyph has been found!
        glyph = searchGlyph->second; //searchGlyph->second contains the actual glyph, while searchGlyph->first contains the key (charCode in our case)
    } else { //No glyph corresponding to our charCode has already been loaded from the font => Load it!
        //Load character glyph
        error = FT_Load_Char(face, charCode, FT_LOAD_RENDER); //TODO: Error check!
        FT_GlyphSlot tempGlyph = face->glyph;
        FT_Bitmap* glyphBitmap = &(tempGlyph->bitmap);

        //Copy the loaded glyph in our loadedGlyphs map
        glyph = new My_GlyphSlot();
        glyph->metrics = tempGlyph->metrics;
        glyph->advance = tempGlyph->advance;
        glyph->width = glyphBitmap->width;
        glyph->height = glyphBitmap->rows;
        glyph->bitmap_top = tempGlyph->bitmap_top;
        glyph->bitmap_left = tempGlyph->bitmap_left;

        glyph->tex = createTextureA8(glyphBitmap->buffer, glyph->width, glyph->height);
        loadedGlyphs[charCode] = glyph;
    }

    return glyph;
}

FT_ULong Font::getCharUTF8(const char* buffer, u32* charLen) {
    u32 len = 1;
    FT_ULong ret = 0;
    char tempChar = *buffer++;
    u8 firstCharMask = 0x3F;
    u8 totalBits = 0;

    //Single byte char
    if (!(tempChar & 0x80)) {
        if (charLen != NULL)
            *charLen = len;
        return (FT_ULong)(tempChar);
    }

    //Multiple bytes char
    while ((tempChar & 0xC0) == 0xC0) {
        tempChar <<= 1;
        totalBits += 6;
        ret <<= 6;
        ret |= (*buffer++) & 0x3F;
        firstCharMask >>= 1;
        len++;
    }

    ret |= ((tempChar >> (len - 1)) & firstCharMask) << totalBits;

    if (charLen != NULL)
        *charLen = len;

    return ret;
}

int Font::drawChar(int x, int y, FT_ULong charCode) {
    int tempY = y;
    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return -1;

    if (charCode < ' ') {
        return 0;
    }

    //Load char from font file or font texture
    My_GlyphSlot* glyph = loadChar(charCode);

    //drawTextureColor(x + glyph->bitmap_left, y + this->size - glyph->bitmap_top, color, glyph->tex);
    switch (verticalAlignment) {
        case TOP:
            tempY = y + this->size - (glyph->metrics.horiBearingY >> 6);
        break;

        case CENTER:
            tempY = y + (getCharBearingY('O') >> 1) - (glyph->metrics.horiBearingY >> 6);
        break;

        case BOTTOM:
            tempY = y - (glyph->metrics.horiBearingY >> 6);
        break;
    }
    drawTextureColor(x + (glyph->metrics.horiBearingX >> 6), tempY, color, glyph->tex);
    return glyph->advance.x >> 6;
}

int Font::getCharWidth(FT_ULong charCode) {
    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return -1;

    //Load char from font file or font texture
    My_GlyphSlot* glyph = loadChar(charCode);

    return glyph->advance.x >> 6;
}

int Font::getCharHeight(FT_ULong charCode) {
    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return -1;

    //Load char from font file or font texture
    My_GlyphSlot* glyph = loadChar(charCode);

    return (this->size - (glyph->metrics.horiBearingY >> 6) + glyph->height);
}

int Font::getCharBearingX(FT_ULong charCode) {
    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return -1;

    //Load char from font file or font texture
    My_GlyphSlot* glyph = loadChar(charCode);

    return (glyph->metrics.horiBearingX >> 6);
}

int Font::getCharBearingY(FT_ULong charCode) {
    //Check if there is no library or face initialized
    if (Font::library == NULL || face == NULL) return -1;

    //Load char from font file or font texture
    My_GlyphSlot* glyph = loadChar(charCode);

    return (glyph->metrics.horiBearingY >> 6);
}

int Font::printf(int x, int y, const char* format, ...) {
    //char buffer[256];
    va_list args;
    size_t len;
    u32 charLen;
    size_t i;
    int w = 0;

    va_start(args, format);
    vsnprintf(charBuffer, 256, format, args);
    len = strlen(charBuffer);
    i = 0;
    while (i < len) {
        FT_ULong c = getCharUTF8(&charBuffer[i], &charLen);
        int delta = drawChar(x, y, c);
        x += delta;
        w += delta;
        i += charLen;
    }
    va_end(args);

    return w;
}

int Font::print(int x, int y, const std::string str) {
    u32 charLen;
    FT_ULong c;
    int w = 0;
    const char* buffer = str.c_str();

    while ((c = getCharUTF8(buffer, &charLen))) {
        int delta = drawChar(x, y, c);
        x += delta;
        w += delta;
        buffer += charLen;
    }

    return w;
}

int Font::getTextWidth(const char* format, ...) {
    va_list args;
    size_t len;
    size_t i;
    int x = 0;

    va_start(args, format);
    vsnprintf(charBuffer, 256, format, args);
    len = strlen(charBuffer);
    for (i = 0; i < len; i++) {
        x += getCharWidth(charBuffer[i]);
    }
    va_end(args);

    return x;
}

int Font::getTextHeight(const char* format, ...) {
    va_list args;
    size_t len;
    size_t i;
    int height = 0;

    va_start(args, format);
    vsnprintf(charBuffer, 256, format, args);
    len = strlen(charBuffer);
    for (i = 0; i < len; i++) {
        int temp = getCharHeight(charBuffer[i]);
        if (temp > height)
            height = temp;
    }
    va_end(args);

    return height;
}
