#pragma once

#include <gccore.h>
#include "libgui.h"
#include "draw.h"
#include "plot.h"
#include "rect.h"

namespace Gfx
{
    void init();
    void setClearColor(u8 r, u8 g, u8 b);
    void fillScreen();
    void startDrawing();
    void endDrawing();
    void concatViewMatrix(Mtx mi, Mtx mo);
    void restoreViewMatrix();
    void identity();
    void translate(f32 x, f32 y);
    void rotate(f32 px, f32 py, f32 deg);
    void pushMatrix();
    void popMatrix();
    void getCurMatrix(Mtx out);
    size_t getMatrixStackSize();
    size_t getScissorBoxStackSize();
    Rect getCurScissorBox();
    void pushScissorBox(u32 w, u32 h);
    void popScissorBox();
    void pushIdentityScissorBox();

    class Window {
        bool scissor;
        bool done;
        public:
            Window(f32 x, f32 y, u32 w, u32 h) {
                pushMatrix();
                translate(x, y);
                if (w != 0 && h != 0) {
                    pushScissorBox(w, h);
                    scissor = true;
                } else {
                    scissor = false;
                }
                done = false;
            }

            ~Window() {
                if (scissor) {
                    popScissorBox();
                }
                popMatrix();
            }

            void operator ++() {
                done = true;
            }

            bool operator !() {
                return !done;
            }
    };

    class DrawHandler {
        bool done;
        public:
            DrawHandler() {
                startDrawing();
                done = false;
            }

            ~DrawHandler() {
                endDrawing();
            }

            void operator ++() {
                done = true;
            }

            bool operator !() {
                return !done;
            }
    };
}

#define GFXWindow(x, y, w, h) for (Gfx::Window _gfxwin(x, y, w, h); !_gfxwin; ++_gfxwin)
#define GFXDraw for (Gfx::DrawHandler _drawhandler; !_drawhandler; ++_drawhandler)
