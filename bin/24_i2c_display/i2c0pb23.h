#ifndef I2C0PB23_H_INCLUDED
#define I2C0PB23_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t slaveAddress;
    bool isRead;
    bool uartDebug;
} I2C0PB23_InitParams;

void I2C0PB23_Init_(I2C0PB23_InitParams params);
#define I2C0PB23_Init(...) I2C0PB23_Init_((I2C0PB23_InitParams){__VA_ARGS__})
void I2C0PB23_SingleSend(uint8_t data);
void I2C0PB23_BurstSend(const uint8_t *const dataArr, uint32_t dataLen);

#endif
