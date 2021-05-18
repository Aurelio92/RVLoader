#ifndef _I2C_H_
#define _I2C_H_

#include <gccore.h>

#define GPIO_SCL (1<<14)
#define GPIO_SDA (1<<15)
#define GPIO_I2C (GPIO_SCL | GPIO_SDA)

#define AVE_ADDR (0x70 << 1)
#define LM49450_ADDR (0x7D << 1)

#ifdef __cplusplus
extern "C" {
#endif

void i2c_setSCLDir(u8 dir);
void i2c_setSDADir(u8 dir);
void i2c_setSCL(u8 s);
void i2c_setSDA(u8 s);
u8 i2c_getSDA(void);
void i2c_init(void);
void i2c_deinit(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_restart(void);
void i2c_ack(void);
void i2c_nack(void);
u8 i2c_sendByte(u8 byte);
u8 i2c_getByte(void);

u8 i2c_read8(u8 addr, u8 reg, u8* error);
u16 i2c_read16(u8 addr, u8 reg, u8* error);
void i2c_readBuffer(u8 addr, u8 reg, u8* error, u8* buffer, u16 len);
void i2c_write8(u8 addr, u8 reg, u8 value, u8* error);
void i2c_write16(u8 addr, u8 reg, u16 value, u8* error);
void i2c_writeBuffer(u8 addr, u8 reg, u8* data, u16 len, u8* error);

#ifdef __cplusplus
}
#endif

#endif