#include <png.h>
#include <string.h>
#include "libgui.h"
#include "draw.h"
#include "gfx.h"

GuiImage::GuiImage() {
    tex.data = NULL;
    tex.palette = NULL;
    tplMode = false;
}

GuiImage::GuiImage(const char* filename) {
    png_byte bit_depth;
    png_byte color_type;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
    u32 x, y;

    char header[8]; // Check for support PNG header.

    tex.data = NULL;
    tplMode = false;

    FILE* fp = NULL;
    u8* buffer;

    fp = fopen(filename, "rb");
    if(!fp) return;

    if(fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return;
    }

    if(png_sig_cmp((png_bytep)header, 0, 8)) {
        fclose(fp);
        return;
    }

    // Create a PNG handle
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr) {
        fclose(fp);
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    if(bit_depth != 8){
        fclose(fp);
        return;
    }

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // Read File
    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    // Some helper functions to allow more pngs
    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    //if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    //    png_set_gray_1_2_4_to_8(png_ptr);
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGBA) // Swap RGBA to ARGB
        png_set_swap_alpha(png_ptr);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * png_get_image_height(png_ptr, info_ptr));
    if(!row_pointers){
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        return;
    }
    for(y = 0; y < png_get_image_height(png_ptr, info_ptr); y++)
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    fclose(fp);

    if (color_type == PNG_COLOR_TYPE_RGBA) {
        buffer = (u8*)malloc(4 * png_get_image_width(png_ptr, info_ptr) * png_get_image_height(png_ptr, info_ptr));

        for(y = 0; y < png_get_image_height(png_ptr, info_ptr); y++) {
            for (x = 0; x < png_get_image_width(png_ptr, info_ptr) * 4; x++) {
                buffer[x + y * png_get_image_width(png_ptr, info_ptr) * 4] = row_pointers[y][x];
            }
        }
        tex = createTextureARGB8(buffer, png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr));
        free(buffer);

    } else if (color_type == PNG_COLOR_TYPE_RGB) {
        buffer = (u8*)malloc(3 * png_get_image_width(png_ptr, info_ptr) * png_get_image_height(png_ptr, info_ptr));

        for(y = 0; y < png_get_image_height(png_ptr, info_ptr); y++) {
            for (x = 0; x < png_get_image_width(png_ptr, info_ptr) * 3; x++) {
                buffer[x + y * png_get_image_width(png_ptr, info_ptr) * 3] = row_pointers[y][x];
            }
        }
        tex = createTextureRGB8(buffer, png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr));
        free(buffer);
    }

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    // Free up some memory
    if(row_pointers){
        for(y = 0; y < png_get_image_height(png_ptr, info_ptr); y++){
            free(row_pointers[y]); row_pointers[y] = NULL;
        }
        free(row_pointers); row_pointers = NULL;
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
}

GuiImage::GuiImage(u8* textureBuffer, u8* paletteBuffer, u16 width, u16 height, u32 paletteNItems) {
    tex = createTextureCI8(textureBuffer, paletteBuffer, width, height, paletteNItems);
    this->width = width;
    this->height = height;
    tplMode = false;
}

GuiImage::GuiImage(u8* textureBuffer, u16 width, u16 height) {
    tex = createTextureRGB5A3(textureBuffer, width, height);
    this->width = width;
    this->height = height;
    tplMode = false;
}

GuiImage::GuiImage(const GuiImage& img) {
    tex = copyTexture(img.tex);
    width = img.width;
    height = img.height;
    tplMode = false;
}

GuiImage::GuiImage(TPLFile *tdf, s32 id) {
    tex = createTextureFromTPL(tdf, id);
    width = tex.realWidth;
    height = tex.realHeight;
    tplMode = true;
}

GuiImage::~GuiImage() {
    //tpl texture must be deallocated externally 
    if (!tplMode) {
        if (tex.data != NULL) {
            free(tex.data);
        }
        if (tex.palette != NULL) {
            free(tex.palette);
        }
        tex.data = NULL;
        tex.palette = NULL;
    }
}

void GuiImage::setSize(int _width, int _height) {
    width = _width;
    height = _height;
}

void GuiImage::draw() {
    draw(false, false, false);
}

void GuiImage::draw(bool onFocus) {
    draw(onFocus, false, false);
}

void GuiImage::draw(bool onFocus, bool xMirror, bool yMirror) {
    int tempWidth = xMirror ? -width : width;
    int tempHeight = yMirror ? -height : height;

    if (tex.data != NULL) {
        drawTextureResized(0, 0, tempWidth, tempHeight, tex);
    }
}

void GuiImage::drawAlpha(int alpha) {
    if (tex.data != NULL) {
       drawTextureResizedAlpha(0, 0, width, height, alpha, tex);
    }
}

void GuiImage::drawTextureAlphaTexCoords(int alpha, f32 textCoords[]) {
    if (tex.data != NULL) {
        drawTextureAlphaResizeTexCoords(0, 0, width, height, alpha, textCoords, tex);
    }
}

void GuiImage::drawFromCorners(f32* corners) {
    if (tex.data != NULL) {
        drawTextureFromCorners(corners, tex);
    }
}

GuiImage& GuiImage::operator = (const GuiImage& img) {
    if (this == &img) { //Copying itself?
        return *this;
    }

    if (!tplMode) {
        if (tex.data != NULL) {
            free(tex.data);
        }
        if (tex.palette != NULL) {
            free(tex.palette);
        }
    }

    if (img.tplMode) {
        this->tex = img.tex;
    } else {
        this->tex = copyTexture(img.tex);
    }

    this->width = img.width;
    this->height = img.height;
    this->tplMode = img.tplMode;

    return *this;
}

// Sets the texture coordinate wrapping mode for a given texture: GX_CLAMP, GX_REPEAT or GX_MIRROR
void GuiImage::setTextureWrap(int wrap_s, int wrap_t) {
    setTextureST(&tex.texObj, wrap_s, wrap_t);
}
