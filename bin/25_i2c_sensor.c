//***************************************************************************************
//
// Interface with the humidity and temperature sensor via I2C.
// Exceptions are handled using the <setjmp.h> library.
// The sensor driver mirrors this Arduino library:
//     https://github.com/e-radionicacom/SHT21-Arduino-Library
//
// Circuit's diagram available in this repository.
// Date: 19-12-2021
//
//***************************************************************************************

//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>
#include <inc/hw_memmap.h>
#include <driverlib/debug.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include "macro-utils.h"
#include "uart-init.h"
#include <utils/uartstdio.h>

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define SHT21_SENSOR_I2C_ADDRESS 0x40

#define SHT21_TRIGGER_T_MEASUREMENT_NHM 0XF3  // command trig. temperature measurement
#define SHT21_TRIGGER_RH_MEASUREMENT_NHM 0XF5 // command trig. humidity measurement

//
// UARTprintf cannot print floats, so they have to be split
//   into integer and decimal part.
//
typedef struct FloatInt
{
    int32_t integer;
    int32_t decimal;
} FloatInt;

typedef enum Exception
{
    I2C0EXCEPTION_MASTER_BUSY_TIMEOUT = 1,
    I2C0EXCEPTION_MASTER_ERR_ADDR_ACK,
    I2C0EXCEPTION_MASTER_ERR_DATA_ACK,
    I2C0EXCEPTION_MASTER_ERR_ARB_LOST,
    I2C0EXCEPTION_MASTER_ERR_CLK_TOUT,
    I2C0EXCEPTION_MASTER_ERR_UNEXPECTED,
} Exception;

static const char *const ExceptionStr[] = {
    [I2C0EXCEPTION_MASTER_BUSY_TIMEOUT] = "MASTER_BUSY_TIMEOUT",
    [I2C0EXCEPTION_MASTER_ERR_ADDR_ACK] = "MASTER_ERR_ADDR_ACK",
    [I2C0EXCEPTION_MASTER_ERR_DATA_ACK] = "MASTER_ERR_DATA_ACK",
    [I2C0EXCEPTION_MASTER_ERR_ARB_LOST] = "MASTER_ERR_ARB_LOST",
    [I2C0EXCEPTION_MASTER_ERR_CLK_TOUT] = "MASTER_ERR_CLK_TOUT",
    [I2C0EXCEPTION_MASTER_ERR_UNEXPECTED] = "MASTER_ERR_UNEXPECTED",
};

static jmp_buf saveJmpBuf;
static uint32_t exceptionNumber;

static uint16_t SHT21_ReadSensor(uint8_t command);
static FloatInt SHT21_GetRelativeHumidity(void);
static FloatInt SHT21_GetTemperature(void);

static FloatInt splitFloat(float n);
static void delayMs(uint32_t ms);

static void I2C0PB23_Init(void);
static void I2C0PB23_SingleSend(uint8_t slaveAddress, uint8_t data);
static void I2C0PB23_BurstReceive(uint8_t slaveAddress, uint8_t *const dataArr, uint32_t dataLen);
static bool I2C0PB23_MasterBusy(void);
static void I2C0PB23_CheckMasterErr(void);
static Exception I2C0PB23_MasterErrToException(uint32_t masterErr);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    UART_Init();
    I2C0PB23_Init();

    if ((exceptionNumber = setjmp(saveJmpBuf)) == 0)
    {
        FloatInt humidity = SHT21_GetRelativeHumidity();
        UARTprintf("Relative humidity: %d.%d\n", humidity.integer, humidity.decimal);
    }
    else
    {
        UARTprintf("Failed to get relative humidity: %s\n", ExceptionStr[exceptionNumber]);
    }

    if ((exceptionNumber = setjmp(saveJmpBuf)) == 0)
    {
        FloatInt temperature = SHT21_GetTemperature();
        UARTprintf("Temperature: %d.%d\n", temperature.integer, temperature.decimal);
    }
    else
    {
        UARTprintf("Failed to get temperature: %s\n", ExceptionStr[exceptionNumber]);
    }

    while (1)
    {
        SysCtlSleep();
    }
}

