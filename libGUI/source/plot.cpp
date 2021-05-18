#include <gccore.h>
#include <math.h>
#include "gfx.h"
#include "plot.h"

Plot::Plot() {
    width = 0;
    height = 0;
    lineWidth = 1.0;
    axWidth = 0;
    axHeight = 0;
    axXCoord = 0;
    axYCoord = 0;
    bgColor = RGBA8(0x00, 0x00, 0x00, 0x00);
    font = NULL;
}

Plot::Plot(int _width, int _height) {
    width = _width;
    height = _height;
    lineWidth = 1.0;
    axWidth = _width;
    axHeight = _height;
    axXCoord = 0;
    axYCoord = 0;
    bgColor = RGBA8(0x00, 0x00, 0x00, 0x00);
    font = NULL;
}

Plot::~Plot() {
    for (auto& it : xTicks) {
        free(it.text);
    }
    for (auto& it : yTicks) {
        free(it.text);
    }
    for (auto& it : yTicksAlt) {
        free(it.text);
    }
    xTicks.clear();
    yTicks.clear();
    yTicksAlt.clear();
}

void Plot::setXlim(float x1, float x2) {
    xlim[0] = x1;
    xlim[1] = x2;
}

void Plot::setYlim(float y1, float y2) {
    ylim[0] = y1;
    ylim[1] = y2;
}

void Plot::setYlimAlt(float y1, float y2) {
    ylimAlt[0] = y1;
    ylimAlt[1] = y2;
}

void Plot::setSize(int _width, int _height) {
    width = _width;
    height = _height;
}

void Plot::setAxSize(int _width, int _height){
    axWidth = _width;
    axHeight = _height;
}

void Plot::setAxPos(int _x, int _y){
    axXCoord = _x;
    axYCoord = _y;
}

void Plot::setLineWidth(float _lineWidth) {
    lineWidth = _lineWidth;
}

void Plot::setBackgroundColor(u32 _color) {
    bgColor = _color;
}

void Plot::addXTick(const char* _text, float _value) {
    Tick tick;
    tick.text = (char*)malloc(sizeof(char) * (strlen(_text) + 1));
    strcpy(tick.text, _text);
    tick.value = _value;
    xTicks.push_back(tick);
}

void Plot::addYTick(const char* _text, float _value) {
    Tick tick;
    tick.text = (char*)malloc(sizeof(char) * (strlen(_text) + 1));
    strcpy(tick.text, _text);
    tick.value = _value;
    yTicks.push_back(tick);
}

void Plot::addYTickAlt(const char* _text, float _value) {
    Tick tick;
    tick.text = (char*)malloc(sizeof(char) * (strlen(_text) + 1));
    strcpy(tick.text, _text);
    tick.value = _value;
    yTicksAlt.push_back(tick);
}

void Plot::setFont(Font* _font) {
    font = _font;
}

void Plot::plot(float* x, float* y, u32 nEl, bool computeLimits, u32 color) {
    if (nEl < 2UL) return;

    //Compute limits
    if (computeLimits) {
        xlim[0] = x[0];
        xlim[1] = x[0];
        ylim[0] = y[0];
        ylim[1] = y[0];
        for (u32 i = 0UL; i < nEl; i++) {
            if (x[i] < xlim[0]) {
                xlim[0] = x[i];
            }
            if (x[i] > xlim[1]) {
                xlim[1] = x[i];
            }
            if (y[i] < ylim[0]) {
                ylim[0] = y[i];
            }
            if (y[i] > ylim[1]) {
                ylim[1] = y[i];
            }
        }
    }

    //Start drawing
    //Draw background
    Gfx::pushScissorBox(width, height);
    drawRectangle(0, 0, width, height, bgColor);

    //Move to the axes coordinates
    Gfx::pushMatrix();
    Gfx::translate(axXCoord, axYCoord);
    Gfx::pushScissorBox(axWidth, axHeight);

    //Plot the curve
    float halfwidth = lineWidth / 2.0f;
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4 * (nEl - 1));
    for (u32 i = 0UL; i < nEl - 1; i++) {
        float x1 = (axWidth - 1) * (x[i] - xlim[0]) / (xlim[1] - xlim[0]);
        float y1 = axHeight - 1 - (axHeight - 1) * (y[i] - ylim[0]) / (ylim[1] - ylim[0]);
        float x2 = (axWidth - 1) * (x[i + 1] - xlim[0]) / (xlim[1] - xlim[0]);
        float y2 = axHeight - 1 - (axHeight - 1) * (y[i + 1] - ylim[0]) / (ylim[1] - ylim[0]);
        float angle = atan2(y1 - y2, x2 - x1);

        GX_Position3f32(x1 - halfwidth * sin(angle), y1 - halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x1 + halfwidth * sin(angle), y1 + halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x2 - halfwidth * sin(angle), y2 - halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x2 + halfwidth * sin(angle), y2 + halfwidth * cos(angle), 0);
        GX_Color1u32(color);
    }
    GX_End();

    //Draw outer box
    drawLine(0, 0, axWidth - 1, 0, 4, RGBA8(0x00, 0x00, 0x00, 0xFF));
    drawLine(0, axHeight - 1, axWidth - 1, axHeight - 1, 4, RGBA8(0x00, 0x00, 0x00, 0xFF));
    drawLine(0, 0, 0, axHeight - 1, 4, RGBA8(0x00, 0x00, 0x00, 0xFF));
    drawLine(axWidth - 1, 0, axWidth - 1, axHeight - 1, 4, RGBA8(0x00, 0x00, 0x00, 0xFF));

    //Leave the axes coordinates
    Gfx::popScissorBox();
    Gfx::popMatrix();

    //Draw ticks
    if (font != NULL) {
        //Move to the X ticks coordinates
        Gfx::pushMatrix();
        Gfx::translate(axXCoord, axYCoord + axHeight + 4);

        for (auto& it : xTicks) {
            int w = font->getTextWidth(it.text);
            float x = (axWidth - 1) * (it.value - xlim[0]) / (xlim[1] - xlim[0]);
            font->printf(x - w / 2.0, 0, it.text);
            drawLine(x, -4, x, -16, 2, RGBA8(0x00, 0x00, 0x00, 0xFF));
        }

        //Leave the X ticks coordinates
        Gfx::popMatrix();

        //Move to the Y ticks coordinates
        Gfx::pushMatrix();
        Gfx::translate(axXCoord - 4, axYCoord);

        for (auto& it : yTicks) {
            int w = font->getTextWidth(it.text);
            float y = axHeight - (axHeight - 1) * (it.value - ylim[0]) / (ylim[1] - ylim[0]);
            font->printf(-w, y - font->getSize() / 2.0, it.text);
            drawLine(4, y, 14, y, 2, RGBA8(0x00, 0x00, 0x00, 0xFF));
        }

        //Leave the Y ticks coordinates
        Gfx::popMatrix();
    }

    //Leave the world boundaries
    Gfx::popScissorBox();
}

