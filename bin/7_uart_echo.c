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

static void UARTInit();

static int main(void)
{
    //
    // Set the clocking to run directly from the external crystal/oscillator.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    UARTInit();

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

void UARTInit()
{
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
}