uint16_t SHT21_ReadSensor(uint8_t command)
{
    uint32_t delay;
    if (command == SHT21_TRIGGER_RH_MEASUREMENT_NHM)
        delay = 30;
    else if (command == SHT21_TRIGGER_T_MEASUREMENT_NHM)
        delay = 85;

    I2C0PB23_SingleSend(SHT21_SENSOR_I2C_ADDRESS, command);
    delayMs(delay);

    uint8_t data[3];
    I2C0PB23_BurstReceive(SHT21_SENSOR_I2C_ADDRESS, data, 3);
    uint16_t result = (data[0] << 8);
    result += data[1];

    return result;
}

FloatInt SHT21_GetRelativeHumidity(void)
{
    uint16_t rawReading = SHT21_ReadSensor(SHT21_TRIGGER_RH_MEASUREMENT_NHM);
    rawReading &= ~0x03;
    float reading = -6.0 + 125.0 / 65536 * (float)rawReading;
    return splitFloat(reading);
}

FloatInt SHT21_GetTemperature(void)
{
    uint16_t rawReading = SHT21_ReadSensor(SHT21_TRIGGER_T_MEASUREMENT_NHM);
    rawReading &= ~0x03;
    float reading = -46.85 + 175.72 / 65536 * (float)rawReading;
    return splitFloat(reading);
}

FloatInt splitFloat(float n)
{
    uint32_t intPart = n;
    uint32_t decPart = (n - intPart) * 1000;
    return (FloatInt){
        .integer = intPart,
        .decimal = decPart};
}

void delayMs(uint32_t ms)
{
    static uint32_t clockRate = 0;
    if (!clockRate)
    {
        clockRate = SysCtlClockGet();
    }

    SysCtlDelay((clockRate / 1000) * ms);
}

void I2C0PB23_Init(void)
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_I2C0);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);
}

void I2C0PB23_SingleSend(uint8_t slaveAddress, uint8_t data)
{
    I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddress, false);
    I2CMasterDataPut(I2C0_BASE, data);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while (I2C0PB23_MasterBusy())
        ;

    I2C0PB23_CheckMasterErr();
}

void I2C0PB23_BurstReceive(uint8_t slaveAddress, uint8_t *const dataArr, uint32_t dataLen)
{
    I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddress, true);
    uint32_t dataIdx;
    for (dataIdx = 0; dataIdx < dataLen; dataIdx++)
    {
        uint32_t i2cMasterControlCommand;
        if (dataIdx == 0)
            i2cMasterControlCommand = I2C_MASTER_CMD_BURST_RECEIVE_START;
        else if (dataIdx == dataLen - 1)
            i2cMasterControlCommand = I2C_MASTER_CMD_BURST_RECEIVE_FINISH;
        else
            i2cMasterControlCommand = I2C_MASTER_CMD_BURST_RECEIVE_CONT;

        I2CMasterControl(I2C0_BASE, i2cMasterControlCommand);
        while (I2C0PB23_MasterBusy())
            ;

        I2C0PB23_CheckMasterErr();
        dataArr[dataIdx] = I2CMasterDataGet(I2C0_BASE);
    }
}

bool I2C0PB23_MasterBusy(void)
{
    uint32_t masterBusyCount = 0;
    while (I2CMasterBusy(I2C0_BASE))
    {
        delayMs(10);
        masterBusyCount++;
        if (masterBusyCount > 10)
        {
            longjmp(saveJmpBuf, I2C0EXCEPTION_MASTER_BUSY_TIMEOUT);
        }
    }
    return false;
}

void I2C0PB23_CheckMasterErr(void)
{
    uint32_t masterErr = I2CMasterErr(I2C0_BASE);
    if (masterErr == I2C_MASTER_ERR_NONE)
        return;

    longjmp(saveJmpBuf, I2C0PB23_MasterErrToException(masterErr));
}

Exception I2C0PB23_MasterErrToException(uint32_t masterErr)
{
    if (masterErr & I2C_MASTER_ERR_ADDR_ACK)
        return I2C0EXCEPTION_MASTER_ERR_ADDR_ACK;
    if (masterErr & I2C_MASTER_ERR_DATA_ACK)
        return I2C0EXCEPTION_MASTER_ERR_DATA_ACK;
    if (masterErr & I2C_MASTER_ERR_ARB_LOST)
        return I2C0EXCEPTION_MASTER_ERR_ARB_LOST;
    if (masterErr & I2C_MASTER_ERR_CLK_TOUT)
        return I2C0EXCEPTION_MASTER_ERR_CLK_TOUT;
    return I2C0EXCEPTION_MASTER_ERR_UNEXPECTED;
}