void Plot::plotAlt(float* x, float* y, u32 nEl, bool computeLimits, u32 color) {
    if (nEl < 2UL) return;

    //Compute limits
    if (computeLimits) {
        xlim[0] = x[0];
        xlim[1] = x[0];
        ylimAlt[0] = y[0];
        ylimAlt[1] = y[0];
        for (u32 i = 0UL; i < nEl; i++) {
            if (x[i] < xlim[0]) {
                xlim[0] = x[i];
            }
            if (x[i] > xlim[1]) {
                xlim[1] = x[i];
            }
            if (y[i] < ylimAlt[0]) {
                ylimAlt[0] = y[i];
            }
            if (y[i] > ylimAlt[1]) {
                ylimAlt[1] = y[i];
            }
        }
    }

    //Start drawing
    Gfx::pushScissorBox(width, height);

    //Move to the axes coordinates
    Gfx::pushMatrix();
    Gfx::translate(axXCoord, axYCoord);
    Gfx::pushScissorBox(axWidth, axHeight);

    //Plot the curve
    float halfwidth = lineWidth / 2.0f;
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4 * (nEl - 1));
    for (u32 i = 0UL; i < nEl - 1; i++) {
        float x1 = (axWidth - 1) * (x[i] - xlim[0]) / (xlim[1] - xlim[0]);
        float y1 = axHeight - 1 - (axHeight - 1) * (y[i] - ylimAlt[0]) / (ylimAlt[1] - ylimAlt[0]);
        float x2 = (axWidth - 1) * (x[i + 1] - xlim[0]) / (xlim[1] - xlim[0]);
        float y2 = axHeight - 1 - (axHeight - 1) * (y[i + 1] - ylimAlt[0]) / (ylimAlt[1] - ylimAlt[0]);
        float angle = atan2(y1 - y2, x2 - x1);

        GX_Position3f32(x1 - halfwidth * sin(angle), y1 - halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x1 + halfwidth * sin(angle), y1 + halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x2 - halfwidth * sin(angle), y2 - halfwidth * cos(angle), 0);
        GX_Color1u32(color);

        GX_Position3f32(x2 + halfwidth * sin(angle), y2 + halfwidth * cos(angle), 0);
        GX_Color1u32(color);
    }
    GX_End();

    //Leave the axes coordinates
    Gfx::popScissorBox();
    Gfx::popMatrix();

    //Draw ticks
    if (font != NULL) {
        //Move to the Y ticks coordinates
        Gfx::pushMatrix();
        Gfx::translate(axXCoord + axWidth + 4, axYCoord);

        for (auto& it : yTicksAlt) {
            float y = axHeight - (axHeight - 1) * (it.value - ylimAlt[0]) / (ylimAlt[1] - ylimAlt[0]);
            font->printf(0, y - font->getSize() / 2.0, it.text);
            drawLine(-4, y, -14, y, 2, RGBA8(0x00, 0x00, 0x00, 0xFF));
        }

        //Leave the Y ticks coordinates
        Gfx::popMatrix();
    }

    //Leave the world boundaries
    Gfx::popScissorBox();
}
