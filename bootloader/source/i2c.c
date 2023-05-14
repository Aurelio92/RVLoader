#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>
#include <stdio.h>
#include "gpio.h"
#include "i2c.h"

#define I2C_DELAY 100
#define I2C_HALFDELAY (I2C_DELAY >> 1)
//#define SCL_OD_MODE

void i2c_udelay(int us) {
    u64 now = gettime();
    while (ticks_to_microsecs(gettime() - now) < us);
}

inline u8 i2c_getSDA(void) {
    if (HW_GPIOB_IN & GPIO_SDA) return 1;
    return 0;
}

inline u8 i2c_getSCL(void) {
    if (HW_GPIOB_IN & GPIO_SCL) return 1;
    return 0;
}

inline void i2c_setSCL(u8 s) {
    #ifdef SCL_OD_MODE
        mask32(HW_GPIOB_OUT_ADDR, GPIO_SCL, s ? GPIO_SCL : 0);
        mask32(HW_GPIOB_DIR_ADDR, GPIO_SCL, s ? 0 : GPIO_SCL);

        //Clock stretching
        if (s == 1) {
            do {
                i2c_udelay(I2C_DELAY);
            } while(!i2c_getSCL());
        }

    #else
        mask32(HW_GPIOB_OUT_ADDR, GPIO_SCL, s ? GPIO_SCL : 0);
    #endif
}

inline void i2c_setSDA(u8 s) {
    mask32(HW_GPIOB_OUT_ADDR, GPIO_SDA, s ? GPIO_SDA : 0);
    mask32(HW_GPIOB_DIR_ADDR, GPIO_SDA, s ? 0 : GPIO_SDA);
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

inline void i2c_setSDADir(u8 dir) {
    u32 val = HW_GPIOB_DIR & (~GPIO_SDA);
    val |= (dir << 15);
    HW_GPIOB_DIR = val;
}

void i2c_init(void) {
    /*
     * VIDEO_Init() leaves SDA high and SCL low.
     * Generate a stop condition to bring both high.
    */
    i2c_setSDA(0);
    i2c_udelay(I2C_DELAY);
    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);
    i2c_setSDA(1);
    i2c_udelay(I2C_DELAY);
}

void i2c_deinit(void) {
    /*i2c_setSCLDir(1);
    i2c_udelay(I2C_DELAY);
    i2c_setSDADir(1);
    i2c_udelay(I2C_DELAY);*/
    i2c_setSCL(0);
    i2c_udelay(I2C_DELAY);
    i2c_setSDA(0);
    i2c_udelay(I2C_DELAY);
    HW_GPIOB_OUT &= ~GPIO_I2C;
    HW_GPIOB_DIR &= ~GPIO_I2C; //On boot both are set as outputs
    //HW_GPIOB_IN &= ~GPIO_I2C;
}

void i2c_start(void) {
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
}

void i2c_stop(void) {
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);
}

void i2c_restart(void) {
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_DELAY);
}

void i2c_ack(void) {
    i2c_setSDA(0);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
}

void i2c_nack(void) {
    i2c_setSDA(1);
    i2c_udelay(I2C_HALFDELAY);

    i2c_setSCL(1);
    i2c_udelay(I2C_DELAY);

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);
}

u8 i2c_sendByte(u8 byte) {
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
        return 0;
    }

    i2c_setSCL(0);
    i2c_udelay(I2C_HALFDELAY);

    return 1;
}

u8 i2c_getByte(void) {
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

    return byte;
}

u8 i2c_read8(u8 addr, u8 reg, u8* error) {
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
        return 0;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return 0;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return 0;
    }

    output = i2c_getByte();
    printf("returning %u\n", output);

    i2c_nack();

    i2c_stop();

    _CPU_ISR_Restore(level);

    return output;
}

u16 i2c_read16(u8 addr, u8 reg, u8* error) {
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
        return 0;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return 0;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return 0;
    }

    output = (i2c_getByte() << 8);
    i2c_ack();
    output |= i2c_getByte();

    i2c_nack();

    i2c_stop();

    _CPU_ISR_Restore(level);

    return output;
}

void i2c_readBuffer(u8 addr, u8 reg, u8* error, u8* buffer, u16 len) {
    u32 level;
    u16 output;
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
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    i2c_restart();

    ret = i2c_sendByte(addr | 1); //Read
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
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
}

void i2c_write8(u8 addr, u8 reg, u8 value, u8* error) {
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
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    ret = i2c_sendByte(value);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
}

void i2c_write16(u8 addr, u8 reg, u16 value, u8* error) {
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
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    ret = i2c_sendByte(value >> 8);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    ret = i2c_sendByte(value & 0xFF);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
}

void i2c_writeBuffer(u8 addr, u8 reg, u8* data, u16 len, u8* error) {
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
        return;
    }

    ret = i2c_sendByte(reg);
    if (!ret) {
        i2c_stop();
        printf("Error at line: %d\n", __LINE__);
        if (error != NULL) *error = 1;
        _CPU_ISR_Restore(level);
        return;
    }

    while(len--) {
        ret = i2c_sendByte(*data++);
        if (!ret) {
            i2c_stop();
            printf("Error at line: %d\n", __LINE__);
            if (error != NULL) *error = 1;
            return;
            _CPU_ISR_Restore(level);
        }
    }

    i2c_stop();

    _CPU_ISR_Restore(level);
}
