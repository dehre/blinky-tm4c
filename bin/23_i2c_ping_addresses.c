//***************************************************************************************
//
// I2C "ping" utility to find the default I2C address of a slave device.
// In short, scan through the ~128 addresses and check the ACK/NACK bit.
// If there is an ACK (the data line held low on the 9th clock cycle),
//   a device with that address is present on the I2C bus.
// If there is no ACK, no device is present.
//
// Useful link: https://electronics.stackexchange.com/questions/76617/determining-i2c-address-without-datasheet
// Circuit's diagram available in this repository.
// Date: 08-12-2021
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
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "macro-utils.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

//
// Some addresses are reserved for special purposes, so
// only 111 addresses are available with the 7 bit address scheme.
// Source: https://www.i2c-bus.org/addressing
//
#define I2C_MIN_UNRESERVED_ADDRESS_INCLUSIVE 0x08
#define I2C_MAX_UNRESERVED_ADDRESS_INCLUSIVE 0x77

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    UART_Init();

    //
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    // Select the I2C function for these pins. This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.
    //
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_I2C0);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    //
    // Enable and initialize the I2C0 master module. Use the system clock for
    // the I2C0 module. The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    //
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    UARTprintf("Ping I2C Addresses:");
    UARTprintf("\n   Module    = I2C0");
    UARTprintf("\n   Mode      = Single Send/Receive");
    UARTprintf("\n   Rate      = 100kbps");
    UARTprintf("\n   Direction = Master -> Slave\n\n");

    uint32_t slaveAddress;
    for (
        slaveAddress = I2C_MIN_UNRESERVED_ADDRESS_INCLUSIVE;
        slaveAddress <= I2C_MAX_UNRESERVED_ADDRESS_INCLUSIVE;
        slaveAddress++)
    {
        //
        // Tell the master module what address it will place on the bus when
        // communicating with the slave. The receive parameter is set to false
        // which indicates the I2C Master is initiating writes to the slave. If
        // true, that would indicate that the I2C Master is initiating reads from
        // the slave.
        //
        I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddress, false);

        //
        // Place the data to be sent in the data register
        //
        I2CMasterDataPut(I2C0_BASE, '1');

        //
        // Initiate send of data from the master.
        //
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

        //
        // Wait until master module is done transferring.
        //
        while (I2CMasterBusy(I2C0_BASE))
            ;

        //
        // Check for errors.
        //
        uint32_t masterError = I2CMasterErr(I2C0_BASE);
        if (!(masterError & I2C_MASTER_ERR_ADDR_ACK))
        {
            UARTprintf("SUCCESS -> address 0x%x\n", slaveAddress);
        }
        else
        {
            UARTprintf("Failed  -> address 0x%x\n", slaveAddress);
        }
    }

    UARTprintf("\nDone.\n\n");
    return 0;
}
