#include <gccore.h>
#include <wiiuse/wpad.h>
#include <wiiuse/wiiuse.h>
#include "genpad.h"

namespace GenPad {
    static const int _stickThreshLoHi = 30;
    static const int _stickThreshHiLo = 20;
    static int _curHeld[4];
    static int _curDown[4];
    static int _curUp[4];
    static int _sx[4], _sy[4], _cx[4], _cy[4];

    void update() {
        for (u32 i = 0; i < 4; i++) {
            int sx = PAD_StickX(i);
            int sy = PAD_StickY(i);
            int cx = PAD_SubStickX(i);
            int cy = PAD_SubStickY(i);
            int tempHeld = PAD_ButtonsHeld(i);
            int tempHeldWii = WPAD_ButtonsHeld(i);
            struct expansion_t wiimoteExp;
            WPAD_Expansion(i, &wiimoteExp);
            int wsx = 0, wsy = 0;

            if (wiimoteExp.type == EXP_NUNCHUK) {
                wsx = (wiimoteExp.nunchuk.js.pos.x - wiimoteExp.nunchuk.js.center.x) * 127 / wiimoteExp.nunchuk.js.max.x;
                wsy = (wiimoteExp.nunchuk.js.pos.y - wiimoteExp.nunchuk.js.center.y) * 127 / wiimoteExp.nunchuk.js.max.y;
            } else if (wiimoteExp.type == EXP_CLASSIC) {
                int tempHeldClassic = wiimoteExp.classic.btns_held;
                wsx = (wiimoteExp.classic.ljs.pos.x - wiimoteExp.classic.ljs.center.x) * 127 / wiimoteExp.classic.ljs.max.x;
                wsy = (wiimoteExp.classic.ljs.pos.y - wiimoteExp.classic.ljs.center.y) * 127 / wiimoteExp.classic.ljs.max.y;

                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_RIGHT)
                    tempHeld |= PAD_BUTTON_RIGHT;
                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_LEFT)
                    tempHeld |= PAD_BUTTON_LEFT;
                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_UP)
                    tempHeld |= PAD_BUTTON_UP;
                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_DOWN)
                    tempHeld |= PAD_BUTTON_DOWN;
                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_A)
                    tempHeld |= PAD_BUTTON_A;
                if (tempHeldClassic & CLASSIC_CTRL_BUTTON_B)
                    tempHeld |= PAD_BUTTON_B;
            }

            if (sx > _stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_RIGHT && sx > _stickThreshHiLo))
                tempHeld |= PAD_BUTTON_RIGHT;
            if (sx < -_stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_LEFT && sx < -_stickThreshHiLo))
                tempHeld |= PAD_BUTTON_LEFT;
            if (sy > _stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_UP && sy > _stickThreshHiLo))
                tempHeld |= PAD_BUTTON_UP;
            if (sy < -_stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_DOWN && sy < -_stickThreshHiLo))
                tempHeld |= PAD_BUTTON_DOWN;

            //Map wsx to the dpad only if it's not coming from the nunchuk or if C is not pressed
            if (wiimoteExp.type != EXP_NUNCHUK || (wiimoteExp.type == EXP_NUNCHUK && !(tempHeldWii & WPAD_NUNCHUK_BUTTON_C))) {
                if (wsx > _stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_RIGHT && wsx > _stickThreshHiLo))
                    tempHeld |= PAD_BUTTON_RIGHT;
                if (wsx < -_stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_LEFT && wsx < -_stickThreshHiLo))
                    tempHeld |= PAD_BUTTON_LEFT;
                if (wsy > _stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_UP && wsy > _stickThreshHiLo))
                    tempHeld |= PAD_BUTTON_UP;
                if (wsy < -_stickThreshLoHi || (_curHeld[i] & PAD_BUTTON_DOWN && wsy < -_stickThreshHiLo))
                    tempHeld |= PAD_BUTTON_DOWN;
            }

            if (!(tempHeldWii & WPAD_BUTTON_B)) {
                if (tempHeldWii & WPAD_BUTTON_RIGHT)
                    tempHeld |= PAD_BUTTON_RIGHT;
                if (tempHeldWii & WPAD_BUTTON_LEFT)
                    tempHeld |= PAD_BUTTON_LEFT;
                if (tempHeldWii & WPAD_BUTTON_UP)
                    tempHeld |= PAD_BUTTON_UP;
                if (tempHeldWii & WPAD_BUTTON_DOWN)
                    tempHeld |= PAD_BUTTON_DOWN;
                if (tempHeldWii & WPAD_BUTTON_A)
                    tempHeld |= PAD_BUTTON_A;
                if (tempHeldWii & WPAD_BUTTON_1)
                    tempHeld |= PAD_BUTTON_B;
            }

            if (wiimoteExp.type != EXP_NUNCHUK || (wiimoteExp.type == EXP_NUNCHUK && !(tempHeldWii & WPAD_NUNCHUK_BUTTON_C))) {
                if (wsx != 0)
                    sx = wsx;
                if (wsy != 0)
                    sy = wsy;
            }

            //Map the nunchuk stick to C-stick if holding the C button on a nunchuk
            if (wiimoteExp.type == EXP_NUNCHUK && (tempHeldWii & WPAD_NUNCHUK_BUTTON_C)) {
                if (wsx != 0)
                    cx = wsx;
                if (wsy != 0)
                    cy = wsy;
            } else if (wiimoteExp.type == EXP_CLASSIC) {
                int tempX = (wiimoteExp.classic.rjs.pos.x - wiimoteExp.classic.rjs.center.x) * 127 / wiimoteExp.classic.rjs.max.x;
                int tempY = (wiimoteExp.classic.rjs.pos.y - wiimoteExp.classic.rjs.center.y) * 127 / wiimoteExp.classic.rjs.max.y;

                if (tempX != 0)
                    cx = tempX;
                if (tempY != 0)
                    cy = tempY;
            } else if (tempHeldWii & WPAD_BUTTON_B) { //Map the wiimote DPAD to the C-stick if holding B
                int tempX = 0, tempY = 0;
                if (tempHeldWii & WPAD_BUTTON_RIGHT)
                    tempX = 127;
                else if (tempHeldWii & WPAD_BUTTON_LEFT)
                    tempX = -127;

                if (tempHeldWii & WPAD_BUTTON_UP)
                    tempY = 127;
                else if (tempHeldWii & WPAD_BUTTON_DOWN)
                    tempY = -127;

                //Quick normalization if needed
                if (tempX && tempY) {
                    tempX = 89 * tempX / 127;
                    tempY = 89 * tempY / 127;
                }

                if (tempX != 0)
                    cx = tempX;
                if (tempY != 0)
                    cy = tempY;
            }

            //Map the GC DPAD to the C-stick if holding Z
            if (tempHeld & PAD_TRIGGER_Z) {
                int tempX = 0, tempY = 0;
                if (tempHeld & PAD_BUTTON_RIGHT)
                    tempX = 127;
                else if (tempHeld & PAD_BUTTON_LEFT)
                    tempX = -127;

                if (tempHeld & PAD_BUTTON_UP)
                    tempY = 127;
                else if (tempHeld & PAD_BUTTON_DOWN)
                    tempY = -127;

                //Quick normalization if needed
                if (tempX && tempY) {
                    tempX = 89 * tempX / 127;
                    tempY = 89 * tempY / 127;
                }

                if (tempX != 0)
                    cx = tempX;
                if (tempY != 0)
                    cy = tempY;
                
                tempHeld &= ~PAD_BUTTON_DPAD;
            }

            _curDown[i] = (tempHeld ^ _curHeld[i]) & tempHeld;
            _curUp[i] = ~tempHeld ^ _curHeld[i];
            _curHeld[i] = tempHeld;

            _sx[i] = sx;
            _sy[i] = sy;
            _cx[i] = cx;
            _cy[i] = cy;
        }
    }

    int down(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _curDown[channel];
    }

    int held(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _curHeld[channel];
    }

    int up(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _curUp[channel];
    }

    int stickX(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _sx[channel];
    }

    int stickY(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _sy[channel];
    }

    int subStickX(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _cx[channel];
    }

    int subStickY(u32 channel) {
        if (channel > 3)
            channel = 0;
        return _cy[channel];
    }
};
