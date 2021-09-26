#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <stdio.h>
#include "gpio.h"
#include "i2c.h"

#define I2C_DELAY       10
#define I2C_CSTRDELAY   10
#define I2C_HALFDELAY (I2C_DELAY >> 1)

static u8 _i2cMode = I2C_MODE_PUSHPULL;
static mutex_t _i2cMutex = 0;

extern void udelay(int us);

#define i2c_udelay(us) udelay(us)

/*void i2c_udelay(int us) {
    u64 now = gettime();
    while (ticks_to_microsecs(gettime() - now) < us);
}*/

u8 i2c_getSDA(void) {
    LWP_MutexLock(_i2cMutex);
    if (HW_GPIOB_IN & GPIO_SDA) {
        LWP_MutexUnlock(_i2cMutex);
        return 1;
    }

    LWP_MutexUnlock(_i2cMutex);
    return 0;
}

u8 i2c_getSCL(void) {
    LWP_MutexLock(_i2cMutex);
    if (HW_GPIOB_IN & GPIO_SCL) {
        LWP_MutexUnlock(_i2cMutex);
        return 1;
    }

    LWP_MutexUnlock(_i2cMutex);
    return 0;
}

void i2c_setSDADir(u8 dir) {
    LWP_MutexLock(_i2cMutex);
    u32 val = HW_GPIOB_DIR & (~GPIO_SDA);
    val |= (dir << 15);
    HW_GPIOB_DIR = val;
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_setSCLDir(u8 dir) {
    LWP_MutexLock(_i2cMutex);
    u32 val = HW_GPIOB_DIR & (~GPIO_SCL);
    val |= (dir << 14);
    HW_GPIOB_DIR = val;
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_setSDAOut(u8 out) {
    LWP_MutexLock(_i2cMutex);
    u32 val = HW_GPIOB_OUT & (~GPIO_SDA);
    val |= (out << 15);
    HW_GPIOB_OUT = val;
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_setSCLOut(u8 out) {
    LWP_MutexLock(_i2cMutex);
    u32 val = HW_GPIOB_OUT & (~GPIO_SCL);
    val |= (out << 14);
    HW_GPIOB_OUT = val;
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_setSCL(u8 s) {
    LWP_MutexLock(_i2cMutex);
    if (_i2cMode == I2C_MODE_OPENDRAIN) {
        //mask32(HW_GPIOB_OUT_ADDR, GPIO_SCL, s ? GPIO_SCL : 0);
        //mask32(HW_GPIOB_DIR_ADDR, GPIO_SCL, s ? 0 : GPIO_SCL);
        if (!s)
            i2c_setSCLOut(s);
        i2c_setSCLDir(s ^ 1);

        //Clock stretching
        if (s == 1) {
            do {
                i2c_udelay(I2C_CSTRDELAY);
            } while(!i2c_getSCL());
        }
    } else {
        i2c_setSCLOut(s);
        //mask32(HW_GPIOB_OUT_ADDR, GPIO_SCL, s ? GPIO_SCL : 0);
    }
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_setSDA(u8 s) {
    LWP_MutexLock(_i2cMutex);
    if (!s)
        i2c_setSDAOut(s);
    i2c_setSDADir(s ^ 1);
    LWP_MutexUnlock(_i2cMutex);
    //mask32(HW_GPIOB_OUT_ADDR, GPIO_SDA, s ? GPIO_SDA : 0);
    //mask32(HW_GPIOB_DIR_ADDR, GPIO_SDA, s ? 0 : GPIO_SDA);
}

/*inline void i2c_setSCL(u8 s) {
    u32 val = HW_GPIOB_OUT & (~GPIO_SCL);
    val |= (s << 14);
    HW_GPIOB_OUT = val;
}

inline void i2c_setSDA(u8 s) {
    u32 val = HW_GPIOB_OUT & (~GPIO_SDA);
    val |= (s << 15);
    HW_GPIOB_OUT = val;
}

inline u8 i2c_getSDA(void) {
    if (HW_GPIOB_IN & GPIO_SDA) return 1;
    return 0;
}

inline u8 i2c_getSCL(void) {
    if (HW_GPIOB_IN & GPIO_SCL) return 1;
    return 0;
}*/


#if 0
inline void i2c_setSCLDir(u8 dir) {
    i2c_setSCL(dir ^ 1);
    /*u32 timeout = 100000;
    u32 val = HW_GPIOB_DIR & (~GPIO_SCL);
    val |= (dir << 14);
    HW_GPIOB_DIR = val;

    if (dir == 0) {
        do {
            i2c_udelay(I2C_DELAY);
            //timeout--;
        } while(!i2c_getSCL() && timeout);
    }*/

}
#endif

void i2c_setMode(u8 mode) {
    _i2cMode = mode;
}

void i2c_init(void) {
    if (!_i2cMutex)
        LWP_MutexInit(&_i2cMutex, true);

    /*
     * VIDEO_Init() leaves SDA high and SCL low.
     * Generate a stop condition to bring both high.
    */
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(0);
    i2c_udelay(I2C_DELAY);
    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);
    i2c_setSDA(1);
    i2c_udelay(I2C_DELAY);
    i2c_udelay(1000);
    LWP_MutexUnlock(_i2cMutex);
#if 0
    i2c_setSDAOut(0);
    i2c_setSDADir(1);
    //i2c_setSDA(0);
    i2c_udelay(I2C_DELAY);
#ifdef SCL_OD_MODE
    i2c_setSCLDir(0);
#else
    i2c_setSCLOut(1);
#endif
    //i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);
    i2c_setSDADir(0);
    //i2c_setSDA(1);
    i2c_udelay(I2C_DELAY);
    i2c_udelay(1000);
#ifdef SCL_OD_MODE
    i2c_setSCLOut(0);
#endif
    i2c_setSDAOut(0);
    i2c_udelay(1000);
#endif
}

void i2c_deinit(void) {
    LWP_MutexLock(_i2cMutex);
    /*i2c_setSCLDir(1);
    i2c_udelay(I2C_DELAY);
    i2c_setSDADir(1);
    i2c_udelay(I2C_DELAY);*/
    //HW_GPIOB_OUT &= ~GPIO_I2C;
    //HW_GPIOB_DIR &= ~GPIO_I2C; //On boot both are set as outputs
    //HW_GPIOB_IN &= ~GPIO_I2C;
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_start(void) {
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_stop(void) {
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_restart(void) {
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_DELAY);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_ack(void) {
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_nack(void) {
    LWP_MutexLock(_i2cMutex);
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
    LWP_MutexUnlock(_i2cMutex);
}

u8 i2c_sendByte(u8 byte) {
    LWP_MutexLock(_i2cMutex);
    int i;

    for (i = 0; i < 8; i++) {
        if (byte & 0x80)
            i2c_setSDA(1);
        else
            i2c_setSDA(0);
        i2c_udelay(I2C_HALFDELAY);

        i2c_setSCL(1);
        i2c_udelay(I2C_DELAY);

        i2c_setSCL(0);
        i2c_udelay(I2C_HALFDELAY);

        byte <<= 1;
    }

    //Shift in ACK
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    if (i2c_getSDA()) {
        //No ACK, generate STOP condition
        i2c_setSCL(0);
        i2c_udelay(I2C_HALFDELAY);

        i2c_stop();
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
    LWP_MutexUnlock(_i2cMutex);

    return 1;
}

u8 i2c_getByte(void) {
    LWP_MutexLock(_i2cMutex);
    int i;

    u8 byte = 0;

    //Release SDA
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    for (i = 0; i < 8; i++) {
        byte <<= 1;

        i2c_setSCL(1);
        i2c_udelay(I2C_DELAY);

        if (i2c_getSDA()) byte |= 1;

        i2c_setSCL(0);
        i2c_udelay(I2C_DELAY);

    }
    LWP_MutexUnlock(_i2cMutex);

    return byte;
}

u8 i2c_read8(u8 addr, u8 reg, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 output;
    u8 ret;
    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr); //Write
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    output = i2c_getByte();

    i2c_nack();

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);

    return output;
}

u16 i2c_read16(u8 addr, u8 reg, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u16 output;
    u8 ret;
    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr); //Write
    if (!ret) {
        i2c_stop();
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return 0;
    }

    output = i2c_getByte();
    i2c_ack();
    output |= (i2c_getByte() << 8);

    i2c_nack();

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);

    return output;
}

void i2c_readBuffer(u8 addr, u8 reg, u8* error, u8* buffer, u16 len) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 ret;
    int i;
    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr); //Write
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }
    for (i = 0; i < len; i++) {
        buffer[i] = i2c_getByte();
        if (i == len - 1) {
            i2c_nack();
        } else {
            i2c_ack();
        }
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_write8(u8 addr, u8 reg, u8 value, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 ret;

    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(value);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_write16(u8 addr, u8 reg, u16 value, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 ret;

    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(value & 0xFF);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(value >> 8);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_writeBuffer(u8 addr, u8 reg, u8* data, u16 len, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 ret;

    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    while(len--) {
        ret = i2c_sendByte(*data++);
        if (!ret) {
            i2c_stop();
            printf("Error at line: %d\n", __LINE__);
            if (error != NULL) *error = 1;
            _CPU_ISR_Restore(level);
            LWP_MutexUnlock(_i2cMutex);
            return;
        }
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);
}

void i2c_writeReadBuffer(u8 addr, u8 reg, u8* dataOut, u16 lenOut, u8* dataIn, u16 lenIn, u8* error) {
    LWP_MutexLock(_i2cMutex);
    u32 level;
    u8 ret;
    u16 i;

    if (error != NULL) *error = 0;

    _CPU_ISR_Disable(level);

    i2c_start();

    ret = i2c_sendByte(addr);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }

    while(lenOut--) {
        ret = i2c_sendByte(*dataOut++);
        if (!ret) {
            i2c_stop();
            printf("Error at line: %d\n", __LINE__);
            if (error != NULL) *error = 1;
            _CPU_ISR_Restore(level);
            LWP_MutexUnlock(_i2cMutex);
            return;
        }
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        LWP_MutexUnlock(_i2cMutex);
        return;
    }
    for (i = 0; i < lenIn; i++) {
        dataIn[i] = i2c_getByte();
        if (i == lenIn - 1) {
            i2c_nack();
        } else {
            i2c_ack();
        }
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
    LWP_MutexUnlock(_i2cMutex);
}
