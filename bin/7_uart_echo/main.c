//*****************************************************************************
//
// Receive characters from UART0 and retransmit them using UART0.
// The program can be tested by using a serial terminal program on a host computer.
// For reference: https://github.com/vinodstanur/stellaris-launchpad/blob/master/examples/peripherals/uart/uart_polled.c
// Good resource: https://learn.sparkfun.com/tutorials/terminal-basics/all
// Date: 01-07-2021
//
//*****************************************************************************

//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"
#include "uart-init.h"

//
// The error routine that is called if the driver library encounters an error.
//
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

int main(void)
{
    //
    // Set the clocking to run directly from the external crystal/oscillator.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    UART_Init();

    //
    // Put a string to show start of example.
    // This will display on the terminal.
    //
    UARTprintf("\n> ");

    //
    // Enter a loop to read characters from the UART, and write them back
    // (echo).  When a line end is received, the loop terminates.
    //
    char thisChar;
    do
    {
        //
        // Read a character using the blocking read function.  This function
        // will not return until a character is available.
        //
        thisChar = UARTCharGet(UART0_BASE);

        //
        // Write the same character using the blocking write function.  This
        // function will not return until there was space in the FIFO and
        // the character is written.
        //
        UARTCharPut(UART0_BASE, thisChar);

        //
        // Stay in the loop until either a CR or LF is received.
        //
    } while ((thisChar != '\n') && (thisChar != '\r'));

    //
    // Put a character to show the end of the example.
    // This will display on the terminal.
    //
    UARTCharPut(UART0_BASE, '@');

    //
    // Return no errors
    //
    return 0;
}
