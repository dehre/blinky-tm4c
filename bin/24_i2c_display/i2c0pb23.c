//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "macro-utils.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#include "i2c0pb23.h"

static void I2C0PB23_SendMasterErrToUART0(uint8_t data);
static const char *I2C0PB23_StringifyMasterErr(uint32_t masterErr);

static bool debugging = false;

void I2C0PB23_Init_(I2C0PB23_InitParams params)
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_I2C0);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);
    I2CMasterSlaveAddrSet(I2C0_BASE, params.slaveAddress, params.isRead);

    if (params.uartDebug)
    {
        UART_Init();
        debugging = true;
    }
}

void I2C0PB23_SingleSend(uint8_t data)
{
    I2CMasterDataPut(I2C0_BASE, data);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while (I2CMasterBusy(I2C0_BASE))
        ;

    if (debugging)
    {
        I2C0PB23_SendMasterErrToUART0(data);
    }
}

void I2C0PB23_BurstSend(const uint8_t *const dataArr, uint32_t dataLen)
{
    uint32_t dataIdx;
    for (dataIdx = 0; dataIdx < dataLen; dataIdx++)
    {
        uint8_t data = dataArr[dataIdx];
        uint32_t i2cMasterControlCommand =
            dataIdx == 0
                ? I2C_MASTER_CMD_BURST_SEND_START
                : I2C_MASTER_CMD_BURST_SEND_CONT;

        I2CMasterDataPut(I2C0_BASE, data);
        I2CMasterControl(I2C0_BASE, i2cMasterControlCommand);
        while (I2CMasterBusy(I2C0_BASE))
            ;

        if (debugging)
        {
            I2C0PB23_SendMasterErrToUART0(data);
        }
    }

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_STOP);
}

void I2C0PB23_SendMasterErrToUART0(uint8_t data)
{
    uint32_t masterErr = I2CMasterErr(I2C0_BASE);
    UARTprintf("Sent 0x%x (%c): %s\n", data, data, I2C0PB23_StringifyMasterErr(masterErr));
}

const char *I2C0PB23_StringifyMasterErr(uint32_t masterErr)
{
    if (masterErr == I2C_MASTER_ERR_NONE)
        return "NO_ERROR";
    if (masterErr & I2C_MASTER_ERR_ADDR_ACK)
        return "I2C_MASTER_ERR_ADDR_ACK";
    if (masterErr & I2C_MASTER_ERR_DATA_ACK)
        return "I2C_MASTER_ERR_DATA_ACK";
    if (masterErr & I2C_MASTER_ERR_ARB_LOST)
        return "I2C_MASTER_ERR_ARB_LOST";
    if (masterErr & I2C_MASTER_ERR_CLK_TOUT)
        return "I2C_MASTER_ERR_CLK_TOUT";
    return "UNEXPECTED_ERROR";
}
