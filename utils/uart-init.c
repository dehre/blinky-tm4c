//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"

#include "uart-init.h"

static bool isInitialized = false;

void UART_Init(void)
{
    if (isInitialized)
        return;

    //
    // Enable the peripherals used by this example.
    // The UART itself needs to be enabled, as well as the GPIO port
    // containing the pins that will be used.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the GPIO pin muxing for the UART function.
    // This is only necessary if your part supports GPIO pin function muxing.
    // Study the data sheet to see which functions are allocated per pin.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Since GPIO A0 and A1 are used for the UART function, they must be
    // configured for use as a peripheral function (instead of GPIO).
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 9600 baud rate, 8-N-1 operation.
    // This function uses SysCtlClockGet() to get the system clock
    // frequency.  This could be also be a variable or hard coded value
    // instead of a function call.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 9600, SysCtlClockGet());

    isInitialized = true;
}
